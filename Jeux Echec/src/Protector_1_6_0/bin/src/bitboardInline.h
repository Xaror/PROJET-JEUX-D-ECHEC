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

INLINE bool testSquare(const Bitboard bitboard, const Square square)
{
   return (bool) (((bitboard) & minValue[(square)]) != EMPTY_BITBOARD);
}

/**
 * Get the king moves for the specified square.
 */
INLINE Bitboard getKingMoves(const Square square)
{
   return generalMoves[KING][square];
}

/**
 * Get castling moves depending from the castling rights and 
 * the current position.
 */
INLINE Bitboard getCastlingMoves(const Color color, const BYTE castlingRights,
                                 const Bitboard obstacles)
{
   if (color == WHITE)
   {
      return castlings[WHITE][castlingRights][obstacles & 0xFF];
   }
   else
   {
      return castlings[BLACK][castlingRights][obstacles >> 56];
   }
}

/**
 * Get the queen moves for the specified square.
 */
/*
INLINE Bitboard getQueenMoves(const Square square, const BYTE * obstacles)
{
   const ObstacleSquareInfo *_obsi = &obsi[square];

   return _obsi->hLane[obstacles[_obsi->hLaneNumber]] |
      _obsi->vLane[obstacles[_obsi->vLaneNumber]] |
      _obsi->uLane[obstacles[_obsi->uLaneNumber]] |
      _obsi->dLane[obstacles[_obsi->dLaneNumber]];
}
*/

INLINE int getWidth(const Bitboard set)
{
   if (set == EMPTY_BITBOARD)
   {
      return 0;
   }
   else
   {
      File leftMost = FILE_A, rightMost = FILE_H;

      while ((set & squaresOfFile[leftMost]) == EMPTY_BITBOARD)
      {
         leftMost++;
      }

      while ((set & squaresOfFile[rightMost]) == EMPTY_BITBOARD)
      {
         rightMost--;
      }

      return rightMost - leftMost;
   }
}

/**
 * Get the queen moves for the specified square.
 */
INLINE Bitboard getMagicQueenMoves(const Square square,
                                   const Bitboard obstacles)
{
   MagicSquareInfoRook *msir = &magicSquareInfoRook[square];
   MagicSquareInfoBishop *msib = &magicSquareInfoBishop[square];

   return
      msir->moves[((obstacles & msir->preMask) * msir->magicNumber) >> 52] |
      msib->moves[((obstacles & msib->preMask) * msib->magicNumber) >> 55];
}

/**
 * Get the rook moves for the specified square.
 */
INLINE Bitboard getRookMoves(const Square square, const BYTE * obstacles)
{
   const ObstacleSquareInfo *_obsi = &obsi[square];

   return _obsi->hLane[obstacles[_obsi->hLaneNumber]] |
      _obsi->vLane[obstacles[_obsi->vLaneNumber]];
}

/**
 * Get the rook moves for the specified square.
 */
INLINE Bitboard getMagicRookMoves(const Square square,
                                  const Bitboard obstacles)
{
   MagicSquareInfoRook *msir = &magicSquareInfoRook[square];

   return
      msir->moves[((obstacles & msir->preMask) * msir->magicNumber) >> 52];
}

/**
 * Get the bishop moves for the specified square.
 */
INLINE Bitboard getBishopMoves(const Square square, const BYTE * obstacles)
{
   const ObstacleSquareInfo *_obsi = &obsi[square];

   return _obsi->uLane[obstacles[_obsi->uLaneNumber]] |
      _obsi->dLane[obstacles[_obsi->dLaneNumber]];
}

/**
 * Get the bishop moves for the specified square.
 */
INLINE Bitboard getMagicBishopMoves(const Square square,
                                    const Bitboard obstacles)
{
   MagicSquareInfoBishop *msib = &magicSquareInfoBishop[square];

   return
      msib->moves[((obstacles & msib->preMask) * msib->magicNumber) >> 55];
}

/**
 * Get the knight moves for the specified square.
 */
INLINE Bitboard getKnightMoves(const Square square)
{
   return generalMoves[KNIGHT][square];
}

/**
 * Get the pawn captures for the specified square.
 */
INLINE Bitboard getPawnCaptures(const Piece piece, const Square square,
                                const Bitboard allPieces)
{
   return generalMoves[piece][square] & allPieces;
}

/**
 * Get the pawn advances for the specified square.
 */
