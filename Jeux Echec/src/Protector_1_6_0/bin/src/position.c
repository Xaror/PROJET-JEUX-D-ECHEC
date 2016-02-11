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
#include <string.h>
#include <assert.h>
#include <math.h>
#include "position.h"
#include "fen.h"
#include "io.h"
#include "keytable.h"
#include "hash.h"
#include "evaluation.h"

/* #define TRACE_POSITIONS 1 */

#ifdef TRACE_POSITIONS
const int POSITION_TRACE_START = 4711;
const int POSITION_TRACE_QUANTITY = 100;
#endif

int VALUE_QUEEN_OPENING = DEFAULTVALUE_QUEEN_OPENING;
int VALUE_QUEEN_ENDGAME = DEFAULTVALUE_QUEEN_ENDGAME;
int VALUE_ROOK_OPENING = DEFAULTVALUE_ROOK_OPENING;
int VALUE_ROOK_ENDGAME = DEFAULTVALUE_ROOK_ENDGAME;
int VALUE_BISHOP_OPENING = DEFAULTVALUE_BISHOP_OPENING;
int VALUE_BISHOP_ENDGAME = DEFAULTVALUE_BISHOP_ENDGAME;
int VALUE_KNIGHT_OPENING = DEFAULTVALUE_KNIGHT_OPENING;
int VALUE_KNIGHT_ENDGAME = DEFAULTVALUE_KNIGHT_ENDGAME;
int PAWN_VALUE_OPENING = PAWN_DEFAULTVALUE_OPENING;
int PAWN_VALUE_ENDGAME = PAWN_DEFAULTVALUE_ENDGAME;
int VALUE_BISHOP_PAIR_OPENING = DEFAULTVALUE_BISHOP_PAIR_OPENING;
int VALUE_BISHOP_PAIR_ENDGAME = DEFAULTVALUE_BISHOP_PAIR_ENDGAME;

int basicValue[16], simplePieceValue[16];
INT32 pieceSquareBonus[16][_64_];
BYTE remainingCastlings[_64_];
Square rookOrigin[_64_];
int pieceCountShift[16];
UINT64 pieceCountWeight[16];
UINT64 bishopPieceCountWeight[2][_64_];
UINT32 matSigOfPieceCount[0xFFff];
int krqIndexWhite[4096], bbpIndexWhite[4096];
int krqIndexBlack[4096], bbpIndexBlack[4096];

#ifndef INLINE_IN_HEADERS
#include "positionInline.h"
#endif

static UINT64 getHashValue(const Position * position)
{
   UINT64 hashValue = ULONG_ZERO;
   Square square;
   Piece piece;

   ITERATE(square)
   {
      piece = position->piece[square];

      if (piece != NO_PIECE)
      {
         hashValue ^= GENERATED_KEYTABLE[piece][square];
      }
   }

   if (position->activeColor == BLACK)
   {
      hashValue = ~hashValue;
   }

   hashValue ^= GENERATED_KEYTABLE[0][position->castlingRights];

   if (position->enPassantSquare != NO_SQUARE)
   {
      hashValue ^= GENERATED_KEYTABLE[0][position->enPassantSquare];
   }

   return hashValue;
}

static UINT64 getPawnHashValue(const Position * position)
{
   UINT64 hashValue = ULONG_ZERO;
   Square square;
   Piece piece;

   ITERATE(square)
   {
      piece = position->piece[square];

      if (pieceType(piece) == PAWN)
      {
         hashValue ^= GENERATED_KEYTABLE[piece][square];
      }
   }

   return hashValue;
}

void clearPosition(Position * position)
{
   Square square;

   ITERATE(square)
   {
      position->piece[square] = NO_PIECE;
   }

   position->activeColor = WHITE;
   position->castlingRights = 0;
   position->enPassantSquare = NO_SQUARE;
   position->moveNumber = 1;
   position->halfMoveClock = 0;
}

void initializePosition(Position * position)
{
   int i;
   Square square;

   position->allPieces = EMPTY_BITBOARD;
   position->piecesOfColor[WHITE] = EMPTY_BITBOARD;
   position->piecesOfColor[BLACK] = EMPTY_BITBOARD;
   position->balance = 0;
   position->pieceCount = 0;

   for (i = 0x00; i <= 0x0F; i++)
   {
      position->piecesOfType[i] = EMPTY_BITBOARD;
   }

   if ((position->castlingRights & WHITE_00) &&
       (position->piece[E1] != WHITE_KING ||
        position->piece[H1] != WHITE_ROOK))
   {
      position->castlingRights -= WHITE_00;
   }

   if ((position->castlingRights & WHITE_000) &&
       (position->piece[E1] != WHITE_KING ||
        position->piece[A1] != WHITE_ROOK))
   {
      position->castlingRights -= WHITE_000;
   }

   if ((position->castlingRights & BLACK_00) &&
       (position->piece[E8] != BLACK_KING ||
        position->piece[H8] != BLACK_ROOK))
   {
      position->castlingRights -= BLACK_00;
   }

   if ((position->castlingRights & BLACK_000) &&
       (position->piece[E8] != BLACK_KING ||
        position->piece[A8] != BLACK_ROOK))
   {
      position->castlingRights -= BLACK_000;
   }

   if (position->enPassantSquare != NO_SQUARE)
   {
      if (position->activeColor == WHITE)
      {
         if (rank(position->enPassantSquare) != RANK_6 ||
             position->piece[position->enPassantSquare - 8] != BLACK_PAWN)
         {
            position->enPassantSquare = NO_SQUARE;
         }
      }
      else
      {
         if (rank(position->enPassantSquare) != RANK_3 ||
             position->piece[position->enPassantSquare + 8] != WHITE_PAWN)
         {
            position->enPassantSquare = NO_SQUARE;
         }
      }
   }

   ITERATE(square)
   {
      Piece piece = position->piece[square];
      Color color = pieceColor(piece);

      if (piece != NO_PIECE)
      {
         setSquare(position->allPieces, square);
         setSquare(position->piecesOfColor[color], square);
         setSquare(position->piecesOfType[piece], square);

         if (pieceType(piece) == KING)
         {
            position->king[color] = square;
         }

         addBonusForColor(pieceSquareBonus[piece][square], position, color);

         if (pieceType(piece) == BISHOP)
         {
            position->pieceCount += bishopPieceCountWeight[color][square];
         }
         else
         {
            position->pieceCount += pieceCountWeight[piece];
         }
      }
   }

   position->numberOfPieces[WHITE] =
      getNumberOfSetSquares(position->piecesOfColor[WHITE]);
   position->numberOfPieces[BLACK] =
      getNumberOfSetSquares(position->piecesOfColor[BLACK]);
   position->numberOfPawns[WHITE] =
      getNumberOfSetSquares(position->piecesOfType[WHITE_PAWN]);
   position->numberOfPawns[BLACK] =
      getNumberOfSetSquares(position->piecesOfType[BLACK_PAWN]);

   position->hashValue = getHashValue(position);
   position->pawnHashValue = getPawnHashValue(position);
}

