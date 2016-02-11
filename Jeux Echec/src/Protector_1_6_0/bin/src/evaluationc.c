#undef addBonus
#undef addMalus
#undef COLOR
#undef OPPCOLOR

#ifdef PERSPECTIVE_WHITE
#define COLOR WHITE
#define OPPCOLOR BLACK
#define addBonus(base, bonus) (base->balance += bonus);
#define addMalus(base, bonus) (base->balance -= bonus);
#else
#define COLOR BLACK
#define OPPCOLOR WHITE
#define addBonus(base, bonus) (base->balance -= bonus);
#define addMalus(base, bonus) (base->balance += bonus);
#endif

#ifdef PERSPECTIVE_WHITE
INLINE static void addWhitePieceAttackBonus(const Position * position,
                                            EvaluationBase * base,
                                            const Bitboard moves,
                                            const Bitboard candidateTargets,
                                            const Piece attacker)
#else
INLINE static void addBlackPieceAttackBonus(const Position * position,
                                            EvaluationBase * base,
                                            const Bitboard moves,
                                            const Bitboard candidateTargets,
                                            const Piece attacker)
#endif
{
   Bitboard targets = moves & candidateTargets;
   Square targetSquare;

   ITERATE_BITBOARD(&targets, targetSquare)
   {
      const Piece target = position->piece[targetSquare];

      addBonus(base, piecePieceAttackBonus[attacker][target]);
   }
}

#ifdef PERSPECTIVE_WHITE
INLINE static void evaluateWhiteAttackers(EvaluationBase * base)
#else
INLINE static void evaluateBlackAttackers(EvaluationBase * base)
#endif
{
   static const INT32 multipleAttackersBonus = V(15, 24);

   if (getNumberOfSetSquares(base->hangingPieces[OPPCOLOR]) >= 2)
   {
      addBonus(base, multipleAttackersBonus);
   }
}

#ifdef PERSPECTIVE_WHITE
INLINE static void evaluateWhiteKnight(const Position * position,
                                       EvaluationBase * base,
                                       const Square square)
#else
INLINE static void evaluateBlackKnight(const Position * position,
                                       EvaluationBase * base,
                                       const Square square)
#endif
{
   const Bitboard moves = getKnightMoves(square);
   const Bitboard mobSquares = base->countedSquares[COLOR] |
      getNonPawnPieces(position, OPPCOLOR);
   const int mobilityCount = getNumberOfSetSquares(moves & mobSquares);
   const Bitboard ctm = position->piecesOfType[PAWN | OPPCOLOR] |
      position->piecesOfType[BISHOP | OPPCOLOR];
   const Bitboard cth = position->piecesOfType[ROOK | OPPCOLOR] |
      position->piecesOfType[QUEEN | OPPCOLOR];
   const Bitboard candidateTargets = cth |
      (ctm & base->unprotectedPieces[OPPCOLOR]);

#ifdef PERSPECTIVE_WHITE
   const Square relativeSquare = square;
#else
   const Square relativeSquare = getFlippedSquare(square);
#endif

   base->knightAttackedSquares[COLOR] |= moves;

   if (position->piecesOfType[KNIGHT | OPPCOLOR] == EMPTY_BITBOARD &&
       position->piecesOfType[BISHOP | OPPCOLOR] != EMPTY_BITBOARD &&
       hasAttackingBishop(position, OPPCOLOR, square) == FALSE)
   {
      addBonus(base, V(2, 2));
   }

   if (squareIsPawnSafe(base, COLOR, square) &&
       BONUS_KNIGHT_OUTPOST[relativeSquare] > 0)
   {
      int bonusValue = BONUS_KNIGHT_OUTPOST[relativeSquare];

      if (testSquare(base->pawnProtectedSquares[COLOR], square))
      {
         if (position->piecesOfType[KNIGHT | OPPCOLOR] == EMPTY_BITBOARD &&
             hasAttackingBishop(position, OPPCOLOR, square) == FALSE)
         {
            bonusValue += bonusValue + bonusValue / 2;
         }
         else
         {
            bonusValue += bonusValue;
         }
      }

      addBonus(base, V(bonusValue, bonusValue));
   }

   addBonus(base, KnightMobilityBonus[mobilityCount]);

   if (base->evaluateKingSafety[OPPCOLOR] &&
       (moves & base->kingAttackSquares[OPPCOLOR]) != EMPTY_BITBOARD)
   {
      const Bitboard coronaAttacks =
         moves & getKingMoves(position->king[OPPCOLOR]);

      base->kingSquaresAttackCount[COLOR] +=
         getNumberOfSetSquares(coronaAttacks);
      base->attackInfo[COLOR] += V(KNIGHT_BONUS_ATTACK, 1);
   }

#ifdef PERSPECTIVE_WHITE
   addWhitePieceAttackBonus(position, base, moves, candidateTargets,
                            WHITE_KNIGHT);
#else
   addBlackPieceAttackBonus(position, base, moves, candidateTargets,
                            BLACK_KNIGHT);
#endif

   if (testSquare(base->pawnProtectedSquares[OPPCOLOR], square))
   {
      addMalus(base, piecePieceAttackBonus[PAWN | OPPCOLOR][KNIGHT | COLOR]);
      base->hangingPieces[COLOR] |= minValue[square];
   }

   base->hangingPieces[OPPCOLOR] |= (moves & cth);
}

