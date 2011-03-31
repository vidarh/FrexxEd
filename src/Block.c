/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
*
* Block.c
*
* Support block functions!
*
*******/

#ifdef AMIGA
#include <exec/memory.h>
#include <intuition/preferences.h>
#include <libraries/iffparse.h>
#include <proto/exec.h>
#include <proto/FPL.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <string.h>

#include "Buf.h"
//#include "Alloc.h"
#include "Block.h"
#include "BufControl.h"
#include "Button.h"
//#include "Command.h"
#include "Cursor.h"
#include "Edit.h"
#include "FACT.h"
#include "Fold.h"
#include "GetFile.h"
#include "IDCMP.h"
#include "Limit.h"
//#include "OpenClose.h"
#include "Reqlist.h"
//#include "Request.h"
//#include "Slider.h"
#include "Sort.h"
//#include "Strings.h"
#include "Undo.h"
#include "UpdtBlock.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"
#include "WindowOutput.h"

extern DefaultStruct Default;
extern BlockStruct *BlockBuffer;
extern int Visible;
extern int UpDtNeeded;
extern char CursorOnOff;
extern struct screen_buffer ScreenBuffer;
extern struct IOStdReq *WriteReq;
extern int allowcleaning;
extern char buffer[];
extern char GlobalEmptyString[];
extern FACT *DefaultFact;

