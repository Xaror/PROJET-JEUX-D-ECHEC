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

#include "xboard.h"
#include "coordination.h"
#include "tools.h"
#include "io.h"
#include "fen.h"
#include "pgn.h"
#ifdef INCLUDE_TABLEBASE_ACCESS
#include "tablebase.h"
#endif
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

static SearchTask task;
static Variation variation;
static PGNGame game;
static XboardStatus status;
bool resetGlobalHashtable = FALSE;
static char pv[256];
static int numUciMoves = 1000;
const int valueRangePct = 10;

const char *USN_NT = "Number Of Threads";
const char *USN_QO = "Value Queen Opening";
const char *USN_QE = "Value Queen Endgame";
const char *USN_RO = "Value Rook Opening";
const char *USN_RE = "Value Rook Endgame";
const char *USN_BO = "Value Bishop Opening";
const char *USN_BE = "Value Bishop Endgame";
const char *USN_NO = "Value Knight Opening";
const char *USN_NE = "Value Knight Endgame";
const char *USN_PO = "Value Pawn Opening";
const char *USN_PE = "Value Pawn Endgame";
const char *USN_BPO = "Value Bishop pair Opening";
const char *USN_BPE = "Value Bishop pair Endgame";
const char *USN_TL = "Threat Limit";

/* #define DEBUG_GUI_PROTOCOL */
/* #define DEBUG_GUI_CONVERSATION */

static Move readUciMove(const char *buffer)
{
   const Square from = getSquare(buffer[0] - 'a', buffer[1] - '1');
   const Square to = getSquare(buffer[2] - 'a', buffer[3] - '1');
   Piece newPiece = NO_PIECE;

   switch (buffer[4])
   {
   case 'q':
   case 'Q':
      newPiece = (Piece) (QUEEN);
      break;

   case 'r':
   case 'R':
      newPiece = (Piece) (ROOK);
      break;

   case 'b':
   case 'B':
      newPiece = (Piece) (BISHOP);
      break;

   case 'n':
   case 'N':
      newPiece = (Piece) (KNIGHT);
      break;

   default:
      newPiece = (Piece) (NO_PIECE);
   }

   return getPackedMove(from, to, newPiece);
}

static const char *getUciToken(const char *uciString, const char *tokenName)
{
   size_t tokenNameLength = strlen(tokenName);
   const char *tokenHit = strstr(uciString, tokenName);

#ifdef   DEBUG_GUI_PROTOCOL
   logDebug("find >%s< in >%s<\n", tokenName, uciString);
#endif

   if (tokenHit == 0)
   {
      return 0;
   }
   else
   {
      const char nextChar = *(tokenHit + tokenNameLength);

      if ((tokenHit == uciString || isspace((int) *(tokenHit - 1))) &&
          (nextChar == '\0' || isspace((int) nextChar)))
      {
         return tokenHit;
      }
      else
      {
         const char *nextPosstibleTokenOccurence =
            tokenHit + tokenNameLength + 1;

         if (nextPosstibleTokenOccurence + tokenNameLength <=
             uciString + strlen(uciString))
         {
            return getUciToken(nextPosstibleTokenOccurence, tokenName);
         }
         else
         {
            return 0;
         }
      }
   }
}

static void getNextUciToken(const char *uciString, char *buffer)
{
   const char *start = uciString, *end;
   unsigned int tokenLength;

   while (*start != '\0' && isspace(*start))
   {
      start++;
   }

   if (*start == '\0')
   {
      strcpy(buffer, "");

      return;
   }

   assert(*start != '\0' && isspace(*start) == FALSE);

   end = start + 1;

   while (*end != '\0' && isspace(*end) == FALSE)
   {
      end++;
   }

   assert(*end == '\0' || isspace(*end));

   tokenLength = (unsigned int) (end - start);

   strncpy(buffer, start, tokenLength);
   buffer[tokenLength] = '\0';
}

static long getLongUciValue(const char *uciString, const char *name,
                            int defaultValue)
{
   long value;
   char valueBuffer[256];
   const char *nameStart = getUciToken(uciString, name);

   if (nameStart == 0)
   {
      value = defaultValue;
   }
   else
   {
      getNextUciToken(nameStart + strlen(name), valueBuffer);
      value = atol(valueBuffer);
   }

#ifdef   DEBUG_GUI_PROTOCOL
   logDebug("get uci long value; %s = %ld\n", name, value);
#endif

   return value;
}

static void getUciNamedValue(const char *uciString, char *name, char *value)
{
   const char *nameTagStart = getUciToken(uciString, "name");
   const char *valueTagStart = getUciToken(uciString, "value");

   name[0] = value[0] = '\0';

   if (nameTagStart != 0 && valueTagStart != 0 &&
       nameTagStart < valueTagStart)
   {
      const int nameLength = (int) (valueTagStart - 1 - (nameTagStart + 5));

      strncpy(name, nameTagStart + 5, nameLength);
      name[nameLength] = '\0';
      getNextUciToken(valueTagStart + 6, value);

      /* logDebug("name=<%s> value=<%s>\n", name, value); */
   }
}

