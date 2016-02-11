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

INLINE INT16 getHashentryValue(const Hashentry * entry)
{
   return (INT16) (entry->data & 0xFFFF);
}

INLINE INT16 getHashentryStaticValue(const Hashentry * entry)
{
   return (INT16) (entry->staticValueData & 0xFFFF);
}

INLINE INT16 getHashentryFutilityMargin(const Hashentry * entry)
{
   return (INT16) ((entry->staticValueData >> 16) & 0xFFFF);
}

INLINE UINT8 getHashentryImportance(const Hashentry * entry)
{
   return (UINT8) ((entry->data >> 16) & 0xFF);
}

INLINE UINT16 getHashentryMove(const Hashentry * entry)
{
   return (UINT16) ((entry->data >> 32) & 0xFFFF);
}

INLINE UINT8 getHashentryDate(const Hashentry * entry)
{
   return (UINT8) ((entry->data >> 48) & 0xFF);
}

INLINE UINT8 getHashentryFlag(const Hashentry * entry)
{
   return (UINT8) ((entry->data >> 56) & 0xFF);
}

INLINE UINT64 getHashentryKey(const Hashentry * entry)
{
   return entry->key ^ entry->data ^ (UINT64) (entry->staticValueData);
}
