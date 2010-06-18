/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/*****************************************
*
* Cursor.c
*
* Most of all the cursor stuff!
*
*****/

#include <graphics/rastport.h>
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <proto/graphics.h>
#include <string.h>

#include "Buf.h"
#include "BufControl.h"
#include "Cursor.h"
#include "FACT.h"
#include "Fold.h"
#include "OpenClose.h"
#include "Slider.h"
#include "UpdtScreen.h"
#include "WindowOutput.h"

extern struct GfxBase *GfxBase;
extern char buffer[];
extern DefaultStruct Default;
extern char CursorOnOff;
extern int Visible;

/*****************************************
*
* CursorUD(BufStruct *);
*
* Handles all cursor and screen limits when moving cursor up and down.
*
*****/
void __regargs CursorUD(BufStruct *Storage)
{
  int rad=BUF(curr_line), langd;
  int pos=BUF(cursor_x)+BUF(screen_x);
  if ( BUF(cursor_x) >=       /* shorter line ? */
      (langd=(Byte2Col(Storage, LEN(rad)-(SHS(line)!=rad), RAD(rad))-BUF(screen_x)))) {
    if (BUF(wanted_x)<pos)
      BUF(wanted_x)=pos;
    BUF(cursor_x)=langd;
    BUF(string_pos)=LEN(rad)-(SHS(line)!=rad);
  } else {		/* align cursor_x position to a correct char */
    int radlen=langd+BUF(screen_x);

    /* should the cursor should step a few steps to a side... */
    if (BUF(wanted_x)) {
      if (BUF(wanted_x)>=radlen) {
	BUF(cursor_x)=radlen;
	BUF(string_pos)=Col2Byte(Storage, radlen, RAD(rad), LEN(rad)+(SHS(line)==rad));
      } else
	BUF(cursor_x)=Byte2Col(Storage,
				BUF(string_pos)=Col2Byte(Storage, BUF(wanted_x), RAD(rad), LEN(rad)),
				RAD(rad));
      BUF(cursor_x)-=BUF(screen_x);
    } else {
      BUF(string_pos)=Col2Byte(Storage, pos, RAD(rad), LEN(rad) );
      BUF(cursor_x)=Byte2Col(Storage,BUF(string_pos), RAD(rad))-BUF(screen_x);
      if (pos!=BUF(cursor_x)+BUF(screen_x))
	BUF(wanted_x)=pos;
    }
  }
  CheckPos(Storage);
}

/*****************************************
*
* CursorLR(BufStruct *);
*
* Handles all cursor and screen limits when moving cursor left and right.
* Return: 1=nya cursorplatsen var giltig.
*         0=ej giltig.
*****/
int __regargs CursorLR(BufStruct *Storage)
{
  int rad=BUF(curr_line);
  int tempvisible=Visible;
  int ret=1;
  BUF(wanted_x)=0;

  if (BUF(string_pos)<0) { /* did we go left when standing on col 1 */
    register int oldscreenx=BUF(screen_x);
    if (rad==1) {
      ret=0;
      BUF(string_pos)=BUF(screen_x)=0;
      BUF(cursor_x)=1;
    } else {
      BUF(cursor_x)=(rad>1?Byte2Col(Storage, LEN(rad-1)-(SHS(line)!=rad-1), RAD(rad-1)):1);
      BUF(string_pos)=rad>1?LEN(rad-1)-1:0;
      BUF(screen_x)=0;
      BUF(cursor_y)--;
      BUF(curr_line)--;
      Visible=VISIBLE_OFF;
      if (!CheckPos(Storage) && BUF(screen_x)==oldscreenx) {
        Visible=tempvisible;
        CursorUp(Storage);
      } else {
        CursorUp(Storage);
        Visible=tempvisible;
        UpdateScreen(Storage);
      }
    }
  } else if (BUF(string_pos)>=(LEN(rad)/*+(rad==SHS(line))*/)) {/* beyond the edge */
    if (rad<SHS(line)) {
      BUF(cursor_x)=1;
      BUF(string_pos)=0;
      BUF(cursor_y)++;
      BUF(curr_line)++;
      if (BUF(screen_x)) {
        register int temp=Visible;
	Visible=VISIBLE_OFF;
	CursorDown(Storage);
	Visible=temp;
	BUF(screen_x)=0;
	UpdateScreen(Storage);
      } else CursorDown(Storage);
    } else { /* if we're on the last line ! */
      if (BUF(string_pos)>LEN(rad)) {
        BUF(string_pos)--;
        BUF(cursor_x)--;
      }
      ret=0;
    }
  } else
    CheckPos(Storage);
  return(ret);
}


/*****************************************
*
* int CursorUp (BufStruct *);
*
* Call CursorUp() on every cursorup event. It returns (int) whether the
* screen has scrolled or not.
*
*****/
int __regargs CursorUp(BufStruct *Storage)
{
  int ret=FALSE;
  if ((BUF(cursor_y)<=BUF(uppermarg)) && (BUF(curr_topline)>1)) {

//    BUF(curr_topline)--;
    if (BUF(cursor_y)<=0) {
      BUF(cursor_y)=0;
    }
    BUF(cursor_y)++;
    ScrollScreen(Storage, 1, scroll_NORMAL);
    MoveSlider(Storage);

    ret=TRUE;
  } else if ((BUF(curr_topline)==1) && (BUF(cursor_y)<1)) {
    BUF(cursor_y)++;
    BUF(curr_line)++;
  }
//  CursorXY(Storage, BUF(cursor_x), BUF(cursor_y));
  AdjustCursorUp(Storage);
  return(ret);
}

