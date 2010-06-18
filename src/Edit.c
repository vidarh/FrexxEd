/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
*
* Edit.c
*
* Support functions for some of the internal FrexxEd functions!
*
*******/

#include <stdio.h>
#include <string.h>
#include <math.h>

#include "Buf.h"
#include "Alloc.h"
#include "Block.h"
#include "Command.h"
#include "Cursor.h"
#include "Edit.h"
#include "FACT.h"
#include "Fold.h"
#include "GetFile.h"
#include "Limit.h"
#include "OpenClose.h"
#include "Slider.h"
#include "Undo.h"
#include "UpdtBlock.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"
#include "WindowOutput.h"

#include <intuition/preferences.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

extern char CursorOnOff;
extern struct IOStdReq *WriteReq;
extern char buffer[];
extern int bufferlen;

extern int Visible;
extern int UpDtNeeded;

/*************************************************************
*
* char *Deleteline(BufStruct *, int *);
*
* Performs a delete_line and returns a pointer to the character deleted
* with length at 'int *'.
* You have to free the return pointer yourself.
*********/
char __regargs *Deleteline(BufStruct *Storage, int *lenstore)
{
  int editrad=BUF(curr_line);
  String ret;

  Updated_Face(BUF(shared), editrad);
  ret.string=RAD(editrad);
  ret.length=LEN(editrad);
  SHS(size)-=LEN(editrad);
  if (ret.string>=SHS(fileblock) || ret.string<(SHS(fileblock)+SHS(fileblocklen))) {
    if (ret.length) {
      ret.string=Malloc(ret.length);
      if (!ret.string) {
        *lenstore=0;
        return(NULL);
      }
      memcpy(ret.string, RAD(editrad), ret.length);
    } else
      ret.string=NULL;
    DeallocLine(Storage, editrad);
  }
  if (editrad<SHS(line)) {
    if (editrad!=SHS(line) && (LFLAGS(editrad)&FOLD_BEGIN) && FOLD(editrad)==FOLD(editrad+1))
      LFLAGS(editrad+1)=LFLAGS(editrad);	// kopiera fold-flaggorna till nya raden
    movmem(SHS(text)+(editrad+1), SHS(text)+editrad,
           (SHS(line)-editrad)*sizeof(TextStruct));	/* Flytta textpekarna uppåt */
    SHS(line)--;
  } else {
    RAD(editrad)=NULL;
    LEN(editrad)=0;
    FOLD(editrad)=0;
    LFLAGS(editrad)=0;
  }
  BUF(cursor_x)=1;
  BUF(screen_x)=0;
  BUF(wanted_x)=1;
  BUF(string_pos)=0;

  DeleteLine(Storage, editrad, 1);
  if (Visible==VISIBLE_ON) {
    if (CheckPos(Storage) && 
        (!BUF(block_exists) || BUF(block_end_y)<=editrad)) {
      UpdateLines(Storage, editrad);
    } else {
      BUF(cursor_x)=1;
      BUF(string_pos)=0;
      UpdateScreen(Storage);
    }
    MoveSlider(Storage);
  } else {
    UpDtNeeded|=UD_REDRAW_BUFFS;
    SHS(update)|=UD_REDRAW_BUFFS;
  }
  if (LFLAGS(BUF(curr_line))&FOLD_BEGIN)
    FoldShow(Storage, BUF(curr_line));
  *lenstore=ret.length;
  return(ret.string);
}