/*********************************************************
*
*  BlockCopy(BufStruct *, BlockStruct *, int flag, BlockCutStruct *blockcut)
*
*  Copy your block to wanted block buffer (NULL = BlockBuffer).
*  Flags are defined in 'Block.h'.
*  Function clears block_exists flag.
*  Input:  wanted BufStruct pointer.
*  Return: a 'ret' value.
*
***********/
int __regargs BlockCopy(BufStruct *Storage, BlockStruct *block, int special, BlockCutStruct *blockcut)
{
  int ret=OK;
  int ycount, xcount, xstop;
  int linedeleted=0;
  int ydif;
  int len;
  int xstore;
  int append=special & bl_APPEND;
  int cut=special & bl_CUT;
  int undoadd=special & bl_UNDOADD;
  int noalloc=special & bl_NOALLOC;
  int nostore=special & bl_NOSTORE;
  int undosort;
  int tempcleaning=allowcleaning;
  int columnar;
  int cursorystop=BUF(curr_line);	/* Vilken rad cursorn ska stå på när cutten är klar */
  int cursorxstop=BUF(cursor_x)+BUF(screen_x);		/* motsvarande kolumn */
  char *undostring;
  int undolen=0;
  int endy, endx, starty, startx;

  allowcleaning=FALSE;

  if (!blockcut)
    blockcut=(BlockCutStruct *)&BUF(block_begin_x);

  if (!blockcut->block_exists || (blockcut->blocktype==bl_COLUMNAR && blockcut->block_end_x==blockcut->block_begin_x))
    return(NO_BLOCK_MARKED);

  if (special & bl_UNDOAPPEND)
    undosort=undoAPPENDBEFORE;
  else
    undosort=bl_NORMAL;

  if (!block)
    block=BlockBuffer;

  if (block==BUF(shared))
    return(EMPTY_BLOCKBUFFER);

  if (!append && !nostore)
    ClearBlock(block);

  columnar=FALSE;
  if (blockcut->blocktype==bl_COLUMNAR)
    columnar=TRUE;

  append=TRUE;
  if (block->line==0)
    append=FALSE;


  endy=blockcut->block_end_y;
  endx=blockcut->block_end_x;
  starty=blockcut->block_begin_y;
  startx=blockcut->block_begin_x;

  if (columnar) {
    if (startx>endx)
      SWAP4(startx,endx);
  } else
    xstop=Col2Byte(Storage, blockcut->block_end_x, RAD(blockcut->block_end_y), LEN(blockcut->block_end_y)+(SHS(line)==blockcut->block_end_y));

  if (blockcut->blocktype==bl_LINE) {
    startx=0;
    if (endy<SHS(line)) {
      endy=blockcut->block_end_y+1;
      endx=0;
      xstop=0;
    } else {
      xstop=LEN(endy);
//      xstop=Byte2Col(Storage, LEN(endy), RAD(endy));
      endx=xstop;
    }
  }

  if (cut) {
    Updated_Face(BUF(shared), starty);
  }

  ydif=endy-starty+4;
  if (append)
    ydif+=block->line;
  if (!nostore) {
    register TextStruct *tempmem;
    tempmem=(TextStruct *)Remalloc((char *)block->text,ydif*sizeof(TextStruct));
    if (!tempmem)
      return(OUT_OF_MEM);
    block->text=tempmem;
    block->taket=ydif;
  }
  ycount=starty;
  xcount=Col2Byte(Storage, startx, RAD(ycount), LEN(ycount)+(SHS(line)==ycount));

  if (cut) {
    blockcut->block_exists=be_NONE;
    xstore=xcount;
                     /* Flytta cursorn innan cutten (ej nödvändigt, men bra) */
    if (columnar) {
      if (cursorystop>=starty &&
          cursorystop<=endy) { 
        if (BUF(cursor_x)+BUF(screen_x) > startx) {
          if (BUF(cursor_x)+BUF(screen_x) < endx)
            cursorxstop=startx;
          else
            cursorxstop-=endx-startx;
        }
      }
    } else {
      if ((cursorystop > starty ||
           (cursorystop==starty &&
            BUF(cursor_x)+BUF(screen_x) > startx))) {
        if ((cursorystop < endy ||
             (cursorystop==endy &&
              BUF(cursor_x)+BUF(screen_x) < endx))) {
                                                    // cursorn är i blocket
          cursorxstop=startx;
          cursorystop=starty;
        } else {
          if (cursorystop >= endy) {
            if (cursorystop==endy)
              cursorxstop-=endx-startx;
            cursorystop-=endy-starty;
          }
        }
      }
    }
    BUF(curr_line)=starty;
    BUF(cursor_y)=starty-BUF(curr_topline)+1;
  }
  if (!nostore && (!append || !block->line || (block->text[1].length && (BUF(using_fact)->flags[*(block->text[block->line].text+block->text[block->line].length-1)] & fact_NEWLINE)))) {
    block->line++;
    block->text[block->line].text=NULL;
    block->text[block->line].length=0;
    block->text[block->line].fold_no=0;
    block->text[block->line].flags=0;
    append=FALSE;
  }
  if (blockcut->block_exists==be_MARKING)
    blockcut->block_exists=be_NONE;

  {
    if (cut && !columnar) {
      register int counter, temp=0;
      for (counter=starty; counter<endy; counter++)
        temp+=LEN(counter);
      temp+=xstop-xcount;
      undostring=Malloc(temp);
      if (!undostring)
        return(OUT_OF_MEM);
    }
    if (!columnar) {
      if (ycount==endy) {
        len=xstop-xcount;
        linedeleted--;
      } else
        len=LEN(ycount)-xcount;
    } else {
      xstop=Col2Byte(Storage, endx, RAD(ycount), LEN(ycount)+(SHS(line)==ycount));
      if (blockcut->blocktype==bl_LINE && xstop>LEN(ycount))
        xstop=LEN(ycount);
      len=xstop-xcount+(nostore?0:1);
    }
    if (!nostore) {
      if (append) {
        register char *tempmem;
        tempmem=Remalloc(block->text[block->line].text,
			 block->text[block->line].length+len);
        if (!tempmem)
          return(OUT_OF_MEM);
        block->text[block->line].text=tempmem;
        memcpy(tempmem+block->text[block->line].length, RAD(ycount)+xcount, len);
        block->text[block->line].length+=len;
        block->size+=len;
        if (columnar) {
          len--;
          tempmem[len]=BUF(using_fact)->newlinechar;
        }
      } else {
        block->text[block->line].length=len;
        block->size+=len;
        if (noalloc)
          block->text[block->line].text=RAD(ycount)+xcount;
        else {
          block->text[block->line].text=Malloc(len);
          if (!block->text[block->line].text)
            return(OUT_OF_MEM);
          memcpy(block->text[block->line].text, RAD(ycount)+xcount, len);
          if (columnar) {
            len--;
            block->text[block->line].text[len]=BUF(using_fact)->newlinechar;
          }
        }
      }
      block->text[block->line].fold_no=0;
      block->text[block->line].flags=0;
    }
    if (cut) {
      register int temp;
      if (columnar) {
        if (undoadd) {
          BUF(string_pos)=xcount;
          UndoAdd(Storage, RAD(ycount)+xcount, undosort, len);
          undosort=undoAPPENDAFTER;
        }
      } else {
        memcpy(undostring+undolen, RAD(ycount)+xcount, len);
        undolen+=len;
        BUF(string_pos)=0;
        linedeleted++;
      }
      memcpy(RAD(ycount)+xcount, RAD(ycount)+xcount+len, (temp=LEN(ycount)-len)-xcount);
      RAD(ycount)=ReallocLine(BUF(shared), ycount, temp);
      SHS(size)-=len;
      LEN(ycount)=temp;
    }
    ycount++;
    xcount=0;
    if (cut) {
      BUF(cursor_y)++;
      BUF(curr_line)++;
    }

    while (ycount<endy) {
      if (columnar) {
        xcount=Col2Byte(Storage, startx, RAD(ycount), LEN(ycount));
        xstop=Col2Byte(Storage, endx, RAD(ycount), LEN(ycount));
        len=xstop-xcount+(nostore?0:1);
      } else
        len=LEN(ycount);
      if (!nostore) {
        block->line++;
        if (noalloc)
          block->text[block->line].text=RAD(ycount)+xcount;
        else {
          block->text[block->line].text=Malloc(len);
          if (!block->text[block->line].text) {
            block->line--;
            return(OUT_OF_MEM);
          }
          memcpy(block->text[block->line].text, RAD(ycount)+xcount,len);
        }
        block->text[block->line].fold_no=0;
        block->text[block->line].flags=0;
        block->size+=len;
        block->text[block->line].length=len;
        if (columnar) {
          len--;
          block->text[block->line].text[len]=BUF(using_fact)->newlinechar;
        }
      }
      if (cut) {
        if (columnar) {
          register int temp;
          if (undoadd) {
            BUF(string_pos)=xcount;
            UndoAdd(Storage, RAD(ycount)+xcount, undosort, len);
          }
          memcpy(RAD(ycount)+xcount, RAD(ycount)+xcount+len, (temp=LEN(ycount)-len)-xcount);
          RAD(ycount)=ReallocLine(BUF(shared), ycount, temp);
          LEN(ycount)=temp;
          BUF(cursor_y)++;
          BUF(curr_line)++;
        } else {
          memcpy(undostring+undolen, RAD(ycount)+xcount, len);
          undolen+=len;
          linedeleted++;
        }
        SHS(size)-=len;
      }
      ycount++;
    }
    if (ycount<=endy) {
      if (xstop>0 || columnar) {
        if (columnar) {
          xcount=Col2Byte(Storage, startx, RAD(ycount), LEN(ycount)+(SHS(line)==ycount));
          xstop=Col2Byte(Storage, endx, RAD(ycount), LEN(ycount)+(SHS(line)==ycount));
          len=xstop-xcount+(nostore?0:1);
        } else
          len=xstop;
        if (!nostore) {
          block->line++;
          block->text[block->line].length=len;
          block->size+=len;
          if (noalloc)
            block->text[block->line].text=RAD(ycount)+xcount;
          else {
            block->text[block->line].text=Malloc(len);
            if (!block->text[block->line].text) {
              block->line--;
              return(OUT_OF_MEM);
            }
            block->text[block->line].fold_no=0;
            block->text[block->line].flags=0;
            memcpy(block->text[block->line].text,RAD(ycount)+xcount,len);
            if (columnar) {
              len--;
              block->text[block->line].text[len]=BUF(using_fact)->newlinechar;
            }
          }
        }
        
        if (cut) {
          register int temp;
          if (columnar) {
            if (undoadd) {
              BUF(string_pos)=xcount;
              UndoAdd(Storage, RAD(ycount)+xcount, undosort, len);
            }
          } else {
            memcpy(undostring+undolen, RAD(ycount)+xcount, len);
            undolen+=len;
          }
          memcpy(RAD(ycount)+xcount, RAD(ycount)+xcount+len, (temp=LEN(ycount)-len)-xcount);
          RAD(ycount)=ReallocLine(BUF(shared), ycount, temp);
          LEN(ycount)=temp;
          BUF(cursor_y)++;
          BUF(curr_line)++;
          SHS(size)-=len;
        }
      }
    }
    if (cut) {
      if (linedeleted) {
        register int count;
        register int blockstart=starty;
        for (count=1; count<linedeleted; count++)
          DeallocLine(Storage, count+blockstart);
        {
          register char *tempmem;
          tempmem=ReallocLine(BUF(shared), blockstart, LEN(blockstart)+LEN(ycount));
          if (!tempmem)
            return(OUT_OF_MEM);
          RAD(blockstart)=tempmem;
        }
        memcpy(RAD(blockstart)+LEN(blockstart), RAD(ycount), LEN(ycount));
        if (FOLD(blockstart)<FOLD(ycount)) {
          FOLD(blockstart)=FOLD(ycount);
          LFLAGS(blockstart)=(LFLAGS(ycount)&FOLD_HIDDEN)|FOLD_BEGIN;
          {
            register int count=ycount;
            register int no=0;
            if (blockstart>1)
              no=FOLD(blockstart-1);
            while (count<=SHS(line) && FOLD(count)!=no) {
              FOLD(count)=no+1;
              count++;
            }
          }
        }

        LEN(blockstart)+=LEN(ycount);
        DeallocLine(Storage, ycount);
        memcpy(SHS(text)+blockstart+1, SHS(text)+blockstart+linedeleted+1,
               (SHS(line)-blockstart-linedeleted)*sizeof(TextStruct));
        SHS(line)-=linedeleted;
        DeleteLine(Storage, blockstart, linedeleted);
      }
      if (!columnar) {
        register int temptopline=BUF(curr_topline);
        if (BUF(cursor_y)+temptopline>=endy)
          if (temptopline-linedeleted>0)
            temptopline=temptopline-linedeleted;
        {
          register int temp=Visible;
          Visible=VISIBLE_OFF;
          SetScreen(Storage, xstore, starty);
          Visible=temp;
        }
        UndoAdd(Storage, undostring, undosort, undolen);
        Dealloc(undostring);
        BUF(curr_topline)=temptopline;
      }
    }
    if (allowcleaning=tempcleaning)
      DeallocLine(Storage, 0);
    UpdateDupScreen(Storage);
    MoveSlider(Storage);
  }
  if (!nostore) {
    if (block->text[1].length && (BUF(using_fact)->flags[*(block->text[block->line].text+block->text[block->line].length-1)] & fact_NEWLINE)) {
      block->line++;
      block->text[block->line].text=GlobalEmptyString;
      block->text[block->line].length=0;
      block->text[block->line].fold_no=0;
      block->text[block->line].flags=0;
    }
  }
  if (!nostore && block->Entry) {
    FreeUndoLines(block, block->Undotop+1);
    UpdateDupScreen(block->Entry);
  }

  if (cut && blockcut==(BlockCutStruct *)&BUF(block_begin_x)) {
    SetScreen(Storage, Col2Byte(Storage, cursorxstop, RAD(cursorystop), LEN(cursorystop)), cursorystop);
  }
  return(ret);
}