void flipPosition(Position * position)
{
   Square square;
   BYTE castlingRights = 0;

   position->activeColor = opponent(position->activeColor);

   if (position->castlingRights & WHITE_00)
   {
      castlingRights += BLACK_00;
   }

   if (position->castlingRights & WHITE_000)
   {
      castlingRights += BLACK_000;
   }

   if (position->castlingRights & BLACK_00)
   {
      castlingRights += WHITE_00;
   }

   if (position->castlingRights & BLACK_000)
   {
      castlingRights += WHITE_000;
   }

   position->castlingRights = castlingRights;

   if (position->enPassantSquare != NO_SQUARE)
   {
      position->enPassantSquare = getFlippedSquare(position->enPassantSquare);
   }

   for (square = A1; square <= H4; square++)
   {
      const Square flippedSquare = getFlippedSquare(square);
      const Piece piece = position->piece[square];

      position->piece[square] =
         (position->piece[flippedSquare] == NO_PIECE ? NO_PIECE :
          (Piece) (position->piece[flippedSquare] ^ BLACK));
      position->piece[flippedSquare] =
         (piece == NO_PIECE ? NO_PIECE : (Piece) (piece ^ BLACK));
   }
}

void resetHistoryValues(Variation * variation)
{
   int i;

   for (i = 0; i < HISTORY_SIZE; i++)
   {
      variation->historyValue[i] = 0;
   }
}

void resetGainValues(Variation * variation)
{
   int i;

   for (i = 0; i < HISTORY_SIZE; i++)
   {
      variation->positionalGain[i] = 0;
   }
}

void prepareSearch(Variation * variation)
{
   int i;

   variation->nodes = variation->nodesAtTimeCheck = 0;
   variation->nodesBetweenTimecheck = NODES_BETWEEN_TIMECHECK_DEFAULT;
   initializePosition(&variation->singlePosition);
   variation->hashtable = &globalHashtable;
   variation->drawScore[WHITE] = variation->drawScore[BLACK] = 0;

   for (i = 0; i < MAX_DEPTH_ARRAY_SIZE; i++)
   {
      PlyInfo *pi = &(variation->plyInfo[i]);

      pi->killerMove1 = pi->killerMove2 =
         pi->killerMove3 = pi->killerMove4 = NO_MOVE;
      variation->bestMoveChangeCount[i] = 0;
   }
}

void shrinkHistoryValues(Variation * variation)
{
   int i;

   for (i = 0; i < HISTORY_SIZE; i++)
   {
      variation->historyValue[i] = (variation->historyValue[i] + 1) / 2;
   }
}

void initializeVariation(Variation * variation, const char *fen)
{
   variation->ply = 0;
   readFen(fen, &variation->singlePosition);
   prepareSearch(variation);
   variation->startPosition = variation->singlePosition;
}

void setBasePosition(Variation * variation, const Position * position)
{
   variation->ply = 0;
   variation->singlePosition = *position;
   variation->startPosition = variation->singlePosition;
}

void setDrawScore(Variation * variation, int score, Color color)
{
   variation->drawScore[color] = score;
   variation->drawScore[opponent(color)] = -score;
}

Bitboard getInterestedPieces(const Position * position, const Square square,
                             const Color attackerColor)
{
   Bitboard king = getKingMoves(square);
   Bitboard dia = getMagicBishopMoves(square, position->allPieces);
   Bitboard ortho = getMagicRookMoves(square, position->allPieces);
   Bitboard knights = getKnightMoves(square);
   Bitboard pawns = getInterestedPawns(attackerColor, square,
                                       position->allPieces);

   king &= position->piecesOfType[KING | attackerColor];
   ortho &= (position->piecesOfType[QUEEN | attackerColor] |
             position->piecesOfType[ROOK | attackerColor]);
   dia &= (position->piecesOfType[QUEEN | attackerColor] |
           position->piecesOfType[BISHOP | attackerColor]);
   knights &= position->piecesOfType[KNIGHT | attackerColor];
   pawns &= position->piecesOfType[PAWN | attackerColor];

   return king | ortho | dia | knights | pawns;
}

int checkVariation(Variation * variation)
{
   if (checkConsistency(&variation->singlePosition) != 0)
   {
      logDebug("consistency check failed!\n");

      dumpVariation(variation);
   }

   return 0;
}