/*************************************************************
*
* int Backspace(BufStruct *);
*
* Performs a backspace and returns the character deleted.
* -1==error
*********/
int __regargs Backspace(BufStruct *Storage)
{
  int retval=-1;
  int blockexists=0;
  int editrad=BUF(curr_line);

  if ((BUF(cursor_x)+BUF(screen_x))>1) {
    if (BUF(block_exists) &&
        BUF(block_end_y)==editrad &&
        BUF(block_end_x)>BUF(cursor_x)+BUF(screen_x)) {
      blockexists=Col2Byte(Storage, BUF(block_end_x), RAD(editrad), LEN(editrad));
    }
    {
      register char tecken=*(RAD(editrad)+(--BUF(string_pos)));
      if (BUF(using_fact)->flags[tecken] & fact_TAB)
        BUF(cursor_x)= Byte2Col(Storage, BUF(string_pos), RAD(editrad))-BUF(screen_x);
      else
        BUF(cursor_x)-=BUF(using_fact)->length[tecken];
    }
    retval=*(RAD(editrad)+BUF(string_pos));
    memcpy(RAD(editrad)+BUF(string_pos), RAD(editrad)+BUF(string_pos)+1, LEN(editrad)-BUF(string_pos));
    RAD(editrad)=ReallocLine(BUF(shared), editrad, LEN(editrad)-1);
    LEN(editrad)--;
    Updated_Face(BUF(shared), editrad);
    SHS(size)--;
    if (!CheckPos(Storage))
      PrintLine(Storage, editrad);
    if (blockexists) {
      BUF(block_end_x)=Byte2Col(Storage, blockexists-1, RAD(editrad));
      BUF(block_anc_x)=BUF(block_end_x);
    }
    if (BUF(block_exists)) {
      SetBlockMark(Storage, FALSE);	/* OBS! Updaterar för mycket (=hela blocket) */
    }
  } else if (editrad!=1) {                    /* backspace in column one */
    retval=*(RAD(editrad-1)+LEN(editrad-1)-1);	/* Ge tillbaka returnvärdet från föregående rad */
    if (editrad!=SHS(line) && (LFLAGS(editrad)&FOLD_BEGIN) && FOLD(editrad)==FOLD(editrad+1))
      LFLAGS(editrad+1)=LFLAGS(editrad);	// kopiera fold-flaggorna till nya raden
    if (CursorOnOff) {
      CursorXY(Storage, -1, -1);
    }
    if (BUF(block_exists)) {
      if (BUF(block_end_y)==editrad &&
          BUF(block_end_x)>BUF(cursor_x)+BUF(screen_x)) {
        blockexists=Col2Byte(Storage, BUF(block_end_x), RAD(editrad), LEN(editrad));
      } else {
/* DeleteLine()-funktionen borde fixa det här.
        if (BUF(block_end_y)>editrad) {
          BUF(block_anc_y)--;
          BUF(block_end_y)--;
        }
*/
      }
    }
    BUF(cursor_x)=Byte2Col(Storage, LEN(editrad-1)-1, RAD(editrad-1));
    BUF(string_pos)=LEN(editrad-1)-1;
    {
      register char *tempmem;
      tempmem=ReallocLine(BUF(shared), editrad-1, LEN(editrad-1)+LEN(editrad)-1);
      if (!tempmem)
        return(-1);
      RAD(editrad-1)=tempmem;
      Updated_Face(BUF(shared), editrad-1);
    }
    memcpy(RAD(editrad-1)+LEN(editrad-1)-1, RAD(editrad), LEN(editrad));
    LEN(editrad-1)+=LEN(editrad)-1;
    DeallocLine(Storage, editrad);
    movmem(SHS(text)+(editrad+1), SHS(text)+editrad,
           (SHS(line)-editrad)*sizeof(TextStruct));	/* Flytta textpekarna uppåt */
    SHS(size)--;
    SHS(line)--;
    if (blockexists) {
      BUF(block_end_x)=Byte2Col(Storage,
                                  blockexists+BUF(string_pos),
                                  RAD(editrad-1));
      BUF(block_end_y)--;
      BUF(block_anc_y)--;
      BUF(block_anc_x)=BUF(block_end_x);
    }
    DeleteLine(Storage, editrad, 1);

    BUF(curr_line)--;
    if (!(LFLAGS(BUF(curr_line))&FOLD_HIDDEN)) {
      BUF(cursor_y)--;
    }
    if (Visible==VISIBLE_ON) {
      Visible=VISIBLE_OFF;
      if (BUF(cursor_y)>BUF(uppermarg) && !CheckPos(Storage) &&
          (!BUF(block_exists) || BUF(block_end_y)<=editrad)) {
        Visible=VISIBLE_ON;
//        BUF(cursor_y)++;	/* OBS Behövs dethär */
        UpdateLines(Storage, editrad-1);
//        BUF(cursor_y)--;
      } else {
        if (BUF(cursor_y)<=0)
          TestCursorPos(Storage);
        Visible=VISIBLE_ON;
        UpdateScreen(Storage);
      }
      MoveSlider(Storage);
    } else {
      UpDtNeeded|=UD_REDRAW_ENTRY;
      BUF(update)|=UD_REDRAW_ENTRY;
    }
    if (BUF(block_exists))
      SetBlockMark(Storage, FALSE);
  }
  BUF(wanted_x)=BUF(cursor_x)+BUF(screen_x);
  return(retval);
}