/*********************************************************
*
*  BlockPaste(BufStruct *, int undosort, BlockStruct *block)
*
*  Paste wanted block (NULL = BlockBuffer).
*  Tell (int) how to access the undo buffer.
*  Tell (int) if the block is to be erased.
*  Return: a ret value.
*
***********/
int __regargs BlockPaste(BufStruct *Storage, int undosort, BlockStruct *block, int flag)
{
  int ret=OK, slask;
  int counter=1, count=0;
  char *stringtemp;
  int len;
  int cursorxstart;
  int editrad=BUF(curr_line);
  int tempcleaning=allowcleaning;
  int fold_no, fold_flags;
  BOOL blockkill=flag&bp_KILL;
  BOOL columnar=flag&bp_COLUMN;

  if (!block) {
    if (BUF(block_exists)==be_NONE) {
      block=BlockBuffer;
      if (block->line==0 || block==BUF(shared))
        return(EMPTY_BLOCKBUFFER);
    } else {
      block=(BlockStruct *)Malloc(sizeof(BlockStruct));
      if (!block)
        return(OUT_OF_MEM);
      memset(block, 0, sizeof(BlockStruct));
      ret=BlockCopy(Storage, block, 0, NULL);
      blockkill=TRUE;
      if (ret<OK) {
        ClearBlock(block);
        Dealloc((char *)block);
        return(ret);
      }
    }
  }
  if (!block->line || block==BUF(shared))
    return(EMPTY_BLOCKBUFFER);

  if (CursorOnOff) {
    CursorXY(Storage, -1, -1);
  }

  Updated_Face(BUF(shared), BUF(curr_line));

  if (block->line==1) {
    {
      register int len=block->text[1].length;
      if (columnar && len && (BUF(using_fact)->flags[block->text[1].text[len-1]] & fact_NEWLINE))
        len--;
      count=OutputText(Storage, block->text[1].text, len, TRUE, NULL);
    }
    UndoAdd(Storage, (char *)count, undosort | undoONLYBACKSPACES, 0);
    if (blockkill) 
      Dealloc(block->text[1].text);
  } else {
    Visible=VISIBLE_OFF;
    if (columnar) {
      allowcleaning=FALSE;
      if (block->line>1) {
        if (block->line+editrad>SHS(line)) {
          register int dif=block->line+editrad-SHS(line);
          register int count;
          register char *tempmem;
          if (ExpandText(BUF(shared), SHS(line)+dif)<OK)
            return(OUT_OF_MEM);
          tempmem=ReallocLine(BUF(shared), SHS(line), LEN(SHS(line))+1);
          if (!tempmem)
            return(OUT_OF_MEM);
          RAD(SHS(line))=tempmem;
          *(RAD(SHS(line))+LEN(SHS(line)))=BUF(using_fact)->newlinechar;
          LEN(SHS(line))++;
          SHS(size)++;
          for (count=1; count<dif-1; count++) {
            tempmem=Malloc(1);
            if (!tempmem)
              return(OUT_OF_MEM);
            tempmem[0]=BUF(using_fact)->newlinechar;
            RAD(SHS(line)+count)=tempmem;
            LEN(SHS(line)+count)=1;
            SHS(size)++;
          }
          RAD(SHS(line)+count)=NULL;
          LEN(SHS(line)+count)=0;
          FOLD(SHS(line)+count)=0;
          LFLAGS(SHS(line)+count)=0;
        }
      }
      cursorxstart=BUF(cursor_x)+BUF(screen_x);
      for (counter=1; counter<=block->line; counter++) {
        if (counter!=1) {
          editrad++;
          BUF(string_pos)=Col2Byte(Storage, cursorxstart, RAD(editrad), LEN(editrad));
          BUF(cursor_x)=Byte2Col(Storage, BUF(string_pos), RAD(editrad))-BUF(screen_x);
          BUF(cursor_y)++;
          BUF(curr_line)++;
        }
        if (editrad>SHS(line)) {
          SHS(line)++;
          count=1;
        } else
          count=0;
        {
          register int len=block->text[counter].length;
          if (len && (BUF(using_fact)->flags[block->text[counter].text[len-1]] & fact_NEWLINE))
            len--;
          if (len) {
            count+=OutputText(Storage, block->text[counter].text, len, TRUE, NULL);
            UndoAdd(Storage, (char *)count, undosort|undoONLYBACKSPACES, 0);
          }
        }
        if (counter==1) {
          undosort|=undoAPPENDBEFORE;
          if (undosort & undoNORMAL)
            undosort-=undoNORMAL;
        }
        if (blockkill)
          Dealloc(block->text[counter].text);
      }
      if (allowcleaning=tempcleaning)
        DeallocLine(Storage, 0);
    } else {
      if (ExpandText(BUF(shared), SHS(line)+block->line)<OK)
        return(OUT_OF_MEM);
      SHS(line)+=block->line-1;
      InsertLine(Storage, editrad, block->line-1);
      fold_no=FOLD(editrad);
      fold_flags=LFLAGS(editrad)&~FOLD_BEGIN;
      MoveUp4((int *)(SHS(text)+(editrad+block->line-1)), (int *)(SHS(text)+editrad),
             ((SHS(line)-editrad-block->line+2)*sizeof(TextStruct)/4));
      memset(SHS(text)+editrad+1, 0, sizeof(TextStruct)*(block->line-1));
      {
        register int temp;
        register char *tempmem;
        temp=(LEN(editrad)-BUF(string_pos));
        tempmem=(char *)Malloc(temp);
        if (!tempmem)
          return(OUT_OF_MEM);
        LEN(editrad+block->line-1)=temp;
        RAD(editrad+block->line-1)=tempmem;
        FOLD(editrad+block->line-1)=fold_no;
        LFLAGS(editrad+block->line-1)=fold_flags&~FOLD_BEGIN;
        SHS(size)+=temp;
        memcpy(RAD(editrad+block->line-1), RAD(editrad)+BUF(string_pos), LEN(editrad+block->line-1));
      }
      count+=(len=(block->text[counter].length));
      slask=BUF(string_pos)+len;
      stringtemp=(char *)Malloc(slask+1);
      if (!stringtemp)
        return(OUT_OF_MEM);
      if (BUF(string_pos))
        memcpy(stringtemp, RAD(editrad), BUF(string_pos));
      memcpy(stringtemp+BUF(string_pos), block->text[1].text, len);
      if (blockkill)
        Dealloc(block->text[1].text);
      DeallocLine(Storage, editrad);
      SHS(size)+=slask-LEN(editrad);
      LEN(editrad)=slask;
      RAD(editrad++)=stringtemp;

      for (counter=2; counter<block->line; counter++) {
        count+=(len=(block->text[counter].length));
        FOLD(editrad)=fold_no;
        LFLAGS(editrad)=fold_flags;
        if (!blockkill) {
          RAD(editrad)=(char *)Malloc(len);
          if (!RAD(editrad))
            return(OUT_OF_MEM);
          LEN(editrad)=len;
          memcpy(RAD(editrad++), block->text[counter].text, len);
        } else {
          RAD(editrad)=block->text[counter].text;
          LEN(editrad++)=len;
        }
        SHS(size)+=len;
      }

/*
      if (BUF(block_exists)) {
        if (BUF(block_anc_x)==BUF(block_end_x) && BUF(block_anc_y)==BUF(block_end_y)) {
          BUF(block_anc_y)+=block->line-1;
          BUF(block_end_y)+=block->line-1;
        }
      }
*/
      BUF(cursor_x)=1;
      BUF(string_pos)=0;
      BUF(screen_x)=0;
      BUF(cursor_y)+=block->line-1;
      BUF(curr_line)+=block->line-1;
      count+=OutputText(Storage, block->text[counter].text, block->text[counter].length, TRUE, NULL);
      if (blockkill)
        Dealloc(block->text[counter].text);
      UndoAdd(Storage, (char *)count, undosort | undoONLYBACKSPACES, 0);
      UpDtNeeded|=UD_REDRAW_BUFFS;
      SHS(update)|=UD_REDRAW_BUFFS;
    }
  }
  SetScreen(Storage, BUF(string_pos), BUF(curr_line));
  BUF(wanted_x)=BUF(cursor_x)+BUF(screen_x);
  MoveSlider(Storage);
  if (blockkill) {
    if (block->line) {
      Dealloc((char *)block->text);
    }
    Dealloc((char *)block);
  }
  return(ret);
}