/******************************************************************************
 *
 * Get the specified move in xboard format.
 *
 ******************************************************************************/
static void getGuiMoveString(const Move move, char *buffer)
{
   char from[3], to[3];

   getSquareName(getFromSquare(move), from);
   getSquareName(getToSquare(move), to);

   if (getNewPiece(move) == NO_PIECE)
   {
      sprintf(buffer, "%s%s", from, to);
   }
   else
   {
      sprintf(buffer, "%s%s%c", from, to, pieceSymbol[getNewPiece(move)]);
   }
}

/******************************************************************************
 *
 * Get the specified param from the specified xboard command string.
 *
 ******************************************************************************/
static void getTokenByNumber(const char *command, int paramNumber,
                             char *buffer)
{
   int paramCount = 0;
   char currentChar;
   bool escapeMode = FALSE;
   char *pbuffer = buffer;

   while ((currentChar = *command++) != '\0' && paramCount <= paramNumber)
   {
      if (currentChar == '{')
      {
         escapeMode = TRUE;
      }
      else if (currentChar == '}')
      {
         escapeMode = FALSE;
      }

      if (isspace(currentChar) && escapeMode == FALSE)
      {
         paramCount++;

         while (isspace(*command))
         {
            command++;
         }
      }

      if (paramCount == paramNumber)
      {
         *pbuffer++ = currentChar;
      }
   }

   *pbuffer = '\0';
   trim(buffer);
}

/******************************************************************************
 *
 * Send the specified command via stdout to xboard.
 *
 ******************************************************************************/
static void sendToXboard(const char *fmt, ...)
{
   va_list args;
   char buffer[4096];

   va_start(args, fmt);
   vsprintf(buffer, fmt, args);
   va_end(args);

   fprintf(stdout, "%s\n", buffer);
   fflush(stdout);

#ifdef DEBUG_GUI_CONVERSATION
   logDebug("### sent to xboard: %s\n", buffer);
#endif
}

/******************************************************************************
 *
 * Send the specified command via stdout to xboard.
 *
 ******************************************************************************/
static void sendToXboardNonDebug(const char *fmt, ...)
{
   va_list args;
   char buffer[4096];

   va_start(args, fmt);
   vsprintf(buffer, fmt, args);
   va_end(args);

   fprintf(stdout, "%s\n", buffer);
   fflush(stdout);
}

/******************************************************************************
 *
 * Determine the calculation time for the next task in milliseconds.
 *
 ******************************************************************************/
static int getCalculationTime(TimecontrolData * data)
{
   if (data->restTime < 0)
   {
      return 2 * data->incrementTime;
   }

   if (data->movesToGo > 0)
   {
      return data->restTime / min(25, data->movesToGo) + data->incrementTime;
   }
   else
   {
      return (data->incrementTime > 0 ?
              data->incrementTime + data->restTime / 30 :
              data->restTime / 40);
   }
}

/******************************************************************************
 *
 * Determine the calculation time for the next task in milliseconds.
 *
 ******************************************************************************/
static int getMaximumCalculationTime(TimecontrolData * data)
{
   if (data->restTime < 0)
   {
      return data->incrementTime;
   }

   if (data->movesToGo > 0)
   {
      const int maximumTime = data->restTime -
         (4 + data->movesToGo) * TIME_CHECK_INTERVALL_IN_MS;
      const int limitTime =
         (data->restTime * data->movesToGo) / (2 * data->movesToGo - 1) +
         data->restTime / 10;

      return min(maximumTime, limitTime);
   }
   else
   {
      const int maxTime1 = max(data->restTime / 10,
                               data->restTime + data->incrementTime - 2250);
      const int maxTime2 = data->restTime / 4 + data->incrementTime;

      return min(maxTime1, maxTime2);
   }
}

/******************************************************************************
 *
 * Determine the calculation time for the next task in milliseconds.
 *
 ******************************************************************************/
static int determineCalculationTime(bool targetTime)
{
   if (status.operationMode == XBOARD_OPERATIONMODE_ANALYSIS)
   {
      return 0;
   }
   else
   {
      TimecontrolData *tcd = &status.timecontrolData[status.engineColor];

      initializeVariationFromGame(&variation, &game);
      tcd->numberOfMovesPlayed = variation.singlePosition.moveNumber - 1;

      if (targetTime)
      {
         return max(1, 95 * getCalculationTime(tcd) / 100);
      }
      else
      {
         return max(1, 95 * getMaximumCalculationTime(tcd) / 100);
      }
   }
}

