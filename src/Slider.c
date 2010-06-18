/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/***********************************************
 *
 *  Slider.c
 *
 ***********/

#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "BufControl.h"
#include "MultML.h"
#include "Slider.h"
#include "UpdtScreen.h"


extern int Visible;
extern int UpDtNeeded;
extern DefaultStruct Default; /* the default values of a BufStruct */
extern struct TextFont *SystemFont;
extern struct Gadget VertSlider;
extern struct PropInfo VertSliderSInfo;
extern struct Image SliderImage;
extern WORD sliderhighlite, statuscolor, statustextcolor;

/* Makes some initialisations depending if the slider is to the left or right. */
void __regargs PositionSlider(BufStruct *Storage)
{
  int xpos=0;
  int slidewidth;
  int val;

  val=BUF(window->slider);
  if (BUF(window)->window_pointer && (BUF(window)->real_window_height<=BUF(window)->window_minheight || BUF(window)->real_window_height<BUF(window)->window_pointer->MinHeight))
    val=sl_OFF;	// Turn off sliders if the window is too small.

  slidewidth=16;
  if (BUF(window)->window&FX_WINDOWBIT) {
    if (val==sl_RIGHT /* val!=sl_OFF */) {
//      val=sl_RIGHT;
      if (BUF(window)->window_pointer)
        slidewidth=BUF(window)->window_pointer->BorderRight-8;
    }
    if (val==sl_LEFT) { //960902
      xpos=BUF(window)->window_pointer->BorderLeft;
    }
  }
  switch(val) {
  case sl_OFF:
    xpos=-40;
    break;  
  case sl_RIGHT:
    xpos=-slidewidth+1;
    break;
  case sl_LEFT:
//    xpos=0;
    break;  
  }
  if (BUF(window)->window_pointer) {
    if (val==sl_RIGHT) {
      if (!(BUF(window)->window&FX_WINDOWBIT))
        xpos=BUF(window)->real_window_width+xpos-1;
      else
        xpos-=4;
    }
  }
  if (BUF(window)) {
    BUF(slide.Slider.LeftEdge)=xpos;
    BUF(slide.Slider.Width)=slidewidth;
    if ((BUF(window)->window&FX_WINDOWBIT) && val==sl_RIGHT)
      BUF(slide.Slider.Flags)=GFLG_RELRIGHT;
    else
      BUF(slide.Slider.Flags)=NULL;

    BUF(slide.Border.gadget.LeftEdge)=xpos;
    InitBorder(Storage);

    BUF(left_offset)=0;
    BufLimits(Storage);
  }
}

/* Initialises the slider */
void __regargs InitSlider(BufStruct *Storage)
{
  if (BUF(window) && BUF(window->window_pointer)) {
    int yoffs=((BUF(top_offset)-1)*SystemFont->tf_YSize)+BUF(window)->text_start+2;
    int xoffs=((BUF(screen_lines)+1)*SystemFont->tf_YSize)-4;
    if ((BUF(window->window)&FX_WINDOWBIT) && !BUF(PrevShowBuf) &&
        BUF(window->slider)==sl_RIGHT) { // Översta viewn
      register int temp;
      temp=(BUF(window->real_window_height)-(BUF(window)->text_start-1)-BUF(window)->window_pointer->BorderBottom-1)%SystemFont->tf_YSize;
      xoffs+=-8+temp;
    }
    if (!(BUF(window)->window&FX_WINDOWBIT) || BUF(window->slider)==sl_LEFT) {
      yoffs-=2;
      xoffs+=4;
    }
    ShortInitSlider(Storage);
  
    if ((BUF(slide.Slider.TopEdge)!=yoffs) ||
        (BUF(slide.Slider.Height)!=xoffs)) {
      BUF(slide.Slider.Height)=xoffs;
      BUF(slide.Slider.TopEdge)=yoffs;
      InitBorder(Storage);
      if (BUF(window)) {
        if (BUF(window)->Visible==VISIBLE_ON &&
            Visible==VISIBLE_ON) {
          if ((BUF(window)->window&FX_WINDOWBIT) && BUF(window->slider)==sl_RIGHT)
            RefreshWindowFrame(BUF(window)->window_pointer);
          else
            RefreshGList(&BUF(slide.Border.gadget), BUF(window)->window_pointer, NULL, 2);
        } else {
          UpDtNeeded|=UD_REDRAW_BORDER;
          BUF(window)->UpDtNeeded|=UD_REDRAW_BORDER;
        }
      }
    }
    MoveSlider(Storage);
  }
}