/*************************************************************
*
*  int Block2String(BufStruct *, BlockStruct *block, char **, int *) 
*
*  Copy the marked/given block to a string, returned in (char **) and len
*  in (int *).
*  Return a ret value.
************/
int __regargs Block2String(BufStruct *Storage, BlockStruct *block, char **retstring, int *retlen)
{
  int ret=OK;
  int len=0, count;
  char *string;
  int usingdefblock;

  if (block)
    usingdefblock=TRUE;
  else if (BUF(block_exists)==be_NONE) {
    usingdefblock=TRUE;
    block=BlockBuffer;
    if (block->line==0)
      return(NO_BLOCK_MARKED);
  } else {
    usingdefblock=FALSE;
    block=(BlockStruct *)Malloc(sizeof(BlockStruct));
    if (!block)
      return(OUT_OF_MEM);
    memset(block, 0, sizeof(BlockStruct));
    ret=BlockCopy(Storage, block, BUF(blocktype)==bl_COLUMNAR?0:bl_NOALLOC, 0);
  }
  if (ret==OK) {
    {
      register int count;
      for (count=1; count<=block->line; count++)
        len+=block->text[count].length;
    }
    if (!block->line) {
      *retstring=NULL;
      *retlen=0;
      return(EMPTY_BLOCKBUFFER);
    }
    string=(char *)Malloc(len+1);
    len=0;
    if (string) {
      for (count=1; count<=block->line; count++) {
        memcpy(string+len, block->text[count].text, block->text[count].length);
        len+=block->text[count].length;
      }
      *(string+len)=0;
    } else
      ret=OUT_OF_MEM;
    *retstring=string;
    *retlen=len;
  }
  if (!usingdefblock) {
    if (BUF(blocktype)==bl_COLUMNAR)
      ClearBlock(block);
    else
      Dealloc((char *)block->text);
    Dealloc((char *)block);
  }
  return(ret);
}

