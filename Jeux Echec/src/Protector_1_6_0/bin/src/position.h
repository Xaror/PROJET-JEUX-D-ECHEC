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

#ifndef _position_h_
#define _position_h_

#include "protector.h"
#include "bitboard.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_DEPTH 64
#define MAX_DEPTH_ARRAY_SIZE (MAX_DEPTH + 2)
#define POSITION_HISTORY_OFFSET 100
#define MAX_GAIN 512
#define MAX_GAIN_MOVESORT 127

#define DEFAULT_HASHTABLE_EXPONENT   16

#define SEARCHEVENT_SEARCH_FINISHED    1
#define SEARCHEVENT_PLY_FINISHED       2
#define SEARCHEVENT_NEW_PV             3
#define SEARCHEVENT_NEW_BASEMOVE       4
#define SEARCHEVENT_STATISTICS_UPDATE  5

#define PAWN_DEFAULTVALUE_OPENING      (80)
#define PAWN_DEFAULTVALUE_ENDGAME     (125)

#define PHASE_MAX 62
#define PHASE_INDEX_MAX 256
#define PIECEWEIGHT_ENDGAME 0

#define DEFAULTVALUE_KNIGHT_OPENING     (331)
#define DEFAULTVALUE_KNIGHT_ENDGAME     (417)
#define DEFAULTVALUE_BISHOP_OPENING     (329)
#define DEFAULTVALUE_BISHOP_ENDGAME     (428)
#define DEFAULTVALUE_ROOK_OPENING       (518)
#define DEFAULTVALUE_ROOK_ENDGAME       (652)
#define DEFAULTVALUE_QUEEN_OPENING      (975)
#define DEFAULTVALUE_QUEEN_ENDGAME     (1312)

#define DEFAULTVALUE_BISHOP_PAIR_OPENING  40
#define DEFAULTVALUE_BISHOP_PAIR_ENDGAME  61
#define DEFAULTVALUE_PIECE_UP_OPENING  19
#define DEFAULTVALUE_PIECE_UP_ENDGAME   4

extern int VALUE_QUEEN_OPENING;
extern int VALUE_QUEEN_ENDGAME;
extern int VALUE_ROOK_OPENING;
extern int VALUE_ROOK_ENDGAME;
extern int VALUE_BISHOP_OPENING;
extern int VALUE_BISHOP_ENDGAME;
extern int VALUE_KNIGHT_OPENING;
extern int VALUE_KNIGHT_ENDGAME;
extern int PAWN_VALUE_OPENING;
extern int PAWN_VALUE_ENDGAME;
extern int VALUE_BISHOP_PAIR_OPENING;
extern int VALUE_BISHOP_PAIR_ENDGAME;

#define OPENING 0
#define ENDGAME 1

extern int basicValue[16], simplePieceValue[16];
extern INT32 pieceSquareBonus[16][_64_];
extern int pieceCountShift[16];
extern int krqIndexWhite[4096], bbpIndexWhite[4096];
extern int krqIndexBlack[4096], bbpIndexBlack[4096];
extern const int materialUpPawnCountWeight[9];

typedef struct
{
   Piece piece[_64_];
   Color activeColor;
   BYTE castlingRights;
   Square enPassantSquare;
   int moveNumber, halfMoveClock;

   /**
    * Redundant data
    */
   Bitboard allPieces;
   Bitboard piecesOfColor[2];
   Bitboard piecesOfType[16];
   Square king[2];
   int numberOfPieces[2];
   int numberOfPawns[2];
   UINT64 pieceCount;
   INT32 balance;
   UINT64 hashValue, pawnHashValue;
}
Position;

typedef UINT32 Move;

typedef struct
{
   Square square;
   Bitboard moves;
}
MovesOfPiece;

typedef struct
{
   Bitboard diaAttackers, orthoAttackers, knightAttackers, pawnAttackers[2];
   Square attackedByDia[_64_], attackedByOrtho[_64_];
}
KingAttacks;

typedef struct
{
   Move killerMove1, killerMove2, killerMove3, killerMove4;
   Move currentMove;
   int indexCurrentMove;
   bool currentMoveIsCheck;
   Piece captured, restorePiece1, restorePiece2;
   Square enPassantSquare, restoreSquare1, restoreSquare2, kingSquare;
   BYTE castlingRights;
   int halfMoveClock;
   Bitboard allPieces, whitePieces, blackPieces, hashValue, pawnHashValue;
   INT32 balance;
   int staticValue, refinedStaticValue, futilityMargin;
   bool staticValueAvailable, gainsUpdated;
   bool quietMove;
   UINT64 pieceCount;
}
PlyInfo;

