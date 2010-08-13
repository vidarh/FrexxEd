/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * UpdtScreen.c
 *
 * Screen-drawing routines...
 *
 *********/

#include <devices/console.h>
#include <exec/io.h>
#include <exec/types.h>
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

#include "util.h"

#include "Buf.h"
#include "Block.h"
#include "BufControl.h"
#include "Change.h"
#include "Command.h"
#include "Cursor.h"
#include "Edit.h"
#include "FACT.h"
#include "Fold.h"
#include "Init.h"
#include "Limit.h"
#include "OpenClose.h"
#include "Request.h"
#include "Setting.h"
#include "Slider.h"
#include "Strings.h"
#include "UpdtBlock.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"
#include "WindowOutput.h"
#ifdef USE_FASTSCROLL
#include "fastGraphics.h"
#include "fastGraphics_pragma.h"
#include "fastGraphics_proto.h"
#endif

extern struct Setting **sets;
extern struct IOStdReq *WriteReq;
extern struct IntuitionBase *IntuitionBase;
extern int Visible;
extern struct screen_buffer ScreenBuffer;
extern struct GfxBase *GfxBase;
extern int radmodulo;
extern char CursorOnOff;
extern char buffer[];
extern int bufferlen;
extern DefaultStruct Default;
extern char LoChar, HiChar;
extern WORD charbredd, charhojd, baseline;
extern char *statusbuffer;
extern WORD sliderhighlite, statuscolor, statustextcolor;
extern BOOL OwnWindow;
extern struct TextFont *SystemFont;
extern struct TextFont *RequestFont;
extern int systemfont_leftmarg;
extern int systemfont_rightmarg;
extern int BarHeight;
extern int MacroOn;		/* Macro på (1) eller av (0) */
extern struct MsgPort *WindowPort;

extern int UpDtNeeded;
extern int redrawneeded;
extern BOOL match_for_fastscroll;
extern FACT *DefaultFact;
extern struct Library *FastGraphicsBase;
extern struct FastGraphicsTable *fast_gfx_table;

extern char FrexxEdStarted;


/*****************************************
*
* UpdateScreen (BufStruct *);
*
* Simply redraws the screen according to the (BufStruct *) values.
*
*****/
void __regargs UpdateScreen(BufStruct *Storage)
{
  if (Visible==VISIBLE_ON) {
    if (BUF(window) && BUF(window)->Visible==VISIBLE_ON) {
      int antal;
      BUF(window)->UpdtVars.UsingFact=BUF(using_fact);
      BUF(text_using_bitplanes)=0;
      BUF(visible_bitplanes)=0;
      CursorXY(Storage, -1, -1);
      antal=UpdtLinesC(Storage, &ScreenBuffer, BUF(curr_topline),
                       BUF(screen_lines), 0);
      SystemPrint(Storage, &ScreenBuffer, antal, 1);
      if (BUF(block_exists)) {
        SetBlockMark(Storage, FALSE);
      }
    }
    BUF(update)=UD_NOTHING;
  } else {
    UpDtNeeded|=UD_REDRAW_ENTRY;
    BUF(update)|=UD_REDRAW_ENTRY;
  }
}

/*****************************************
*
* UpdateDupScreen (BufStruct *);
*
* Redraws all screens duplicated by the (BufStruct *).
*
*****/
void __regargs UpdateDupScreen(BufStruct *Storage2)
{
  register BufStruct *Storage=Storage2;
  char check=0;
  do {
    if (BUF(window)) {
      if (CursorOnOff)
        CursorXY(Storage, -1, -1);
      UpdateScreen(Storage);
    }
    if (!check) {
      if (BUF(NextSplitBuf))
        Storage=BUF(NextSplitBuf);
      else {
        Storage=Storage2;
        check++;
      }
    }
    if (check) {
      if (BUF(PrevSplitBuf))
        Storage=BUF(PrevSplitBuf);
      else {
        check++;
      }
    }
  } while (check<2);
}