/*****************************************
*
* int CursorDown (BufStruct *);
*
* Call CursorDown() on every cursordown event. It returns (int) whether the
* screen has scrolled or not.
*
*****/

int __regargs CursorDown(BufStruct *Storage)
{
  int ret=FALSE;
  if (BUF(curr_line) > SHS(line) ) {
    BUF(cursor_y)--;
    BUF(curr_line)--;
//    CursorXY(Storage, BUF(cursor_x), BUF(cursor_y));
  } else {
    if ( (BUF(cursor_y)>(BUF(screen_lines)-BUF(lowermarg))) &&
         (FoldFindLine(Storage, SHS(line))<0) ) {

//      BUF(curr_topline)++;
      BUF(cursor_y)--;
      ScrollScreen(Storage, -1, scroll_NORMAL);
      MoveSlider(Storage);
      ret=TRUE;
    }
  }
  AdjustCursorDown(Storage);
  if (BUF(curr_line)==SHS(line) && LFLAGS(SHS(line))&FOLD_HIDDEN)
    BUF(cursor_y)=FoldFindLine(Storage, BUF(curr_line))+BUF(top_offset)-1;
  return(ret);
}



/**************************************************************
 *
 * MoveCursor(BufStruct *Storage, int antalcounter)
 *
 * Flytta cursorn (x) antal steg i sidled.
 *
 * Returnera antal steg som flyttats.
 *********/
int __regargs MoveCursor(BufStruct *Storage, int antalcounter)
{
  int retval=0;
  if (antalcounter>0) {			// Cursor Right
    while (--antalcounter>=0) {
      register rad=BUF(curr_line);
      if (LEN(rad)) {
        if (antalcounter>LEN(rad)) {
          antalcounter-=LEN(rad)-BUF(string_pos)-1;
          retval+=LEN(rad)-BUF(string_pos)-1;
          BUF(string_pos)+=LEN(rad)+1;
          BUF(cursor_x)=BUF(screen_x)=0;
        } else {
          BUF(cursor_x) += (BUF(using_fact)->flags[*(RAD(rad)+BUF(string_pos))]&fact_TAB)?
            BUF(tabsize) - (BUF(cursor_x)+BUF(screen_x)-1) % BUF(tabsize):
              (BUF(using_fact)->length[*(RAD(rad)+BUF(string_pos))]);
          BUF(string_pos)++;
        }
        retval++;
        if (!CursorLR(Storage)) {
          BUF(cursor_x)=Byte2Col(Storage, BUF(string_pos), RAD(rad));
          BUF(screen_x)=0;
          CheckPos(Storage);
          break;	// Exit the while(), reached end/beginning of buffer
        }
      } else
        break;// Avsluta om raden är tom.
    }// while()
  } else if (antalcounter<0) {		// Cursor Left
    antalcounter=-antalcounter;
    while (--antalcounter>=0) {
      if (antalcounter>BUF(string_pos)) {
        antalcounter-=BUF(string_pos);
        retval-=BUF(string_pos);
        BUF(string_pos)=-1;
        BUF(cursor_x)=BUF(screen_x)=0;
      } else {
        register rad=BUF(curr_line);
        if (BUF(string_pos)>0) {
          BUF(cursor_x)= (BUF(using_fact)->flags[*(RAD(rad)+(--BUF(string_pos) ))]&fact_TAB)?
            Byte2Col(Storage, BUF(string_pos), RAD(rad))-BUF(screen_x):
              BUF(cursor_x)-BUF(using_fact)->length[*(RAD(rad)+BUF(string_pos))];
        } else {
          BUF(string_pos)--;
          BUF(cursor_x)=BUF(screen_x)=0;
        }
      }
      if (!CursorLR(Storage))
        break;	// Exit the while(), reached end/beginning of buffer
      retval--;
    }// while()
  }
  return(retval);
}

void __regargs SaveCursorPos(BufStruct *Storage, CursorPosStruct *cpos)
{
  cpos->curr_topline=BUF(curr_topline);
  cpos->cursor_x=BUF(cursor_x);
  cpos->cursor_y=BUF(cursor_y);
  cpos->curr_line=BUF(curr_line);
  cpos->screen_x=BUF(screen_x);
  cpos->wanted_x=BUF(wanted_x);
  cpos->string_pos=BUF(string_pos);
}

void __regargs RestoreCursorPos(BufStruct *Storage, CursorPosStruct *cpos, BOOL testc)
{
  BUF(curr_topline)=cpos->curr_topline;
  BUF(cursor_x)=cpos->cursor_x;
  BUF(cursor_y)=cpos->cursor_y;
  BUF(curr_line)=cpos->curr_line;
  BUF(screen_x)=cpos->screen_x;
  BUF(wanted_x)=cpos->wanted_x;
  BUF(string_pos)=cpos->string_pos;
  if (testc) {
    TestCursorPos(Storage);
    UpdateScreen(Storage);
  }
}

