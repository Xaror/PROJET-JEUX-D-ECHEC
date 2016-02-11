/*
    Protector -- a UCI chess engine

    Copyright (C) 2009-2010 Raimund Heid (Raimund_Heid@yahoo.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include "search.h"
#include "matesearch.h"
#include "io.h"
#include "movegeneration.h"
#include "hash.h"
#include "evaluation.h"
#include "book.h"
#include "coordination.h"

#ifdef INCLUDE_TABLEBASE_ACCESS
#include "tablebase.h"
#endif

/* #define DEBUG_THREAD_COORDINATION */

extern bool resetGlobalHashtable;
const int FAIL_LOW_MARGIN = 20; /* was 20 */
const int HASH_DEPTH_OFFSET = 3;
INT32 checkTimeCount = 0;

int quietMoveCountLimit[32];    /* number of quiet moves to be examined @ specific restDepth */
int quietPvMoveReduction[64][64];       /* [restDepth][moveCount] */
int quietMoveReduction[64][64]; /* [restDepth][moveCount] */
int futilityMargin[21][64];     /* [restDepth][moveCount] */
int maxPieceValue[16];          /* the maximal value of a piece type */
int log2[64];                   /* log of base 2 */
int hashFlagBonus[4];

/* Prototypes */
static int searchBest(Variation * variation, int alpha, int beta,
                      const int ply, const int restDepth, const bool pvNode,
                      const bool reduction, Move * bestMove,
                      Move excludeMove);

INLINE static void checkTime(Variation * variation)
{
   const unsigned long ts = getTimestamp();

   if (ts > 0)                  /* Only check if the timestamp is valid */
   {
      const long tsGap = ts - variation->timestamp;
      const long thinkingTime = (long) (ts - variation->startTime);

      variation->timestamp = ts;
      variation->nodesAtTimeCheck = variation->nodes;

      if (thinkingTime >= 100)
      {
         long nps = ((long) (variation->nodes * 1000)) / thinkingTime;

         if (nps > 100 * 1000 && nps < 10 * 1000 * 1000)
         {
            const int div = 1000 / TIME_CHECK_INTERVALL_IN_MS;
            const long oldValue = (long) variation->nodesBetweenTimecheck;
            const long newValue = min(100 * 1000, nps / div);

            variation->nodesBetweenTimecheck = (7 * oldValue + newValue) / 8;

            /* logReport("nps=%lu, nodeGapBetweenTimeCheck=%lu\n",
               nps, variation->nodesBetweenTimecheck); */
         }
      }

      if (variation->timeLimit != 0 &&
          thinkingTime + tsGap >= variation->timeLimit &&
          variation->searchStatus == SEARCH_STATUS_RUNNING &&
          variation->ponderMode == FALSE)
      {
         variation->searchStatus = SEARCH_STATUS_TERMINATE;

#ifdef DEBUG_THREAD_COORDINATION
         logDebug
            ("Time limit reached (time=%lu ms, limit=%lu ms)).\n",
             variation->timestamp - variation->startTime,
             variation->timeLimit);
#endif

         /* logReport("Time limit reached (time=%lu ms, limit=%lu ms)).\n",
            variation->timestamp - variation->startTime,
            variation->timeLimit); */
         reportVariation(variation);
      }

      if ((checkTimeCount++ % 10) == 0 && variation->handleSearchEvent != 0)
      {
         variation->handleSearchEvent(SEARCHEVENT_STATISTICS_UPDATE,
                                      variation);
      }
   }
}

INLINE static void initPlyInfo(PlyInfo * info)
{
   info->quietMove = info->staticValueAvailable = info->gainsUpdated = FALSE;
}

INLINE static bool hasDangerousPawns(const Position * position,
                                     const Color color)
{
   if (color == WHITE)
   {
      return (bool)
         ((position->piecesOfType[WHITE_PAWN] & squaresOfRank[RANK_7]) !=
          EMPTY_BITBOARD);
   }
   else
   {
      return (bool)
         ((position->piecesOfType[BLACK_PAWN] & squaresOfRank[RANK_2]) !=
          EMPTY_BITBOARD);
   }
}

static INLINE int getStaticValue(Variation * variation, const int ply)
{
   PlyInfo *pi = &variation->plyInfo[ply];

   if (pi->staticValueAvailable == FALSE)
   {
      EvaluationBase base;

      base.ownColor = getOwnColor(variation);
      pi->staticValue = pi->refinedStaticValue =
         getValue(&variation->singlePosition, &base, variation->pawnHashtable,
                  variation->kingsafetyHashtable);
      pi->futilityMargin =
         base.futilityMargin[variation->singlePosition.activeColor];
      pi->staticValueAvailable = TRUE;
   }

   return pi->staticValue;
}

static INLINE int getStaticFutilityMargin(Variation * variation,
                                          const int ply)
{
   PlyInfo *pi = &variation->plyInfo[ply];

   if (pi->staticValueAvailable == FALSE)
   {
      EvaluationBase base;

      base.ownColor = getOwnColor(variation);
      pi->staticValue = pi->refinedStaticValue =
         getValue(&variation->singlePosition, &base, variation->pawnHashtable,
                  variation->kingsafetyHashtable);
      pi->futilityMargin =
         base.futilityMargin[variation->singlePosition.activeColor];
      pi->staticValueAvailable = TRUE;
   }

   return pi->futilityMargin;
}

static INLINE int getRefinedStaticValue(Variation * variation, const int ply)
{
   PlyInfo *pi = &variation->plyInfo[ply];

   if (pi->staticValueAvailable == FALSE)
   {
      EvaluationBase base;

      base.ownColor = getOwnColor(variation);
      pi->staticValue = pi->refinedStaticValue =
         getValue(&variation->singlePosition, &base, variation->pawnHashtable,
                  variation->kingsafetyHashtable);
      pi->futilityMargin =
         base.futilityMargin[variation->singlePosition.activeColor];
      pi->staticValueAvailable = TRUE;
   }

   return pi->refinedStaticValue;
}

static INLINE void updateGains(Variation * variation, const int ply)
{
   if (variation->plyInfo[ply].gainsUpdated == FALSE &&
       variation->plyInfo[ply - 1].quietMove != FALSE)
   {
      const int moveIndex = variation->plyInfo[ply - 1].indexCurrentMove;
      INT16 *storedGain = &variation->positionalGain[moveIndex];
      const INT16 currentDiff = (INT16)
         (getStaticValue(variation, ply) +
          variation->plyInfo[ply - 1].staticValue);

      *storedGain =
         (currentDiff >= (*storedGain) ? currentDiff : (*storedGain) - 1);
   }

   variation->plyInfo[ply].gainsUpdated = TRUE;
}

static INLINE Hashentry *getSuperiorHashEntry(Hashentry * entry1,
                                              Hashentry * entry2)
{
   if (entry1 == 0 || getHashentryMove(entry1) == NO_MOVE)
   {
      return entry2;
   }
   else if (entry2 == 0 || getHashentryMove(entry2) == NO_MOVE)
   {
      return entry1;
   }
   else
   {
      const int imp1 = getHashentryImportance(entry1);
      const int imp2 = getHashentryImportance(entry2);

      assert(getHashentryMove(entry1) != NO_MOVE);
      assert(getHashentryMove(entry2) != NO_MOVE);

      if (imp1 != imp2)
      {
         return (imp1 > imp2 ? entry1 : entry2);
      }
      else
      {
         const int f1 = getHashentryFlag(entry1) & 0x03;
         const int f2 = getHashentryFlag(entry2) & 0x03;

         return (hashFlagBonus[f1] > hashFlagBonus[f2] ? entry1 : entry2);
      }
   }
}