int makeMove(Variation * variation, const Move move)
{
   Position *position = &variation->singlePosition;
   PlyInfo *plyInfo = &variation->plyInfo[variation->ply++];
   const Square from = getFromSquare(move);
   const Square to = getToSquare(move);
   const Piece newPiece = getNewPiece(move);
   const Piece movingPiece = position->piece[from];
   const Piece capturedPiece = position->piece[to];
   const Color activeColor = position->activeColor;
   const Color passiveColor = opponent(activeColor);
   const Bitboard minTo = minValue[to];
   const Bitboard maxFrom = maxValue[from];
   int result = 0;

   assert(to == from || pieceType(capturedPiece) != KING);

   variation->positionHistory[POSITION_HISTORY_OFFSET - 1 + variation->ply] =
      plyInfo->hashValue = position->hashValue;
   plyInfo->currentMove = move;
   plyInfo->balance = position->balance;
   position->hashValue = ~position->hashValue;

   if (position->enPassantSquare != NO_SQUARE)
   {
      position->hashValue ^= GENERATED_KEYTABLE[0][position->enPassantSquare];
   }

   plyInfo->pawnHashValue = position->pawnHashValue;
   plyInfo->enPassantSquare = position->enPassantSquare;
   position->enPassantSquare = NO_SQUARE;
   position->activeColor = passiveColor;

   if (to == from)
   {
      assert(checkVariation(variation) == 0);

      return result;            /* Nullmove */
   }

   plyInfo->captured = capturedPiece;
   plyInfo->kingSquare = position->king[activeColor];
   plyInfo->castlingRights = position->castlingRights;
   plyInfo->halfMoveClock = position->halfMoveClock;
   plyInfo->allPieces = position->allPieces;
   plyInfo->whitePieces = position->piecesOfColor[WHITE];
   plyInfo->blackPieces = position->piecesOfColor[BLACK];
   plyInfo->pieceCount = position->pieceCount;
   variation->plyInfo[variation->ply].staticValueAvailable = FALSE;
   variation->plyInfo[variation->ply].gainsUpdated = FALSE;
   position->piecesOfColor[activeColor] &= maxFrom;
   position->piecesOfColor[activeColor] |= minTo;
   position->piecesOfType[movingPiece] &= maxFrom;
   position->hashValue ^=
      GENERATED_KEYTABLE[movingPiece][from] ^
      GENERATED_KEYTABLE[movingPiece][to];
   position->castlingRights &=
      remainingCastlings[to] & remainingCastlings[from];

   if (position->castlingRights != plyInfo->castlingRights)
   {
      position->hashValue ^=
         GENERATED_KEYTABLE[0][plyInfo->castlingRights] ^
         GENERATED_KEYTABLE[0][position->castlingRights];
   }

   position->halfMoveClock++;
   position->piece[to] = movingPiece;
   position->piece[from] = NO_PIECE;
   addBonusForColor(pieceSquareBonus[movingPiece][to] -
                    pieceSquareBonus[movingPiece][from], position,
                    activeColor);

   if (capturedPiece != NO_PIECE)
   {
      position->halfMoveClock = 0;
      position->piecesOfColor[passiveColor] &= ~minTo;
      position->piecesOfType[capturedPiece] &= ~minTo;
      position->numberOfPieces[passiveColor]--;

      if (pieceType(capturedPiece) == PAWN)
      {
         position->numberOfPawns[passiveColor]--;
         position->pawnHashValue ^= GENERATED_KEYTABLE[capturedPiece][to];
      }
      else
      {
         position->pieceCount -= (capturedPiece == (BISHOP | passiveColor) ?
                                  bishopPieceCountWeight[passiveColor][to] :
                                  pieceCountWeight[capturedPiece]);
      }

      position->hashValue ^= GENERATED_KEYTABLE[capturedPiece][to];
      addBonusForColor(pieceSquareBonus[capturedPiece][to], position,
                       activeColor);
   }

   if (pieceType(movingPiece) == PAWN)
   {
      position->halfMoveClock = 0;
      position->pawnHashValue ^=
         GENERATED_KEYTABLE[movingPiece][from] ^
         GENERATED_KEYTABLE[movingPiece][to];

      if (distance(from, to) == 2)
      {
         position->enPassantSquare = (Square) ((from + to) >> 1);
         position->hashValue ^=
            GENERATED_KEYTABLE[0][position->enPassantSquare];
      }
      else if (to == plyInfo->enPassantSquare)
      {
         const Square captureSquare =
            (Square) (to + (rank(from) - rank(to)) * 8);
         const Piece capturedPawn = position->piece[captureSquare];

         clearSquare(position->piecesOfColor[passiveColor], captureSquare);
         clearSquare(position->piecesOfType[capturedPawn], captureSquare);
         position->hashValue ^=
            GENERATED_KEYTABLE[capturedPawn][captureSquare];
         position->pawnHashValue ^=
            GENERATED_KEYTABLE[capturedPawn][captureSquare];
         addBonusForColor(pieceSquareBonus[capturedPawn][captureSquare],
                          position, activeColor);

         plyInfo->restoreSquare1 = captureSquare;
         plyInfo->restorePiece1 = capturedPawn;
         position->piece[captureSquare] = NO_PIECE;
         position->numberOfPieces[passiveColor]--;
         position->numberOfPawns[passiveColor]--;
      }
      else if (newPiece != NO_PIECE)
      {
         const Piece effectiveNewPiece = (Piece) (newPiece | activeColor);

         plyInfo->restoreSquare1 = from;
         plyInfo->restorePiece1 = movingPiece;
         position->piece[to] = effectiveNewPiece;
         position->numberOfPawns[activeColor]--;
         position->pieceCount +=
            (newPiece == (Piece) BISHOP ?
             bishopPieceCountWeight[activeColor][to] :
             pieceCountWeight[effectiveNewPiece]);
         position->hashValue ^=
            GENERATED_KEYTABLE[movingPiece][to] ^
            GENERATED_KEYTABLE[effectiveNewPiece][to];
         position->pawnHashValue ^= GENERATED_KEYTABLE[movingPiece][to];
         addBonusForColor(pieceSquareBonus[effectiveNewPiece][to] -
                          pieceSquareBonus[movingPiece][to],
                          position, activeColor);
         setSquare(position->piecesOfType[position->piece[to]], to);
      }
   }

   else if (pieceType(movingPiece) == KING)
   {
      position->king[activeColor] = to;

      if (distance(from, to) == 2)
      {
         const Square rookFrom = rookOrigin[to];
         const Square rookTo = (Square) ((from + to) >> 1);
         const Piece movingRook = position->piece[rookFrom];

         plyInfo->restoreSquare1 = rookFrom;
         plyInfo->restorePiece1 = movingRook;
         plyInfo->restoreSquare2 = rookTo;
         plyInfo->restorePiece2 = position->piece[rookTo];
         position->piece[rookFrom] = NO_PIECE;
         position->piece[rookTo] = movingRook;
         position->halfMoveClock = 0;

         setSquare(position->piecesOfColor[activeColor], rookTo);
         clearSquare(position->piecesOfColor[activeColor], rookFrom);
         setSquare(position->piecesOfType[movingRook], rookTo);
         clearSquare(position->piecesOfType[movingRook], rookFrom);
         position->hashValue ^=
            GENERATED_KEYTABLE[movingRook][rookFrom] ^
            GENERATED_KEYTABLE[movingRook][rookTo];
         addBonusForColor(pieceSquareBonus[movingRook][rookTo] -
                          pieceSquareBonus[movingRook][rookFrom],
                          position, activeColor);

         if (getDirectAttackers(position, from, passiveColor,
                                position->allPieces) != EMPTY_BITBOARD ||
             getDirectAttackers(position, rookTo, passiveColor,
                                position->allPieces) != EMPTY_BITBOARD)
         {
            result = 1;         /* castling move was not legal */
         }
      }
   }

   setSquare(position->piecesOfType[position->piece[to]], to);
   position->allPieces =
      position->piecesOfColor[WHITE] | position->piecesOfColor[BLACK];

   assert(checkVariation(variation) == 0);

   return result;
}

#define PERSPECTIVE_WHITE
#include "positionc.c"
#undef PERSPECTIVE_WHITE
#include "positionc.c"

int makeMoveFast(Variation * variation, const Move move)
{
   if (variation->singlePosition.activeColor == WHITE)
   {
      return makeWhiteMove(variation, move);
   }
   else
   {
      return makeBlackMove(variation, move);
   }
}

void unmakeLastMove(Variation * variation)
{
   Position *position = &variation->singlePosition;
   const PlyInfo *plyInfo = &variation->plyInfo[--variation->ply];
   const Move move = plyInfo->currentMove;
   const Square from = getFromSquare(move);
   const Square to = getToSquare(move);
   const Piece newPiece = getNewPiece(move);
   const Color activeColor = position->activeColor =
      opponent(position->activeColor);

   position->enPassantSquare = plyInfo->enPassantSquare;
   position->hashValue = plyInfo->hashValue;

   if (from == to)
   {
      assert(checkVariation(variation) == 0);

      return;                   /* Nullmove */
   }

   position->pawnHashValue = plyInfo->pawnHashValue;
   clearSquare(position->piecesOfType[position->piece[to]], to);
   position->king[activeColor] = plyInfo->kingSquare;
   position->piece[from] = position->piece[to];
   position->piece[to] = plyInfo->captured;
   position->pieceCount = plyInfo->pieceCount;

   if (newPiece != NO_PIECE)
   {
      position->numberOfPawns[activeColor]++;
      position->piece[plyInfo->restoreSquare1] = plyInfo->restorePiece1;
   }

   setSquare(position->piecesOfType[position->piece[from]], from);
   position->halfMoveClock = plyInfo->halfMoveClock;
   position->castlingRights = plyInfo->castlingRights;
   position->piecesOfColor[WHITE] = plyInfo->whitePieces;
   position->piecesOfColor[BLACK] = plyInfo->blackPieces;
   position->allPieces = plyInfo->allPieces;
   position->balance = plyInfo->balance;

   if (plyInfo->captured != NO_PIECE)
   {
      const Color passiveColor = opponent(activeColor);

      setSquare(position->piecesOfType[plyInfo->captured], to);
      position->numberOfPieces[passiveColor]++;

      if (pieceType(plyInfo->captured) == PAWN)
      {
         position->numberOfPawns[passiveColor]++;
      }
   }
   else if (to == plyInfo->enPassantSquare &&
            pieceType(position->piece[from]) == PAWN)
   {
      const Color passiveColor = opponent(activeColor);

      position->piece[plyInfo->restoreSquare1] = plyInfo->restorePiece1;
      setSquare(position->piecesOfType[plyInfo->restorePiece1],
                plyInfo->restoreSquare1);
      position->numberOfPieces[passiveColor]++;
      position->numberOfPawns[passiveColor]++;
   }
   else if (distance(from, to) == 2 &&
            pieceType(position->piece[from]) == KING)
   {
      position->piece[plyInfo->restoreSquare1] = plyInfo->restorePiece1;
      position->piece[plyInfo->restoreSquare2] = plyInfo->restorePiece2;
      setSquare(position->piecesOfType[plyInfo->restorePiece1],
                plyInfo->restoreSquare1);
      clearSquare(position->piecesOfType[plyInfo->restorePiece1],
                  plyInfo->restoreSquare2);
   }

   assert(checkVariation(variation) == 0);
}

