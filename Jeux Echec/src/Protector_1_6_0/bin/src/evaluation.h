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

#ifndef _evaluation_h_
#define _evaluation_h_

#include "position.h"
#include "bitboard.h"
#include "keytable.h"
#include "io.h"

#ifndef NDEBUG
extern bool debugEval;
#endif

#define MATERIALINFO_TABLE_SIZE ( 648 * 648 )
extern MaterialInfo materialInfo[MATERIALINFO_TABLE_SIZE];
extern Bitboard companionFiles[_64_];
extern Bitboard troitzkyArea[2];
extern Bitboard pawnOpponents[2][_64_];
extern Bitboard krprkDrawFiles;
extern Bitboard A1C1, F1H1, A1B1, G1H1;

#define VALUE_TEMPO_OPENING 20
#define VALUE_TEMPO_ENDGAME 10
#define MIN_PIECE_WEIGHT_FOR_KING_ATTACK 14

INLINE void addEvalBonusForColor(EvaluationBase * base, const Color color,
                                 const INT32 bonus);
INLINE void addEvalMalusForColor(EvaluationBase * base, const Color color,
                                 const INT32 bonus);
INLINE Color getWinningColor(const Position * position, const int value);
INLINE Bitboard getPromotablePawns(const Position * position,
                                   const Color color);
INLINE bool oppositeColoredBishops(const Position * position);
INLINE int getKnnkpChances(const Position * position, const Color color);
INLINE bool passiveKingStopsPawn(const Square kingSquare,
                                 const Square pawnSquare,
                                 const Color pawnColor);
INLINE int getKrppkrChances(const Position * position, const Color color);
INLINE int getKrpkrChances(const Position * position, const Color color);
INLINE int getKqppkqChances(const Position * position, const Color color);
INLINE int getKqpkqChances(const Position * position, const Color color);
INLINE int getKpkChances(const Position * position, const Color color);
INLINE int getKbpkChances(const Position * position, const Color color);
INLINE int specialPositionChances(const Position * position,
                                  const SpecialEvalType type,
                                  const Color color);
INLINE int getChances(const Position * position,
                      const MaterialInfo * mi, const Color winningColor);
INLINE bool hasBishopPair(const Position * position, const Color color);
INLINE int phaseValue(INT32 value, INT32 materialValue,
                      const Position * position, const MaterialInfo * mi);
INLINE INT32 materialBalance(const Position * position);
INLINE INT32 positionalBalance(const Position * position,
                               EvaluationBase * base);
INLINE int basicPositionalBalance(Position * position);
int getValue(const Position * position,
             EvaluationBase * base,
             PawnHashInfo * pawnHashtable,
             KingSafetyHashInfo * kingsafetyHashtable);
INLINE bool hasWinningPotential(Position * position, Color color);
INLINE Bitboard getKingPawnSafetyHashValue(const Position * position,
                                           const Color color);
INLINE int getPawnWidth(const Position * position, const Color color);
INLINE int getPassedPawnWidth(const Position * position,
                              const EvaluationBase * base, const Color color);
INLINE bool kpkpValueAvailable(const Position * position);
INLINE int getMaterialUpPawnCountWeight(int numPawns);

#ifdef INLINE_IN_HEADERS
#include "evaluationInline.h"
#endif

/**
 * Calculate the value of the specified position.
 *
 * @return the value of the specified position
 */
int getValue(const Position * position,
             EvaluationBase * base,
             PawnHashInfo * pawnHashtable,
             KingSafetyHashInfo * kingsafetyHashtable);

/**
 * Check if the pawn at the specified square is a passed pawn.
 */
bool pawnIsPassed(const Position * position, const Square pawnSquare,
                  const Color pawnColor);

/**
 * Check if a pawn capture creates at least one passer.
 */
bool captureCreatesPasser(Position * position, const Square captureSquare,
                          const Piece capturingPiece);

/**
 * Reset the pawn hashtable.
 */
void resetPawnHashtable(void);

/**
 * Flip the given position and check if it yields the same result.
 *
 * @return FALSE if the flipped position yields a diffent result
 */
bool flipTest(Position * position, PawnHashInfo * pawnHashtable,
              KingSafetyHashInfo * kingsafetyHashtable);

/**
 * Initialize this module.
 *
 * @return 0 if no errors occurred.
 */
int initializeModuleEvaluation(void);

/**
 * Test this module.
 *
 * @return 0 if all tests succeed.
 */
int testModuleEvaluation(void);

#endif
