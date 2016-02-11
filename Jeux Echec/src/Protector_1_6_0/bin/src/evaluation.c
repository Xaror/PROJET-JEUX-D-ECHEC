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

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "position.h"
#include "fen.h"
#include "io.h"
#ifdef INCLUDE_TABLEBASE_ACCESS
#include "tablebase.h"
#endif

#define dumpPos dumpPosition(position);

/* #define GENERATE_TABLES */

#define BONUS_ROOK_OUTPOST
#define MALUS_KING_PAWN_DISTANCE */

#include "evaluation.h"

KingSafetyHashInfo
   kingSafetyHashtable[MAX_THREADS][KINGSAFETY_HASHTABLE_SIZE];

#define MAX_MOVES_KNIGHT 8
#define MAX_MOVES_BISHOP 13
#define MAX_MOVES_ROOK 14
#define MAX_MOVES_QUEEN 27

INT32 KnightMobilityBonus[MAX_MOVES_KNIGHT + 1];
INT32 BishopMobilityBonus[MAX_MOVES_BISHOP + 1];
INT32 RookMobilityBonus[MAX_MOVES_ROOK + 1];
INT32 QueenMobilityBonus[MAX_MOVES_QUEEN + 1];

/* *INDENT-OFF* */

const INT32 PstPawn[64] = {
V(  0,  0), V(  0,  0), V(  0,  0), V(  0,  0), V(  0,  0), V(  0,  0), V(  0,  0), V(  0,  0),  /* rank 1 */
V(-10, -3), V( -2, -3), V(  1, -3), V(  5, -3), V(  5, -3), V(  1, -3), V( -2, -3), V(-10, -3),  /* rank 2 */
V(-10, -3), V( -2, -3), V(  3, -3), V( 14, -3), V( 14, -3), V(  3, -3), V( -2, -3), V(-10, -3),  /* rank 3 */
V(-10, -3), V( -2, -3), V(  6, -3), V( 22, -3), V( 22, -3), V(  6, -3), V( -2, -3), V(-10, -3),  /* rank 4 */
V(-10, -3), V( -2, -3), V(  6, -3), V( 14, -3), V( 14, -3), V(  6, -3), V( -2, -3), V(-10, -3),  /* rank 5 */
V(-10, -3), V( -2, -3), V(  3, -3), V(  5, -3), V(  5, -3), V(  3, -3), V( -2, -3), V(-10, -3),  /* rank 6 */
V(-10, -3), V( -2, -3), V(  1, -3), V(  5, -3), V(  5, -3), V(  1, -3), V( -2, -3), V(-10, -3),  /* rank 7 */
V(  0,  0), V(  0,  0), V(  0,  0), V(  0,  0), V(  0,  0), V(  0,  0), V(  0,  0), V(  0,  0)}; /* rank 8 */

const INT32 PstKnight[64] = {
V(-52,-40), V(-41,-30), V(-31,-21), V(-26,-16), V(-26,-16), V(-31,-21), V(-41,-30), V(-52,-40),  /* rank 1 */
V(-36,-30), V(-26,-21), V(-15,-11), V( -9, -6), V( -9, -6), V(-15,-11), V(-26,-21), V(-36,-30),  /* rank 2 */
V(-20,-21), V( -9,-11), V(  0, -2), V(  5,  1), V(  5,  1), V(  0, -2), V( -9,-11), V(-20,-21),  /* rank 3 */
V( -9,-16), V(  0, -6), V( 10,  1), V( 16,  7), V( 16,  7), V( 10,  1), V(  0, -6), V( -9,-16),  /* rank 4 */
V( -4,-16), V(  5, -6), V( 16,  1), V( 21,  7), V( 21,  7), V( 16,  1), V(  5, -6), V( -4,-16),  /* rank 5 */
V( -4,-21), V(  5,-11), V( 16, -2), V( 21,  1), V( 21,  1), V( 16, -2), V(  5,-11), V( -4,-21),  /* rank 6 */
V(-20,-30), V( -9,-21), V(  0,-11), V(  5, -6), V(  5, -6), V(  0,-11), V( -9,-21), V(-20,-30),  /* rank 7 */
V(-75,-40), V(-26,-30), V(-15,-21), V( -9,-16), V( -9,-16), V(-15,-21), V(-26,-30), V(-75,-40)}; /* rank 8 */

const INT32 PstBishop[64] = {
V(-15,-23), V(-15,-16), V(-13,-13), V(-11,-10), V(-11,-10), V(-13,-13), V(-15,-16), V(-15,-23),  /* rank 1 */
V( -6,-16), V(  0,-10), V( -1, -7), V(  0, -4), V(  0, -4), V( -1, -7), V(  0,-10), V( -6,-16),  /* rank 2 */
V( -5,-13), V( -1, -7), V(  3, -4), V(  1, -1), V(  1, -1), V(  3, -4), V( -1, -7), V( -5,-13),  /* rank 3 */
V( -3,-10), V(  0, -4), V(  1, -1), V(  6,  1), V(  6,  1), V(  1, -1), V(  0, -4), V( -3,-10),  /* rank 4 */
V( -3,-10), V(  0, -4), V(  1, -1), V(  6,  1), V(  6,  1), V(  1, -1), V(  0, -4), V( -3,-10),  /* rank 5 */
V( -5,-13), V( -1, -7), V(  3, -4), V(  1, -1), V(  1, -1), V(  3, -4), V( -1, -7), V( -5,-13),  /* rank 6 */
V( -6,-16), V(  0,-10), V( -1, -7), V(  0, -4), V(  0, -4), V( -1, -7), V(  0,-10), V( -6,-16),  /* rank 7 */
V( -6,-23), V( -6,-16), V( -5,-13), V( -3,-10), V( -3,-10), V( -5,-13), V( -6,-16), V( -6,-23)}; /* rank 8 */

const INT32 PstRook[64] = {
V( -4,  1), V( -2,  1), V(  0,  1), V(  0,  1), V(  0,  1), V(  0,  1), V( -2,  1), V( -4,  1),  /* rank 1 */
V( -4,  1), V( -2,  1), V(  0,  1), V(  0,  1), V(  0,  1), V(  0,  1), V( -2,  1), V( -4,  1),  /* rank 2 */
V( -4,  1), V( -2,  1), V(  0,  1), V(  0,  1), V(  0,  1), V(  0,  1), V( -2,  1), V( -4,  1),  /* rank 3 */
V( -4,  1), V( -2,  1), V(  0,  1), V(  0,  1), V(  0,  1), V(  0,  1), V( -2,  1), V( -4,  1),  /* rank 4 */
V( -4,  1), V( -2,  1), V(  0,  1), V(  0,  1), V(  0,  1), V(  0,  1), V( -2,  1), V( -4,  1),  /* rank 5 */
V( -4,  1), V( -2,  1), V(  0,  1), V(  0,  1), V(  0,  1), V(  0,  1), V( -2,  1), V( -4,  1),  /* rank 6 */
V( -4,  1), V( -2,  1), V(  0,  1), V(  0,  1), V(  0,  1), V(  0,  1), V( -2,  1), V( -4,  1),  /* rank 7 */
V( -4,  1), V( -2,  1), V(  0,  1), V(  0,  1), V(  0,  1), V(  0,  1), V( -2,  1), V( -4,  1)}; /* rank 8 */

const INT32 PstQueen[64] = {
V(  3,-31), V(  3,-21), V(  3,-16), V(  3,-11), V(  3,-11), V(  3,-16), V(  3,-21), V(  3,-31),  /* rank 1 */
V(  3,-21), V(  3,-11), V(  3, -7), V(  3, -2), V(  3, -2), V(  3, -7), V(  3,-11), V(  3,-21),  /* rank 2 */
V(  3,-16), V(  3, -7), V(  3, -2), V(  3,  2), V(  3,  2), V(  3, -2), V(  3, -7), V(  3,-16),  /* rank 3 */
V(  3,-11), V(  3, -2), V(  3,  2), V(  3,  7), V(  3,  7), V(  3,  2), V(  3, -2), V(  3,-11),  /* rank 4 */
V(  3,-11), V(  3, -2), V(  3,  2), V(  3,  7), V(  3,  7), V(  3,  2), V(  3, -2), V(  3,-11),  /* rank 5 */
V(  3,-16), V(  3, -7), V(  3, -2), V(  3,  2), V(  3,  2), V(  3, -2), V(  3, -7), V(  3,-16),  /* rank 6 */
V(  3,-21), V(  3,-11), V(  3, -7), V(  3, -2), V(  3, -2), V(  3, -7), V(  3,-11), V(  3,-21),  /* rank 7 */
V(  3,-31), V(  3,-21), V(  3,-16), V(  3,-11), V(  3,-11), V(  3,-16), V(  3,-21), V(  3,-31)}; /* rank 8 */

const INT32 PstKing[64] = {
V(112,  7), V(121, 30), V(102, 41), V( 83, 52), V( 83, 52), V(102, 41), V(121, 30), V(112,  7),  /* rank 1 */
V(102, 30), V(112, 52), V( 92, 64), V( 74, 75), V( 74, 75), V( 92, 64), V(112, 52), V(102, 30),  /* rank 2 */
V( 83, 41), V( 92, 64), V( 74, 75), V( 55, 86), V( 55, 86), V( 74, 75), V( 92, 64), V( 83, 41),  /* rank 3 */
V( 74, 52), V( 83, 75), V( 65, 86), V( 46, 98), V( 46, 98), V( 65, 86), V( 83, 75), V( 74, 52),  /* rank 4 */
V( 65, 52), V( 74, 75), V( 55, 86), V( 36, 98), V( 36, 98), V( 55, 86), V( 74, 75), V( 65, 52),  /* rank 5 */
V( 55, 41), V( 65, 64), V( 46, 75), V( 26, 86), V( 26, 86), V( 46, 75), V( 65, 64), V( 55, 41),  /* rank 6 */
V( 46, 30), V( 55, 52), V( 36, 64), V( 17, 75), V( 17, 75), V( 36, 64), V( 55, 52), V( 46, 30),  /* rank 7 */
V( 36,  7), V( 46, 30), V( 26, 41), V(  8, 52), V(  8, 52), V( 26, 41), V( 46, 30), V( 36,  7)}; /* rank 8 */

/* *INDENT-ON* */

/* -------------------------------------------------------------------------- */

static const int KNIGHT_BONUS_ATTACK = 2;
static const int BISHOP_BONUS_ATTACK = 2;
static const int BISHOP_MALUS_BLOCKED = 50;
static const int BISHOP_MALUS_TRAPPED = 125;
static const int ROOK_BONUS_ATTACK = 3;
static const int QUEEN_BONUS_ATTACK = 5;

INT32 mvImpact[16];

static const int KINGSAFETY_PAWN_MALUS_DEFENDER[3][8] = {
   {30, 0, 5, 15, 20, 25, 25, 25},      /* towards nearest border */
   {55, 0, 15, 40, 50, 55, 55, 55},
   {30, 0, 10, 20, 25, 30, 30, 30}      /* towards center */
};

static const int KINGSAFETY_PAWN_BONUS_DEFENDER_DIAG[4][8] = {
   {10, 0, 2, 4, 6, 8, 10, 10},
   {8, 0, 2, 4, 6, 7, 8, 8},
   {6, 0, 2, 3, 4, 5, 6, 6},
   {4, 0, 1, 2, 3, 4, 4, 4}
};

static const int KINGSAFETY_PAWN_BONUS_ATTACKER[3][8] = {
   {5, 0, 40, 15, 5, 0, 0, 0},  /* towards nearest border */
   {10, 0, 50, 20, 10, 0, 0, 0},
   {10, 0, 50, 20, 10, 0, 0, 0} /* towards center */
};

#define OWN_COLOR_WEIGHT_DIV 16
#define OWN_COLOR_WEIGHT_KINGSAFETY 18

#define PASSED_PAWN_WEIGHT_OP 116
#define PASSED_PAWN_WEIGHT_EG 126
#define KS_EXP 2.0
#define KS_WEIGHT 124
#define KS_PAWN_STRUCTURE_DIV 16
#define KS_PAWNSTRUCTURE_WEIGHT 57
#define HOMELAND_SECURITY_WEIGHT 21
#define KING_SAFETY_MALUS_DIM (100*KS_PAWN_STRUCTURE_DIV)

int KING_SAFETY_MALUS[KING_SAFETY_MALUS_DIM];

#define KPKP_TABLESIZE (2*32*49*49)

/* -------------------------------------------------------------------------- */

int centerDistance[_64_], centerTaxiDistance[_64_];
int attackPoints[16];
Bitboard butterflySquares[_64_];
Bitboard lateralSquares[_64_];
Bitboard companionFiles[_64_];
Bitboard passedPawnRectangle[2][_64_];
Bitboard passedPawnCorridor[2][_64_];
Bitboard candidateDefenders[2][_64_];   /* excludes squares of rank */
Bitboard candidateSupporters[2][_64_];  /* includes squares of rank */
Bitboard pawnOpponents[2][_64_];
Bitboard kingTrapsRook[2][64];
Bitboard rookBlockers[_64_];
Bitboard centralFiles;
Bitboard kingRealm[2][_64_][_64_];
Bitboard attackingRealm[2];
Bitboard homeland[2];
Bitboard troitzkyArea[2];
Bitboard krprkDrawFiles;
Bitboard A1C1, F1H1, A1B1, G1H1;
Bitboard filesBCFG;
KingAttacks kingAttacks[_64_];
int kingChaseMalus[3][_64_];
INT32 piecePieceAttackBonus[16][16];
MaterialInfo materialInfo[MATERIALINFO_TABLE_SIZE];

/* *INDENT-OFF* */
static const int MALUS_PAWN_SQUARE_TYPE_HR[64] = {
   0, 0, 0, 0, 0, 0, 0, 0,
   1, 1, 2, 2, 2, 2, 1, 1,
   1, 2, 3, 3, 3, 3, 2, 1,
   1, 2, 3, 5, 5, 3, 2, 1,
   1, 2, 3, 5, 5, 3, 2, 1,
   1, 2, 3, 3, 3, 3, 2, 1,
   1, 1, 2, 2, 2, 2, 1, 1,
   0, 0, 0, 0, 0, 0, 0, 0
};

static const int MALUS_KING_SQUARE_HR[64] = {
   15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15,
   15, 15, 15, 15, 15, 15, 15, 15,
    7, 10, 12, 12, 12, 12, 10,  7,
    2,  2,  4,  8,  8,  4,  2,  2,
    2,  0,  2,  5,  5,  2,  0,  2,
};

static const int BONUS_KNIGHT_OUTPOST_HR[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  2,  7,  7,  7,  7,  2,  0,
    0,  3, 10, 14, 14, 10,  3,  0,
    0,  2,  7, 10, 10,  7,  2,  0,
    0,  0,  2,  3,  3,  2,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
};

static const int BONUS_BISHOP_OUTPOST_HR[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  2,  3,  3,  3,  3,  2,  0,
    0,  4,  8,  8,  8,  8,  4,  0,
    0,  2,  4,  4,  4,  4,  2,  0,
    0,  0,  2,  2,  2,  2,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,
};
/* *INDENT-ON* */

static int MALUS_PAWN_SQUARE_TYPE[64];
static int MALUS_KING_SQUARE[64];
static int BONUS_KNIGHT_OUTPOST[64];
static int BONUS_BISHOP_OUTPOST[64];

#ifdef GENERATE_TABLES
UINT64 kpkpTempTable[KPKP_TABLESIZE];
#endif

#ifndef NDEBUG
bool debugEval = FALSE;
#endif

#ifndef INLINE_IN_HEADERS
#include "evaluationInline.h"
#endif

static int getWhiteKingTableIndex(const Square whiteKingSquare)
{
   const File fileCount = file(whiteKingSquare);
   const Rank rankCount = rank(whiteKingSquare);

   return 4 * rankCount + fileCount;
}

static INLINE unsigned int getKpkpIndexFromSquares(const Square wk,
                                                   const Square bk,
                                                   const Square wp,
                                                   const Square bp)
{
   const unsigned int M1 = 2;
   const unsigned int M2 = 32 * M1;
   const unsigned int M3 = 64 * M2;
   const unsigned int M4 = 49 * M3;
   unsigned int index;

   index = getWhiteKingTableIndex(wk) * M1 + (bk * M2) +
      ((wp - A2) * M3) + ((bp - A2) * M4);

   assert((wp - A2) >= 0);
   assert((bp - A2) >= 0);
   assert((wp - A2) < 49);
   assert((bp - A2) < 49);
   assert(index < (KPKP_TABLESIZE << 6));

   return index;
}

static INLINE unsigned int getKpkpIndex(const Position * position)
{
   Square wk = position->king[WHITE];
   Square bk = position->king[BLACK];
   Square wp = A8, bp = A8;

   if (position->piecesOfType[WHITE_PAWN] != EMPTY_BITBOARD)
   {
      Bitboard whitePawns = position->piecesOfType[WHITE_PAWN];

      wp = getLastSquare(&whitePawns);
   }

   if (position->piecesOfType[BLACK_PAWN] != EMPTY_BITBOARD)
   {
      Bitboard blackPawns = position->piecesOfType[BLACK_PAWN];

      bp = getLastSquare(&blackPawns);
   }

   if (position->activeColor == BLACK)
   {
      const Square tmpWk = wk;
      const Square tmpWp = wp;

#ifndef NDEBUG
      if (debugEval)
      {
         logDebug("before vertical flip\n");
         logDebug("wk=");
         dumpSquare(wk);
         logDebug("bk=");
         dumpSquare(bk);
         logDebug("wp=");
         dumpSquare(wp);
         logDebug("bp=");
         dumpSquare(bp);
         logPosition(position);
      }
#endif

      wk = getFlippedSquare(bk);
      bk = getFlippedSquare(tmpWk);
      wp = (bp == A8 ? bp : getFlippedSquare(bp));
      bp = (tmpWp == A8 ? tmpWp : getFlippedSquare(tmpWp));

#ifndef NDEBUG
      if (debugEval)
      {
         logDebug("after vertical flip\n");
         logDebug("wk=");
         dumpSquare(wk);
         logDebug("bk=");
         dumpSquare(bk);
         logDebug("wp=");
         dumpSquare(wp);
         logDebug("bp=");
         dumpSquare(bp);
         logPosition(position);
      }
#endif
   }

   if (file(wk) >= FILE_E)
   {
#ifndef NDEBUG
      if (debugEval)
      {
         logDebug("before horizontal flip\n");
         logDebug("wk=");
         dumpSquare(wk);
         logDebug("bk=");
         dumpSquare(bk);
         logDebug("wp=");
         dumpSquare(wp);
         logDebug("bp=");
         dumpSquare(bp);
         logPosition(position);
      }
#endif

      wk = getHflippedSquare(wk);
      bk = getHflippedSquare(bk);
      wp = (wp == A8 ? wp : getHflippedSquare(wp));
      bp = (bp == A8 ? bp : getHflippedSquare(bp));

#ifndef NDEBUG
      if (debugEval)
      {
         logDebug("after horizontal flip\n");
         logDebug("wk=");
         dumpSquare(wk);
         logDebug("bk=");
         dumpSquare(bk);
         logDebug("wp=");
         dumpSquare(wp);
         logDebug("bp=");
         dumpSquare(bp);
         logPosition(position);
      }
#endif
   }

   return getKpkpIndexFromSquares(wk, bk, wp, bp);
}