bool moveIsCheck(const Move move, const Position * position)
{
   const Square kingSquare = position->king[opponent(position->activeColor)];
   const Square from = getFromSquare(move);
   const Square to = getToSquare(move);
   const Piece piece = position->piece[from];

   if (testSquare(generalMoves[QUEEN][kingSquare], from) &&
       (position->allPieces & squaresBetween[kingSquare][from]) ==
       EMPTY_BITBOARD &&
       testSquare(squaresBetween[kingSquare][from], to) == FALSE &&
       testSquare(squaresBehind[from][kingSquare], to) == FALSE)
   {
      /* Detect undiscovered check: */

      const Color color = position->activeColor;
      Bitboard batteryPieces;
      Square square;

      if (rank(from) == rank(kingSquare) || file(from) == file(kingSquare))
      {
         batteryPieces = squaresBehind[from][kingSquare] &
            (position->piecesOfType[ROOK | color] |
             position->piecesOfType[QUEEN | color]);
      }
      else
      {
         batteryPieces = squaresBehind[from][kingSquare] &
            (position->piecesOfType[BISHOP | color] |
             position->piecesOfType[QUEEN | color]);
      }

      ITERATE_BITBOARD(&batteryPieces, square)
      {
         if ((squaresBetween[square][kingSquare] & position->allPieces) ==
             minValue[from])
         {
            return TRUE;
         }
      }
   }

   /* No undiscovered check: */

   if (piece & PP_SLIDING_PIECE)
   {
      return (bool)
         (testSquare(generalMoves[pieceType(piece)][to], kingSquare) &&
          (position->allPieces & squaresBetween[kingSquare][to]) ==
          EMPTY_BITBOARD);
   }
   else if (pieceType(piece) == KNIGHT)
   {
      return (testSquare(generalMoves[KNIGHT][kingSquare], to) !=
              EMPTY_BITBOARD);
   }
   else if (pieceType(piece) == PAWN)
   {
      const Piece newPiece = getNewPiece(move);

      if (newPiece == NO_PIECE)
      {
         if (testSquare(generalMoves[piece][to], kingSquare) != FALSE)
         {
            return TRUE;
         }

         if (to == position->enPassantSquare)
         {
            const Square captureSquare =
               (Square) (to + (rank(from) - rank(to)) * 8);

            if (testSquare(generalMoves[QUEEN][kingSquare], captureSquare) &&
                (squaresBetween[captureSquare][kingSquare] &
                 position->allPieces) == EMPTY_BITBOARD)
            {
               const Color color = position->activeColor;
               Bitboard batteryPieces;
               const Bitboard blockers = minValue[to] |
                  (position->allPieces & maxValue[captureSquare] &
                   maxValue[from]);
               Square square;

               if (rank(captureSquare) == rank(kingSquare) ||
                   file(captureSquare) == file(kingSquare))
               {
                  batteryPieces = squaresBehind[captureSquare][kingSquare] &
                     (position->piecesOfType[ROOK | color] |
                      position->piecesOfType[QUEEN | color]);
               }
               else
               {
                  batteryPieces = squaresBehind[captureSquare][kingSquare] &
                     (position->piecesOfType[BISHOP | color] |
                      position->piecesOfType[QUEEN | color]);
               }

               ITERATE_BITBOARD(&batteryPieces, square)
               {
                  if ((squaresBetween[kingSquare][square] & blockers) ==
                      EMPTY_BITBOARD)
                  {
                     return TRUE;
                  }
               }
            }
         }
      }
      else
      {
         if (newPiece & PP_SLIDING_PIECE)
         {
            const Bitboard blockers = position->allPieces & maxValue[from];

            return (bool)
               (testSquare(generalMoves[newPiece][to], kingSquare) &&
                (blockers & squaresBetween[kingSquare][to]) ==
                EMPTY_BITBOARD);
         }
         else
         {
            return (testSquare(generalMoves[KNIGHT][kingSquare], to) !=
                    EMPTY_BITBOARD);
         }
      }
   }
   else if (pieceType(piece) == KING && abs(to - from) == 2)
   {
      const Bitboard blockers = position->allPieces & maxValue[from];
      const int rookSquare = (to + from) / 2;

      return (bool)
         (testSquare(generalMoves[ROOK][rookSquare], kingSquare) &&
          (blockers & squaresBetween[kingSquare][rookSquare]) ==
          EMPTY_BITBOARD);
   }

   return FALSE;
}

