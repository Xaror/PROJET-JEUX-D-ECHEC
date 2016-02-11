#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFSIZE  4096

typedef struct
{
   char firstPlayer[256];
   char secondPlayer[256];

   int numWinsFirst, numWinsSecond, draws;
}
Result;

static Result result;

static void interpretLine(const char *linebuffer, int lineCount)
{
   char *tag1, *tag2;

   if (lineCount == 1)
   {
      if ((tag1 = strstr(linebuffer, "Match ")) == NULL)
      {
         return;
      }

      tag1 += strlen("Match ");
      tag2 = strstr(linebuffer, " vs. ");

      strncpy(result.firstPlayer, tag1, tag2 - tag1);
      result.firstPlayer[tag2 - tag1] = '\0';

      tag1 = tag2 + strlen(" vs. ");
      tag2 = strstr(linebuffer, ": final score");

      strncpy(result.secondPlayer, tag1, tag2 - tag1);
      result.secondPlayer[tag2 - tag1] = '\0';
   }

   if ((tag1 = strstr(linebuffer, "score ")) == NULL)
   {
      return;
   }

   result.numWinsFirst += atoi(tag1 + strlen("score "));
   result.numWinsSecond += atoi(tag1 + strlen("score ") + 2);
   result.draws += atoi(tag1 + strlen("score ") + 4);
}

static void interpretFile(const char *filename)
{
   FILE *file = fopen(filename, "r");
   char linebuffer[BUFSIZE + 1];
   int lineCount = 0;
   float pointsFirst, pointsSecond, percentFirst, percentSecond;

   if (file == NULL)
   {
      printf("file %s couldn't be opened.\n", filename);

      return;
   }

   result.numWinsFirst = result.numWinsSecond = result.draws = 0;

   while (fgets(linebuffer, BUFSIZE, file) != NULL)
   {
      interpretLine(linebuffer, ++lineCount);
   }

   pointsFirst =
      (float) ((float) result.numWinsFirst + (float) result.draws / 2.0);
   pointsSecond =
      (float) ((float) result.numWinsSecond + (float) result.draws / 2.0);
   percentFirst =
      (float) ((pointsFirst * 100.0) / (pointsFirst + pointsSecond));
   percentSecond =
      (float) ((pointsSecond * 100.0) / (pointsFirst + pointsSecond));

   printf("%s: %.1f (%0.f%%), %s: %.1f (%0.f%%)\n", result.firstPlayer,
          pointsFirst, percentFirst, result.secondPlayer, pointsSecond,
          percentSecond);
}

int main(int argc, char *argv[])
{
   if (argc != 2)
   {
      printf("usage: resultcount <resultfile>\n");
   }
   else
   {
      interpretFile(argv[1]);
   }

   return 0;
}