/******************************************************************************
 *
 * Start the calculation of the current position.
 *
 ******************************************************************************/
static void startCalculation()
{
   variation.timeTarget = determineCalculationTime(TRUE);
   variation.timeLimit = determineCalculationTime(FALSE);
   setDrawScore(&variation, 0, status.engineColor);
   status.bestMoveWasSent = FALSE;

#ifdef DEBUG_GUI_CONVERSATION
   logDebug("Scheduling task. Timelimits: %d/%d\n", variation.timeTarget,
            variation.timeLimit);
#endif

   scheduleTask(&task);
}

/******************************************************************************
 *
 * Delete the current ponder result.
 *
 ******************************************************************************/
static void deletePonderResult()
{
   status.ponderResultMove = NO_MOVE;
}

/******************************************************************************
 *
 * Get a pv according to the uci protocol.
 *
 ******************************************************************************/
static char *getUciPv(const Variation * variation)
{
   char *buffer = malloc(256);
   int i;

   strcpy(buffer, "");

   for (i = 0; i < min(8, variation->pv.length); i++)
   {
      const Move move = (Move) variation->pv.move[i];

      if (move != NO_MOVE)
      {
         char moveBuffer[8];

         if (i > 0)
         {
            strcat(buffer, " ");
         }

         getGuiMoveString(move, moveBuffer);
         strcat(buffer, moveBuffer);
      }
      else
      {
         break;
      }
   }

   return buffer;
}

/******************************************************************************
 *
 * Post a principal variation line.
 *
 ******************************************************************************/
static void postPv(Variation * var, bool sendAnyway)
{
   double time = var->timestamp - var->startTime;
   char *pvmoves;
   char scoreBuffer[16];

   if (time >= 250 || sendAnyway)
   {
      UINT64 nodeCount = getNodeCount();

      pvmoves = getUciPv(var);
      formatUciValue(var->pv.score, scoreBuffer);

      sprintf(pv,
              "info depth %d seldepth %d time %.0f nodes %lld pv %s score %s tbhits %lu",
              var->nominalDepth, var->selDepth, time, nodeCount, pvmoves,
              scoreBuffer, var->tbHits);

      sendToXboardNonDebug("%s", pv);

      free(pvmoves);
      var->numPvUpdates++;
   }
}

/******************************************************************************
 *
 * Post a statistics information about the current search.
 *
 ******************************************************************************/
static void reportBaseMoveUpdate(const Variation * var)
{
   const double time = var->timestamp - var->startTime;
   char movetext[10];

   if (time >= 500)
   {
      getGuiMoveString(var->currentBaseMove, movetext);

      sendToXboardNonDebug
         ("info depth %d seldepth %d currmove %s currmovenumber %d",
          var->nominalDepth, var->selDepth, movetext,
          var->numberOfCurrentBaseMove);
   }
}

/******************************************************************************
 *
 * Post a statistics information about the current search.
 *
 ******************************************************************************/

static void reportStatisticsUpdate(Variation * var)
{
   UINT64 nodeCount = getNodeCount();
   const double time = var->timestamp - var->startTime;
   const double nps = (nodeCount / max((double) 0.001, (time / 1000.0)));
   const double hashUsage =
      ((double) var->hashtable->entriesUsed * 1000.0) /
      ((double) var->hashtable->tableSize);

   sendToXboardNonDebug
      ("info time %0.f nodes %lld nps %.0f hashfull %.0f tbhits %lu",
       time, nodeCount, nps, hashUsage, var->tbHits);
   reportBaseMoveUpdate(var);

   if (var->numPvUpdates == 0)
   {
      postPv(var, FALSE);
   }
}

/******************************************************************************
 *
 * Send a bestmove info to the gui.
 *
 ******************************************************************************/
static void sendBestmoveInfo(Variation * var)
{
   char moveBuffer[8];

   postPv(var, TRUE);

   if (moveIsLegal(&var->startPosition, var->bestBaseMove))
   {
      Variation tmp = *var;

      variation.expectedScore = getMoveValue(var->bestBaseMove);
      getGuiMoveString(var->bestBaseMove, moveBuffer);
      status.ponderingMove = (Move) var->pv.move[1];
      setBasePosition(&tmp, &var->startPosition);
      makeMove(&tmp, var->bestBaseMove);

      if (status.ponderingMove != NO_MOVE &&
          moveIsLegal(&tmp.singlePosition, status.ponderingMove))
      {
         char ponderMoveBuffer[8];

         getGuiMoveString(status.ponderingMove, ponderMoveBuffer);
         sendToXboard("bestmove %s ponder %s", moveBuffer, ponderMoveBuffer);
      }
      else
      {
         status.ponderingMove = NO_MOVE;
         sendToXboard("bestmove %s", moveBuffer);
      }

      unmakeLastMove(&tmp);
   }
   else
   {
      getGuiMoveString(var->bestBaseMove, moveBuffer);

#ifdef DEBUG_GUI_CONVERSATION
      logDebug("### Illegal best move %s ###\n", moveBuffer);
#endif
   }

   status.bestMoveWasSent = TRUE;
}