INLINE Bitboard getPawnAdvances(const Color color, const Square square,
                                const Bitboard obstacles)
{
   Bitboard advances;

   if (color == WHITE)
   {
      advances = (minValue[square] << 8) & ~obstacles;

      if (rank(square) == RANK_2)
      {
         advances |= (advances << 8) & ~obstacles;
      }
   }
   else
   {
      advances = (minValue[square] >> 8) & ~obstacles;

      if (rank(square) == RANK_7)
      {
         advances |= (advances >> 8) & ~obstacles;
      }
   }

   return advances;
}

/**
 * Get the pawns interested in advancing to the specified square.
 */
INLINE Bitboard getInterestedPawns(const Color color, const Square square,
                                   const Bitboard obstacles)
{
   Bitboard interestedPawns;

   if (color == WHITE)
   {
      interestedPawns = minValue[square] >> 8;

      if (rank(square) == RANK_4 &&
          (interestedPawns & obstacles) == EMPTY_BITBOARD)
      {
         interestedPawns = minValue[square] >> 16;
      }
   }
   else
   {
      interestedPawns = minValue[square] << 8;

      if (rank(square) == RANK_5 &&
          (interestedPawns & obstacles) == EMPTY_BITBOARD)
      {
         interestedPawns = minValue[square] << 16;
      }
   }

   return interestedPawns;
}

/**
 * Get the squares between the two specified squares.
 */
INLINE Bitboard getSquaresBetween(const Square square1, const Square square2)
{
   return squaresBetween[square1][square2];
}

/**
 * Get the squares behind 'target', as seen from 'viewpoint'.
 */
INLINE Bitboard getSquaresBehind(const Square target, const Square viewpoint)
{
   return squaresBehind[target][viewpoint];
}

/**
 * Shift all set squares one file to the left.
 */
INLINE Bitboard shiftLeft(const Bitboard bitboard)
{
   return (bitboard & nonA) >> 1;
}

/**
 * Shift all set squares one file to the right.
 */
INLINE Bitboard shiftRight(const Bitboard bitboard)
{
   return (bitboard & nonH) << 1;
}

/**
 * Get all squares lateral to the specified squares.
 */
INLINE Bitboard getLateralSquares(const Bitboard squares)
{
   return ((squares & nonA) >> 1) | ((squares & nonH) << 1);
}

/**
 * Get the squares of the specified file.
 */
INLINE Bitboard getSquaresOfFile(const File file)
{
   return squaresOfFile[file];
}

/**
 * Get the squares of the specified rank.
 */
INLINE Bitboard getSquaresOfRank(const Rank rank)
{
   return squaresOfRank[rank];
}

/**
 * Get the number of set squares in the specified bitboard.
 */
INLINE int getNumberOfSetSquares(const Bitboard bitboard)
{
   return numSetBits[bitboard & UHEX_FFFF] +
      numSetBits[(bitboard >> 16) & UHEX_FFFF] +
      numSetBits[(bitboard >> 32) & UHEX_FFFF] +
      numSetBits[(bitboard >> 48) & UHEX_FFFF];
}

/**
 * Get the rank overlay of the specified bitboard.
 */
INLINE int getRankOverlay(const Bitboard bitboard)
{
   return rankOverlay[(bitboard) & UHEX_FFFF] |
      rankOverlay[(bitboard >> 16) & UHEX_FFFF] |
      rankOverlay[(bitboard >> 32) & UHEX_FFFF] |
      rankOverlay[(bitboard >> 48) & UHEX_FFFF];
}

/**
 * Get the moves of the the specified piece.
 */
INLINE Bitboard getMoves(Square square, Piece piece, Bitboard allPieces)
{
   switch (pieceType(piece))
   {
   case PAWN:
      return getPawnCaptures(piece, square, allPieces) |
         getPawnAdvances(pieceColor(piece), square, allPieces);
   case KING:
      return getKingMoves(square);
   case KNIGHT:
      return getKnightMoves(square);
   case ROOK:
      return getMagicRookMoves(square, allPieces);
   case BISHOP:
      return getMagicBishopMoves(square, allPieces);
   default:
      return getMagicQueenMoves(square, allPieces);
   }
}

/**
 * Get the capture moves of the the specified piece.
 */
INLINE Bitboard getCaptureMoves(Square square, Piece piece,
                                Bitboard allPieces)
{
   switch (pieceType(piece))
   {
   case PAWN:
      return getPawnCaptures(piece, square, allPieces);
   case KING:
      return getKingMoves(square);
   case KNIGHT:
      return getKnightMoves(square);
   case ROOK:
      return getMagicRookMoves(square, allPieces);
   case BISHOP:
      return getMagicBishopMoves(square, allPieces);
   default:
      return getMagicQueenMoves(square, allPieces);
   }
}