typedef struct
{
   Position *position;
   PlyInfo *plyInfo;
   Move hashMove;
   Move moves[MAX_MOVES_PER_POSITION];
   Move badCaptures[MAX_MOVES_PER_POSITION];
   bool killer1Executed, killer2Executed, killer3Executed, killer4Executed;
   MovesOfPiece movesOfPiece[16];
   int numberOfMoves, numberOfBadCaptures;
   int nextMove, currentStage, numberOfPieces;
   UINT16 *historyValue;
   INT16 *positionalGain;
}
Movelist;

#define HASHVALUE_UPPER_LIMIT 0
#define HASHVALUE_EXACT 1
#define HASHVALUE_LOWER_LIMIT 2
#define HASHVALUE_EVAL 3

typedef struct
{
   UINT64 key;
   UINT64 data;
   UINT32 staticValueData;
}
Hashentry;

typedef struct
{
   Hashentry *table;
   unsigned long tableSize, entriesUsed;
   UINT64 hashMask;
   int exponent;
   UINT8 date;
}
Hashtable;

typedef enum
{
   SEARCH_STATUS_RUNNING,
   SEARCH_STATUS_TERMINATE,
   SEARCH_STATUS_ABORT,
   SEARCH_STATUS_FINISHED
}
SearchStatus;

#define HISTORY_SIZE (12*64*64)
#define HISTORY_MAX  16384
#define HISTORY_LIMIT 60        /* (60%) */
#define PAWN_HASHTABLE_MASK 0xffff
#define PAWN_HASHTABLE_SIZE ((PAWN_HASHTABLE_MASK)+0x0001)
#define KINGSAFETY_HASHTABLE_MASK 0x07ffff
#define KINGSAFETY_HASHTABLE_SIZE ((KINGSAFETY_HASHTABLE_MASK)+0x0001)

typedef struct
{
   UINT16 move[MAX_DEPTH_ARRAY_SIZE];
   int length, score;
}
PrincipalVariation;

typedef struct
{
   Bitboard hashValue;
   Bitboard pawnProtectedSquares[2];
   Bitboard passedPawns[2];
   Bitboard pawnAttackableSquares[2];
   bool hasPassersOrCandidates[2];
   INT32 balance;
   int pawnLightSquareMalus[2], pawnDarkSquareMalus[2];
}
PawnHashInfo;

typedef struct
{
   Bitboard hashValue;
   int safetyMalus;
}
KingSafetyHashInfo;

extern KingSafetyHashInfo
   kingSafetyHashtable[MAX_THREADS][KINGSAFETY_HASHTABLE_SIZE];

#define BONUS_HIDDEN_PASSER

typedef enum
{
   Se_None,
   Se_KpK,
   Se_KnnKp,
   Se_KbpK,
   Se_KrpKb,
   Se_KrpKr,
   Se_KrppKr,
   Se_KqpKq,
   Se_KqppKq
} SpecialEvalType;

typedef struct
{
   UINT8 chancesWhite, chancesBlack;
   SpecialEvalType specialEvalWhite, specialEvalBlack;
   INT32 materialBalance;
   int phaseIndex;
}
MaterialInfo;

typedef struct
{
   Bitboard upwardRealm[2];
   Bitboard downwardRealm[2];
   Bitboard pawnAttackableSquares[2];
   Bitboard pawnProtectedSquares[2];
   Bitboard doubledPawns[2];
   Bitboard passedPawns[2];
   Bitboard candidatePawns[2];

#ifdef BONUS_HIDDEN_PASSER
   Bitboard hiddenCandidatePawns[2];
   bool hasPassersOrCandidates[2];
#endif

#ifdef MALUS_SLIGHTLY_BACKWARD
   Bitboard slightlyBackward[2];
#endif

   Bitboard weakPawns[2];
   Bitboard fixedPawns[2];
   Bitboard countedSquares[2];
   Bitboard unprotectedPieces[2];
   Bitboard kingAttackSquares[2];
   Bitboard attackedSquares[2];
   Bitboard knightAttackedSquares[2];
   Bitboard bishopAttackedSquares[2];
   Bitboard rookAttackedSquares[2];
   Bitboard rookSupportedSquares[2];
   Bitboard queenAttackedSquares[2];
   Bitboard queenSupportedSquares[2];
   Bitboard chainPawns[2];
   INT32 attackInfo[2];
   int kingSquaresAttackCount[2];
   Bitboard hangingPieces[2];
   int spaceAttackPoints[2];
   int pawnLightSquareMalus[2];
   int pawnDarkSquareMalus[2];
   bool evaluateKingSafety[2];
   KingSafetyHashInfo *kingsafetyHashtable;
   INT32 balance, materialBalance;
   int futilityMargin[2];
   MaterialInfo *materialInfo;
   Color ownColor;
}
EvaluationBase;