#ifndef NDEBUG
static void dumpTableAccess(const Position * position, const int egtbValue,
                            const int internalValue)
{
   int index;

   debugEval = TRUE;
   index = getKpkpIndex(position);

   logDebug("\n### egtbValue=%d internalValue=%d\n\n", egtbValue,
            internalValue);
   dumpPosition(position);
}
#endif

/**
 * Return a tablebase value for a kpkp position.
 *
 * return 0 for draw, 1 for white winning, 2 for black winning
 */
static INLINE int getKpkpValue(const Position * position)
{
   const int WIN_VALUE = 800;
   const unsigned int index = getKpkpIndex(position);
   const unsigned int bucketIndex = index >> 6;
   const unsigned int shiftIndex = index - 64 * bucketIndex;
   const int value = (int) (3 & (kpkpTable[bucketIndex] >> shiftIndex));

   assert((shiftIndex % 2) == 0);
   assert(value >= 0 && value <= 2);

#ifdef INCLUDE_TABLEBASE_ACCESS
#ifndef NDEBUG
   if (tbAvailable != FALSE)
   {
      const int tbValue = probeTablebase(position);

      if (tbValue == 0)
      {
         if (value != 0)
         {
            dumpTableAccess(position, tbValue, value);
         }

         /* assert(value == 0); */
      }

      if (tbValue > 0 && tbValue != TABLEBASE_ERROR)
      {
         if (value != 1)
         {
            dumpTableAccess(position, tbValue, value);
         }

         /* assert(value == 1); */
      }

      if (tbValue < 0)
      {
         if (value != 2)
         {
            dumpTableAccess(position, tbValue, value);
         }

         /* assert(value == 2); */
      }
   }
#endif
#endif

   if (value != 0)
   {
#ifndef NDEBUG
      if (debugEval)
      {
         logDebug("tablevalue=%d\n", value);
         dumpPosition(position);
      }
#endif

      return (value == 1 ? WIN_VALUE : -WIN_VALUE);
   }
   else
   {
      return value;
   }
}

#ifdef GENERATE_TABLES
void cleanPosition(Position * position)
{
   Square square;

   ITERATE(square)
   {
      position->piece[square] = NO_PIECE;
   }

   position->activeColor = WHITE;
   position->enPassantSquare = NO_SQUARE;
}

INLINE static UINT64 getTableValue(int dtmValue)
{
   if (dtmValue == 0)
   {
      return 0;
   }

   return (dtmValue > 0 ? 1 : 2);
}

void updateKpkpValue(const Position * position)
{
   const UINT64 tableValue = getTableValue(probeTablebase(position));
   const unsigned int index = getKpkpIndex(position);
   const unsigned int bucketIndex = index >> 6;
   const unsigned int shiftIndex = index - 64 * bucketIndex;
   const UINT64 mask = ~(((UINT64) 3) << shiftIndex);
   const UINT64 oldBucket = kpkpTempTable[bucketIndex];
   const UINT64 newBucket = (oldBucket & mask) | (tableValue << shiftIndex);

   assert((shiftIndex % 2) == 0);
   assert(tableValue >= 0 && tableValue <= 2);
   assert(bucketIndex < KPKP_TABLESIZE);
   assert(shiftIndex <= 62);

   kpkpTempTable[bucketIndex] = newBucket;
}

void generateKpkpTable()
{
   int i;
   Square wk, bk, wp, bp;
   Position position;
   long count = 0;

   for (i = 0; i < KPKP_TABLESIZE; i++)
   {
      kpkpTempTable[i] = ULONG_ZERO;
   }

   for (wk = A1; wk <= H8; wk++)
   {
      if (file(wk) >= FILE_E)
      {
         continue;              /* assume wk 2b at the queenside */
      }

      for (bk = A1; bk <= H8; bk++)
      {
         if (distance(bk, wk) < 2)
         {
            continue;
         }

         for (wp = A2; wp <= A8; wp++)
         {
            if (wp != A8)
            {
               if (wp == wk || wp == bk)
               {
                  continue;
               }
            }

            for (bp = A2; bp <= A8; bp++)
            {
               if (bp != A8)
               {
                  if (bp == wk || bp == bk || bp == wp)
                  {
                     continue;
                  }
               }

               cleanPosition(&position);
               position.piece[wk] = WHITE_KING;
               position.piece[bk] = BLACK_KING;

               if (wp >= A2 && wp <= H7)
               {
                  position.piece[wp] = WHITE_PAWN;
               }

               if (bp >= A2 && bp <= H7)
               {
                  position.piece[bp] = BLACK_PAWN;
               }

               initializePosition(&position);
               updateKpkpValue(&position);
               count++;

               if ((count & 0xffff) == 0)
               {
                  logDebug("%ld done.\n", count);
               }
            }
         }
      }
   }

   writeTableToFile(&kpkpTempTable[0], KPKP_TABLESIZE, "kpkp.c", "kpkpTable");
}
#endif

INLINE static int quad(int y_min, int y_max, int rank)
{
   const int bonusPerRank[8] = { 0, 0, 0, 26, 77, 154, 256, 0 };

   return y_min + ((y_max - y_min) * bonusPerRank[rank] + 128) / 256;
}

INLINE static int getHomeSecurityWeight(const Position * position)
{
   const int count = getPieceCount(position, WHITE_KNIGHT) +
      getPieceCount(position, (Piece) WHITE_BISHOP_DARK) +
      getPieceCount(position, (Piece) WHITE_BISHOP_LIGHT) +
      getPieceCount(position, BLACK_KNIGHT) +
      getPieceCount(position, (Piece) BLACK_BISHOP_DARK) +
      getPieceCount(position, (Piece) BLACK_BISHOP_LIGHT);

   return count * count;
}

INLINE static bool squareIsPawnSafe(const EvaluationBase * base,
                                    const Color color, const Square square)
{
   return testSquare(base->pawnAttackableSquares[opponent(color)],
                     square) == FALSE;
}

INLINE static bool hasAttackingBishop(const Position * position,
                                      const Color attackingColor,
                                      const Square square)
{
   const Bitboard attackers =
      ((lightSquares & minValue[square]) != EMPTY_BITBOARD ?
       lightSquares : darkSquares);

   return (bool)
      ((attackers & position->piecesOfType[BISHOP | attackingColor]) !=
       EMPTY_BITBOARD);
}

INLINE static void getPawnInfo(const Position * position,
                               EvaluationBase * base)
{
   const Bitboard white = position->piecesOfType[WHITE_PAWN];
   const Bitboard black = position->piecesOfType[BLACK_PAWN];
   Bitboard whiteLateralSquares, blackLateralSquares;
   Bitboard whiteSwamp, blackSwamp;
   Bitboard pawnAttackableSquaresWhite, pawnAttackableSquaresBlack;
   register Bitboard tmp1, tmp2;
   Bitboard blocked, free;
   Square square;

   /* Calculate upward and downward realms */
   tmp1 = (white << 8) | (white << 16) | (white << 24);
   tmp1 |= (tmp1 << 24);
   tmp2 = (white >> 8) | (white >> 16) | (white >> 24);
   tmp2 |= (tmp2 >> 24);

   base->doubledPawns[WHITE] = white & tmp2;
   base->upwardRealm[WHITE] = (tmp1 = tmp1 | white);
   pawnAttackableSquaresWhite = ((tmp1 & nonA) << 7) | ((tmp1 & nonH) << 9);
   base->downwardRealm[WHITE] = tmp2;

   /* Calculate upward and downward realms */
   tmp1 = (black >> 8) | (black >> 16) | (black >> 24);
   tmp1 |= (tmp1 >> 24);
   tmp2 = (black << 8) | (black << 16) | (black << 24);
   tmp2 |= (tmp2 << 24);

   base->doubledPawns[BLACK] = black & tmp2;
   base->upwardRealm[BLACK] = (tmp1 = tmp1 | black);
   pawnAttackableSquaresBlack = ((tmp1 & nonA) >> 9) | ((tmp1 & nonH) >> 7);
   base->downwardRealm[BLACK] = tmp2;

   /* Calculate the squares protected by a pawn */
   whiteLateralSquares = ((white & nonA) >> 1) | ((white & nonH) << 1);
   base->pawnProtectedSquares[WHITE] = whiteLateralSquares << 8;
   blackLateralSquares = ((black & nonA) >> 1) | ((black & nonH) << 1);
   base->pawnProtectedSquares[BLACK] = blackLateralSquares >> 8;

   /* Identify the passed pawns */
   whiteSwamp = base->downwardRealm[BLACK] | base->upwardRealm[WHITE] |
      pawnAttackableSquaresWhite;
   blackSwamp = base->downwardRealm[WHITE] | base->upwardRealm[BLACK] |
      pawnAttackableSquaresBlack;

   base->passedPawns[WHITE] = white & ~blackSwamp;
   base->passedPawns[BLACK] = black & ~whiteSwamp;

   /* Calculate the weak pawns */
   tmp2 = ~(white | black | base->pawnProtectedSquares[BLACK]);
   tmp1 = (whiteLateralSquares & tmp2) >> 8;
   tmp1 |= (tmp1 & squaresOfRank[RANK_3] & tmp2) >> 8;
   base->weakPawns[WHITE] =
      (white & ~(pawnAttackableSquaresWhite | whiteLateralSquares | tmp1));

   tmp2 = ~(white | black | base->pawnProtectedSquares[WHITE]);
   tmp1 = (blackLateralSquares & tmp2) << 8;
   tmp1 |= (tmp1 & squaresOfRank[RANK_6] & tmp2) << 8;
   base->weakPawns[BLACK] =
      (black & ~(pawnAttackableSquaresBlack | blackLateralSquares | tmp1));

   /* Calculate the candidates */
   base->candidatePawns[WHITE] = white & ~base->passedPawns[WHITE] &
      (pawnAttackableSquaresWhite | whiteLateralSquares) &
      ~(base->upwardRealm[BLACK] | base->downwardRealm[WHITE]);

   base->candidatePawns[BLACK] = black & ~base->passedPawns[BLACK] &
      (pawnAttackableSquaresBlack | blackLateralSquares) &
      ~(base->upwardRealm[WHITE] | base->downwardRealm[BLACK]);

#ifdef BONUS_HIDDEN_PASSER
   /* Calculate the hidden candidates */
   base->hiddenCandidatePawns[WHITE] = white & (black >> 8) &
      ~pawnAttackableSquaresBlack & ~(blackLateralSquares) &
      (squaresOfRank[RANK_5] | squaresOfRank[RANK_6]) &
      base->pawnProtectedSquares[WHITE];

   base->hiddenCandidatePawns[BLACK] = black & (white << 8) &
      ~pawnAttackableSquaresWhite & ~(whiteLateralSquares) &
      (squaresOfRank[RANK_4] | squaresOfRank[RANK_3]) &
      base->pawnProtectedSquares[BLACK];
#endif

   tmp1 = black & base->pawnProtectedSquares[BLACK];
   tmp2 = ((tmp1 & nonA) >> 9) & ((tmp1 & nonH) >> 7);
   tmp2 &= ~pawnAttackableSquaresWhite;
   tmp1 = tmp2 | (tmp2 >> 8);
   base->fixedPawns[WHITE] = tmp1 | (tmp1 >> 16) | (tmp1 >> 32);

   tmp1 = white & base->pawnProtectedSquares[WHITE];
   tmp2 = ((tmp1 & nonA) << 7) & ((tmp1 & nonH) << 9);
   tmp2 &= ~pawnAttackableSquaresBlack;
   tmp1 = tmp2 | (tmp2 << 8);
   base->fixedPawns[BLACK] = tmp1 | (tmp1 << 16) | (tmp1 << 32);

#ifdef BONUS_HIDDEN_PASSER
   base->hasPassersOrCandidates[WHITE] = (bool)
      (base->passedPawns[WHITE] != EMPTY_BITBOARD ||
       base->candidatePawns[WHITE] != EMPTY_BITBOARD ||
       base->hiddenCandidatePawns[WHITE] != EMPTY_BITBOARD);

   base->hasPassersOrCandidates[BLACK] = (bool)
      (base->passedPawns[BLACK] != EMPTY_BITBOARD ||
       base->candidatePawns[BLACK] != EMPTY_BITBOARD ||
       base->hiddenCandidatePawns[BLACK] != EMPTY_BITBOARD);
#endif

   tmp1 = (white << 8) & ~(white | black);
   tmp2 = white | (tmp1 & ~base->pawnProtectedSquares[BLACK]);
   tmp2 |= tmp1 & base->pawnProtectedSquares[WHITE];
   tmp1 &= squaresOfRank[RANK_3] & ~base->pawnProtectedSquares[BLACK];
   tmp1 = (tmp1 << 8) & ~(white | black);
   tmp2 |= tmp1 & ~base->pawnProtectedSquares[BLACK];
   tmp2 |= tmp1 & base->pawnProtectedSquares[WHITE];
   base->pawnAttackableSquares[WHITE] =
      ((tmp2 & nonA) << 7) | ((tmp2 & nonH) << 9);

   /*dumpBitboard(base->pawnAttackableSquares[WHITE], "pawnAttackable white");
      dumpPosition(position); */

   tmp1 = (black >> 8) & ~(white | black);
   tmp2 = black | (tmp1 & ~base->pawnProtectedSquares[WHITE]);
   tmp2 |= tmp1 & base->pawnProtectedSquares[BLACK];
   tmp1 &= squaresOfRank[RANK_6] & ~base->pawnProtectedSquares[WHITE];
   tmp1 = (tmp1 >> 8) & ~(white | black);
   tmp2 |= tmp1 & ~base->pawnProtectedSquares[WHITE];
   tmp2 |= tmp1 & base->pawnProtectedSquares[BLACK];
   base->pawnAttackableSquares[BLACK] =
      ((tmp2 & nonA) >> 9) | ((tmp2 & nonH) >> 7);

   base->pawnLightSquareMalus[WHITE] = base->pawnLightSquareMalus[BLACK] = 0;
   base->pawnDarkSquareMalus[WHITE] = base->pawnDarkSquareMalus[BLACK] = 0;
   blocked = white & (black >> 8);
   free = white & ~blocked;

   ITERATE_BITBOARD(&blocked, square)
   {
      const int malus =
         MALUS_PAWN_SQUARE_TYPE[square] + MALUS_PAWN_SQUARE_TYPE[square];

      if (testSquare(lightSquares, square))
      {
         base->pawnLightSquareMalus[WHITE] += malus;
      }
      else
      {
         base->pawnDarkSquareMalus[WHITE] += malus;
      }
   }

   ITERATE_BITBOARD(&free, square)
   {
      const int malus = MALUS_PAWN_SQUARE_TYPE[square];

      if (testSquare(lightSquares, square))
      {
         base->pawnLightSquareMalus[WHITE] += malus;
      }
      else
      {
         base->pawnDarkSquareMalus[WHITE] += malus;
      }
   }

   blocked = black & (white << 8);
   free = black & ~blocked;

   ITERATE_BITBOARD(&blocked, square)
   {
      const int malus =
         MALUS_PAWN_SQUARE_TYPE[square] + MALUS_PAWN_SQUARE_TYPE[square];

      if (testSquare(lightSquares, square))
      {
         base->pawnLightSquareMalus[BLACK] += malus;
      }
      else
      {
         base->pawnDarkSquareMalus[BLACK] += malus;
      }
   }

   ITERATE_BITBOARD(&free, square)
   {
      const int malus = MALUS_PAWN_SQUARE_TYPE[square];

      if (testSquare(lightSquares, square))
      {
         base->pawnLightSquareMalus[BLACK] += malus;
      }
      else
      {
         base->pawnDarkSquareMalus[BLACK] += malus;
      }
   }

   base->chainPawns[WHITE] = white &
      (base->pawnProtectedSquares[WHITE] | whiteLateralSquares);
   base->chainPawns[BLACK] = black &
      (base->pawnProtectedSquares[BLACK] | blackLateralSquares);
}

static INLINE int getKingWhitePawnDistance(const Square pawnSquare,
                                           const Square kingSquare)
{
   const int rankDistance = abs(rank(pawnSquare) - rank(kingSquare));
   const int fileDistance = abs(file(pawnSquare) - file(kingSquare));

   return (kingSquare > pawnSquare ? 3 : 6) * rankDistance + 6 * fileDistance;
}

INLINE int getKingBlackPawnDistance(const Square pawnSquare,
                                    const Square kingSquare)
{
   const int rankDistance = abs(rank(pawnSquare) - rank(kingSquare));
   const int fileDistance = abs(file(pawnSquare) - file(kingSquare));

   return (kingSquare < pawnSquare ? 3 : 6) * rankDistance + 6 * fileDistance;
}

static INLINE int getKingPawnDistanceDiff(const Position * position)
{
   Square square;
   Bitboard allWhite = position->piecesOfType[WHITE_PAWN];
   Bitboard allBlack = position->piecesOfType[BLACK_PAWN];
   const Square whiteKingSquare = position->king[WHITE];
   const Square blackKingSquare = position->king[BLACK];
   int whiteKingDistance = 1024, blackKingDistance = 1024;

   ITERATE_BITBOARD(&allWhite, square)
   {
      const int whiteDistance = getKingWhitePawnDistance(square,
                                                         whiteKingSquare);
      const int blackDistance = getKingWhitePawnDistance(square,
                                                         blackKingSquare);

      if (whiteDistance < whiteKingDistance)
      {
         whiteKingDistance = whiteDistance;
      }

      if (blackDistance < blackKingDistance)
      {
         blackKingDistance = blackDistance;
      }
   }

   ITERATE_BITBOARD(&allBlack, square)
   {
      const int whiteDistance = getKingBlackPawnDistance(square,
                                                         whiteKingSquare);
      const int blackDistance = getKingBlackPawnDistance(square,
                                                         blackKingSquare);

      if (whiteDistance < whiteKingDistance)
      {
         whiteKingDistance = whiteDistance;
      }

      if (blackDistance < blackKingDistance)
      {
         blackKingDistance = blackDistance;
      }
   }

   return blackKingDistance - whiteKingDistance;
}

bool pawnIsPassed(const Position * position, const Square pawnSquare,
                  const Color pawnColor)
{
   const Color defenderColor = opponent(pawnColor);
   const Bitboard corridor = passedPawnCorridor[pawnColor][pawnSquare];
   const Bitboard defenders = position->piecesOfType[PAWN | defenderColor] &
      (candidateDefenders[pawnColor][pawnSquare] | corridor);

   if (defenders == EMPTY_BITBOARD)
   {
      const Bitboard blockers = position->piecesOfType[PAWN | pawnColor] &
         corridor;

      return (bool) (blockers == EMPTY_BITBOARD);
   }

   return FALSE;
}

