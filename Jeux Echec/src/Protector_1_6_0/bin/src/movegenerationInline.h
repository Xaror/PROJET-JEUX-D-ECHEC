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

/**
 * Register the specified killermove.
 */
INLINE void registerKillerMove(PlyInfo * plyInfo, Move killerMove)
{
   if (plyInfo->killerMove1 != killerMove)
   {
      plyInfo->killerMove2 = plyInfo->killerMove1;
      plyInfo->killerMove1 = killerMove;
   }
}

/**
 * Test if the passive king can be captured.
 */
INLINE bool passiveKingIsSafe(Position * position)
{
   return (bool)
      (getDirectAttackers(position,
                          position->king[opponent(position->activeColor)],
                          position->activeColor,
                          position->allPieces) == EMPTY_BITBOARD);
}

/**
 * Test if the active king is safe (i.e. not in check).
 */
INLINE bool activeKingIsSafe(Position * position)
{
   return (bool)
      (getDirectAttackers(position,
                          position->king[position->activeColor],
                          opponent(position->activeColor),
                          position->allPieces) == EMPTY_BITBOARD);
}

INLINE int seeMove(Position * position, const Move move)
{
   const Square to = getToSquare(move);
   const Piece targetPiece = position->piece[to];
   const Bitboard all = position->allPieces;
   const Square enPassantSquare = position->enPassantSquare;
   int result;
   Bitboard attackers[2];

   attackers[WHITE] =
      getDirectAttackers(position, to, WHITE, position->allPieces);
   attackers[BLACK] =
      getDirectAttackers(position, to, BLACK, position->allPieces);

   result = seeMoveRec(position, move, attackers, VALUE_MATED);
   position->enPassantSquare = enPassantSquare;
   position->allPieces = all;
   position->piece[to] = targetPiece;

   return result;
}

/**
 * Compare the value of the two specified moves.
 */
INLINE int CDECL compareMoves(const void *move1, const void *move2)
{
   return getMoveValue(*((Move *) move2)) - getMoveValue(*((Move *) move1));
}

/**
 * Sort the specified movelist.
 */
INLINE void sortMoves(Movelist * movelist)
{
   qsort(&(movelist->moves[0]), movelist->numberOfMoves,
         sizeof(Move), compareMoves);
}

/**
 * Initialize the specified movelist for quiescence move generation.
 */
INLINE void initQuiescenceMovelist(Movelist * movelist,
                                   Position * position, PlyInfo * plyInfo,
                                   UINT16 * historyValue, const Move hashMove,
                                   const int restDepth, const bool check)
{
   movelist->position = position;
   movelist->plyInfo = plyInfo;
   movelist->historyValue = historyValue;
   movelist->nextMove = movelist->numberOfPieces = 0;
   movelist->numberOfMoves = movelist->numberOfBadCaptures = 0;
   movelist->hashMove = hashMove;
   movelist->killer1Executed = movelist->killer2Executed = FALSE;
   movelist->killer3Executed = movelist->killer4Executed = FALSE;

   if (check)
   {
      movelist->currentStage = MG_SCHEME_ESCAPE;
   }
   else
   {
      movelist->currentStage =
         (restDepth >= 0 ?
          MG_SCHEME_QUIESCENCE_WITH_CHECKS : MG_SCHEME_QUIESCENCE);
   }

   if (hashMove != NO_MOVE)
   {
      movelist->moves[movelist->numberOfMoves++] = hashMove;
   }
}

/**
 * Initialize the specified movelist for standard move generation.
 */
INLINE void initStandardMovelist(Movelist * movelist, Position * position,
                                 PlyInfo * plyInfo, UINT16 * historyValue,
                                 const Move hashMove, const bool check)
{
   movelist->position = position;
   movelist->plyInfo = plyInfo;
   movelist->historyValue = historyValue;
   movelist->nextMove = movelist->numberOfPieces = 0;
   movelist->numberOfMoves = movelist->numberOfBadCaptures = 0;
   movelist->hashMove = hashMove;

   if (check)
   {
      movelist->currentStage = MG_SCHEME_ESCAPE;
   }
   else
   {
      movelist->currentStage = MG_SCHEME_STANDARD;

      if (hashMove != NO_MOVE)
      {
         movelist->moves[movelist->numberOfMoves++] = hashMove;
      }
   }
}

