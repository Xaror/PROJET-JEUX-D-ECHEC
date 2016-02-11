#include <stdio.h>
#include <limits.h>
#include <stdlib.h>

#define RANDOMSEED 15021965
#define VALUESEED  12135172739435486027ull
#define NUMVALUES 1024

typedef unsigned short UINT16;

#define MAX_UINT16 USHRT_MAX

#ifdef MSFT_CC
typedef unsigned __int64 UINT64;
#else
typedef unsigned long long UINT64;
#endif

int numBitsSet[MAX_UINT16];
UINT64 minTerm[64];

/*
 *
 * This program generates 64-bit hashkey-tables with a
 * guaranteed minimum Hamming distance.
 *
 * According to R. W. Hamming, 1950, the Hamming distance between two
 * numbers is the number of different bit positions in their binary
 * representation. Example: the Hamming distance of 17 and 18 is 2:
 *
 * 17 ^= 1001
 * 18 ^= 1010
 *			^^
 * The Hamming distance of a set of numbers is the smallest Hamming
 * distance of all pairs of the set.
 *
 * I recommend to generate tables with a Hamming distance of 24.
 *
 * @author Raimund Heid, Feb 16 1999
 *
 */

/*
 * Count the number of bits set in 'v'. This routine is only
 * used to generate a table.
 */
UINT16 countNumberOfSetBits(UINT64 v)
{
   int count = 0, i;

   for (i = 0; i < 16; i++)
   {
      count += (v >> i) & 1;
   }

   return (UINT16) count;
}

/*
 * Get the number of bits set in 'v'.
 */
int getNumberOfSetBits(UINT64 v)
{
   return numBitsSet[v & 0xFFFF] + numBitsSet[(v >> 16) & 0xFFFF] +
      numBitsSet[(v >> 32) & 0xFFFF] + numBitsSet[(v >> 48) & 0xFFFF];
}

/*
 * Check if 'candidate' would be a legal member in 'v': see if
 * it differs in at least 'minDistance' bit positions from every
 * value in 'v'.
 */
int isLegalMember(UINT64 candidate, UINT64 v[NUMVALUES], int minDistance)
{
   unsigned int i;

   for (i = 0; i < NUMVALUES; i++)
   {
      if (getNumberOfSetBits(candidate ^ v[i]) < minDistance)
      {
         return 0;
      }
   }

   return 1;
}

/*
 * Generate a set of Hamming numbers.
 *
 * @param amount the amount of numbers in the set
 * @param minDistance the minimum Hamming distance between all pairs
 *		  in the set.
 */
void getHammingNumbers(UINT64 vector[NUMVALUES], int minDistance)
{
   const UINT64 MAX16 = 0xFFFF;
   UINT64 min = 1, nextCandidate = VALUESEED, i;
   int j;

   for (i = 0; i <= MAX16; i++)
   {
      numBitsSet[i] = countNumberOfSetBits(i);
   }

   for (j = 0; j < 64; j++)
   {
      minTerm[j] = min;
      min *= 2;
   }

   for (j = 0; j < NUMVALUES; j++)
   {
      nextCandidate ^= minTerm[rand() % 64];

      while (!isLegalMember(nextCandidate, vector, minDistance))
      {
         nextCandidate ^= minTerm[rand() % 64];
      }

      vector[j] = nextCandidate;
      printf("*");
      fflush(stdout);

      if (j % 100 == 0 && j > 0)
      {
         printf(" %d ", j);
         fflush(stdout);
      }
   }

   printf("\n");
}

/*
 * Generate a C/C++ header file and write the generated Hamming numbers
 * into an array definition.
 */
void generateHeaderFile(const char *filename, int distance)
{
   UINT64 vector[NUMVALUES];
   FILE *file;
   int ap = 0;
   unsigned int j, k;

   getHammingNumbers(vector, distance);

   file = fopen(filename, "wb");

   fprintf(file, "/* Generated file -- do not edit! */\n");
   fprintf(file, "/* Hamming distance: %d */\n\n", distance);

   fprintf(file, "UINT64 GENERATED_KEYTABLE[16][64] ={");

   for (j = 0; j < 16; j++)
   {
      fprintf(file, "\n{");

      for (k = 0; k < 64; k++)
      {
         fprintf(file, "%lluull, ", vector[ap++]);

         if (k % 2 == 1)
         {
            fprintf(file, "\n");
         }
      }

      fprintf(file, "}");

      if (j < 15)
      {
         fprintf(file, ",");
      }
   }

   fprintf(file, "};\n\n");

   fclose(file);
}

/*
 * Read some arguments from the command line and start the whole stuff.
 */
int main(int argc, char *argv[])
{
   int distance = 24;

   srand(RANDOMSEED);

   if (argc == 3)
   {
      int arg = atoi(argv[2]);

      distance = (arg > 0 ? arg : distance);

      generateHeaderFile(argv[1], distance);
   }
   else
   {
      printf("usage: hamming <filename> <distance>\n");
      return -1;
   }

   return 0;
}
