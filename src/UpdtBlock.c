/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************************
 *
 *  UpdtBlock.c
 *
 *  Different functions to update your block mark
 *
 ******/


#include <exec/types.h>
#include <graphics/gfxmacros.h>
#undef GetOutlinePen
#include <proto/graphics.h>
#include <stdio.h>
#include <string.h>
#include "Buf.h"
#include "Block.h"
#include "Cursor.h"
#include "Edit.h"
#include "Fold.h"
#include "UpdtBlock.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"
#include "WindowOutput.h"

extern struct BitMap EmptyBitMap;
extern struct TextFont *SystemFont;
extern struct TextFont *RequestFont;
extern DefaultStruct Default;
extern int Visible;
extern int UpDtNeeded;

/************************************************
 *
 *  UpdateBlock(BufStruct *)
 *
 *  Updates your block, relative to the last updating.
 *  Input wanted BufStruct.  No return.
 *********/
void __regargs UpdateBlock(BufStruct *Storage)
{
  int ypos=BUF(curr_line);
  int xpos=BUF(cursor_x)+BUF(screen_x);

  if (BUF(block_exists)) {
    if (ypos!=BUF(block_last_y) || xpos!=(BUF(block_last_x))) {
      CursorXY(Storage, -1, -1);
      if (BUF(blocktype)!=bl_NORMAL) {
        if (BUF(blocktype)==bl_COLUMNAR || ypos!=BUF(block_last_y)) {
          PasteBlockMark(Storage, BUF(block_begin_x), BUF(block_begin_y),
                         BUF(block_end_x), BUF(block_end_y), 0);
          if (BUF(block_exists)==be_MARKING) {
            if ((ypos > BUF(block_anc_y)) ||
                ( (ypos == BUF(block_anc_y)) && (xpos>BUF(block_anc_x)) ) ) {
              BUF(block_begin_x)=BUF(block_anc_x);
              BUF(block_begin_y)=BUF(block_anc_y);
              BUF(block_end_x)=xpos;
              BUF(block_end_y)=ypos;
            } else {
              BUF(block_begin_x)=xpos;
              BUF(block_begin_y)=ypos;
              BUF(block_end_x)=BUF(block_anc_x);
              BUF(block_end_y)=BUF(block_anc_y);
            }
          }
          if (BUF(blocktype)==bl_LINE) {
            BUF(block_begin_x)=0;
            BUF(block_end_x)=Byte2Col(Storage, LEN(BUF(block_end_y)), RAD(BUF(block_end_y)));
          }
          PasteBlockMark(Storage, BUF(block_begin_x), BUF(block_begin_y),
                         BUF(block_end_x), BUF(block_end_y), -1);
        }
      } else if (BUF(block_exists)==be_EXIST) {
        PasteBlockMark(Storage, BUF(block_begin_x), BUF(block_begin_y),
                       BUF(block_end_x), BUF(block_end_y), -1);
      } else {
        if ((ypos > BUF(block_anc_y)) ||
            ( (ypos == BUF(block_anc_y)) && (xpos>BUF(block_anc_x)) ) ) {
          if (BUF(block_anc_y)==BUF(block_begin_y) &&
              BUF(block_anc_x)==BUF(block_begin_x)) {
            if (ypos<BUF(block_end_y) || (ypos<=BUF(block_end_y) &&
                xpos<=BUF(block_end_x))) {
              PasteBlockMark(Storage,
                            xpos, ypos,
                            BUF(block_end_x), BUF(block_end_y), 0);
            } else {
              PasteBlockMark(Storage,
                            BUF(block_end_x), BUF(block_end_y),
                            xpos, ypos, -1);
            }
          } else {
            PasteBlockMark(Storage,
                          BUF(block_begin_x), BUF(block_begin_y),
                          BUF(block_anc_x), BUF(block_anc_y), 0);
            PasteBlockMark(Storage,
                          BUF(block_anc_x), BUF(block_anc_y),
                          xpos, ypos, -1);
          }
          BUF(block_begin_x)=BUF(block_anc_x);
          BUF(block_begin_y)=BUF(block_anc_y);
          BUF(block_end_x)=xpos;
          BUF(block_end_y)=ypos;
       } else {
    
         if (BUF(block_anc_y)==BUF(block_end_y) &&
             BUF(block_anc_x)==BUF(block_end_x)) {
            if (ypos<BUF(block_begin_y) || (ypos<=BUF(block_begin_y) &&
                xpos<=BUF(block_begin_x))) {
              PasteBlockMark(Storage,
                            xpos, ypos,
                            BUF(block_begin_x), BUF(block_begin_y), -1);
            } else {
              PasteBlockMark(Storage,
                            BUF(block_begin_x), BUF(block_begin_y),
                            xpos, ypos, 0);
            }
          } else {
            PasteBlockMark(Storage,
                          BUF(block_anc_x), BUF(block_anc_y),
                          BUF(block_end_x), BUF(block_end_y), 0);
            PasteBlockMark(Storage,
                          xpos, ypos,
                          BUF(block_anc_x), BUF(block_anc_y), -1);
          }
          BUF(block_begin_x)=xpos;
          BUF(block_begin_y)=ypos;
          BUF(block_end_x)=BUF(block_anc_x);
          BUF(block_end_y)=BUF(block_anc_y);
        }
      }
      if (Visible==VISIBLE_ON) {
        BUF(block_last_x)=xpos; 
        BUF(block_last_y)=ypos; 
        BUF(update)&=~UD_REDRAW_BLOCK;
      }
    }
  }
}