static INLINE bool positionIsWellKnown(Variation * variation,
                                       Position * position,
                                       const UINT64 hashKey,
                                       Hashentry ** bestTableHit,
                                       const int ply, const int alpha,
                                       const int beta, const int restDepth,
                                       const bool pvNode,
                                       const bool updateGainValues,
                                       Move * hashmove,
                                       const Move excludeMove, int *value)
{
   int tableIndex = 0;

   do
   {
      Hashentry *tableHit = getIndexedDatedEntry(variation->hashtable,
                                                 hashKey,
                                                 &tableIndex);

      if (tableHit != NULL)
      {                         /* 45% */
         const int importance = getHashentryImportance(tableHit);
         const int flag = getHashentryFlag(tableHit) & 0x03;
         int hashEntryValue;
         PlyInfo *pi = &variation->plyInfo[ply];

         *bestTableHit = getSuperiorHashEntry(*bestTableHit, tableHit);

         if (*hashmove != NO_MOVE)
         {                      /* 81% */
            assert(moveIsPseudoLegal(position, *hashmove));

            if (moveIsPseudoLegal(position, *hashmove))
            {
               assert(moveIsLegal(position, *hashmove));
            }
            else
            {
               *hashmove = NO_MOVE;
               continue;
            }
         }

         /*
            if (getHashentryStaticValue(tableHit) !=
            getStaticValue(variation, ply))
            {
            int tv = getHashentryStaticValue(tableHit);
            int sv = getStaticValue(variation, ply);

            logDebug("tv=%d sv=%d", tv, sv);
            dumpVariation(variation);
            }
          */

         assert(getHashentryStaticValue(tableHit) ==
                getStaticValue(variation, ply));
         assert(getHashentryFutilityMargin(tableHit) ==
                getStaticFutilityMargin(variation, ply));

         if (pi->staticValueAvailable == FALSE)
         {
            pi->staticValue = pi->refinedStaticValue =
               getHashentryStaticValue(tableHit);
            pi->futilityMargin = getHashentryFutilityMargin(tableHit);
            pi->staticValueAvailable = TRUE;

            if (updateGainValues)
            {
               updateGains(variation, ply);
            }
         }

         hashEntryValue =
            calcEffectiveValue(getHashentryValue(tableHit), ply);

         if (pvNode == FALSE && excludeMove != NULLMOVE &&
             restDepth <= importance && flag != HASHVALUE_EVAL)
         {                      /* 99% */
            const int hashValue = hashEntryValue;

            assert(flag == HASHVALUE_UPPER_LIMIT ||
                   flag == HASHVALUE_EXACT || flag == HASHVALUE_LOWER_LIMIT);
            assert(hashValue >= VALUE_MATED && hashValue < -VALUE_MATED);

            switch (flag)
            {
            case HASHVALUE_UPPER_LIMIT:
               if (hashValue <= alpha)
               {
                  *value = (hashValue <= VALUE_ALMOST_MATED ?
                            alpha : hashValue);

                  if (restDepth >= HASH_DEPTH_OFFSET)
                  {
                     refreshEntryDate(variation->hashtable,
                                      variation->singlePosition.hashValue,
                                      tableHit);
                  }

                  return TRUE;
               }

               if (restDepth >= HASH_DEPTH_OFFSET &&
                   hashValue < getStaticValue(variation, ply))
               {
                  variation->plyInfo[ply].refinedStaticValue = hashValue;

                  refreshEntryDate(variation->hashtable,
                                   variation->singlePosition.hashValue,
                                   tableHit);
               }
               break;

            case HASHVALUE_EXACT:
               *value = hashValue;

               if (restDepth >= HASH_DEPTH_OFFSET)
               {
                  refreshEntryDate(variation->hashtable,
                                   variation->singlePosition.hashValue,
                                   tableHit);
               }

               return TRUE;

            case HASHVALUE_LOWER_LIMIT:
               if (hashValue >= beta)
               {
                  *value = (hashValue >= -VALUE_ALMOST_MATED ?
                            beta : hashValue);

                  if (restDepth >= HASH_DEPTH_OFFSET)
                  {
                     refreshEntryDate(variation->hashtable,
                                      variation->singlePosition.hashValue,
                                      tableHit);
                  }

                  return TRUE;
               }

               if (restDepth >= HASH_DEPTH_OFFSET &&
                   hashValue > getStaticValue(variation, ply))
               {
                  variation->plyInfo[ply].refinedStaticValue = hashValue;

                  refreshEntryDate(variation->hashtable,
                                   variation->singlePosition.hashValue,
                                   tableHit);
               }
               break;

            default:;
            }
         }
      }
   }
   while (tableIndex < 4);

   if (*bestTableHit != 0)
   {
      *hashmove = (Move) getHashentryMove(*bestTableHit);
   }

   return FALSE;
}

static int searchBestQuiescence(Variation * variation, int alpha, int beta,
                                const int ply, const int restDepth,
                                Move * bestMove, const bool pvNode)
{
   const int oldAlpha = alpha;
   UINT8 hashentryFlag;
   UINT8 hashDepth = (restDepth >= 0 ? 2 : 1);
   Position *position = &variation->singlePosition;
   int best = VALUE_MATED, currentValue = VALUE_MATED, historyLimit, i;
   const int VALUE_MATE_HERE = -VALUE_MATED - ply + 1;
   const int VALUE_MATED_HERE = VALUE_MATED + ply;
   Movelist movelist;
   Move currentMove, bestReply, hashmove = NO_MOVE;
   const bool inCheck = variation->plyInfo[ply - 1].currentMoveIsCheck;
   EvaluationBase base;

   assert(alpha >= VALUE_MATED && alpha <= -VALUE_MATED);
   assert(beta >= VALUE_MATED && beta <= -VALUE_MATED);
   assert(alpha < beta);
   assert(ply > 0 && ply < MAX_DEPTH);
   assert(restDepth < DEPTH_RESOLUTION);
   assert(passiveKingIsSafe(position));
   assert((inCheck != FALSE) == (activeKingIsSafe(position) == FALSE));

   *bestMove = NO_MOVE;
   variation->plyInfo[ply].quietMove = FALSE;   /* avoid subsequent gain updates */
   movelist.positionalGain = &(variation->positionalGain[0]);

   base.futilityMargin[WHITE] = base.futilityMargin[BLACK] = 0;
   variation->nodes++;

   if (variation->nodes - variation->nodesAtTimeCheck >=
       variation->nodesBetweenTimecheck)
   {
      checkTime(variation);
   }

   if (variation->terminate &&
       (variation->threadNumber > 0 ||
        movesAreEqual(variation->bestBaseMove, NO_MOVE) == FALSE))
   {
      variation->searchStatus = SEARCH_STATUS_TERMINATE;
   }

   if (variation->searchStatus != SEARCH_STATUS_RUNNING &&
       variation->nominalDepth > 1)
   {
      return 0;
   }

   /* Check for a draw according to the 50-move-rule */
   /* ---------------------------------------------- */
   if (position->halfMoveClock > 100)
   {
      return variation->drawScore[position->activeColor];
   }

   /* Check for a draw by repetition. */
   /* ------------------------------- */
   historyLimit = POSITION_HISTORY_OFFSET + variation->ply -
      position->halfMoveClock;

   assert(historyLimit >= 0);

   for (i = POSITION_HISTORY_OFFSET + variation->ply - 4;
        i >= historyLimit; i -= 2)
   {
      if (position->hashValue == variation->positionHistory[i])
      {
         return variation->drawScore[position->activeColor];
      }
   }

   /* Probe the transposition table */
   /* ----------------------------- */
   {
      Hashentry *bestTableHit = 0;
      int hashValue;

      if (positionIsWellKnown(variation, position, position->hashValue,
                              &bestTableHit, ply, alpha, beta,
                              hashDepth, pvNode, FALSE,
                              &hashmove, NO_MOVE, &hashValue))
      {
         *bestMove = hashmove;

         return hashValue;
      }
      else if (bestTableHit != 0)
      {
         base.futilityMargin[position->activeColor] =
            getHashentryFutilityMargin(bestTableHit);
      }
   }

   if (hashmove != NO_MOVE && restDepth < 0 &&
       getNewPiece(hashmove) == NO_PIECE && inCheck == FALSE &&
       position->piece[getToSquare(hashmove)] == NO_PIECE)
   {
      hashmove = NO_MOVE;
   }

   if (inCheck == FALSE)
   {
      const bool staticValueAvailable =
         variation->plyInfo[ply].staticValueAvailable;

      assert(flipTest(position,
                      variation->pawnHashtable,
                      variation->kingsafetyHashtable) != FALSE);

      if (staticValueAvailable == FALSE)
      {
         base.ownColor = getOwnColor(variation);
         best = getValue(position,
                         &base,
                         variation->pawnHashtable,
                         variation->kingsafetyHashtable);
         variation->plyInfo[ply].staticValue =
            variation->plyInfo[ply].refinedStaticValue = best;
         variation->plyInfo[ply].futilityMargin =
            base.futilityMargin[position->activeColor];
         variation->plyInfo[ply].staticValueAvailable = TRUE;
      }
      else
      {
         best = variation->plyInfo[ply].staticValue;
      }

      if (best > alpha)
      {
         alpha = best;

         if (best >= beta)
         {
            if (staticValueAvailable == FALSE)
            {
               UINT8 hashentryFlag = HASHVALUE_EVAL;

               setDatedEntry(variation->hashtable, position->hashValue,
                             calcHashtableValue(best, ply),
                             0, packedMove(NO_MOVE), hashentryFlag,
                             (INT16) getStaticValue(variation, ply),
                             (INT16) getStaticFutilityMargin(variation, ply));
            }

            return best;
         }
      }

      currentValue = best;
   }

   if (ply >= MAX_DEPTH)
   {
      assert(flipTest(position,
                      variation->pawnHashtable,
                      variation->kingsafetyHashtable) != FALSE);

      return getStaticValue(variation, ply);
   }

   if (alpha < VALUE_MATED_HERE && inCheck == FALSE)
   {
      alpha = VALUE_MATED_HERE;

      if (alpha >= beta)
      {
         return VALUE_MATED_HERE;
      }
   }

   if (beta > VALUE_MATE_HERE)
   {
      beta = VALUE_MATE_HERE;

      if (beta <= alpha)
      {
         return VALUE_MATE_HERE;
      }
   }

   initQuiescenceMovelist(&movelist, &variation->singlePosition,
                          &variation->plyInfo[ply],
                          &variation->historyValue[0],
                          hashmove, restDepth, inCheck);
   initializePlyInfo(variation);

   while ((currentMove = getNextMove(&movelist)) != NO_MOVE)
   {
      int value, newDepth =
         (inCheck ? restDepth : restDepth - DEPTH_RESOLUTION);
      const int delta = 64 + base.futilityMargin[position->activeColor];
      int optValue = currentValue + delta +
         maxPieceValue[position->piece[getToSquare(currentMove)]];
      const Square toSquare = getToSquare(currentMove);

      if (pvNode == FALSE && inCheck == FALSE && optValue < alpha &&
          movesAreEqual(currentMove, hashmove) == FALSE &&
          pieceType(position->piece[toSquare]) != QUEEN &&
          getNewPiece(currentMove) != (Piece) QUEEN &&
          numberOfNonPawnPieces(position, position->activeColor) > 1 &&
          (pieceType(position->piece[getFromSquare(currentMove)]) != PAWN ||
           colorRank(position->activeColor, toSquare) != RANK_7) &&
          (pieceType(position->piece[toSquare]) != PAWN ||
           pawnIsPassed(position, toSquare,
                        opponent(position->activeColor)) == FALSE))
      {
         if (getNewPiece(currentMove) != NO_PIECE)
         {
            optValue +=
               maxPieceValue[getNewPiece(currentMove)] - basicValue[PAWN];
         }

         if (getToSquare(currentMove) == position->enPassantSquare &&
             pieceType(position->piece[getFromSquare(currentMove)]) == PAWN)
         {
            optValue += maxPieceValue[PAWN];
         }

         if (optValue < alpha && moveIsCheck(currentMove, position) == FALSE)
         {
            best = max(best, optValue);

            continue;
         }
      }

      assert(moveIsPseudoLegal(position, currentMove));

      if (makeMoveFast(variation, currentMove) != 0 ||
          passiveKingIsSafe(&variation->singlePosition) == FALSE)
      {
         unmakeLastMove(variation);

         continue;
      }

      variation->plyInfo[ply].currentMoveIsCheck =
         activeKingIsSafe(&variation->singlePosition) == FALSE;

      assert(position->piece[getToSquare(currentMove)] != NO_PIECE ||
             (getToSquare(currentMove) == position->enPassantSquare &&
              position->piece[getFromSquare(currentMove)] ==
              (PAWN | position->activeColor)) ||
             getNewPiece(currentMove) != NO_PIECE ||
             inCheck || variation->plyInfo[ply].currentMoveIsCheck);

      assert(inCheck != FALSE ||
             basicValue[position->piece[getFromSquare(currentMove)]] <=
             basicValue[position->piece[getToSquare(currentMove)]] ||
             seeMove(position, currentMove) >= 0);

      value = -searchBestQuiescence(variation, -beta, -alpha, ply + 1,
                                    newDepth, &bestReply, pvNode);

      unmakeLastMove(variation);

      if (variation->searchStatus != SEARCH_STATUS_RUNNING &&
          variation->nominalDepth > 1)
      {
         return 0;
      }

      if (value > best)
      {
         best = value;

         if (best > alpha)
         {
            alpha = best;
            *bestMove = currentMove;

            if (best >= beta)
            {
               break;
            }
         }
      }
   }

   if (best == VALUE_MATED)
   {
      /* mate */

      assert(inCheck != FALSE);

      best = VALUE_MATED + ply;
   }

   /* Store the value in the transposition table. */
   /* ------------------------------------------- */
   if (best > oldAlpha)
   {
      hashentryFlag =
         (best >= beta ? HASHVALUE_LOWER_LIMIT : HASHVALUE_EXACT);
   }
   else
   {
      hashentryFlag =
         (inCheck == FALSE && best == getStaticValue(variation, ply) ?
          HASHVALUE_EVAL : HASHVALUE_UPPER_LIMIT);
   }

   setDatedEntry(variation->hashtable, position->hashValue,
                 calcHashtableValue(best, ply),
                 hashDepth, packedMove(*bestMove), hashentryFlag,
                 (INT16) getStaticValue(variation, ply),
                 (INT16) getStaticFutilityMargin(variation, ply));

   return best;
}