/*************************************************************
*
*  int String2Block(BlockStruct *block, char *, int, BOOL ) 
*
*  Copy the string to give block else default block
*  Return a ret value.
************/
int __regargs String2Block(BlockStruct *block, char *string, int stringlen, char append)
{
  int ret=OK;
  int count;
  TextStruct *textarray;
  int lines;
  char *tempstring;
  char *temp1;

  if (!block)
    block=BlockBuffer;

  if (Default.rwed_sensitive && (block->fileprotection&S_IWRITE))
    return(WRITE_PROTECTED);

  if (!append)
    ClearBlock(block);

  Updated_Face(block, 1);

  ret=ExtractLines(string, stringlen, &textarray, &lines, NULL);
  if (ret>=OK && textarray[1].length) {
    block->size+=stringlen;
    if (block->line+lines>=block->taket) {
      temp1=Remalloc((char *)block->text, sizeof(TextStruct)*(block->taket+lines+4));
      if (temp1) {
        block->text=(TextStruct *)temp1;
        block->taket+=lines+5;
      } else {
        Dealloc(temp1);
        return(OUT_OF_MEM);
      }
    }
    count=1;
    if (block->line &&
        block->text[block->line].length &&
         (!(DefaultFact->flags[*(block->text[block->line].text+block->text[block->line].length-1)] & fact_NEWLINE))) {
      temp1=ReallocLine(block, block->line,
                        block->text[block->line].length+textarray[1].length);
      if (!temp1)
        ret=OUT_OF_MEM;
      else {
        block->text[block->line].text=temp1;
        memcpy(block->text[block->line].text+block->text[block->line].length,
               textarray[1].text, textarray[1].length);
        block->text[block->line].length+=textarray[1].length;
        if (DefaultFact->flags[textarray[1].text[textarray[1].length-1]] & fact_NEWLINE)
          block->line++;
        count++;
      }
    }
    if (ret>=OK) {
      for (; count<=lines; count++) {
        tempstring=NULL;
        if (textarray[count].length) {
          tempstring=Malloc(textarray[count].length);
          if (!tempstring) {
//          ClearBlock(block);
            ret=OUT_OF_MEM;
            if (block->line)
              block->line--;
            break;
          }
          memcpy(tempstring, textarray[count].text, textarray[count].length);
        }
        if (!block->line)
          block->line++;
        block->text[block->line].text=tempstring;
        block->text[block->line].length=textarray[count].length;
        block->text[block->line].fold_no=0;
        block->text[block->line].flags=0;
        if (count!=lines)
          block->line++;

      }
    }
    Dealloc(textarray);
    if (!block->line || (block->text[block->line].length &&
        (DefaultFact->flags[*(block->text[block->line].text+block->text[block->line].length-1)] & fact_NEWLINE))) {
      block->line++;
      block->text[block->line].text=GlobalEmptyString;
      block->text[block->line].length=0;
      block->text[block->line].fold_no=0;
      block->text[block->line].flags=0;
    }
    if (block->Entry) {
      Command(NULL, DO_NOTHING|NO_HOOK, 0, NULL, 0);
      FreeUndoLines(block, block->Undotop+1);
      UpdateDupScreen(block->Entry);
      MoveSlider(block->Entry);
    }
  }
  return(ret);
}