bool captureCreatesPasser(Position * position, const Square captureSquare,
                          const Piece capturingPiece)
{
   const Piece captured = position->piece[captureSquare];
   const Color capturedColor = pieceColor(captured);
   const Color passerColor = opponent(capturedColor);
   Bitboard candidates = (passedPawnCorridor[capturedColor][captureSquare] |
                          candidateDefenders[capturedColor][captureSquare]) &
      position->piecesOfType[PAWN | passerColor];
   bool result = FALSE;

   if (pieceType(capturingPiece) == PAWN &&
       pawnIsPassed(position, captureSquare, passerColor))
   {
      /* dumpSquare(captureSquare);
         dumpPosition(position); */

      return TRUE;
   }

   if (candidates != EMPTY_BITBOARD)
   {
      const Bitboard pawnsOriginal =
         position->piecesOfType[PAWN | capturedColor];
      Square square;

      clearSquare(position->piecesOfType[PAWN | capturedColor],
                  captureSquare);

      ITERATE_BITBOARD(&candidates, square)
      {
         if (pawnIsPassed(position, square, passerColor))
         {
            /*dumpSquare(captureSquare);
               dumpSquare(square);
               dumpBitboard(cc, "cd");
               dumpPosition(position); */

            result = TRUE;
            candidates = EMPTY_BITBOARD;        /* force loop exit */
         }
      }

      position->piecesOfType[PAWN | capturedColor] = pawnsOriginal;
   }

   return result;
}

static INLINE bool passerWalks(const Position * position,
                               const Square passerSquare,
                               const Color passerColor)
{
   const Square attackerKingSquare = position->king[passerColor];
   const Square defenderKingSquare = position->king[opponent(passerColor)];
   const int attackerDistance = distance(attackerKingSquare, passerSquare);
   const Rank kingRank = colorRank(passerColor, attackerKingSquare);
   const File passerFile = file(passerSquare);

   if (passerFile >= FILE_B && passerFile <= FILE_G)
   {
      if ((kingRank == RANK_6 || kingRank == RANK_7) &&
          kingRank > colorRank(passerColor, passerSquare) &&
          abs(file(attackerKingSquare) - passerFile) <= 1 &&
          attackerDistance <= 2)
      {
         if (position->activeColor == passerColor ||
             attackerDistance == 1 ||
             distance(defenderKingSquare, passerSquare) > 1)
         {
            return TRUE;
         }
      }

      /*
         if (kingRank == colorRank(passerColor, passerSquare) + 2 &&
         abs(file(attackerKingSquare) - passerFile) <= 1 &&
         attackerDistance <= 2)
         {
         if (position->activeColor == passerColor ||
         attackerDistance == 1 ||
         distance(defenderKingSquare, passerSquare) > 1)
         {
         return TRUE;
         }
         }
       */
   }
   else if ((kingRank == RANK_7 || kingRank == RANK_8) &&
            abs(file(attackerKingSquare) - passerFile) == 1 &&
            attackerDistance <= 2)
   {
      if (position->activeColor == passerColor ||
          attackerDistance == 1 ||
          distance(defenderKingSquare, passerSquare) > 1)
      {
         return TRUE;
      }
   }

   return FALSE;
}

INLINE static Piece getOrthoBatteryPiece(const Position * position,
                                         const Bitboard moves,
                                         const Square attackerSquare,
                                         const Color kingColor)
{
   const Square kingSquare = position->king[kingColor];
   Bitboard middlePiece = EMPTY_BITBOARD;

   if (testSquare(generalMoves[ROOK][kingSquare], attackerSquare))
   {
      middlePiece = generalMoves[ROOK][attackerSquare] &
         moves & getMagicRookMoves(kingSquare, position->allPieces);

      if (middlePiece != EMPTY_BITBOARD)
      {
         const Square pieceSquare = getLastSquare(&middlePiece);

         return position->piece[pieceSquare];
      }
   }

   return NO_PIECE;
}

INLINE static Piece getDiaBatteryPiece(const Position * position,
                                       const Bitboard moves,
                                       const Square attackerSquare,
                                       const Color kingColor)
{
   const Square kingSquare = position->king[kingColor];
   Bitboard middlePiece = EMPTY_BITBOARD;

   if (testSquare(generalMoves[BISHOP][kingSquare], attackerSquare))
   {
      middlePiece = generalMoves[BISHOP][attackerSquare] &
         moves & getMagicBishopMoves(kingSquare, position->allPieces);

      if (middlePiece != EMPTY_BITBOARD)
      {
         const Square pieceSquare = getLastSquare(&middlePiece);

         return position->piece[pieceSquare];
      }
   }

   return NO_PIECE;
}

INLINE static bool grantSeventhRankBonus(const Position * position,
                                         const Color color,
                                         const Color oppColor,
                                         const Square pieceSquare)
{
   if (colorRank(color, pieceSquare) == RANK_7)
   {
      const static Rank seventhRankByColor[2] = { RANK_7, RANK_2 };
      const Bitboard seventhRank = squaresOfRank[seventhRankByColor[color]];

      if ((seventhRank & position->piecesOfType[PAWN | oppColor]) ||
          (colorRank(color, position->king[oppColor]) >= RANK_7))
      {
         return TRUE;
      }
   }

   return FALSE;
}

#define PERSPECTIVE_WHITE
#include "evaluationc.c"
#undef PERSPECTIVE_WHITE
#include "evaluationc.c"

INLINE static void evaluatePawns(const Position * position,
                                 EvaluationBase * base)
{
   const static INT32 isolatedMalusPerFile[8] = {
      V(9, 9), V(13, 11), V(14, 11), V(14, 11),
      V(14, 11), V(14, 11), V(13, 11), V(9, 9)
   };
   const static INT32 isolatedMalusPerOpenFile[8] = {
      V(13, 14), V(19, 16), V(21, 16), V(21, 16),
      V(21, 16), V(21, 16), V(19, 16), V(13, 14)
   };
   const static INT32 backwardMalusPerFile[8] = {
      V(7, 9), V(10, 10), V(12, 10), V(12, 10),
      V(12, 10), V(12, 10), V(10, 10), V(7, 9)
   };
   const static INT32 backwardMalusPerOpenFile[8] = {
      V(11, 13), V(15, 14), V(18, 14), V(18, 14),
      V(18, 14), V(18, 14), V(15, 14), V(11, 13)
   };
   const static INT32 candidateBonusPerRank[8] = {
      V(0, 0), V(2, 5), V(2, 5), V(5, 11),
      V(13, 26), V(32, 64), V(0, 0), V(0, 0)
   };

   Square square;
   Bitboard pieces = (base->weakPawns[WHITE] | base->weakPawns[BLACK]);

   ITERATE_BITBOARD(&pieces, square)
   {
      const Color pawnColor = pieceColor(position->piece[square]);
      const Color oppColor = opponent(pawnColor);
      const File pawnfile = file(square);
      const bool isolated = (bool)
         ((companionFiles[square] &
           position->piecesOfType[position->piece[square]]) ==
          EMPTY_BITBOARD);
      const bool onOpenFile = (bool)
         (testSquare(base->upwardRealm[oppColor], square) == FALSE &&
          testSquare(base->doubledPawns[pawnColor], square) == FALSE);

      if (isolated)
      {
         if (onOpenFile)
         {
            addEvalMalusForColor(base, pawnColor,
                                 isolatedMalusPerOpenFile[pawnfile]);
         }
         else
         {
            addEvalMalusForColor(base, pawnColor,
                                 isolatedMalusPerFile[pawnfile]);
         }

         if (testSquare(base->fixedPawns[pawnColor], square))
         {
            addEvalMalusForColor(base, pawnColor, V(2, 3));
         }
      }
      else                      /* backward */
      {
         if (onOpenFile)
         {
            addEvalMalusForColor(base, pawnColor,
                                 backwardMalusPerOpenFile[pawnfile]);
         }
         else
         {
            addEvalMalusForColor(base, pawnColor,
                                 backwardMalusPerFile[pawnfile]);
         }

         if (testSquare(base->fixedPawns[pawnColor], square))
         {
            addEvalMalusForColor(base, pawnColor, V(2, 3));
         }
      }
   }

   pieces = (base->candidatePawns[WHITE] | base->candidatePawns[BLACK]);

   ITERATE_BITBOARD(&pieces, square)
   {
      const Color pawnColor = pieceColor(position->piece[square]);
      const Bitboard supporters = candidateSupporters[pawnColor][square] &
         position->piecesOfType[PAWN | pawnColor];
      const Bitboard defenders = candidateDefenders[pawnColor][square] &
         position->piecesOfType[PAWN | opponent(pawnColor)];

      if (getNumberOfSetSquares(supporters) >=
          getNumberOfSetSquares(defenders))
      {
         const Bitboard ownDefenders =
            generalMoves[PAWN | opponent(pawnColor)][square] &
            position->piecesOfType[PAWN | pawnColor];
         const Bitboard attackers =
            generalMoves[PAWN | pawnColor][square] &
            position->piecesOfType[PAWN | opponent(pawnColor)];

         if (getNumberOfSetSquares(ownDefenders) >=
             getNumberOfSetSquares(attackers))
         {
            const Rank pawnRank = colorRank(pawnColor, square);

            addEvalBonusForColor(base, pawnColor,
                                 candidateBonusPerRank[pawnRank]);
         }
      }
   }

   evaluateWhitePawns(base);
   evaluateBlackPawns(base);
}

INLINE static void evaluatePassedPawns(const Position * position,
                                       EvaluationBase * base)
{
   Square square;
   Bitboard pieces = base->passedPawns[WHITE];

   ITERATE_BITBOARD(&pieces, square)
   {
      evaluateWhitePasser(position, base, square);
   }

   pieces = base->passedPawns[BLACK];

   ITERATE_BITBOARD(&pieces, square)
   {
      evaluateBlackPasser(position, base, square);
   }
}

INLINE static void evaluateWhiteTrappedBishops(const Position * position,
                                               EvaluationBase * base)
{
   if ((position->piece[A7] == WHITE_BISHOP &&
        position->piece[B6] == BLACK_PAWN) ||
       (position->piece[B8] == WHITE_BISHOP &&
        position->piece[C7] == BLACK_PAWN) ||
       (position->piece[H7] == WHITE_BISHOP &&
        position->piece[G6] == BLACK_PAWN) ||
       (position->piece[G8] == WHITE_BISHOP &&
        position->piece[F7] == BLACK_PAWN))
   {
      base->balance -= V(BISHOP_MALUS_TRAPPED, BISHOP_MALUS_TRAPPED);
   }

   if ((position->piece[A6] == WHITE_BISHOP &&
        position->piece[B5] == BLACK_PAWN) ||
       (position->piece[H6] == WHITE_BISHOP &&
        position->piece[G5] == BLACK_PAWN))
   {
      base->balance -= V(BISHOP_MALUS_TRAPPED / 2, BISHOP_MALUS_TRAPPED / 2);
   }

   if ((position->piece[C1] == WHITE_BISHOP &&
        position->piece[D2] == WHITE_PAWN &&
        position->piece[D3] != NO_PIECE) ||
       (position->piece[F1] == WHITE_BISHOP &&
        position->piece[E2] == WHITE_PAWN && position->piece[E3] != NO_PIECE))
   {
      base->balance -= V(BISHOP_MALUS_BLOCKED, BISHOP_MALUS_BLOCKED);
   }
}

INLINE static void evaluateBlackTrappedBishops(const Position * position,
                                               EvaluationBase * base)
{
   if ((position->piece[A2] == BLACK_BISHOP &&
        position->piece[B3] == WHITE_PAWN) ||
       (position->piece[B1] == BLACK_BISHOP &&
        position->piece[C2] == WHITE_PAWN) ||
       (position->piece[H2] == BLACK_BISHOP &&
        position->piece[G3] == WHITE_PAWN) ||
       (position->piece[G1] == BLACK_BISHOP &&
        position->piece[F2] == WHITE_PAWN))
   {
      base->balance += V(BISHOP_MALUS_TRAPPED, BISHOP_MALUS_TRAPPED);
   }

   if ((position->piece[A3] == BLACK_BISHOP &&
        position->piece[B4] == WHITE_PAWN) ||
       (position->piece[H3] == BLACK_BISHOP &&
        position->piece[G4] == WHITE_PAWN))
   {
      base->balance += V(BISHOP_MALUS_TRAPPED / 2, BISHOP_MALUS_TRAPPED / 2);
   }

   if ((position->piece[C8] == BLACK_BISHOP &&
        position->piece[D7] == BLACK_PAWN &&
        position->piece[D6] != NO_PIECE) ||
       (position->piece[F8] == BLACK_BISHOP &&
        position->piece[E7] == BLACK_PAWN && position->piece[E6] != NO_PIECE))
   {
      base->balance += V(BISHOP_MALUS_BLOCKED, BISHOP_MALUS_BLOCKED);
   }
}

INLINE static int getSafetyMalusOfKingFile(const Position * position,
                                           const int file,
                                           const Square kingSquare,
                                           const int fileType,
                                           const Color color)
{
   return (color == WHITE ?
           getPawnSafetyMalusOfWhiteKingFile(position, file, kingSquare,
                                             fileType) :
           getPawnSafetyMalusOfBlackKingFile(position, file, kingSquare,
                                             fileType));
}

INLINE static int getSafetyMalusOfKingSquare(const Position * position,
                                             const Square kingSquare,
                                             const Color color)
{
   const int kingFile = file(kingSquare);
   int malus =
      getSafetyMalusOfKingFile(position, kingFile, kingSquare, 1, color);

   if (kingFile > FILE_A)
   {
      const int fileType = (kingFile <= FILE_D ? 0 : 2);

      malus += getSafetyMalusOfKingFile(position, kingFile - 1, kingSquare,
                                        fileType, color);
   }

   if (kingFile < FILE_H)
   {
      const int fileType = (kingFile <= FILE_D ? 2 : 0);

      malus += getSafetyMalusOfKingFile(position, kingFile + 1, kingSquare,
                                        fileType, color);
   }

   if (malus == 0)
   {
      malus = 10;               /* malus for weak back rank */
   }

   return malus;
}

static int getPawnShelterMalus(const Position * position, const Color color,
                               KingSafetyHashInfo * kingSafetyHashtable)
{
   const Bitboard hashValue = getKingPawnSafetyHashValue(position, color);
   KingSafetyHashInfo *kingSafetyHashInfo =
      &kingSafetyHashtable[hashValue & KINGSAFETY_HASHTABLE_MASK];

   if (kingSafetyHashInfo->hashValue == hashValue &&
       kingSafetyHashInfo->hashValue != 0)
   {
      return kingSafetyHashInfo->safetyMalus;
   }
   else
   {
      const static int rankByColor[2] = { RANK_1, RANK_8 };
      int cr00, cr000;
      const Square kingSquare = position->king[color];
      int pawnShelterMalus = 0, castlingShelterMalus = 0;

      if (color == WHITE)
      {
         cr00 = WHITE_00, cr000 = WHITE_000;
      }
      else
      {
         cr00 = BLACK_00, cr000 = BLACK_000;
      }

      pawnShelterMalus = castlingShelterMalus =
         getSafetyMalusOfKingSquare(position, kingSquare, color);

      if (position->castlingRights & cr00)
      {
         const Square kingSquare = getSquare(FILE_G, rankByColor[color]);
         const int malus00 =
            getSafetyMalusOfKingSquare(position, kingSquare, color);

         castlingShelterMalus = min(malus00, castlingShelterMalus);
      }

      if (position->castlingRights & cr000)
      {
         const Square kingSquare = getSquare(FILE_B, rankByColor[color]);
         const int malus000 =
            getSafetyMalusOfKingSquare(position, kingSquare, color);

         castlingShelterMalus = min(malus000, castlingShelterMalus);
      }

      pawnShelterMalus = (pawnShelterMalus + castlingShelterMalus) / 2;
      kingSafetyHashInfo->hashValue = hashValue;
      kingSafetyHashInfo->safetyMalus = pawnShelterMalus;

      return pawnShelterMalus;
   }
}

static INLINE Bitboard getWellDefendedSquares(EvaluationBase * base,
                                              const Color color)
{
   return base->pawnProtectedSquares[color] |
      base->knightAttackedSquares[color] | base->bishopAttackedSquares[color];
}