/* Removes the slider for a buffer */
void __regargs RemoveBufGadget(BufStruct *Storage)
{
  if (BUF(window)) {
    if (BUF(window)->window_pointer && BUF(slider_on)) {
      RemoveGadget(BUF(window)->window_pointer, &BUF(slide.Border.gadget));
      RemoveGadget(BUF(window)->window_pointer, &BUF(slide.Slider));
      BUF(slider_on)=FALSE;
    }
    if (BUF(window)->Visible==VISIBLE_ON &&
        Visible==VISIBLE_ON)
      RefreshGList(BUF(window)->window_pointer->FirstGadget, BUF(window)->window_pointer, NULL, -1);
    else {
      UpDtNeeded|=UD_REDRAW_BORDER;
      BUF(window)->UpDtNeeded|=UD_REDRAW_BORDER;
    }
  }
}

/* Adds a slider for a buffer and place it in its window */
void __regargs AddBufGadget(BufStruct *Storage)
{
  int temp;

  if (!BUF(window) || BUF(slider_on) || !BUF(window)->window_pointer)
    return;

  memcpy(&BUF(slide),&Default.BufStructDefault.slide,sizeof(struct FrexxSlider));
//  *(BUF(Slider))=Default.Slider;
//  *(BUF(SliderSInfo))=Default.SliderSInfo;
//  *(BUF(SliderImage))=Default.SliderImage;

  PositionSlider(Storage);

  BUF(slide.Slider.SpecialInfo)=&BUF(slide.SliderSInfo);
  BUF(slide.Slider.GadgetRender)=&BUF(slide.SliderImage);

  BUF(slide.SliderSInfo.HorizPot)=NULL;
  BUF(slide.SliderSInfo.HorizBody)=NULL;

  BUF(slide.SliderSInfo.VertPot)=(SHS(line) > BUF(screen_lines))?
		((temp=SHS(line)-BUF(screen_lines)) < BUF(curr_topline)?MAXBODY:
		MAXBODY*(BUF(curr_topline)-1)/temp):
		NULL;
  BUF(slide.SliderSInfo.VertBody)=(SHS(line) > BUF(screen_lines))?
		MAXBODY*BUF(screen_lines)/SHS(line):MAXBODY;
  BUF(slide.Slider.TopEdge)=(BUF(top_offset)-1)*SystemFont->tf_YSize+BUF(window)->text_start+2;
  BUF(slide.Slider.Height)=(BUF(screen_lines)+1)*SystemFont->tf_YSize-4;
  if (BUF(window)->window_pointer && (BUF(window)->window&FX_WINDOWBIT) && 
      !BUF(PrevShowBuf) && BUF(window->slider)==sl_RIGHT) {
    BUF(slide.Slider.Height)+=-8+((BUF(window)->real_window_height-(BUF(window)->text_start-1)-BUF(window)->window_pointer->BorderBottom-1)%SystemFont->tf_YSize);
  }
  ShortInitSlider(Storage);
  if (!(BUF(window)->window&FX_WINDOWBIT) || BUF(window->slider)==sl_LEFT) {
    BUF(slide.Slider.TopEdge)-=2;
    BUF(slide.Slider.Height)+=4;
  }

  InitBorder(Storage);

  if (BUF(window)->window_pointer) {
    AddGadget(BUF(window)->window_pointer, &BUF(slide.Slider), 0);
    AddGadget(BUF(window)->window_pointer, &BUF(slide.Border.gadget), 0);
    RefreshGList(&BUF(slide.Border.gadget), BUF(window)->window_pointer, NULL, 2);
    BUF(slider_on)=TRUE;
  }

}

/* Short Amiga specified initialisation depending on the window type */
void __regargs ShortInitSlider(BufStruct *Storage)
{
	// only called from AddBufGadget and OpenUpScreen
  if ((BUF(window)->window&FX_WINDOWBIT) && BUF(window->slider)==sl_RIGHT)
    BUF(slide.SliderSInfo.Flags)=AUTOKNOB|FREEVERT|PROPBORDERLESS|PROPNEWLOOK;
  else
    BUF(slide.SliderSInfo.Flags)=AUTOKNOB|FREEVERT;
}

