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

#include "coordination.h"
#include "protector.h"
#include "search.h"
#include "matesearch.h"
#include "io.h"
#include "hash.h"
#include <stdio.h>
#include <assert.h>

#ifdef TARGET_WINDOWS
#include <windows.h>
#endif
#ifdef TARGET_LINUX
#include <pthread.h>
#endif

/* #define DEBUG_COORDINATION */

#ifdef TARGET_WINDOWS
HANDLE searchThread[MAX_THREADS];
DWORD searchThreadId[MAX_THREADS];
LPCTSTR commonMutexObject = (LPCTSTR) ("Mo");
HANDLE guiSearchMutex;
#endif
#ifdef TARGET_LINUX
pthread_t searchThread[MAX_THREADS];
static pthread_mutex_t guiSearchMutex = PTHREAD_MUTEX_INITIALIZER;
long searchThreadId[MAX_THREADS];
#endif

static int numThreads = 1;
static SearchTask dummyTask;
static SearchTask *currentTask = &dummyTask;
static Variation variations[MAX_THREADS];
static Hashtable localHashtable;
static PawnHashInfo pawnHashtable[MAX_THREADS][PAWN_HASHTABLE_SIZE];
static UINT16 _historyValue[MAX_THREADS][HISTORY_SIZE];
static INT16 _positionalGain[MAX_THREADS][HISTORY_SIZE];

int setNumberOfThreads(int _numThreads)
{
   numThreads = max(1, min(MAX_THREADS, _numThreads));

   return numThreads;
}

UINT64 getNodeCount(void)
{
   int threadCount;
   UINT64 sum = 0;

   for (threadCount = 0; threadCount < numThreads; threadCount++)
   {
      sum += variations[threadCount].nodes;
   }

   return sum;
}

Variation *getCurrentVariation()
{
   return &variations[0];
}

void getGuiSearchMutex(void)
{
#ifdef DEBUG_COORDINATION
   logDebug("aquiring search lock...\n");
#endif

#ifdef MSFT_CC
   WaitForSingleObject(guiSearchMutex, INFINITE);
#else
   pthread_mutex_lock(&guiSearchMutex);
#endif

#ifdef DEBUG_COORDINATION
   logDebug("search lock aquired...\n");
#endif
}

void releaseGuiSearchMutex(void)
{
#ifdef MSFT_CC
   ReleaseMutex(guiSearchMutex);
#else
   pthread_mutex_unlock(&guiSearchMutex);
#endif

#ifdef DEBUG_COORDINATION
   logDebug("search lock released...\n");
#endif
}

static int startSearch(Variation * currentVariation)
{
   currentVariation->searchStatus = SEARCH_STATUS_RUNNING;

   switch (currentTask->type)
   {
   case TASKTYPE_BEST_MOVE:
      currentTask->bestMove = search(currentVariation, NULL);
      break;

   case TASKTYPE_TEST_BEST_MOVE:
      currentTask->bestMove =
         search(currentVariation, &currentTask->solutions);
      break;

   case TASKTYPE_MATE_IN_N:
      searchForMate(currentVariation,
                    &currentTask->calculatedSolutions,
                    currentTask->numberOfMoves);
      break;

   case TASKTYPE_TEST_MATE_IN_N:
      searchForMate(currentVariation,
                    &currentTask->calculatedSolutions,
                    currentTask->numberOfMoves);
      break;

   default:
      break;
   }

   currentTask->nodes = getNodeCount();
   currentVariation->searchStatus = SEARCH_STATUS_FINISHED;

   if (currentVariation->threadNumber == 0)
   {
      int threadCount;

      for (threadCount = 1; threadCount < numThreads; threadCount++)
      {
         variations[threadCount].terminate = TRUE;
      }
   }

#ifdef DEBUG_COORDINATION
   logDebug("Search thread terminated.\n");
#endif

   return 0;
}

#ifdef TARGET_WINDOWS
DWORD WINAPI executeSearch(PVOID arg)
{
   Variation *currentVariation = arg;

   startSearch(currentVariation);

   return 0;
}
#endif
#ifdef TARGET_LINUX
static void *executeSearch(void *arg)
{
   Variation *currentVariation = arg;

   startSearch(currentVariation);

   return 0;
}
#endif