/*****************************************
*
* UpdateLines (BufStruct *, int editrad);
*
* Update chosen buffer from line (int).
*
*****/
void __regargs UpdateLines (BufStruct *Storage2, int editrad)
{
  int antaltecken;
  int screenrad;
  if (Visible==VISIBLE_ON) {
    BufStruct *Storage=Storage2;
    int check=0;
    if (CursorOnOff) {
      CursorXY(Storage, -1, -1);
    }
    do {
      if (BUF(window) && BUF(window)->Visible==VISIBLE_ON) {
        screenrad=FoldFindLine(Storage, editrad);
        if (screenrad>0) {
          antaltecken=UpdtLinesC(Storage, &ScreenBuffer, editrad,
                                  BUF(screen_lines)-(screenrad-1),
                                  screenrad-1);
          SystemPrint(Storage, &ScreenBuffer, antaltecken, screenrad);
        }
      }
      if (!check) {
        if (BUF(NextSplitBuf))
          Storage=BUF(NextSplitBuf);
        else {
          Storage=Storage2;
          check++;
        }
      }
      if (check) {
        if (BUF(PrevSplitBuf))
          Storage=BUF(PrevSplitBuf);
        else {
          check++;
        }
      }
    } while (check<2);
  } else {
    UpDtNeeded|=UD_REDRAW_ENTRY;
    Storage2->update|=UD_REDRAW_ENTRY;
  }
}



/*****************************************
*
* UpdateWindow;
*
* Simply redraws all bufs in the window.
*
*****/
void UpdateWindow(WindowStruct *win)
{
  BufStruct *Storage;
  if (Visible==VISIBLE_ON) {
    if (win->Visible==VISIBLE_ON) {
      Storage=win->NextShowBuf;
  
      while (Storage) {
        UpdateScreen(Storage);
  
        InitSlider(Storage);
        MoveSlider(Storage);
        Showstat(Storage);
        Storage=BUF(NextShowBuf);
      }
      if (win->window&FX_WINDOWBIT)
        RefreshGList(win->window_pointer->FirstGadget, win->window_pointer, NULL, -1);
      UpDtNeeded&=~UD_REDRAW_VIEWS;
      win->UpDtNeeded&=~UD_REDRAW_VIEWS;
    }
  } else {
    UpDtNeeded|=UD_REDRAW_VIEWS;
    win->UpDtNeeded|=UD_REDRAW_VIEWS;
  }
}
/*****************************************
*
* UpdateAll (void);
*
* Simply redraws all bufs according to the BufStruct values.
*
*****/
void UpdateAll(void)
{
  WindowStruct *wincount;

  wincount=FRONTWINDOW;
  while (wincount) {
    UpdateWindow(wincount);
    wincount=wincount->next;
  }
}

/*****************************************
*
*  CenterScreen(BufStruct *)
*
*  Centrera önskad skärm.
*  Skärmen uppdateras EJ!
**********/
void __regargs CenterScreen(BufStruct *Storage)
{
  int top;

  top=FoldMoveLine(Storage, SHS(line), -(BUF(screen_lines)-1));
  BUF(curr_topline)=FoldMoveLine(Storage, BUF(curr_line), -BUF(screen_lines)/2);
  if (BUF(curr_topline>top))
    BUF(curr_topline)=top;
  if (BUF(curr_topline)<=0)
    BUF(curr_topline)=1;
  BUF(cursor_y)=FoldFindLine(Storage, BUF(curr_line));
	/* curr_line ska inte ändras */

#if before_fold
  if ((dif=BUF(curr_topline)+BUF(screen_lines)-SHS(line)-1)>=0) {
    if ((BUF(curr_topline)-=dif)<=0) {
      dif+=BUF(curr_topline)-1;
      BUF(curr_topline)=1;
    }
    BUF(cursor_y)+=dif;
    BUF(curr_line)+=dif;
  }
#endif
}


/*****************************************
*
* int Col2Byte(BufStruct *, int, char *, int length);
*
* Returns (int) where in the string (char *) with length (int)
* you should peek if you want the string from column (int).
*
*****/

int __regargs Col2Byte(BufStruct *Storage, int var, char *text, int length)
{
  int len=0;
  int templen=length;
  char *temp=text;
  if (!var || !text) return(0);
  while(length--) {
    if (len>=var) {
      if ((len=text-temp)>0)
        len--;
      return(len);
    }
    len += (BUF(using_fact)->flags[*text] & fact_TAB) ? BUF(tabsize)*(len/BUF(tabsize)+1)-len : BUF(using_fact)->length[*text];
    text++;
  }
  if (templen>0)
    templen--;
  return(templen);
}