static int getKingSafetyMalus(const Position * position,
                              EvaluationBase * base, const Color color)
{
   const Color oppColor = opponent(color);
   const int pawnShelterMalus =
      getPawnShelterMalus(position, color, base->kingsafetyHashtable);
   const Square kingSquare = position->king[color];
   const Bitboard corona = getKingMoves(kingSquare);
   const Bitboard protectingPawn = corona &
      passedPawnCorridor[color][kingSquare];
   const Bitboard pawnAttackers =
      position->piecesOfType[PAWN | oppColor] &
      kingAttacks[kingSquare].pawnAttackers[oppColor] & ~protectingPawn;
   const int numAttackers = getEndgameValue(base->attackInfo[oppColor]) +
      getNumberOfSetSquares(pawnAttackers) / 2;
   const int pawnStructureWeight =
      KS_PAWNSTRUCTURE_WEIGHT * (33 + getPieceWeight(position, oppColor));
   int attackUnits = 0, malus = 0;

   if (base->kingSquaresAttackCount[oppColor] > 0)
   {
      const Square relativeKingSquare =
         (color == WHITE ? kingSquare : getFlippedSquare(kingSquare));
      const int kingSquareMalus = MALUS_KING_SQUARE[relativeKingSquare];
      const int attackersWeight = getOpeningValue(base->attackInfo[oppColor]);
      const Bitboard oppQueenAttacks = base->queenAttackedSquares[oppColor];
      const Bitboard oppAttacks = base->attackedSquares[oppColor] |
         getKingMoves(position->king[oppColor]);
      const Bitboard attackedSquares = corona & oppAttacks;
      const Bitboard undefendedSquares =
         attackedSquares & ~base->attackedSquares[color];
      const Bitboard safeChecks =
         ~(position->piecesOfColor[oppColor] | base->attackedSquares[color]);
      const Bitboard safeQueenChecks =
         oppQueenAttacks & undefendedSquares & safeChecks &
         (getWellDefendedSquares(base, oppColor) |
          base->rookAttackedSquares[oppColor] |
          base->rookSupportedSquares[oppColor] |
          base->queenSupportedSquares[oppColor] |
          getKingMoves(position->king[oppColor]));
      const Bitboard safeRookChecks =
         base->rookAttackedSquares[oppColor] &
         undefendedSquares & safeChecks &
         generalMoves[ROOK][kingSquare] &
         (getWellDefendedSquares(base, oppColor) |
          base->rookSupportedSquares[oppColor] |
          base->queenSupportedSquares[oppColor]);
      const int squareAttackCount =
         (base->kingSquaresAttackCount[oppColor] +
          getNumberOfSetSquares(undefendedSquares));
      const Bitboard safeDistantChecks = safeChecks & ~corona;
      const Bitboard safeOrthoChecks =
         safeDistantChecks &
         getMagicRookMoves(kingSquare, position->allPieces);
      const Bitboard safeDiaChecks =
         safeDistantChecks &
         getMagicBishopMoves(kingSquare, position->allPieces);
      const Bitboard safeKnightChecks =
         safeDistantChecks & getKnightMoves(kingSquare);

      attackUnits =
         min(25, numAttackers * attackersWeight / 2) +
         3 * squareAttackCount + kingSquareMalus;

      if (safeQueenChecks != EMPTY_BITBOARD)
      {
         const int numChecks = getNumberOfSetSquares(safeQueenChecks);
         const int activeColorFactor =
            (oppColor == position->activeColor ? 12 : 6);

         attackUnits += numChecks * activeColorFactor;
      }

      if (safeRookChecks != EMPTY_BITBOARD)
      {
         const int numChecks = getNumberOfSetSquares(safeRookChecks);
         const int activeColorFactor =
            (oppColor == position->activeColor ? 8 : 4);

         attackUnits += numChecks * activeColorFactor;
      }

      if (safeOrthoChecks != EMPTY_BITBOARD)
      {
         const Bitboard queenChecks =
            safeOrthoChecks & base->queenAttackedSquares[oppColor];
         const Bitboard rookChecks =
            safeOrthoChecks & base->rookAttackedSquares[oppColor];

         if (queenChecks != EMPTY_BITBOARD)
         {
            attackUnits += 3 * getNumberOfSetSquares(queenChecks);
         }

         if (rookChecks != EMPTY_BITBOARD)
         {
            attackUnits += 2 * getNumberOfSetSquares(rookChecks);
         }
      }

      if (safeDiaChecks != EMPTY_BITBOARD)
      {
         const Bitboard queenChecks =
            safeDiaChecks & base->queenAttackedSquares[oppColor];
         const Bitboard bishopChecks =
            safeDiaChecks & base->bishopAttackedSquares[oppColor];

         if (queenChecks != EMPTY_BITBOARD)
         {
            attackUnits += 3 * getNumberOfSetSquares(queenChecks);
         }

         if (bishopChecks != EMPTY_BITBOARD)
         {
            attackUnits += getNumberOfSetSquares(bishopChecks);
         }
      }

      if (safeKnightChecks != EMPTY_BITBOARD)
      {
         const Bitboard knightChecks =
            safeKnightChecks & base->knightAttackedSquares[oppColor];

         if (knightChecks != EMPTY_BITBOARD)
         {
            attackUnits += getNumberOfSetSquares(knightChecks);
         }
      }

      attackUnits = attackUnits * KS_PAWN_STRUCTURE_DIV + pawnShelterMalus;

      if (attackUnits >= KING_SAFETY_MALUS_DIM)
      {
         attackUnits = KING_SAFETY_MALUS_DIM - 1;
      }
      else if (attackUnits < 0)
      {
         attackUnits = 0;
      }

      malus = KING_SAFETY_MALUS[attackUnits];

      base->futilityMargin[color] = max(base->futilityMargin[color], malus);
   }

   malus += (pawnShelterMalus * pawnStructureWeight) / 4096;

   return malus;
}

INLINE static bool kingSafetyEvalRequired(const Position * position,
                                          const Color color)
{
   const Color oppColor = opponent(color);

   return (bool)
      (getPieceCount(position, (Piece) (QUEEN | oppColor)) > 0 &&
       getPieceWeight(position,
                      oppColor) >= MIN_PIECE_WEIGHT_FOR_KING_ATTACK);
}

#ifndef NDEBUG
static Bitboard randomBitboard()
{
   Bitboard tmp1 = rand();
   Bitboard tmp2 = rand();
   Bitboard tmp3 = rand();
   Bitboard tmp4 = rand();

   return tmp1 + (tmp2 << 16) + (tmp3 << 32) + (tmp4 << 48);
}
#endif

INLINE static void initializeEvaluationBase(EvaluationBase * base,
                                            KingSafetyHashInfo *
                                            kingsafetyHashtable,
                                            const Position * position)
{
#ifndef NDEBUG
   base->attackedSquares[WHITE] = randomBitboard();
   base->attackedSquares[BLACK] = randomBitboard();
   base->queenAttackedSquares[WHITE] = randomBitboard();
   base->queenAttackedSquares[BLACK] = randomBitboard();
   base->queenSupportedSquares[WHITE] = randomBitboard();
   base->queenSupportedSquares[BLACK] = randomBitboard();
   base->rookAttackedSquares[WHITE] = randomBitboard();
   base->rookAttackedSquares[BLACK] = randomBitboard();
   base->rookSupportedSquares[WHITE] = randomBitboard();
   base->rookSupportedSquares[BLACK] = randomBitboard();
   base->bishopAttackedSquares[WHITE] = randomBitboard();
   base->bishopAttackedSquares[BLACK] = randomBitboard();
   base->knightAttackedSquares[WHITE] = randomBitboard();
   base->knightAttackedSquares[BLACK] = randomBitboard();
   base->balance = rand();
   base->candidatePawns[WHITE] = randomBitboard();
   base->candidatePawns[BLACK] = randomBitboard();
   base->countedSquares[WHITE] = randomBitboard();
   base->countedSquares[BLACK] = randomBitboard();
   base->doubledPawns[WHITE] = randomBitboard();
   base->doubledPawns[BLACK] = randomBitboard();
   base->downwardRealm[WHITE] = randomBitboard();
   base->downwardRealm[BLACK] = randomBitboard();
   base->evaluateKingSafety[WHITE] = (bool) rand();
   base->evaluateKingSafety[BLACK] = (bool) rand();
   base->fixedPawns[WHITE] = randomBitboard();
   base->fixedPawns[BLACK] = randomBitboard();
   base->hasPassersOrCandidates[WHITE] = (bool) rand();
   base->hasPassersOrCandidates[BLACK] = (bool) rand();
   base->hiddenCandidatePawns[WHITE] = randomBitboard();
   base->hiddenCandidatePawns[BLACK] = randomBitboard();
   base->kingAttackSquares[WHITE] = randomBitboard();
   base->kingAttackSquares[BLACK] = randomBitboard();
   base->hangingPieces[WHITE] = randomBitboard();
   base->hangingPieces[BLACK] = randomBitboard();
   base->passedPawns[WHITE] = randomBitboard();
   base->passedPawns[BLACK] = randomBitboard();
   base->pawnAttackableSquares[WHITE] = randomBitboard();
   base->pawnAttackableSquares[BLACK] = randomBitboard();
   base->pawnDarkSquareMalus[WHITE] = rand();
   base->pawnDarkSquareMalus[BLACK] = rand();
   base->pawnLightSquareMalus[WHITE] = rand();
   base->pawnLightSquareMalus[BLACK] = rand();
   base->pawnProtectedSquares[WHITE] = randomBitboard();
   base->pawnProtectedSquares[BLACK] = randomBitboard();
   base->spaceAttackPoints[WHITE] = rand();
   base->spaceAttackPoints[BLACK] = rand();
   base->unprotectedPieces[WHITE] = randomBitboard();
   base->unprotectedPieces[BLACK] = randomBitboard();
   base->upwardRealm[WHITE] = randomBitboard();
   base->upwardRealm[BLACK] = randomBitboard();
   base->weakPawns[WHITE] = randomBitboard();
   base->weakPawns[BLACK] = randomBitboard();
   base->kingSquaresAttackCount[WHITE] = rand();
   base->kingSquaresAttackCount[BLACK] = rand();
   base->attackInfo[WHITE] = rand();
   base->attackInfo[BLACK] = rand();
#endif
   base->balance = 0;
   base->kingsafetyHashtable = kingsafetyHashtable;
   base->futilityMargin[WHITE] = base->futilityMargin[BLACK] = 0;
   base->hangingPieces[WHITE] = base->hangingPieces[BLACK] = EMPTY_BITBOARD;
   base->materialInfo = &materialInfo[calculateMaterialSignature(position)];
}

INLINE static void evaluatePieces(const Position * position,
                                  EvaluationBase * base)
{
   const Bitboard exclude = position->piecesOfType[WHITE_PAWN] |
      position->piecesOfType[BLACK_PAWN] |
      minValue[position->king[WHITE]] | minValue[position->king[BLACK]];
   Bitboard pieces = position->allPieces & (~exclude);
   Square square;

#ifndef NDEBUG
   if (debugEval)
   {
      logDebug("\nStart of piece evaluation\n");
   }
#endif

   ITERATE_BITBOARD(&pieces, square)
   {
#ifndef NDEBUG
      if (debugEval)
      {
         dumpSquare(square);
         logDebug("op=%d eg=%d\n", getOpeningValue(base->balance),
                  getEndgameValue(base->balance));
      }
#endif
      base->materialBalance += mvImpact[position->piece[square]];

      switch (position->piece[square])
      {
      case WHITE_QUEEN:
         evaluateWhiteQueen(position, base, square);
         break;

      case BLACK_QUEEN:
         evaluateBlackQueen(position, base, square);
         break;

      case WHITE_ROOK:
         evaluateWhiteRook(position, base, square);
         break;

      case BLACK_ROOK:
         evaluateBlackRook(position, base, square);
         break;

      case WHITE_BISHOP:
         evaluateWhiteBishop(position, base, square);
         break;

      case BLACK_BISHOP:
         evaluateBlackBishop(position, base, square);
         break;

      case WHITE_KNIGHT:
         evaluateWhiteKnight(position, base, square);
         break;

      case BLACK_KNIGHT:
         evaluateBlackKnight(position, base, square);
         break;

      default:
         break;
      }

#ifndef NDEBUG
      if (debugEval)
      {
         logDebug("op=%d eg=%d\n", getOpeningValue(base->balance),
                  getEndgameValue(base->balance));
      }
#endif
   }

   if (position->piecesOfType[WHITE_BISHOP] != EMPTY_BITBOARD)
   {
      evaluateWhiteTrappedBishops(position, base);
   }

   if (position->piecesOfType[BLACK_BISHOP] != EMPTY_BITBOARD)
   {
      evaluateBlackTrappedBishops(position, base);
   }

#ifndef NDEBUG
   if (debugEval)
   {
      logDebug("Values after piece evaluation\n");
      logDebug("op=%d eg=%d\n", getOpeningValue(base->balance),
               getEndgameValue(base->balance));
      logDebug("End of piece evaluation\n");
   }
#endif
}

/*
static INLINE Bitboard getFixedPawns(const Position * position,
                              const EvaluationBase * base, const Color color)
{
   const Color oppColor = opponent(color);
   const Bitboard candidates = position->piecesOfType[PAWN | color] &
      ~base->pawnProtectedSquares[color];
   const Bitboard blockers = position->piecesOfType[PAWN | color] |
      position->piecesOfColor[oppColor] |
      base->pawnProtectedSquares[oppColor];

   return candidates & (color == WHITE ? blockers >> 8 : blockers << 8);
}
*/

static INLINE bool evaluateKingChase(const Position * position,
                                     const Color kingColor)
{
   if (position->numberOfPawns[WHITE] == 0 &&
       position->numberOfPawns[BLACK] == 0 &&
       numberOfNonPawnPieces(position, kingColor) <= 2)
   {
      return (bool) (getPieceWeight(position, kingColor) <
                     getPieceWeight(position, opponent(kingColor)));
   }

   return FALSE;
}

static INLINE int getKingChaseMalus(const Position * position,
                                    const Color huntedKingColor)
{
   const Color attackingColor = opponent(huntedKingColor);
   const Square huntedKingSquare = position->king[huntedKingColor];
   const Square attackingKingSquare = position->king[attackingColor];
   const int mutualDistance =
      distance(huntedKingSquare, attackingKingSquare) - 2;
   int cornerDistanceMalus = kingChaseMalus[ALL][huntedKingSquare];

   if (hasOrthoPieces(position, attackingColor) == FALSE &&
       position->piecesOfType[(Piece) (BISHOP | attackingColor)] !=
       EMPTY_BITBOARD)
   {
      const Bitboard attackingBishops =
         position->piecesOfType[BISHOP | attackingColor];

      if ((lightSquares & attackingBishops) == EMPTY_BITBOARD)
      {
         cornerDistanceMalus = kingChaseMalus[DARK][huntedKingSquare];
      }

      if ((darkSquares & attackingBishops) == EMPTY_BITBOARD)
      {
         cornerDistanceMalus = kingChaseMalus[LIGHT][huntedKingSquare];
      }
   }

   return 5 * (5 - mutualDistance) + 15 * cornerDistanceMalus;
}

static int getHomelandSecurityCount(const Position * position,
                                    EvaluationBase * base, const Color color)
{
   const Color oppColor = opponent(color);
   const Bitboard ownPawns = position->piecesOfType[PAWN | color];
   const Bitboard exclude = ownPawns |
      base->pawnProtectedSquares[oppColor] |
      (base->attackedSquares[oppColor] & ~base->attackedSquares[color]);
   const Bitboard safeSquares = homeland[color] & ~exclude;
   const Bitboard superSafeSquares =
      safeSquares & (color == WHITE ? (ownPawns >> 8) | (ownPawns >> 16) :
                     (ownPawns << 8) | (ownPawns << 16));

   return getNumberOfSetSquares(safeSquares) +
      getNumberOfSetSquares(superSafeSquares);
}

INLINE static int numberOfPawnsAttackedByKing(const Position * position,
                                              const EvaluationBase * base,
                                              const Color kingColor)
{
   const Bitboard attacks = getKingMoves(position->king[kingColor]);
   const Color pawnColor = opponent(kingColor);
   const Bitboard targets = attacks &
      position->piecesOfType[PAWN | pawnColor] &
      ~base->pawnProtectedSquares[pawnColor];

   return (targets != EMPTY_BITBOARD ? getNumberOfSetSquares(targets) : 0);
}

