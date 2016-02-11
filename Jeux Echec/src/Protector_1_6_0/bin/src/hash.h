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

#ifndef _hash_h_
#define _hash_h_

#include "protector.h"
#include "position.h"

extern Hashtable globalHashtable;

/**
 * Define the size of the specified hashtable.
 *
 * @param exponent the hashtable will contain 2^(exponent+1) entries
 */
void setHashtableExponent(Hashtable * hashtable, int exponent);

/**
 * Initialize the specified hashtable. Call this function exactly once
 * for every hashtable before using it.
 */
void initializeHashtable(Hashtable * hashtable);

/**
 * Reset the specified hashtable. Call this function in order to
 * erase all stored data.
 */
void resetHashtable(Hashtable * hashtable);

/**
 * Increment the date of the specified hashtable.
 */
void incrementDate(Hashtable * hashtable);

/**
 * Put the specified entry into the hashtable.
 */
void setDatedEntry(Hashtable * hashtable, UINT64 key, INT16 value,
                   UINT8 importance, UINT16 bestMove, UINT8 flag,
                   INT16 staticValue, INT16 futilityMargin);

/**
 * Get the entry specified by key.
 */
Hashentry *getDatedEntry(Hashtable * hashtable, UINT64 key);

/**
 * Get the entry specified by key.
 */
Hashentry *getIndexedDatedEntry(Hashtable * hashtable, UINT64 key,
                                int *tableIndex);

/**
 * Get the entry specified by key. Start with *tableIndex.
 */
Hashentry *getIndexedDatedEntry(Hashtable * hashtable, UINT64 key,
                                int *tableIndex);

/**
 * Refresh the entry specified by tableEntry.
 */
void refreshEntryDate(Hashtable * hashtable, UINT64 key,
                      Hashentry * tableEntry);

INLINE INT16 getHashentryValue(const Hashentry * entry);
INLINE UINT8 getHashentryImportance(const Hashentry * entry);
INLINE UINT16 getHashentryMove(const Hashentry * entry);
INLINE UINT8 getHashentryDate(const Hashentry * entry);
INLINE UINT8 getHashentryFlag(const Hashentry * entry);
INLINE UINT64 getHashentryKey(const Hashentry * entry);
INLINE INT16 getHashentryStaticValue(const Hashentry * entry);
INLINE INT16 getHashentryFutilityMargin(const Hashentry * entry);

#ifdef INLINE_IN_HEADERS
#include "hashInline.h"
#endif

/**
 * Initialize this module.
 *
 * @return 0 if no errors occurred.
 */
int initializeModuleHash(void);

/**
 * Test this module.
 *
 * @return 0 if all tests succeed.
 */
int testModuleHash(void);

#endif