/*****************************************
*
* int Byte2Col(BufStruct *, int, char *);
*
* Returns (int) the screen length of (int) bytes of the string (char *).
*
*****/

int __regargs Byte2Col(BufStruct *Storage, int var, char *text)
{
  int blaj, len=0;
  for (blaj=0; blaj<var; blaj++) {
    len += (BUF(using_fact)->flags[*text] & fact_TAB) ? BUF(tabsize)*(len/BUF(tabsize)+1)-len : BUF(using_fact)->length[*text];
    text++;
  }
  return(len+1);
}

/*****************************************
*
* PrintLine (BufStruct *, int line);
*
* Prints a line on the screen.
*
*****/
void __regargs PrintLine(BufStruct *Storage2, int editrad)
{
  if (Visible==VISIBLE_ON) {
    BufStruct *Storage=Storage2;
    int check=0, temp, screenrad;

    CursorXY(NULL, -1, -1);
    do {
      if (BUF(window) && BUF(window)->Visible==VISIBLE_ON) {
        screenrad=FoldFindLine(Storage, editrad);
        if (screenrad>0) {
          BUF(window)->UpdtVars.UsingFact=BUF(using_fact);
          temp=UpdtLinesC(Storage, &ScreenBuffer, editrad, 1, screenrad-1);
          SystemPrint(Storage, &ScreenBuffer, temp, screenrad);
          if (BUF(block_exists)) {
            SetBlockMark(Storage, FALSE);
          }
        }
      }
      if (!check) {
        if (BUF(NextSplitBuf))
          Storage=BUF(NextSplitBuf);
        else {
          Storage=Storage2;
         check++;
        }
      }
      if (check) {
        if (BUF(PrevSplitBuf))
          Storage=BUF(PrevSplitBuf);
        else {
          check++;
        }
      }
    } while (check<2);
  } else {
    UpDtNeeded|=UD_REDRAW_BUFFS;
    Storage2->shared->update|=UD_REDRAW_BUFFS;
  }
}

/*****************************************
*
* int CheckPos(BufStruct *);
*
* Moves the screen if the BUF(cursor_x) is out of line in any direction.
* Returns TRUE if the screen was updated, otherwise FALSE;
*
*****/
int __regargs CheckPos(BufStruct *Storage)
{
  int slask=BUF(screen_x);
  if (BUF(screen_x)<=0)
    BUF(screen_x)=0;
  while (BUF(cursor_x)>(BUF(screen_col)-BUF(rightmarg))) {
    BUF(screen_x)+=BUF(move_screen);
    BUF(cursor_x)-=BUF(move_screen);
  }
  while ((BUF(cursor_x)<=BUF(leftmarg)) && BUF(screen_x)>0) {
    BUF(screen_x)-=BUF(move_screen);
    BUF(cursor_x)+=BUF(move_screen);
  }
  if (BUF(screen_x)<0)
    BUF(screen_x)=0;

  if (slask!=BUF(screen_x)) {
    UpdateScreen(Storage);
    return(TRUE);
  } else {
    return(FALSE);
  }
}


