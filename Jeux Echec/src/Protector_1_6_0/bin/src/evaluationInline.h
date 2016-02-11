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

INLINE void addEvalBonusForColor(EvaluationBase * base, const Color color,
                                 const INT32 bonus)
{
   if (color == WHITE)
   {
      base->balance += bonus;
   }
   else
   {
      base->balance -= bonus;
   }
}

INLINE void addEvalMalusForColor(EvaluationBase * base, const Color color,
                                 const INT32 bonus)
{
   if (color == WHITE)
   {
      base->balance -= bonus;
   }
   else
   {
      base->balance += bonus;
   }
}

INLINE Color getWinningColor(const Position * position, const int value)
{
   if (position->activeColor == WHITE)
   {
      return (value >= 0 ? WHITE : BLACK);
   }
   else
   {
      return (value <= 0 ? WHITE : BLACK);
   }
}

INLINE Bitboard getPromotablePawns(const Position * position,
                                   const Color color)
{
   const Color oppColor = opponent(color);
   const Square oppKing = position->king[oppColor];
   const bool lightSquaredBishop = (bool)
      (EMPTY_BITBOARD !=
       (lightSquares & position->piecesOfType[BISHOP | color]));
   const Bitboard pawns = position->piecesOfType[PAWN | color];
   Bitboard supporters;
   File excludeFile;
   Rank promotionRank;
   Square promotionSquare;

   if (color == WHITE)
   {
      excludeFile = (lightSquaredBishop ? FILE_H : FILE_A);
      promotionRank = RANK_8;
   }
   else
   {
      excludeFile = (lightSquaredBishop ? FILE_A : FILE_H);
      promotionRank = RANK_1;
   }

   promotionSquare = getSquare(excludeFile, promotionRank);
   supporters = companionFiles[promotionSquare] & pawns;

   if (distance(oppKing, promotionSquare) <= 1 &&
       supporters == EMPTY_BITBOARD)
   {
      return pawns & ~squaresOfFile[excludeFile];
   }
   else
   {
      return pawns;
   }
}

INLINE bool oppositeColoredBishops(const Position * position)
{
   if (getPieceCount(position, (Piece) WHITE_BISHOP_DARK) +
       getPieceCount(position, (Piece) WHITE_BISHOP_LIGHT) == 1 &&
       getPieceCount(position, (Piece) BLACK_BISHOP_DARK) +
       getPieceCount(position, (Piece) BLACK_BISHOP_LIGHT) == 1)
   {
      const Bitboard bishops =
         position->piecesOfType[WHITE_BISHOP] |
         position->piecesOfType[BLACK_BISHOP];

      return (bool) ((lightSquares & bishops) != EMPTY_BITBOARD &&
                     (darkSquares & bishops) != EMPTY_BITBOARD);
   }
   else
   {
      return FALSE;
   }
}

INLINE int getKnnkpChances(const Position * position, const Color color)
{
   Bitboard oppPawns = position->piecesOfType[PAWN | opponent(color)];

   return ((oppPawns & troitzkyArea[color]) == EMPTY_BITBOARD ? 0 : 8);
}

INLINE bool passiveKingStopsPawn(const Square kingSquare,
                                 const Square pawnSquare,
                                 const Color pawnColor)
{
   return testSquare(pawnOpponents[pawnColor][pawnSquare], kingSquare);
}

INLINE int getKrppkrChances(const Position * position, const Color color)
{
   const Color oppColor = opponent(color);
   Bitboard pawns = position->piecesOfType[(Piece) (PAWN | color)];
   const Square oppKing = position->king[oppColor];
   Square square;
   Bitboard files = EMPTY_BITBOARD;

   assert(getNumberOfSetSquares(pawns) == 2);

   if (colorRank(color, oppKing) == RANK_8)
   {
      return 16;
   }

   ITERATE_BITBOARD(&pawns, square)
   {
      const File pawnFile = file(square);

      files |= minValue[getSquare(pawnFile, RANK_1)];

      if (passiveKingStopsPawn(oppKing, square, color) == FALSE)
      {
         const int fileDiff = abs(file(oppKing) - pawnFile);
         const int rankDiff =
            colorRank(color, square) - colorRank(color, oppKing);

         if ((fileDiff > 2) || (fileDiff == 2 && rankDiff >= 0))
         {
            return 16;
         }
      }
   }

   if (files == A1C1 || files == F1H1 || getNumberOfSetSquares(files) == 1)
   {
      return 4;
   }

   return 16;
}

