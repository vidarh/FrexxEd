/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************
*
* DoSearch.c
*
* Perform the Boyer-Moore search...
*
* Den letar alltid från översta positionen till men inte med sista.
********/

#include <exec/types.h>
#include <string.h>
#include <stdio.h>
#include "Buf.h"
#include "Fact.h"
#include "Fold.h"
#include "Search.h"

extern char search_fastmap[256];
extern FACT *DefaultFact;

void __regargs BMS_init_backward(char *buffer, int len, FACT *fact, char *fastmap)
{
  memset(fastmap, len, 256);
  while (--len>=0) {
    fastmap[buffer[len]]=len;
    if (fact && (fact->xflags[buffer[len]]&(factx_UPPERCASE|factx_LOWERCASE)))
      fastmap[fact->cases[buffer[len]]]=len;
  }
}
void __regargs BMS_init_forward(char *buffer, int len, FACT *fact, char *fastmap)
{
  int count=0;
  memset(fastmap, len, 256);
  while (len-count-1>=0) {
    fastmap[buffer[count]]=len-count-1;
    if (fact && (fact->xflags[buffer[count]]&(factx_UPPERCASE|factx_LOWERCASE)))
      fastmap[fact->cases[buffer[count]]]=len-count-1;
    count++;
  }
}


/********************************************************************
*
* void DoSearch(SearchStruct *);
*
* Hitta rätt sträng framåt, genom innehållet i SearchStructen.
* Returnerar TRUE/FALSE.
******/
BOOL __regargs DoSearch(SearchStruct *search, FACT *fact)
{
  int line=search->begin_y;
  register int byte=search->begin_x;
  char *casetable=NULL;
  int range=search->range;
  int jump;

  int linelength;
  int templine;

  search->found_x=-1;
  search->found_y=-1;

  if (!(search->flags & CASE_SENSITIVE))
    casetable=fact->cases;

  linelength=search->text[line].length;
  if (!search->find_direct && range) {
    byte++;	// Begin search one char ahead
  }
  byte+=search->buflength-1;
  while (byte>=linelength) {
    byte-=linelength;
    line++;
    if (line>search->end_y)
      break;
    linelength=search->text[line].length;
    if (line==search->end_y)
      linelength=search->end_x;
  }

  if (!search->buflength || line>search->end_y || (line==search->end_y && byte>=search->end_x))
    return(FALSE);

  do {
    char *text=search->text[line].text;
    do {
      if (!(jump=search_fastmap[text[byte]])) {
        char *tempradpek;
        register char *temptext=&text[byte];
        register int count=1;
        templine=line;
        tempradpek=search->text[line].text;
        if (line==search->begin_y)
          tempradpek+=search->begin_x;	// Borde behöve en Col2Byte-konvertering (950504)
        do {
          if (count==search->buflength) {
            register OK=TRUE;
            if (search->flags & ONLY_WORDS) {
              if (temptext==tempradpek) {
                if (templine>search->begin_y &&
                   (fact->xflags[search->text[templine-1].text[search->text[templine-1].length-1]] & factx_CLASS_WORD))
                  OK=FALSE;
              } else if ((templine<search->end_y || byte<search->end_x) && (fact->xflags[*(temptext-1)] & factx_CLASS_WORD))
                OK=FALSE;
              if (byte>=linelength-1) {
                if ((line+1<search->end_y || (line+1==search->end_y && 0<search->end_x)) &&
                       (fact->xflags[*(search->text[line+1].text)] & factx_CLASS_WORD))
                OK=FALSE;
              } else {
                if (fact->xflags[text[byte+1]] & factx_CLASS_WORD)
                  OK=FALSE;
              }
            }
            if (OK) {
              search->found_y=templine;
              search->found_x=temptext-search->text[templine].text;
              if (templine==search->end_y &&
                  &text[byte]-search->text[templine].text>=search->end_x) {
                search->found_x=-1;
                break;
              }
              search->last_x=byte;
              search->last_y=line;
              return(TRUE);
            }
            break;
          }
          temptext--;
          if (temptext<tempradpek) {	// OBS 'byte' får inte användas...
            templine--;
            if (templine<search->begin_y)
              break;
            temptext=search->text[templine].text+search->text[templine].length-1;
            tempradpek=search->text[templine].text;
          }
          count++;
        } while (*temptext==search->buffer[search->buflength-count] ||
                 (casetable && casetable[*temptext]==search->buffer[search->buflength-count] &&
                  (fact->xflags[*temptext]&(factx_UPPERCASE|factx_LOWERCASE))));
        byte++;
        range--;
        if (range<=0)
          return(FALSE);
      } else {
        byte+=jump;
        range-=jump;
        if (range<=0)
          return(FALSE);
      }
    } while (byte<linelength);
    do {
      byte-=linelength;
      line++;
      if (!(search->flags&INSIDE_FOLD)) {
        while (line<=search->end_y && (search->text[line].flags&FOLD_HIDDEN))
          line++;
      }
      linelength=search->text[line].length;
      if (line>=search->end_y) {
        if (line>search->end_y)
          return(FALSE);
        if (line==search->end_y)
          linelength=search->end_x;
      }
    } while (byte>=linelength);
  } while (1);
  return(FALSE);
}