/*************************************************************
*
*  int BlockSort(BufStruct *, BlockStruct *block, int pos, int flags)
*
*  Sort the marked/given block.  If nothing marked/given: the BlockBuffer.
*  Start sorting at field (int pos).
*  Return a ret value.
************/
int __regargs BlockSort(BufStruct *Storage, BlockStruct *block, int pos, int flags)
{
  char **string1;
  int *lens;
  char **string2;
  int *lens2;
  char *tempstring;
  int count, count2, len, temppos;
  int ret=OK;
  int usingdefblock=FALSE;
  int tempcleaning=allowcleaning;
  CursorPosStruct cpos;
  BOOL cposused=FALSE;
  char columnar=0;
  BOOL newlineexpand=FALSE;
  
  if (block) {
    usingdefblock=TRUE;
  } else if (BUF(block_exists)==be_NONE) {
    usingdefblock=TRUE;
    block=BlockBuffer;
    if (block->line==0)
      return(NO_BLOCK_MARKED);
  } else if (Default.rwed_sensitive && (SHS(fileprotection)&S_IWRITE))
    return(WRITE_PROTECTED);

  if (pos<0) {
    char tempbuffer[12];
    ButtonArrayStruct butarray[2];
    ButtonStruct Buttons;
    int retval;
  
    memset(butarray, 0, sizeof(butarray));
  
    butarray[0].name= RetString(STR_SORT_FORWARD);
    butarray[0].flags=2-(flags&2);
    butarray[1].name= RetString(STR_SORT_CASE_SENSITIVE);
    butarray[1].flags=1-(flags&1);
    tempbuffer[0]='1';
    tempbuffer[1]=0;

    InitButtonStruct(&Buttons);
    Buttons.array=butarray;
    Buttons.antal=2;

    retval=Reqlist(Storage,
                   REQTAG_STRING1, tempbuffer, 12,
                   REQTAG_BUTTON, &Buttons,
                   REQTAG_TITLE, RetString(STR_SORT_ACCORDING_FIELD),
                   REQTAG_END);

    if (retval<rl_NOMATCH || !tempbuffer[0])
      ret=FUNCTION_CANCEL;
    else if (retval==rl_ERROR)
      ret=OUT_OF_MEM;
    else {
      flags=(butarray[0].flags?0:SORT_BACK)|(butarray[1].flags?0:SORT_INCASE);
      pos=fplStrtol(tempbuffer, 10, &tempstring);	/* tempstring is a dummy */
    }

  }
  if (ret>=OK) {
    FreezeEditor(2);
    if (!usingdefblock) {
      if (BUF(block_exists)==be_EXIST) {
        SaveCursorPos(Storage, &cpos);
        cposused=TRUE;
      }
      Visible=VISIBLE_OFF;
      allowcleaning=FALSE;
      block=(BlockStruct *)Malloc(sizeof(BlockStruct));
      ret=OUT_OF_MEM;	// Ställs om ett par rader ner ändå...
      if (block) {
        memset(block, 0, sizeof(BlockStruct));
        if (BUF(blocktype)==bl_COLUMNAR)
          columnar=bp_COLUMN;
        ret=BlockCopy(Storage, block, bl_UNDOADD | bl_CUT, NULL);
        if ((columnar || cposused) && ret>=OK)
          SetScreen(Storage, Col2Byte(Storage, BUF(block_begin_x), RAD(BUF(block_begin_y)), LEN(BUF(block_begin_y))+(BUF(block_begin_y)==SHS(line))), BUF(block_begin_y));
      }
    }
    if (ret>=OK) {
      ret=OUT_OF_MEM;
      string1=(char **)Malloc(sizeof(char *)*(block->line+1));
      if (string1) {
        lens=(int *)Malloc(sizeof(int)*(block->line+1));
        if (lens) {
          string2=(char **)Malloc(sizeof(char *)*(block->line+1));
          if (string2) {
            lens2=(int *)Malloc(sizeof(int)*(block->line+1));
            if (lens2) {
              char **stringarray[4];
              int antal=0;
              ret=OK;
              stringarray[0]=string1;
              stringarray[1]=(char **)lens;
              stringarray[2]=string2;
              stringarray[3]=(char **)lens2;
          
              for (count=1; count<=block->line; count++) {
                len=block->text[count].length;
                if (!len)
                  break;	// quit loop
                antal++;
                tempstring=block->text[count].text;
                count2=0;
                temppos=pos;
                if (temppos-->0 && count2<len) {
                  while((BUF(using_fact)->xflags[*(tempstring+count2)] & factx_CLASS_SPACE) &&
                          count2<len)
                    count2++;
                }
                while (temppos-->0 && count2<len) {
                  while(!(BUF(using_fact)->xflags[*(tempstring+count2)] & factx_CLASS_SPACE) &&
                          count2<len)
                    count2++;
                  while((BUF(using_fact)->xflags[*(tempstring+count2)] & factx_CLASS_SPACE) &&
                          count2<len)
                    count2++;
                }
                if (count2==len) {
                  string1[count-1]=GlobalEmptyString;
                  lens[count-1]=0;
                } else {
                  string1[count-1]=(block->text[count].text+count2);
                  lens[count-1]=block->text[count].length-count2;
                }
                lens2[count-1]=block->text[count].length;
                string2[count-1]=block->text[count].text;
              }
              if (antal) {
                if (block->text[antal].length && !(BUF(using_fact)->flags[*(block->text[antal].text+block->text[antal].length-1)] & fact_NEWLINE)) {
                  newlineexpand=TRUE;
//                  if (!block->Entry)
//                    block->text[antal].text=Remalloc(block->text[antal].text, block->text[antal].length+1);
//                  else
                    block->text[antal].text=ReallocLine(block, antal, block->text[antal].length+1);
                  if (!block->text[antal].text)
                    return(OUT_OF_MEM);
                  block->text[antal].text[block->text[antal].length]=BUF(using_fact)->newlinechar;
                  block->text[antal].length++;
                }
              }
                
              ShellSort(stringarray, 2, 2, antal, flags);
              Updated_Face(block, 1);
              for (count=1; count<=antal; count++) {
                block->text[count].text=string2[count-1];
                block->text[count].length=lens2[count-1];
              }
              if (newlineexpand) {
                if (block->text[antal].text[block->text[antal].length-1]==BUF(using_fact)->newlinechar) {
                  block->text[antal].length--;
                }
              }
              Dealloc((char *)lens2);
            }
            Dealloc((char *)string2);
          }
          Dealloc((char *)lens);
        }
        Dealloc((char *)string1);
      }
    }
    if (!usingdefblock) {
      ret=BlockPaste(Storage, undoAPPENDBEFORE, block, columnar /*bp_KILL*/);	/* Paste&Kill */
      ClearBlock(block);
      Dealloc(block);
      if (cposused) {
        RestoreCursorPos(Storage, &cpos, TRUE);
        ClearVisible();
      }
    } else {
      if (block->Entry) {
        UpdateDupScreen(block->Entry);
        FreeUndoLines(block, block->Undotop+1);
      }
    }
    if (allowcleaning=tempcleaning)
      DeallocLine(Storage, 0);
    ActivateEditor();
  }
  return(ret);
}

/*************************************************************
*
*  int BlockCase(BufStruct *, BlockStruct *block, int flag)
*
*  Change the case of the marked/given block.
*  Defines for flag in Block.h
*  Return a ret value.
************/
int __regargs BlockCase(BufStruct *Storage, BlockStruct *block, int flag)
{
  char chr;
  int count, count2;
  int ret=OK;
  BOOL usingdefblock=FALSE;
  int tempcleaning=allowcleaning;
  CursorPosStruct cpos;
  BOOL cposused=FALSE;
  char columnar=0;
//  int tempvisible=Visible;

  if (block)
    usingdefblock=TRUE;
  else if (BUF(block_exists)==be_NONE) {
    usingdefblock=TRUE;
    block=BlockBuffer;
    if (block->line==0)
      return(NO_BLOCK_MARKED);
  } else if (Default.rwed_sensitive && (SHS(fileprotection)&S_IWRITE))
    return(WRITE_PROTECTED);


  if (!usingdefblock) {
    if (BUF(blocktype)==bl_COLUMNAR)
      columnar=bp_COLUMN;
    if (BUF(block_exists)==be_EXIST) {
      SaveCursorPos(Storage, &cpos);
      cposused=TRUE;
    }
    Visible=VISIBLE_OFF;
    allowcleaning=FALSE;
    block=(BlockStruct *)Malloc(sizeof(BlockStruct));
    ret=OUT_OF_MEM;	// Ställs om ett par rader ner ändå...
    if (block) {
      memset(block, 0, sizeof(BlockStruct));
      ret=BlockCopy(Storage, block, bl_UNDOADD | bl_CUT, NULL);
      if ((columnar || cposused) && ret>=OK)
        SetScreen(Storage, Col2Byte(Storage, BUF(block_begin_x), RAD(BUF(block_begin_y)), LEN(BUF(block_begin_y))+(BUF(block_begin_y)==SHS(line))), BUF(block_begin_y));
    }
  }
  if (ret>=OK) {

    Updated_Face(block, 1);

    for (count=1; count<=block->line; count++) {
      for (count2=0; count2<block->text[count].length; count2++) {
        chr=block->text[count].text[count2];
        if (BUF(using_fact)->xflags[chr] & (factx_UPPERCASE | factx_LOWERCASE)) {
          if (BUF(using_fact)->xflags[chr] & factx_UPPERCASE) {
            if (flag!=bc_UPPER_CASE)
             block->text[count].text[count2]=BUF(using_fact)->cases[chr];
          } else {
            if (flag!=bc_LOWER_CASE)
              block->text[count].text[count2]=BUF(using_fact)->cases[chr];
          }
        }
      }
    }
    if (!usingdefblock) {
      ret=BlockPaste(Storage, undoAPPENDBEFORE, block, columnar/*|bp_KILL*/);	/* Paste&Kill */
      ClearBlock(block);
      Dealloc(block);
      if (cposused) {
        RestoreCursorPos(Storage, &cpos, TRUE);
        ClearVisible();
      }
    } else {
      if (block->Entry) {
        FreeUndoLines(block, block->Undotop+1);
        UpdateDupScreen(block->Entry);
      }
    }
  }
  if (allowcleaning=tempcleaning)
    DeallocLine(Storage, 0);
//  UpdateDupScreen(Storage);
  return(ret);
}

