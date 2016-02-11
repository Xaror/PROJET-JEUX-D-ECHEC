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

INLINE Square relativeSquare(const Square square, const Color color)
{
   return (color == WHITE ? square : getFlippedSquare(square));
}

INLINE INT32 evalBonus(INT32 openingBonus, INT32 endgameBonus)
{
   return V(openingBonus, endgameBonus);
}

INLINE int getOpeningValue(INT32 value)
{
   return (int) ((INT16) (value & 0xFFFF));
}

INLINE int getEndgameValue(INT32 value)
{
   return (int) ((value + 0x8000) >> 16);
}

INLINE void addBonusForColor(const INT32 bonus, Position * position,
                             const Color color)
{
   if (color == WHITE)
   {
      position->balance += bonus;
   }
   else
   {
      position->balance -= bonus;
   }
}

/**
 * Pack the specified move into a 16-bit-uint.
 */
INLINE UINT16 packedMove(const Move move)
{
   return (UINT16) (move & 0xFFFF);
}

/**
 * Construct the specified move.
 */
INLINE Move getMove(const Square from, const Square to,
                    const Piece newPiece, const INT16 value)
{
   return (value << 16) | (newPiece << 12) | (to << 6) | from;
}

/**
 * Construct the specified ordinary move.
 */
INLINE Move getOrdinaryMove(const Square from, const Square to)
{
   return (to << 6) | from;
}

/**
 * Construct the specified packed move.
 */
INLINE Move getPackedMove(const Square from, const Square to,
                          const Piece newPiece)
{
   return (newPiece << 12) | (to << 6) | from;
}

/**
 * Get the from square of the specified move.
 */
INLINE Square getFromSquare(const Move move)
{
   return (Square) (move & 0x3F);
}

/**
 * Get the to square of the specified move.
 */
INLINE Square getToSquare(const Move move)
{
   return (Square) ((move >> 6) & 0x3F);
}

/**
 * Get the new piece of the specified move.
 */
INLINE Piece getNewPiece(const Move move)
{
   return (Piece) ((move >> 12) & 0x0F);
}

/**
 * Get the value of the specified move.
 */
INLINE INT16 getMoveValue(const Move move)
{
   return (INT16) (move >> 16);
}

/**
 * Set the value of the specified move.
 */
INLINE void setMoveValue(Move * move, const int value)
{
   *move = (*move & 0xFFFF) | (value << 16);
}

/**
 * Get the opponent color of the specified color.
 */
INLINE Color opponent(Color color)
{
   return (Color) (1 - color);
}

/**
 * Get the direct attackers of 'attackerColor' on 'square'.
 */
INLINE Bitboard getDirectAttackers(const Position * position,
                                   const Square square,
                                   const Color attackerColor,
                                   const Bitboard obstacles)
{
   const Bitboard king = getKingMoves(square) &
      minValue[position->king[attackerColor]];
   Bitboard dia = getMagicBishopMoves(square, obstacles);
   Bitboard ortho = getMagicRookMoves(square, obstacles);
   Bitboard knights = getKnightMoves(square);
   const Bitboard pawns =
      getPawnCaptures((Piece) (PAWN | opponent(attackerColor)),
                      square, position->piecesOfType[PAWN | attackerColor]);

   ortho &= (position->piecesOfType[QUEEN | attackerColor] |
             position->piecesOfType[ROOK | attackerColor]);
   dia &= (position->piecesOfType[QUEEN | attackerColor] |
           position->piecesOfType[BISHOP | attackerColor]);
   knights &= position->piecesOfType[KNIGHT | attackerColor];

   return king | ortho | dia | knights | pawns;
}

/**
 * Get the squares behind targetSquare, seen from 'viewPoint'.
 */
INLINE Bitboard getDiaSquaresBehind(const Position * position,
                                    const Square targetSquare,
                                    const Square viewPoint)
{
   return squaresBehind[targetSquare][viewPoint] &
      getMagicBishopMoves(targetSquare, position->allPieces);
}

/**
 * Get the squares behind targetSquare, seen from 'viewPoint'.
 */
INLINE Bitboard getOrthoSquaresBehind(const Position * position,
                                      const Square targetSquare,
                                      const Square viewPoint)
{
   return squaresBehind[targetSquare][viewPoint] &
      getMagicRookMoves(targetSquare, position->allPieces);
}

/**
 * Initialize the current plyInfo data structure.
 */
