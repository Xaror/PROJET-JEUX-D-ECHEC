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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "hash.h"
#include "protector.h"
#include "io.h"
#include "keytable.h"

Hashtable globalHashtable;
const int NUM_DATES = 16;
const int CLUSTER_SIZE = 4;
const UINT8 DEPTH_NONE = 0;

#ifndef INLINE_IN_HEADERS
#include "hashInline.h"
#endif

static INLINE int getAge(const Hashtable * hashtable, const UINT8 date)
{
   assert(date < NUM_DATES);
   assert(hashtable->date < NUM_DATES);

   return (hashtable->date + NUM_DATES - date) & (NUM_DATES - 1);
}

void incrementDate(Hashtable * hashtable)
{
   assert(hashtable->date < NUM_DATES);

   hashtable->date = (UINT8) ((hashtable->date + 1) % NUM_DATES);

   assert(hashtable->date < NUM_DATES);
}

static void deleteTables(Hashtable * hashtable)
{
   if (hashtable->table != 0)
   {
      free(hashtable->table);
      hashtable->table = 0;
   }
}

static INLINE UINT64 _getHashData(INT16 value, UINT8 importance,
                                  UINT16 bestMove, UINT8 date, UINT8 flag)
{
   return (UINT64) (value & 0xFFFF) | ((UINT64) importance) << 16 |
      ((UINT64) bestMove) << 32 | ((UINT64) date) << 48 |
      ((UINT64) flag) << 56;
}

static INLINE UINT32 _getStaticValueData(INT16 staticValue,
                                         INT16 futilityMargin)
{
   return ((UINT32) (staticValue & 0xFFFF)) |
      (((UINT32) (futilityMargin & 0xFFFF)) << 16);
}

void resetHashtable(Hashtable * hashtable)
{
   unsigned long l;
   Hashentry emptyEntry;

   emptyEntry.key = ULONG_ZERO;
   emptyEntry.data = _getHashData(-VALUE_MATED, DEPTH_NONE, (UINT16) NO_MOVE,
                                  0, HASHVALUE_UPPER_LIMIT);

   for (l = 0; l < hashtable->tableSize; l++)
   {
      hashtable->table[l] = emptyEntry;
   }

   hashtable->date = 0;
   hashtable->entriesUsed = 0;

   /* logDebug("hashtable reset done.\n"); */
}

void initializeHashtable(Hashtable * hashtable)
{
   hashtable->table = 0;
   hashtable->tableSize = 0;
   hashtable->entriesUsed = 0;
}

void setHashtableExponent(Hashtable * hashtable, int exponent)
{
   deleteTables(hashtable);

   hashtable->exponent = max(10, exponent);
   hashtable->exponent = min(56, exponent);

   hashtable->tableSize = (1L << hashtable->exponent) + CLUSTER_SIZE - 1;
   hashtable->table = malloc((hashtable->tableSize) * sizeof(Hashentry));
   hashtable->hashMask = (1L << hashtable->exponent) - 1;

   /* logDebug("Hashtable size: %ld bytes\n",
      (hashtable->tableSize) * sizeof(Hashentry)); */
}

#define getHashIndex(hashtable, key) ((key) & (hashtable)->hashMask)

void setDatedEntry(Hashtable * hashtable, UINT64 key, INT16 value,
                   UINT8 importance, UINT16 bestMove, UINT8 flag,
                   INT16 staticValue, INT16 futilityMargin)
{
   const int index = (int) getHashIndex(hashtable, key);
   int i, bestEntry = 0, bestEntryScore = -1024;
   UINT64 data;
   UINT32 staticValueData;
   Hashentry *entryToBeReplaced;
   const int valueFlag = flag & 0x03;

   for (i = 0; i < CLUSTER_SIZE; i++)
   {
      Hashentry copy = hashtable->table[index + i];
      const UINT8 copyDate = getHashentryDate(&copy);
      const UINT8 copyImportance = getHashentryImportance(&copy);
      const int copyFlag = getHashentryFlag(&copy) & 0x03;

      if (getHashentryKey(&copy) == key &&
          (importance >= copyImportance ||
           valueFlag == HASHVALUE_EXACT ||
           (importance >= copyImportance - 2 &&
            !movesAreEqual(bestMove, getHashentryMove(&copy)))))
      {
         if (copyDate != hashtable->date)
         {
            hashtable->entriesUsed++;
         }

         if (bestMove == (UINT16) NO_MOVE && copyImportance >= importance)
         {
            bestMove = getHashentryMove(&copy);
         }

         data = _getHashData(value, importance, bestMove,
                             hashtable->date, flag);
         staticValueData = _getStaticValueData(staticValue, futilityMargin);

         hashtable->table[index + i].key = key ^ data ^ staticValueData;
         hashtable->table[index + i].data = data;
         hashtable->table[index + i].staticValueData = staticValueData;

         return;
      }
      else
      {
         const int bonus = (copyFlag == HASHVALUE_EXACT ? 4 : 0);
         const int malus = (copy.key == ULONG_ZERO ? 8192 : 0);
         const int malus2 =
            (getHashentryMove(&copy) == (UINT16) NO_MOVE ? 2 : 0);
         const int score = getAge(hashtable, copyDate) * 256 +
            malus + malus2 - copyImportance - bonus;

         if (score > bestEntryScore)
         {
            bestEntry = i;
            bestEntryScore = score;
         }
      }
   }

   if (hashtable->table[index + bestEntry].key == ULONG_ZERO ||
       getHashentryDate(&hashtable->table[index + bestEntry]) !=
       hashtable->date)
   {
      hashtable->entriesUsed++;
   }

   entryToBeReplaced = &hashtable->table[index + bestEntry];

   if (bestMove == NO_MOVE &&
       getHashentryKey(entryToBeReplaced) == key &&
       getHashentryImportance(entryToBeReplaced) >= importance)
   {
      bestMove = getHashentryMove(entryToBeReplaced);
   }

   data = _getHashData(value, importance, bestMove, hashtable->date, flag);
   staticValueData = _getStaticValueData(staticValue, futilityMargin);
   entryToBeReplaced->key = key ^ data ^ staticValueData;
   entryToBeReplaced->data = data;
   entryToBeReplaced->staticValueData = staticValueData;
}