/***********************************************
*
*  BackspaceNr(BufStruct *, char *, int)
*
*  Perform (int) backspaces.  Får bara tillkallas med ett godkänt antal steg.
*  Return the undo-string in (char *).
*
*  Return also a ret-value.
*******/
int __regargs BackspaceNr(BufStruct *Storage, char *retstring, int antal)
{
  int counter, len, slask, ret=OK;
  char *retstring2;

  for (counter=antal-1; counter>=0; counter--) {
    if (BUF(cursor_x)==1) {
      len=LEN(BUF(curr_line)-1)-1;
      if (counter>=len) {
        BUF(cursor_y)--;
        BUF(curr_line)--;
        retstring2=(char *)Deleteline(Storage, &slask);
        if (retstring2) {
          memcpy(&retstring[counter-len],retstring2, len+1);
          Dealloc(retstring2);
        }
        counter-=len;
      } else {
        register int val=Backspace(Storage);
        if (val<0) {
          ret=OUT_OF_MEM;
          break;  // for()
        }
        retstring[counter]=(char)val;
      }
    } else {
      register int val=Backspace(Storage);
      if (val<0) {
        ret=OUT_OF_MEM;
        break;  // for()
      }
      retstring[counter]=(char)val;
    }
  }
  BUF(wanted_x)=BUF(cursor_x)+BUF(screen_x);
  return(ret);
}

/***********************************************
*
*  BackspaceUntil(BufStruct *, int, int, char **, int *)
*
*  Perform backspaces until given string-position.
*  Return the undo-string in (char **) with length (int *).
*
*  Return also a ret-value.
*******/
int __regargs BackspaceUntil(BufStruct *Storage, int xstop, int ystop, char **retstring, int *retlength)
{
  int antal, ret=OK;
  char *retstring3;
  int editrad=BUF(curr_line);

  {
    register int counter=editrad-1;
    antal=BUF(string_pos)-xstop;
    while (counter>=ystop)
      antal+=LEN(counter--);
  }
  retstring3=Malloc(antal+1);
  *retlength=0;

  if (retstring3) {
    ret=BackspaceNr(Storage, retstring3, antal);
    if (ret>=OK)
      *retlength=antal;
    else {
      Dealloc(retstring3);
      retstring3=NULL;
    }
  }
  *retstring=retstring3;
  return(ret);
}