typedef struct
{
   int ply, nominalDepth, selDepth;
   int numberOfBaseMoves, numberOfCurrentBaseMove;
   Move currentBaseMove, bestBaseMove;
   Position singlePosition, startPosition;
   PlyInfo plyInfo[MAX_DEPTH_ARRAY_SIZE];
   PrincipalVariation pv;
   Hashtable *hashtable;
   PawnHashInfo *pawnHashtable;
   KingSafetyHashInfo *kingsafetyHashtable;
   UINT64 positionHistory[POSITION_HISTORY_OFFSET + MAX_DEPTH_ARRAY_SIZE];
   UINT64 nodes, nodesAtTimeCheck, nodesBetweenTimecheck;
   UINT16 *historyValue;
   INT16 *positionalGain;
   long startTime, timeTarget, timeLimit, finishTime, timestamp;
   long startTimeProcess, finishTimeProcess;
   unsigned long tbHits;
   void (*handleSearchEvent) (int, void *);
   SearchStatus searchStatus;
   int drawScore[2];
   bool easyMove, bestMoveChange;
   int bestMoveChangeCount[MAX_DEPTH_ARRAY_SIZE];
   bool terminate, ponderMode, terminatePondering, failingHighOrLow;
   int previousBest, expectedScore;
   long numPvUpdates;
   unsigned int threadNumber;
}
Variation;

#define TIME_CHECK_INTERVALL_IN_MS 100
#define NODES_BETWEEN_TIMECHECK_DEFAULT 25000

#define V(op,eg) ( (op) + ( (eg) << 16 ) )
#define HV(op,eg) ( (((op)*100)/256) + ( (((eg)*100)/256) << 16 ) )

/**
 * Flip the specified position.
 */
void flipPosition(Position * position);

/**
 * Remove all pieces from the specified position.
 */
void clearPosition(Position * position);

/**
 * Calculate the redundant data of the specified position.
 */
void initializePosition(Position * position);

/**
 * Prepare a search with the specified variation.
 */
void prepareSearch(Variation * variation);

/**
 * Reset history values.
 */
void resetHistoryValues(Variation * variation);

/**
 * Reset history hit values.
 */
void resetHistoryHitValues(Variation * variation);

/**
 * Reset gain values.
 */
void resetGainValues(Variation * variation);

/**
 * Shrink all history values.
 */
void shrinkHistoryValues(Variation * variation);

/**
 * Shrink all history hit values.
 */
void shrinkHistoryHitValues(Variation * variation);

/**
 * Initialize a variation with the position specified by 'fen'.
 * Prepare a search.
 */
void initializeVariation(Variation * variation, const char *fen);

/**
 * Initialize a variation with the position specified by 'position'.
 * To perform a search, call 'prepareSearch' afterwards.
 */
void setBasePosition(Variation * variation, const Position * position);

/**
 * Set the value of a definite draw.
 */
void setDrawScore(Variation * variation, int score, Color color);

/**
 * Get the pieces interested in moving to 'square'.
 */
Bitboard getInterestedPieces(const Position * position, const Square square,
                             const Color attackerColor);

/**
 * Make the specified move at the current end of the specified variation.
 *
 * @return 1 if the move was an illegal castling move, 0 otherwise
 */
int makeMove(Variation * variation, const Move move);

/**
 * Make the specified move at the current end of the specified variation.
 *
 * @return 1 if the move was an illegal castling move, 0 otherwise
 */
int makeMoveFast(Variation * variation, const Move move);

/**
 * Unmake the last move.
 */
void unmakeLastMove(Variation * variation);

/**
 * Test if a move attacks the opponent's king.
 *
 * @return FALSE if the specified move doesn't attack
 *         the opponent's king, TRUE otherwise
 */
bool moveIsCheck(const Move move, const Position * position);

/**
 * Check if the specified position is consistent.
 *
 * @return 0 if the specified position is consistent
 */
int checkConsistency(const Position * position);

/**
 * Check if the specified variation is consistent.
 *
 * @return 0 if the specified variation is consistent
 */
int checkVariation(Variation * variation);

/**
 * Check if the given position is legal.
 */
bool positionIsLegal(const Position * position);

/**
 * Check if the specified position are identical (excluding move numbers).
 */
bool positionsAreIdentical(const Position * position1,
                           const Position * position2);