INLINE static bool moveIsQuiet(const Move move, const Position * position)
{
   return (bool) (position->piece[getToSquare(move)] == NO_PIECE &&
                  getNewPiece(move) == NO_PIECE &&
                  (getToSquare(move) != position->enPassantSquare ||
                   pieceType(position->piece[getFromSquare(move)]) != PAWN));
}

INLINE static bool moveIsCastling(const Move move, const Position * position)
{
   return (bool) (pieceType(position->piece[getFromSquare(move)]) == KING &&
                  distance(getFromSquare(move), getToSquare(move)) == 2);
}

INLINE static bool moveAttacksSquare(const Move move, const Square square,
                                     const Position * position)
{
   const Piece piece = position->piece[getFromSquare(move)];
   Bitboard orginalMoves =
      getCaptureMoves(getFromSquare(move), piece, position->allPieces);
   Bitboard movesFromTarget =
      getCaptureMoves(getToSquare(move), piece, position->allPieces);

   return testSquare(orginalMoves, square) == FALSE &&
      testSquare(movesFromTarget, square);
}

INLINE static bool movesAreConnected(const Move firstMove,
                                     const Move secondMove,
                                     Position * position)
{
   const Square firstFrom = getFromSquare(firstMove);
   const Square firstTo = getToSquare(firstMove);
   const Square secondFrom = getFromSquare(secondMove);
   const Square secondTo = getToSquare(secondMove);
   const Piece firstPiece = position->piece[firstTo];
   const Piece secondPiece = position->piece[secondFrom];
   const bool firstPieceIsSlider =
      (bool) (firstPiece & PP_SLIDING_PIECE) != 0;
   const bool secondPieceIsSlider =
      (bool) (secondPiece & PP_SLIDING_PIECE) != 0;

   if (firstTo == secondFrom)
   {
      assert(firstPieceIsSlider == secondPieceIsSlider);

      if (secondTo == firstFrom)
      {
         return FALSE;          /* avoid shuffling */
      }

      if (firstPieceIsSlider)
      {
         const Bitboard backyards = squaresBehind[firstTo][firstFrom] |
            squaresBehind[firstFrom][secondFrom];

         return (testSquare(backyards, secondTo) == FALSE);     /* slider changes direction */
      }
      else
      {
         return (bool) distance(firstFrom, secondTo) >= 2;
      }
   }

   if (secondPieceIsSlider)
   {
      const Bitboard corridor = squaresBetween[secondFrom][secondTo];

      if (testSquare(corridor, firstFrom))
      {
         return TRUE;           /* first move was clearing square */
      }
   }

   if (secondTo == firstFrom)
   {
      return TRUE;              /* first move was clearing square */
   }

   if (firstPieceIsSlider)
   {
      const Square oppKingSquare = position->king[position->activeColor];
      const Bitboard corridor = squaresBetween[firstTo][oppKingSquare];

      if (testSquare(corridor, secondFrom) &&
          (corridor & position->allPieces) == minValue[secondFrom])
      {
         return TRUE;           /* uncovered check */
      }
   }

   return FALSE;
}

INLINE static bool moveIsDefence(const Move move, const Move threatMove,
                                 Position * position)
{
   const Square threatTarget = getToSquare(threatMove);
   const Square threatFromSquare = getFromSquare(threatMove);
   const Square fromSquare = getFromSquare(move);

   if (fromSquare == threatTarget && seeMove(position, move) >= 0)
   {
      return TRUE;              /* threatened piece was moved */
   }
   else if (basicValue[position->piece[threatFromSquare]] >=
            basicValue[position->piece[fromSquare]] &&
            (position->piece[threatFromSquare] & PP_SLIDING_PIECE) != 0 &&
            distance(threatFromSquare, threatTarget) > 1)
   {
      Bitboard sliderCorridor =
         squaresBetween[threatFromSquare][threatTarget];

      assert(pieceType(position->piece[threatFromSquare]) == QUEEN ||
             pieceType(position->piece[threatFromSquare]) == ROOK ||
             pieceType(position->piece[threatFromSquare]) == BISHOP);
      assert(sliderCorridor != EMPTY_BITBOARD);

      if (testSquare(sliderCorridor, getToSquare(move)) &&
          seeMove(position, move) >= 0)
      {
         return TRUE;           /* move blocks slider */
      }
   }

   if (basicValue[position->piece[threatTarget]] > 0 &&
       basicValue[position->piece[threatFromSquare]] >=
       basicValue[position->piece[threatTarget]])
   {
      if (moveAttacksSquare(move, threatTarget, position) &&
          seeMove(position, move) >= 0)
      {
         return TRUE;           /* move defends target */
      }
   }

   return FALSE;
}

static INLINE bool isPassedPawnMove(const Square pawnSquare,
                                    const Square targetSquare,
                                    const Position * position)
{
   const Piece piece = position->piece[pawnSquare];

   if (pieceType(piece) == PAWN)
   {
      return pawnIsPassed(position, targetSquare, pieceColor(piece));
   }
   else
   {
      return FALSE;
   }
}

static INLINE int getFutilityMargin(const int restDepth, const int moveCount)
{
   return (restDepth <= 20 ? futilityMargin[restDepth][min(63, moveCount)] :
           -VALUE_MATED);
}