INLINE int getKrpkrChances(const Position * position, const Color color)
{
   const Color oppColor = opponent(color);
   Bitboard pawns = position->piecesOfType[(Piece) (PAWN | color)];
   const Square oppKing = position->king[oppColor];
   Square square = getLastSquare(&pawns);

   assert(getNumberOfSetSquares(pawns) == 0);

   if (passiveKingStopsPawn(oppKing, square, color))
   {
      if (testSquare(krprkDrawFiles, square))
      {
         return 2;
      }
      else
      {
         const Square king = position->king[color];
         const Rank kingRank = colorRank(color, king);
         const Rank pawnRank = colorRank(color, square);

         if (kingRank < RANK_6 || pawnRank < RANK_6)
         {
            return 2;
         }
      }
   }

   return 16;
}

INLINE int getKqppkqChances(const Position * position, const Color color)
{
   const Color oppColor = opponent(color);
   Bitboard pawns = position->piecesOfType[(Piece) (PAWN | color)];
   const Square oppKing = position->king[oppColor];
   Square square;
   Bitboard files = EMPTY_BITBOARD;

   assert(getNumberOfSetSquares(pawns) == 2);

   ITERATE_BITBOARD(&pawns, square)
   {
      if (passiveKingStopsPawn(oppKing, square, color) == FALSE)
      {
         return 16;
      }

      files |= minValue[getSquare(file(square), RANK_1)];
   }

   if (files == A1B1 || files == G1H1 || getNumberOfSetSquares(files) == 1)
   {
      return 4;
   }

   return 16;
}

INLINE int getKqpkqChances(const Position * position, const Color color)
{
   const Color oppColor = opponent(color);
   Bitboard pawns = position->piecesOfType[(Piece) (PAWN | color)];
   const Square oppKing = position->king[oppColor];
   Square square = getLastSquare(&pawns);
   const File pawnFile = file(square);
   const Rank pawnRank = colorRank(color, square);
   const int distDiff = distance(oppKing, square) -
      distance(position->king[color], square);
   const int distDiv = (distDiff <= 0 ? 2 : 1);

   assert(getNumberOfSetSquares(pawns) == 0);

   if (pawnRank <= RANK_6 && (pawnFile <= FILE_B || pawnFile >= FILE_G))
   {
      return (passiveKingStopsPawn(oppKing, square, color) ? 1 : 4 / distDiv);
   }
   else
   {
      return (passiveKingStopsPawn(oppKing, square, color) ?
              4 : 16 / distDiv);
   }
}

INLINE int getKpkChances(const Position * position, const Color color)
{
   const Color oppColor = opponent(color);
   Bitboard pawns = position->piecesOfType[(Piece) (PAWN | color)];

   if ((pawns & nonA & nonH) != EMPTY_BITBOARD)
   {
      return 16;
   }
   else
   {
      const Square oppKing = position->king[oppColor];
      Square square;

      ITERATE_BITBOARD(&pawns, square)
      {
         if (passiveKingStopsPawn(oppKing, square, color) == FALSE)
         {
            return 16;
         }
      }

      return 0;                 /* king holds pawn(s) */
   }
}

INLINE int getKbpkChances(const Position * position, const Color color)
{
   const Color oppColor = opponent(color);
   const bool oppColors = oppositeColoredBishops(position);
   const int max = (oppColors ? 8 : 16);
   const Bitboard promotablePawns = getPromotablePawns(position, color);
   const int numPromotablePawns = getNumberOfSetSquares(promotablePawns);
   const int numDefenders =
      (oppColors ?
       getNumberOfSetSquares(position->piecesOfType[BISHOP | oppColor]) : 0);

   return (numPromotablePawns > numDefenders ? max : 0);
}