/**
 * Set a square in the specified vector of obstacles.
 */
INLINE void setObstacleSquare(Square square, BYTE obstacles[NUM_LANES])
{
   SquareLaneInfo *sqi = &squareLaneInfo[square];

   obstacles[sqi->hLane] |= sqi->hLaneSetMask;
   obstacles[sqi->vLane] |= sqi->vLaneSetMask;
   obstacles[sqi->uLane] |= sqi->uLaneSetMask;
   obstacles[sqi->dLane] |= sqi->dLaneSetMask;
}

/**
 * Clear a square in the specified vector of obstacles.
 */
INLINE void clearObstacleSquare(Square square, BYTE obstacles[NUM_LANES])
{
   SquareLaneInfo *sqi = &squareLaneInfo[square];

   obstacles[sqi->hLane] &= sqi->hLaneClearMask;
   obstacles[sqi->vLane] &= sqi->vLaneClearMask;
   obstacles[sqi->uLane] &= sqi->uLaneClearMask;
   obstacles[sqi->dLane] &= sqi->dLaneClearMask;
}

/**
 * Calculate all obstacles according to the specified bitboard.
 */
INLINE void calculateObstacles(Bitboard board, BYTE obstacles[NUM_LANES])
{
   Square square;

   memset(obstacles, 0x00, NUM_LANES);

   ITERATE(square)
   {
      if (board & minValue[square])
      {
         setObstacleSquare(square, obstacles);
      }
   }
}

/**
 * Get the number of a set square and clear the corresponding bit.
 *
 * @return NO_SQUARE if no square was set.
 */
INLINE Square getLastSquare(Bitboard * vector);

/**
 * Extend all set bits by one square into all directions.
 */
INLINE void floodBoard(Bitboard * board);

/**
 * Get the targets of all pawns specified by 'whitePawns'.
 */
INLINE Bitboard getWhitePawnTargets(const Bitboard whitePawns)
{
   return ((whitePawns & nonA) << 7) | ((whitePawns & nonH) << 9);
}

/**
 * Get the targets of all pawns specified by 'blackPawns'.
 */
INLINE Bitboard getBlackPawnTargets(const Bitboard blackPawns)
{
   return ((blackPawns & nonA) >> 9) | ((blackPawns & nonH) >> 7);
}

/**
 * Iteration shortcuts
 */
#define ITERATE_BITBOARD(b,sq) while ( ( sq = getLastSquare(b) ) >= 0 )

INLINE void floodBoard(Bitboard * board)
{
   const Bitboard toLeft = *board & nonA, toRight = *board & nonH;

   *board = (toLeft >> 1) | (toLeft << 7) | (*board >> 8) | (toLeft >> 9) |
      (toRight << 1) | (toRight >> 7) | (*board << 8) | (toRight << 9);
}

#ifdef WIN64
INLINE Square getLastSquare(Bitboard * vector)
{
   unsigned long index;

   if (_BitScanReverse64(&index, *vector))
   {
      _bittestandreset64((__int64 *) vector, index);

      return (Square) index;
   }
   else
   {
      return NO_SQUARE;
   }
}

INLINE Square getFirstSquare(Bitboard * vector)
{
   unsigned long index;

   if (_BitScanForward64(&index, *vector))
   {
      _bittestandreset64((__int64 *) vector, index);

      return (Square) index;
   }
   else
   {
      return NO_SQUARE;
   }
}
#else
#if !defined NDEBUG || defined TARGET_LINUX
INLINE Square getLastSquare(Bitboard * vector)
{
   Square nextSquare;

   if ((*vector & UHEX_FFFFffff00000000) != ULONG_ZERO)
   {
      if ((*vector & UHEX_FFFF000000000000) != ULONG_ZERO)
      {
         nextSquare = (Square) (highestBit[(int) (*vector >> 48)] + 48);
         *vector &= maxValue[nextSquare];

         return nextSquare;
      }
      else
      {
         nextSquare = (Square) (highestBit[(int) (*vector >> 32)] + 32);
         *vector &= maxValue[nextSquare];

         return nextSquare;
      }
   }
   else
   {
      if ((*vector & UHEX_00000000FFFF0000) != ULONG_ZERO)
      {
         nextSquare = (Square) (highestBit[(int) (*vector >> 16)] + 16);
         *vector &= maxValue[nextSquare];

         return nextSquare;
      }
      else
      {
         if ((nextSquare = (Square) highestBit[(int) *vector]) >= 0)
         {
            *vector &= maxValue[nextSquare];
         }

         return nextSquare;
      }
   }
}