static INLINE int getSingleMoveExtensionDepth(const bool pvNode)
{
   return (pvNode ? 6 : 8) * DEPTH_RESOLUTION;
}

UINT64 getHashValueAddition(Move move)
{
   const Square from = getFromSquare(move);
   const Square to = getToSquare(move);

   return GENERATED_KEYTABLE[WHITE_KING][from] ^
      GENERATED_KEYTABLE[BLACK_KING][to];
}

static int searchBest(Variation * variation, int alpha, int beta,
                      const int ply, const int restDepth, const bool pvNode,
                      const bool reduction, Move * bestMove, Move excludeMove)
{
   const int oldAlpha = alpha;
   Position *position = &variation->singlePosition;
   int best = VALUE_MATED;
   const int VALUE_MATE_HERE = -VALUE_MATED - ply + 1;
   const int VALUE_MATED_HERE = VALUE_MATED + ply;
   Movelist movelist;
   UINT8 hashentryFlag;
   int i, historyLimit, numMovesPlayed = 0;
   Move hashmove = NO_MOVE, threatMove = NO_MOVE;
   Hashentry *bestTableHit = 0;
   Move currentMove, bestReply;
   Move lastMove = variation->plyInfo[ply - 1].currentMove;
   const bool inCheck = variation->plyInfo[ply - 1].currentMoveIsCheck;
   const bool dangerousPawns =
      hasDangerousPawns(position, position->activeColor);
   const int rdBasic = restDepth / DEPTH_RESOLUTION;
   bool singularExtensionNode = FALSE;
   int hashEntryValue = 0;
   const UINT64 hashKey = (excludeMove == NO_MOVE || excludeMove == NULLMOVE ?
                           position->hashValue : position->hashValue ^
                           getHashValueAddition(excludeMove));
   const bool cutsAreAllowed = abs(beta) < -(VALUE_ALMOST_MATED);

   *bestMove = NO_MOVE;
   variation->plyInfo[ply].quietMove = FALSE;   /* avoid subsequent gain updates */
   movelist.positionalGain = &(variation->positionalGain[0]);

   if (ply + 1 > variation->selDepth)
   {
      variation->selDepth = ply + 1;
   }

   assert(alpha >= VALUE_MATED && alpha <= -VALUE_MATED);
   assert(beta >= VALUE_MATED && beta <= -VALUE_MATED);
   assert(alpha < beta);
   assert(ply > 0 && ply < MAX_DEPTH);
   assert(restDepth >= -3 * DEPTH_RESOLUTION);
   assert(passiveKingIsSafe(position));
   assert((inCheck != FALSE) == (activeKingIsSafe(position) == FALSE));

   if (restDepth < DEPTH_RESOLUTION)
   {                            /* 63% */
      int qsValue;

      assert(excludeMove == NO_MOVE);

      qsValue =
         searchBestQuiescence(variation, alpha, beta, ply, 0, bestMove,
                              pvNode);

      if (inCheck == FALSE &&
          variation->plyInfo[ply].staticValueAvailable != FALSE)
      {
         updateGains(variation, ply);
      }

      return qsValue;
   }

   variation->nodes++;

   if (variation->nodes - variation->nodesAtTimeCheck >=
       variation->nodesBetweenTimecheck)
   {
      checkTime(variation);
   }

   if (variation->terminate &&
       (variation->threadNumber > 0 ||
        movesAreEqual(variation->bestBaseMove, NO_MOVE) == FALSE))
   {
      variation->searchStatus = SEARCH_STATUS_TERMINATE;
   }

   if (variation->searchStatus != SEARCH_STATUS_RUNNING &&
       variation->nominalDepth > 1)
   {
      return 0;
   }

   /* Check for a draw according to the 50-move-rule */
   /* ---------------------------------------------- */
   if (position->halfMoveClock > 100)
   {
      return variation->drawScore[position->activeColor];
   }

   /* Check for a draw by repetition. */
   /* ------------------------------- */
   historyLimit = POSITION_HISTORY_OFFSET + variation->ply -
      position->halfMoveClock;

   assert(historyLimit >= 0);

   for (i = POSITION_HISTORY_OFFSET + variation->ply - 4;
        i >= historyLimit; i -= 2)
   {
      if (position->hashValue == variation->positionHistory[i])
      {
         return variation->drawScore[position->activeColor];
      }
   }

#ifdef INCLUDE_TABLEBASE_ACCESS
   /* Probe the tablebases in case of reduced material */
   /* ------------------------------------------------ */
   if (ply >= 2 && (pvNode || restDepth >= 6 * DEPTH_RESOLUTION) &&
       position->numberOfPieces[WHITE] +
       position->numberOfPieces[BLACK] <= 5 &&
       excludeMove == NO_MOVE && tbAvailable != FALSE)
   {
      int tbValue;

      tbValue = probeTablebase(position);

      if (tbValue != TABLEBASE_ERROR)
      {
         variation->tbHits++;

         if (tbValue == 0)
         {
            return variation->drawScore[position->activeColor];
         }

         return (tbValue > 0 ? tbValue - ply : tbValue + ply);
      }
   }
#endif

   if (ply >= 2 && rdBasic >= 4 && kpkpValueAvailable(position) &&
       getStaticValue(variation, ply) == 0 && excludeMove == NO_MOVE)
   {
      return 0;
   }

   /* Probe the transposition table */
   /* ----------------------------- */
   if (excludeMove == NO_MOVE || excludeMove == NULLMOVE)
   {
      int hashValue;

      if (positionIsWellKnown(variation, position, hashKey,
                              &bestTableHit, ply, alpha, beta,
                              restDepth + HASH_DEPTH_OFFSET, pvNode,
                              inCheck == FALSE, &hashmove, excludeMove,
                              &hashValue))
      {
         *bestMove = hashmove;

         return hashValue;
      }
   }

   if (inCheck == FALSE)
   {
      updateGains(variation, ply);
   }

   if (ply >= MAX_DEPTH)
   {
      return getStaticValue(variation, ply);
   }

   if (alpha < VALUE_MATED_HERE && inCheck == FALSE)
   {
      alpha = VALUE_MATED_HERE;

      if (alpha >= beta)
      {
         return VALUE_MATED_HERE;
      }
   }

   if (beta > VALUE_MATE_HERE)
   {
      beta = VALUE_MATE_HERE;

      if (beta <= alpha)
      {
         return VALUE_MATE_HERE;
      }
   }

   initializePlyInfo(variation);

   /* Razoring */
   if (pvNode == FALSE && restDepth < 4 * DEPTH_RESOLUTION &&
       inCheck == FALSE && hashmove == NO_MOVE &&
       excludeMove == NO_MOVE && cutsAreAllowed && dangerousPawns == FALSE)
   {
      const int limit = alpha - (204 + 32 * (rdBasic - 1));

      if (getRefinedStaticValue(variation, ply) < limit)
      {
         const int qsValue =
            searchBestQuiescence(variation, limit - 1, limit, ply, 0,
                                 bestMove, pvNode);

         if (qsValue < limit)
         {
            return qsValue;
         }
      }
   }

   /* Static nullmove pruning */
   if (pvNode == FALSE && restDepth < 4 * DEPTH_RESOLUTION &&
       inCheck == FALSE && cutsAreAllowed && excludeMove == NO_MOVE &&
       numberOfNonPawnPieces(position, position->activeColor) >= 2 &&
       !hasDangerousPawns(position, opponent(position->activeColor)))
   {
      const int limit = beta + 161 + 82 * (rdBasic - 1);

      if (getRefinedStaticValue(variation, ply) > limit)
      {
         return beta;
      }
   }

   /* Nullmove pruning with verification */
   if (restDepth >= 2 * DEPTH_RESOLUTION && inCheck == FALSE &&
       pvNode == FALSE && cutsAreAllowed && excludeMove == NO_MOVE &&
       numberOfNonPawnPieces(position, position->activeColor) >= 2 &&
       getRefinedStaticValue(variation, ply) >= beta &&
       getNumberOfPieceMoves(position, position->activeColor, 6) >= 6)
   {                            /* 16-32% */
      const int verificationReduction = 5 * DEPTH_RESOLUTION;
      int newDepth = restDepth - 3 * DEPTH_RESOLUTION;
      int nullValue;

      if (newDepth >= 4 * DEPTH_RESOLUTION &&
          getRefinedStaticValue(variation, ply) > beta + PAWN_VALUE_ENDGAME)
      {
         newDepth -= DEPTH_RESOLUTION;
      }

      if (restDepth >= verificationReduction)
      {
         newDepth -= (restDepth / 8) * DEPTH_RESOLUTION;
      }

      assert(flipTest(position,
                      variation->pawnHashtable,
                      variation->kingsafetyHashtable) != FALSE);

      makeMoveFast(variation, NULLMOVE);
      variation->plyInfo[ply].currentMoveIsCheck = FALSE;
      nullValue = -searchBest(variation, -beta, -beta + 1, ply + 1,
                              newDepth, pvNode, FALSE, &bestReply, NO_MOVE);
      unmakeLastMove(variation);
      threatMove = bestReply;

      if (restDepth - verificationReduction >= DEPTH_RESOLUTION &&
          nullValue >= beta)
      {                         /* 2% */
         const int noNullValue = searchBest(variation, alpha, beta, ply,
                                            restDepth - verificationReduction,
                                            FALSE, FALSE, &bestReply,
                                            NULLMOVE);

         if (noNullValue >= beta)
         {                      /* 99% */
            return nullValue;
         }
      }
      else if (nullValue >= beta)
      {                         /* 70% */
         return min(nullValue, VALUE_MATE_HERE);
      }
      else if (reduction &&
               restDepth < 5 * DEPTH_RESOLUTION &&
               threatMove != NO_MOVE &&
               movesAreConnected(lastMove, threatMove, position))
      {
         return beta - 1;
      }
   }

   /* Internal iterative deepening. */
   /* ----------------------------- */
   if (hashmove == NO_MOVE &&
       restDepth >= (pvNode ? 3 : 7) * DEPTH_RESOLUTION &&
       (pvNode ||
        (inCheck == FALSE &&
         getRefinedStaticValue(variation, ply) >= beta - 100)))
   {
      const Move excludeHere =
         (excludeMove != NO_MOVE ? excludeMove : NULLMOVE);
      const int searchDepth =
         (pvNode ? restDepth - 2 * DEPTH_RESOLUTION : restDepth / 2);

      searchBest(variation, alpha, beta, ply, searchDepth, pvNode,
                 FALSE, &bestReply, excludeHere);

      if (moveIsPseudoLegal(position, bestReply))
      {
         hashmove = bestReply;
      }

      if (hashmove != NO_MOVE && excludeMove == NO_MOVE &&
          restDepth >= getSingleMoveExtensionDepth(pvNode))
      {
         UINT64 hashKey = variation->singlePosition.hashValue;
         int tableIndex = 0;

         do
         {
            Hashentry *tableHit = getIndexedDatedEntry(variation->hashtable,
                                                       hashKey, &tableIndex);

            bestTableHit = getSuperiorHashEntry(bestTableHit, tableHit);
         }
         while (tableIndex < 4);
      }
   }

   if (bestTableHit != 0)
   {
      const int singleMoveExtensionDepth =
         getSingleMoveExtensionDepth(pvNode);
      const int importance =
         getHashentryImportance(bestTableHit) - HASH_DEPTH_OFFSET;

      if (restDepth >= singleMoveExtensionDepth &&
          excludeMove == NO_MOVE && hashmove != NO_MOVE &&
          importance >= restDepth - 3 * DEPTH_RESOLUTION)
      {
         singularExtensionNode = TRUE;
         hashEntryValue =
            calcEffectiveValue(getHashentryValue(bestTableHit), ply);
      }

      refreshEntryDate(variation->hashtable,
                       variation->singlePosition.hashValue, bestTableHit);
   }

   if (ply >= 2)
   {
      variation->plyInfo[ply].killerMove3 =
         variation->plyInfo[ply - 2].killerMove1;
      variation->plyInfo[ply].killerMove4 =
         variation->plyInfo[ply - 2].killerMove2;
   }
   else
   {
      variation->plyInfo[ply].killerMove3 = NO_MOVE;
      variation->plyInfo[ply].killerMove4 = NO_MOVE;
   }

   initStandardMovelist(&movelist, &variation->singlePosition,
                        &variation->plyInfo[ply],
                        &variation->historyValue[0], hashmove, inCheck);

   /* Ensure that a static value for this ply is available. */
   getStaticValue(variation, ply);

   /* Loop through all moves in this node. */
   /* ------------------------------------ */
   while ((currentMove = getNextMove(&movelist)) != NO_MOVE)
   {
      const int historyIdx = historyIndex(currentMove, position);
      const int stage = moveGenerationStage[movelist.currentStage];
      int value = 0, newDepth;
      int extension = 0, reduction = 0;
      bool check;
      const bool quietMove = moveIsQuiet(currentMove, position);
      const Square toSquare = getToSquare(currentMove);
      const bool capture = (bool) (position->piece[toSquare] != NO_PIECE);
      const bool pieceCapture = (bool)
         (capture && pieceType(position->piece[toSquare]) != PAWN);
      const bool hashMoveNode = movesAreEqual(currentMove, hashmove);

      if (excludeMove != NO_MOVE && movesAreEqual(currentMove, excludeMove))
      {
         assert(excludeMove != NULLMOVE);

         continue;              /* exclude excludeMove */
      }

      variation->plyInfo[ply].indexCurrentMove =
         moveGainIndex(currentMove, position);
      variation->plyInfo[ply].quietMove = quietMove;

      assert(moveIsPseudoLegal(position, currentMove));
      assert(hashmove == NO_MOVE || numMovesPlayed > 0 ||
             movesAreEqual(currentMove, hashmove));

      /* Optimistic futility cuts */
      if (pvNode == FALSE && inCheck == FALSE &&
          quietMove != FALSE && dangerousPawns == FALSE &&
          numMovesPlayed >= 3 && excludeMove != NULLMOVE &&
          !isPassedPawnMove(getFromSquare(currentMove), toSquare, position) &&
          !moveIsCastling(currentMove, position) &&
          best > VALUE_ALMOST_MATED && cutsAreAllowed)
      {
         bool moveIsRelevant = FALSE;
         const int mcLimitBase =
            (32 * movelist.numberOfMoves * restDepth) / 1024;

         if (numMovesPlayed > 2 +
             (reduction ? (150 * mcLimitBase) / 256 : mcLimitBase))
         {
            if (simpleMoveIsCheck(position, currentMove))
            {
               moveIsRelevant = TRUE;
            }
            else
            {
               if (threatMove != NO_MOVE &&
                   moveIsDefence(currentMove, threatMove, position))
               {
                  moveIsRelevant = TRUE;
               }
               else
               {
                  variation->nodes++;
                  continue;
               }
            }
         }

         if (moveIsRelevant == FALSE)
         {
            const int futLimit =
               getRefinedStaticValue(variation, ply) +
               variation->positionalGain
               [moveGainIndex(currentMove, position)] +
               (263 * restDepth * restDepth) / 256 - (reduction ? 19 : 12);

            if (futLimit < beta &&
                simpleMoveIsCheck(position, currentMove) == FALSE)
            {
               if (futLimit > best)
               {
                  best = futLimit;
               }

               variation->nodes++;
               continue;
            }
         }
      }

      /* Execute the current move and check if it is legal. */
      /* -------------------------------------------------- */
      if (makeMoveFast(variation, currentMove) != 0 ||
          passiveKingIsSafe(&variation->singlePosition) == FALSE)
      {
         unmakeLastMove(variation);

         continue;
      }

      /* Check the conditions for search extensions and finally */
      /* calculate the rest depth for the next ply.             */
      /* ------------------------------------------------------ */
      variation->plyInfo[ply].currentMoveIsCheck = check =
         activeKingIsSafe(&variation->singlePosition) == FALSE;

      if (inCheck && movelist.numberOfMoves <= 2)
      {
         if (movelist.numberOfMoves <= 1)
         {
            extension = DEPTH_RESOLUTION;
         }
         else if (pvNode)
         {
            extension = DEPTH_RESOLUTION / 2;
         }
      }

      if (check)
      {
         extension = (pvNode ? DEPTH_RESOLUTION : DEPTH_RESOLUTION / 2);
      }
      else if (pvNode)
      {
         const Color passiveColor = opponent(position->activeColor);

         if (pieceType(position->piece[toSquare]) == PAWN &&
             pawnIsPassed(position, toSquare, passiveColor))
         {
            extension = DEPTH_RESOLUTION;
         }
         else if (pieceCapture &&
                  numberOfNonPawnPieces(position, WHITE) <= 2 &&
                  numberOfNonPawnPieces(position, WHITE) ==
                  numberOfNonPawnPieces(position, BLACK))
         {
            extension = DEPTH_RESOLUTION;
         }
      }

      if (singularExtensionNode != FALSE &&
          extension < DEPTH_RESOLUTION && hashMoveNode)
      {
         const int limitValue = hashEntryValue - restDepth;

         assert(excludeMove == NO_MOVE);

         if (limitValue > VALUE_ALMOST_MATED &&
             limitValue <= -VALUE_ALMOST_MATED)
         {
            int excludeValue;
            PlyInfo pi = variation->plyInfo[ply];

            unmakeLastMove(variation);
            excludeValue =
               searchBest(variation, limitValue - 1, limitValue, ply,
                          restDepth / 2, FALSE, FALSE, &bestReply, hashmove);
            makeMoveFast(variation, currentMove);
            variation->plyInfo[ply] = pi;

            if (excludeValue < limitValue)
            {
               extension = DEPTH_RESOLUTION;
            }
         }
      }

      newDepth = restDepth - DEPTH_RESOLUTION +
         (excludeMove != NULLMOVE || check ? extension : 0);

      /* History pruning */
      /* --------------- */
      if (inCheck == FALSE && extension == 0 &&
          restDepth >= 3 * DEPTH_RESOLUTION && quietMove != FALSE &&
          stage != MGS_GOOD_CAPTURES_AND_PROMOTIONS &&
          stage != MGS_KILLER_MOVES && excludeMove != NULLMOVE &&
          isPassedPawnMove(toSquare, toSquare, position) == FALSE)
      {
         const int moveIndex = min(63, numMovesPlayed);
         const int depthIndex = min(63, rdBasic);

         reduction =
            (pvNode ? quietPvMoveReduction[depthIndex][moveIndex] :
             quietMoveReduction[depthIndex][moveIndex]);
      }

      /* Recursive search */
      /* ---------------- */
      if (pvNode && numMovesPlayed == 0)
      {
         value = -searchBest(variation, -beta, -alpha, ply + 1,
                             newDepth, pvNode, FALSE, &bestReply, NO_MOVE);
      }
      else
      {
         bool fullDepthSearch = TRUE;

         if (reduction > 0)
         {
            value = -searchBest(variation, -alpha - 1, -alpha, ply + 1,
                                newDepth - reduction, FALSE,
                                TRUE, &bestReply, NO_MOVE);

            fullDepthSearch = (bool) (value > alpha);
         }

         if (fullDepthSearch)
         {
            value = -searchBest(variation, -alpha - 1, -alpha, ply + 1,
                                newDepth, FALSE, FALSE, &bestReply, NO_MOVE);

            if (pvNode && value > alpha && value < beta)
            {                   /* 2% */
               value = -searchBest(variation, -beta, -alpha, ply + 1,
                                   newDepth, TRUE, FALSE, &bestReply,
                                   NO_MOVE);
            }
         }
      }

      assert(value >= VALUE_MATED && value <= -VALUE_MATED);

      unmakeLastMove(variation);
      numMovesPlayed++;

      if (quietMove)
      {
         if (value <= best)
         {
            variation->historyValue[historyIdx] = (UINT16)
               (variation->historyValue[historyIdx] -
                (variation->historyValue[historyIdx] * restDepth) / 256);
            assert(variation->historyValue[historyIdx] <= HISTORY_MAX);
         }
      }

      if (variation->searchStatus != SEARCH_STATUS_RUNNING &&
          variation->nominalDepth > 1)
      {
         return 0;
      }

      /* Calculate the parameters controlling the search tree. */
      /* ----------------------------------------------------- */
      if (value > best)
      {                         /* 32% */
         best = value;

         if (best > alpha)
         {                      /* 63% */
            alpha = best;
            *bestMove = currentMove;

            if (best >= beta)
            {                   /* 99% */
               break;
            }
         }
      }
   }

   /* No legal move was found. Check if it's mate or stalemate. */
   /* --------------------------------------------------------- */
   if (best == VALUE_MATED)
   {
      if (excludeMove != NO_MOVE && excludeMove != NULLMOVE)
      {
         return beta - 1;
      }

      if (inCheck)
      {
         /* mate */

         best = VALUE_MATED + ply;
      }
      else
      {
         /* stalemate */

         best = variation->drawScore[position->activeColor];
      }
   }

   /* Calculate the parameters controlling the move ordering. */
   /* ------------------------------------------------------- */
   if (*bestMove != NO_MOVE && moveIsQuiet(*bestMove, position) &&
       (excludeMove == NO_MOVE || excludeMove == NULLMOVE))
   {
      Move killerMove = *bestMove;
      const Piece movingPiece = position->piece[getFromSquare(killerMove)];
      const int index = historyIndex(*bestMove, position);

      variation->historyValue[index] = (UINT16)
         (variation->historyValue[index] +
          ((HISTORY_MAX - variation->historyValue[index]) * restDepth) / 256);
      assert(variation->historyValue[index] <= HISTORY_MAX);

      setMoveValue(&killerMove, movingPiece);
      registerKillerMove(&variation->plyInfo[ply], killerMove);
   }

   /* Store the value in the transposition table. */
   /* ------------------------------------------- */
   if (variation->searchStatus == SEARCH_STATUS_RUNNING)
   {
      if (best > oldAlpha)
      {
         hashentryFlag =
            (best >= beta ? HASHVALUE_LOWER_LIMIT : HASHVALUE_EXACT);
      }
      else
      {
         hashentryFlag = HASHVALUE_UPPER_LIMIT;
      }

      setDatedEntry(variation->hashtable, hashKey,
                    calcHashtableValue(best, ply),
                    (UINT8) (restDepth + HASH_DEPTH_OFFSET),
                    packedMove(*bestMove), hashentryFlag,
                    (INT16) getStaticValue(variation, ply),
                    (INT16) getStaticFutilityMargin(variation, ply));
   }

   return best;
}