/*************************************************************
*
* int OutputText(BufStruct *, char *, int, int, String *);
*
* Inserts a text-string (no backspaces) with length (int) 
* with insertmode if wanted (int).  If insert=FALSE the retstring
* will be placed in the (String *).
* Returns number of chars.
*
*********/
int __regargs OutputText(BufStruct *Storage, char *output, int length, int insert, String *getstring)
{
  int updt=0;
  int count=0, slask;
  int strlength, len;
  int editrad;
  int outputlen=length;
  int downcount;
  char *tempstring;
  char *retstring;	// Only if Insert=FALSE.
  int new_block_end_x=-1, new_block_anc_x=-1;
  int new_block_end_y, new_block_anc_y;

  if (getstring) {
    getstring->string=NULL;
    getstring->length=NULL;
    if (!insert) {
      downcount=length;
      retstring=Malloc(downcount*2);
      if (!retstring) {
        return(0);
      }
      getstring->string=retstring;
    }
  }
  Updated_Face(BUF(shared), BUF(curr_line));
  while (length>0) {
    {
       register int countstr=count;
       while(length && !(BUF(using_fact)->flags[output[countstr]] & fact_NEWLINE)) {
         length--;
         countstr++;
       }
       strlength=countstr-count;
    }

    editrad=BUF(curr_line);

    if (length && (BUF(using_fact)->flags[output[strlength+count]] & fact_NEWLINE) && (insert || !strlength)) {
      updt|=2;		// Avslutas med ett newline
      length--;
      strlength++;
      if (ExpandText(BUF(shared), SHS(line))<OK) {
        return(count);
      }
      SHS(line)++;
      movmem(SHS(text)+editrad, SHS(text)+(editrad+1),
             (SHS(line)-editrad)*sizeof(TextStruct));	/* Flytta textpekarna neråt */
      if (BUF(string_pos)==0) {
        TEXT(editrad+1)=TEXT(editrad);
        LFLAGS(editrad+1)&=~FOLD_BEGIN;	/* Töm begin */
        LEN(editrad)=strlength;
        RAD(editrad)=(char *)Malloc(strlength);
        /* (flaggorna är fortfarande identiska och behövs inte kopieras) */
        if (!RAD(editrad)) {
          LEN(editrad)=0;
          return(count);
        }
        movmem(&output[count], RAD(editrad), strlength);
        SHS(size)+=LEN(editrad);
        BUF(cursor_x)=1;
        BUF(screen_x)=0;
      } else {
        if (BUF(block_exists)) {
          if (BUF(block_end_y)==editrad && BUF(block_end_x)>BUF(cursor_x)) {
            new_block_end_x=Byte2Col(Storage,
                                      Col2Byte(Storage,
                                               BUF(block_end_x),
                                               RAD(editrad),
					 LEN(editrad))-BUF(string_pos),
                                      RAD(editrad)+BUF(string_pos));
            new_block_end_y=BUF(block_end_y)+1;
            if (BUF(block_exists)&be_MARKING) {
              new_block_anc_y=BUF(block_anc_y)+1;
              new_block_anc_x=BUF(block_end_x);
            }
          }
/*
          else if (BUF(block_end_y)>editrad) {
            BUF(block_end_y)++;
            if (BUF(block_exists)&be_MARKING)
              BUF(block_anc_y)++;
          }
*/
        }
        TEXT(editrad+1)=TEXT(editrad);	/* kopiera flaggorna, pekarna skrivs över */
        LFLAGS(editrad+1)&=~FOLD_BEGIN;	/* Töm begin */
//        if (editrad+2<=SHS(line))
//          LFLAGS(editrad+1)|=LFLAGS(editrad+2)&FOLD_HIDDEN;	/* Sätt hidden */
        LEN(editrad+1)=LEN(editrad)-BUF(string_pos);
        len=LEN(editrad+1);
        RAD(editrad+1)=(char *)Malloc(len);
        if (!RAD(editrad+1)) {
          LEN(editrad+1)=0;
          return(count);
        }
        movmem(RAD(editrad)+BUF(string_pos), RAD(editrad+1), len);
        {
          register char *tempmem=(char *)ReallocLine(BUF(shared),editrad,BUF(string_pos)+strlength);
          if (!tempmem) {
            return(count);
          }
          RAD(editrad)=tempmem;
        }
        LEN(editrad)=BUF(string_pos)+strlength;
        SHS(size)+=strlength;
        movmem(&output[count],
               RAD(editrad)+BUF(string_pos),
               strlength);
        BUF(string_pos)=BUF(screen_x)=0;
        BUF(cursor_x)=1;
      }
      BUF(curr_line)++;
      if (!(LFLAGS(BUF(curr_line))&FOLD_HIDDEN)) {
        register int tempvisible=Visible;
        BUF(cursor_y)++;
        Visible=VISIBLE_OFF;
        CursorDown(Storage);
        Visible=tempvisible;
      }
      InsertLine(Storage, editrad, 1);
      count+=strlength;
    } else {	/* Inget newline ska sättas in */
      updt|=1;
      if (insert) {
        if (BUF(block_exists) && BUF(block_end_y)==editrad &&
            BUF(block_end_x)>BUF(cursor_x)+BUF(screen_x))
          slask=Col2Byte(Storage, BUF(block_end_x), RAD(editrad), LEN(editrad));
        else
          slask=0;
        len=(LEN(editrad))+strlength;
        tempstring=Malloc(len);
        if (!tempstring) {
          return(count);
        }
        if (BUF(string_pos)) {
          memcpy(tempstring,
                 RAD(editrad),
                 BUF(string_pos));
        }
        memcpy(tempstring+BUF(string_pos)+strlength,
               RAD(editrad)+BUF(string_pos),
               LEN(editrad)-BUF(string_pos));
        memcpy(tempstring+BUF(string_pos),
               &output[count],
               strlength);
        DeallocLine(Storage, editrad);
        SHS(size)-=LEN(editrad);
        RAD(editrad)=tempstring;
        LEN(editrad)=len;
        SHS(size)+=LEN(editrad);
        BUF(string_pos)+=strlength;
        BUF(cursor_x)=Byte2Col(Storage, BUF(string_pos), RAD(BUF(curr_line)))-BUF(screen_x); 
        count+=strlength;
        if (slask) {
          BUF(block_end_x)=Byte2Col(Storage, slask+strlength, RAD(editrad));
          BUF(block_anc_x)=BUF(block_end_x);
        }
      } else {
        int dif;
        dif=LEN(editrad)-BUF(string_pos)-(SHS(line)!=editrad)-strlength;
        if (0>dif) {
          register char *tempmem;
          tempmem=ReallocLine(BUF(shared), editrad, LEN(editrad)-dif);
          if (tempmem) {
            register int temp=LEN(editrad)-BUF(string_pos)-(SHS(line)!=editrad);
            RAD(editrad)=tempmem;
            if (temp) {
              getstring->length+=temp;
              downcount-=temp;
              memcpy(&retstring[downcount],
                     RAD(editrad)+BUF(string_pos), temp);
            }
            LEN(editrad)-=dif;
            SHS(size)-=dif;
            if (SHS(line)!=editrad)
              *(RAD(editrad)+LEN(editrad)-1)=BUF(using_fact)->newlinechar;
          } else
            break; // while (length>0)
        } else {
          getstring->length+=strlength;
          downcount-=strlength;
          memcpy(&retstring[downcount],
                 RAD(editrad)+BUF(string_pos), strlength);
        }
        memcpy(RAD(editrad)+BUF(string_pos), &output[count], strlength);
        count+=strlength;
        BUF(string_pos)+=strlength;
        BUF(cursor_x)=Byte2Col(Storage, BUF(string_pos), RAD(BUF(curr_line)))-BUF(screen_x); 
      }
    }
  }
  if (!insert) {
    if (downcount) {
      memcpy(retstring, retstring+downcount, outputlen*2-downcount);
    }
  }
  if (new_block_end_x>=0) {
    BUF(block_end_x)=new_block_end_x;
    BUF(block_end_y)=new_block_end_y;
  }
  if (new_block_anc_x>=0) {
    BUF(block_anc_x)=new_block_anc_x;
    BUF(block_anc_y)=new_block_anc_y;
  }
  if (!CheckPos(Storage)) {
    if (updt>1)
      UpdateDupScreen(Storage);
    else
      PrintLine(Storage, editrad);
  }
  return(count);
}


