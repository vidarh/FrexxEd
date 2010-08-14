/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * WindowOutput.c
 *
 *********/

#include <graphics/gfxmacros.h>
#undef GetOutlinePen
#include <graphics/rastport.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "Edit.h"
#include "FACT.h"
#include "Fold.h"
#include "MultML.h"
#include "Slider.h"
#include "UpdtBlock.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"
#include "WindowOutput.h"

extern int Visible;
extern struct IntuitionBase *IntuitionBase;
extern char CursorOnOff;
extern WORD charbredd, charhojd, baseline;
extern struct TextFont *SystemFont;
extern struct TextFont *RequestFont;
extern struct screen_buffer ScreenBuffer;
extern int systemfont_leftmarg;
extern int systemfont_rightmarg;
extern BOOL OwnWindow;
extern DefaultStruct Default;
extern WORD sliderhighlite, statuscolor, statustextcolor;
extern int UpDtNeeded;
extern char cursorhidden;
extern unsigned char *statusbuffer;
extern struct MsgPort *WindowPort;


/************************************************************
 *
 * void __regargs SystemPrint(BufStruct *Storage, char *buf, int len, int line)
 *
 * Print your string relative to BUF(top_offset).
 * If line<0 you decide the pixel line to print at.
 *
 * No return.
 *********/