/* Initialise the borde around the slider (Amiga specific) */
void __regargs InitBorder(BufStruct *Storage)
{
  register struct Gadget *gadget=&BUF(slide.Slider);

  if (gadget && BUF(window) && BUF(window)->window_pointer) {

    if (BUF(window->slider)==sl_RIGHT /* || ((BUF(window)->window&FX_WINDOWBIT) && BUF(window->slider)==sl_LEFT) */)
      BUF(slide.Border.gadget.Flags)=GADGHBOX|GFLG_RELRIGHT|GACT_RIGHTBORDER;
    else
      BUF(slide.Border.gadget.Flags)=GADGHBOX;

    if (BUF(window->slider)==sl_RIGHT)
      BUF(slide.Border.gadget.LeftEdge)=-BUF(window)->window_pointer->BorderRight+3;
    else
      BUF(slide.Border.gadget.LeftEdge)=gadget->LeftEdge-1;//BUF(window)->window_pointer->BorderLeft;
    BUF(slide.Border.gadget.TopEdge)=gadget->TopEdge;

    BUF(slide.Border.gadget.GadgetRender)=(APTR)&(BUF(slide.Border.border1));
    BUF(slide.Border.border1.XY)=(APTR)&(BUF(slide.Border.pairs1));
    BUF(slide.Border.border1.FrontPen)=sliderhighlite;

    BUF(slide.Border.pairs1[0])=BUF(window)->window_pointer->BorderRight-4;
    BUF(slide.Border.pairs1[1])=-1;
    BUF(slide.Border.pairs1[2])=-1;
    BUF(slide.Border.pairs1[3])=-1;

    BUF(slide.Border.pairs1[4])=-1;
    BUF(slide.Border.pairs1[5])=gadget->Height-3;
    BUF(slide.Border.pairs1[6])=0;
    BUF(slide.Border.pairs1[7])=gadget->Height-3;
    BUF(slide.Border.pairs1[8])=0;
    BUF(slide.Border.pairs1[9])=-1;


    BUF(slide.Border.border1.Count)=2;
    if (!(BUF(window)->window&FX_WINDOWBIT) || BUF(window->slider)==sl_LEFT) {
      if (BUF(window->slider)==sl_LEFT) {
//        if (!(BUF(window)->window&FX_WINDOWBIT))
          BUF(slide.Border.gadget.LeftEdge)+=3;
      } else
        BUF(slide.Border.gadget.LeftEdge)=-gadget->Width+3;
      BUF(slide.Border.gadget.TopEdge)=gadget->TopEdge+2;
      BUF(slide.Border.pairs1[0])=gadget->Width-4;
      BUF(slide.Border.border1.Count)=5;
    }
  }
}

/* Move and adjust the size of the slider */ 
void __regargs MoveSlider(BufStruct *Storage)
{
  if (BUF(window) && BUF(window)->window_pointer && BUF(window->slider)!=sl_OFF &&
      BUF(window)->Visible==VISIBLE_ON) {
    if (Visible==VISIBLE_ON) {
      int temp;
      register int vertpot;
      register int vertbody;         /* VertBody */

      temp=SHS(line)-BUF(curr_topline)-BUF(screen_lines)+1;
      if (temp<0)
        temp=SHS(line)-temp;
      else
        temp=SHS(line);
      if (temp > BUF(screen_lines)) {
        vertbody=MultML(MAXBODY, BUF(screen_lines), temp);
        vertpot=((temp=SHS(line)-BUF(screen_lines)) < BUF(curr_topline)?MAXBODY:
    		  MultML(MAXBODY,(BUF(curr_topline)-1),temp));
      } else {
        vertbody=MAXBODY;
        vertpot=NULL;
      }
      temp=AUTOKNOB|FREEVERT;
      if ((BUF(window)->window&FX_WINDOWBIT) && BUF(window->slider)==sl_RIGHT)
        temp=AUTOKNOB|FREEVERT|PROPNEWLOOK|PROPBORDERLESS;
      NewModifyProp(&BUF(slide.Slider), BUF(window->window_pointer), NULL, temp,
    		  NULL,
    		  vertpot,
    		  0,//((Default.WindowDefault.window==FX_WINDOW)?(MAXBODY-(MAXBODY>>2)):MAXBODY),
    		  vertbody,
    		  1L);	
    } else
      UpDtNeeded|=UD_MOVE_SLIDER;
  }
}