INLINE void initializePlyInfo(Variation * variation)
{
   const Position *position = &variation->singlePosition;
   PlyInfo *plyInfo = &variation->plyInfo[variation->ply];
   const Color activeColor = position->activeColor;

   plyInfo->pawnHashValue = position->pawnHashValue;
   plyInfo->enPassantSquare = position->enPassantSquare;
   plyInfo->kingSquare = position->king[activeColor];
   plyInfo->castlingRights = position->castlingRights;
   plyInfo->halfMoveClock = position->halfMoveClock;
   plyInfo->allPieces = position->allPieces;
   plyInfo->whitePieces = position->piecesOfColor[WHITE];
   plyInfo->blackPieces = position->piecesOfColor[BLACK];
   plyInfo->balance = 0;
}

/**
 * Get the number of non-pawn pieces for the specified color.
 */
INLINE int numberOfNonPawnPieces(const Position * position, const Color color)
{
   return position->numberOfPieces[color] - position->numberOfPawns[color];
}

/**
 * Check if the specified color has a rook or a queen.
 */
INLINE bool hasOrthoPieces(const Position * position, const Color color)
{
   return (bool) (position->piecesOfType[ROOK | color] != EMPTY_BITBOARD ||
                  position->piecesOfType[QUEEN | color] != EMPTY_BITBOARD);
}

/**
 * Check if the specified color has a queen.
 */
INLINE bool hasQueen(const Position * position, const Color color)
{
   return (bool) (position->piecesOfType[QUEEN | color] != EMPTY_BITBOARD);
}

/**
 * Append the given move to the old pv and copy the new pv to 'new'.
 */
INLINE void appendMoveToPv(const PrincipalVariation * oldPv,
                           PrincipalVariation * newPv, const Move move)
{
   newPv->move[0] = packedMove(move);
   newPv->length = oldPv->length + 1;
   memmove((void *) &newPv->move[1], (void *) &oldPv->move[0],
           oldPv->length * sizeof(UINT16));
}

/**
 * Calculate the value to be stored in the hashtable.
 */
INLINE INT16 calcHashtableValue(const int value, const int ply)
{
   if (value >= -VALUE_ALMOST_MATED)
   {
      return (INT16) (value + ply);
   }
   else if (value <= VALUE_ALMOST_MATED)
   {
      return (INT16) (value - ply);
   }

   return (INT16) value;
}

/**
 * Calculate the effective value from the specified hashtable value.
 */
INLINE int calcEffectiveValue(const int value, const int ply)
{
   if (value >= -VALUE_ALMOST_MATED)
   {
      return value - ply;
   }
   else if (value <= VALUE_ALMOST_MATED)
   {
      return value + ply;
   }

   return value;
}

/**
 * Get all ordinary pieces (queens, rooks, bishops, knights) 
 * of the specified color.
 */
INLINE Bitboard getOrdinaryPieces(const Position * position,
                                  const Color color)
{
   return position->piecesOfColor[color] &
      ~(position->piecesOfType[PAWN | color] |
        minValue[position->king[color]]);
}

/**
 * Get all non pawn pieces of the specified color.
 */
INLINE Bitboard getNonPawnPieces(const Position * position, const Color color)
{
   return position->piecesOfColor[color] &
      ~position->piecesOfType[PAWN | color];
}

/**
 * Get all ortho pieces (queens, rooks) 
 * of the specified color.
 */
INLINE Bitboard getOrthoPieces(const Position * position, const Color color)
{
   return position->piecesOfType[ROOK | color] |
      position->piecesOfType[QUEEN | color];
}

/**
 * Check if the given moves are equal by ignoring their respective values.
 */
INLINE bool movesAreEqual(const Move m1, const Move m2)
{
   return (bool) ((m1 & 0xFFFF) == (m2 & 0xFFFF));
}

/**
 * Get the population count for the specified piece in the specified position.
 */
INLINE int getPieceCount(const Position * position, const Piece piece)
{
   return (int) ((position->pieceCount >> pieceCountShift[piece]) & 0x0F);
}

/**
 * Check if the specified piece is present in the specified position.
 */
INLINE bool pieceIsPresent(const Position * position, const Piece piece)
{
   return (bool) (position->piecesOfType[piece] != EMPTY_BITBOARD);
}

/**
 * Get the history index of the specified move.
 */
INLINE int historyIndex(const Move move, const Position * position)
{
   const int npWhite = min(7, numberOfNonPawnPieces(position, WHITE) - 1);
   const int npBlack = min(7, numberOfNonPawnPieces(position, BLACK) - 1);
   const int nonPawnCountIndex = 64 * (npWhite + 8 * npBlack);

   assert(nonPawnCountIndex >= 0 && nonPawnCountIndex <= 63 * 64);

   return pieceIndex[position->piece[getFromSquare(move)]] +
      nonPawnCountIndex + getToSquare(move);
}