static void copyPvFromHashtable(Variation * variation, const int pvIndex,
                                PrincipalVariation * pv,
                                const Move bestBaseMove)
{
   int tableIndex = 0;
   Move bestMove = NO_MOVE;

   if (pvIndex == 0)
   {
      bestMove = bestBaseMove;
   }
   else
   {
      Hashentry *bestTableHit = 0;

      do
      {
         Hashentry *tableHit = getIndexedDatedEntry(variation->hashtable,
                                                    variation->singlePosition.
                                                    hashValue, &tableIndex);
         if (tableHit != NULL)
         {
            Move currentMove = (Move) getHashentryMove(tableHit);

            if (moveIsLegal(&variation->singlePosition, currentMove))
            {
               bestTableHit = getSuperiorHashEntry(bestTableHit, tableHit);
            }
         }
      }
      while (tableIndex < 4);

      if (bestTableHit != 0)
      {
         bestMove = (Move) getHashentryMove(bestTableHit);
      }
   }

   if (bestMove != NO_MOVE && pvIndex < MAX_DEPTH)
   {
      pv->move[pvIndex] = (UINT16) bestMove;
      pv->move[pvIndex + 1] = (UINT16) NO_MOVE;
      pv->length = pvIndex + 1;
      makeMove(variation, bestMove);
      copyPvFromHashtable(variation, pvIndex + 1, pv, bestBaseMove);
      unmakeLastMove(variation);
   }
   else
   {
      pv->move[pvIndex] = (UINT16) NO_MOVE;
      pv->length = pvIndex;
   }
}