int checkConsistency(const Position * position)
{
   Square square;
   int numPieces[2], numPawns[2], value[2], i;
   int openingValue[2], endgameValue[2];
   INT32 balance = 0;
   Bitboard temp;
   BYTE obstacles[NUM_LANES];
   UINT32 materialSignature = calculateMaterialSignature(position);

   numPieces[WHITE] = numPieces[BLACK] = 0;
   numPawns[WHITE] = numPawns[BLACK] = 0;
   memset(obstacles, 0x00, NUM_LANES);
   value[WHITE] = value[BLACK] = 0;
   openingValue[WHITE] = openingValue[BLACK] = 0;
   endgameValue[WHITE] = endgameValue[BLACK] = 0;

   assert(position->activeColor == WHITE || position->activeColor == BLACK);
   assert(getMaterialSignatureFromPieceCount(position) == materialSignature);

   {
      int calculatedNumWhiteQueens;
      int calculatedNumWhiteRooks;
      int calculatedNumWhiteLightSquareBishops;
      int calculatedNumWhiteDarkSquareBishops;
      int calculatedNumWhiteKnights;
      int calculatedNumWhitePawns;
      int calculatedNumBlackQueens;
      int calculatedNumBlackRooks;
      int calculatedNumBlackLightSquareBishops;
      int calculatedNumBlackDarkSquareBishops;
      int calculatedNumBlackKnights;
      int calculatedNumBlackPawns;

      getPieceCounters(materialSignature,
                       &calculatedNumWhiteQueens, &calculatedNumWhiteRooks,
                       &calculatedNumWhiteLightSquareBishops,
                       &calculatedNumWhiteDarkSquareBishops,
                       &calculatedNumWhiteKnights,
                       &calculatedNumWhitePawns, &calculatedNumBlackQueens,
                       &calculatedNumBlackRooks,
                       &calculatedNumBlackLightSquareBishops,
                       &calculatedNumBlackDarkSquareBishops,
                       &calculatedNumBlackKnights, &calculatedNumBlackPawns);

      assert(getPieceCount(position, WHITE_QUEEN) > 1 ||
             getPieceCount(position, WHITE_QUEEN) ==
             calculatedNumWhiteQueens);
      assert(getPieceCount(position, WHITE_ROOK) > 2 ||
             getPieceCount(position, WHITE_ROOK) == calculatedNumWhiteRooks);
      assert(getPieceCount(position, WHITE_BISHOP_LIGHT) > 1 ||
             getPieceCount(position, WHITE_BISHOP_LIGHT) ==
             calculatedNumWhiteLightSquareBishops);
      assert(getPieceCount(position, WHITE_BISHOP_DARK) > 1 ||
             getPieceCount(position, WHITE_BISHOP_DARK) ==
             calculatedNumWhiteDarkSquareBishops);
      assert(getPieceCount(position, WHITE_KNIGHT) > 2 ||
             getPieceCount(position, WHITE_KNIGHT) ==
             calculatedNumWhiteKnights);
      assert(position->numberOfPawns[WHITE] == calculatedNumWhitePawns);
      assert(getPieceCount(position, BLACK_QUEEN) > 1 ||
             getPieceCount(position, BLACK_QUEEN) ==
             calculatedNumBlackQueens);
      assert(getPieceCount(position, BLACK_ROOK) > 2 ||
             getPieceCount(position, BLACK_ROOK) == calculatedNumBlackRooks);
      assert(getPieceCount(position, BLACK_BISHOP_LIGHT) > 1 ||
             getPieceCount(position, BLACK_BISHOP_LIGHT) ==
             calculatedNumBlackLightSquareBishops);
      assert(getPieceCount(position, BLACK_BISHOP_DARK) > 1 ||
             getPieceCount(position, BLACK_BISHOP_DARK) ==
             calculatedNumBlackDarkSquareBishops);
      assert(getPieceCount(position, BLACK_KNIGHT) > 2 ||
             getPieceCount(position, BLACK_KNIGHT) ==
             calculatedNumBlackKnights);
      assert(position->numberOfPawns[BLACK] == calculatedNumBlackPawns);

      if (getPieceCount(position, WHITE_QUEEN) <= 1 &&
          getPieceCount(position, WHITE_ROOK) <= 2 &&
          getPieceCount(position, WHITE_BISHOP_LIGHT) <= 1 &&
          getPieceCount(position, WHITE_BISHOP_DARK) <= 1 &&
          getPieceCount(position, WHITE_KNIGHT) <= 2 &&
          getPieceCount(position, BLACK_QUEEN) <= 1 &&
          getPieceCount(position, BLACK_ROOK) <= 2 &&
          getPieceCount(position, BLACK_BISHOP_LIGHT) <= 1 &&
          getPieceCount(position, BLACK_BISHOP_DARK) <= 1 &&
          getPieceCount(position, BLACK_KNIGHT) <= 2)
      {
         const MaterialInfo *mi = &materialInfo[materialSignature];

         if (mi->materialBalance != materialBalance(position))
         {
            logDebug("mimb=%d cmb=%d\n", mi->materialBalance,
                     materialBalance(position));
            dumpPosition(position);
         }

         assert(mi->materialBalance == materialBalance(position));
         assert(mi->phaseIndex == phaseIndex(position));
      }
   }

   if (position->castlingRights & WHITE_00)
   {
      assert(position->piece[E1] == WHITE_KING);
      assert(position->piece[H1] == WHITE_ROOK);
   }

   if (position->castlingRights & WHITE_000)
   {
      assert(position->piece[E1] == WHITE_KING);
      assert(position->piece[A1] == WHITE_ROOK);
   }

   if (position->castlingRights & BLACK_00)
   {
      assert(position->piece[E8] == BLACK_KING);
      assert(position->piece[H8] == BLACK_ROOK);
   }

   if (position->castlingRights & BLACK_000)
   {
      assert(position->piece[E8] == BLACK_KING);
      assert(position->piece[A8] == BLACK_ROOK);
   }

   assert(position->enPassantSquare == NO_SQUARE ||
          squareIsValid(position->enPassantSquare));

   if (position->enPassantSquare != NO_SQUARE)
   {
      if (position->activeColor == WHITE)
      {
         assert(rank(position->enPassantSquare) == RANK_6);
         assert(position->piece[position->enPassantSquare - 8] == BLACK_PAWN);
      }
      else
      {
         assert(rank(position->enPassantSquare) == RANK_3);
         assert(position->piece[position->enPassantSquare + 8] == WHITE_PAWN);
      }
   }

   assert((position->piecesOfColor[WHITE] ^ position->piecesOfColor[BLACK]) ==
          position->allPieces);

   temp = EMPTY_BITBOARD;

   for (i = 1; i <= 0x0F; i++)
   {
      temp ^= position->piecesOfType[i];
   }

   assert(temp == position->allPieces);

   assert(squareIsValid(position->king[WHITE]));
   assert(position->piece[position->king[WHITE]] == WHITE_KING);
   assert(squareIsValid(position->king[BLACK]));
   assert(position->piece[position->king[BLACK]] == BLACK_KING);
   assert(testSquare
          (position->piecesOfType[WHITE_KING], position->king[WHITE]));
   assert(getNumberOfSetSquares(position->piecesOfType[WHITE_KING]) == 1);
   assert(testSquare
          (position->piecesOfType[BLACK_KING], position->king[BLACK]));
   assert(getNumberOfSetSquares(position->piecesOfType[BLACK_KING]) == 1);

   assert(getNumberOfSetSquares(position->piecesOfType[WHITE_QUEEN]) ==
          getPieceCount(position, WHITE_QUEEN));
   assert(getNumberOfSetSquares(position->piecesOfType[WHITE_ROOK]) ==
          getPieceCount(position, WHITE_ROOK));
   assert(getNumberOfSetSquares
          (position->piecesOfType[WHITE_BISHOP] & lightSquares) ==
          getPieceCount(position, WHITE_BISHOP_LIGHT));
   assert(getNumberOfSetSquares
          (position->piecesOfType[WHITE_BISHOP] & darkSquares) ==
          getPieceCount(position, WHITE_BISHOP_DARK));
   assert(getNumberOfSetSquares(position->piecesOfType[WHITE_KNIGHT]) ==
          getPieceCount(position, WHITE_KNIGHT));
   assert(getNumberOfSetSquares(position->piecesOfType[BLACK_QUEEN]) ==
          getPieceCount(position, BLACK_QUEEN));
   assert(getNumberOfSetSquares(position->piecesOfType[BLACK_ROOK]) ==
          getPieceCount(position, BLACK_ROOK));
   assert(getNumberOfSetSquares
          (position->piecesOfType[BLACK_BISHOP] & lightSquares) ==
          getPieceCount(position, BLACK_BISHOP_LIGHT));
   assert(getNumberOfSetSquares
          (position->piecesOfType[BLACK_BISHOP] & darkSquares) ==
          getPieceCount(position, BLACK_BISHOP_DARK));
   assert(getNumberOfSetSquares(position->piecesOfType[BLACK_KNIGHT]) ==
          getPieceCount(position, BLACK_KNIGHT));

   ITERATE(square)
   {
      Piece piece = position->piece[square];
      PieceType pieceType = pieceType(piece);
      Color color = pieceColor(piece);

      assert(piece >= 0 && piece <= 15);

      if (piece != NO_PIECE)
      {
         numPieces[color]++;
         setObstacleSquare(square, obstacles);

         assert(testSquare(position->allPieces, square));
         assert(testSquare(position->piecesOfColor[color], square));
         assert(testSquare(position->piecesOfType[piece], square));

         if (pieceType == PAWN)
         {
            numPawns[color]++;
         }

         if (pieceType != KING)
         {
            value[color] += basicValue[piece];
         }

         openingValue[color] +=
            getOpeningValue(pieceSquareBonus[piece][square]);
         endgameValue[color] +=
            getEndgameValue(pieceSquareBonus[piece][square]);

         if (color == WHITE)
         {
            balance += pieceSquareBonus[piece][square];
         }
         else
         {
            balance -= pieceSquareBonus[piece][square];
         }
      }
      else
      {
         assert(testSquare(position->allPieces, square) == FALSE);
         assert(testSquare(position->piecesOfColor[WHITE], square) == FALSE);
         assert(testSquare(position->piecesOfColor[BLACK], square) == FALSE);
      }
   }

   assert(numPieces[WHITE] == position->numberOfPieces[WHITE]);
   assert(numPieces[WHITE] >= 1);
   assert(numPieces[WHITE] <= 16);
   assert(numPieces[WHITE] ==
          getNumberOfSetSquares(position->piecesOfColor[WHITE]));
   assert(numPieces[BLACK] == position->numberOfPieces[BLACK]);
   assert(numPieces[BLACK] >= 1);
   assert(numPieces[BLACK] <= 16);
   assert(numPieces[BLACK] ==
          getNumberOfSetSquares(position->piecesOfColor[BLACK]));

   assert(numPawns[WHITE] == position->numberOfPawns[WHITE]);
   assert(numPawns[WHITE] >= 0);
   assert(numPawns[WHITE] <= 8);
   assert(numPawns[WHITE] ==
          getNumberOfSetSquares(position->piecesOfType[WHITE_PAWN]));
   assert(numPawns[BLACK] == position->numberOfPawns[BLACK]);
   assert(numPawns[BLACK] >= 0);
   assert(numPawns[BLACK] <= 8);
   assert(numPawns[BLACK] ==
          getNumberOfSetSquares(position->piecesOfType[BLACK_PAWN]));
   assert(balance == position->balance);

   assert(getHashValue(position) == position->hashValue);
   assert(getPawnHashValue(position) == position->pawnHashValue);

   return 0;
}