/******************************************************************************
 *
 * Stop the calculation of the current position.
 *
 ******************************************************************************/
static void stopCalculation()
{
   Variation tmp;

   prepareSearchAbort(&tmp);
   status.engineIsPondering = FALSE;
}

/******************************************************************************
 *
 * Handle events generated by the search engine.
 *
 ******************************************************************************/
static void handleSearchEvent(int eventId, void *var)
{
   Variation *variation = (Variation *) var;

   switch (eventId)
   {
   case SEARCHEVENT_SEARCH_FINISHED:

      getGuiSearchMutex();

      if (status.engineIsPondering == FALSE)
      {
         sendBestmoveInfo(var);
      }
      else
      {
#ifdef DEBUG_GUI_CONVERSATION
         logDebug("Search finished. Engine is pondering.\n");
         logDebug("No best move info sent.\n");
#endif
      }

      status.engineIsPondering = status.engineIsActive = FALSE;

      releaseGuiSearchMutex();

      break;

   case SEARCHEVENT_PLY_FINISHED:
      postPv(variation, FALSE);
      break;

   case SEARCHEVENT_NEW_BASEMOVE:
      reportBaseMoveUpdate(variation);
      break;

   case SEARCHEVENT_STATISTICS_UPDATE:
      reportStatisticsUpdate(variation);
      break;

   case SEARCHEVENT_NEW_PV:
      postPv(variation, TRUE);
      break;

   default:
      break;
   }

}

static int getIntValue(const char *value, int minValue, int defaultValue,
                       int maxValue)
{
   int parsedValue = atoi(value);

   if (parsedValue == 0 && parsedValue < minValue)
   {
      return defaultValue;
   }
   else
   {
      return min(max(parsedValue, minValue), maxValue);
   }
}

static int getValueLimit(int value, int diffPct)
{
   return (value * (100 + diffPct)) / 100;
}

static int getStdIntValue(const char *value, int defaultValue)
{
   const int minValue = getValueLimit(defaultValue, -valueRangePct);
   const int maxValue = getValueLimit(defaultValue, valueRangePct);

   return getIntValue(value, minValue, defaultValue, maxValue);
}

static void sendUciSpinOption(const char *name, const int defaultValue,
                              int minValue, int maxValue)
{
   sendToXboard("option name %s type spin default %d min %d max %d",
                name, defaultValue, minValue, maxValue);
}

/******************************************************************************
 *
 * Process the specified UCI command.
 *
 ******************************************************************************/