/**
 * Initialize the specified movelist for pre quiescence move generation.
 */
INLINE void initPreQuiescenceMovelist(Movelist * movelist,
                                      Position * position,
                                      PlyInfo * plyInfo,
                                      UINT16 * historyValue,
                                      const Move hashMove, const bool check)
{
   movelist->position = position;
   movelist->plyInfo = plyInfo;
   movelist->historyValue = historyValue;
   movelist->nextMove = movelist->numberOfPieces = 0;
   movelist->numberOfMoves = movelist->numberOfBadCaptures = 0;
   movelist->hashMove = hashMove;

   if (check)
   {
      movelist->currentStage = MG_SCHEME_ESCAPE;
   }
   else
   {
      movelist->currentStage = MG_SCHEME_PRE_QUIESCENCE;

      if (hashMove != NO_MOVE)
      {
         movelist->moves[movelist->numberOfMoves++] = hashMove;
      }
   }
}

/**
 * Initialize the specified movelist for check move generation.
 */
INLINE void initCheckMovelist(Movelist * movelist, Position * position,
                              UINT16 * historyValue)
{
   movelist->position = position;
   movelist->plyInfo = 0;
   movelist->historyValue = historyValue;
   movelist->nextMove = movelist->numberOfPieces = 0;
   movelist->numberOfMoves = movelist->numberOfBadCaptures = 0;
   movelist->hashMove = NO_MOVE;
   movelist->currentStage = MG_SCHEME_CHECKS;
   movelist->killer1Executed = movelist->killer2Executed = FALSE;
   movelist->killer3Executed = movelist->killer4Executed = FALSE;
}

/**
 * Initialize the specified movelist for move insertion.
 */
INLINE void initMovelist(Movelist * movelist, Position * position)
{
   movelist->position = position;
   movelist->plyInfo = 0;
   movelist->historyValue = 0;
   movelist->nextMove = movelist->numberOfPieces = 0;
   movelist->numberOfMoves = movelist->numberOfBadCaptures = 0;
   movelist->hashMove = NO_MOVE;
   movelist->currentStage = MG_SCHEME_CHECKS;
   movelist->killer1Executed = movelist->killer2Executed = FALSE;
   movelist->killer3Executed = movelist->killer4Executed = FALSE;
}