bool positionIsLegal(const Position * position)
{
   Square square;
   int numPieces[2], numPawns[2], value[2], i;
   Bitboard temp;
   BYTE obstacles[NUM_LANES];

   numPieces[WHITE] = numPieces[BLACK] = 0;
   numPawns[WHITE] = numPawns[BLACK] = 0;
   memset(obstacles, 0x00, NUM_LANES);
   value[WHITE] = value[BLACK] = 0;

   if (position->activeColor != WHITE && position->activeColor != BLACK)
   {
      return FALSE;
   }

   if (position->castlingRights & WHITE_00)
   {
      if (position->piece[E1] != WHITE_KING)
      {
         return FALSE;
      }

      if (position->piece[H1] != WHITE_ROOK)
      {
         return FALSE;
      }
   }

   if (position->castlingRights & WHITE_000)
   {
      if (position->piece[E1] != WHITE_KING)
      {
         return FALSE;
      }

      if (position->piece[A1] != WHITE_ROOK)
      {
         return FALSE;
      }
   }

   if (position->castlingRights & BLACK_00)
   {
      if (position->piece[E8] != BLACK_KING)
      {
         return FALSE;
      }

      if (position->piece[H8] != BLACK_ROOK)
      {
         return FALSE;
      }
   }

   if (position->castlingRights & BLACK_000)
   {
      if (position->piece[E8] != BLACK_KING)
      {
         return FALSE;
      }

      if (position->piece[A8] != BLACK_ROOK)
      {
         return FALSE;
      }
   }

   if (position->enPassantSquare != NO_SQUARE)
   {
      if (squareIsValid(position->enPassantSquare) == FALSE)
      {
         return FALSE;
      }

      if (position->activeColor == WHITE)
      {
         if (rank(position->enPassantSquare) != RANK_6)
         {
            return FALSE;
         }

         if (position->piece[position->enPassantSquare - 8] != BLACK_PAWN)
         {
            return FALSE;
         }
      }
      else
      {
         if (rank(position->enPassantSquare) != RANK_3)
         {
            return FALSE;
         }

         if (position->piece[position->enPassantSquare + 8] != WHITE_PAWN)
         {
            return FALSE;
         }
      }
   }

   if ((position->piecesOfColor[WHITE] ^ position->piecesOfColor[BLACK]) !=
       position->allPieces)
   {
      return FALSE;
   }

   temp = EMPTY_BITBOARD;

   for (i = 1; i <= 0x0F; i++)
   {
      temp ^= position->piecesOfType[i];
   }

   /*
      dumpBitboard(temp, "temp");
      dumpBitboard(position->allPieces, "allPieces");
    */

   if (temp != position->allPieces)
   {
      return FALSE;
   }

   if (squareIsValid(position->king[WHITE]) == FALSE)
   {
      return FALSE;
   }

   if (position->piece[position->king[WHITE]] != WHITE_KING)
   {
      return FALSE;
   }

   if (squareIsValid(position->king[BLACK]) == FALSE)
   {
      return FALSE;
   }

   if (position->piece[position->king[BLACK]] != BLACK_KING)
   {
      return FALSE;
   }

   if (distance(position->king[WHITE], position->king[BLACK]) < 2)
   {
      return FALSE;
   }

   if (testSquare(position->piecesOfType[WHITE_KING],
                  position->king[WHITE]) == FALSE)
   {
      return FALSE;
   }

   if (getNumberOfSetSquares(position->piecesOfType[WHITE_KING]) != 1)
   {
      return FALSE;
   }

   if (testSquare(position->piecesOfType[BLACK_KING],
                  position->king[BLACK]) == FALSE)
   {
      return FALSE;
   }

   if (getNumberOfSetSquares(position->piecesOfType[BLACK_KING]) != 1)
   {
      return FALSE;
   }

   ITERATE(square)
   {
      Piece piece = position->piece[square];
      PieceType pieceType = pieceType(piece);
      Color color = pieceColor(piece);

      if (piece < 0 || piece > 15)
      {
         return FALSE;
      }

      if (piece != NO_PIECE)
      {
         numPieces[color]++;
         setObstacleSquare(square, obstacles);

         if (testSquare(position->allPieces, square) == FALSE)
         {
            return FALSE;
         }

         if (testSquare(position->piecesOfColor[color], square) == FALSE)
         {
            return FALSE;
         }

         if (testSquare(position->piecesOfType[piece], square) == FALSE)
         {
            return FALSE;
         }

         if (pieceType == PAWN)
         {
            numPawns[color]++;
         }

         if (pieceType != KING)
         {
            value[color] += basicValue[piece];
         }
      }
      else
      {
         if (testSquare(position->allPieces, square))
         {
            return FALSE;
         }

         if (testSquare(position->piecesOfColor[WHITE], square))
         {
            return FALSE;
         }

         if (testSquare(position->piecesOfColor[BLACK], square))
         {
            return FALSE;
         }
      }
   }

   if (numPieces[WHITE] < 1)
   {
      return FALSE;
   }

   if (numPieces[WHITE] > 16)
   {
      return FALSE;
   }

   if (numPieces[BLACK] < 1)
   {
      return FALSE;
   }

   if (numPieces[BLACK] > 16)
   {
      return FALSE;
   }

   if (numPawns[WHITE] < 0)
   {
      return FALSE;
   }

   if (numPawns[WHITE] > 8)
   {
      return FALSE;
   }

   if (numPawns[BLACK] < 0)
   {
      return FALSE;
   }

   if (numPawns[BLACK] > 8)
   {
      return FALSE;
   }

   return TRUE;
}

bool positionsAreIdentical(const Position * position1,
                           const Position * position2)
{
   Square square;

   if (position1->activeColor != position2->activeColor ||
       position1->castlingRights != position2->castlingRights ||
       position1->enPassantSquare != position2->enPassantSquare)
   {
      logDebug("activeColor1=%d activeColor2=%d\n",
               position1->activeColor, position2->activeColor);
      logDebug("castlingRights1=%d castlingRights2=%d\n",
               position1->castlingRights, position2->castlingRights);
      logDebug("enPassantSquare1=%d enPassantSquare2=%d\n",
               position1->enPassantSquare, position2->enPassantSquare);

      return FALSE;
   }

   if (position1->halfMoveClock != position2->halfMoveClock ||
       position1->moveNumber != position2->moveNumber)
   {
      logDebug("halfMoveClock1=%d halfMoveClock2=%d\n",
               position1->halfMoveClock, position2->halfMoveClock);
      logDebug("moveNumber1=%d moveNumber2=%d\n",
               position1->moveNumber, position2->moveNumber);

      return FALSE;
   }

   ITERATE(square)
   {
      if (position1->piece[square] != position2->piece[square])
      {
         logDebug("piece diff!\n");
         dumpSquare(square);

         return FALSE;
      }
   }

   return (bool) (checkConsistency(position1) == 0);
}