void __regargs ClearBlock(BlockStruct *block)
{
  if (block->Entry)
    Clear(block->Entry, FALSE);
  else {
    int count;
    if (block->line) {
      for (count=1; count<=block->line; count++)
        Dealloc(block->text[count].text);
      block->line=0;
    }
    Dealloc((char *)block->text);
    block->text=NULL;
    block->taket=0;
  }
}

/***************************************************************
 *
 *  AllocBlock
 *
 *  Alloc a block and put it in the linked list.
 *  Return the block pointer, NULL==error.
 ******/
BlockStruct __regargs *AllocBlock(char *name)
{
  BlockStruct *block;
  BlockStruct **blockcount=&Default.FirstBlock;
  BufStruct *Storage2;

  if (!name && *blockcount)
    return(NULL);

  Storage2=MakeNewBuf(NULL);
  if (Storage2) {
    block=Storage2->shared;
    block->type=type_BLOCK|type_HIDDEN;
    if (name)
      Split(block, name);
  }
  return(block);
}

/***************************************************************
 *
 *  FreeBlock
 *
 *  If no block is specified, then the ID will be used.
 *  Return ret-value.
 ******/
int __regargs FreeBlock(BlockStruct *block, char *name)
{
  BlockStruct **blockcount=&Default.FirstBlock;

  if (block)
    name=block->filnamn;

  while (*blockcount && Stricmp((*blockcount)->filnamn, name))
    blockcount=&(*blockcount)->Next;

  if (!(*blockcount))
    return(NO_BLOCK_FOUND);

  block=*blockcount;
  *blockcount=block->Next;
  if (Stricmp(BlockBuffer->filnamn, name))
    BlockBuffer=Default.FirstBlock;
  ClearBlock(block);
  Dealloc(block);
  return(OK);
}


BlockStruct __regargs *FindBlock(BufStruct *name)
{
  BlockStruct *block=NULL;
  BufStruct *buf;

  buf=CheckBufID(name);
  if (buf)
    block=buf->shared;
  return(block);
}


/*************************************************************
 *
 *  BlockMove(BufStruct *, int move, int undoadd)
 *
 *  Move a block int steps rigth(+) or left(-).
 *  Return a retvalue.
 * 
 *  Obs:  Will only consider a block from the first line to the last -1.
 ***********/
int __regargs BlockMove(BufStruct *Storage, int move, int undoadd)
{
  int ytemp=BUF(cursor_y), toptemp=BUF(curr_topline), xtemp=BUF(cursor_x);
  int linetemp=BUF(curr_line);
  int postemp=BUF(string_pos), screentemp=BUF(screen_x);
  int rad=BUF(block_begin_y);
  int rader;
  char *input=NULL;
  char *retstring;
  int retlength;
  int undosort=undoNORMAL;
  int ret=OK;

  if (BUF(block_exists)==be_NONE)
    return(NO_BLOCK_MARKED);

  rader=BUF(block_end_y)-rad;

  if (BUF(blocktype)==bl_COLUMNAR)
    return(WRONG_BLOCKTYPE);
  if (BUF(blocktype)==bl_NORMAL) {
    if (BUF(block_begin_x)>1) {
      rad++;
      rader--;
    }
    if (rader<=0)
      return(NO_BLOCK_MARKED);
  } else
    rader++;

  if (move) {
    Status(Storage, RetString(STR_MOVING_BLOCK), 0x81);
    Visible=VISIBLE_OFF;
    allowcleaning=FALSE;
    if (move>0) {
      input=Malloc(move);
      if (!input)
        return(OUT_OF_MEM);
      memset(input, ' ', move);
    }
    BUF(curr_topline)=1;
    BUF(screen_x)=0;
    while (rader) {
      BUF(cursor_y)=rad;
      BUF(curr_line)=rad;
      if (move>0) {
        BUF(cursor_x)=1;
        BUF(string_pos)=0;
        retlength=OutputText(Storage, input, move, TRUE, NULL);
        if (retlength && undoadd) {
          UndoAdd(Storage, (char *)retlength, undoONLYBACKSPACES|undosort, 0);
          undosort=undoAPPENDBEFORE;
        }
      } else {
        if (LEN(rad)<=-move)
          BUF(string_pos)=LEN(rad)-1;
        else
          BUF(string_pos)=-move;
        if (BUF(string_pos)>0) {
          BUF(cursor_x)=Byte2Col(Storage, BUF(string_pos), RAD(rad));
          ret=BackspaceUntil(Storage, 0, rad, &retstring, &retlength);
          if (ret>=OK && undoadd) {
            UndoAdd(Storage, retstring, undoONLYTEXT|undosort, retlength);
            undosort=undoAPPENDBEFORE;
          }
          Dealloc(retstring);
        }
      }
      rader--;
      rad++;
    }
    Dealloc(input);
    allowcleaning=TRUE;
    DeallocLine(Storage, 0);
    BUF(cursor_y)=ytemp;
    BUF(curr_line)=linetemp;
    BUF(cursor_x)=xtemp;
    BUF(screen_x)=screentemp;
    BUF(string_pos)=postemp;
    BUF(curr_topline)=toptemp;
  }
  return(ret);
}