void scheduleTask(SearchTask * task)
{
   int threadCount;

   localHashtable.entriesUsed = 0;

   for (threadCount = 0; threadCount < numThreads; threadCount++)
   {
      Variation *currentVariation = &variations[threadCount];

      currentVariation->terminate = TRUE;

#ifdef DEBUG_COORDINATION
      logDebug("Schedule task: Search abort signaled.\n");
#endif

      currentTask = task;
      *currentVariation = *(currentTask->variation);
      currentVariation->searchStatus = SEARCH_STATUS_TERMINATE;
      currentVariation->terminate = FALSE;
      currentVariation->hashtable = &localHashtable;
      currentVariation->pawnHashtable = &(pawnHashtable[threadCount][0]);
      currentVariation->kingsafetyHashtable =
         &(kingSafetyHashtable[threadCount][0]);
      currentVariation->historyValue = &(_historyValue[threadCount][0]);
      currentVariation->positionalGain = &(_positionalGain[threadCount][0]);
      currentVariation->threadNumber = threadCount;

      if (threadCount != 0)
      {
         currentVariation->handleSearchEvent = 0;
         currentVariation->timeLimit = 0;
      }

#ifdef TARGET_WINDOWS
      searchThread[threadCount] =
         CreateThread(NULL, 0, executeSearch, currentVariation,
                      0, &searchThreadId[threadCount]);

      if (searchThread[threadCount] != NULL)
      {
         if (threadCount > 0)
         {
            SetThreadPriority(searchThread[threadCount],
                              THREAD_PRIORITY_BELOW_NORMAL);
         }

#ifdef DEBUG_COORDINATION
         logDebug("Search thread %d started.\n", threadCount);
#endif
      }
      else
      {
         logDebug("### Search thread could not be started. ###\n");

         exit(EXIT_FAILURE);
      }
#endif
#ifdef TARGET_LINUX
      if (pthread_create(&searchThread[threadCount], NULL,
                         &executeSearch, currentVariation) == 0)
      {
#ifdef DEBUG_COORDINATION
         logDebug("Search thread %d started.\n", threadCount);
#endif
      }
      else
      {
         logDebug("### Search thread could not be started. ###\n");

         exit(EXIT_FAILURE);
      }
#endif
   }
}

void waitForSearchTermination(void)
{
   int threadCount;
   bool finished;
   int count = 0;

   do
   {
      finished = TRUE;

      if (count > 1000)
      {
         logDebug("waiting for search termination.\n");
         count = 0;
      }

      for (threadCount = 0; threadCount < numThreads; threadCount++)
      {
         Variation *currentVariation = &variations[threadCount];

         if (currentVariation->searchStatus != SEARCH_STATUS_FINISHED)
         {
            finished = FALSE;

#ifdef TARGET_WINDOWS
            WaitForSingleObject(searchThread[threadCount], 100);
#endif
#ifdef TARGET_LINUX
            pthread_join(searchThread[threadCount], 0);
#endif
            break;
         }

#ifdef DEBUG_COORDINATION
         logDebug("Task %d finished.\n", threadCount);
#endif
      }

      count++;
   }
   while (finished == FALSE);
}

void completeTask(SearchTask * task)
{
   scheduleTask(task);

#ifdef DEBUG_COORDINATION
   logDebug("Task scheduled. Waiting for completion.\n");
#endif

   waitForSearchTermination();
}

void prepareSearchAbort(Variation * variation)
{
   int threadCount;

   for (threadCount = 0; threadCount < numThreads; threadCount++)
   {
      variations[threadCount].terminate = TRUE;
   }

   *variation = variations[0];
}

void unsetPonderMode(void)
{
   int threadCount;

   for (threadCount = 0; threadCount < numThreads; threadCount++)
   {
      Variation *currentVariation = &variations[threadCount];

      currentVariation->ponderMode = FALSE;
   }
}

void setTimeLimit(unsigned long timeTarget, unsigned long timeLimit)
{
   int threadCount;

   for (threadCount = 0; threadCount < numThreads; threadCount++)
   {
      Variation *currentVariation = &variations[threadCount];

      currentVariation->timeTarget = timeTarget;
      currentVariation->timeLimit = timeLimit;
   }
}

void setHashtableSize(unsigned int size)
{
   unsigned long tablesize = sizeof(Hashentry);
   int exponent = 0;

   while (tablesize <= (unsigned long) size * 512 * 1024)
   {
      exponent++;
      tablesize *= 2;
   }

   setHashtableExponent(&localHashtable, exponent);
   resetHashtable(&localHashtable);
}

int initializeModuleCoordination()
{
   int threadCount;

#ifdef TARGET_WINDOWS
   guiSearchMutex = CreateMutex(NULL, FALSE, commonMutexObject);
#endif

   initializeHashtable(&localHashtable);
   setHashtableExponent(&localHashtable, 18);
   resetHashtable(&localHashtable);

   for (threadCount = 0; threadCount < MAX_THREADS; threadCount++)
   {
      Variation *currentVariation = &variations[threadCount];

      currentVariation->searchStatus = SEARCH_STATUS_FINISHED;
   }

   return 0;
}

int testModuleCoordination()
{
   return 0;
}