#ifdef PERSPECTIVE_WHITE
INLINE static void evaluateWhiteBishop(const Position * position,
                                       EvaluationBase * base,
                                       const Square square)
#else
INLINE static void evaluateBlackBishop(const Position * position,
                                       EvaluationBase * base,
                                       const Square square)
#endif
{
   const Bitboard xrayPieces = position->piecesOfType[QUEEN | COLOR];
   const Bitboard moves =
      getMagicBishopMoves(square, position->allPieces & ~xrayPieces);
   const Bitboard mobSquares = base->countedSquares[COLOR] |
      getNonPawnPieces(position, OPPCOLOR);
   const int mobilityCount = getNumberOfSetSquares(moves & mobSquares);
   Bitboard squareColorTargets = position->piecesOfType[PAWN | OPPCOLOR] &
      squaresBelow[COLOR][square] & base->unprotectedPieces[OPPCOLOR];
   const Bitboard ctm = position->piecesOfType[PAWN | OPPCOLOR] |
      position->piecesOfType[KNIGHT | OPPCOLOR];
   const Bitboard cth = position->piecesOfType[ROOK | OPPCOLOR] |
      position->piecesOfType[QUEEN | OPPCOLOR];
   const Bitboard candidateTargets = cth |
      (ctm & base->unprotectedPieces[OPPCOLOR]);

#ifdef PERSPECTIVE_WHITE
   const Square relativeSquare = square;
#else
   const Square relativeSquare = getFlippedSquare(square);
#endif

   if (testSquare(lightSquares, square))
   {
      const INT32 bonus = ((base->pawnLightSquareMalus[OPPCOLOR] / 2) -
                           base->pawnLightSquareMalus[COLOR]) * V(1, 1);

      addBonus(base, bonus);

      squareColorTargets &= lightSquares;
   }
   else
   {
      const INT32 bonus = ((base->pawnDarkSquareMalus[OPPCOLOR] / 2) -
                           base->pawnDarkSquareMalus[COLOR]) * V(1, 1);

      addBonus(base, bonus);

      squareColorTargets &= darkSquares;
   }

   addBonus(base, getNumberOfSetSquares(squareColorTargets) * V(0, 2));

   base->bishopAttackedSquares[COLOR] |= moves;

   if (squareIsPawnSafe(base, COLOR, square) &&
       BONUS_BISHOP_OUTPOST[relativeSquare] > 0)
   {
      int bonusValue = BONUS_BISHOP_OUTPOST[relativeSquare];

      if (testSquare(base->pawnProtectedSquares[COLOR], square))
      {
         if (position->piecesOfType[KNIGHT | OPPCOLOR] == EMPTY_BITBOARD &&
             hasAttackingBishop(position, OPPCOLOR, square) == FALSE)
         {
            bonusValue += bonusValue + bonusValue / 2;
         }
         else
         {
            bonusValue += bonusValue;
         }
      }

      addBonus(base, V(bonusValue, bonusValue));
   }

#ifdef PERSPECTIVE_WHITE
   addWhitePieceAttackBonus(position, base, moves, candidateTargets,
                            WHITE_BISHOP);
#else
   addBlackPieceAttackBonus(position, base, moves, candidateTargets,
                            BLACK_BISHOP);
#endif

   if (testSquare(base->pawnProtectedSquares[OPPCOLOR], square))
   {
      addMalus(base, piecePieceAttackBonus[PAWN | OPPCOLOR][BISHOP | COLOR]);
      base->hangingPieces[COLOR] |= minValue[square];
   }

   base->hangingPieces[OPPCOLOR] |= (moves & cth);
   addBonus(base, BishopMobilityBonus[mobilityCount]);

   if (base->evaluateKingSafety[OPPCOLOR])
   {
      if ((moves & base->kingAttackSquares[OPPCOLOR]) != EMPTY_BITBOARD)
      {
         const Bitboard coronaAttacks =
            moves & getKingMoves(position->king[OPPCOLOR]);

         base->kingSquaresAttackCount[COLOR] +=
            getNumberOfSetSquares(coronaAttacks);
         base->attackInfo[COLOR] += V(BISHOP_BONUS_ATTACK, 1);
      }
   }
}