static int processUciCommand(const char *command)
{
   char buffer[8192];

   getTokenByNumber(command, 0, buffer);

   if (strcmp(buffer, "uci") == 0)
   {
      char nameString[256];

      strcpy(nameString, "id name Protector ");
      strcat(nameString, programVersionNumber);
      strcat(nameString, " ");
      strcat(nameString, systemVersion);

      sendToXboard(nameString);
      sendToXboard("id author Raimund Heid");
      sendToXboard("option name Hash type spin default 16 min 8 max 4096");
#ifdef INCLUDE_TABLEBASE_ACCESS
      sendToXboard("option name NalimovPath type string default <empty>");
      sendToXboard
         ("option name NalimovCache type spin default 4 min 1 max 64");
#endif
      sendToXboard("option name Ponder type check default true");
      sendUciSpinOption(USN_NT, 1, 1, MAX_THREADS);
      sendUciSpinOption(USN_PO, PAWN_DEFAULTVALUE_OPENING,
                        getValueLimit(PAWN_DEFAULTVALUE_OPENING,
                                      -valueRangePct),
                        getValueLimit(PAWN_DEFAULTVALUE_OPENING,
                                      valueRangePct));
      sendUciSpinOption(USN_PE, PAWN_DEFAULTVALUE_ENDGAME,
                        getValueLimit(PAWN_DEFAULTVALUE_ENDGAME,
                                      -valueRangePct),
                        getValueLimit(PAWN_DEFAULTVALUE_ENDGAME,
                                      valueRangePct));
      sendUciSpinOption(USN_NO, DEFAULTVALUE_KNIGHT_OPENING,
                        getValueLimit(DEFAULTVALUE_KNIGHT_OPENING,
                                      -valueRangePct),
                        getValueLimit(DEFAULTVALUE_KNIGHT_OPENING,
                                      valueRangePct));
      sendUciSpinOption(USN_NE, DEFAULTVALUE_KNIGHT_ENDGAME,
                        getValueLimit(DEFAULTVALUE_KNIGHT_ENDGAME,
                                      -valueRangePct),
                        getValueLimit(DEFAULTVALUE_KNIGHT_ENDGAME,
                                      valueRangePct));
      sendUciSpinOption(USN_BO, DEFAULTVALUE_BISHOP_OPENING,
                        getValueLimit(DEFAULTVALUE_BISHOP_OPENING,
                                      -valueRangePct),
                        getValueLimit(DEFAULTVALUE_BISHOP_OPENING,
                                      valueRangePct));
      sendUciSpinOption(USN_BE, DEFAULTVALUE_BISHOP_ENDGAME,
                        getValueLimit(DEFAULTVALUE_BISHOP_ENDGAME,
                                      -valueRangePct),
                        getValueLimit(DEFAULTVALUE_BISHOP_ENDGAME,
                                      valueRangePct));
      sendUciSpinOption(USN_RO, DEFAULTVALUE_ROOK_OPENING,
                        getValueLimit(DEFAULTVALUE_ROOK_OPENING,
                                      -valueRangePct),
                        getValueLimit(DEFAULTVALUE_ROOK_OPENING,
                                      valueRangePct));
      sendUciSpinOption(USN_RE, DEFAULTVALUE_ROOK_ENDGAME,
                        getValueLimit(DEFAULTVALUE_ROOK_ENDGAME,
                                      -valueRangePct),
                        getValueLimit(DEFAULTVALUE_ROOK_ENDGAME,
                                      valueRangePct));
      sendUciSpinOption(USN_QO, DEFAULTVALUE_QUEEN_OPENING,
                        getValueLimit(DEFAULTVALUE_QUEEN_OPENING,
                                      -valueRangePct),
                        getValueLimit(DEFAULTVALUE_QUEEN_OPENING,
                                      valueRangePct));
      sendUciSpinOption(USN_QE, DEFAULTVALUE_QUEEN_ENDGAME,
                        getValueLimit(DEFAULTVALUE_QUEEN_ENDGAME,
                                      -valueRangePct),
                        getValueLimit(DEFAULTVALUE_QUEEN_ENDGAME,
                                      valueRangePct));
      sendUciSpinOption(USN_BPO, DEFAULTVALUE_BISHOP_PAIR_OPENING,
                        getValueLimit(DEFAULTVALUE_BISHOP_PAIR_OPENING,
                                      -valueRangePct),
                        getValueLimit(DEFAULTVALUE_BISHOP_PAIR_OPENING,
                                      valueRangePct));
      sendUciSpinOption(USN_BPE, DEFAULTVALUE_BISHOP_PAIR_ENDGAME,
                        getValueLimit(DEFAULTVALUE_BISHOP_PAIR_ENDGAME,
                                      -valueRangePct),
                        getValueLimit(DEFAULTVALUE_BISHOP_PAIR_ENDGAME,
                                      valueRangePct));

      sendToXboard("uciok");

      return TRUE;
   }

   if (strcmp(buffer, "isready") == 0)
   {
      sendToXboard("readyok");

      return TRUE;
   }

   if (strcmp(buffer, "ucinewgame") == 0)
   {
      resetGlobalHashtable = TRUE;
      variation.expectedScore = 0;

      return TRUE;
   }

   if (strcmp(buffer, "setoption") == 0)
   {
      char name[256], value[256];

      getUciNamedValue(command, name, value);

#ifdef INCLUDE_TABLEBASE_ACCESS
      if (strcmp(name, "NalimovPath") == 0)
      {
         initializeTablebase(value);

         return TRUE;
      }

      if (strcmp(name, "NalimovCache") == 0)
      {
         const int cacheSize = atoi(value);

         setTablebaseCacheSize(cacheSize);

         return TRUE;
      }
#endif

      if (strcmp(name, "Hash") == 0)
      {
         const unsigned int hashsize = (unsigned int) max(8, atoi(value));

         setHashtableSize(hashsize);

         return TRUE;
      }

      if (strcmp(name, USN_NT) == 0)
      {
         const unsigned int numThreads =
            (unsigned int) getIntValue(value, 1, 1, MAX_THREADS);

         setNumberOfThreads(numThreads);

         return TRUE;
      }

      if (strcmp(name, USN_PO) == 0)
      {
         PAWN_VALUE_OPENING = (unsigned int)
            getStdIntValue(value, PAWN_DEFAULTVALUE_OPENING);

         return TRUE;
      }

      if (strcmp(name, USN_PE) == 0)
      {
         PAWN_VALUE_ENDGAME = (unsigned int)
            getStdIntValue(value, PAWN_DEFAULTVALUE_ENDGAME);

         return TRUE;
      }

      if (strcmp(name, USN_NO) == 0)
      {
         VALUE_KNIGHT_OPENING = (unsigned int)
            getStdIntValue(value, DEFAULTVALUE_KNIGHT_OPENING);

         return TRUE;
      }

      if (strcmp(name, USN_NE) == 0)
      {
         VALUE_KNIGHT_ENDGAME = (unsigned int)
            getStdIntValue(value, DEFAULTVALUE_KNIGHT_ENDGAME);

         return TRUE;
      }

      if (strcmp(name, USN_BO) == 0)
      {
         VALUE_BISHOP_OPENING = (unsigned int)
            getStdIntValue(value, DEFAULTVALUE_BISHOP_OPENING);

         return TRUE;
      }

      if (strcmp(name, USN_BE) == 0)
      {
         VALUE_BISHOP_ENDGAME = (unsigned int)
            getStdIntValue(value, DEFAULTVALUE_BISHOP_ENDGAME);

         return TRUE;
      }

      if (strcmp(name, USN_RO) == 0)
      {
         VALUE_ROOK_OPENING = (unsigned int)
            getStdIntValue(value, DEFAULTVALUE_ROOK_OPENING);

         return TRUE;
      }

      if (strcmp(name, USN_RE) == 0)
      {
         VALUE_ROOK_ENDGAME = (unsigned int)
            getStdIntValue(value, DEFAULTVALUE_ROOK_ENDGAME);

         return TRUE;
      }

      if (strcmp(name, USN_QO) == 0)
      {
         VALUE_QUEEN_OPENING = (unsigned int)
            getStdIntValue(value, DEFAULTVALUE_QUEEN_OPENING);

         return TRUE;
      }

      if (strcmp(name, USN_QE) == 0)
      {
         VALUE_QUEEN_ENDGAME = (unsigned int)
            getStdIntValue(value, DEFAULTVALUE_QUEEN_ENDGAME);

         return TRUE;
      }

      if (strcmp(name, USN_BPO) == 0)
      {
         VALUE_BISHOP_PAIR_OPENING = (unsigned int)
            getStdIntValue(value, DEFAULTVALUE_BISHOP_PAIR_OPENING);

         return TRUE;
      }

      if (strcmp(name, USN_BPE) == 0)
      {
         VALUE_BISHOP_PAIR_ENDGAME = (unsigned int)
            getStdIntValue(value, DEFAULTVALUE_BISHOP_PAIR_ENDGAME);

         return TRUE;
      }
   }

   if (strcmp(buffer, "position") == 0)
   {
      resetPGNGame(&game);

      if (getUciToken(command, "fen") != 0)
      {
         const char *fenStart = getUciToken(command, "fen") + 3;
         const char *fenEnd = getUciToken(command, "moves");

         if (fenEnd == 0)
         {
            strcpy(game.fen, fenStart);
         }
         else
         {
            const int length = (int) (fenEnd - fenStart - 1);

            strncpy(game.fen, fenStart, length);
            game.fen[length] = '\0';
         }

         trim(game.fen);
         strcpy(game.setup, "1");

#ifdef   DEBUG_GUI_PROTOCOL
         logDebug("fen set: >%s<\n", game.fen);
#endif
      }

      if (getUciToken(command, "moves") != 0)
      {
         char moveBuffer[8];
         const char *currentMove = getUciToken(command, "moves") + 5;
         bool finished = FALSE;
         int moveCount = 0;

         do
         {
            getNextUciToken(currentMove, moveBuffer);

            if (strlen(moveBuffer) > 0)
            {
               Move move = readUciMove(moveBuffer);

#ifdef   DEBUG_GUI_PROTOCOL
               logDebug("move found: >%s<\n", moveBuffer);
#endif

               if (appendMove(&game, move) == 0)
               {
                  currentMove += strlen(moveBuffer) + 1;
                  moveCount++;
               }
               else
               {
                  finished = TRUE;
               }
            }
            else
            {
               finished = TRUE;
            }
         }
         while (finished == FALSE);

         if (moveCount < numUciMoves - 1)
         {
            resetGlobalHashtable = TRUE;
            variation.expectedScore = 0;
         }

         numUciMoves = moveCount;
      }

      return TRUE;
   }

   if (strcmp(buffer, "stop") == 0)
   {
      getGuiSearchMutex();

      if (status.engineIsActive)
      {
#ifdef DEBUG_GUI_CONVERSATION
         logDebug("stopping search...\n");
#endif
         status.engineIsPondering = FALSE;
         stopCalculation();
         releaseGuiSearchMutex();
         waitForSearchTermination();
         getGuiSearchMutex();
      }
      else
      {
         if (status.bestMoveWasSent == FALSE)
         {
#ifdef DEBUG_GUI_CONVERSATION
            logDebug("sending best move info...\n");
#endif
            sendBestmoveInfo(getCurrentVariation());
         }
         else
         {
#ifdef DEBUG_GUI_CONVERSATION
            logDebug("sending 'readyok'...\n");
#endif
            sendToXboard("readyok");
         }
      }

      releaseGuiSearchMutex();

      return TRUE;
   }

   if (strcmp(buffer, "ponderhit") == 0)
   {
      getGuiSearchMutex();

      if (status.engineIsPondering)
      {
         status.engineIsPondering = FALSE;

         if (getCurrentVariation()->terminatePondering &&
             getCurrentVariation()->failingHighOrLow == FALSE)
         {
            Variation tmp;

#ifdef DEBUG_GUI_CONVERSATION
            logDebug("immediate termination of pondering.\n");
#endif

            prepareSearchAbort(&tmp);
         }
         else
         {
#ifdef DEBUG_GUI_CONVERSATION
            logDebug("unsetting pondering mode.\n");
#endif

            unsetPonderMode();
         }
      }
      else
      {
#ifdef DEBUG_GUI_CONVERSATION
         logDebug("Pondering finished prematurely. Sending best move info\n");
#endif

         sendBestmoveInfo(getCurrentVariation());
      }

      releaseGuiSearchMutex();

      return TRUE;
   }

   if (strcmp(buffer, "go") == 0)
   {
      getGuiSearchMutex();
      status.engineIsActive = TRUE;
      task.type = TASKTYPE_BEST_MOVE;

      initializeVariationFromGame(&variation, &game);
      status.engineColor = variation.singlePosition.activeColor;

      if (getUciToken(command, "depth") != 0)
      {
         status.operationMode = XBOARD_OPERATIONMODE_ANALYSIS;
      }
      else if (getUciToken(command, "nodes") != 0)
      {
         status.operationMode = XBOARD_OPERATIONMODE_ANALYSIS;
      }
      else if (getUciToken(command, "mate") != 0)
      {
         task.type = TASKTYPE_MATE_IN_N;
         task.numberOfMoves = getLongUciValue(command, "mate", 1);
         status.operationMode = XBOARD_OPERATIONMODE_ANALYSIS;

#ifdef   DEBUG_GUI_PROTOCOL
         logDebug("Searching mate in %d", task.numberOfMoves);
#endif
      }
      else if (getUciToken(command, "movetime") != 0)
      {
         status.operationMode = XBOARD_OPERATIONMODE_USERGAME;

         status.timecontrolData[WHITE].restTime =
            status.timecontrolData[BLACK].restTime = -1;
         status.timecontrolData[WHITE].incrementTime =
            status.timecontrolData[BLACK].incrementTime =
            getLongUciValue(command, "movetime", 5000);
      }
      else if (getUciToken(command, "infinite") != 0)
      {
         status.operationMode = XBOARD_OPERATIONMODE_ANALYSIS;
      }
      else
      {
         const int numMovesPlayed = variation.singlePosition.moveNumber - 1;
         const int movesToGo = (int) getLongUciValue(command, "movestogo", 0);

         status.operationMode = XBOARD_OPERATIONMODE_USERGAME;

         status.timecontrolData[WHITE].restTime =
            getLongUciValue(command, "wtime", 1000);
         status.timecontrolData[WHITE].incrementTime =
            getLongUciValue(command, "winc", 0);
         status.timecontrolData[BLACK].restTime =
            getLongUciValue(command, "btime", 1000);
         status.timecontrolData[BLACK].incrementTime =
            getLongUciValue(command, "binc", 0);
         status.timecontrolData[WHITE].numberOfMovesPlayed =
            status.timecontrolData[BLACK].numberOfMovesPlayed =
            numMovesPlayed;

         if (movesToGo > 0)
         {
            status.timecontrolData[WHITE].movesToGo =
               status.timecontrolData[BLACK].movesToGo = movesToGo;
         }
         else
         {
            status.timecontrolData[WHITE].movesToGo =
               status.timecontrolData[BLACK].movesToGo = 0;
         }
      }

      if (getUciToken(command, "ponder") == 0)
      {
         status.engineIsPondering = variation.ponderMode = FALSE;
      }
      else
      {
         status.engineIsPondering = variation.ponderMode = TRUE;
         variation.terminatePondering = FALSE;  /* avoid premature search aborts */
      }

      startCalculation();
      releaseGuiSearchMutex();

      return TRUE;
   }

   if (strcmp(buffer, "quit") == 0)
   {
      stopCalculation();

      return FALSE;
   }

   return TRUE;
}