/**
 * Get a simple move index of the specified move.
 */
INLINE int moveIndex(const Move move, const Position * position)
{
   return (position->piece[getFromSquare(move)] << 6) + getToSquare(move);
}

/**
 * Get the move gain index of the specified move.
 */
INLINE int moveGainIndex(const Move move, const Position * position)
{
   return historyIndex(move, position);
}

/**
 * Calculate the distance to the next piece of a given type.
 *
 * @return the distance or 8 if no piece was found
 */
INLINE int getMinimalDistance(const Position * position,
                              const Square origin, const Piece piece)
{
   int distance;

   for (distance = 1; distance <= 7; distance++)
   {
      if ((squaresInDistance[distance][origin] &
           position->piecesOfType[piece]) != EMPTY_BITBOARD)
      {
         return distance;
      }
   }

   return 8;
}

/**
 * Calculate the distance to the next piece of a given type.
 *
 * @return the taxidistance or 15 if no piece was found
 */
INLINE int getMinimalTaxiDistance(const Position * position,
                                  const Square origin, const Piece piece)
{
   int distance;

   for (distance = 1; distance <= 14; distance++)
   {
      if ((squaresInTaxiDistance[distance][origin] &
           position->piecesOfType[piece]) != EMPTY_BITBOARD)
      {
         return distance;
      }
   }

   return 15;
}

/**
 * Calculate the weight of the non-pawn-pieces of the specified color.
 *
 * @return a value in the range [0-103]
 */
INLINE int getPieceWeight(const Position * position, const Color color)
{
   const int numNonPawnPieces = numberOfNonPawnPieces(position, color) - 1;
   const int numRooks = getPieceCount(position, (Piece) (ROOK | color));
   const int numQueens = getPieceCount(position, (Piece) (QUEEN | color));

   return 6 * numQueens + 2 * numRooks + 3 * numNonPawnPieces;  /* q=9, r=5, b,n=3 */
}

/**
 * Calculate the phase index of the specified position.
 *
 * @return a value in the range [0(initial position)-256(endgame)]
 */
INLINE int phaseIndex(const Position * position)
{
   const int weightWhite = getPieceWeight(position, WHITE);
   const int weightBlack = getPieceWeight(position, BLACK);
   const int basicPhase =
      (weightWhite + weightBlack <= PIECEWEIGHT_ENDGAME ?
       PHASE_MAX : PHASE_MAX - weightWhite - weightBlack);

   assert(getPieceWeight(position, WHITE) >= 0);
   assert(getPieceWeight(position, WHITE) <= 103);
   assert(getPieceWeight(position, BLACK) >= 0);
   assert(getPieceWeight(position, BLACK) <= 103);
   assert(basicPhase >= 0);
   assert(basicPhase <= PHASE_MAX);

   return (basicPhase * 256 + (PHASE_MAX / 2)) / PHASE_MAX;
}

/**
 * Get the piece counters from a material signature.
 */
INLINE void getPieceCounters(UINT32 materialSignature,
                             int *numWhiteQueens, int *numWhiteRooks,
                             int *numWhiteLightSquareBishops,
                             int *numWhiteDarkSquareBishops,
                             int *numWhiteKnights, int *numWhitePawns,
                             int *numBlackQueens, int *numBlackRooks,
                             int *numBlackLightSquareBishops,
                             int *numBlackDarkSquareBishops,
                             int *numBlackKnights, int *numBlackPawns)
{
   *numWhitePawns = materialSignature % 9;
   materialSignature /= 9;
   *numWhiteLightSquareBishops = materialSignature % 2;
   materialSignature /= 2;
   *numWhiteDarkSquareBishops = materialSignature % 2;
   materialSignature /= 2;
   *numWhiteQueens = materialSignature % 2;
   materialSignature /= 2;
   *numWhiteRooks = materialSignature % 3;
   materialSignature /= 3;
   *numWhiteKnights = materialSignature % 3;
   materialSignature /= 3;

   *numBlackPawns = materialSignature % 9;
   materialSignature /= 9;
   *numBlackLightSquareBishops = materialSignature % 2;
   materialSignature /= 2;
   *numBlackDarkSquareBishops = materialSignature % 2;
   materialSignature /= 2;
   *numBlackQueens = materialSignature % 2;
   materialSignature /= 2;
   *numBlackRooks = materialSignature % 3;
   materialSignature /= 3;
   *numBlackKnights = materialSignature % 3;
}