void __regargs SystemPrint(BufStruct *Storage, struct screen_buffer *buf,
                           int len, int line)
{
  struct RastPort *rp;
  register char control;
  register int type=0;
  int printlen, totallen=0;
  int x=BUF(left_offset), y;
  int eraseline=0;
  char bitplanes=1;
  int offset=0;
  char color_a, color_b;
  short color;
  char non_char_color_a, non_char_color_b;

  if (BUF(window) && BUF(window)->window_pointer) {
#ifdef USE_FASTSCROLL
    BOOL use_ztext=FALSE;
    if (match_for_fastscroll && Default.fast_scroll &&
        FastGraphicsBase && !(BUF(window)->window&FX_WINDOWBIT)) {
      if (!fast_gfx_table) {
        SetFont(BUF(window)->window_pointer->RPort, SystemFont);
        fast_gfx_table=fgAlloc(BUF(window)->window_pointer->RPort);
      }
      if (fast_gfx_table)
        use_ztext=TRUE;
    }
#endif
    rp=BUF(window)->window_pointer->RPort;
    if (CursorOnOff) {
      CursorXY(NULL, -1, -1);
    }
  
    if (line>0) {
      line+=BUF(top_offset)-2;
      y=line*charhojd+baseline+BUF(window)->text_start;
    } else {
      y=-line+(BUF(top_offset)-1)*charhojd+baseline+BUF(window)->text_start;
      line=-line/charhojd;
    }
    SetDrMd(rp, JAM2);
    SetFont(rp, SystemFont);

    if (!BUF(using_fact)->extra[FACT_NON_CHAR].length) {
      non_char_color_a=0;
      non_char_color_b=0;
    } else {
      non_char_color_a=*(char *)&BUF(using_fact)->extra[FACT_NON_CHAR].string[2];
      non_char_color_b=*(char *)&BUF(using_fact)->extra[FACT_NON_CHAR].string[3];
      if (BUF(using_fact)->extra[FACT_NON_CHAR].string[0]&cb_REVERSE)
        SWAP1(non_char_color_a, non_char_color_b);
    }
    goto line_start;
  
    while (1) {
      control=buf->control[offset];
      if (control!=type) {
        if (control==cb_EOS) {			/* End of string */
          break;
        } else if (control==cb_NEWLINE) {		/* return */
          offset++;
          {
            BOOL erase=BUF(window)->UpdtVars.EndPos[line].eraseline;
                       //&& !(type&(cb_ITALIC|cb_BOLD)); // Töm inte hela raden om sista tecknet var italic eller bold
            if (erase ||
                BUF(window)->UpdtVars.EndPos[line].blockstart>=0) {
              if (erase ||
                  BUF(window)->UpdtVars.EndPos[line].blockstart+BUF(window)->UpdtVars.EndPos[line].blocksize >
                  totallen) {
                int x=BUF(left_offset)+totallen*charbredd;
                if (type&cb_BOLD)
                  x+=SystemFont->tf_BoldSmear;
                if (type&cb_ITALIC)
                  x+=systemfont_rightmarg-SystemFont->tf_BoldSmear;
                SetAPen(rp, non_char_color_a);
                SetBPen(rp, non_char_color_b);
                RectFill(BUF(window)->window_pointer->RPort, x,
                         y-baseline,
                         BUF(window)->window_right_offset,
                         y-baseline+charhojd-1);
              }
            }
          }
          x=BUF(left_offset);
          y+=charhojd;
          Move(rp, x, y);
          BUF(window)->UpdtVars.EndPos[line].eraseline=eraseline;
          BUF(window)->UpdtVars.EndPos[line].bitplanes=bitplanes;
          BUF(text_using_bitplanes)|=bitplanes;
          BUF(visible_bitplanes)|=bitplanes;
          BUF(window)->UpdtVars.EndPos[line].blockstart=-1;
          line++;
          if (buf->control[offset]==cb_EOS)			/* End of string */
            break;
  line_start:		/* beginning of line */
          totallen=0;
          eraseline=0;
          type=0;
          SetSoftStyle(rp, 0, FSF_UNDERLINED|FSF_BOLD|FSF_ITALIC);
          bitplanes=non_char_color_a|non_char_color_b;
          if (bitplanes)
            eraseline=TRUE;
          SetWrMsk(rp, BUF(window)->graphic_card?-1:BUF(window)->UpdtVars.EndPos[line].bitplanes|bitplanes);
          if (eraseline || BUF(window)->UpdtVars.EndPos[line].eraseline) {
            SetAPen(rp, non_char_color_a);
            SetBPen(rp, non_char_color_b);
            RectFill(BUF(window)->window_pointer->RPort,
                     BUF(window)->window_left_offset-systemfont_leftmarg,
                     y-baseline,
                     BUF(left_offset),
                     y-baseline+charhojd-1);
          }
          color_a=1;
          color_b=0;
          color=1;
          bitplanes|=1;
          SetAPen(rp, color_a);
          SetBPen(rp, color_b);
          Move(rp, x, y);
          continue;
        }
			/* Default */
        if ((control^type) & (cb_BOLD|cb_ITALIC|cb_UNDERLINE)) {
          register int temp=0;
          if (control&cb_BOLD) {
            temp|=FSF_BOLD;
            eraseline=temp;
          }
          if (control&cb_ITALIC) {
            temp|=FSF_ITALIC;
            eraseline=temp;
          }
          if (control&cb_UNDERLINE)
            temp|=FSF_UNDERLINED;
          SetSoftStyle(rp, temp, FSF_UNDERLINED|FSF_BOLD|FSF_ITALIC);
        }
        if ((control&cb_REVERSE)!=(type&cb_REVERSE)) {
          if (control&cb_REVERSE) {
            SetAPen(rp, color_b);
            SetBPen(rp, color_a);
          } else {
            SetAPen(rp, color_a);
            SetBPen(rp, color_b);
          }
        }
        type=control;
      }
      if (color!=buf->colors[offset]) {
        color=buf->colors[offset];
        color_a=color&255;
        color_b=(color>>8)&255;
        if (control&cb_REVERSE) {
          SetAPen(rp, color_b);
          SetBPen(rp, color_a);
        } else {
          SetAPen(rp, color_a);
          SetBPen(rp, color_b);
        }
        bitplanes|=color_a|color_b;
        SetWrMsk(rp, BUF(window)->graphic_card?-1:BUF(window)->UpdtVars.EndPos[line].bitplanes|bitplanes);
      }
      {
        char *pos;
        pos=&buf->text[offset];
        do {
          offset++;
        } while (buf->control[offset]==control && buf->colors[offset]==color);
        printlen=&buf->text[offset]-pos;
        totallen+=printlen;
        if (totallen>BUF(screen_col)) {
          printlen=BUF(screen_col)-(totallen-printlen);
          totallen=BUF(screen_col);
        }
        if (printlen>0) {
#ifdef USE_FASTSCROLL
          if (use_ztext)
            fgText(rp, pos, printlen, fast_gfx_table);
          else
#endif
            Text(rp, (CONST_STRPTR)pos, printlen);
        }
      }      
    }
    SetAPen(rp, 1);
    SetBPen(rp, 0);
  }
}