#ifdef PERSPECTIVE_WHITE
INLINE static void evaluateWhiteRook(const Position * position,
                                     EvaluationBase * base,
                                     const Square square)
#else
INLINE static void evaluateBlackRook(const Position * position,
                                     EvaluationBase * base,
                                     const Square square)
#endif
{
   const Piece piece = position->piece[square];
   const Bitboard moves = getMagicRookMoves(square, position->allPieces);
   const Bitboard xrayPieces = position->piecesOfType[QUEEN | COLOR] |
      position->piecesOfType[piece];
   const Bitboard xrayMoves =
      getMagicRookMoves(square, position->allPieces & ~xrayPieces);
   const Bitboard mobSquares = base->countedSquares[COLOR];
   const int mobilityCount = getNumberOfSetSquares(xrayMoves & mobSquares);
   const Bitboard fileSquares = squaresOfFile[file(square)];
   const Bitboard ownPawns = position->piecesOfType[PAWN | COLOR];
   const Rank relativeRank = colorRank(COLOR, square);
   const Bitboard ctm = position->piecesOfType[PAWN | OPPCOLOR] |
      position->piecesOfType[KNIGHT | OPPCOLOR] |
      position->piecesOfType[BISHOP | OPPCOLOR];
   const Bitboard cth = position->piecesOfType[QUEEN | OPPCOLOR];
   const Bitboard candidateTargets = cth |
      (ctm & base->unprotectedPieces[OPPCOLOR]);

   base->rookAttackedSquares[COLOR] |= moves;
   base->rookSupportedSquares[COLOR] |= xrayMoves & ~moves;

#ifdef BONUS_ROOK_OUTPOST
   if (squareIsPawnSafe(base, COLOR, square) &&
       relativeRank >= RANK_4 && relativeRank <= RANK_6 &&
       testSquare(border, square) == FALSE &&
       testSquare(base->pawnProtectedSquares[COLOR], square))
   {
      Bitboard targets = base->kingAttackSquares[OPPCOLOR] |
         (squaresOfRank[rank(square)] & base->unprotectedPieces[OPPCOLOR]);

      addBonus(base, V(1, 2));

      if ((moves & targets) != EMPTY_BITBOARD)
      {
         addBonus(base, V(3, 4));
      }
   }
#endif

#ifdef PERSPECTIVE_WHITE
   addWhitePieceAttackBonus(position, base, moves, candidateTargets,
                            WHITE_ROOK);
#else
   addBlackPieceAttackBonus(position, base, moves, candidateTargets,
                            BLACK_ROOK);
#endif

   if (testSquare(base->pawnProtectedSquares[OPPCOLOR], square))
   {
      addMalus(base, piecePieceAttackBonus[PAWN | OPPCOLOR][ROOK | COLOR]);
      base->hangingPieces[COLOR] |= minValue[square];
   }

   base->hangingPieces[OPPCOLOR] |= (moves & cth);

   addBonus(base, RookMobilityBonus[mobilityCount]);

   /* Add a bonus if this rook is located on an open file. */
   if ((ownPawns & fileSquares) == EMPTY_BITBOARD)
   {
      const Bitboard oppPawns = position->piecesOfType[PAWN | OPPCOLOR];
      const Bitboard frontSquares = fileSquares & squaresAbove[COLOR][square];
      Bitboard protectedBlockers = frontSquares &
         base->pawnProtectedSquares[OPPCOLOR] &
         (oppPawns | position->piecesOfType[OPPCOLOR | KNIGHT] |
          position->piecesOfType[OPPCOLOR | BISHOP]);

      if (protectedBlockers == EMPTY_BITBOARD)
      {
         if ((frontSquares & oppPawns) == EMPTY_BITBOARD)
         {
            addBonus(base, V(20, 20));
         }
         else
         {
            addBonus(base, V(10, 10));
         }
      }
      else
      {
         if ((protectedBlockers & oppPawns) != EMPTY_BITBOARD)
         {
            addBonus(base, V(8, 0));
         }
         else
         {
#ifdef PERSPECTIVE_WHITE
            const Square minorSquare = getFirstSquare(&protectedBlockers);
#else
            const Square minorSquare = getLastSquare(&protectedBlockers);
#endif
            if (squareIsPawnSafe(base, OPPCOLOR, minorSquare))
            {
               addBonus(base, V(10, 0));
            }
            else
            {
               addBonus(base, V(15, 5));
            }
         }
      }
   }
   else if (testSquare(kingTrapsRook[COLOR][position->king[COLOR]], square) &&
            (moves & centralFiles) == EMPTY_BITBOARD)
   {
      const int basicMalus = max(0, 51 - 9 * mobilityCount);

      addMalus(base, V(basicMalus, 0));
   }

   if (grantSeventhRankBonus(position, COLOR, OPPCOLOR, square))
   {
      addBonus(base, V(10, 30));

      if (colorRank(COLOR, position->king[OPPCOLOR]) == RANK_8)
      {
         Bitboard companions = position->piecesOfType[ROOK | COLOR] |
            position->piecesOfType[QUEEN | COLOR];

         if ((moves & companions & squaresOfRank[rank(square)]) !=
             EMPTY_BITBOARD)
         {
            addBonus(base, V(10, 20));
         }
      }
   }

   if (base->evaluateKingSafety[OPPCOLOR])
   {
      if ((xrayMoves & base->kingAttackSquares[OPPCOLOR]) != EMPTY_BITBOARD)
      {
         const Bitboard coronaAttacks =
            xrayMoves & getKingMoves(position->king[OPPCOLOR]);

         base->kingSquaresAttackCount[COLOR] +=
            getNumberOfSetSquares(coronaAttacks);
         base->attackInfo[COLOR] += V(ROOK_BONUS_ATTACK, 1);
      }
   }

   /* Give a malus for a rook blocking his own passer on the 7th rank */
   if (numberOfNonPawnPieces(position, COLOR) == 2 &&
       colorRank(COLOR, square) == RANK_8 &&
       testSquare(base->passedPawns[COLOR],
                  (Square) downward(COLOR, square)) &&
       (fileSquares & squaresBelow[COLOR][square] &
        position->piecesOfType[ROOK | OPPCOLOR]) != EMPTY_BITBOARD &&
       (companionFiles[square] &
        (position->piecesOfType[WHITE_PAWN] |
         position->piecesOfType[BLACK_PAWN])) == EMPTY_BITBOARD)
   {
      addMalus(base, V(0, 90));
   }
}