/******************************************************************************
 *
 * Read xboard's commands from stdin and handle them.
 *
 ******************************************************************************/
void acceptGuiCommands()
{
   bool finished = FALSE;
   char command[8192];

   /* signal(SIGINT, SIG_IGN); */

   while (finished == FALSE)
   {
      if (fgets(command, sizeof(command), stdin) == NULL)
      {
         finished = TRUE;
      }
      else
      {
         trim(command);

#ifdef DEBUG_GUI_CONVERSATION
         logDebug("\n### gui command: >%s<\n", command);
#endif

         finished = (bool) (processUciCommand(command) == FALSE);

#ifdef DEBUG_GUI_CONVERSATION
         logDebug(">%s< processed.\n\n", command);
#endif
      }
   }

#ifdef   DEBUG_GUI_PROTOCOL
   logDebug("GUI command processing terminated.\n");
#endif
}

/******************************************************************************
 *
 * (See the header file comment for this function.)
 *
 ******************************************************************************/
int initializeModuleXboard()
{
   status.operationMode = XBOARD_OPERATIONMODE_USERGAME;
   status.engineColor = WHITE;
   status.pondering = TRUE;
   status.ponderingMove = NO_MOVE;
   status.engineIsPondering = FALSE;
   status.engineIsActive = FALSE;
   status.bestMoveWasSent = TRUE;
   status.maxPlies = 0;
   status.timecontrolData[WHITE].movesToGo = 0;
   status.timecontrolData[WHITE].incrementTime = 0;
   status.timecontrolData[WHITE].numberOfMovesPlayed = 0;
   status.timecontrolData[WHITE].restTime = 300 * 1000;
   status.timecontrolData[BLACK].movesToGo = 0;
   status.timecontrolData[BLACK].incrementTime =
      status.timecontrolData[WHITE].incrementTime;
   status.timecontrolData[BLACK].numberOfMovesPlayed = 0;
   status.timecontrolData[BLACK].restTime =
      status.timecontrolData[WHITE].restTime;
   deletePonderResult();

   initializePGNGame(&game);
   variation.timeLimit = 5000;
   variation.ponderMode = FALSE;
   variation.handleSearchEvent = &handleSearchEvent;
   variation.expectedScore = 0;
   task.variation = &variation;
   task.type = TASKTYPE_BEST_MOVE;

   return 0;
}

