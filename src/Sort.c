/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */

#ifdef AMIGA
#include <proto/utility.h>
#endif

#include <string.h>
#include <math.h>

#include "Buf.h"
#include "Edit.h"
#include "Sort.h"

/************************************************
*
*  ShellSort(char ***, int antal_fält, int antal_sort_falt, int antal_poster, char flags)
*
*  Sorterar enligt Shell-Metznermetoden.
*  Input:  Arraypekare till en stringarray,
*          totala antalet fält (multipliceras med två av sorten),
*          antalet fält som ska ingå i sorteringen (multipliceras med två av sorten),
*          antalet poster.
*          SORT_CASE | SORT_BACK
*  Returnerar ingenting.
*
*  Varannan array i stringarrayen skall peka på längderna för motsvarande
*  sträng.  Om strängen är nollavslutad, sätt pekaren till NULL.
*********/
void ShellSort(char ***StringArrayArray, int no_falt, int sort_no_falt, int no_post, char flags)
{
  int countM=no_post;
  int countI, countJ, countK, countL;
  int stop=FALSE;
  int countA;
  int result;
  BOOL kejs=flags&SORT_INCASE;
  BOOL reverse=flags&SORT_BACK;

  no_falt*=2;
  sort_no_falt*=2;
  do {
    countM=countM/2;
    if (!countM)
      stop=TRUE;
    else {
      countJ=0;
      countK=no_post-countM-1;
      
      do {
        countI=countJ;
        do {
          countL=countI+countM;
          for (countA=0; countA<sort_no_falt; countA+=2) {
            {
              register char *stringA=StringArrayArray[countA][countI];
              register char *stringB=StringArrayArray[countA][countL];
              if (!StringArrayArray[countA+1]) {
                if (!kejs)
                  result=strcmp(stringA, stringB);
                else
                  result=Stricmp(stringA, stringB);
              } else {
                register int len=min((int)StringArrayArray[countA+1][countI],(int)StringArrayArray[countA+1][countL]);
                if (!kejs)
                  result=strncmp(stringA, stringB, len);
                else
                  result=Strnicmp(stringA, stringB, len);
                if (!result) {
                  if (StringArrayArray[countA+1][countI]!=StringArrayArray[countA+1][countL]) {
                    if (StringArrayArray[countA+1][countI]<StringArrayArray[countA+1][countL])
                      result=-1;
                    else
                      result=1;
                  }
                }
              }
            }
            if (reverse)
              result=-result;
            if (result>0) {
              for (countA=0; countA<no_falt; countA+=2) {
                SWAP4(StringArrayArray[countA][countI],StringArrayArray[countA][countL]);
                if (StringArrayArray[countA+1]) {
                  SWAP4(StringArrayArray[countA+1][countI],StringArrayArray[countA+1][countL]);
                }
              }
              countI-=countM;
              countA=sort_no_falt;
            } else if (result<0 || countA>=(sort_no_falt-2)) {
              countI=-1;
              countA=sort_no_falt;
            }
          }
        } while (countI>=0);
        countJ+=1;
      } while (countJ<=countK);
    }
  } while (!stop);
}