/******************************************************
*
*  SetBlockMark(BufStruct *)
*
*  Simply update your block mark.  You have to clear
*  the screen yourself.
*  If 'erase' is TRUE, the block will be removed.
*  Input wanted BufStruct pointer.
********/
void __regargs SetBlockMark(BufStruct *Storage, BOOL erase)
{
  int ypos=BUF(curr_line);
  int xpos=BUF(cursor_x)+BUF(screen_x);

  if (BUF(blocktype)==bl_COLUMNAR && !erase) {
    PasteBlockMark(Storage,
                   BUF(block_anc_x), BUF(block_anc_y),
                   BUF(block_last_x), BUF(block_last_y), 0);
  }
  if (BUF(block_exists)==be_MARKING) {
    if ((ypos > BUF(block_anc_y)) ||
     ( (ypos == BUF(block_anc_y)) && (xpos>BUF(block_anc_x)) ) ) {

      BUF(block_begin_x)=BUF(block_anc_x);
      BUF(block_begin_y)=BUF(block_anc_y);
      BUF(block_end_x)=xpos;
      BUF(block_end_y)=ypos;
    } else {
      BUF(block_begin_x)=xpos;
      BUF(block_begin_y)=ypos;
      BUF(block_end_x)=BUF(block_anc_x);
      BUF(block_end_y)=BUF(block_anc_y);
    }
    if (Visible==VISIBLE_ON) {
      BUF(block_last_x)=xpos; 
      BUF(block_last_y)=ypos; 
    }
    if (BUF(blocktype)==bl_LINE) {
      BUF(block_begin_x)=0;
      BUF(block_end_x)=Byte2Col(Storage, LEN(BUF(block_end_y)), RAD(BUF(block_end_y)));
    }
  }
  if (BUF(block_exists)) {
    PasteBlockMark(Storage,
                   BUF(block_begin_x), BUF(block_begin_y),
                   BUF(block_end_x), BUF(block_end_y), erase?0:-1);
  }
  if (erase)
    BUF(visible_bitplanes)=BUF(text_using_bitplanes);

}
/******************************************************
*
*  PasteBlockMark(BufStruct *, ...)
*
*  Paste a block in Bpl2 according to your command.
*
********/
void __regargs PasteBlockMark(BufStruct *Storage, int xstart, int ystart,
                    int xstop, int ystop, int pattern)
{
  int xsize=SystemFont->tf_XSize;
  int ysize=SystemFont->tf_YSize;
  int xoffs=BUF(left_offset);
  int ycount;
  int xstorlek;
  int screen_line, screen_stop;
  char bitplanes=3;

  if (Visible==VISIBLE_ON && BUF(window) && BUF(window)->Visible==VISIBLE_ON) {
    if (ystart>ystop || (ystart==ystop && xstart>xstop)) {
      SWAP4(ystart, ystop);
      SWAP4(xstart, xstop);
    }
  
    if (ystart<BUF(curr_topline)) {
      if (ystop<BUF(curr_topline))
        return;
      ystart=BUF(curr_topline);
      if (BUF(blocktype)!=bl_COLUMNAR)
        xstart=1;
    }
    screen_line=FoldFindLine(Storage, ystart);
    if (screen_line<0)
      return;	// Out of range
    screen_stop=FoldFindLine(Storage, ystop);
    if (screen_stop==0)
      return;	// Out of range
    if (screen_stop<0)
      screen_stop=BUF(screen_lines)+1;

    SetWrMsk(BUF(window)->window_pointer->RPort, BUF(window->block_pen));
    SetDrMd(BUF(window)->window_pointer->RPort, COMPLEMENT);
    BUF(visible_bitplanes)|=BUF(window->block_pen);
  
    if (BUF(blocktype)==bl_LINE) {
      xstart=0;
      xstop=BUF(screen_col);
    } else {
      if ((xstop-=BUF(screen_x)+1)<0)
        xstop=0;
    }
    if ((xstart-=BUF(screen_x)+1)<0)
      xstart=0;
    if (xstop>BUF(screen_col))
      xstop=BUF(screen_col);

    ycount=screen_line+BUF(top_offset)-2;
    if (BUF(blocktype)==bl_COLUMNAR) {
      int ydif;

      ydif=screen_stop-screen_line+1;
      if (ydif>BUF(screen_lines)+BUF(top_offset)-1-ycount)
        ydif=(BUF(screen_lines)+BUF(top_offset)-1)-ycount;

      xstart+=BUF(l_c_len)+BUF(fold_len);
      xstop+=BUF(l_c_len)+BUF(fold_len);

      if (xstart>xstop)
        SWAP4(xstart, xstop);
      if (xstop>BUF(screen_col))
        xstop=BUF(screen_col);
      xstorlek=xstop-xstart;
      if (xstorlek>0 && ydif) {
/*
        if (pattern)
          SetAPen(BUF(window)->window_pointer->RPort, 2);
        else
          SetAPen(BUF(window)->window_pointer->RPort, 0);
*/
        {
          int count=0;
          for (count=0; count<ydif; count++) {
            if ((BUF(window)->UpdtVars.EndPos[count+ycount].blockstart<0 && pattern) ||
                (BUF(window)->UpdtVars.EndPos[count+ycount].blockstart>=0 && !pattern)) {
              RectFill(BUF(window)->window_pointer->RPort,
                       xoffs+xstart*xsize,
                       (count+ycount)*ysize+BUF(window)->text_start,
                       xoffs+xsize*(xstart+xstorlek)-1,
                       ysize*(count+ycount+1)+BUF(window)->text_start-1);
              BUF(window)->UpdtVars.EndPos[ycount+count].blockstart=pattern?xstart:-1;
              BUF(window)->UpdtVars.EndPos[ycount+count].blocksize=xstorlek;
              BUF(window)->UpdtVars.EndPos[ycount+count].bitplanes|=bitplanes;
            }
          }
        }
      }
    } else {
      xstart+=BUF(l_c_len)+BUF(fold_len);
      while (screen_line<=screen_stop && screen_line<=BUF(screen_lines)) { //ycount<=BUF(screen_lines)+BUF(top_offset)-1) {
        if (screen_line==screen_stop) {
          xstorlek=xstop+BUF(l_c_len)+BUF(fold_len)-xstart;
        } else
          xstorlek=BUF(window)->UpdtVars.EndPos[ycount].len-xstart;
        if (xstorlek>0) {
          BOOL draw=FALSE;
/*
          if (pattern)
            SetAPen(BUF(window)->window_pointer->RPort, 2);
          else
            SetAPen(BUF(window)->window_pointer->RPort, 0);
*/
          if (BUF(window)->UpdtVars.EndPos[ycount].blockstart>=0 && !pattern) {
            draw=TRUE;
            if (BUF(window)->UpdtVars.EndPos[ycount].blockstart==xstart)
              BUF(window)->UpdtVars.EndPos[ycount].blockstart+=xstorlek;
            BUF(window)->UpdtVars.EndPos[ycount].blocksize-=xstorlek;
            if (!BUF(window)->UpdtVars.EndPos[ycount].blocksize)
              BUF(window)->UpdtVars.EndPos[ycount].blockstart=-1;
          } else if (pattern) {
            draw=TRUE;
            if (BUF(window)->UpdtVars.EndPos[ycount].blockstart<0) {
              BUF(window)->UpdtVars.EndPos[ycount].blockstart=xstart;
              BUF(window)->UpdtVars.EndPos[ycount].blocksize=xstorlek;
            } else if (BUF(window)->UpdtVars.EndPos[ycount].blockstart>xstart) {
              if (xstorlek>BUF(window)->UpdtVars.EndPos[ycount].blockstart-xstart);
                xstorlek=BUF(window)->UpdtVars.EndPos[ycount].blockstart-xstart;
              BUF(window)->UpdtVars.EndPos[ycount].blockstart=xstart;
              BUF(window)->UpdtVars.EndPos[ycount].blocksize+=xstorlek;
            } else if (BUF(window)->UpdtVars.EndPos[ycount].blockstart<xstart) {
              BUF(window)->UpdtVars.EndPos[ycount].blocksize+=xstorlek;
            } else if (BUF(window)->UpdtVars.EndPos[ycount].blocksize<xstorlek) {
              xstart+=BUF(window)->UpdtVars.EndPos[ycount].blocksize;
              xstorlek-=BUF(window)->UpdtVars.EndPos[ycount].blocksize;
              BUF(window)->UpdtVars.EndPos[ycount].blocksize+=xstorlek;
            } else {
              draw=FALSE;
            }
          }
          if (xstart+xstorlek>BUF(window)->UpdtVars.EndPos[ycount].len)
            xstorlek=BUF(window)->UpdtVars.EndPos[ycount].len-xstart;
          if (draw && xstorlek>0) {
            RectFill(BUF(window)->window_pointer->RPort,
                      xoffs+xstart*xsize,
                      ycount*ysize+BUF(window)->text_start,
                      xoffs+(xstart+xstorlek)*xsize-1,
                      ycount*ysize+BUF(window)->text_start-1+ysize);
            BUF(window)->UpdtVars.EndPos[ycount].bitplanes|=bitplanes;
          }
        }
        ycount++;
        xstart=BUF(l_c_len)+BUF(fold_len);
        screen_line++;
      }
    }
  } else {
    UpDtNeeded|=UD_REDRAW_BLOCK;
    BUF(update)|=UD_REDRAW_BLOCK;
  }
}
