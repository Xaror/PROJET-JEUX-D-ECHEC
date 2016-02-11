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

#ifndef _tools_h_
#define _tools_h_

#ifdef GCC_INLINE_STYLE
#undef INLINE_IN_HEADERS
#else
#define INLINE_IN_HEADERS 1
#endif

#ifndef TARGET_LINUX
#define TARGET_WINDOWS 1
#define MSFT_CC
#define CDECL __cdecl
#else
#define CDECL
#endif

#ifdef TARGET_WINDOWS
#pragma warning( disable : 4996; once : ; error : )
#endif

#define INCLUDE_TABLEBASE_ACCESS        /* activate egtb access code */

#define avg(a,b) (((a)+(b))/2)
#define max4(a,b,c,d)      ( max(max((a),(b)),max((c),(d))) )

/**
 * Get the system time in milliseconds.
 */
unsigned long getTimestamp(void);

/**
 * Get the process time in milliseconds.
 */
long getProcessTimestamp(void);

typedef struct
{
   char *buffer;
   char *tail;
   unsigned int bufferSize;
}
String;

/**
 * Get an initialized empty string.
 */
String getEmptyString(void);

/**
 * Get an initialized string.
 */
String getString(const char *buffer, const char *lastChar);

/**
 * Free all memory allocated for the specified String.
 */
void deleteString(String * string);

/**
 * Append a formatted C-string to String.
 *
 * @return a pointer to the new String
 */
String *appendToString(String * string, const char *fmt, ...);

/**
 * Break the string specified by 'buffer' into lines of a maximum length
 * of 'maxLineLength'. The breaks will be realised by substituting
 * appropriate space characters with newline characters.
 */
void breakLines(char *buffer, unsigned int maxLineLength);

/**
 * Remove all space characters from the beginning and from the end
 * of 'buffer'.
 */
void trim(char *buffer);

/**
 * Get a token delimited by 'tokenDelimiters'.
 *
 * @return a substring of token, obtained using 'malloc'
 */
char *getToken(const char *token, const char *tokenDelimiters);

/**
 * Test if a number is a prime.
 *
 * @return 0 if 'candidate' is no prime
 */
int isPrime(unsigned long candidate);

/**
 * Calculate y = ln(x) 
 *
 * @return a rounded integer
 */
int logIntValue(const double zeroPoint, const int maxPoint,
                const double maxValue, const int x);

/**
 * Initialize this module.
 *
 * @return 0 if no errors occurred.
 */
int initializeModuleTools(void);

/**
 * Test this module.
 *
 * @return 0 if all tests succeed.
 */
int testModuleTools(void);

#endif