/* Testar alla synliga buffrar */
int __regargs TestAllCursorPos()
{
  WindowStruct *win=FRONTWINDOW;
  register int ret=0;
  while (win) {
    register BufStruct *Storage=win->NextShowBuf;
    while (Storage) {
      ret|=TestCursorPos(Storage);
      Storage=BUF(NextShowBuf);
    }
    win=win->next;
  }
  return(ret);
}

  
/*****************************************
*
* int TestCursorPos(BufStruct *);
*
* Test if the cursor and blockmark are in valid places.
* Return TRUE if the screen was updated.
*****/
int __regargs TestCursorPos(BufStruct *Storage)
{
  int updt=FALSE, temp=Visible, tempx, dif;

  dif=BUF(curr_line)-SHS(line);
  if (BUF(curr_line)>SHS(line)) {
    BUF(curr_line)=SHS(line);
    updt=TRUE;
  }
  if (BUF(curr_topline)>BUF(curr_line)) {
    BUF(curr_topline)=BUF(curr_line);
    updt=TRUE;
  }
  updt|=LFLAGS(BUF(curr_topline)&FOLD_HIDDEN);
  updt|=LFLAGS(BUF(curr_line)&FOLD_HIDDEN);
  BUF(curr_topline)=FoldNearestLine(Storage, BUF(curr_topline));
  BUF(curr_line)=FoldNearestLine(Storage, BUF(curr_line));

  BUF(cursor_y)=1;
  for (dif=BUF(curr_line); dif>BUF(curr_topline); dif--) {
    if (!(LFLAGS(dif)&FOLD_HIDDEN)) {
      if (BUF(cursor_y)==BUF(screen_lines)) {
        BUF(curr_topline)=FoldNearestLine(Storage, dif);
        updt=TRUE;
        break;
      }
      BUF(cursor_y)++;
    }
  }

  Visible=VISIBLE_OFF;
  if (CheckPos(Storage))
    updt=TRUE;
  Visible=temp;
  tempx=BUF(cursor_x);
  {
    register int editrad=BUF(curr_line);
    if (BUF(string_pos)>LEN(editrad)-(editrad!=SHS(line)))
      BUF(string_pos)=LEN(editrad)-(editrad!=SHS(line));
    BUF(cursor_x)=Byte2Col(Storage, BUF(string_pos),
                           RAD(editrad))-BUF(screen_x);
  }
  if (tempx!=BUF(cursor_x)) {
    Visible=VISIBLE_OFF;
    CheckPos(Storage);
    Visible=temp;
    updt=TRUE;
  }
  if (BUF(block_exists)) {
    int temp=BUF(block_anc_x);
    if (BUF(block_anc_y)>SHS(line)) {
      BUF(block_anc_y)=SHS(line);
      updt=TRUE;
    }
    if (temp!=(BUF(block_anc_x)=Byte2Col(Storage, Col2Byte(Storage,
                                                           BUF(block_anc_x),
					    	         RAD(BUF(block_anc_y)),
					    	         LEN(BUF(block_anc_y))),
			                  RAD(BUF(block_anc_y)))))
      updt=TRUE;
  }
  if (updt) {
    UpdateScreen(Storage);
    MoveSlider(Storage);
  }
  return(updt);
}


