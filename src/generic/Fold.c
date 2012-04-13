/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Fold.c
 *
 *********/

#include <stdlib.h>
#include <string.h>
#include <math.h>


#include "Buf.h"
#include "Alloc.h"
#include "FACT.h"
#include "Fold.h"
#include "UpdtScreen.h"

extern DefaultStruct Default;

int __regargs Fold(BufStruct *Storage, int startline, int endline)
{
  int count;
  int fold_no;
  startline=max(1, startline);
  startline=min(SHS(line), startline);
  endline=max(1, endline);
  endline=min(SHS(line), endline);
  while (startline<=SHS(line) && LFLAGS(startline)&FOLD_BEGIN)
    startline++;
  fold_no=FOLD(startline);
  if (startline<endline) {
    for (count=startline; FOLD(count)>=fold_no && count<endline; count++) {
      FOLD(count)++;
      LFLAGS(count)|=FOLD_HIDDEN;
    }
    LFLAGS(startline)=FOLD_BEGIN;
    if (BUF(curr_line)>startline)
      BUF(cursor_y)-=endline-startline-1;	/* justera raden */
    TestCursorPos(Storage);
    UpdateDupScreen(Storage);
  }
  return(OK);
}
int __regargs UnFold(BufStruct *Storage, int line)
{
  int fold_no, fold_flags;
  if (line==-1) {
    for (line=1; line<SHS(line); line++) {
      FOLD(line)=0;
      LFLAGS(line)=0;
    }
    TestCursorPos(Storage);
  } else {
    line=max(1, line);
    line=min(SHS(line), line);
    fold_no=FOLD(line);
    if (fold_no) {
      while (!(LFLAGS(line)&FOLD_BEGIN) || FOLD(line)!=fold_no)
        line--;
      LFLAGS(line)&=~FOLD_BEGIN;
      fold_flags=LFLAGS(line);
      do {
        if (FOLD(line)==fold_no || (FOLD(line)==fold_no+1 && (LFLAGS(line)&FOLD_BEGIN)))
          LFLAGS(line)=fold_flags|(LFLAGS(line)&FOLD_BEGIN);
        FOLD(line)--;
//        if (!FOLD(line) || (FOLD(line)==1 && (LFLAGS(line)&FOLD_BEGIN)))
//          LFLAGS(line)&=~FOLD_HIDDEN;
        line++;
      } while (line<=SHS(line) && (FOLD(line)>fold_no || (FOLD(line)==fold_no && !(LFLAGS(line)&FOLD_BEGIN))));
    }
  }
  UpdateDupScreen(Storage);
  return(OK);
}

int __regargs FoldShow(BufStruct *Storage, int line)
{
  int fold_no;
  int count=line;
  if (line==-1) {
    for (line=1; line<=SHS(line); line++) {
      LFLAGS(line)&=FOLD_BEGIN;	// Spara bara början-flaggan
    }
  } else {
    if (count==0)
      line=BUF(curr_line);
    else {
      line=max(1, line);
      line=min(SHS(line), line);
      line=FoldNearestLine(Storage, line);
    }
    while (line>1 && !(LFLAGS(BUF(curr_line))&FOLD_BEGIN))
      line--;
    if (count==0) {
      for (count=2; count<=SHS(line); count++)
        LFLAGS(count)|=FOLD_HIDDEN;
      LFLAGS(line)&=~FOLD_HIDDEN;
    } 
    fold_no=FOLD(line);
    line++;
    while (line<=SHS(line) && (LFLAGS(line)&FOLD_HIDDEN)) {
      if (FOLD(line)==fold_no && (LFLAGS(line)&FOLD_BEGIN))
        break;
      if (FOLD(line)==fold_no || (FOLD(line)==fold_no+1 && (LFLAGS(line)&FOLD_BEGIN)))
        LFLAGS(line)&=~FOLD_HIDDEN;
      line++;
    }
  }
  TestCursorPos(Storage);
  UpdateDupScreen(Storage);
  return(OK);
}
int __regargs FoldHide(BufStruct *Storage, int line)
{
  int fold_no;
  int count=line;
  if (line==-1) {
    for (line=1; line<SHS(line); line++) {
      if (FOLD(line)>1 || (FOLD(line)==1 && !(LFLAGS(line)&FOLD_BEGIN)))
        LFLAGS(line)|=FOLD_HIDDEN;
    }
  } else {
    line=max(1, line);
    line=min(SHS(line), line);
    if (count==0) {
      for (count=1; count<=SHS(line); count++) {
        if (!FOLD(count) || (FOLD(count)==1 && (LFLAGS(count)&FOLD_BEGIN)))
          LFLAGS(count)&=~FOLD_HIDDEN;
      }
      line=BUF(curr_line);
    }

    fold_no=FOLD(line);
    if (fold_no) {
      if (fold_no>1 && (LFLAGS(line+1)&FOLD_HIDDEN))
        fold_no--;
      while (line>0 && (!(LFLAGS(line)&FOLD_BEGIN) || FOLD(line)!=fold_no))
        line--;
      line++;
      while (line<=SHS(line) && (FOLD(line)>fold_no || (FOLD(line)==fold_no && !(LFLAGS(line)&FOLD_BEGIN)))) {
        LFLAGS(line)|=FOLD_HIDDEN;
        line++;
      }
    }
  }
  TestCursorPos(Storage);
  UpdateDupScreen(Storage);
  return(OK);
}

void __regargs AdjustStringPos(BufStruct *Storage)
{
  if (BUF(string_pos)>=LEN(BUF(curr_line))-1)
    BUF(string_pos)=LEN(BUF(curr_line))-1;
  BUF(cursor_x)=Byte2Col(Storage, BUF(string_pos), RAD(BUF(curr_line)))-BUF(screen_x);
  CheckPos(Storage);
}