int __regargs GetWord(BufStruct *Storage, int line, int column, String *result)
{
  int type;
  int length=0;
  SharedStruct *shared=BUF(shared);
  char *rad=shared->text[line].text;

  result->length=0;
  result->string=NULL;
  if (rad && column>=0 && shared->text[line].length && column<shared->text[line].length) {
    type=BUF(using_fact)->xflags[rad[column]] & (factx_CLASS_SYMBOL|factx_CLASS_WORD|factx_CLASS_SYMBOL);
    while (--column>=0 && type&(BUF(using_fact)->xflags[rad[column]] & (factx_CLASS_SYMBOL|factx_CLASS_WORD|factx_CLASS_SYMBOL)))
      ;
    column++;
    while (length+column<shared->text[line].length) {
      if (!(type&(BUF(using_fact)->xflags[rad[column+length]] & (factx_CLASS_SYMBOL|factx_CLASS_WORD|factx_CLASS_SYMBOL))))
        break;
      length++;
    }
    result->length=length;
    result->string=&rad[column];
  }
  return(OK);
}

/*************************************************************
*
* int WCfwd(BufStruct *);
*
* Returns number of characters to be played with by other routines that handles
* entire words. (delete word, right word, etc)
*
*********/
int __regargs WCfwd(BufStruct *Storage)
{
  int len=0, countdown, rad=BUF(curr_line);
  char *text=RAD(rad)+BUF(string_pos);
  int class;

  countdown=LEN(rad)-BUF(string_pos);
  if (countdown<=0) {
    rad++;
    if (rad>SHS(line))
      return(0);		/* Nedre högra hörnet */
    text=RAD(rad);
    countdown=LEN(rad);
  }
/*
  if (BUF(using_fact)->xflags[*text] & factx_CLASS_SYMBOL)
    class=factx_CLASS_SYMBOL;
*/
  class=BUF(using_fact)->xflags[*text]&(factx_CLASS_SYMBOL|factx_CLASS_WORD|factx_CLASS_SPACE);

  while (BUF(using_fact)->xflags[*text] & class) {
    class&=BUF(using_fact)->xflags[*text];
    len++;
    if (!--countdown) {
      rad++;
      countdown=LEN(rad);
      text=RAD(rad);
      if (rad>SHS(line))
        break;
    } else {
      text++;
    }
  }
  if (rad<=SHS(line)) {
    while (BUF(using_fact)->xflags[*text] & factx_CLASS_SPACE) {
    len++;
      if (!--countdown) {
        rad++;
        countdown=LEN(rad);
        text=RAD(rad);
        if (rad>SHS(line) || !countdown)
          break;
      } else {
        text++;
      }
    }
  }
  if (!len)
    len++;
  return(len);
}