/*****************************************
*
* SetScreen(BufStruct *, int, int);
*
* Set cursor at string position x (int) and line y (int).
* Returns TRUE if screen is updated.
*
*****/
int __regargs SetScreen(BufStruct *Storage, int x, int new_line)
{
  int old_top=BUF(curr_topline), top_dif;
  int old_scr=BUF(screen_x);
  int temp, uppermarg, lowermarg;

  if (new_line==0) new_line=1;
  if (new_line<0 || SHS(line)<new_line) new_line=SHS(line);
  if (x<0) x=LEN(new_line);
  if (x > (temp=LEN(new_line)-(SHS(line)!=new_line)))
    x=temp;
  BUF(string_pos)=x;
  x=Byte2Col(Storage, x, RAD(new_line));
  BUF(wanted_x)=0;

  BUF(curr_line)=new_line;

  uppermarg=BUF(uppermarg);
  if (BUF(uppermarg)>BUF(cursor_y)-1 && BUF(cursor_y)>0)
    uppermarg=BUF(cursor_y)-1;

  lowermarg=BUF(lowermarg);
  if (BUF(lowermarg)>BUF(screen_lines)-BUF(cursor_y)+1 && BUF(cursor_y)<=BUF(screen_lines))
    lowermarg=BUF(screen_lines)-BUF(cursor_y);

  temp=FoldFindLine(Storage, new_line);
  if (temp<0 || temp>=BUF(screen_lines)-lowermarg) {
    if (SHS(line)!=FoldMoveLine(Storage, new_line, lowermarg))
      BUF(curr_topline)=FoldMoveLine(Storage, new_line, -(BUF(screen_lines)-lowermarg-1));
    else
      BUF(curr_topline)=FoldMoveLine(Storage, SHS(line), -(BUF(screen_lines)-1));
  } else if (temp<=uppermarg) {
    BUF(curr_topline)=FoldMoveLine(Storage, new_line, -uppermarg);
  }
  BUF(cursor_y)=FoldFindLine(Storage, new_line);

#if before_fold
  if ((BUF(curr_topline)+uppermarg+1) <= new_line) {
    if ((new_line-BUF(curr_topline)) >= (BUF(screen_lines)-lowermarg)) {
      if (SHS(line)<BUF(screen_lines))
        BUF(cursor_y)=new_line;
      else {
        if ((new_line+lowermarg) > SHS(line))
          BUF(cursor_y)=BUF(screen_lines)-(SHS(line)-new_line);
        else
	  BUF(cursor_y) = BUF(screen_lines)-lowermarg;
      }
      BUF(curr_topline) = new_line-BUF(cursor_y)+1;
    } else {
      if (BUF(curr_topline)==1) {
        BUF(cursor_y)=new_line;
      } else {
        BUF(cursor_y) = new_line - BUF(curr_topline) + 1;
      }
    }
  } else {
    BUF(cursor_y) = uppermarg+1;
    BUF(curr_topline) = new_line-BUF(cursor_y) +1;
    if (BUF(curr_topline)<=0) {
      BUF(cursor_y)+=BUF(curr_topline)-1;
      BUF(curr_topline)=1;
    }
  }
#endif //before_fold

  if (((BUF(screen_x)+BUF(screen_col)-BUF(rightmarg)) <= x ) ||
       (((BUF(screen_x)+BUF(leftmarg)) >= x ) && BUF(screen_x))) {
    while ((BUF(screen_x)+BUF(screen_col)-BUF(rightmarg)) <= x)
      BUF(screen_x)+=BUF(move_screen);
    while (((BUF(screen_x)+BUF(leftmarg)) >= x) && BUF(screen_x)>0)
      BUF(screen_x)-=BUF(move_screen);
    if (BUF(screen_x)<0)
      BUF(screen_x)=0;
  }
  BUF(cursor_x) = x - BUF(screen_x);
  if (BUF(cursor_x)<=0) {
    BUF(screen_x)+=BUF(cursor_x)+1;
    BUF(cursor_x)=1;
  }
  if ((top_dif=old_top-BUF(curr_topline)) || old_scr!=BUF(screen_x)) {
#if before_fold
    BUF(curr_topline)=old_top;
    ScrollScreen(Storage, top_dif, scroll_NORMAL);
    if (old_scr!=BUF(screen_x)) {
      UpdateScreen(Storage);
    }
#else
    UpdateScreen(Storage);
#endif
    MoveSlider(Storage);
    return(TRUE);
  } else if (CursorOnOff)
    CursorXY(Storage, BUF(cursor_x),BUF(cursor_y));
  return(FALSE);
}


void __regargs UpdtDupStat(BufStruct *Storage)
{
  BufStruct *Storage0=Storage;
  if (BUF(window))
    Showstat(Storage);
  while (Storage0=Storage0->NextSplitBuf) {
    if (Storage0->window)
      Showstat(Storage0);
  }
  Storage0=Storage;
  while (Storage0=Storage0->PrevSplitBuf) {
    if (Storage0->window)
      Showstat(Storage0);
  }
}

void __regargs Showstat(BufStruct *Storage)
{
  if (FRONTWINDOW && (FrexxEdStarted!=2 || Storage!=FRONTWINDOW->NextShowBuf)) {
    Showname(Storage);
    Showplace(Storage);
  }
}

void ShowAllstat(void)
{
  WindowStruct *win=FRONTWINDOW;
  while (win) {
    register BufStruct *Storage=win->NextShowBuf;
    while (Storage) {
      Showstat(Storage);
      Storage=BUF(NextShowBuf);
    }
    win=win->next;
  }
}