static void copyPvToHashtable(Variation * variation, const int pvIndex,
                              const int pvLength)
{
   Move move = (Move) variation->pv.move[pvIndex];

   if (pvIndex < pvLength && moveIsLegal(&variation->singlePosition, move))
   {
      UINT8 importance = (UINT8) HASH_DEPTH_OFFSET;
      bool entryExists = FALSE;
      int tableIndex = 0;
      Move bestMove = NO_MOVE;
      Hashentry *bestTableHit = 0;

      do
      {
         Hashentry *tableHit = getIndexedDatedEntry(variation->hashtable,
                                                    variation->singlePosition.
                                                    hashValue, &tableIndex);

         if (tableHit != NULL)
         {
            bestTableHit = getSuperiorHashEntry(bestTableHit, tableHit);
         }
      }
      while (tableIndex < 4);

      if (bestTableHit != 0)
      {
         bestMove = (Move) getHashentryMove(bestTableHit);
         importance =
            max(importance, getHashentryImportance(bestTableHit) + 1);

         if (bestMove != NO_MOVE && movesAreEqual(bestMove, move))
         {
            entryExists = TRUE;
         }
      }

      if (entryExists == FALSE)
      {
         UINT8 hashentryFlag = HASHVALUE_LOWER_LIMIT;

         /* Store the move in the transposition table. */
         /* ------------------------------------------- */
         initPlyInfo(&variation->plyInfo[0]);

         setDatedEntry(variation->hashtable,
                       variation->singlePosition.hashValue, VALUE_MATED,
                       importance, packedMove(move),
                       hashentryFlag, (INT16) getStaticValue(variation, 0),
                       (INT16) getStaticFutilityMargin(variation, 0));
      }

      makeMove(variation, move);
      copyPvToHashtable(variation, pvIndex + 1, pvLength);
      unmakeLastMove(variation);
   }
}

static void registerNewBestMove(Variation * variation, Move * move,
                                const int value)
{
   if (variation->searchStatus == SEARCH_STATUS_RUNNING)
   {
      setMoveValue(move, value);
      variation->pv.score = value;
      copyPvFromHashtable(variation, 0, &variation->pv, *move);
      variation->bestBaseMove = *move;

      if (variation->nominalDepth > 4 &&
          variation->numberOfCurrentBaseMove > 1)
      {
         variation->bestMoveChange = TRUE;
         variation->bestMoveChangeCount[variation->nominalDepth]++;
         variation->easyMove = FALSE;
      }

      if (variation->handleSearchEvent != 0)
      {
         variation->handleSearchEvent(SEARCHEVENT_NEW_PV, variation);
      }
   }
}