/********************************************************************
 *
 * Status(BufStruct *, char *, int mode);
 *
 * Writes the text (char *) on the status line.
 * Mode: 0 - hela raden
 *       1 - namnet
 *       2 - infot
 *	 3 - namnet om möjligt, annars så mycket som möjlgt.
 *   bit 7 - forcea utskriften, trots Visible-flaggan.
 ********/
void __regargs Status(BufStruct *Storage, char *text, int mode)
{
  int textlen;
  int offset;

  if ((Visible==VISIBLE_ON || mode&0x80) && OwnWindow) {
    register int pos;
    register int bredd=BUF(screen_col);
    textlen=strlen(text);
    offset=0;
    mode&=0x7f;
    if (!BUF(window) && FRONTWINDOW)
      Storage=FRONTWINDOW->NextShowBuf;
    if (Storage && BUF(window) && BUF(window)->window_pointer &&
        BUF(window)->Visible==VISIBLE_ON) {
      register struct RastPort *rp=BUF(window)->window_pointer->RPort;
      if (mode) {
        if (mode==2) {
          offset=(BUF(screen_col)>>1);
          bredd=BUF(screen_col)-offset;
          if (bredd<=0)
            return;
          BUF(namedisplayed)&=~2;
        } else {
          if (!(BUF(namedisplayed) & 2)) {	/* Hela namnet om infot är tomt. */
            mode=0;
          } else {
            if (mode==1) {
              if (bredd>(BUF(screen_col)>>1))
                bredd=(BUF(screen_col)>>1);
            } else {
              if (mode==3) {
                if (textlen<(BUF(screen_col)>>1)) {
                  bredd=(BUF(screen_col)>>1);
                } else { //if (textlen<=bredd) {}
//                  bredd=textlen+1;
                  BUF(namedisplayed)=0;
                }
              }
            }
          }
        }
      } else
        BUF(namedisplayed)=0;
      if (mode!=2)
        BUF(namedisplayed)&=~1;
      if (BUF(window->colour_status_info)<0 || mode!=2)
        SetAPen(rp, statustextcolor);
      else
        SetAPen(rp, BUF(window->colour_status_info));
      SetSoftStyle(rp, FS_NORMAL, FSF_UNDERLINED|FSF_BOLD|FSF_ITALIC);
      SetDrMd(rp, JAM2);
      SetWrMsk(rp, (unsigned char)-1);
      SetBPen(rp, statuscolor);

      pos=stccpy((char *)statusbuffer, text, bredd+1)-1;

      if (pos<bredd) {
        memset(statusbuffer+pos, ' ', bredd-pos+1);
      }

      Move(rp, offset*SystemFont->tf_XSize+BUF(left_offset),
           (BUF(top_offset)+BUF(screen_lines)-1)*charhojd+baseline+BUF(window)->text_start);
      Text(rp, statusbuffer, bredd);

      BUF(window)->UpdtVars.EndPos[BUF(screen_lines)+BUF(top_offset)-1].len=BUF(screen_col);
      BUF(window)->UpdtVars.EndPos[BUF(screen_lines)+BUF(top_offset)-1].blockstart=-1;
      BUF(window)->UpdtVars.EndPos[BUF(screen_lines)+BUF(top_offset)-1].bitplanes=(unsigned char)-1;
      BUF(window)->UpdtVars.EndPos[BUF(screen_lines)+BUF(top_offset)-1].eraseline=TRUE;
    }
  } else {
    UpDtNeeded|=UD_SHOW_STATUS;
    BUF(update)|=UD_SHOW_STATUS;
  }
}