void __regargs Showname(BufStruct *Storage)
{
  register int len=0;
  char array[7];

  if (BUF(window) && BUF(window)->Visible==VISIBLE_ON) {
    if (Visible==VISIBLE_ON) {
      if (SHS(name_number))
        len+=Sprintf(buffer+len, "(%ld) ", SHS(name_number))-1;
    
      if (SHS(shared)>1)
        len+=Sprintf(buffer+len, " #%ld ", BUF(view_number))-1;
    
      buffer[len++]=' ';
      if (!SHS(filnamn[0])) {
        strcpy(buffer+len, DEFAULTNAME);
        len+=sizeof(DEFAULTNAME);
      } else {
        if (Default.showpath) {
          register int bredd=(BUF(screen_col)>>1)-1;
          strmfp(buffer+len, SHS(path), SHS(filnamn));
          len=strlen(buffer);
          if (len>bredd && len>8) {
            bredd=bredd>>1;
            memcpy(buffer+bredd, "...", 3);
            strcpy(buffer+bredd+3, buffer+len-bredd+3);
          }
        } else {
          Sprintf(array, "%%-%02ds ", 30 - len);
          Sprintf(&buffer[len], array, SHS(filnamn));
        }
      }
      Status(Storage, buffer, 1);
      BUF(namedisplayed)|=1;
    } else
      UpDtNeeded|=UD_SHOW_STATUS;
  }
}

void __regargs Showplace(BufStruct *Storage)
{
//  static char buffer[80]="                    ";  borta 950328
  static char *randomlength[]={"%ld","%lx","%lX"};
  static char *set_length[]={"%ld  ","%02lx","%02lX"};
#ifdef STATUSTEST
  static int lapcount=0;
#endif
  char *syntax=Default.status_line;
  int pointer=0;
  int currentline=BUF(curr_line);
  BOOL repeat;
  BOOL boolean;
  char true_char, false_char;
  char hex;
  int len;
  int pad;

  if (BUF(window)) {
    while (*syntax) {
      switch (*syntax) {
      case '$':
        hex=0;
        boolean=FALSE;
        pad=0;
        do {
          repeat=FALSE;
          syntax++;
          switch (*syntax) {
          case 'f':	// flaggor
            buffer[pointer++]=SHS(changes)?	'c':' ';
            buffer[pointer++]=FLAG(insert_mode)?	'I':' ';
            buffer[pointer++]=BUF(block_exists)?
                      ((BUF(block_exists)==be_MARKING)?
                      			'b':'B'):
          				    ' ';
            buffer[pointer++]=MacroOn?		'M':' ';
            buffer[pointer++]=SHS(pack_type[0])?	'P':' ';
            buffer[pointer++]=(SHS(fileprotection)&S_IWRITE)?	'R':' ';
            break;
          case 'x':	// hex på
            hex=2;// ska vara så.
            repeat=TRUE;
            break;
          case 'X':	// hex stora på
            hex=1;// ska vara så.
            repeat=TRUE;
            break;
          case 'l':	// radnummer
            len=Sprintf(&buffer[pointer], randomlength[hex], currentline);
            pointer+=len;
            pad=5-len;
            break;
          case 'L':	// radnummer-sträng
            pointer+=Sprintf(&buffer[pointer], RetString(STR_LINE));
            break;
          case 'C':	// kolumn-sträng
            pointer+=Sprintf(&buffer[pointer], RetString(STR_COL));
            break;
          case 'c':	// kolumn
            len=Sprintf(&buffer[pointer], randomlength[hex], BUF(cursor_x)+BUF(screen_x));
            pointer+=len;
            pad=4-len;
            break;
          case 'a':	// ASCII-tecknet
            {
              char tkn=32;
              if (LEN(currentline)>BUF(string_pos))
                tkn=*(RAD(currentline)+BUF(string_pos));
              buffer[pointer]=tkn;
            }
            pointer++;
            break;
          case 'A':	// ASCII-numret
            {
              register char tkn=0;
              if (LEN(currentline)>BUF(string_pos))
                tkn=*(RAD(currentline)+BUF(string_pos));
              Sprintf(&buffer[pointer], set_length[hex], tkn);
            }
            pointer+=(hex?2:3);
            break;
          case 'P':	// Page
            len=Sprintf(&buffer[pointer], randomlength[hex], (currentline-1)/Default.page_length+1);
            pointer+=len;
            pad=4-len;
            break;
          case 'p':	// Pagerad
            Sprintf(&buffer[pointer], set_length[hex], (currentline-1)%Default.page_length+1);
            pointer+=2;
            break;
  
          case 's':	// Size
            len=Sprintf(&buffer[pointer], randomlength[hex], SHS(size));
            pointer+=len;
            pad=5-len;
            break;
          case 'r':	// antal rader
            len=Sprintf(&buffer[pointer], randomlength[hex], SHS(line));
            pointer+=len;
            pad=5-len;
            break;
          case 'n':	// Ändringar
            len=Sprintf(&buffer[pointer], randomlength[hex], SHS(changes));
            pointer+=len;
            pad=3-len;
            break;
          case 'b':	// Boolean
            boolean=TRUE;
            if (syntax[1]) {
              true_char=syntax[1];
              syntax++;
              if (syntax[1]) {
                false_char=syntax[1];
                syntax++;
                repeat=TRUE;
              }
            }
            break;
          case '\'':	// setting
            syntax++;
            {
              register char *stop=strchr(syntax, '\'');
              if (stop) {
                memcpy(&buffer[1000], syntax, stop-syntax);
                buffer[1000+stop-syntax]=0;
                syntax=stop;
                len=GetSettingName(&buffer[1000]);
                if (len>=0) {
                  register int value=ChangeAsk(Storage, len, NULL);
                  if (boolean) {
                    pointer+=Sprintf(&buffer[pointer], "%lc", (((sets[len]->type&15)!=ST_STRING)?value:*(char *)value)?true_char:false_char);
                  } else {
                    pointer+=Sprintf(&buffer[pointer],
                                     ((sets[len]->type&15)==ST_STRING)?"%s":randomlength[hex],
                                     value);
                  }
                }
              }
            }
            break;
#ifdef DEBUGTEST
          case 'd':	//Debug
            pointer+=Sprintf(&buffer[pointer], "%ld", BUF(cursor_y)+BUF(curr_topline)-1); // DEBUG
            break;
#endif
          default:
            buffer[pointer]=*syntax;
            pointer++;
            break;
          }
        } while (repeat);
        if (pad>0) {
          memset(&buffer[pointer], ' ', pad);
          pointer+=pad;
        }
        break;
      default:
        buffer[pointer]=*syntax;
        pointer++;
        break;
      }
      syntax++;
    }
    buffer[pointer]=0;
  
  
  
#ifdef STATUSTEST
    Sprintf(&buffer[strlen(buffer)], " %ld", lapcount++);
#endif
  
    Status(Storage, buffer, 2);
    BUF(namedisplayed)|=2;
  }
}