/**
 * Calculate the signature for a given set of piece counters.
 */
UINT32 materialSignature(const UINT32 numQueens, const UINT32 numRooks,
                         const UINT32 numLightSquareBishops,
                         const UINT32 numDarkSquareBishops,
                         const UINT32 numKnights, const UINT32 numPawns);

INLINE Square relativeSquare(const Square square, const Color color);
INLINE INT32 evalBonus(INT32 openingBonus, INT32 endgameBonus);
INLINE int getOpeningValue(INT32 value);
INLINE int getEndgameValue(INT32 value);
INLINE void addBonusForColor(const INT32 bonus, Position * position,
                             const Color color);
INLINE UINT16 packedMove(const Move move);
INLINE Move getMove(const Square from, const Square to,
                    const Piece newPiece, const INT16 value);
INLINE Move getOrdinaryMove(const Square from, const Square to);
INLINE Move getPackedMove(const Square from, const Square to,
                          const Piece newPiece);
INLINE Square getFromSquare(const Move move);
INLINE Square getToSquare(const Move move);
INLINE Piece getNewPiece(const Move move);
INLINE INT16 getMoveValue(const Move move);
INLINE void setMoveValue(Move * move, const int value);
INLINE Color opponent(Color color);
INLINE Bitboard getDirectAttackers(const Position * position,
                                   const Square square,
                                   const Color attackerColor,
                                   const Bitboard obstacles);
INLINE Bitboard getDiaSquaresBehind(const Position * position,
                                    const Square targetSquare,
                                    const Square viewPoint);
INLINE Bitboard getOrthoSquaresBehind(const Position * position,
                                      const Square targetSquare,
                                      const Square viewPoint);
INLINE void initializePlyInfo(Variation * variation);
INLINE int numberOfNonPawnPieces(const Position * position,
                                 const Color color);
INLINE bool hasOrthoPieces(const Position * position, const Color color);
INLINE bool hasQueen(const Position * position, const Color color);
INLINE void appendMoveToPv(const PrincipalVariation * oldPv,
                           PrincipalVariation * newPv, const Move move);
INLINE INT16 calcHashtableValue(const int value, const int ply);
INLINE int calcEffectiveValue(const int value, const int ply);
INLINE Bitboard getOrdinaryPieces(const Position * position,
                                  const Color color);
INLINE Bitboard getNonPawnPieces(const Position * position,
                                 const Color color);
INLINE Bitboard getOrthoPieces(const Position * position, const Color color);
INLINE bool movesAreEqual(const Move m1, const Move m2);
INLINE int getPieceCount(const Position * position, const Piece piece);
INLINE bool pieceIsPresent(const Position * position, const Piece piece);
INLINE int historyIndex(const Move move, const Position * position);
INLINE int moveIndex(const Move move, const Position * position);
INLINE int moveGainIndex(const Move move, const Position * position);
INLINE int getMinimalDistance(const Position * position,
                              const Square origin, const Piece piece);
INLINE int getMinimalTaxiDistance(const Position * position,
                                  const Square origin, const Piece piece);
INLINE int getPieceWeight(const Position * position, const Color color);
INLINE int phaseIndex(const Position * position);
INLINE void getPieceCounters(UINT32 materialSignature,
                             int *numWhiteQueens, int *numWhiteRooks,
                             int *numWhiteLightSquareBishops,
                             int *numWhiteDarkSquareBishops,
                             int *numWhiteKnights, int *numWhitePawns,
                             int *numBlackQueens, int *numBlackRooks,
                             int *numBlackLightSquareBishops,
                             int *numBlackDarkSquareBishops,
                             int *numBlackKnights, int *numBlackPawns);
INLINE UINT32 calculateMaterialSignature(const Position * position);
INLINE UINT32 bilateralSignature(const UINT32 signatureWhite,
                                 const UINT32 signatureBlack);
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
                                   const int numBlackPawns);
INLINE UINT32 getSingleMaterialSignature(const int numQueens,
                                         const int numRooks,
                                         const int numLightSquareBishops,
                                         const int numDarkSquareBishops,
                                         const int numKnights,
                                         const int numPawns);
INLINE UINT32 getMaterialSignatureFromPieceCount(const Position * position);
INLINE Color getOwnColor(Variation * variation);

#ifdef INLINE_IN_HEADERS
#include "positionInline.h"
#endif

/**
 * Initialize this module.
 *
 * @return 0 if no errors occurred.
 */
int initializeModulePosition(void);

/**
 * Test this module.
 *
 * @return 0 if all tests succeed.
 */
int testModulePosition(void);

#endif