INLINE Square getFirstSquare(Bitboard * vector)
{
   Square nextSquare;

   if ((*vector & UHEX_00000000FFFFffff) != ULONG_ZERO)
   {
      if ((*vector & UHEX_000000000000ffff) != ULONG_ZERO)
      {
         nextSquare = (Square) lowestBit[(int) (*vector & 0xffff)];
         *vector &= maxValue[nextSquare];

         return nextSquare;
      }
      else
      {
         nextSquare =
            (Square) (lowestBit[(int) ((*vector >> 16) & 0xffff)] + 16);
         *vector &= maxValue[nextSquare];

         return nextSquare;
      }
   }
   else
   {
      if ((*vector & UHEX_0000ffff00000000) != ULONG_ZERO)
      {
         nextSquare =
            (Square) (lowestBit[(int) ((*vector >> 32) & 0xffff)] + 32);
         *vector &= maxValue[nextSquare];

         return nextSquare;
      }
      else
      {
         nextSquare = (Square) (lowestBit[(int) (*vector >> 48)]);

         if (nextSquare >= 0)
         {
            nextSquare = (Square) (nextSquare + 48);
            *vector &= maxValue[nextSquare];

            return nextSquare;
         }
         else
         {
            return NO_SQUARE;
         }
      }
   }
}
#else
INLINE Square getLastSquare(Bitboard * vector)
{
   unsigned long index;

   if (*vector > UHEX_00000000FFFFffff)
   {
      unsigned long *uw = ((unsigned long *) (vector)) + 1;

      _BitScanReverse(&index, *uw);
      _bittestandreset((long *) uw, index);

      return (Square) index + 32;
   }
   else
   {
      if (_BitScanReverse(&index, (unsigned long) *vector))
      {
         _bittestandreset((long *) vector, index);

         return (Square) index;
      }
      else
      {
         return NO_SQUARE;
      }
   }
}

INLINE Square getFirstSquare(Bitboard * vector)
{
   unsigned long index;

   if (*vector & UHEX_00000000FFFFffff)
   {
      _BitScanForward(&index, (unsigned long) *vector);
      _bittestandreset((long *) vector, index);

      return (Square) index;
   }
   else
   {
      unsigned long *uw = ((unsigned long *) (vector)) + 1;

      if (_BitScanForward(&index, (unsigned long) *uw))
      {
         _bittestandreset((long *) uw, index);

         return (Square) index + 32;
      }
      else
      {
         return NO_SQUARE;
      }
   }
}
#endif
#endif

INLINE int getSetSquares(const Bitboard board, UINT8 squares[_64_])
{
   int index;
   UINT8 *currentSquare = &squares[0];

   if ((index = (int) (0xFFFF & board)) != 0)
   {
      const SetSquaresInfo *info = &setSquares[0][index];

      memcpy(currentSquare, info->setSquares, info->numSetSquares);
      currentSquare += info->numSetSquares;
   }

   if ((index = (int) (0xFFFF & (board >> 16))) != 0)
   {
      const SetSquaresInfo *info = &setSquares[1][index];

      memcpy(currentSquare, info->setSquares, info->numSetSquares);
      currentSquare += info->numSetSquares;
   }

   if ((index = (int) (0xFFFF & (board >> 32))) != 0)
   {
      const SetSquaresInfo *info = &setSquares[2][index];

      memcpy(currentSquare, info->setSquares, info->numSetSquares);
      currentSquare += info->numSetSquares;
   }

   if ((index = (int) (board >> 48)) != 0)
   {
      const SetSquaresInfo *info = &setSquares[3][index];

      memcpy(currentSquare, info->setSquares, info->numSetSquares);
      currentSquare += info->numSetSquares;
   }

   return (int) (currentSquare - &squares[0]);
}

INLINE Bitboard getMultipleSquaresBetween(const Square origin,
                                          Bitboard targets)
{
   Bitboard squares = targets;
   Square targetSquare;
   Bitboard *sqb = &(squaresBetween[origin][0]);

   ITERATE_BITBOARD(&targets, targetSquare)
   {
      squares |= sqb[targetSquare];
   }

   return squares;
}