static void initializeKrqSignatureTable()
{
   UINT32 qc, rc, kc;

   for (qc = 0; qc <= 9; qc++)
   {
      for (rc = 0; rc <= 10; rc++)
      {
         for (kc = 0; kc <= 10; kc++)
         {
            const int index = (qc << pieceCountShift[WHITE_QUEEN]) +
               (rc << pieceCountShift[WHITE_ROOK]) +
               (kc << pieceCountShift[WHITE_KNIGHT]);

            assert(index >= 0 && index < 4096);

            krqIndexWhite[index] =
               getMaterialSignature(qc, rc, 0, 0, kc, 0, 0, 0, 0, 0, 0, 0);
            krqIndexBlack[index] =
               getMaterialSignature(0, 0, 0, 0, 0, 0, qc, rc, 0, 0, kc, 0);
         }
      }
   }
}

static void initializeBbpSignatureTable()
{
   UINT32 pc, dc, lc;

   for (pc = 0; pc <= 8; pc++)
   {
      for (dc = 0; dc <= 9; dc++)
      {
         for (lc = 0; lc <= 9; lc++)
         {
            const int index =
               (lc << (pieceCountShift[WHITE_BISHOP_LIGHT] - 8)) +
               (dc << (pieceCountShift[WHITE_BISHOP_DARK] - 8)) + pc;

            assert(index >= 0 && index < 4096);

            bbpIndexWhite[index] =
               getMaterialSignature(0, 0, lc, dc, 0, pc, 0, 0, 0, 0, 0, 0);
            bbpIndexBlack[index] =
               getMaterialSignature(0, 0, 0, 0, 0, 0, 0, 0, lc, dc, 0, pc);
         }
      }
   }
}

int initializeModulePosition()
{
   Square square;
   int i;

   ITERATE(square)
   {
      remainingCastlings[square] = (WHITE_00 | WHITE_000 |
                                    BLACK_00 | BLACK_000);

      switch (square)
      {
      case A1:
         remainingCastlings[square] -= WHITE_000;
         break;

      case E1:
         remainingCastlings[square] -= WHITE_000;
         remainingCastlings[square] -= WHITE_00;
         break;

      case H1:
         remainingCastlings[square] -= WHITE_00;
         break;

      case A8:
         remainingCastlings[square] -= BLACK_000;
         break;

      case E8:
         remainingCastlings[square] -= BLACK_000;
         remainingCastlings[square] -= BLACK_00;
         break;

      case H8:
         remainingCastlings[square] -= BLACK_00;
         break;

      default:
         break;
      }
   }

   rookOrigin[C1] = A1;
   rookOrigin[G1] = H1;
   rookOrigin[C8] = A8;
   rookOrigin[G8] = H8;

   basicValue[NO_PIECE] = 0;
   basicValue[WHITE_KING] = basicValue[BLACK_KING] = -VALUE_MATED;
   basicValue[WHITE_QUEEN] = basicValue[BLACK_QUEEN] =
      max(VALUE_QUEEN_OPENING, VALUE_QUEEN_ENDGAME);
   basicValue[WHITE_ROOK] = basicValue[BLACK_ROOK] =
      max(VALUE_ROOK_OPENING, VALUE_ROOK_ENDGAME);
   basicValue[WHITE_BISHOP] = basicValue[BLACK_BISHOP] =
      max(VALUE_BISHOP_OPENING, VALUE_BISHOP_ENDGAME);
   basicValue[WHITE_KNIGHT] = basicValue[BLACK_KNIGHT] =
      max(VALUE_KNIGHT_OPENING, VALUE_KNIGHT_ENDGAME);
   basicValue[WHITE_PAWN] = basicValue[BLACK_PAWN] =
      max(PAWN_DEFAULTVALUE_OPENING, PAWN_DEFAULTVALUE_ENDGAME);

   simplePieceValue[NO_PIECE] = 0;
   simplePieceValue[WHITE_KING] = simplePieceValue[BLACK_KING] = -VALUE_MATED;
   simplePieceValue[WHITE_QUEEN] = simplePieceValue[BLACK_QUEEN] = 975;
   simplePieceValue[WHITE_ROOK] = simplePieceValue[BLACK_ROOK] = 500;
   simplePieceValue[WHITE_BISHOP] = simplePieceValue[BLACK_BISHOP] = 325;
   simplePieceValue[WHITE_KNIGHT] = simplePieceValue[BLACK_KNIGHT] = 325;
   simplePieceValue[WHITE_PAWN] = simplePieceValue[BLACK_PAWN] = 100;

   /*logDebug("size of Variation: %d bytes\n", sizeof(Variation)); */

   pieceCountShift[WHITE_KING] = 40;
   pieceCountShift[WHITE_QUEEN] = 0;
   pieceCountShift[WHITE_ROOK] = 4;
   pieceCountShift[WHITE_KNIGHT] = 8;
   pieceCountShift[WHITE_BISHOP_LIGHT] = 12;
   pieceCountShift[WHITE_BISHOP_DARK] = 16;
   pieceCountShift[WHITE_PAWN] = 40;
   pieceCountShift[BLACK_KING] = 40;
   pieceCountShift[BLACK_QUEEN] = 20;
   pieceCountShift[BLACK_ROOK] = 24;
   pieceCountShift[BLACK_KNIGHT] = 28;
   pieceCountShift[BLACK_BISHOP_LIGHT] = 32;
   pieceCountShift[BLACK_BISHOP_DARK] = 36;
   pieceCountShift[BLACK_PAWN] = 40;

   for (i = 0; i < 16; i++)
   {
      pieceCountWeight[i] =
         (pieceCountShift[i] < 40 ? ((UINT64) (1)) << pieceCountShift[i] : 0);

      /* logDebug("pcw(%d) = %lu (%d)\n", i, pieceCountWeight[i], pieceCountShift[i]); */
   }

   ITERATE(square)
   {
      if (testSquare(darkSquares, square))
      {
         bishopPieceCountWeight[WHITE][square] =
            pieceCountWeight[WHITE_BISHOP_DARK];
         bishopPieceCountWeight[BLACK][square] =
            pieceCountWeight[BLACK_BISHOP_DARK];
      }
      else
      {
         bishopPieceCountWeight[WHITE][square] =
            pieceCountWeight[WHITE_BISHOP_LIGHT];
         bishopPieceCountWeight[BLACK][square] =
            pieceCountWeight[BLACK_BISHOP_LIGHT];
      }
   }

   initializeKrqSignatureTable();
   initializeBbpSignatureTable();

   return 0;
}

static int checkMove(Square from, Square to, Piece newPiece,
                     Variation * variation)
{
   Move move = getPackedMove(from, to, newPiece);

   makeMove(variation, move);
   assert(checkConsistency(&variation->singlePosition) == 0);

   return 0;
}