INLINE static void getPositionalValue(const Position * position,
                                      EvaluationBase * base)
{
   const Bitboard whiteCorona = getKingMoves(position->king[WHITE]);
   const Bitboard blackCorona = getKingMoves(position->king[BLACK]);

   base->countedSquares[WHITE] = ~(position->piecesOfColor[WHITE] |
                                   base->pawnProtectedSquares[BLACK]);
   base->unprotectedPieces[WHITE] = position->piecesOfColor[WHITE] &
      ~base->pawnProtectedSquares[WHITE];
   base->countedSquares[BLACK] = ~(position->piecesOfColor[BLACK] |
                                   base->pawnProtectedSquares[WHITE]);
   base->unprotectedPieces[BLACK] = position->piecesOfColor[BLACK] &
      ~base->pawnProtectedSquares[BLACK];
   base->spaceAttackPoints[WHITE] = base->spaceAttackPoints[BLACK] = 0;
   base->attackedSquares[WHITE] = base->pawnProtectedSquares[WHITE];
   base->attackedSquares[BLACK] = base->pawnProtectedSquares[BLACK];
   base->knightAttackedSquares[WHITE] = EMPTY_BITBOARD;
   base->knightAttackedSquares[BLACK] = EMPTY_BITBOARD;
   base->bishopAttackedSquares[WHITE] = EMPTY_BITBOARD;
   base->bishopAttackedSquares[BLACK] = EMPTY_BITBOARD;
   base->rookAttackedSquares[WHITE] = EMPTY_BITBOARD;
   base->rookAttackedSquares[BLACK] = EMPTY_BITBOARD;
   base->rookSupportedSquares[WHITE] = EMPTY_BITBOARD;
   base->rookSupportedSquares[BLACK] = EMPTY_BITBOARD;
   base->queenAttackedSquares[WHITE] = EMPTY_BITBOARD;
   base->queenAttackedSquares[BLACK] = EMPTY_BITBOARD;
   base->queenSupportedSquares[WHITE] = EMPTY_BITBOARD;
   base->queenSupportedSquares[BLACK] = EMPTY_BITBOARD;
   base->kingAttackSquares[WHITE] = whiteCorona | (whiteCorona << 8);
   base->kingAttackSquares[BLACK] = blackCorona | (blackCorona >> 8);
   base->evaluateKingSafety[WHITE] = kingSafetyEvalRequired(position, WHITE);
   base->evaluateKingSafety[BLACK] = kingSafetyEvalRequired(position, BLACK);
   base->kingSquaresAttackCount[WHITE] =
      base->kingSquaresAttackCount[BLACK] = 0;
   base->attackInfo[WHITE] = base->attackInfo[BLACK] = 0;
   base->materialBalance =
      position->numberOfPawns[WHITE] * mvImpact[WHITE_PAWN] +
      position->numberOfPawns[BLACK] * mvImpact[BLACK_PAWN];

   evaluatePieces(position, base);
   base->attackedSquares[WHITE] |= base->knightAttackedSquares[WHITE] |
      base->bishopAttackedSquares[WHITE] | base->rookAttackedSquares[WHITE] |
      base->queenAttackedSquares[WHITE];
   base->attackedSquares[BLACK] |= base->knightAttackedSquares[BLACK] |
      base->bishopAttackedSquares[BLACK] | base->rookAttackedSquares[BLACK] |
      base->queenAttackedSquares[BLACK];

#ifndef NDEBUG
   if (debugEval)
   {
      logDebug("\nEval values before king safety eval:\n");
      logDebug("op=%d eg=%d\n", getOpeningValue(base->balance),
               getEndgameValue(base->balance));
   }

   {
      Color color;

      for (color = WHITE; color <= BLACK; color++)
      {
         Bitboard pawns = position->piecesOfType[PAWN | color];
         Square pawnSquare;

         ITERATE_BITBOARD(&pawns, pawnSquare)
         {
            if (testSquare(base->passedPawns[color], pawnSquare))
            {
               assert(pawnIsPassed(position, pawnSquare, color));
            }
            else
            {
               assert(pawnIsPassed(position, pawnSquare, color) == FALSE);
            }
         }
      }
   }
#endif

   if (base->materialInfo->phaseIndex < PHASE_INDEX_MAX)
   {
      int malus = getKingSafetyMalus(position, base, WHITE);

#ifdef NDEBUG
      if (base->ownColor == WHITE)
      {
         malus = (OWN_COLOR_WEIGHT_KINGSAFETY * malus) / OWN_COLOR_WEIGHT_DIV;
      }
#endif

      base->balance -= V(malus, 0);
   }

   if (base->materialInfo->phaseIndex < PHASE_INDEX_MAX)
   {
      int malus = getKingSafetyMalus(position, base, BLACK);

#ifdef NDEBUG
      if (base->ownColor == BLACK)
      {
         malus = (OWN_COLOR_WEIGHT_KINGSAFETY * malus) / OWN_COLOR_WEIGHT_DIV;
      }
#endif

      base->balance += V(malus, 0);
   }

#ifndef NDEBUG
   if (debugEval)
   {
      logDebug("\nEval values before passed pawn eval:\n");
      logDebug("op=%d eg=%d\n", getOpeningValue(base->balance),
               getEndgameValue(base->balance));
   }
#endif

   if (base->passedPawns[WHITE] != EMPTY_BITBOARD ||
       base->passedPawns[BLACK] != EMPTY_BITBOARD)
   {
#ifndef NDEBUG
      if (debugEval)
      {
         dumpBitboard(base->passedPawns[WHITE], "white passers");
         dumpBitboard(base->passedPawns[BLACK], "black passers");
      }
#endif

      evaluatePassedPawns(position, base);
   }

   if (evaluateKingChase(position, WHITE))
   {
      const int kingChaseMalus = getKingChaseMalus(position, WHITE);

      base->balance -= V(kingChaseMalus, kingChaseMalus);
   }
   else if (evaluateKingChase(position, BLACK))
   {
      const int kingChaseMalus = getKingChaseMalus(position, BLACK);

      base->balance += V(kingChaseMalus, kingChaseMalus);
   }

#ifndef NDEBUG
   if (debugEval)
   {
      logDebug("\nEval values before space attack eval:\n");
      logDebug("op=%d eg=%d\n", getOpeningValue(base->balance),
               getEndgameValue(base->balance));
   }
#endif

   {
      const Bitboard whiteExcludes = base->attackedSquares[WHITE] |
         getKingMoves(position->king[WHITE]) |
         position->piecesOfType[WHITE_PAWN];
      const Bitboard blackExcludes = base->attackedSquares[BLACK] |
         getKingMoves(position->king[BLACK]) |
         position->piecesOfType[BLACK_PAWN];
      Bitboard whiteAdditions = position->piecesOfColor[WHITE] &
         base->attackedSquares[BLACK] & ~whiteExcludes;
      Bitboard blackAdditions = position->piecesOfColor[BLACK] &
         base->attackedSquares[WHITE] & ~blackExcludes;

      /*
         if (whiteAdditions != EMPTY_BITBOARD)
         {
         const Bitboard excludes =
         (base->knightAttackedSquares[WHITE] &
         position->piecesOfType[BLACK_KNIGHT]) |
         (base->bishopAttackedSquares[WHITE] &
         position->piecesOfType[BLACK_BISHOP]) |
         (base->rookAttackedSquares[WHITE] &
         position->piecesOfType[BLACK_ROOK]) |
         (base->queenAttackedSquares[WHITE] &
         position->piecesOfType[BLACK_QUEEN]);

         whiteAdditions &= ~excludes;
         }

         if (blackAdditions != EMPTY_BITBOARD)
         {
         const Bitboard excludes =
         (base->knightAttackedSquares[BLACK] &
         position->piecesOfType[WHITE_KNIGHT]) |
         (base->bishopAttackedSquares[BLACK] &
         position->piecesOfType[WHITE_BISHOP]) |
         (base->rookAttackedSquares[BLACK] &
         position->piecesOfType[WHITE_ROOK]) |
         (base->queenAttackedSquares[BLACK] &
         position->piecesOfType[WHITE_QUEEN]);

         blackAdditions &= ~excludes;
         }
       */

      base->hangingPieces[WHITE] |= whiteAdditions;
      base->hangingPieces[BLACK] |= blackAdditions;

      /* if (getNumberOfSetSquares(base->hangingPieces[WHITE])>=4)
         {
         dumpBitboard(base->hangingPieces[WHITE], "hpw");
         dumpPosition(position);
         } */

      evaluateWhiteAttackers(base);
      evaluateBlackAttackers(base);
   }

   base->attackedSquares[WHITE] |= getKingMoves(position->king[WHITE]);
   base->attackedSquares[BLACK] |= getKingMoves(position->king[BLACK]);

   if (base->materialInfo->phaseIndex < PHASE_INDEX_MAX)
   {
      const int weight = getHomeSecurityWeight(position);

      if (weight > 0)
      {
         const int countWhite =
            getHomelandSecurityCount(position, base, WHITE);
         const int countBlack =
            getHomelandSecurityCount(position, base, BLACK);
         const int baseValue = (countWhite - countBlack) * weight;

         base->balance += V((baseValue * HOMELAND_SECURITY_WEIGHT) / 256, 0);
      }
   }

#ifdef MALUS_KING_PAWN_DISTANCE
   if (numberOfNonPawnPieces(position, WHITE) <= 3 &&
       numberOfNonPawnPieces(position, BLACK) <= 3)
   {
      const int diff = getKingPawnDistanceDiff(position);

      base->balance += V(0, diff);
   }
#endif

   {
      INT32 bonus = V(0, 6);

      base->balance +=
         bonus * (numberOfPawnsAttackedByKing(position, base, WHITE) -
                  numberOfPawnsAttackedByKing(position, base, BLACK));
   }

#ifndef NDEBUG
   if (debugEval)
   {
      logDebug("\nFinal eval values:\n");
      logDebug("op=%d eg=%d\n", getOpeningValue(base->balance),
               getEndgameValue(base->balance));
   }
#endif
}

int getValue(const Position * position,
             EvaluationBase * base,
             PawnHashInfo * pawnHashtable,
             KingSafetyHashInfo * kingsafetyHashtable)
{
   PawnHashInfo *pawnHashInfo =
      &pawnHashtable[position->pawnHashValue & PAWN_HASHTABLE_MASK];

   initializeEvaluationBase(base, kingsafetyHashtable, position);

   if (pawnHashInfo->hashValue == position->pawnHashValue &&
       pawnHashInfo->hashValue != 0)
   {
#ifndef NDEBUG
      getPawnInfo(position, base);
      evaluatePawns(position, base);

      assert(base->balance == pawnHashInfo->balance);
      assert(base->pawnProtectedSquares[WHITE] ==
             pawnHashInfo->pawnProtectedSquares[WHITE]);
      assert(base->pawnProtectedSquares[BLACK] ==
             pawnHashInfo->pawnProtectedSquares[BLACK]);
      assert(base->passedPawns[WHITE] == pawnHashInfo->passedPawns[WHITE]);
      assert(base->passedPawns[BLACK] == pawnHashInfo->passedPawns[BLACK]);
#endif

      base->balance = pawnHashInfo->balance;
      base->passedPawns[WHITE] = pawnHashInfo->passedPawns[WHITE];
      base->passedPawns[BLACK] = pawnHashInfo->passedPawns[BLACK];
      base->pawnProtectedSquares[WHITE] =
         pawnHashInfo->pawnProtectedSquares[WHITE];
      base->pawnProtectedSquares[BLACK] =
         pawnHashInfo->pawnProtectedSquares[BLACK];
      base->pawnAttackableSquares[WHITE] =
         pawnHashInfo->pawnAttackableSquares[WHITE];
      base->pawnAttackableSquares[BLACK] =
         pawnHashInfo->pawnAttackableSquares[BLACK];
      base->pawnLightSquareMalus[WHITE] =
         pawnHashInfo->pawnLightSquareMalus[WHITE];
      base->pawnLightSquareMalus[BLACK] =
         pawnHashInfo->pawnLightSquareMalus[BLACK];
      base->pawnDarkSquareMalus[WHITE] =
         pawnHashInfo->pawnDarkSquareMalus[WHITE];
      base->pawnDarkSquareMalus[BLACK] =
         pawnHashInfo->pawnDarkSquareMalus[BLACK];
#ifdef BONUS_HIDDEN_PASSER
      base->hasPassersOrCandidates[WHITE] =
         pawnHashInfo->hasPassersOrCandidates[WHITE];
      base->hasPassersOrCandidates[BLACK] =
         pawnHashInfo->hasPassersOrCandidates[BLACK];
#endif
   }
   else
   {
      getPawnInfo(position, base);
      evaluatePawns(position, base);

      pawnHashInfo->hashValue = position->pawnHashValue;
      pawnHashInfo->balance = base->balance;
      pawnHashInfo->pawnProtectedSquares[WHITE] =
         base->pawnProtectedSquares[WHITE];
      pawnHashInfo->pawnProtectedSquares[BLACK] =
         base->pawnProtectedSquares[BLACK];
      pawnHashInfo->passedPawns[WHITE] = base->passedPawns[WHITE];
      pawnHashInfo->passedPawns[BLACK] = base->passedPawns[BLACK];
      pawnHashInfo->pawnAttackableSquares[WHITE] =
         base->pawnAttackableSquares[WHITE];
      pawnHashInfo->pawnAttackableSquares[BLACK] =
         base->pawnAttackableSquares[BLACK];
      pawnHashInfo->pawnLightSquareMalus[WHITE] =
         base->pawnLightSquareMalus[WHITE];
      pawnHashInfo->pawnLightSquareMalus[BLACK] =
         base->pawnLightSquareMalus[BLACK];
      pawnHashInfo->pawnDarkSquareMalus[WHITE] =
         base->pawnDarkSquareMalus[WHITE];
      pawnHashInfo->pawnDarkSquareMalus[BLACK] =
         base->pawnDarkSquareMalus[BLACK];
#ifdef BONUS_HIDDEN_PASSER
      pawnHashInfo->hasPassersOrCandidates[WHITE] =
         base->hasPassersOrCandidates[WHITE];
      pawnHashInfo->hasPassersOrCandidates[BLACK] =
         base->hasPassersOrCandidates[BLACK];
#endif
   }

#ifdef TRACE_EVAL
   logDebug("\nStarting evaluation.\n");
   logDebug("phaseIndex = %d\n", phaseIndex(position));
   logDebug("opvWhite = %d egvWhite = %d\n",
            position->openingValue[WHITE], position->endgameValue[WHITE]);
   logDebug("opvBlack = %d egvBlack = %d\n",
            position->openingValue[BLACK], position->endgameValue[BLACK]);
   logDebug("basicValue = %d\n\n", basicValue);
   logPosition(position);
   logDebug("\n");
#endif

   if (kpkpValueAvailable(position))
   {
      int currentPositionalValue = getKpkpValue(position);

      if (currentPositionalValue == 0)
      {
         return 0;
      }

      getPositionalValue(position, base);
   }
   else
   {
      getPositionalValue(position, base);
   }

   return positionalBalance(position, base);
}

static void transposeMatrix(const int human[], int machine[])
{
   int file, rank, i = 0;

   for (rank = RANK_8; rank >= RANK_1; rank--)
   {
      for (file = FILE_A; file <= FILE_H; file++)
      {
         const Square machineSquare = getSquare(file, rank);

         machine[machineSquare] = human[i++];
      }
   }
}

int pstOpeningValue(INT32 value, const int weight)
{
   return (weight * getOpeningValue(value)) / 256;
}

int pstEndgameValue(INT32 value, const int weight)
{
   return (weight * getEndgameValue(value)) / 256;
}

static void initializePieceSquareValues()
{
   Square sq;

   mvImpact[WHITE_QUEEN] = V(VALUE_QUEEN_OPENING, VALUE_QUEEN_ENDGAME);
   mvImpact[BLACK_QUEEN] = V(-VALUE_QUEEN_OPENING, -VALUE_QUEEN_ENDGAME);
   mvImpact[WHITE_ROOK] = V(VALUE_ROOK_OPENING, VALUE_ROOK_ENDGAME);
   mvImpact[BLACK_ROOK] = V(-VALUE_ROOK_OPENING, -VALUE_ROOK_ENDGAME);
   mvImpact[WHITE_BISHOP] = V(VALUE_BISHOP_OPENING, VALUE_BISHOP_ENDGAME);
   mvImpact[BLACK_BISHOP] = V(-VALUE_BISHOP_OPENING, -VALUE_BISHOP_ENDGAME);
   mvImpact[WHITE_KNIGHT] = V(VALUE_KNIGHT_OPENING, VALUE_KNIGHT_ENDGAME);
   mvImpact[BLACK_KNIGHT] = V(-VALUE_KNIGHT_OPENING, -VALUE_KNIGHT_ENDGAME);
   mvImpact[WHITE_PAWN] =
      V(PAWN_DEFAULTVALUE_OPENING, PAWN_DEFAULTVALUE_ENDGAME);
   mvImpact[BLACK_PAWN] =
      V(-PAWN_DEFAULTVALUE_OPENING, -PAWN_DEFAULTVALUE_ENDGAME);

   ITERATE(sq)
   {
      int ov, ev;

      ov = pstOpeningValue(PstPawn[sq], 258);
      ev = pstEndgameValue(PstPawn[sq], 257);
      pieceSquareBonus[WHITE_PAWN][sq] =
         pieceSquareBonus[BLACK_PAWN][getFlippedSquare(sq)] = V(ov, ev);

      ov = pstOpeningValue(PstKnight[sq], 261);
      ev = pstEndgameValue(PstKnight[sq], 257);
      pieceSquareBonus[WHITE_KNIGHT][sq] =
         pieceSquareBonus[BLACK_KNIGHT][getFlippedSquare(sq)] = V(ov, ev);

      ov = pstOpeningValue(PstBishop[sq], 261);
      ev = pstEndgameValue(PstBishop[sq], 256);
      pieceSquareBonus[WHITE_BISHOP][sq] =
         pieceSquareBonus[BLACK_BISHOP][getFlippedSquare(sq)] = V(ov, ev);

      ov = pstOpeningValue(PstRook[sq], 256);
      ev = pstEndgameValue(PstRook[sq], 256);
      pieceSquareBonus[WHITE_ROOK][sq] =
         pieceSquareBonus[BLACK_ROOK][getFlippedSquare(sq)] = V(ov, ev);

      ov = pstOpeningValue(PstQueen[sq], 256);
      ev = pstEndgameValue(PstQueen[sq], 256);
      pieceSquareBonus[WHITE_QUEEN][sq] =
         pieceSquareBonus[BLACK_QUEEN][getFlippedSquare(sq)] = V(ov, ev);

      ov = pstOpeningValue(PstKing[sq], 256);
      ev = pstEndgameValue(PstKing[sq], 256);
      pieceSquareBonus[WHITE_KING][sq] =
         pieceSquareBonus[BLACK_KING][getFlippedSquare(sq)] = V(ov, ev);
   }
}

static void initializeKingAttacks()
{
   Square square;

   ITERATE(square)
   {
      const Bitboard corona = getKingMoves(square);
      KingAttacks *attackInfo = &kingAttacks[square];
      Square attackerSquare;

      attackInfo->diaAttackers = attackInfo->orthoAttackers =
         attackInfo->knightAttackers = attackInfo->pawnAttackers[WHITE] =
         attackInfo->pawnAttackers[BLACK] = EMPTY_BITBOARD;

      ITERATE(attackerSquare)
      {
         attackInfo->attackedByDia[attackerSquare] =
            attackInfo->attackedByOrtho[attackerSquare] = NO_SQUARE;
      }

      ITERATE(attackerSquare)
      {
         Bitboard dia, ortho;
         const Bitboard knight =
            corona & generalMoves[WHITE_KNIGHT][attackerSquare];
         const Bitboard whitePawn =
            corona & generalMoves[WHITE_PAWN][attackerSquare];
         const Bitboard blackPawn =
            corona & generalMoves[BLACK_PAWN][attackerSquare];

         dia = corona & generalMoves[WHITE_BISHOP][attackerSquare];
         ortho = corona & generalMoves[WHITE_ROOK][attackerSquare];

         if (dia != EMPTY_BITBOARD)
         {
            Square attackedSquare;
            int dist = 8;

            setSquare(attackInfo->diaAttackers, attackerSquare);

            ITERATE_BITBOARD(&dia, attackedSquare)
            {
               const int currentDistance =
                  distance(attackerSquare, attackedSquare);

               if (currentDistance < dist)
               {
                  attackInfo->attackedByDia[attackerSquare] = attackedSquare;
                  dist = currentDistance;
               }
            }
         }

         if (ortho != EMPTY_BITBOARD)
         {
            Square attackedSquare;
            int dist = 8;

            setSquare(attackInfo->orthoAttackers, attackerSquare);

            ITERATE_BITBOARD(&ortho, attackedSquare)
            {
               const int currentDistance =
                  distance(attackerSquare, attackedSquare);

               if (currentDistance < dist)
               {
                  attackInfo->attackedByOrtho[attackerSquare] =
                     attackedSquare;
                  dist = currentDistance;
               }
            }
         }

         if (knight != EMPTY_BITBOARD)
         {
            setSquare(attackInfo->knightAttackers, attackerSquare);
         }

         if (whitePawn != EMPTY_BITBOARD)
         {
            setSquare(attackInfo->pawnAttackers[WHITE], attackerSquare);
         }

         if (blackPawn != EMPTY_BITBOARD)
         {
            setSquare(attackInfo->pawnAttackers[BLACK], attackerSquare);
         }
      }
   }
}

#define AVOID_TRADES_WITH_TWO_KNIGHTS   /* */