void __regargs Showerror(BufStruct *Storage, int error)
{
  if (error<0) {
    if (error>LAST_ERROR) {
      Showplace(Storage);
      Status(Storage, RetString(abs(error)-1), 3);
    } else
      Status(Storage, (char *)(-error), 3);
  }
}

void ClearVisible()
{
  register BufStruct *Storage;
  WindowStruct *wincount; //loop

  if (!FRONTWINDOW || !Default.windows_opened || !FrexxEdStarted)
    return;

  if (UpDtNeeded & UD_CHECK_FACT) {
    FACT *factcount=DefaultFact;
    do {
      CheckFACT(factcount);
      factcount=factcount->next;
    } while (factcount);
  }
  Visible=VISIBLE_ON;
  {
    WindowStruct *win=FRONTWINDOW; //loop
    while (win) {
      CheckWindowSize(win);
      win=win->next;
    }
  }

  wincount=FRONTWINDOW; //loop
  while (wincount) {

    if (wincount->UpDtNeeded & UD_CLEAR_SCREEN) {
      CursorXY(NULL, -1, -1);
      ClearWindow(wincount);
    }
    if (UpDtNeeded & UD_REDRAW_SETTING) {
      if (wincount->redrawneeded) {
        if (OK<=ReDrawSetting(wincount->NextShowBuf, wincount->redrawneeded))
          CheckWindowSize(wincount);
      }
    }
    if (wincount->Visible==VISIBLE_ON) {
      if (wincount->UpDtNeeded & UD_REDRAW_BORDER) {
        if (wincount->slider && wincount->Visible==VISIBLE_ON) {
          if (!(wincount->window&FX_WINDOWBIT)) {
            RefreshGList(wincount->window_pointer->FirstGadget,
                         wincount->window_pointer, NULL, -1);
          } else
            RefreshWindowFrame(wincount->window_pointer);
        }
      }
      if (wincount->UpDtNeeded & UD_REDRAW_VIEWS)
        UpdateWindow(wincount);
      if (UpDtNeeded & UD_MOVE_SLIDER) {
        Storage=wincount->NextShowBuf;
        while (Storage) {
          MoveSlider(Storage);
          Storage=BUF(NextShowBuf);
        }
      }
    }
    wincount->UpDtNeeded=UD_NOTHING;
    wincount=wincount->next;
  }
  if (UpDtNeeded & UD_REDRAW_BUFFS) {
    SharedStruct *shared=Default.SharedDefault.Next;
    while (shared) {
      if (shared->update&UD_REDRAW_BUFFS)
        UpdateDupScreen(shared->Entry);
      shared=shared->Next;
    }
  }
  if (UpDtNeeded & UD_REDRAW_ENTRY) {
    Storage=Default.BufStructDefault.NextBuf;
    while (Storage) {
      if (BUF(window) && BUF(window)->Visible==VISIBLE_ON &&
          BUF(update)&UD_REDRAW_ENTRY)
        UpdateScreen(Storage);
      Storage=BUF(NextBuf);
    }
  }
/*
  if (UpDtNeeded & UD_FACE_UPDATE) {
    Storage=Default.BufStructDefault.NextBuf;
    while (Storage) {
      if (BUF(window) && BUF(window)->Visible==VISIBLE_ON &&
          BUF(update)&UD_FACE_UPDATE) {
        BufStruct *shared=BUF(shared->Entry);
        int face=SHS(face_updated_line);
        while (shared) {
          UpdateLines(Storage, face);
          shared=shared->NextSplitBuf;
        }
      }
      Storage=BUF(NextBuf);
    }
  }
*/
  if (UpDtNeeded & UD_REDRAW_BLOCK) {
    Storage=Default.BufStructDefault.NextBuf;
    while (Storage) {
      if (BUF(window) && BUF(window)->window_pointer &&
          (BUF(update)&UD_REDRAW_BLOCK) && BUF(block_exists)) {
        SetBlockMark(Storage, FALSE);
      }
      Storage=BUF(NextBuf);
    }
  }
  if (UpDtNeeded & UD_SHOW_STATUS)
    ShowAllstat();
  if (FRONTWINDOW->Visible==VISIBLE_ON) {
    Visible=VISIBLE_ON;
    UpDtNeeded=UD_NOTHING;
    redrawneeded=0;
  } else
    Visible=VISIBLE_OFF;
}