void __regargs AdjustCursorUp(BufStruct *Storage)
{
  int templine=BUF(curr_line);
  while (BUF(curr_line)>1 && (LFLAGS(BUF(curr_line))&FOLD_HIDDEN)) {
    BUF(curr_line)--;
  }
  if (templine!=BUF(curr_line))
    AdjustStringPos(Storage);
}
void __regargs AdjustCursorDown(BufStruct *Storage)
{
  int templine=BUF(curr_line);
  while (BUF(curr_line)<SHS(line) && (LFLAGS(BUF(curr_line))&FOLD_HIDDEN)) {
    BUF(curr_line)++;
  }
  if (templine!=BUF(curr_line))
    AdjustStringPos(Storage);
}
int __regargs FoldNearestLine(BufStruct *Storage, int line)
{
  while (line>1 && (LFLAGS(line)&FOLD_HIDDEN)) {
    line--;
  }
  return(line);
}

int __regargs FoldDiff(BufStruct *Storage, int line1, int line2)
{
  int retint=0;
  if (line1<line2) {
    while (line1!=line2) {
      if (!(LFLAGS(line1)&FOLD_HIDDEN))
        retint++;
      line1++;
    }
  }
  while (line1!=line2) {
    if (!(LFLAGS(line1)&FOLD_HIDDEN))
      retint--;
    line1--;
  }
  return(retint);
}

/* Returnerar 'cursor_y' för en rad (0 om den är ovanför) (-1 om den är nedanför) */
int __regargs FoldFindLine(BufStruct *Storage, int line)
{
  int count=BUF(curr_topline);
  int ret=0;
  while (count<=line) {
    if (!(LFLAGS(count)&FOLD_HIDDEN)) {
      ret++;
      if (ret>BUF(screen_lines)) {
        ret=-1;
        break;	//break while
      }
    }
    count++;
  }
  return(ret);
}

/* Flyttar 'line' 'move' steg och hoppar över alla folds.  Returnera slutraden. */
int __regargs FoldMoveLine(BufStruct *Storage, int line, int move)
{
  if (line<SHS(line)) {
    while (move>0 && ++line<SHS(line)) {
      if (!(LFLAGS(line)&FOLD_HIDDEN))
        move--;
    }
  }
  if (line>1) {
    while (move<0 && --line>1) {
      if (!(LFLAGS(line)&FOLD_HIDDEN))
        move++;
    }
  }
  return(line);
}

/***************************************************
*
*  FoldCompactBuf(SharedStruct *, String *)
*
*  Lägger hela texten i en klump.
*  Ge en pekare för resultatet.  (if NULL, omorganisera valda buffern).
*  Returnerar ett ret-värde.
*
*******/
int __regargs FoldCompactBuf(SharedStruct *shared, String *retstring)
{
  int ret=OK;
  int rad=1;
  char *mem, *mem2;
  int size=0;
  int folds=0;
  int extrasize;
  int former_line=0;
  int temp;
  char newline=Default.BufStructDefault.using_fact->newlinechar;
  BOOL hidden=FALSE;

  if (Default.fold_save) {
    for (rad=shared->line; rad>0; rad--) {
      if (shared->text[rad].flags&FOLD_BEGIN)
        folds++;
    }
  }
  if (!folds) {
    ret=CompactBuf(shared, retstring);
  } else {
    extrasize=folds*(3+sizeof(FOLD_STRING_BEGIN)+sizeof(FOLD_STRING_END)+FOLD_STRING_OVERHEAD*3+
                     strlen(shared->comment_begin)+strlen(shared->comment_end));

    mem=Malloc(shared->size+extrasize);
    mem2=mem;
    if (!mem)
      return(OUT_OF_MEM);
  
    rad=1;
    former_line=0;
    do {
      memcpy(mem, shared->text[rad].text, shared->text[rad].length);
      mem+=shared->text[rad].length;
      size+=shared->text[rad].length;
      if (//shared->text[rad].length &&
          (former_line!=shared->text[rad].fold_no ||
           (shared->text[rad].flags&FOLD_BEGIN))) {
        if (former_line) {
          temp=0;
          while (former_line && 
                 (former_line > shared->text[rad].fold_no ||
                  (former_line >= shared->text[rad].fold_no &&
                   (shared->text[rad].flags&FOLD_BEGIN)))) {
            if (shared->text[rad].length) {
              mem--;
              newline=*mem;
            }
            size--;
            temp=Sprintf(mem, "%s%s%ld.%s%lc", shared->comment_begin, FOLD_STRING_END,
                          former_line, shared->comment_end, newline);
//            if (!shared->text[rad].length)
//              temp--;
            mem+=temp;
            size+=temp;
            former_line--;
          }
        }
        if (shared->text[rad].flags&FOLD_BEGIN) {
          hidden=FALSE;
          if ((shared->text[rad].flags&FOLD_HIDDEN) ||
               (!(shared->text[rad].flags&FOLD_HIDDEN) &&
               ((shared->text[rad+1].flags&FOLD_HIDDEN))))
            hidden=TRUE;
          size--;
          mem--;
          newline=*mem;
          temp=Sprintf(mem, "%s%s%ld.%ld.%ld%s.%s%lc", shared->comment_begin, FOLD_STRING_BEGIN,
                       shared->text[rad].fold_no, strlen(shared->comment_begin),
                       strlen(shared->comment_end),
                       hidden?"":",",
                       shared->comment_end, newline);
          mem+=temp;
          size+=temp;
        }
      }
      former_line=shared->text[rad].fold_no;
      rad++;
    } while (rad<=shared->line);

    if (retstring) {
      retstring->string=mem2;
      retstring->length=size;
    }
  }
  return(ret);
}