static void getPieceTradeSignatures(UINT32 * materialSignatureWhite,
                                    UINT32 * materialSignatureBlack)
{
   int numWhiteQueens, numWhiteRooks, numWhiteLightSquareBishops;
   int numWhiteDarkSquareBishops, numWhiteKnights, numWhitePawns;
   int numBlackQueens, numBlackRooks, numBlackLightSquareBishops;
   int numBlackDarkSquareBishops, numBlackKnights, numBlackPawns;
   bool finished = TRUE;
   bool whiteKnightTradesOnly = FALSE, blackKnightTradesOnly = FALSE;
   bool knightTradesOnly = FALSE;
   const UINT32 signature =
      bilateralSignature(*materialSignatureWhite, *materialSignatureBlack);

#ifdef AVOID_TRADES_WITH_TWO_KNIGHTS
   int numWhiteNonPawns, numBlackNonPawns;
#endif

   getPieceCounters(signature, &numWhiteQueens, &numWhiteRooks,
                    &numWhiteLightSquareBishops,
                    &numWhiteDarkSquareBishops, &numWhiteKnights,
                    &numWhitePawns, &numBlackQueens, &numBlackRooks,
                    &numBlackLightSquareBishops,
                    &numBlackDarkSquareBishops, &numBlackKnights,
                    &numBlackPawns);

#ifdef AVOID_TRADES_WITH_TWO_KNIGHTS
   numWhiteNonPawns =
      numWhiteQueens + numWhiteRooks + numWhiteLightSquareBishops +
      numWhiteDarkSquareBishops + numWhiteKnights;
   numBlackNonPawns =
      numBlackQueens + numBlackRooks + numBlackLightSquareBishops +
      numBlackDarkSquareBishops + numBlackKnights;

   if (numWhitePawns + numBlackPawns == 0 && numWhiteKnights == 2 &&
       numWhiteNonPawns == 3 && numWhiteNonPawns - numBlackNonPawns >= 2)
   {
      whiteKnightTradesOnly = TRUE;     /* white will avoid to trade pieces */
   }

   if (numBlackPawns + numWhitePawns == 0 && numBlackKnights == 2 &&
       numBlackNonPawns == 3 && numBlackNonPawns - numWhiteNonPawns >= 2)
   {
      blackKnightTradesOnly = TRUE;     /* black will avoid to trade pieces */
   }

   knightTradesOnly = whiteKnightTradesOnly || blackKnightTradesOnly;
#endif

   if (knightTradesOnly == FALSE && numWhiteQueens > 0 && numBlackQueens > 0)
   {
      numWhiteQueens--;
      numBlackQueens--;
      finished = FALSE;
      goto calculateSignature;
   }

   if (knightTradesOnly == FALSE && numWhiteRooks > 0 && numBlackRooks > 0)
   {
      numWhiteRooks--;
      numBlackRooks--;
      finished = FALSE;
      goto calculateSignature;
   }

   if (knightTradesOnly == FALSE &&
       numWhiteLightSquareBishops > 0 && numBlackLightSquareBishops > 0)
   {
      numWhiteLightSquareBishops--;
      numBlackLightSquareBishops--;
      finished = FALSE;
      goto calculateSignature;
   }

   if (knightTradesOnly == FALSE &&
       numWhiteDarkSquareBishops > 0 && numBlackDarkSquareBishops > 0)
   {
      numWhiteDarkSquareBishops--;
      numBlackDarkSquareBishops--;
      finished = FALSE;
      goto calculateSignature;
   }

   if (knightTradesOnly == FALSE &&
       numWhiteLightSquareBishops > 0 && numBlackDarkSquareBishops > 0)
   {
      numWhiteLightSquareBishops--;
      numBlackDarkSquareBishops--;
      finished = FALSE;
      goto calculateSignature;
   }

   if (knightTradesOnly == FALSE &&
       numWhiteDarkSquareBishops > 0 && numBlackLightSquareBishops > 0)
   {
      numWhiteDarkSquareBishops--;
      numBlackLightSquareBishops--;
      finished = FALSE;
      goto calculateSignature;
   }

   if (numWhiteKnights > 0 && numBlackKnights > 0)
   {
      numWhiteKnights--;
      numBlackKnights--;
      finished = FALSE;
      goto calculateSignature;
   }

   if (whiteKnightTradesOnly == FALSE &&
       numWhiteLightSquareBishops > 0 && numBlackKnights > 0 &&
       numWhiteDarkSquareBishops == 0)
   {
      numWhiteLightSquareBishops--;
      numBlackKnights--;
      finished = FALSE;
      goto calculateSignature;
   }

   if (whiteKnightTradesOnly == FALSE &&
       numWhiteDarkSquareBishops > 0 && numBlackKnights > 0 &&
       numWhiteLightSquareBishops == 0)
   {
      numWhiteDarkSquareBishops--;
      numBlackKnights--;
      finished = FALSE;
      goto calculateSignature;
   }

   if (blackKnightTradesOnly == FALSE &&
       numWhiteKnights > 0 && numBlackLightSquareBishops > 0 &&
       numBlackDarkSquareBishops == 0)
   {
      numWhiteKnights--;
      numBlackLightSquareBishops--;
      finished = FALSE;
      goto calculateSignature;
   }

   if (blackKnightTradesOnly == FALSE &&
       numWhiteKnights > 0 && numBlackDarkSquareBishops > 0 &&
       numBlackLightSquareBishops == 0)
   {
      numWhiteKnights--;
      numBlackDarkSquareBishops--;
      finished = FALSE;
      goto calculateSignature;
   }

 calculateSignature:

   *materialSignatureWhite =
      getSingleMaterialSignature(numWhiteQueens, numWhiteRooks,
                                 numWhiteLightSquareBishops,
                                 numWhiteDarkSquareBishops, numWhiteKnights,
                                 numWhitePawns);
   *materialSignatureBlack =
      getSingleMaterialSignature(numBlackQueens, numBlackRooks,
                                 numBlackLightSquareBishops,
                                 numBlackDarkSquareBishops, numBlackKnights,
                                 numBlackPawns);

   if (finished == FALSE)
   {
      getPieceTradeSignatures(materialSignatureWhite, materialSignatureBlack);
   }
}

static bool hasMaterialForMate(const UINT32 materialSignature,
                               const UINT32 oppMaterialSignature,
                               SpecialEvalType * specialEval,
                               const bool tradePieces,
                               const bool evaluateOppMaterial)
{
   int numQueens, numRooks, numLightSquareBishops, numDarkSquareBishops;
   int numKnights, numPawns;
   int numOppQueens, numOppRooks, numOppLightSquareBishops;
   int numOppDarkSquareBishops, numOppKnights, numOppPawns;
   int numBishops, numOppBishops;
   int numPieces, numOppPieces;
   const UINT32 signature =
      bilateralSignature(materialSignature, oppMaterialSignature);

   if (tradePieces)
   {
      UINT32 ms = materialSignature, mso = oppMaterialSignature;
      SpecialEvalType dummy;

      getPieceTradeSignatures(&ms, &mso);

      return hasMaterialForMate(ms, mso, &dummy, FALSE, evaluateOppMaterial);
   }

   getPieceCounters(signature, &numQueens, &numRooks,
                    &numLightSquareBishops, &numDarkSquareBishops,
                    &numKnights, &numPawns, &numOppQueens, &numOppRooks,
                    &numOppLightSquareBishops, &numOppDarkSquareBishops,
                    &numOppKnights, &numOppPawns);

   numBishops = numLightSquareBishops + numDarkSquareBishops;
   numOppBishops = numOppLightSquareBishops + numOppDarkSquareBishops;
   numPieces = numQueens + numRooks + numBishops + numKnights;
   numOppPieces = numOppQueens + numOppRooks + numOppBishops + numOppKnights;

   if (evaluateOppMaterial && numPawns == 0)
   {
      if (numPieces == 1)
      {
         if (numRooks == 1 && numOppPieces > 0)
         {
            return FALSE;
         }

         if (numQueens == 1)
         {
            if (numOppQueens >= 1 || numOppKnights >= 2)
            {
               return FALSE;
            }

            if (numOppPieces >= 2 && numOppRooks >= 1)
            {
               return FALSE;
            }
         }
      }
      else if (numPieces == 2)
      {
         if (numBishops == 2 &&
             numOppQueens + numOppRooks + numOppBishops > 0)
         {
            return FALSE;
         }

         if (numBishops == 1 && numKnights == 1 && numOppPieces > 0)
         {
            return FALSE;
         }
      }
   }

   if (numQueens + numRooks + numLightSquareBishops + numDarkSquareBishops +
       numKnights == 0 && numPawns > 0)
   {
      *specialEval = Se_KpK;

      return TRUE;
   }

   if (numQueens + numRooks + numLightSquareBishops + numDarkSquareBishops +
       numPawns == 0 && numKnights == 2)
   {
      if (numOppQueens + numOppRooks + numOppLightSquareBishops +
          numOppDarkSquareBishops + numOppKnights == 0 && numOppPawns > 0)
      {
         *specialEval = Se_KnnKp;

         return TRUE;
      }
   }

   if (numQueens + numRooks + numKnights == 0 &&
       numLightSquareBishops + numDarkSquareBishops == 1 && numPawns > 0)
   {
      *specialEval = Se_KbpK;

      return TRUE;
   }

   if (numQueens + numBishops + numKnights == 0 && numRooks == 1 &&
       numPawns == 1 && numOppBishops >= 1)
   {
      *specialEval = Se_KrpKb;

      return TRUE;
   }

   if (numQueens + numBishops + numKnights == 0 && numRooks == 1 &&
       numPawns == 1 && numOppRooks >= 1)
   {
      *specialEval = Se_KrpKr;

      return TRUE;
   }

   if (numQueens + numBishops + numKnights == 0 && numRooks == 1 &&
       numPawns == 2 && numOppRooks >= 1)
   {
      *specialEval = Se_KrppKr;

      return TRUE;
   }

   if (numRooks + numBishops + numKnights == 0 && numQueens == 1 &&
       numPawns == 1 && numOppQueens >= 1)
   {
      *specialEval = Se_KqpKq;

      return TRUE;
   }

   if (numRooks + numBishops + numKnights == 0 && numQueens == 1 &&
       numPawns == 2 && numOppQueens >= 1)
   {
      *specialEval = Se_KqppKq;

      return TRUE;
   }

   if (numQueens + numRooks + numPawns > 0 || numKnights >= 3)
   {
      return TRUE;
   }

   if (numLightSquareBishops > 0 && numDarkSquareBishops > 0)
   {
      return TRUE;
   }

   if (numKnights > 0 && numLightSquareBishops + numDarkSquareBishops > 0)
   {
      return TRUE;
   }

   return FALSE;
}

static PieceType getKamikazePiece(const UINT32 ownMaterialSignature,
                                  const UINT32 oppMaterialSignature)
{
   int numQueens, numRooks, numLightSquareBishops;
   int numDarkSquareBishops, numKnights, numPawns;
   int numOppQueens, numOppRooks, numOppLightSquareBishops;
   int numOppDarkSquareBishops, numOppKnights, numOppPawns;
   int ownSignature;
   const UINT32 signature =
      bilateralSignature(ownMaterialSignature, oppMaterialSignature);

   getPieceCounters(signature, &numQueens, &numRooks,
                    &numLightSquareBishops, &numDarkSquareBishops,
                    &numKnights, &numPawns, &numOppQueens, &numOppRooks,
                    &numOppLightSquareBishops, &numOppDarkSquareBishops,
                    &numOppKnights, &numOppPawns);

   ownSignature =
      getSingleMaterialSignature(numQueens, numRooks,
                                 numLightSquareBishops,
                                 numDarkSquareBishops, numKnights,
                                 numPawns - 1);

   if (numOppRooks > 0)
   {
      const int oppSignature =
         getSingleMaterialSignature(numOppQueens, numOppRooks - 1,
                                    numOppLightSquareBishops,
                                    numOppDarkSquareBishops,
                                    numOppKnights, numOppPawns);

      if (hasMaterialForMate(ownSignature, oppSignature, 0, TRUE, FALSE) ==
          FALSE)
      {
         return ROOK;
      }
   }

   if (numOppLightSquareBishops > 0)
   {
      const int oppSignature =
         getSingleMaterialSignature(numOppQueens, numOppRooks,
                                    numOppLightSquareBishops - 1,
                                    numOppDarkSquareBishops,
                                    numOppKnights, numOppPawns);

      if (hasMaterialForMate(ownSignature, oppSignature, 0, TRUE, FALSE) ==
          FALSE)
      {
         return BISHOP;
      }
   }

   if (numOppDarkSquareBishops > 0)
   {
      const int oppSignature =
         getSingleMaterialSignature(numOppQueens, numOppRooks,
                                    numOppLightSquareBishops,
                                    numOppDarkSquareBishops - 1,
                                    numOppKnights, numOppPawns);

      if (hasMaterialForMate(ownSignature, oppSignature, 0, TRUE, FALSE) ==
          FALSE)
      {
         return BISHOP;
      }
   }

   if (numOppKnights > 0)
   {
      const int oppSignature =
         getSingleMaterialSignature(numOppQueens, numOppRooks,
                                    numOppLightSquareBishops,
                                    numOppDarkSquareBishops,
                                    numOppKnights - 1,
                                    numOppPawns);

      if (hasMaterialForMate(ownSignature, oppSignature, 0, TRUE, FALSE) ==
          FALSE)
      {
         return KNIGHT;
      }
   }

   return NO_PIECETYPE;
}

static UINT8 getWinningChances(const UINT32 materialSignature,
                               const UINT32 oppMaterialSignature)
{
   int numQueens, numRooks, numLightSquareBishops;
   int numDarkSquareBishops, numKnights, numPawns;
   int numOppQueens, numOppRooks, numOppLightSquareBishops;
   int numOppDarkSquareBishops, numOppKnights, numOppPawns;
   int numPieces;
   int numOppBishops, numOppMinors, numOppSliders;
   bool oppositeColoredBishops;
   const UINT32 signature =
      bilateralSignature(materialSignature, oppMaterialSignature);
   PieceType kamikazePiece = getKamikazePiece(materialSignature,
                                              oppMaterialSignature);

   getPieceCounters(signature, &numQueens, &numRooks,
                    &numLightSquareBishops, &numDarkSquareBishops,
                    &numKnights, &numPawns, &numOppQueens, &numOppRooks,
                    &numOppLightSquareBishops, &numOppDarkSquareBishops,
                    &numOppKnights, &numOppPawns);
   numPieces = numQueens + numRooks + numLightSquareBishops +
      numDarkSquareBishops + numKnights;
   numOppBishops = numOppLightSquareBishops + numOppDarkSquareBishops;
   numOppMinors = numOppBishops + numOppKnights;
   numOppSliders = numOppQueens + numOppRooks + numOppBishops;
   oppositeColoredBishops = (bool)
      (numLightSquareBishops + numDarkSquareBishops > 0 &&
       ((numOppLightSquareBishops > 0 && numLightSquareBishops == 0) ||
        (numOppDarkSquareBishops > 0 && numDarkSquareBishops == 0)));

   if (numPieces == 0)
   {
      if (numPawns <= 1 && numOppSliders > 0)
      {
         return 0;
      }

      if (numPawns <= 1 && numOppKnights > 0)
      {
         return 4;
      }

      if (numPawns == 2)
      {
         return (numOppSliders >= 2 ? 2 : 8);
      }
   }

   if (numPieces == 1)
   {
      if (oppositeColoredBishops)
      {
         const int pawnDiff = min(3, abs(numPawns - numOppPawns));

         return (UINT8) (numPawns > 1 ? 8 + 2 * pawnDiff : 0);
      }

      if (numPawns == 1)        /* One piece, one pawn: */
      {
         if (numQueens > 0 && numOppRooks >= 2)
         {
            return 1;
         }

         if (numQueens > 0 && numOppRooks + numOppMinors >= 2)
         {
            return 12;          /* usually won, but difficult */
         }

         if (kamikazePiece != NO_PIECETYPE)
         {
            switch (kamikazePiece)
            {
            case ROOK:
               return 1;
            case BISHOP:
               return 2;
            case KNIGHT:
               return 4;
            default:
               break;
            }
         }
      }
   }
   else if (numPieces == 2)     /* has more than one piece: */
   {
      if (numPawns <= 1)
      {
         if (numRooks == 2 && numOppQueens > 0)
         {
            return (numPawns == 0 ? 1 : 2);
         }

         if (kamikazePiece != NO_PIECETYPE)
         {
            switch (kamikazePiece)
            {
            case ROOK:
               return 1;
            case BISHOP:
               return 2;
            case KNIGHT:
               return 4;
            default:
               break;
            }
         }
      }

      if (numQueens == 0 && numRooks <= 1 && numRooks == numOppRooks &&
          oppositeColoredBishops)
      {
         const int pawnDiff = min(3, abs(numPawns - numOppPawns));

         return (UINT8) (12 + pawnDiff);
      }
   }

   return 16;
}

static UINT8 getWinningChancesWithoutPawn(UINT32 materialSignature,
                                          UINT32 oppMaterialSignature)
{
   int numQueens, numRooks, numLightSquareBishops;
   int numDarkSquareBishops, numKnights, numPawns;
   int numOppQueens, numOppRooks, numOppLightSquareBishops;
   int numOppDarkSquareBishops, numOppKnights, numOppPawns;
   int numPieces, numOppPieces;
   int numOppBishops, numOppMinors, numOppSliders;
   const UINT32 signature =
      bilateralSignature(materialSignature, oppMaterialSignature);

   getPieceTradeSignatures(&materialSignature, &oppMaterialSignature);

   getPieceCounters(signature, &numQueens, &numRooks,
                    &numLightSquareBishops, &numDarkSquareBishops,
                    &numKnights, &numPawns, &numOppQueens, &numOppRooks,
                    &numOppLightSquareBishops, &numOppDarkSquareBishops,
                    &numOppKnights, &numOppPawns);
   numPieces = numQueens + numRooks + numLightSquareBishops +
      numDarkSquareBishops + numKnights;
   numOppBishops = numOppLightSquareBishops + numOppDarkSquareBishops;
   numOppMinors = numOppBishops + numOppKnights;
   numOppSliders = numOppQueens + numOppRooks + numOppBishops;
   numOppPieces = numOppSliders + numOppKnights;

   if (numPieces == 0)
   {
      return 0;
   }

   if (numPieces == 1)
   {
      if (numQueens > 0 && numOppRooks > 0 && numOppRooks + numOppMinors >= 2)
      {
         return 1;
      }

      if (numQueens > 0 && numOppKnights >= 2)
      {
         return 1;
      }

      if (numRooks > 0 && numOppQueens + numOppRooks > 0)
      {
         return 1;
      }

      if (numRooks > 0 && numOppMinors > 0)
      {
         return (numOppMinors == 1 ? 2 : 1);
      }

      if (numLightSquareBishops + numDarkSquareBishops + numKnights > 0)
      {
         return 0;
      }
   }
   else if (numPieces <= 3)
   {
      if (numQueens == 0 && numOppQueens > 0)
      {
         if (numRooks <= 1)
         {
            return 1;
         }
         else
         {
            return 8;           /* hard to win */
         }
      }

      if (numRooks + numQueens == 0 &&
          numLightSquareBishops + numDarkSquareBishops <= 1 &&
          numOppRooks + numOppQueens >= 1)
      {
         return (numOppQueens >= 1 ? 1 : 2);
      }
   }

   if (numLightSquareBishops == 1 && numDarkSquareBishops == 1)
   {
      if (numOppPieces == 1 && numOppRooks == 1)
      {
         return (numPieces == 2 ? 1 : 12);
      }

      if (numOppKnights > 0)
      {
         return 8;              /* hard to win sometimes */
      }
   }

   return 16;
}

static void testMaterialSignatureNew(const int numWhiteQueens,
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

   if (numWhiteRooks <= 2 && numWhiteKnights <= 2 &&
       numBlackRooks <= 2 && numBlackKnights <= 2)
   {
      const UINT32 signature = getMaterialSignature(numWhiteQueens,
                                                    numWhiteRooks,
                                                    numWhiteLightSquareBishops,
                                                    numWhiteDarkSquareBishops,
                                                    numWhiteKnights,
                                                    numWhitePawns,
                                                    numBlackQueens,
                                                    numBlackRooks,
                                                    numBlackLightSquareBishops,
                                                    numBlackDarkSquareBishops,
                                                    numBlackKnights,
                                                    numBlackPawns);

      getPieceCounters(signature,
                       &calculatedNumWhiteQueens, &calculatedNumWhiteRooks,
                       &calculatedNumWhiteLightSquareBishops,
                       &calculatedNumWhiteDarkSquareBishops,
                       &calculatedNumWhiteKnights,
                       &calculatedNumWhitePawns, &calculatedNumBlackQueens,
                       &calculatedNumBlackRooks,
                       &calculatedNumBlackLightSquareBishops,
                       &calculatedNumBlackDarkSquareBishops,
                       &calculatedNumBlackKnights, &calculatedNumBlackPawns);

      assert(calculatedNumWhiteQueens == numWhiteQueens);
      assert(calculatedNumWhiteRooks == numWhiteRooks);
      assert(calculatedNumWhiteLightSquareBishops ==
             numWhiteLightSquareBishops);
      assert(calculatedNumWhiteDarkSquareBishops ==
             numWhiteDarkSquareBishops);
      assert(calculatedNumWhiteKnights == numWhiteKnights);
      assert(calculatedNumWhitePawns == numWhitePawns);
      assert(calculatedNumBlackQueens == numBlackQueens);
      assert(calculatedNumBlackRooks == numBlackRooks);
      assert(calculatedNumBlackLightSquareBishops ==
             numBlackLightSquareBishops);
      assert(calculatedNumBlackDarkSquareBishops ==
             numBlackDarkSquareBishops);
      assert(calculatedNumBlackKnights == numBlackKnights);
      assert(calculatedNumBlackPawns == numBlackPawns);
   }
}