static int getBaseMoveValue(Variation * variation, const Move move,
                            const int alpha, const int beta,
                            const bool fullWindow)
{
   int depth = DEPTH_RESOLUTION * variation->nominalDepth;
   int value;
   Move bestReply;

   assert(alpha >= VALUE_MATED);
   assert(alpha <= -VALUE_MATED);
   assert(beta >= VALUE_MATED);
   assert(beta <= -VALUE_MATED);
   assert(alpha < beta);

   makeMoveFast(variation, move);

   if (variation->handleSearchEvent != 0)
   {
      variation->handleSearchEvent(SEARCHEVENT_NEW_BASEMOVE, variation);
   }

   if (activeKingIsSafe(&variation->singlePosition) == FALSE)
   {
      variation->plyInfo[0].currentMoveIsCheck = TRUE;
      depth += DEPTH_RESOLUTION;
   }
   else
   {
      variation->plyInfo[0].currentMoveIsCheck = FALSE;
   }

   if (fullWindow)
   {
      value = -searchBest(variation, -beta, -alpha, 1,
                          depth - DEPTH_RESOLUTION, TRUE, FALSE, &bestReply,
                          NO_MOVE);
   }
   else
   {
      value = -searchBest(variation, -alpha - 1, -alpha, 1,
                          depth - DEPTH_RESOLUTION, FALSE, FALSE,
                          &bestReply, NO_MOVE);

      if (value > alpha)
      {
         value = -searchBest(variation, -beta, -alpha, 1,
                             depth - DEPTH_RESOLUTION, TRUE,
                             FALSE, &bestReply, NO_MOVE);
      }
   }

   unmakeLastMove(variation);

   return value;
}

static void initializePv(PrincipalVariation * pv)
{
   int i;

   pv->length = 0;

   for (i = 0; i < MAX_DEPTH_ARRAY_SIZE; i++)
   {
      pv->move[i] = (UINT16) NO_MOVE;
   }
}

static void exploreBaseMoves(Variation * variation, Movelist * basemoves,
                             const int aspirationWindow)
{
   const int previousBest = variation->previousBest;
   int ply = 0;
   Position *position = &variation->singlePosition;
   const bool fullWindow = (bool) (variation->nominalDepth <= 1);
   int window = aspirationWindow, best;
   bool exactValueFound = FALSE;
   int staticValue, futilityMargin;

   variation->failingHighOrLow = FALSE;
   variation->selDepth = variation->nominalDepth;
   initPlyInfo(&variation->plyInfo[ply]);
   staticValue = getStaticValue(variation, ply);
   futilityMargin = getStaticFutilityMargin(variation, ply);

   do
   {
      const int alpha =
         (variation->nominalDepth <= 5 ? VALUE_MATED :
          max(VALUE_MATED, previousBest - window));
      const int beta =
         (variation->nominalDepth <= 5 ? -VALUE_MATED :
          min(-VALUE_MATED, previousBest + window));

      initializeMoveValues(basemoves);
      best = VALUE_MATED;

      for (variation->numberOfCurrentBaseMove = 1;
           variation->numberOfCurrentBaseMove <= basemoves->numberOfMoves;
           variation->numberOfCurrentBaseMove++)
      {
         int value;
         const int icm = variation->numberOfCurrentBaseMove - 1;
         const int searchAlpha = max(alpha, best);
         const bool pvNode = (bool) (icm == 0 || fullWindow);

         variation->currentBaseMove = basemoves->moves[icm];
         variation->plyInfo[ply].indexCurrentMove =
            moveGainIndex(variation->currentBaseMove, position);

         value = getBaseMoveValue(variation, basemoves->moves[icm],
                                  searchAlpha, beta, pvNode);

         if (variation->searchStatus != SEARCH_STATUS_RUNNING &&
             variation->nominalDepth > 1)
         {
            break;
         }

         if (icm == 0 && value <= searchAlpha)
         {
            /* Update search info */
            assert(movesAreEqual
                   (basemoves->moves[icm], variation->bestBaseMove));
            registerNewBestMove(variation, &basemoves->moves[icm], value);
         }

         if (fullWindow)
         {
            setMoveValue(&basemoves->moves[icm], value);
         }

         if (value > best)
         {
            best = value;

            if (value > alpha)
            {
               registerNewBestMove(variation, &basemoves->moves[icm], value);

               if (value >= beta)
               {
                  break;
               }
            }
         }
      }

      /* Store the value in the transposition table. */
      /* ------------------------------------------- */
      if (variation->searchStatus == SEARCH_STATUS_RUNNING)
      {
         UINT8 hashentryFlag;
         const int depth = DEPTH_RESOLUTION * variation->nominalDepth;
         const Move bestMove = variation->bestBaseMove;

         if (best > alpha)
         {
            hashentryFlag =
               (best >= beta ? HASHVALUE_LOWER_LIMIT : HASHVALUE_EXACT);
         }
         else
         {
            hashentryFlag = HASHVALUE_UPPER_LIMIT;
         }

         setDatedEntry(variation->hashtable, position->hashValue,
                       calcHashtableValue(best, ply),
                       (UINT8) (depth + HASH_DEPTH_OFFSET),
                       packedMove(bestMove), hashentryFlag,
                       (INT16) staticValue, (INT16) futilityMargin);
      }

      if (best >= beta)
      {
         window = window + window / 2;
      }
      else if (best <= alpha && best > VALUE_MATED + 2)
      {
         window = window + window / 2;
         variation->failingHighOrLow = TRUE;
      }
      else
      {
         exactValueFound = TRUE;        /* exact value found */
      }

      sortMoves(basemoves);

      /*
         if (!movesAreEqual(basemoves->moves[0], variation->bestBaseMove))
         {
         dumpMove(variation->bestBaseMove);
         dumpMovelist(basemoves);
         }
       */

      assert(variation->nominalDepth <= 1 ||
             movesAreEqual(basemoves->moves[0], variation->bestBaseMove));

      copyPvToHashtable(variation, 0, variation->pv.length);
   }
   while (variation->searchStatus == SEARCH_STATUS_RUNNING &&
          exactValueFound == FALSE && abs(best) < -VALUE_ALMOST_MATED);

   if (variation->searchStatus == SEARCH_STATUS_RUNNING)
   {
      if (fullWindow &&
          (basemoves->numberOfMoves < 2 ||
           getMoveValue(basemoves->moves[0]) >
           getMoveValue(basemoves->moves[1]) + 150))
      {
         variation->easyMove = TRUE;
      }
   }
}

static void initializePawnHashtable(PawnHashInfo * pawnHashtable)
{
   int i;

   for (i = 0; i < PAWN_HASHTABLE_SIZE; i++)
   {
      pawnHashtable[i].hashValue = 0;
   }
}

static void initializeKingsafetyHashtable(KingSafetyHashInfo *
                                          kingsafetyHashtable)
{
   int i;

   for (i = 0; i < KINGSAFETY_HASHTABLE_SIZE; i++)
   {
      kingsafetyHashtable[i].hashValue = 0;
   }
}

static void updatePieceValues()
{
   maxPieceValue[WHITE_QUEEN] = maxPieceValue[BLACK_QUEEN] =
      max(VALUE_QUEEN_OPENING, VALUE_QUEEN_ENDGAME);
   maxPieceValue[WHITE_ROOK] = maxPieceValue[BLACK_ROOK] =
      max(VALUE_ROOK_OPENING, VALUE_ROOK_ENDGAME);
   maxPieceValue[WHITE_BISHOP] = maxPieceValue[BLACK_BISHOP] =
      max(VALUE_BISHOP_OPENING, VALUE_BISHOP_ENDGAME);
   maxPieceValue[WHITE_KNIGHT] = maxPieceValue[BLACK_KNIGHT] =
      max(VALUE_KNIGHT_OPENING, VALUE_KNIGHT_ENDGAME);
   maxPieceValue[WHITE_PAWN] = maxPieceValue[BLACK_PAWN] =
      max(PAWN_VALUE_OPENING, PAWN_VALUE_ENDGAME);
}