Hashentry *getDatedEntry(Hashtable * hashtable, UINT64 key)
{
   const int index = (int) getHashIndex(hashtable, key);
   int i;

   for (i = 0; i < CLUSTER_SIZE; i++)
   {
      Hashentry *tableEntry = &hashtable->table[index + i];

      if (getHashentryKey(tableEntry) == key)
      {
         if (getHashentryDate(tableEntry) != hashtable->date)
         {
            const INT16 value = getHashentryValue(tableEntry);
            const UINT8 importance = getHashentryImportance(tableEntry);
            const UINT16 bestMove = getHashentryMove(tableEntry);
            const UINT8 flag = getHashentryFlag(tableEntry);
            const UINT64 data = _getHashData(value, importance, bestMove,
                                             hashtable->date, flag);

            tableEntry->key = key ^ data ^ tableEntry->staticValueData;
            tableEntry->data = data;
         }

         return tableEntry;
      }
   }

   return 0;
}

Hashentry *getIndexedDatedEntry(Hashtable * hashtable, UINT64 key,
                                int *tableIndex)
{
   const int index = (int) getHashIndex(hashtable, key);
   int i;

   assert(*tableIndex >= 0);
   assert(*tableIndex < CLUSTER_SIZE);

   for (i = *tableIndex; i < CLUSTER_SIZE; i++)
   {
      Hashentry *tableEntry = &hashtable->table[index + i];

      if (getHashentryKey(tableEntry) == key)
      {
         *tableIndex = i + 1;

         return tableEntry;
      }
   }

   *tableIndex = CLUSTER_SIZE;

   return 0;
}

void refreshEntryDate(Hashtable * hashtable, UINT64 key,
                      Hashentry * tableEntry)
{
   if (getHashentryDate(tableEntry) != hashtable->date)
   {
      const INT16 value = getHashentryValue(tableEntry);
      const UINT8 importance = getHashentryImportance(tableEntry);
      const UINT16 bestMove = getHashentryMove(tableEntry);
      const UINT8 flag = getHashentryFlag(tableEntry);
      const UINT64 data = _getHashData(value, importance, bestMove,
                                       hashtable->date, flag);

      tableEntry->key = key ^ data ^ tableEntry->staticValueData;
      tableEntry->data = data;
   }
}

int initializeModuleHash()
{
   initializeHashtable(&globalHashtable);
   setHashtableExponent(&globalHashtable, 14);
   resetHashtable(&globalHashtable);

   return 0;
}

static int testAgeCalculation()
{
   Hashtable hashtable;

   hashtable.date = 0;
   assert(getAge(&hashtable, (UINT8) (NUM_DATES - 1)) == 1);
   assert(getAge(&hashtable, 0) == 0);
   assert(getAge(&hashtable, 1) == NUM_DATES - 1);

   hashtable.date = 2;
   assert(getAge(&hashtable, 1) == 1);
   assert(getAge(&hashtable, 2) == 0);
   assert(getAge(&hashtable, 3) == NUM_DATES - 1);

   hashtable.date = (UINT8) (NUM_DATES - 1);
   assert(getAge(&hashtable, (UINT8) (NUM_DATES - 2)) == 1);
   assert(getAge(&hashtable, (UINT8) (NUM_DATES - 1)) == 0);
   assert(getAge(&hashtable, 0) == NUM_DATES - 1);

   return 0;
}

int testModuleHash()
{
   int result = 0;

   if ((result = testAgeCalculation()) != 0)
   {
      return result;
   }

   return 0;
}