void UpdateFace()
{
  BufStruct *Storage;
  Storage=Default.BufStructDefault.NextBuf;
  face_update=FALSE;
  while (Storage) {
    if (BUF(window)) {
      if (BUF(face_top_updated_line)>0 &&
          BUF(face_top_updated_line)<=BUF(face_bottom_updated_line)) {
        int face_start=BUF(face_top_updated_line);
        int face_end=BUF(face_bottom_updated_line);
        {
          int screen_start, screen_end;
          screen_start=FoldFindLine(Storage, face_start);
          if (screen_start>=0) {
            if (screen_start==0)
              screen_start=BUF(curr_topline);
            screen_end=FoldFindLine(Storage, face_end);
            if (screen_end<0)
              screen_end=BUF(screen_lines);
            if (screen_end!=0 && screen_start<=screen_end) {
              int antaltecken;
//FPrintf(Output(), "Screen change no %ld: screen_start %ld, end %ld (face_start %ld, end %ld)\n", BUF(view_number), screen_start, screen_end, face_start, face_end);
              antaltecken=UpdtLinesC(Storage, &ScreenBuffer, face_start,
                                      screen_end-screen_start+1, //antal rader
                                      screen_start-1);
              SystemPrint(Storage, &ScreenBuffer, antaltecken, screen_start);
              if (face_end!=BUF(face_bottom_updated_line))
                continue; /* Repeat again if necessary */
            } else
              UpdateScreen(Storage);
          }
        }
      } else {
        BUF(face_top_updated_line)=0;
        BUF(face_bottom_updated_line)=0;
      }
    }
    Storage=BUF(NextBuf);
  }
}