Move search(Variation * variation, Movelist * acceptableSolutions)
{
   Movelist movelist;
   int iteration;
   long timeTarget;
   int stableIterationCount = 0;
   int stableBestMoveCount = 0;
   Move bestMove = NO_MOVE;
   UINT64 nodeCount = 0;
   bool failLow = FALSE;
   int iv1 = 0, iv2 = 0, iv3 = 0;

   if (resetGlobalHashtable)
   {
      resetHashtable(variation->hashtable);
      initializePawnHashtable(variation->pawnHashtable);
      initializeKingsafetyHashtable(variation->kingsafetyHashtable);
      resetGlobalHashtable = FALSE;
   }

   resetHistoryValues(variation);
   resetGainValues(variation);

   variation->startTime = getTimestamp();
   variation->nodes = variation->nodesAtTimeCheck = 0;
   variation->startTimeProcess = getProcessTimestamp();
   variation->timestamp = variation->startTime + 1;
   variation->tbHits = 0;
   variation->numPvUpdates = 0;
   variation->easyMove = FALSE;
   variation->terminatePondering = FALSE;
   variation->previousBest = getStaticValue(variation, 0);
   variation->bestBaseMove = NO_MOVE;
   variation->failingHighOrLow = FALSE;
   movelist.positionalGain = &(variation->positionalGain[0]);
   getLegalMoves(variation, &movelist);
   initializePlyInfo(variation);
   initializePv(&(variation->pv));

#ifdef TRACE_EVAL
   getValue(&variation->singlePosition, VALUE_MATED, -VALUE_MATED,
            variation->pawnHashtable, variation->kingsafetyHashtable);
#endif

#ifdef USE_BOOK
   if (globalBook.indexFile != NULL && globalBook.moveFile != NULL &&
       &variation->singlePosition->moveNumber <= 12)
   {
      Move bookMove = getBookmove(&globalBook,
                                  &variation->singlePosition->hashValue,
                                  &movelist);

      if (bookMove != NO_MOVE)
      {
         variation->bestBaseMove = bookMove;
         variation->searchStatus = SEARCH_STATUS_TERMINATE;
         variation->finishTime = getTimestamp();

         if (variation->handleSearchEvent != 0)
         {
            variation->handleSearchEvent(SEARCHEVENT_SEARCH_FINISHED,
                                         variation);
         }

         variation->nodes = 0;

         return variation->bestBaseMove;
      }
   }
#endif

   variation->numberOfBaseMoves = movelist.numberOfMoves;
   setMoveValue(&variation->bestBaseMove, VALUE_MATED);

   for (iteration = 1; iteration <= MAX_DEPTH; iteration++)
   {
      long calculationTime;
      int iterationValue, referenceValue, aspirationWindow;

      variation->bestMoveChange = FALSE;
      variation->nominalDepth = iteration;

      aspirationWindow =
         min(12, max(8, (abs(iv1 - iv2) + abs(iv2 - iv3)) / 2));
      exploreBaseMoves(variation, &movelist, aspirationWindow);
      calculationTime =
         (unsigned long) (getTimestamp() - variation->startTime);

      if (movesAreEqual(variation->bestBaseMove, bestMove))
      {
         stableBestMoveCount++;
      }
      else
      {
         stableBestMoveCount = 0;
      }

      bestMove = variation->bestBaseMove;
      iv3 = iv2;
      iv2 = iv1;
      iv1 = iterationValue = getMoveValue(variation->bestBaseMove);

      referenceValue = max(variation->previousBest, variation->expectedScore);
      failLow = (bool) (iterationValue < referenceValue);
      variation->previousBest = iterationValue;

      assert(calculationTime >= 0);

      if (acceptableSolutions != 0 &&
          listContainsMove(acceptableSolutions, variation->bestBaseMove))
      {
         stableIterationCount++;

         if (stableIterationCount == 1)
         {
            nodeCount = variation->nodes;
         }
      }
      else
      {
         stableIterationCount = 0;
         nodeCount = variation->nodes;
      }

      /* Check for a fail low. */
      /* --------------------- */

      if (iterationValue < referenceValue &&
          calculationTime >= variation->timeTarget / 32)
      {
         variation->easyMove = FALSE;
      }

      if (variation->numberOfBaseMoves == 1)
      {
         timeTarget = (1 * variation->timeTarget) / 12;
      }
      else if (variation->bestMoveChangeCount[iteration] > 0 ||
               variation->bestMoveChangeCount[max(0, iteration - 1)] > 0)
      {
         const int timeWeight = 12 +
            6 * variation->bestMoveChangeCount[iteration] +
            4 * variation->bestMoveChangeCount[max(0, iteration - 1)];

         timeTarget = (timeWeight * variation->timeTarget) / 12;
      }
      else if (variation->easyMove && iterationValue >= referenceValue)
      {
         timeTarget = (3 * variation->timeTarget) / 12;
      }
      else
      {
         const int weight = (iterationValue < referenceValue ? 8 : 6);
         const int bonusWeight =
            (iterationValue < variation->previousBest - 25 ? 10 : 0);

         timeTarget = ((weight + bonusWeight) * variation->timeTarget / 12);
      }

      if (variation->searchStatus == SEARCH_STATUS_RUNNING &&
          iteration > 8 && variation->timeLimit != 0 &&
          calculationTime >= timeTarget)
      {
#ifdef DEBUG_THREAD_COORDINATION
         logDebug
            ("Time target reached (%lu/%lu ms, %lu%%)).\n",
             calculationTime, variation->timeTarget,
             (calculationTime * 100) / variation->timeTarget);
#endif

         if (variation->ponderMode)
         {
            variation->terminatePondering = TRUE;

#ifdef DEBUG_THREAD_COORDINATION
            logDebug("Setting ponder termination flag.\n");
#endif
         }
         else
         {
            variation->searchStatus = SEARCH_STATUS_TERMINATE;

#ifdef DEBUG_THREAD_COORDINATION
            logDebug("Terminating search.\n");
#endif
         }
      }

      if (variation->searchStatus == SEARCH_STATUS_RUNNING &&
          (getMoveValue(variation->bestBaseMove) <= VALUE_MATED + iteration ||
           getMoveValue(variation->bestBaseMove) >= -VALUE_MATED - iteration))
      {
#ifdef DEBUG_THREAD_COORDINATION
         logDebug("Best value out of bounds (%d). Terminating search.\n",
                  getMoveValue(variation->bestBaseMove));
#endif
         variation->searchStatus = SEARCH_STATUS_TERMINATE;
      }

      if (variation->searchStatus == SEARCH_STATUS_RUNNING &&
          iteration == MAX_DEPTH)
      {
#ifdef DEBUG_THREAD_COORDINATION
         logDebug("Max depth reached. Terminating search.\n");
#endif
         variation->searchStatus = SEARCH_STATUS_TERMINATE;
      }

      if (variation->searchStatus != SEARCH_STATUS_RUNNING)
      {
#ifdef DEBUG_THREAD_COORDINATION
         logReport
            ("search status != SEARCH_STATUS_RUNNING -> exiting search.\n",
             getMoveValue(variation->bestBaseMove));
#endif

         break;
      }

      if (acceptableSolutions != 0 && stableIterationCount >= 1 &&
          (getMoveValue(variation->bestBaseMove) > 20000 ||
           (stableIterationCount >= 2 &&
            (getMoveValue(variation->bestBaseMove) >= 25 ||
             (getTimestamp() - variation->startTime) >= 3000))))
      {
#ifdef DEBUG_THREAD_COORDINATION
         logDebug("Solution found (value=%d). Terminating search.\n",
                  getMoveValue(variation->bestBaseMove));
#endif
         variation->searchStatus = SEARCH_STATUS_TERMINATE;

         break;
      }
   }

   variation->terminatePondering = TRUE;
   variation->searchStatus = SEARCH_STATUS_TERMINATE;
   incrementDate(variation->hashtable);
   variation->finishTime = getTimestamp();
   variation->finishTimeProcess = getProcessTimestamp();

   if (variation->handleSearchEvent != 0)
   {
      variation->handleSearchEvent(SEARCHEVENT_SEARCH_FINISHED, variation);
   }

   variation->nodes = nodeCount;

   if (statCount1 != 0 || statCount2 != 0)
   {
      logReport("statCount1=%lld statCount2=%lld (%lld%%) \n",
                statCount1, statCount2,
                (statCount2 * 100) / max(1, statCount1));
   }

   return variation->bestBaseMove;
}

/* #define DEBUG_FUT_VALUES */

static void initializeArrays(void)
{
   const double margin18 = 1250.0;
   const double base = pow(margin18, 1.0 / 18.0);
   int i, j;

   for (i = 0; i < 64; i++)
   {
      for (j = 0; j < 64; j++)
      {
         if (i == 0 || j == 0)
         {
            quietPvMoveReduction[i][j] = quietMoveReduction[i][j] = 0;
         }
         else
         {
            double pvReduction =
               0.5 + log((double) (i)) * log((double) (j)) / 5.88;
            double nonPvReduction =
               0.5 + log((double) (i)) * log((double) (j)) / 2.95;

            quietPvMoveReduction[i][j] =
               (int) (pvReduction >= 1.0 ?
                      floor(pvReduction * DEPTH_RESOLUTION) : 0);
            quietMoveReduction[i][j] =
               (int) (nonPvReduction >= 1.0 ?
                      floor(nonPvReduction * DEPTH_RESOLUTION) : 0);
         }
      }

      if (i > 0)
      {
         log2[i] = (int) floor(log(i) / log(2.0));
      }
      else
      {
         log2[i] = 0;
      }
   }

   for (i = 0; i < 32; i++)
   {
      quietMoveCountLimit[i] = 3 + ((i * i) / 8);

#ifdef DEBUG_FUT_VALUES
      logDebug("mcl[%d]=%d\n", i, quietMoveCountLimit[i]);
#endif
   }

   for (i = 0; i < 21; i++)
   {
      for (j = 0; j < 64; j++)
      {
         double margin = pow(base, (double) (i));

         /*futilityMargin[i][j] =
            (198 * (i * i)) / 600 + (159 * i) / 10 + 12 - 2 * j; */
         futilityMargin[i][j] = (int) margin + 10;

#ifdef DEBUG_FUT_VALUES
         if (j <= 2)
         {
            logDebug("fm[%d][%d]=%d\n", i, j, futilityMargin[i][j]);
         }
#endif
      }
   }

   hashFlagBonus[HASHVALUE_UPPER_LIMIT] = 1;
   hashFlagBonus[HASHVALUE_EXACT] = 3;
   hashFlagBonus[HASHVALUE_LOWER_LIMIT] = 2;
   hashFlagBonus[HASHVALUE_EVAL] = 0;

   updatePieceValues();

#ifdef DEBUG_FUT_VALUES
   getKeyStroke();
#endif
}

int initializeModuleSearch(void)
{
   initializeArrays();

   return 0;
}

int testModuleSearch(void)
{
   return 0;
}