#ifndef NDEBUG

/******************************************************************************
 *
 * Test parameter parsing.
 *
 ******************************************************************************/
static int testParameterParsing()
{
   char buffer[1024];

   getTokenByNumber("protover 2", 1, buffer);
   assert(strcmp("2", buffer) == 0);

   getTokenByNumber("result 1-0 {White mates}", 2, buffer);
   assert(strcmp("{White mates}", buffer) == 0);

   return 0;
}

/******************************************************************************
 *
 * Test time calculation.
 *
 ******************************************************************************/
static int testTimeCalculation()
{
   return 0;
}

/******************************************************************************
 *
 * Test the uci tokenizer.
 *
 ******************************************************************************/
static int testUciTokenizer()
{
   char buffer[64], name[64], value[64];
   const char *uciString =
      "setoption name\tNalimovPath    value  \t  C:\\chess\\tablebases   time  641273423";
   const char *trickyUciString =
      "setoption name\tNalimovPathvalue    value  \t  C:\\chess\\tablebases   time  641273423 tablebases";
   const char *token1 = getUciToken(uciString, "NalimovPath");
   const char *token2 = getUciToken(uciString, "tablebases");
   const char *token3 = getUciToken(uciString, "name");
   const char *token4 = getUciToken(uciString, "value");
   const char *token5 = getUciToken(trickyUciString, "tablebases");

   assert(strstr(token1, "NalimovPath") == token1);
   assert(token2 == 0);
   assert(strstr(token3, "name") == token3);

   getNextUciToken(token3 + 4, buffer);
   assert(strcmp(buffer, "NalimovPath") == 0);

   getNextUciToken(token4 + 5, buffer);
   assert(strcmp(buffer, "C:\\chess\\tablebases") == 0);

   assert(getLongUciValue(uciString, "time", 0) == 641273423);

   assert(strstr(trickyUciString, "423 tablebases") == token5 - 4);

   getUciNamedValue(uciString, name, value);
   assert(strcmp(name, "NalimovPath") == 0);
   assert(strcmp(value, "C:\\chess\\tablebases") == 0);

   getUciNamedValue(trickyUciString, name, value);
   assert(strcmp(name, "NalimovPathvalue") == 0);
   assert(strcmp(value, "C:\\chess\\tablebases") == 0);

   return 0;
}

#endif

/******************************************************************************
 *
 * (See the header file comment for this function.)
 *
 ******************************************************************************/
int testModuleXboard()
{
#ifndef NDEBUG
   int result;

   if ((result = testParameterParsing()) != 0)
   {
      return result;
   }

   if ((result = testTimeCalculation()) != 0)
   {
      return result;
   }

   if ((result = testUciTokenizer()) != 0)
   {
      return result;
   }
#endif

   return 0;
}