#ifdef PERSPECTIVE_WHITE
INLINE static void evaluateWhiteQueen(const Position * position,
                                      EvaluationBase * base,
                                      const Square square)
#else
INLINE static void evaluateBlackQueen(const Position * position,
                                      EvaluationBase * base,
                                      const Square square)
#endif
{
   const Bitboard moves = getMagicQueenMoves(square, position->allPieces);
   const Bitboard xrayPieces = position->piecesOfType[ROOK | COLOR] |
      position->piecesOfType[QUEEN | COLOR];
   const Bitboard xrayMoves =
      getMagicRookMoves(square, position->allPieces & ~xrayPieces);
   const int mobilityCount =
      getNumberOfSetSquares(moves & base->countedSquares[COLOR]);
   const Bitboard candidateTargets = base->unprotectedPieces[OPPCOLOR];

   base->queenAttackedSquares[COLOR] |= moves;
   base->queenSupportedSquares[COLOR] |= xrayMoves & ~moves;

   addBonus(base, QueenMobilityBonus[mobilityCount]);

   if (grantSeventhRankBonus(position, COLOR, OPPCOLOR, square))
   {
      addBonus(base, V(5, 25));

      if (colorRank(COLOR, position->king[OPPCOLOR]) == RANK_8)
      {
         if ((moves & position->piecesOfType[ROOK | COLOR] &
              squaresOfRank[rank(square)]) != EMPTY_BITBOARD)
         {
            addBonus(base, V(10, 15));
         }
      }
   }

   if (base->evaluateKingSafety[OPPCOLOR])
   {
      if ((moves & base->kingAttackSquares[OPPCOLOR]) != EMPTY_BITBOARD)
      {
         const Bitboard coronaAttacks =
            moves & getKingMoves(position->king[OPPCOLOR]);

         base->kingSquaresAttackCount[COLOR] +=
            getNumberOfSetSquares(coronaAttacks);
         base->attackInfo[COLOR] += V(QUEEN_BONUS_ATTACK, 1);
      }
   }

#ifdef PERSPECTIVE_WHITE
   addWhitePieceAttackBonus(position, base, moves, candidateTargets,
                            WHITE_QUEEN);
#else
   addBlackPieceAttackBonus(position, base, moves, candidateTargets,
                            BLACK_QUEEN);
#endif

   if (testSquare(base->pawnProtectedSquares[OPPCOLOR], square))
   {
      addMalus(base, piecePieceAttackBonus[PAWN | OPPCOLOR][QUEEN | COLOR]);
      base->hangingPieces[COLOR] |= minValue[square];
   }
}