INLINE int specialPositionChances(const Position * position,
                                  const SpecialEvalType type,
                                  const Color color)
{
   switch (type)
   {
   case Se_KpK:
      return getKpkChances(position, color);

   case Se_KbpK:
      return getKbpkChances(position, color);

   case Se_KrpKr:
      return getKrpkrChances(position, color);

   case Se_KrppKr:
      return getKrppkrChances(position, color);

   case Se_KqpKq:
      return getKqpkqChances(position, color);

   case Se_KqppKq:
      return getKqppkqChances(position, color);

   case Se_KnnKp:
      return getKnnkpChances(position, color);

   default:
      return 16;
   }
}

INLINE int getChances(const Position * position,
                      const MaterialInfo * mi, const Color winningColor)
{
   int chances = 16;

   if (numberOfNonPawnPieces(position, winningColor) <= 4 &&
       getPieceCount(position, WHITE_QUEEN) <= 1 &&
       getPieceCount(position, BLACK_QUEEN) <= 1)
   {
      if (winningColor == WHITE)
      {
         if (mi->specialEvalWhite != Se_None)
         {
            const int specialChances =
               specialPositionChances(position, mi->specialEvalWhite, WHITE);

            chances = min(specialChances, mi->chancesWhite);
         }
         else
         {
            chances = mi->chancesWhite;
         }
      }
      else
      {
         if (mi->specialEvalBlack != Se_None)
         {
            const int specialChances =
               specialPositionChances(position, mi->specialEvalBlack, BLACK);

            chances = min(specialChances, mi->chancesBlack);
         }
         else
         {
            chances = mi->chancesBlack;
         }
      }
   }

   return chances;
}

INLINE bool hasBishopPair(const Position * position, const Color color)
{
   const Bitboard *bishops =
      &position->piecesOfType[(Piece) (BISHOP | color)];

   return (bool) ((lightSquares & *bishops) != EMPTY_BITBOARD &&
                  (darkSquares & *bishops) != EMPTY_BITBOARD);
}

INLINE int phaseValue(INT32 value, INT32 materialValue,
                      const Position * position, const MaterialInfo * mi)
{
   const int materialOpeningValue = getOpeningValue(materialValue);
   const int materialEndgameValue = getEndgameValue(materialValue);
   const int chancesWhite = getChances(position, mi, WHITE);
   const int chancesBlack = getChances(position, mi, BLACK);
   const int chances = max(chancesWhite, chancesBlack);
   const int openingValue = materialOpeningValue + getOpeningValue(value);
   const int positionalEndgameValue = getEndgameValue(value);
   const int endgameValue = (materialEndgameValue * chances) / 16;

   return (openingValue * (256 - mi->phaseIndex) +
           (endgameValue + positionalEndgameValue) * mi->phaseIndex) / 256;
}