/*************************************************************
*
* int WCbwd(BufStruct *);
*
* Returns number of characters to be played with by other routines that handles
* entire words. (backspace word, left word, etc)
*
*********/
int __regargs WCbwd(BufStruct *Storage)
{
  int len=0, rad=BUF(curr_line);
  char *text=RAD(rad)+BUF(string_pos)-1;
  int countdown;
  int class;

  countdown=BUF(string_pos)-1;
  if (countdown<0) {
    rad--;
    if (rad==0)
      return(0);		/* Övre vänstra hörnet */
    text=RAD(rad)+LEN(rad)-1;
    countdown=LEN(rad)-1;
  }
  while (BUF(using_fact)->xflags[*text] & factx_CLASS_SPACE) {
    len++;
    if (--countdown<0) {
      rad--;
      countdown=LEN(rad)-1;
      text=RAD(rad)+LEN(rad)-1;
      if (rad==0)
        return(len);
    } else {
      text--;
    }
  }
/*
  if (BUF(using_fact)->xflags[*text] & factx_CLASS_SYMBOL)
    class=factx_CLASS_SYMBOL;
*/
  class=BUF(using_fact)->xflags[*text]&(factx_CLASS_SYMBOL|factx_CLASS_WORD|factx_CLASS_SPACE);
  while (BUF(using_fact)->xflags[*text] & class) {
    class&=BUF(using_fact)->xflags[*text];
    len++;
    if (--countdown<0) {
      rad--;
      countdown=LEN(rad)-1;
      text=RAD(rad)+LEN(rad)-1;
      if (rad==0)
        return(len);
    } else {
      text--;
    }
  }
  if (!len)
    len++;
  return(len);
}