static int testPawnMoves()
{
   Variation variation;

   initializeVariation(&variation, FEN_GAMESTART);
   assert(checkConsistency(&variation.singlePosition) == 0);

   checkMove(E2, E4, NO_PIECE, &variation);
   assert(variation.singlePosition.enPassantSquare == E3);
   checkMove(D7, D5, NO_PIECE, &variation);
   assert(variation.singlePosition.enPassantSquare == D6);

   checkMove(E4, D5, NO_PIECE, &variation);
   assert(variation.singlePosition.enPassantSquare == NO_SQUARE);
   checkMove(D8, D5, NO_PIECE, &variation);

   checkMove(D2, D4, NO_PIECE, &variation);
   checkMove(D5, D8, NO_PIECE, &variation);

   checkMove(D4, D5, NO_PIECE, &variation);
   checkMove(C7, C5, NO_PIECE, &variation);
   assert(variation.singlePosition.enPassantSquare == C6);

   checkMove(D5, C6, NO_PIECE, &variation);
   checkMove(E7, E5, NO_PIECE, &variation);

   checkMove(C6, B7, NO_PIECE, &variation);
   checkMove(E5, E4, NO_PIECE, &variation);

   checkMove(F2, F4, NO_PIECE, &variation);
   assert(variation.singlePosition.enPassantSquare == F3);
   checkMove(E4, F3, NO_PIECE, &variation);

   checkMove(B7, C8, WHITE_ROOK, &variation);
   checkMove(F3, G2, NO_PIECE, &variation);

   checkMove(C8, B8, NO_PIECE, &variation);
   checkMove(G2, H1, WHITE_QUEEN, &variation);

   checkMove(D1, D8, WHITE_QUEEN, &variation);  /* checkmate; what a game! */

   return 0;
}

static int testShortCastlings()
{
   Variation variation;

   initializeVariation(&variation, FEN_GAMESTART);
   assert(checkConsistency(&variation.singlePosition) == 0);

   checkMove(E2, E4, NO_PIECE, &variation);
   checkMove(E7, E5, NO_PIECE, &variation);

   checkMove(G1, F3, NO_PIECE, &variation);
   checkMove(G8, F6, NO_PIECE, &variation);

   checkMove(F1, C4, NO_PIECE, &variation);
   checkMove(F8, C5, NO_PIECE, &variation);

   checkMove(E1, G1, NO_PIECE, &variation);
   assert(variation.singlePosition.piece[F1] == WHITE_ROOK);
   checkMove(E8, G8, NO_PIECE, &variation);
   assert(variation.singlePosition.piece[F8] == BLACK_ROOK);

   return 0;
}

static int testLongCastlings()
{
   Variation variation;

   initializeVariation(&variation, FEN_GAMESTART);
   assert(checkConsistency(&variation.singlePosition) == 0);

   checkMove(D2, D4, NO_PIECE, &variation);
   checkMove(D7, D5, NO_PIECE, &variation);

   checkMove(B1, C3, NO_PIECE, &variation);
   checkMove(B8, C6, NO_PIECE, &variation);

   checkMove(C1, F4, NO_PIECE, &variation);
   checkMove(C8, F5, NO_PIECE, &variation);

   checkMove(D1, D2, NO_PIECE, &variation);
   checkMove(D8, D7, NO_PIECE, &variation);

   checkMove(E1, C1, NO_PIECE, &variation);
   assert(variation.singlePosition.piece[D1] == WHITE_ROOK);
   checkMove(E8, C8, NO_PIECE, &variation);
   assert(variation.singlePosition.piece[D8] == BLACK_ROOK);

   return 0;
}

static int testCastlingLegality()
{
   Variation variation, *p_variation = &variation;

   initializeVariation(&variation, FEN_GAMESTART);
   assert(checkConsistency(&variation.singlePosition) == 0);

   checkMove(E2, E4, NO_PIECE, &variation);
   checkMove(E7, E5, NO_PIECE, &variation);

   checkMove(G1, F3, NO_PIECE, &variation);
   checkMove(G8, F6, NO_PIECE, &variation);

   checkMove(F3, E5, NO_PIECE, &variation);
   checkMove(F6, E4, NO_PIECE, &variation);

   checkMove(E5, G6, NO_PIECE, &variation);
   checkMove(E4, G3, NO_PIECE, &variation);

   checkMove(F1, C4, NO_PIECE, &variation);
   checkMove(F8, C5, NO_PIECE, &variation);

   assert(makeMove(&variation, getPackedMove(E1, G1, NO_PIECE)) == 1);
   unmakeLastMove(p_variation);

   checkMove(H2, G3, NO_PIECE, &variation);

   assert(makeMove(&variation, getPackedMove(E8, G8, NO_PIECE)) == 1);
   unmakeLastMove(p_variation);

   checkMove(H7, G6, NO_PIECE, &variation);

   assert(makeMove(&variation, getPackedMove(E1, G1, NO_PIECE)) == 0);

   assert(makeMove(&variation, getPackedMove(E8, G8, NO_PIECE)) == 0);

   initializeVariation(&variation, FEN_GAMESTART);
   assert(checkConsistency(&variation.singlePosition) == 0);

   checkMove(E2, E4, NO_PIECE, &variation);
   checkMove(E7, E5, NO_PIECE, &variation);

   checkMove(G1, F3, NO_PIECE, &variation);
   checkMove(G8, F6, NO_PIECE, &variation);

   checkMove(F3, E5, NO_PIECE, &variation);
   checkMove(F6, E4, NO_PIECE, &variation);

   checkMove(E5, F3, NO_PIECE, &variation);
   checkMove(E4, F6, NO_PIECE, &variation);

   checkMove(F1, C4, NO_PIECE, &variation);
   checkMove(F8, C5, NO_PIECE, &variation);

   checkMove(D1, E2, NO_PIECE, &variation);

   assert(makeMove(&variation, getPackedMove(E8, G8, NO_PIECE)) == 1);
   unmakeLastMove(p_variation); /* 0-0 */
   unmakeLastMove(p_variation); /* Qe2+ */

   assert(makeMove(&variation, getPackedMove(E1, G1, NO_PIECE)) == 0);
   unmakeLastMove(p_variation); /* 0-0 */

   checkMove(D2, D3, NO_PIECE, &variation);

   assert(makeMove(&variation, getPackedMove(E8, G8, NO_PIECE)) == 0);
   unmakeLastMove(p_variation); /* 0-0 */

   checkMove(D8, E7, NO_PIECE, &variation);

   assert(makeMove(&variation, getPackedMove(E1, G1, NO_PIECE)) == 1);
   unmakeLastMove(p_variation); /* 0-0 */
   unmakeLastMove(p_variation); /* Qe7+ */

   checkMove(D7, D6, NO_PIECE, &variation);

   assert(makeMove(&variation, getPackedMove(E1, G1, NO_PIECE)) == 0);

   return 0;
}

static int testMove()
{
#ifndef NDEBUG
   Move move;

   move = getMove(A1, C3, NO_PIECE, -17);

   assert(getFromSquare(move) == A1);
   assert(getToSquare(move) == C3);
   assert(getNewPiece(move) == NO_PIECE);
   assert(getMoveValue(move) == -17);
#endif

   return 0;
}

int testModulePosition()
{
   int result;

   if ((result = testPawnMoves()) != 0)
   {
      return result;
   }

   if ((result = testShortCastlings()) != 0)
   {
      return result;
   }

   if ((result = testLongCastlings()) != 0)
   {
      return result;
   }

   if ((result = testCastlingLegality()) != 0)
   {
      return result;
   }

   if ((result = testMove()) != 0)
   {
      return result;
   }

   return 0;
}