static int calculatePhase(UINT32 signature)
{
   int numWhiteQueens;
   int numWhiteRooks;
   int numWhiteLightSquareBishops;
   int numWhiteDarkSquareBishops;
   int numWhiteKnights;
   int numWhitePawns;
   int numBlackQueens;
   int numBlackRooks;
   int numBlackLightSquareBishops;
   int numBlackDarkSquareBishops;
   int numBlackKnights;
   int numBlackPawns;
   int whiteWeight, blackWeight, basicPhase;

   getPieceCounters(signature,
                    &numWhiteQueens, &numWhiteRooks,
                    &numWhiteLightSquareBishops,
                    &numWhiteDarkSquareBishops,
                    &numWhiteKnights, &numWhitePawns,
                    &numBlackQueens, &numBlackRooks,
                    &numBlackLightSquareBishops,
                    &numBlackDarkSquareBishops,
                    &numBlackKnights, &numBlackPawns);

   whiteWeight =
      9 * numWhiteQueens + 5 * numWhiteRooks +
      3 * numWhiteLightSquareBishops + 3 * numWhiteDarkSquareBishops +
      3 * numWhiteKnights;
   blackWeight =
      9 * numBlackQueens + 5 * numBlackRooks +
      3 * numBlackLightSquareBishops + 3 * numBlackDarkSquareBishops +
      3 * numBlackKnights;

   basicPhase = (whiteWeight + blackWeight <= PIECEWEIGHT_ENDGAME ?
                 PHASE_MAX : max(0, PHASE_MAX - whiteWeight - blackWeight));

   return (basicPhase * 256 + (PHASE_MAX / 2)) / PHASE_MAX;
}