INLINE INT32 materialBalance(const Position * position)
{
   const INT32 bishopPairBonus =
      V(VALUE_BISHOP_PAIR_OPENING, VALUE_BISHOP_PAIR_ENDGAME);
   static const INT32 knightBonus = V(0, 5);
   static const INT32 rookMalus = V(5, 0);
   static const INT32 rookPairMalus = V(16, 24);
   static const INT32 rookQueenMalus = V(8, 12);
   static const INT32 pieceUpBonus =
      V(DEFAULTVALUE_PIECE_UP_OPENING, DEFAULTVALUE_PIECE_UP_ENDGAME);

   INT32 balance = 0;
   const int pawnCountWhite = position->numberOfPawns[WHITE] - 5;
   const int pawnCountBlack = position->numberOfPawns[BLACK] - 5;
   const int numWhiteKnights = getPieceCount(position, WHITE_KNIGHT);
   const int numBlackKnights = getPieceCount(position, BLACK_KNIGHT);
   const int knightSaldo = pawnCountWhite * numWhiteKnights -
      pawnCountBlack * numBlackKnights;
   const int numWhiteRooks = getPieceCount(position, WHITE_ROOK);
   const int numBlackRooks = getPieceCount(position, BLACK_ROOK);
   const int rookSaldo = pawnCountWhite * numWhiteRooks -
      pawnCountBlack * numBlackRooks;
   const int pieceCountSaldo =
      (numberOfNonPawnPieces(position, WHITE) - numWhiteRooks -
       getPieceCount(position, WHITE_QUEEN)) -
      (numberOfNonPawnPieces(position, BLACK) - numBlackRooks -
       getPieceCount(position, BLACK_QUEEN));

   if (hasBishopPair(position, WHITE))
   {
      balance += bishopPairBonus;
   }

   if (hasBishopPair(position, BLACK))
   {
      balance -= bishopPairBonus;
   }

   balance += knightSaldo * knightBonus - rookSaldo * rookMalus;

   if (numWhiteRooks >= 2)
   {
      balance -= rookPairMalus + rookQueenMalus;
   }
   else if (numWhiteRooks + getPieceCount(position, WHITE_QUEEN) >= 2)
   {
      balance -= rookQueenMalus;
   }

   if (numBlackRooks >= 2)
   {
      balance += rookPairMalus + rookQueenMalus;
   }
   else if (numBlackRooks + getPieceCount(position, BLACK_QUEEN) >= 2)
   {
      balance += rookQueenMalus;
   }

   if (pieceCountSaldo > 0)
   {
      balance += pieceUpBonus;
   }
   else if (pieceCountSaldo < 0)
   {
      balance -= pieceUpBonus;
   }

   return balance;
}

/**
 * Calculate a rough value of the specified position,
 * based on the current pst-values and the specified evaluation base.
 *
 * @return the value of the specified position
 */
INLINE INT32 positionalBalance(const Position * position,
                               EvaluationBase * base)
{
   static const INT32 tempoBonus[2] = {
      V(VALUE_TEMPO_OPENING, VALUE_TEMPO_ENDGAME),
      V(-VALUE_TEMPO_OPENING, -VALUE_TEMPO_ENDGAME)
   };
   const INT32 balance = position->balance + base->balance +
      tempoBonus[position->activeColor];
   const int value = phaseValue(balance, base->materialBalance +
                                base->materialInfo->materialBalance,
                                position, base->materialInfo);

   return (position->activeColor == WHITE ? value : -value);
}

/**
 * Check if the specified color can win the specified position.
 *
 * @return FALSE if the specified color doesn't have sufficient material
 *         left to win the position
 */
INLINE bool hasWinningPotential(Position * position, Color color)
{
   return (bool) (position->numberOfPieces[color] > 1);
}

/**
 * Get the king safety hash value for the given king square.
 */
INLINE Bitboard getKingPawnSafetyHashValue(const Position * position,
                                           const Color color)
{
   const int mask[2] =
      { WHITE_00 | WHITE_000 | 16, BLACK_00 | BLACK_000 | 32 };
   const int index = (position->castlingRights | 48) & mask[color];

   return position->pawnHashValue ^
      GENERATED_KEYTABLE[WHITE][position->king[WHITE]] ^
      GENERATED_KEYTABLE[BLACK][position->king[BLACK]] ^
      GENERATED_KEYTABLE[2][index];
}

INLINE int getPawnWidth(const Position * position, const Color color)
{
   const Bitboard tmp = position->piecesOfType[(Piece) (PAWN | color)] |
      minValue[position->king[opponent(color)]];

   return getWidth(tmp);
}

INLINE int getPassedPawnWidth(const Position * position,
                              const EvaluationBase * base, const Color color)
{
   const Bitboard tmp = base->passedPawns[color] |
      minValue[position->king[opponent(color)]];

   return getWidth(tmp);
}

INLINE bool kpkpValueAvailable(const Position * position)
{
   return (bool) (position->numberOfPieces[WHITE] <= 2 &&
                  numberOfNonPawnPieces(position, WHITE) == 1 &&
                  position->numberOfPieces[BLACK] <= 2 &&
                  numberOfNonPawnPieces(position, BLACK) == 1 &&
                  position->numberOfPawns[WHITE] <= 1 &&
                  position->numberOfPawns[BLACK] <= 1 &&
                  position->enPassantSquare == NO_SQUARE);
}