INLINE Move getNextMove(Movelist * movelist)
{
   do
   {
      if (movelist->nextMove < movelist->numberOfMoves)
      {
         switch (moveGenerationStage[movelist->currentStage])
         {
         case MGS_GOOD_CAPTURES_AND_PROMOTIONS:
         case MGS_GOOD_CAPTURES_AND_PROMOTIONS_PURE:
            {
               const Move move = movelist->moves[movelist->nextMove++];

               if (basicValue[movelist->position->piece[getFromSquare(move)]]
                   > basicValue[movelist->position->piece[getToSquare(move)]]
                   && seeMove(movelist->position, move) < 0)
               {
                  movelist->badCaptures[movelist->numberOfBadCaptures++] =
                     move;

                  continue;
               }

               return move;
            }

         case MGS_SAFE_CHECKS:
            {
               const Move move = movelist->moves[movelist->nextMove++];

               if (seeMove(movelist->position, move) < 0)
               {
                  continue;
               }

               return move;
            }

         default:

            return movelist->moves[movelist->nextMove++];
         }
      }
      else
      {
         Position *position;
         Move killer1, killer2, killer3, killer4;

         switch (moveGenerationStage[++movelist->currentStage])
         {
         case MGS_GOOD_CAPTURES_AND_PROMOTIONS:
            generateSpecialMoves(movelist);
            break;

         case MGS_GOOD_CAPTURES_AND_PROMOTIONS_PURE:
            generateSpecialMovesPure(movelist);
            break;

         case MGS_KILLER_MOVES:
            movelist->killer1Executed = movelist->killer2Executed = FALSE;
            movelist->killer3Executed = movelist->killer4Executed = FALSE;
            movelist->numberOfMoves = movelist->nextMove = 0;
            position = movelist->position;
            killer1 = movelist->plyInfo->killerMove1;
            killer2 = movelist->plyInfo->killerMove2;
            killer3 = movelist->plyInfo->killerMove3;
            killer4 = movelist->plyInfo->killerMove4;

            if (moveIsPseudoLegal(position, killer1) &&
                position->piece[getToSquare(killer1)] == NO_PIECE &&
                position->piece[getFromSquare(killer1)] ==
                getMoveValue(killer1) &&
                (pieceType(position->piece[getFromSquare(killer1)]) != PAWN ||
                 getToSquare(killer1) != position->enPassantSquare) &&
                movesAreEqual(killer1, movelist->hashMove) == FALSE)
            {
               setMoveValue(&killer1, 4);
               movelist->moves[movelist->numberOfMoves++] = killer1;
               movelist->killer1Executed = TRUE;
            }

            if (moveIsPseudoLegal(position, killer2) &&
                position->piece[getToSquare(killer2)] == NO_PIECE &&
                position->piece[getFromSquare(killer2)] ==
                getMoveValue(killer2) &&
                (pieceType(position->piece[getFromSquare(killer2)]) != PAWN ||
                 getToSquare(killer2) != position->enPassantSquare) &&
                movesAreEqual(killer2, movelist->hashMove) == FALSE)
            {
               setMoveValue(&killer2, 3);
               movelist->moves[movelist->numberOfMoves++] = killer2;
               movelist->killer2Executed = TRUE;
            }

            if (movelist->killer1Executed == FALSE &&
                movelist->killer2Executed == FALSE &&
                moveIsPseudoLegal(position, killer3) &&
                position->piece[getToSquare(killer3)] == NO_PIECE &&
                position->piece[getFromSquare(killer3)] ==
                getMoveValue(killer3) &&
                (pieceType(position->piece[getFromSquare(killer3)]) != PAWN ||
                 getToSquare(killer3) != position->enPassantSquare) &&
                movesAreEqual(killer3, movelist->hashMove) == FALSE)
            {
               setMoveValue(&killer3, 2);
               movelist->moves[movelist->numberOfMoves++] = killer3;
               movelist->killer3Executed = TRUE;
            }

            if (movelist->killer1Executed == FALSE &&
                movelist->killer2Executed == FALSE &&
                moveIsPseudoLegal(position, killer4) &&
                position->piece[getToSquare(killer4)] == NO_PIECE &&
                position->piece[getFromSquare(killer4)] ==
                getMoveValue(killer4) &&
                (pieceType(position->piece[getFromSquare(killer4)]) != PAWN ||
                 getToSquare(killer4) != position->enPassantSquare) &&
                movesAreEqual(killer4, movelist->hashMove) == FALSE)
            {
               setMoveValue(&killer4, 1);
               movelist->moves[movelist->numberOfMoves++] = killer4;
               movelist->killer4Executed = TRUE;
            }

            break;

         case MGS_REST:
            generateRestMoves(movelist);
            break;

         case MGS_BAD_CAPTURES:
            movelist->numberOfMoves = movelist->numberOfBadCaptures;
            movelist->nextMove = 0;
            memmove(&movelist->moves[0],
                    &movelist->badCaptures[0],
                    movelist->numberOfMoves * sizeof(Move));
            break;

         case MGS_ESCAPES:
            generateEscapes(movelist);
            break;

         case MGS_SAFE_CHECKS:
            generateChecks(movelist, FALSE);
            break;

         case MGS_CHECKS:
            generateChecks(movelist, TRUE);
            break;

         case MGS_DANGEROUS_PAWN_ADVANCES:
            generateDangerousPawnAdvances(movelist);
            break;

         default:
            break;
         }
      }
   }
   while (moveGenerationStage[movelist->currentStage] != MGS_FINISHED);

   return NO_MOVE;
}