/*****************************************
*
* ScrollScreen(BufStruct *, int dif, int special);
*
* Scroll the screen <dif> steps.  Up (-) and down (+).
* Defines for 'special' defined in "WindowOutput.h"
* No return.
*
* For the Amiga version will this function have an internal loop while the
* user presses the slider gadget.
* 
*****/
void __regargs ScrollScreen(BufStruct *Storage, int totaldif, int special)
{
  if (Visible!=VISIBLE_ON || !BUF(window) || BUF(window)->Visible!=VISIBLE_ON ||
      !BUF(window)->window_pointer) {
    BUF(curr_topline)=FoldMoveLine(Storage, BUF(curr_topline), -totaldif);
    if (Visible!=VISIBLE_ON) {
      UpDtNeeded|=UD_REDRAW_ENTRY;
      BUF(update)|=UD_REDRAW_ENTRY;
    }
  } else if ((totaldif || special==scroll_SLIDER)) {
    int antaltecken;
    int bitmask;
    int ysize=SystemFont->tf_YSize;
    int dif, antalrader=totaldif, stop=TRUE, goingline=0, blockflag=TRUE;
    struct IntuiMessage *msg;
    int old_topline;

    int ymin=BUF(window)->text_start+ysize*(BUF(top_offset)-1);
    int ymax=BUF(window)->text_start-1+ysize*((BUF(top_offset)-1)+BUF(screen_lines));


    CursorXY(Storage, -1, -1);
    if (abs(totaldif)>=BUF(screen_lines)) {
      BUF(curr_topline)=FoldMoveLine(Storage, BUF(curr_topline), -totaldif);
      {
        register int temp;
        temp=FoldFindLine(Storage, SHS(line));
        if (temp>0)
          BUF(curr_topline)=FoldMoveLine(Storage, SHS(line), -(BUF(screen_lines)-1));
      }
      UpdateScreen(Storage);
    } else {

#ifdef USE_FASTSCROLL
      if (Default.fast_scroll && FastGraphicsBase && !fast_gfx_table) {
        SetFont(BUF(window)->window_pointer->RPort, SystemFont);
        fast_gfx_table=fgAlloc(BUF(window)->window_pointer->RPort);
      }
#endif
      stop=FALSE;
      while (!stop) {
        bitmask=BUF(text_using_bitplanes);
        if (BUF(block_exists)) {
          if (BUF(block_exists)==be_MARKING || 
              (BUF(block_end_y)>=BUF(curr_topline) &&
               BUF(block_begin_y)<FoldMoveLine(Storage, BUF(curr_topline), BUF(screen_lines))))
          bitmask=BUF(visible_bitplanes);
        }
        old_topline=BUF(curr_topline);
        if (special==scroll_SLIDER) {
          if (!stop) {
            if (!(msg=(struct IntuiMessage *)GetMsg(WindowPort)))
              Wait(1 << WindowPort->mp_SigBit);
            if (msg || (msg=(struct IntuiMessage *)GetMsg(WindowPort))) {
              if (msg->Class==IDCMP_GADGETUP)
                stop=TRUE;
              ReplyMsg((struct Message *)msg);
            }
          }
          {
            register int line=-1, dif;
            if (SHS(line)-BUF(screen_lines)>0)
              line =1+MultML((int)(((struct PropInfo *)BUF(slide.Slider.SpecialInfo))->VertPot),(SHS(line)-BUF(screen_lines)),MAXBODY);
            if (line) {
              if (line<=0)
                line=1;
              line=FoldNearestLine(Storage, line);
              {
                register int temp=FoldMoveLine(Storage, SHS(line), -BUF(screen_lines)+1);
                if (temp<line)
                  line=temp;
              }
              if (goingline!=line) {
                blockflag=FALSE;
                goingline=line;
                dif=old_topline-line;
                BUF(curr_line)-=dif;
                BUF(curr_topline)=line;
                if (abs(dif) > BUF(screen_lines)) {
                  {
                    register int counter;
                    for (counter=0; counter<=BUF(screen_lines); counter++)
                      BUF(window)->UpdtVars.EndPos[BUF(top_offset)+counter].len=BUF(screen_col);
                  }
                  UpdateScreen(Storage);
                  Showstat(Storage);
                  old_topline=BUF(curr_topline);
                }
              }
            }
          }
        } else {
          stop=TRUE;
          BUF(curr_topline)=FoldMoveLine(Storage, BUF(curr_topline), -antalrader);
          if (BUF(curr_topline)>old_topline) {
            register int temp;
            temp=FoldFindLine(Storage, SHS(line));
            if (temp>0)
              BUF(curr_topline)=FoldMoveLine(Storage, SHS(line), -(BUF(screen_lines)-1));
          }
        }
        dif=FoldDiff(Storage, BUF(curr_topline), old_topline);
        antalrader=dif;
        dif*=ysize;
        SetBPen(BUF(window)->window_pointer->RPort, 0);
        SetDrMd(BUF(window)->window_pointer->RPort, JAM1);
        SetWrMsk(BUF(window)->window_pointer->RPort, BUF(window)->graphic_card?-1:bitmask);
        BUF(window)->UpdtVars.UsingFact=BUF(using_fact);
        if (dif<0) {					/* Flytta upp */
          if (BUF(screen_lines)>1) {
#ifdef USE_FASTSCROLL
            if (Default.fast_scroll && FastGraphicsBase && fast_gfx_table) {
              fgScrollRaster(fast_gfx_table,
                        BUF(window)->window_pointer->RPort, 0, -dif,
                        BUF(window)->window_pointer->BorderLeft, ymin,
                        BUF(window)->window_right_offset, ymax);
            } else
#endif
              ScrollRaster(BUF(window)->window_pointer->RPort, 0, -dif, 
                           BUF(window)->window_left_offset-systemfont_leftmarg, ymin, 
                           BUF(window)->window_right_offset, ymax);
            if (antalrader)
              memcpy((char *)&BUF(window)->UpdtVars.EndPos[BUF(top_offset)-1],
                     (char *)&BUF(window)->UpdtVars.EndPos[BUF(top_offset)-1-antalrader],
                     (int)((BUF(screen_lines)+antalrader)*sizeof(struct EndPosStruct)));
          }
          if (antalrader) {
            antaltecken=UpdtLinesC(Storage, &ScreenBuffer,
                                    FoldMoveLine(Storage, BUF(curr_topline), BUF(screen_lines)+antalrader), -antalrader,
                                    BUF(screen_lines)+antalrader);
//            SystemPrint(Storage, &ScreenBuffer, antaltecken, -(ysize*(BUF(screen_lines)+antalrader)));
            SystemPrint(Storage, &ScreenBuffer, antaltecken, BUF(screen_lines)+antalrader+1);
          }
        } else if (dif>0) {				/* Flytta ner */
          if (BUF(screen_lines)>1) {
#ifdef USE_FASTSCROLL
            if (Default.fast_scroll && FastGraphicsBase && fast_gfx_table) {
              fgScrollRaster(fast_gfx_table,
                        BUF(window)->window_pointer->RPort, 0, -dif,
                        BUF(window)->window_pointer->BorderLeft, ymin, BUF(window)->window_right_offset, ymax);
            } else
#endif
              ScrollRaster(BUF(window)->window_pointer->RPort, 0, -dif, 
                           BUF(window)->window_left_offset-systemfont_leftmarg, ymin, 
                           BUF(window)->window_right_offset, ymax);
            if (antalrader)
              MoveUp(&BUF(window)->UpdtVars.EndPos[BUF(top_offset)-1+antalrader],
                     &BUF(window)->UpdtVars.EndPos[BUF(top_offset)-1],
                     (sizeof(struct EndPosStruct)*(BUF(screen_lines)-antalrader)));
          }
          if (antalrader) {
            antaltecken=UpdtLinesC(Storage, &ScreenBuffer, BUF(curr_topline), antalrader, 0);
            SystemPrint(Storage, &ScreenBuffer, antaltecken, 1);
          }
        }
        if (dif && special==scroll_SLIDER) {
          BUF(curr_line)=FoldMoveLine(Storage, BUF(curr_topline), BUF(cursor_y)-1);
          Showstat(Storage);
        }
        if (BUF(block_exists)) {
          UpdateBlock(Storage);
          SetBlockMark(Storage, FALSE);
        }
      }
      if (special==scroll_SLIDER && SHS(line)-BUF(screen_lines)<=0)
        MoveSlider(Storage);
    }
  }
}