static INT32 calculateMaterialBalance(UINT32 signature)
{
   const INT32 bishopPairBonus =
      V(VALUE_BISHOP_PAIR_OPENING, VALUE_BISHOP_PAIR_ENDGAME);
   static const INT32 knightBonus = V(0, 5);
   static const INT32 rookMalus = V(5, 0);
   static const INT32 rookPairMalus = V(16, 24);
   static const INT32 rookQueenMalus = V(8, 12);
   static const INT32 pieceUpBonus =
      V(DEFAULTVALUE_PIECE_UP_OPENING, DEFAULTVALUE_PIECE_UP_ENDGAME);

   int numWhiteQueens;
   int numWhiteRooks;
   int numWhiteLightSquareBishops;
   int numWhiteDarkSquareBishops;
   int numWhiteKnights;
   int numWhitePawns;
   int numBlackQueens;
   int numBlackRooks;
   int numBlackLightSquareBishops;
   int numBlackDarkSquareBishops;
   int numBlackKnights;
   int numBlackPawns;
   int pawnCountWhite, pawnCountBlack;
   int knightSaldo, rookSaldo, pieceCountSaldo;
   INT32 balance = 0;

   getPieceCounters(signature,
                    &numWhiteQueens, &numWhiteRooks,
                    &numWhiteLightSquareBishops,
                    &numWhiteDarkSquareBishops,
                    &numWhiteKnights,
                    &numWhitePawns, &numBlackQueens,
                    &numBlackRooks,
                    &numBlackLightSquareBishops,
                    &numBlackDarkSquareBishops,
                    &numBlackKnights, &numBlackPawns);

   pawnCountWhite = numWhitePawns - 5;
   pawnCountBlack = numBlackPawns - 5;
   knightSaldo = pawnCountWhite * numWhiteKnights -
      pawnCountBlack * numBlackKnights;
   rookSaldo = pawnCountWhite * numWhiteRooks -
      pawnCountBlack * numBlackRooks;
   pieceCountSaldo =
      (numWhiteLightSquareBishops +
       numWhiteDarkSquareBishops + numWhiteKnights) -
      (numBlackLightSquareBishops +
       numBlackDarkSquareBishops + numBlackKnights);

   if (numWhiteLightSquareBishops > 0 && numWhiteDarkSquareBishops > 0)
   {
      balance += bishopPairBonus;
   }

   if (numBlackLightSquareBishops > 0 && numBlackDarkSquareBishops > 0)
   {
      balance -= bishopPairBonus;
   }

   balance += knightSaldo * knightBonus - rookSaldo * rookMalus;

   if (numWhiteRooks >= 2)
   {
      balance -= rookPairMalus + rookQueenMalus;
   }
   else if (numWhiteRooks + numWhiteQueens >= 2)
   {
      balance -= rookQueenMalus;
   }

   if (numBlackRooks >= 2)
   {
      balance += rookPairMalus + rookQueenMalus;
   }
   else if (numBlackRooks + numBlackQueens >= 2)
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

static void initializeMaterialInfoTableCore1(const UINT32 signatureWhite,
                                             const UINT32 signatureBlack)
{
   const UINT32 signature =
      bilateralSignature(signatureWhite, signatureBlack);
   SpecialEvalType specialEvalWhite = Se_None;
   SpecialEvalType specialEvalBlack = Se_None;
   const bool whiteMateMat =
      hasMaterialForMate(signatureWhite, signatureBlack, &specialEvalWhite,
                         FALSE, FALSE);
   const bool blackMateMat =
      hasMaterialForMate(signatureBlack, signatureWhite, &specialEvalBlack,
                         FALSE, FALSE);
   int numWhiteQueens, numWhiteRooks, numWhiteLightSquareBishops;
   int numWhiteDarkSquareBishops, numWhiteKnights, numWhitePawns;
   int numBlackQueens, numBlackRooks, numBlackLightSquareBishops;
   int numBlackDarkSquareBishops, numBlackKnights, numBlackPawns;

   getPieceCounters(signature, &numWhiteQueens, &numWhiteRooks,
                    &numWhiteLightSquareBishops,
                    &numWhiteDarkSquareBishops, &numWhiteKnights,
                    &numWhitePawns, &numBlackQueens, &numBlackRooks,
                    &numBlackLightSquareBishops,
                    &numBlackDarkSquareBishops, &numBlackKnights,
                    &numBlackPawns);

   testMaterialSignatureNew(numWhiteQueens,
                            numWhiteRooks,
                            numWhiteLightSquareBishops,
                            numWhiteDarkSquareBishops,
                            numWhiteKnights,
                            numWhitePawns,
                            numBlackQueens,
                            numBlackRooks,
                            numBlackLightSquareBishops,
                            numBlackDarkSquareBishops,
                            numBlackKnights, numBlackPawns);

   materialInfo[signature].chancesWhite = (whiteMateMat == FALSE ? 0 : 16);
   materialInfo[signature].chancesBlack = (blackMateMat == FALSE ? 0 : 16);
   materialInfo[signature].specialEvalWhite = specialEvalWhite;
   materialInfo[signature].specialEvalBlack = specialEvalBlack;

   if (whiteMateMat != FALSE)
   {
      if (numWhitePawns == 0)
      {
         if (hasMaterialForMate(signatureWhite, signatureBlack, 0,
                                TRUE, TRUE) == FALSE)
         {
            materialInfo[signature].chancesWhite = 1;
         }
         else
         {
            materialInfo[signature].chancesWhite =
               getWinningChancesWithoutPawn(signatureWhite, signatureBlack);
         }
      }
      else
      {
         materialInfo[signature].chancesWhite =
            getWinningChances(signatureWhite, signatureBlack);
      }
   }

   if (blackMateMat != FALSE)
   {
      if (numBlackPawns == 0)
      {
         if (hasMaterialForMate(signatureBlack, signatureWhite, 0,
                                TRUE, TRUE) == FALSE)
         {
            materialInfo[signature].chancesBlack = 1;
         }
         else
         {
            materialInfo[signature].chancesBlack =
               getWinningChancesWithoutPawn(signatureBlack, signatureWhite);
         }
      }
      else
      {
         materialInfo[signature].chancesBlack =
            getWinningChances(signatureBlack, signatureWhite);
      }
   }

   materialInfo[signature].materialBalance =
      calculateMaterialBalance(signature);
   materialInfo[signature].phaseIndex = calculatePhase(signature);
}

static void initializeMaterialInfoTable()
{
   int whiteSignature, blackSignature;

   for (whiteSignature = 0; whiteSignature < 648; whiteSignature++)
   {
      for (blackSignature = 0; blackSignature < 648; blackSignature++)
      {
         initializeMaterialInfoTableCore1(whiteSignature, blackSignature);
      }
   }
}

static int initializeKingSafetyTable()
{
   const int MAX_MALUS = 500;
   const int MAX_STEP = 12;
   int i;
   const int factor = KS_WEIGHT;

   for (i = 0; i < KING_SAFETY_MALUS_DIM; i++)
   {
      const int expFactor = (int) pow((double) i, KS_EXP);

      KING_SAFETY_MALUS[i] = (factor * expFactor) /
         (1000 * KS_PAWN_STRUCTURE_DIV * KS_PAWN_STRUCTURE_DIV);

      if (i > 0 && KING_SAFETY_MALUS[i] - KING_SAFETY_MALUS[i - 1] > MAX_STEP)
      {
         KING_SAFETY_MALUS[i] = KING_SAFETY_MALUS[i - 1] + MAX_STEP;
      }

      KING_SAFETY_MALUS[i] = min(KING_SAFETY_MALUS[i], MAX_MALUS);
      /* logDebug("ksm(%d)=%d\n", i, KING_SAFETY_MALUS[i]); */
   }

   /* getKeyStroke(); */

   return 0;
}

/* #define LOG_MOBILILITY_VALUES */

static void initializeMoveBonusValue()
{
   int i;

   for (i = 0; i <= MAX_MOVES_QUEEN; i++)
   {
      const int opValue = logIntValue(3.94, MAX_MOVES_QUEEN + 1,
                                      8.04, i + 1);
      const int egValue = logIntValue(6.1, MAX_MOVES_QUEEN + 1,
                                      16.06, i + 1);

      QueenMobilityBonus[i] = V(opValue, egValue);

#ifdef LOG_MOBILILITY_VALUES
      logDebug("mQ(%d)=(%d/%d)\n", i, getOpeningValue(QueenMobilityBonus[i]),
               getEndgameValue(QueenMobilityBonus[i]));
#endif
   }

   logDebug("\n");

   for (i = 0; i <= MAX_MOVES_ROOK; i++)
   {
      const int opValue = logIntValue(3.08, MAX_MOVES_ROOK + 1,
                                      15.74, i + 1);
      const int egValue = logIntValue(5.0, MAX_MOVES_ROOK + 1,
                                      50.74, i + 1);

      RookMobilityBonus[i] = V(opValue, egValue);

#ifdef LOG_MOBILILITY_VALUES
      logDebug("mR(%d)=(%d/%d)\n", i, getOpeningValue(RookMobilityBonus[i]),
               getEndgameValue(RookMobilityBonus[i]));
#endif
   }

   logDebug("\n");

   for (i = 0; i <= MAX_MOVES_BISHOP; i++)
   {
      const int opValue = logIntValue(2.99, MAX_MOVES_BISHOP + 1,
                                      35.42, i + 1);
      const int egValue = logIntValue(4.84, MAX_MOVES_BISHOP + 1,
                                      34.13, i + 1);

      BishopMobilityBonus[i] = V(opValue, egValue);

#ifdef LOG_MOBILILITY_VALUES
      logDebug("mB(%d)=(%d/%d)\n", i, getOpeningValue(BishopMobilityBonus[i]),
               getEndgameValue(BishopMobilityBonus[i]));
#endif
   }

   logDebug("\n");

   for (i = 0; i <= MAX_MOVES_KNIGHT; i++)
   {
      const int opValue = logIntValue(3.98, MAX_MOVES_KNIGHT + 1,
                                      15.875, i + 1);
      const int egValue = logIntValue(4.05, MAX_MOVES_KNIGHT + 1,
                                      12.15, i + 1);

      KnightMobilityBonus[i] = V(opValue, egValue);

#ifdef LOG_MOBILILITY_VALUES
      logDebug("mN(%d)=(%d/%d)\n", i, getOpeningValue(KnightMobilityBonus[i]),
               getEndgameValue(KnightMobilityBonus[i]));
#endif
   }

#ifdef LOG_MOBILILITY_VALUES
   getKeyStroke();
#endif
}

int initializeModuleEvaluation()
{
   int i;
   Square square, kingsquare, catchersquare;

   centralFiles = squaresOfFile[FILE_D] | squaresOfFile[FILE_E];
   attackingRealm[WHITE] = squaresOfRank[RANK_5] | squaresOfRank[RANK_6] |
      squaresOfRank[RANK_7] | squaresOfRank[RANK_8];
   attackingRealm[BLACK] = squaresOfRank[RANK_4] | squaresOfRank[RANK_3] |
      squaresOfRank[RANK_2] | squaresOfRank[RANK_1];
   filesBCFG = squaresOfFileRange[FILE_B][FILE_C] |
      squaresOfFileRange[FILE_F][FILE_G];

   ITERATE(square)
   {
      Color color;
      Square kingSquare;

      for (color = WHITE; color <= BLACK; color++)
      {
         passedPawnRectangle[color][square] =
            passedPawnCorridor[color][square] =
            candidateDefenders[color][square] =
            candidateSupporters[color][square] =
            pawnOpponents[color][square] = EMPTY_BITBOARD;
      }

      ITERATE(kingSquare)
      {
         kingRealm[WHITE][square][kingSquare] =
            kingRealm[BLACK][square][kingSquare] = EMPTY_BITBOARD;
      }

      kingTrapsRook[WHITE][square] = kingTrapsRook[BLACK][square] =
         EMPTY_BITBOARD;
   }

   setSquare(kingTrapsRook[WHITE][F1], H1);     /* a king on f1 traps a rook on h1 ... */
   setSquare(kingTrapsRook[WHITE][F1], G1);
   setSquare(kingTrapsRook[WHITE][F1], H2);
   setSquare(kingTrapsRook[WHITE][F1], G2);
   setSquare(kingTrapsRook[WHITE][G1], H1);
   setSquare(kingTrapsRook[WHITE][G1], H2);
   setSquare(kingTrapsRook[WHITE][G1], G2);
   setSquare(kingTrapsRook[WHITE][G2], H2);

   setSquare(kingTrapsRook[WHITE][C1], A1);
   setSquare(kingTrapsRook[WHITE][C1], B1);
   setSquare(kingTrapsRook[WHITE][C1], A2);
   setSquare(kingTrapsRook[WHITE][C1], B2);
   setSquare(kingTrapsRook[WHITE][B1], A1);
   setSquare(kingTrapsRook[WHITE][B1], A2);
   setSquare(kingTrapsRook[WHITE][B1], B2);
   setSquare(kingTrapsRook[WHITE][B2], A2);

   setSquare(kingTrapsRook[BLACK][F8], H8);
   setSquare(kingTrapsRook[BLACK][F8], G8);
   setSquare(kingTrapsRook[BLACK][F8], H7);
   setSquare(kingTrapsRook[BLACK][F8], G7);
   setSquare(kingTrapsRook[BLACK][G8], H8);
   setSquare(kingTrapsRook[BLACK][G8], H7);
   setSquare(kingTrapsRook[BLACK][G8], G7);
   setSquare(kingTrapsRook[BLACK][G7], H7);

   setSquare(kingTrapsRook[BLACK][C8], A8);
   setSquare(kingTrapsRook[BLACK][C8], B8);
   setSquare(kingTrapsRook[BLACK][C8], A7);
   setSquare(kingTrapsRook[BLACK][C8], B7);
   setSquare(kingTrapsRook[BLACK][B8], A8);
   setSquare(kingTrapsRook[BLACK][B8], A7);
   setSquare(kingTrapsRook[BLACK][B8], B7);
   setSquare(kingTrapsRook[BLACK][B7], A7);

   ITERATE(square)
   {
      const int squarefile = file(square);
      const int squarerank = rank(square);
      int d1 = min(distance(square, D4), distance(square, E4));
      int d2 = min(distance(square, D5), distance(square, E5));
      int td1 = min(taxiDistance(square, D4), taxiDistance(square, E4));
      int td2 = min(taxiDistance(square, D5), taxiDistance(square, E5));

      centerDistance[square] = min(d1, d2);
      centerTaxiDistance[square] = min(td1, td2);
      butterflySquares[square] =
         generalMoves[KING][square] & ~squaresOfFile[squarefile];
      lateralSquares[square] =
         generalMoves[KING][square] & squaresOfRank[squarerank];
      companionFiles[square] =
         ((squaresOfFile[squarefile] & nonA) >> 1) |
         ((squaresOfFile[squarefile] & nonH) << 1);
      rookBlockers[square] = EMPTY_BITBOARD;

      ITERATE(kingsquare)
      {
         const int kingsquarefile = file(kingsquare);
         const int kingsquarerank = rank(kingsquare);
         Square targetSquare;

         if (kingsquarerank >= squarerank &&
             distance(square, kingsquare) <= 7 - squarerank)
         {
            setSquare(passedPawnRectangle[WHITE][square], kingsquare);
         }

         if (kingsquarerank <= squarerank &&
             distance(square, kingsquare) <= squarerank)
         {
            setSquare(passedPawnRectangle[BLACK][square], kingsquare);
         }

         if (kingsquarefile == squarefile)
         {
            if (kingsquarerank > squarerank)
            {
               setSquare(passedPawnCorridor[WHITE][square], kingsquare);
            }

            if (kingsquarerank < squarerank)
            {
               setSquare(passedPawnCorridor[BLACK][square], kingsquare);
            }
         }

         if (squarerank == kingsquarerank)
         {
            if (squarefile <= FILE_C && kingsquarefile <= FILE_C &&
                kingsquarefile > squarefile)
            {
               setSquare(rookBlockers[square], kingsquare);
            }

            if (squarefile >= FILE_F && kingsquarefile >= FILE_F &&
                kingsquarefile < squarefile)
            {
               setSquare(rookBlockers[square], kingsquare);
            }
         }

         ITERATE(targetSquare)
         {
            if (distance(square, targetSquare) <
                distance(kingsquare, targetSquare))
            {
               const Rank targetrank = rank(targetSquare);

               if (targetrank <= squarerank + 1)
               {
                  setSquare(kingRealm[WHITE][square][kingsquare],
                            targetSquare);
               }

               if (targetrank >= squarerank - 1)
               {
                  setSquare(kingRealm[BLACK][square][kingsquare],
                            targetSquare);
               }
            }
         }
      }

      ITERATE(catchersquare)
      {
         if (abs(file(catchersquare) - squarefile) == 1)
         {
            if (rank(catchersquare) > squarerank)
            {
               setSquare(candidateDefenders[WHITE][square], catchersquare);
            }

            if (rank(catchersquare) <= squarerank)
            {
               setSquare(candidateSupporters[WHITE][square], catchersquare);
            }

            if (rank(catchersquare) < squarerank)
            {
               setSquare(candidateDefenders[BLACK][square], catchersquare);
            }

            if (rank(catchersquare) >= squarerank)
            {
               setSquare(candidateSupporters[BLACK][square], catchersquare);
            }
         }

         if (abs(file(catchersquare) - squarefile) <= 1)
         {
            if (rank(catchersquare) >= squarerank)
            {
               setSquare(pawnOpponents[WHITE][square], catchersquare);
            }

            if (rank(catchersquare) <= squarerank)
            {
               setSquare(pawnOpponents[BLACK][square], catchersquare);
            }
         }
      }
   }

   ITERATE(square)
   {
      const int dDark =
         min(taxiDistance(square, A1), taxiDistance(square, H8));
      const int dLight =
         min(taxiDistance(square, A8), taxiDistance(square, H1));
      const int dStandard = centerDistance[square];

      kingChaseMalus[DARK][square] = 3 * (7 - dDark) + dStandard;
      kingChaseMalus[LIGHT][square] = 3 * (7 - dLight) + dStandard;
      kingChaseMalus[ALL][square] = 6 - min(dDark, dLight) +
         centerDistance[square];
   }

   /*
      dumpBoardValues(kingChaseMalus[DARK]);
      dumpBoardValues(kingChaseMalus[LIGHT]);
      dumpBoardValues(kingChaseMalus[ALL]);
      getKeyStroke();
    */

   initializePieceSquareValues();
   initializeKingAttacks();
   initializeKingSafetyTable();

   attackPoints[WHITE_KING] = 0;
   attackPoints[WHITE_QUEEN] = QUEEN_BONUS_ATTACK;
   attackPoints[WHITE_ROOK] = ROOK_BONUS_ATTACK;
   attackPoints[WHITE_BISHOP] = BISHOP_BONUS_ATTACK;
   attackPoints[WHITE_KNIGHT] = KNIGHT_BONUS_ATTACK;
   attackPoints[WHITE_PAWN] = 0;
   attackPoints[BLACK_KING] = 0;
   attackPoints[BLACK_QUEEN] = QUEEN_BONUS_ATTACK;
   attackPoints[BLACK_ROOK] = ROOK_BONUS_ATTACK;
   attackPoints[BLACK_BISHOP] = BISHOP_BONUS_ATTACK;
   attackPoints[BLACK_KNIGHT] = KNIGHT_BONUS_ATTACK;
   attackPoints[BLACK_PAWN] = 0;

   homeland[WHITE] = ( /* squaresOfRank[RANK_1] | */ squaresOfRank[RANK_2] |
                      squaresOfRank[RANK_3] | squaresOfRank[RANK_4]) &
      (squaresOfFile[FILE_C] | squaresOfFile[FILE_D] |
       squaresOfFile[FILE_E] | squaresOfFile[FILE_F]);
   /*
      homeland[WHITE] |= (squaresOfRank[RANK_1] |
      squaresOfRank[RANK_2] | squaresOfRank[RANK_3]) &
      (squaresOfFile[FILE_A] | squaresOfFile[FILE_B] |
      squaresOfFile[FILE_G] | squaresOfFile[FILE_H]);
    */
   homeland[BLACK] = getFlippedBitboard(homeland[WHITE]);

#ifdef GENERATE_TABLES
   generateKpkpTable();
#endif

   for (i = 0; i < 16; i++)
   {
      int j;

      for (j = 0; j < 16; j++)
      {
         piecePieceAttackBonus[i][j] = 0;
      }
   }

   piecePieceAttackBonus[WHITE_PAWN][BLACK_KNIGHT] = V(24, 27); /* tuned */
   piecePieceAttackBonus[WHITE_PAWN][BLACK_BISHOP] = V(24, 27); /* tuned */
   piecePieceAttackBonus[WHITE_PAWN][BLACK_ROOK] = V(29, 39);
   piecePieceAttackBonus[WHITE_PAWN][BLACK_QUEEN] = V(33, 46);

   piecePieceAttackBonus[WHITE_KNIGHT][BLACK_PAWN] = V(3, 15);
   piecePieceAttackBonus[WHITE_KNIGHT][BLACK_BISHOP] = V(9, 19);
   piecePieceAttackBonus[WHITE_KNIGHT][BLACK_ROOK] = V(16, 39); /* tuned */
   piecePieceAttackBonus[WHITE_KNIGHT][BLACK_QUEEN] = V(16, 39);        /* tuned */

   piecePieceAttackBonus[WHITE_BISHOP][BLACK_PAWN] = V(3, 15);
   piecePieceAttackBonus[WHITE_BISHOP][BLACK_KNIGHT] = V(9, 19);
   piecePieceAttackBonus[WHITE_BISHOP][BLACK_ROOK] = V(16, 39); /* tuned */
   piecePieceAttackBonus[WHITE_BISHOP][BLACK_QUEEN] = V(16, 39);        /* tuned */

   piecePieceAttackBonus[WHITE_ROOK][BLACK_PAWN] = V(2, 11);
   piecePieceAttackBonus[WHITE_ROOK][BLACK_KNIGHT] = V(5, 18);  /* egv tuned */
   piecePieceAttackBonus[WHITE_ROOK][BLACK_BISHOP] = V(5, 18);  /* egv tuned */
   piecePieceAttackBonus[WHITE_ROOK][BLACK_QUEEN] = V(9, 19);

   piecePieceAttackBonus[WHITE_QUEEN][BLACK_PAWN] = V(5, 16);   /* egv tuned */
   piecePieceAttackBonus[WHITE_QUEEN][BLACK_KNIGHT] = V(5, 16); /* egv tuned */
   piecePieceAttackBonus[WHITE_QUEEN][BLACK_BISHOP] = V(5, 16); /* egv tuned */
   piecePieceAttackBonus[WHITE_QUEEN][BLACK_ROOK] = V(5, 16);   /* egv tuned */

   for (i = 0; i < 16; i++)
   {
      int j;

      for (j = 0; j < 16; j++)
      {
         if (pieceColor(i) == BLACK)
         {
            const Color reversedColor = opponent(pieceColor(j));
            PieceType attacker = (PieceType) (pieceType(i) | WHITE);
            PieceType attackedPiece =
               (PieceType) (pieceType(j) | reversedColor);

            piecePieceAttackBonus[i][j] =
               piecePieceAttackBonus[attacker][attackedPiece];
         }
      }
   }

   troitzkyArea[WHITE] =
      passedPawnCorridor[WHITE][A3] | passedPawnCorridor[WHITE][B5] |
      passedPawnCorridor[WHITE][C3] | passedPawnCorridor[WHITE][D3] |
      passedPawnCorridor[WHITE][E3] | passedPawnCorridor[WHITE][F3] |
      passedPawnCorridor[WHITE][G5] | passedPawnCorridor[WHITE][H3];
   troitzkyArea[BLACK] = getFlippedBitboard(troitzkyArea[WHITE]);

   krprkDrawFiles = squaresOfFile[FILE_A] | squaresOfFile[FILE_B] |
      squaresOfFile[FILE_G] | squaresOfFile[FILE_H];
   A1C1 = minValue[A1] | minValue[C1], F1H1 = minValue[F1] | minValue[H1];
   A1B1 = minValue[A1] | minValue[B1], G1H1 = minValue[G1] | minValue[H1];

   initializeMaterialInfoTable();

   transposeMatrix(MALUS_PAWN_SQUARE_TYPE_HR, MALUS_PAWN_SQUARE_TYPE);
   transposeMatrix(MALUS_KING_SQUARE_HR, MALUS_KING_SQUARE);
   transposeMatrix(BONUS_KNIGHT_OUTPOST_HR, BONUS_KNIGHT_OUTPOST);
   transposeMatrix(BONUS_BISHOP_OUTPOST_HR, BONUS_BISHOP_OUTPOST);

   initializeMoveBonusValue();

   return 0;
}

#ifndef NDEBUG
bool flipTest(Position * position,
              PawnHashInfo * pawnHashtable,
              KingSafetyHashInfo * kingsafetyHashtable)
{
   int v1, v2;
   EvaluationBase base;

   initializePosition(position);
   v1 = getValue(position, &base, pawnHashtable, kingsafetyHashtable);

   flipPosition(position);
   initializePosition(position);
   v2 = getValue(position, &base, pawnHashtable, kingsafetyHashtable);

   flipPosition(position);
   initializePosition(position);

   if (v1 != v2)
   {
      const int debugFlag = debugOutput;
      const bool debugEvalFlag = debugEval;

      debugOutput = TRUE;
      debugEval = TRUE;

      logDebug("flip test failed: v1=%d v2=%d\n", v1, v2);
      logPosition(position);
      logDebug("hash: %llu\n", position->hashValue);
      getValue(position, &base, pawnHashtable, kingsafetyHashtable);
      flipPosition(position);
      initializePosition(position);
      logPosition(position);
      getValue(position, &base, pawnHashtable, kingsafetyHashtable);
      flipPosition(position);
      initializePosition(position);

      debugEval = debugEvalFlag;
      debugOutput = debugFlag;
   }

   return (bool) (v1 == v2);
}
#endif

static int testPawnInfoGeneration()
{
   Variation variation;
   EvaluationBase base;

   initializeVariation(&variation,
                       "8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - - 0 1");
   getPawnInfo(&variation.singlePosition, &base);

   assert(getNumberOfSetSquares(base.pawnProtectedSquares[WHITE]) == 8);
   assert(testSquare(base.pawnProtectedSquares[WHITE], B4));
   assert(testSquare(base.pawnProtectedSquares[WHITE], A3));
   assert(testSquare(base.pawnProtectedSquares[WHITE], C3));
   assert(testSquare(base.pawnProtectedSquares[WHITE], D4));
   assert(testSquare(base.pawnProtectedSquares[WHITE], F4));
   assert(testSquare(base.pawnProtectedSquares[WHITE], E5));
   assert(testSquare(base.pawnProtectedSquares[WHITE], G5));
   assert(testSquare(base.pawnProtectedSquares[WHITE], G3));

   assert(getNumberOfSetSquares(base.pawnProtectedSquares[BLACK]) == 7);
   assert(testSquare(base.pawnProtectedSquares[BLACK], B3));
   assert(testSquare(base.pawnProtectedSquares[BLACK], D3));
   assert(testSquare(base.pawnProtectedSquares[BLACK], C2));
   assert(testSquare(base.pawnProtectedSquares[BLACK], E2));
   assert(testSquare(base.pawnProtectedSquares[BLACK], E4));
   assert(testSquare(base.pawnProtectedSquares[BLACK], G4));
   assert(testSquare(base.pawnProtectedSquares[BLACK], G6));

   assert(getNumberOfSetSquares(base.passedPawns[WHITE]) == 0);
   assert(getNumberOfSetSquares(base.passedPawns[BLACK]) == 1);
   assert(testSquare(base.passedPawns[BLACK], D3));

   assert(getNumberOfSetSquares(base.weakPawns[WHITE]) == 3);
   assert(testSquare(base.weakPawns[WHITE], B2));
   assert(testSquare(base.weakPawns[WHITE], E3));
   assert(testSquare(base.weakPawns[WHITE], H2));

   assert(getNumberOfSetSquares(base.weakPawns[BLACK]) == 4);
   assert(testSquare(base.weakPawns[BLACK], A4));
   assert(testSquare(base.weakPawns[BLACK], C4));
   assert(testSquare(base.weakPawns[BLACK], F5));
   assert(testSquare(base.weakPawns[BLACK], H7));

   initializeVariation(&variation,
                       "4k3/2p5/p2p4/P2P4/1PP3p1/7p/7P/4K3 w - - 0 1");
   getPawnInfo(&variation.singlePosition, &base);

   assert(getNumberOfSetSquares(base.passedPawns[WHITE]) == 0);
   assert(getNumberOfSetSquares(base.passedPawns[BLACK]) == 0);
   assert(getNumberOfSetSquares(base.weakPawns[WHITE]) == 1);
   assert(testSquare(base.weakPawns[WHITE], H2));

   assert(getNumberOfSetSquares(base.weakPawns[BLACK]) == 3);
   assert(testSquare(base.weakPawns[BLACK], A6));
   assert(testSquare(base.weakPawns[BLACK], C7));
   assert(testSquare(base.weakPawns[BLACK], G4));

   return 0;
}

static int testWeakPawnDetection()
{
   Position position;
   Bitboard expectedResult = EMPTY_BITBOARD;
   EvaluationBase base;

   clearPosition(&position);
   position.piece[E1] = WHITE_KING;
   position.piece[E8] = BLACK_KING;
   position.piece[A3] = WHITE_PAWN;
   position.piece[B5] = WHITE_PAWN;
   position.piece[B6] = WHITE_PAWN;
   position.piece[C4] = WHITE_PAWN;
   position.piece[E4] = WHITE_PAWN;
   position.piece[G4] = WHITE_PAWN;
   position.piece[H2] = WHITE_PAWN;
   position.piece[A4] = BLACK_PAWN;
   position.piece[B7] = BLACK_PAWN;
   setSquare(expectedResult, A3);
   setSquare(expectedResult, E4);
   initializePosition(&position);
   getPawnInfo(&position, &base);
   assert(base.weakPawns[WHITE] == expectedResult);
   expectedResult = EMPTY_BITBOARD;
   setSquare(expectedResult, B7);
   assert(base.weakPawns[BLACK] == expectedResult);
   assert(base.candidatePawns[BLACK] == EMPTY_BITBOARD);

   position.piece[C4] = NO_PIECE;
   position.piece[C5] = WHITE_PAWN;
   initializePosition(&position);
   getPawnInfo(&position, &base);
   expectedResult = EMPTY_BITBOARD;
   setSquare(expectedResult, C5);
   assert(base.candidatePawns[WHITE] == expectedResult);

   clearPosition(&position);
   position.piece[E1] = WHITE_KING;
   position.piece[E8] = BLACK_KING;
   position.piece[A3] = WHITE_PAWN;
   position.piece[B5] = WHITE_PAWN;
   position.piece[B6] = WHITE_PAWN;
   position.piece[C4] = WHITE_PAWN;
   position.piece[E4] = WHITE_PAWN;
   position.piece[G4] = WHITE_PAWN;
   position.piece[H2] = WHITE_PAWN;
   position.piece[A4] = BLACK_PAWN;
   position.piece[B7] = BLACK_PAWN;
   position.piece[D6] = BLACK_PAWN;
   expectedResult = EMPTY_BITBOARD;
   setSquare(expectedResult, A3);
   setSquare(expectedResult, E4);
   setSquare(expectedResult, C4);
   initializePosition(&position);
   getPawnInfo(&position, &base);
   assert(base.weakPawns[WHITE] == expectedResult);
   assert(base.candidatePawns[WHITE] == EMPTY_BITBOARD);
   expectedResult = EMPTY_BITBOARD;
   setSquare(expectedResult, B7);
   setSquare(expectedResult, D6);
   assert(base.weakPawns[BLACK] == expectedResult);
   assert(base.candidatePawns[BLACK] == EMPTY_BITBOARD);

   position.piece[G5] = BLACK_PAWN;
   expectedResult = EMPTY_BITBOARD;
   setSquare(expectedResult, A3);
   setSquare(expectedResult, E4);
   setSquare(expectedResult, C4);
   setSquare(expectedResult, H2);
   initializePosition(&position);
   getPawnInfo(&position, &base);
   assert(base.weakPawns[WHITE] == expectedResult);
   assert(base.candidatePawns[WHITE] == EMPTY_BITBOARD);
   expectedResult = EMPTY_BITBOARD;
   setSquare(expectedResult, B7);
   setSquare(expectedResult, D6);
   setSquare(expectedResult, G5);
   assert(base.weakPawns[BLACK] == expectedResult);
   assert(base.candidatePawns[BLACK] == EMPTY_BITBOARD);

   return 0;
}

static int testBaseInitialization()
{
   Variation variation;

   initializeVariation(&variation, FEN_GAMESTART);

   assert(testSquare(passedPawnCorridor[WHITE][B4], B6));
   assert(testSquare(passedPawnCorridor[BLACK][B4], B6) == FALSE);
   assert(testSquare(passedPawnCorridor[WHITE][C2], H7) == FALSE);
   assert(testSquare(passedPawnCorridor[BLACK][G6], G2));

#ifndef NDEBUG
   {
      INT32 testBonus = evalBonus(-1, -1);

      assert(getOpeningValue(testBonus) == -1);
      assert(getEndgameValue(testBonus) == -1);
   }
#endif

   return 0;
}

/*
static int testFlippings()
{
   const char fen1[] =
      "2rr2k1/1b3ppp/pb2p3/1p2P3/1P2BPnq/P1N3P1/1B2Q2P/R4R1K b - - 0 1";
   const char fen2[] = "4k3/2p5/p2p4/P2P4/1PP3p1/7p/7P/4K3 w - - 0 1";
   const char fen3[] = "8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - - 0 1";
   const char fen4[] =
      "6r1/Q2Pn2k/p1p1P2p/5p2/2PqR1r1/1P6/P6P/5R1K b - - 5 4";
   const char fen5[] =
      "Q4rk1/2bb1ppp/4pn2/pQ5q/3P4/N4N2/5PPP/R1B2RK1 w - a6 0 4";
   Variation variation;

   initializeVariation(&variation, fen1);
   assert(flipTest(&variation.singlePosition) != FALSE);

   initializeVariation(&variation, fen2);
   assert(flipTest(&variation.singlePosition) != FALSE);

   initializeVariation(&variation, fen3);
   assert(flipTest(&variation.singlePosition) != FALSE);

   initializeVariation(&variation, fen4);
   assert(flipTest(&variation.singlePosition) != FALSE);

   initializeVariation(&variation, fen5);
   assert(flipTest(&variation.singlePosition) != FALSE);

   return 0;
}
*/

int testModuleEvaluation()
{
   int result;

   if ((result = testPawnInfoGeneration()) != 0)
   {
      return result;
   }

   if ((result = testWeakPawnDetection()) != 0)
   {
      return result;
   }

   if ((result = testBaseInitialization()) != 0)
   {
      return result;
   }

   /*
      if ((result = testFlippings()) != 0)
      {
      return result;
      } 
    */

   return 0;
}