/**
 * Calculate a material signature from a white and a black signature.
 */
INLINE UINT32 bilateralSignature(const UINT32 signatureWhite,
                                 const UINT32 signatureBlack)
{
   return signatureWhite + 648 * signatureBlack;
}

/**
 * Get the piece counters from a material signature.
 */
INLINE UINT32 getMaterialSignature(const int numWhiteQueens,
                                   const int numWhiteRooks,
                                   const int numWhiteLightSquareBishops,
                                   const int numWhiteDarkSquareBishops,
                                   const int numWhiteKnights,
                                   const int numWhitePawns,
                                   const int numBlackQueens,
                                   const int numBlackRooks,
                                   const int numBlackLightSquareBishops,
                                   const int numBlackDarkSquareBishops,
                                   const int numBlackKnights,
                                   const int numBlackPawns)
{
   const int blackFactor = (3 * 3 * 2 * 2 * 2 * 9);

   return numWhitePawns +
      min(1, numWhiteLightSquareBishops) * 9 +
      min(1, numWhiteDarkSquareBishops) * (2 * 9) +
      min(1, numWhiteQueens) * (2 * 2 * 9) +
      min(2, numWhiteRooks) * (2 * 2 * 2 * 9) +
      min(2, numWhiteKnights) * (3 * 2 * 2 * 2 * 9) +
      numBlackPawns * blackFactor +
      min(1, numBlackLightSquareBishops) * 9 * blackFactor +
      min(1, numBlackDarkSquareBishops) * (2 * 9) * blackFactor +
      min(1, numBlackQueens) * (2 * 2 * 9) * blackFactor +
      min(2, numBlackRooks) * (2 * 2 * 2 * 9) * blackFactor +
      min(2, numBlackKnights) * (3 * 2 * 2 * 2 * 9) * blackFactor;
}

/**
 * Get the piece counters from a material signature.
 */
INLINE UINT32 getSingleMaterialSignature(const int numQueens,
                                         const int numRooks,
                                         const int numLightSquareBishops,
                                         const int numDarkSquareBishops,
                                         const int numKnights,
                                         const int numPawns)
{
   return getMaterialSignature(numQueens, numRooks,
                               numLightSquareBishops, numDarkSquareBishops,
                               numKnights, numPawns, 0, 0, 0, 0, 0, 0);
}

/**
 * Get the material signature for the specific position.
 */
INLINE UINT32 getMaterialSignatureFromPieceCount(const Position * position)
{
   return getMaterialSignature(getPieceCount(position, WHITE_QUEEN),
                               getPieceCount(position, WHITE_ROOK),
                               getPieceCount(position,
                                             (Piece) (WHITE_BISHOP_LIGHT)),
                               getPieceCount(position,
                                             (Piece) (WHITE_BISHOP_DARK)),
                               getPieceCount(position, WHITE_KNIGHT),
                               position->numberOfPawns[WHITE],
                               getPieceCount(position, BLACK_QUEEN),
                               getPieceCount(position, BLACK_ROOK),
                               getPieceCount(position,
                                             (Piece) (BLACK_BISHOP_LIGHT)),
                               getPieceCount(position,
                                             (Piece) (BLACK_BISHOP_DARK)),
                               getPieceCount(position, BLACK_KNIGHT),
                               position->numberOfPawns[BLACK]);
}

INLINE UINT32 calculateMaterialSignature(const Position * position)
{
   const UINT64 bbpWhite = ((position->pieceCount >> 8) & 0x0ff0) +
      position->numberOfPawns[WHITE];
   const UINT64 indexWhite = krqIndexWhite[position->pieceCount & 0x0Fff] +
      bbpIndexWhite[bbpWhite];
   const UINT64 bbpBlack = ((position->pieceCount >> 28) & 0x0ff0) +
      position->numberOfPawns[BLACK];
   const UINT64 indexBlack =
      krqIndexBlack[(position->pieceCount >> 20) & 0x0Fff] +
      bbpIndexBlack[bbpBlack];

   assert(bbpWhite >= 0 && bbpWhite < 4096);
   assert(bbpBlack >= 0 && bbpBlack < 4096);
   assert(indexWhite >= 0 && indexWhite < 648);
   assert(indexBlack >= 0 && indexBlack < 648 * 648);

   return (UINT32) (indexWhite + indexBlack);
}

/* Get the active color in the root position */
INLINE Color getOwnColor(Variation * variation)
{
   return ((variation->ply & 1) == 0 ?
           variation->singlePosition.activeColor :
           opponent(variation->singlePosition.activeColor));
}