void __regargs ClearWindow(WindowStruct *win)
{
  if (win && win->window_pointer) {
    SetAPen(win->window_pointer->RPort, 0);
    SetWrMsk(win->window_pointer->RPort, 0xFF);
    SetDrMd(win->window_pointer->RPort, JAM1);
    RectFill(win->window_pointer->RPort, win->window_pointer->BorderLeft,
             win->window_pointer->BorderTop,
              win->window_pointer->Width-win->window_pointer->BorderRight-1,
             win->window_pointer->Height-win->window_pointer->BorderBottom-1);
  }
}


/*****************************************
*
* CursorXY (BufStruct *, int, int);
*
* Places the cursor at the coordinates (int, int).
* x=-1 makes cursor invisible, y=-2 will not write anything.
* x=-2 makes cursor shady. x=-3 makes cursor unshady.
*****/
void __regargs CursorXY(BufStruct *Storage, int x, int y)
{
  static int xpos=-1, ypos=-1, mode=0, shade=FALSE, xwidth=0, ywidth;
  static short oldmask=0;
  static struct RastPort *rp;
  static UWORD areapat[2][2] = {
    { 0x5555, 0xaaaa },
    { 0xaaaa, 0x5555 }
  };
  register int xp, yp;

  if (!cursorhidden) {
    if (!Storage)
      Storage=FRONTWINDOW->NextShowBuf;
    if (CursorOnOff && /* Visible==VISIBLE_ON &&*/
        Storage && BUF(window) && BUF(window)->window_pointer) {
      xp=(x-1+BUF(l_c_len)+BUF(fold_len))*charbredd+BUF(left_offset);
      yp=(BUF(top_offset)+y-2)*charhojd+BUF(window)->text_start;
      if (xpos!=xp || ypos!=yp || shade) {
        {
          if (xpos>=0) {
            SetDrMd(rp,COMPLEMENT);
            SetWrMsk(rp, oldmask);
            if (y!=-2) {
              if (shade) {
                SetAfPt(rp, (UWORD *)&areapat, 1);
                shade=FALSE;
              }
              RectFill(rp, xpos, ypos, xpos+xwidth-1, ypos+ywidth-1);
              SetAfPt(rp, NULL, 0);
            }
          }
        }
        if (x==-1 || Visible!=VISIBLE_ON || BUF(window)->Visible!=VISIBLE_ON) {
          xpos=-1;
          CursorOnOff=FALSE;
        } else {
          if (x!=-2) {
            if (x!=-3) {
              xwidth=charbredd;
              ywidth=charhojd;
              if (Storage) {		// Räkna ut bredden på cursorn
                int rad=BUF(curr_line);
                if (rad<SHS(line) || BUF(string_pos)<LEN(rad)) {
                  unsigned char *line=RAD(rad);
                  if (line) {
                    unsigned char tkn=line[BUF(string_pos)];
                    if (!(BUF(using_fact)->flags[tkn]&fact_TAB)) {
                      tkn=BUF(using_fact)->length[tkn];
                      if (tkn>0) {		// Ändra bara om längden är större än noll.
                        if (tkn>BUF(screen_col)-x+1)
                          tkn=BUF(screen_col)-x+1;
                        xwidth=tkn*charbredd;
                      }
                    }
                  }
                }
              }
              xpos=xp;
              ypos=yp;
            }
          } else
            shade=TRUE;
          mode=Storage?(BUF(block_exists)&be_MARKING):0;
          if (Storage && xpos>=0) {
            rp=BUF(window->window_pointer->RPort);
            oldmask=BUF(window->cursor_pen);
            SetDrMd(rp,COMPLEMENT);
            SetWrMsk(rp, BUF(window->cursor_pen));
            if (shade ||
                !(BUF(window->window_pointer)->Flags&WFLG_WINDOWACTIVE)) { /*!activewindow*/
              shade=TRUE;
              SetAfPt(rp, (UWORD *)&areapat, 1);
            }
            RectFill(rp, xpos, ypos, xpos+xwidth-1, ypos+ywidth-1);
            SetAfPt(rp, NULL, 0);
          } else
            xpos=-1;
        }
      }
    }
  } else
    CursorOnOff=FALSE;
}