/********************************************************************
*
* void DoSearchBack(SearchStruct *);
*
* Hitta rätt sträng bakåt, genom innehållet i SearchStructen.
* Returnerar TRUE/FALSE.
******/
BOOL __regargs DoSearchBack(SearchStruct *search, FACT *fact)
{
  int line=search->begin_y, byte=search->begin_x;
  char *casetable=NULL;
  int range=search->range;
  int jump;

  int linelength, templength;
  int templine;

  search->found_x=-1;
  search->found_y=-1;

  if (!(search->flags & CASE_SENSITIVE))
    casetable=fact->cases;

  linelength=search->text[line].length;

  if (range) {	/* Bara för backsearch.  Rutinen flyttar bak början */
    register int counter=search->buflength;
    range-=counter-2;
    while (counter>=0) {
      if (byte<0) {
        line--;
        if (line<search->end_y)
          return(FALSE);
        linelength=search->text[line].length;
        byte=linelength-1;
      } else if (counter>0) {
        byte--;
      }
      counter--;
    }
  }

  if (range<0 || !search->buflength || line<search->end_y || (line==search->end_y && byte<search->end_x))
    return(FALSE);

  do {
    char *text=search->text[line].text;
    do {
      if (!(jump=search_fastmap[text[byte]])) {
      /* När vi har hittat första tecknet så letar vi fram som vanligt och
         checkar om resten av strängen är rätt. */
        register char *temptext=&text[byte];
        register int count=0;
        templength=linelength;
        templine=line;
        do {
          count++;
          if (count==search->buflength) {
            register OK=TRUE;
            if (templine>search->begin_y || (templine==search->begin_y && (count-templength-byte)>search->begin_x))
              return(FALSE);
            if (search->flags & ONLY_WORDS) {
              if (byte==0) {
                if (line>search->end_y &&
                   (fact->xflags[search->text[line-1].text[search->text[line-1].length-1]] & factx_CLASS_WORD))
                  OK=FALSE;
              } else if (fact->xflags[text[byte-1]] & factx_CLASS_WORD)
                OK=FALSE;
              if (byte==linelength-1) {
                if ((line+1<search->begin_y || 0<search->begin_x) &&
                       (fact->xflags[*(search->text[line+1].text)] & factx_CLASS_WORD))
                OK=FALSE;
              } else {
                if (fact->xflags[temptext[1]] & factx_CLASS_WORD)
                  OK=FALSE;
              }
            }
            if (OK) {
              if (line!=search->end_y || byte>=search->end_x) {
                search->found_y=line;
                search->found_x=byte;
                search->last_y=templine;
                search->last_x=temptext-search->text[templine].text;
                return(TRUE);
              }
            }
            break;
          }
          if (byte+count>=templength) {
            templine++;
            temptext=search->text[templine].text;
            templength=search->text[templine].length+byte+count;
          } else
            temptext++;
        } while (*temptext==search->buffer[count] ||
                 (casetable && casetable[*temptext]==search->buffer[count] &&
                  (fact->xflags[*temptext]&(factx_UPPERCASE|factx_LOWERCASE))));
        byte--;
        range--;
        if (range<=0)
          return(FALSE);
      } else {
        byte-=jump;
        range-=jump;
        if (range<=0)
          return(FALSE);
      }
    } while (byte>=0);
    do {
      line--;
      if (!(search->flags&INSIDE_FOLD)) {
        while (line<=search->end_y && (search->text[line].flags&FOLD_HIDDEN))
          line--;
      }
      linelength=search->text[line].length;
      byte+=linelength;
      if (line<search->end_y || (line==search->end_y && byte<search->end_x))
        return(FALSE);
    } while (byte<0);
  } while (1);
  return(FALSE);
}


char __regargs *FindTextInString(String *text, String *searchstring)
{
  char fastmap[256];	// Place it on the stack
  char *byte;
  int range;
  int jump;
  int count;

  BMS_init_forward(searchstring->string, searchstring->length, DefaultFact, fastmap);
  range=searchstring->length-1;
  byte=text->string+range;
  range=text->length-range;

  while (range>0) {
    if (!(jump=fastmap[*byte])) {
      for (count=1; count<searchstring->length; count++) {
        if (byte[-count]!=searchstring->string[searchstring->length-count-1])
          break;
      }
      if (count==searchstring->length) {
        text->length-=byte-text->string;
        text->string=byte;
        return(&byte[-count+1]);
      }
      jump=1;
    }
    byte+=jump;
    range-=jump;
  }
  return(NULL);
}