/*************************************************************
*
* char *Del2Eol(BufStruct *, int *);
*
* Deletes the rest of the line and returns a char pointer to
* the deleted string with length at (int *).
*
*********/
char __regargs *Del2Eol(BufStruct* Storage, int *lenstore)
{
  String ret;
  int editrad=BUF(curr_line);
  int strpos=BUF(string_pos);	//Col2Byte(Storage,BUF(cursor_x)+BUF(screen_x),RAD(editrad), LEN(editrad));
  int len=LEN(editrad);

  *lenstore=0;

  if (BUF(block_exists) &&
      BUF(block_end_y)==editrad &&
      BUF(block_end_x)>BUF(cursor_x)+BUF(screen_x)) {
    BUF(block_end_x)=BUF(cursor_x)+BUF(screen_x);
    if (BUF(block_exists)&be_MARKING)
      BUF(block_anc_x)=BUF(block_end_x);
  }
  ret.length=len-strpos;
  if (ret.length>0) {
    Updated_Face(BUF(shared), editrad);
    ret.string=Malloc(ret.length);
    if (!ret.string)
      return(NULL);
    memcpy(ret.string, RAD(editrad)+strpos, ret.length);
    if (editrad!=SHS(line)) {	// Ej sista raden.
      ret.length--;		// Returnet ska inte bort.
      *(RAD(editrad)+strpos)=ret.string[ret.length];	// Kopiera returnet
      strpos++;			// Öka för allokering, och längd.
    }
    RAD(editrad)=(char *)ReallocLine(BUF(shared), editrad, strpos);
    LEN(editrad)=strpos;
    SHS(size)-=ret.length;
  } else {
    ret.string=NULL;
    ret.length=0;
  }
  if (Visible==VISIBLE_ON) {
    PrintLine(Storage, editrad);
  } else {
    UpDtNeeded|=UD_REDRAW_BUFFS;
    SHS(update)|=UD_REDRAW_BUFFS;
  }
  if (BUF(block_exists))
    SetBlockMark(Storage, FALSE);	/* OBS! Updaterar för mycket (=hela blocket) */
  *lenstore=ret.length;
  return(ret.string);
}

/******************************************************
*
*  InsertLine(BufStruct *, int, int)
*
*  To be called when a line is inserted in a duplicated buffer.
*********/
void __regargs InsertLine(BufStruct *Storage2, int rad, int antal)
{
  BufStruct *Storage=Storage2;
  ULONG *x, *y;
  char downcount;

  while (BUF(PrevSplitBuf))
    Storage=BUF(PrevSplitBuf);

  do {
    if (Storage!=Storage2) {

      if (BUF(curr_topline)>rad) {
        BUF(curr_topline)+=antal;
      } else if (BUF(curr_line)>=rad) {
        BUF(cursor_y)+=antal;
        BUF(curr_line)+=antal;
      }
    }

    if (BUF(block_exists)) {
      downcount=3;
      do {
        if (downcount==3) {
          x=&BUF(block_begin_x);
          y=&BUF(block_begin_y);
        } else if (downcount==2) {
          x=&BUF(block_end_x);
          y=&BUF(block_end_y);
        } else if (downcount==1) {
          x=&BUF(block_anc_x);
          y=&BUF(block_anc_y);
        }
        if (*y==rad &&
            *x>LEN(rad)) {
          *y+=antal;
          *x=Col2Byte(Storage,
                      *x-Byte2Col(Storage, LEN(rad), RAD(rad)),
                      RAD(rad+1),
                      LEN(rad+1));
        } else if (*y>rad) {
          *y+=antal;
        }
      } while (--downcount);
    }
    if (BUF(window))
      MoveSlider(Storage);
    Storage=BUF(NextSplitBuf);
  } while(Storage);
}