#ifdef PERSPECTIVE_WHITE
INLINE static void evaluateWhitePasser(const Position * position,
                                       EvaluationBase * base,
                                       const Square square)
#else
INLINE static void evaluateBlackPasser(const Position * position,
                                       EvaluationBase * base,
                                       const Square square)
#endif
{
   const Piece currentPiece = position->piece[square];
   const Rank pawnRank = colorRank(COLOR, square);
   const int pawnDirection = (COLOR == WHITE ? 8 : -8);
   const Square stopSquare = (Square) (square + pawnDirection);
   const int numDefenders = position->numberOfPieces[OPPCOLOR] -
      position->numberOfPawns[OPPCOLOR];
   bool unStoppable = FALSE;
   const int rank = pawnRank - RANK_2;
   const int rankFactor = rank * (rank - 1);
   const int openingBonus = 20 * rankFactor;
   int endgameBonus = 10 + rank * rank * 10;
   int opValue, egValue;

   assert(pawnIsPassed(position, square, COLOR));

   if (numDefenders == 1)
   {
      const int egBonus = quad(0, 800, pawnRank);
      const int kingDistance = distance(square, position->king[COLOR]);
      const Square rectangleSquare =
         (COLOR == position->activeColor ?
          square : (Square) (square - pawnDirection));
      const bool kingInRectangle =
         testSquare(passedPawnRectangle[COLOR][rectangleSquare],
                    position->king[OPPCOLOR]);

      if ((kingInRectangle == FALSE &&
           (passedPawnCorridor[COLOR][square] &
            position->piecesOfColor[COLOR]) == EMPTY_BITBOARD))
      {
         addBonus(base, V(0, egBonus));
         unStoppable = TRUE;
      }
      else if (kingDistance == 1)
      {
         const File pawnFile = file(square);
         const File kingFile = file(position->king[COLOR]);
         const Square promotionSquare =
            (COLOR == WHITE ? getSquare(pawnFile, RANK_8) :
             getSquare(pawnFile, RANK_1));
         const bool clearPath = (bool)
            (kingFile != pawnFile ||
             (kingFile != FILE_A && kingFile != FILE_H));

         if (clearPath &&
             distance(promotionSquare, position->king[COLOR]) <= 1)
         {
            addBonus(base, V(0, egBonus));
            unStoppable = TRUE;
         }
      }

      if (unStoppable == FALSE &&
          base->hasPassersOrCandidates[OPPCOLOR] == FALSE &&
          passerWalks(position, square, COLOR))
      {
         addBonus(base, V(0, egBonus));
         unStoppable = TRUE;
      }
   }

   if (rankFactor > 0 && unStoppable == FALSE)
   {
      const Square attKing = position->king[COLOR];
      const Square defKing = position->king[OPPCOLOR];
      const int attackerDistance = distance(attKing, stopSquare);
      const int defenderDistance = distance(defKing, stopSquare);

      endgameBonus -= 4 * rankFactor * attackerDistance;
      endgameBonus += 6 * rankFactor * defenderDistance;

      if (position->piece[stopSquare] == NO_PIECE)
      {
         const Bitboard ownAttacks = base->attackedSquares[COLOR] |
            getKingMoves(attKing);
         const Bitboard oppAttacks = base->attackedSquares[OPPCOLOR] |
            getKingMoves(defKing);
         const Bitboard path = passedPawnCorridor[COLOR][square];
         const Bitboard ownBlockers = path & position->piecesOfColor[COLOR];
         const Bitboard oppBlockers =
            path & position->piecesOfColor[OPPCOLOR];
         Bitboard obstacles = path & oppAttacks;

         if (testSquare(base->attackedSquares[OPPCOLOR], square))
         {
            const Bitboard candidates =
               (position->piecesOfType[ROOK | OPPCOLOR] |
                position->piecesOfType[QUEEN | OPPCOLOR]) &
               passedPawnCorridor[OPPCOLOR][square] &
               getMagicRookMoves(square, position->allPieces);

            if (candidates != EMPTY_BITBOARD)
            {
               obstacles = path;
            }
         }

         obstacles |= oppBlockers;

         if (obstacles == EMPTY_BITBOARD)
         {
            const int bonus = (ownAttacks == path ? 18 : 16);

            endgameBonus += rankFactor * bonus;
         }
         else
         {
            int bonus =
               ((obstacles & ~ownAttacks) == EMPTY_BITBOARD ? 14 : 8);

            endgameBonus += rankFactor * bonus;
         }

         if (ownBlockers == EMPTY_BITBOARD)
         {
            endgameBonus += rankFactor;
         }
      }
   }

   if ((position->piecesOfType[currentPiece] & lateralSquares[square]) !=
       EMPTY_BITBOARD)
   {
      endgameBonus += 22 * rank;
   }
   else if (testSquare(base->pawnProtectedSquares[COLOR], square))
   {
      endgameBonus += 12 * rank;
   }

   if (file(square) == FILE_A || file(square) == FILE_H)
   {
      if (numberOfNonPawnPieces(position, OPPCOLOR) == 2 &&
          getPieceCount(position, (Piece) (KNIGHT | OPPCOLOR)) == 1)
      {
         endgameBonus += endgameBonus / 4;
      }
      else if (hasOrthoPieces(position, OPPCOLOR))
      {
         endgameBonus -= endgameBonus / 4;
      }
   }

   opValue = (openingBonus * PASSED_PAWN_WEIGHT_OP) / 256;
   egValue = (endgameBonus * PASSED_PAWN_WEIGHT_EG) / 256;

   addBonus(base, V(opValue, egValue));
}