int __regargs BlockMovement(BufStruct *Storage)
{
  int rad=BUF(block_begin_y);
  int stop=FALSE;
  int move=0;
  int loop;
  int key;
  int antal;
  char *radbuffer;
  int maxsize=-1;
  int counter;
  char *string=RetString(STR_MOVE_BLOCK_TEXT);
  int lastline, linecount;
  int beginrad, endrad;
  TextStruct temptext;

  if (!BUF(window))
    return(OK);

  if (BUF(block_exists)==be_NONE)
    return(NO_BLOCK_MARKED);

  if (BUF(blocktype)==bl_COLUMNAR)
    return(WRONG_BLOCKTYPE);

  lastline=FoldMoveLine(Storage, BUF(curr_line), BUF(screen_lines)-1);


  if (BUF(blocktype)==bl_NORMAL) {
    if (BUF(block_begin_x)>1)
      rad++;
    if (BUF(block_end_y)==rad)
      return(NO_BLOCK_MARKED);
  }

  if (rad<BUF(curr_topline))
    rad=BUF(curr_topline);

  beginrad=FoldFindLine(Storage, rad);
  endrad=FoldFindLine(Storage, BUF(block_end_y))+(BUF(blocktype)!=bl_NORMAL?1:0);
  if (endrad<0)
    endrad=BUF(screen_lines)+1;

  {
    register int count;
    for (count=BUF(block_begin_y); count<=BUF(block_end_y); count++) {
      if (LEN(count)>maxsize)
        maxsize=LEN(count);
    }
  }
  if (!(radbuffer=Malloc(maxsize+BUF(screen_col))))
    return(OUT_OF_MEM);

  CursorXY(Storage, -1, -1);
  memset(radbuffer, ' ', BUF(screen_col));

  Status(Storage, string, 0x81);
  BUF(namedisplayed)=FALSE;

  while (!stop) {
    loop=TRUE;
    while (loop) {
      if (key=GetKey(Storage, gkWAIT)) {
        if (buffer[0]==(unsigned char)'\x9B') {
          if (buffer[1]=='D' && move>-BUF(screen_col)) {
            move--;
            loop=FALSE;
          } else if (buffer[1]=='C' && move<BUF(screen_col)) {
            move++;
            loop=FALSE;
          } else if (buffer[1]=='Z' && move>-BUF(screen_col)) {
            move=((move-1-(move<=0?BUF(tabsize):0))/BUF(tabsize))*BUF(tabsize);
            loop=FALSE;
          }
        } else if (key<0 || buffer[0]=='q' || buffer[0]=='Q' || buffer[0]==0x1b) {
          move=0;
          stop=TRUE;
          loop=FALSE;
        } else if (BUF(using_fact)->flags[buffer[0]]&fact_TAB) {
          register int oldmove=move;
          move=((move+1+(move<=0?-BUF(tabsize):0))/BUF(tabsize)+1)*BUF(tabsize);
          if (move>BUF(screen_col))
            move=oldmove;
          loop=FALSE;
        } else if (buffer[0]=='\r' || buffer[0]=='y' || buffer[0]=='Y') {
          stop=TRUE;
          loop=FALSE;
        }
      }
    }
    if (!stop) {
      sprintf(buffer, "%s (%ld)", string, move);
      Status(Storage, buffer, 0x81);
      if (beginrad>0) {
        CursorXY(Storage, -1, -1);
        counter=0;
        linecount=beginrad;
        while (linecount<endrad) {
          BUF(window)->UpdtVars.UsingFact=BUF(using_fact);
          if (!(LFLAGS(counter+rad)&FOLD_HIDDEN)) {
            temptext=TEXT(rad+counter);
            if (move>0) {
              memcpy(radbuffer+BUF(screen_col), RAD(rad+counter), LEN(rad+counter));
              temptext.text=radbuffer+BUF(screen_col)-move;
            } else {
              temptext.text=RAD(rad+counter)-move;
            }
            temptext.length=LEN(rad+counter)+move;
            antal=UpdtOneLineC(Storage, &temptext, &ScreenBuffer, linecount, counter+rad);
            SystemPrint(Storage, &ScreenBuffer, antal, linecount);
            linecount++;
          }
          counter++;
        }
        CursorXY(Storage, BUF(cursor_x), BUF(cursor_y));
        if (BUF(block_exists)) {
          SetBlockMark(Storage, FALSE);
        }
      }
    }
  }
  if (move) {
    BlockMove(Storage, move, TRUE);
    if (BUF(block_exists)==be_MARKING)
      BUF(block_exists)=be_NONE;
  }
  UpdateScreen(Storage);
  Dealloc(radbuffer);
  return(OK);
}


/***********************************************************
 *
 *  PlaceBlockMark(...)
 *
 *  Checkar om positionerna i blockmarkeringen stämmer.
 *  Inget return.
 *********/
void __regargs PlaceBlockMark(BufStruct *Storage, BlockCutStruct *blockcut, int x1, int y1, int x2, int y2)
{
  register int temp;

  if (!blockcut)
    blockcut=(BlockCutStruct *)&BUF(block_begin_x);

  if (y1>SHS(line))
    y1=SHS(line);
  temp=Byte2Col(Storage, LEN(y1), RAD(y1));
  if (x1>temp)
    x1=temp;
  blockcut->block_anc_x=x1;
  blockcut->block_anc_y=y1;

  if (y2>SHS(line))
    y2=SHS(line);
  if (y1!=y2)  // kalkulera om temp om det inte är samma rad.
    temp=Byte2Col(Storage, LEN(y2), RAD(y2));
  if (x2>temp)
    x2=temp;
  blockcut->block_begin_x=blockcut->block_end_x=x2;
  blockcut->block_begin_y=blockcut->block_end_y=y2;
  if (y2>y1 || (y2==y1 && x2>x1)) {
    blockcut->block_begin_x=x1;
    blockcut->block_begin_y=y1;
  } else {
    blockcut->block_end_x=x1;
    blockcut->block_end_y=y1;
  }
  blockcut->block_last_x=blockcut->block_last_y=-1;
}