/******************************************************
*
*  DeleteLine(BufStruct *, int, int)
*
*  To be called when a line is deleted in a duplicated buffer.
*********/
void __regargs DeleteLine(BufStruct *Storage2, int rad, int antal)
{
  BufStruct *Storage=Storage2;

  while (BUF(PrevSplitBuf))
    Storage=BUF(PrevSplitBuf);

  do {
    if (BUF(block_exists)) {
      if (BUF(block_exists)&be_MARKING) {
        if (BUF(block_anc_y)==rad) {
          BUF(block_anc_x)=1;
        } else if (BUF(block_anc_y)>rad) {
          BUF(block_anc_y)-=antal;
          if (BUF(block_anc_y)<rad)
            BUF(block_anc_y)=rad;
        }
      }
      if (BUF(block_begin_y)==rad) {
        BUF(block_begin_x)=1;
      } else if (BUF(block_begin_y)>rad) {
        BUF(block_begin_y)-=antal;
        if (BUF(block_begin_y)<rad)
          BUF(block_begin_y)=rad;
      }
  
      if (BUF(block_end_y)==rad) {
        BUF(block_end_x)=1;
      } else if (BUF(block_end_y)>rad) {
        BUF(block_end_y)-=antal;
        if (BUF(block_end_y)<rad)
          BUF(block_end_y)=rad;
      }
      if (BUF(block_end_y)==BUF(block_begin_y) &&
          BUF(block_end_x)==BUF(block_begin_x)) {
        BUF(block_exists)=be_NONE;
      }
    }
    if (Storage!=Storage2) {
      register int updt=FALSE;

      if (FoldMoveLine(Storage, BUF(curr_topline), BUF(screen_lines))>rad) {
        if (BUF(curr_topline)<rad+antal) {
          if (BUF(curr_line)-rad>=0)
            BUF(cursor_y)-=min(antal, BUF(curr_line)+1-rad);
            BUF(curr_line)-=min(antal, BUF(curr_line)+1-rad);
          {
            register int oldtop=BUF(curr_topline);
            BUF(curr_topline)=min(BUF(curr_topline),rad);
            BUF(cursor_y)+=oldtop-BUF(curr_topline);
            BUF(curr_line)+=oldtop-BUF(curr_topline);
          }
          updt=TRUE;
        } else {
          if (BUF(curr_topline)>rad) {
            BUF(curr_topline)-=antal;
            updt=FALSE;
          } else {
            if (BUF(curr_line)>=rad) {
              BUF(cursor_y)-=antal;
              BUF(curr_line)-=antal;
              updt=TRUE;
            }
          }
        }
        if (TestCursorPos(Storage))
          updt=FALSE;
      }
      if (updt)
        UpdateScreen(Storage);
    }
    if (BUF(window))
      MoveSlider(Storage);
    Storage=BUF(NextSplitBuf);
  } while(Storage);
}


/* Expand the text with ALLOC_OVERHEAD+5% of the lines and return a ret-value. */
int __regargs ExpandText(SharedStruct *shared, int wanted)
{
  if (wanted+5>=shared->taket) {
    register char *tempmem;
    register int tempsize=wanted+ALLOC_OVERHEAD+shared->line/20;
    tempmem=Remalloc((char *)shared->text, sizeof(TextStruct)*tempsize);
    if (!tempmem)
      return(OUT_OF_MEM);
    shared->text=(TextStruct *)tempmem;
    shared->taket=tempsize;
  }
  return(OK);
}


int __regargs InsertText(BufStruct *Storage, char *text, int length, int position)
{
  int ret=OUT_OF_MEM;
  String buf;
  TextStruct *newtext;
  int lines;

  if (CompactBuf(BUF(shared), &buf)>=OK) {
    if (position+length>buf.length)
      buf.length=position+length;
    buf.string=Malloc(buf.length);
    if (buf.string) {
      memcpy(buf.string, SHS(fileblock), SHS(size));
      memcpy(&buf.string[position], text, length);
      if (ExtractLines(buf.string, buf.length, &newtext, &lines, NULL)>=OK) {
        Dealloc(SHS(fileblock));
        Dealloc(SHS(text));
        SHS(text)=newtext;
        SHS(fileblock)=buf.string;
        SHS(fileblocklen)=buf.length;
        SHS(size)=buf.length;
        SHS(line)=lines;
        SHS(taket)=lines;
        Command(NULL, DO_NOTHING|NO_HOOK, NULL, NULL, NULL);
        FreeUndoLines(BUF(shared), SHS(Undotop+1));
        {
          register BufStruct *stor=SHS(Entry);
          while (stor) {
            TestCursorPos(stor);
            MoveSlider(stor);
            UpdateScreen(stor);
            stor=stor->NextSplitBuf;
          }
        }
        ret=OK;
      } else
        Dealloc(buf.string);
    }
  }
  return ret;
}

void __regargs Pos2LineByte(SharedStruct *shared,
                           long pos,    /* position */
                           long *line,  /* is this line */
                           long *byte)  /* and this byte position */
{

  if(pos>shared->size) {
    *line = shared->line;
    *byte = shared->text[shared->line].length;
  }
  else {
    register long linecount=1;
    while(pos > shared->text[linecount].length) {
      pos -= shared->text[linecount].length;
      linecount++;
    }
    *line = linecount;
  }
  *byte = pos;
}