#ifdef PERSPECTIVE_WHITE
INLINE static int getPawnSafetyMalusOfWhiteKingFile(const Position * position,
                                                    const int file,
                                                    const Square kingSquare,
                                                    const int fileType)
#else
INLINE static int getPawnSafetyMalusOfBlackKingFile(const Position * position,
                                                    const int file,
                                                    const Square kingSquare,
                                                    const int fileType)
#endif
{
   const int basicAttackWeight = 15;
   const Square baseSquare = getSquare(file, rank(kingSquare));
   const Bitboard pawnRealm = squaresOfRank[rank(kingSquare)] |
      squaresAbove[COLOR][kingSquare];
   const Bitboard fileRealm = squaresOfFile[file] & pawnRealm;
   const Bitboard diagRealm = pawnRealm &
      generalMoves[WHITE_BISHOP][baseSquare] &
      (file(kingSquare) <= FILE_D ?
       squaresRightOf[baseSquare] : squaresLeftOf[baseSquare]);
   const int oppKingFileDiff = abs(file - file(position->king[OPPCOLOR]));
   const int attackWeight = basicAttackWeight + oppKingFileDiff;
   Bitboard fileDefenders = fileRealm & position->piecesOfType[PAWN | COLOR];
   Bitboard diagDefenders = diagRealm & position->piecesOfType[PAWN | COLOR];
   Bitboard attackers = fileRealm & position->piecesOfType[PAWN | OPPCOLOR];
   int fdi = 0, fai = 0, fad = 1;
   int diaMalus = 0;

#ifdef PERSPECTIVE_WHITE
   const Square fileDefenderSquare = getFirstSquare(&fileDefenders);
   const Square fileAttackerSquare = getFirstSquare(&attackers);
#else
   const Square fileDefenderSquare = getLastSquare(&fileDefenders);
   const Square fileAttackerSquare = getLastSquare(&attackers);
#endif

   if (fileType == 1)
   {
#ifdef PERSPECTIVE_WHITE
      const Square diagDefenderSquare = getFirstSquare(&diagDefenders);
#else
      const Square diagDefenderSquare = getLastSquare(&diagDefenders);
#endif

      const File dfi = (File) (file <= FILE_D ? file : FILE_H - file);
      int ddi = 0;

      if (diagDefenderSquare != NO_SQUARE)
      {
         ddi = colorRank(COLOR, diagDefenderSquare);
      }

      assert(dfi >= FILE_A);
      assert(dfi <= FILE_D);
      assert(ddi >= RANK_1);
      assert(ddi <= RANK_8);

      diaMalus = KINGSAFETY_PAWN_BONUS_DEFENDER_DIAG[dfi][ddi];
   }

   if (fileDefenderSquare != NO_SQUARE)
   {
      fdi = colorRank(COLOR, fileDefenderSquare);
   }

   if (fileAttackerSquare != NO_SQUARE)
   {
      fai = colorRank(COLOR, fileAttackerSquare);

      if (fai == fdi + 1)
      {
         fad = 2;
      }
   }

   assert(fileType >= 0);
   assert(fileType <= 2);
   assert(fdi >= RANK_1);
   assert(fdi <= RANK_8);
   assert(fai >= RANK_1);
   assert(fai <= RANK_8);

   return KINGSAFETY_PAWN_MALUS_DEFENDER[fileType][fdi] +
      (KINGSAFETY_PAWN_BONUS_ATTACKER[fileType][fai] *
       attackWeight) / (basicAttackWeight * fad) + diaMalus;
}

#ifdef PERSPECTIVE_WHITE
INLINE static void evaluateWhitePawns(EvaluationBase * base)
#else
INLINE static void evaluateBlackPawns(EvaluationBase * base)
#endif
{
   const static INT32 chainBonusPerFile[8] = {
      V(4, 0), V(5, 0), V(5, 0), V(6, 0),
      V(6, 0), V(5, 0), V(5, 0), V(4, 0)
   };
   const static INT32 doubledMalusPerFile[8] = {
      V(5, 13), V(7, 15), V(8, 15), V(8, 15),
      V(8, 15), V(8, 15), V(7, 15), V(5, 13)
   };

   Bitboard chainPawns = base->chainPawns[COLOR];
   Bitboard doubledPawns = base->doubledPawns[COLOR];
   Square square;

   ITERATE_BITBOARD(&chainPawns, square)
   {
      addBonus(base, chainBonusPerFile[file(square)]);
   }

   ITERATE_BITBOARD(&doubledPawns, square)
   {
      addMalus(base, doubledMalusPerFile[file(square)]);
   }
}
