/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/***********************************************
 *
 *  Init.c
 *
 *  Init routines.
 *
 ***********/

#include <devices/console.h>
#include <dos/dos.h>
#include <exec/execbase.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>
#undef GetOutlinePen
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <libraries/dos.h>
#include <libraries/reqtools.h>
#include <libraries/locale.h>
#include <math.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/reqtools.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "BufControl.h"
#include "Command.h"
#include "FACT.h"
#include "GetFont.h"
#include "Init.h"
#include "KeyAssign.h"
#include "Limit.h"
#include "OpenClose.h"
#include "RawKeys.h"
#include "Search.h" /* Search-flaggorna måste vi ju ha... */
#include "Slider.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"

/********** Globals **********/

extern char GlobalEmptyString[];


extern errno;
extern char *sys_errlist[];

extern struct Gadget VertSlider;
extern struct PropInfo VertSliderSInfo;
extern struct Image SliderImage;
extern struct IOStdReq *WriteReq;
extern struct IntuitionBase *IntuitionBase;
extern char buffer[];	/* temp buffer */
extern int bufferlen;
extern DefaultStruct Default; /* the default values of a BufStruct */
extern struct TextFont *SystemFont;
extern struct TextFont *RequestFont;
extern int systemfont_leftmarg;
extern int systemfont_rightmarg;
extern int BarHeight;
extern int cl_window, cl_screen, cl_copywb, cl_backdrop;
extern char *cl_pubscreen, *cl_startupfile;
extern char *cl_diskname;
extern struct TextAttr topazfont;
extern struct screen_buffer ScreenBuffer;
extern long screen_buffer_size;
extern FrexxBorder borderdef;
extern struct Catalog *catalog;

extern char *statusbuffer;
extern long statusbuffer_len;

extern char *KeyMapRemember;

extern char *sizetext[4];
extern char *slidertext[4];
extern char *windowtext[5];
extern char *windowpostext[3];
extern char *blockexisttext[4];
extern char *expandpathtext[4];
extern char *saveicontext[4];

extern int Visible;
extern int UpDtNeeded;
extern char *version;
extern int version_int;
extern BOOL quitting;

extern char DebugOpt; /* Debug option on/off */


#define RSA_FULL 0x55aa1235
#define RSA_SUB  0xe29affed

/***********************************************
 *
 *  InitDefaultBuf()
 *
 *  Init for the Default.BufStructDefault.
 *
 ***********/
void InitDefaultBuf()
{

  VertSlider.GadgetRender= (APTR)&SliderImage;
  VertSlider.SpecialInfo= (APTR)&VertSliderSInfo;
  Default.BufStructDefault.tabsize=8;
  Default.BufStructDefault.rightmarg=8;
  Default.BufStructDefault.leftmarg=8;
  Default.BufStructDefault.uppermarg=4;
  Default.BufStructDefault.lowermarg=4;
  Default.BufStructDefault.block_exists=be_NONE;
  Default.BufStructDefault.shared=&Default.SharedDefault;
  Default.BufStructDefault.reg.reg=RSA_FULL-RSA_SUB;	// Justeras även senare
  Default.l_c_len_store=5;
  Default.SharedDefault.Undotop=-1;
  Default.SharedDefault.date.ds_Days=-1;
  Default.SharedDefault.type=type_FILE;
  Default.SharedDefault.pack_type[0]='\0';
  Default.SharedDefault.password[0]='\0';
  Default.SharedDefault.comment_begin=Strdup("//");


  Default.SharedDefault.Entry=&Default.BufStructDefault;
  Default.BufStructDefault.window=&Default.WindowDefault;
  strcpy(Default.SharedDefault.protection, "rwd");
  Default.SharedDefault.fileprotection=S_IEXECUTE;
  Default.full_size_buf=acHALF_SIZE;
//  Default.Slider=VertSlider;
//  Default.SliderSInfo=VertSliderSInfo;
//  Default.SliderImage=SliderImage;
//  Default.BufStructDefault.slide.Slider=Default.Slider;
//  Default.BufStructDefault.slide.SliderSInfo=Default.SliderSInfo;
//  Default.BufStructDefault.slide.SliderImage=Default.SliderImage;
//  memcpy(&Default.Slider,&VertSlider,sizeof(VertSlider));
//  memcpy(&Default.SliderSInfo,&VertSliderSInfo,sizeof(VertSliderSInfo));
//  memcpy(&Default.SliderImage,&SliderImage,sizeof(SliderImage));
  memcpy(&Default.BufStructDefault.slide.Slider,&VertSlider,sizeof(VertSlider));
  memcpy(&Default.BufStructDefault.slide.SliderSInfo,&VertSliderSInfo,sizeof(VertSliderSInfo));
  memcpy(&Default.BufStructDefault.slide.SliderImage,&SliderImage,sizeof(SliderImage));
  memcpy(&Default.BufStructDefault.slide.Border,&borderdef,sizeof(FrexxBorder));

  Default.BufStructDefault.reg.reg+=RSA_SUB;	// Justeras även senare
  Default.WindowDefault.slider=sl_RIGHT;

  Default.WindowDefault.DisplayID=HIRES_KEY;
  Default.WindowDefault.OverscanType=OSCAN_TEXT;
  Default.WindowDefault.appwindow=TRUE;
  Default.WindowDefault.autoresize=TRUE;
  Default.WindowDefault.autoscroll=TRUE;
  Default.WindowDefault.screen_width=640;
  Default.WindowDefault.window_width=640;
  Default.WindowDefault.screen_height=200;
  Default.WindowDefault.window_height=200;
  Default.WindowDefault.window_position=WINDOW_VISIBLE;


  Default.fragmentmax=30;
  Default.fragmentmemmax=600;
  Default.BufStructDefault.move_screen=8;
/*  Default.screen_height=256;  Defined in FirstInit
    Default.screen_width=640;
*/
  Default.defaultfile=Strdup("FrexxEd:FPL/FrexxEd.default");
  Default.StartupFile=Strdup("FrexxEd.FPL");

  Default.BufStructDefault.reg.r.reg2|=0x0211;	// Krypteringsövning
  Default.autosaveintervall=100;

  Default.BufStructDefault.curr_topline=1;
  Default.BufStructDefault.cursor_x=1;
  Default.BufStructDefault.cursor_y=1;
  Default.BufStructDefault.curr_line=1;
  Default.BufStructDefault.top_offset=1;

  Default.SharedDefault.shared=1;
  Default.SharedDefault.SaveInfo=TRUE;
  Default.SharedDefault.Undo=TRUE;
  Default.SharedDefault.freeable=TRUE;

  Default.rwed_sensitive=TRUE;

  Default.safesave=TRUE;
  Default.appicon=TRUE;
  Default.fold_save=TRUE;

  Default.BufStructDefault.Flag.insert_mode=TRUE;

  Default.unpack = TRUE;
  Default.filehandler=TRUE;

  Default.search_prompt=1;
  Default.search_forward=1;
  Default.search_limit=1;
  Default.WindowDefault.block_pen=2;
  Default.WindowDefault.cursor_pen=3;

  memcpy(&Default.font, &topazfont, sizeof(struct TextAttr));
  memcpy(&Default.RequestFontAttr, &topazfont, sizeof(struct TextAttr));
  Default.font.ta_Name= Strdup(topazfont.ta_Name);
  Default.RequestFontAttr.ta_Name= Strdup(topazfont.ta_Name);


  Default.Undo_maxline=200;
  Default.Undo_maxmemory=100000;

  Default.SharedDefault.Entry=&Default.BufStructDefault;

  Default.language=catalog?catalog->cat_Language:"english";

  Default.WindowDefault.colour_status_info=2;
  Default.WindowDefault.move_screen=8;

  Default.version_int=version_int;
  Default.version_string=version;

  Default.status_line=Strdup("$f $L:$l$C:$c");
  Default.olddirectory=-1;

  Default.page_length=66;

  Default.FPLdirectory=Strdup("|FrexxEd:FPL/");

  Default.BufStructDefault.reg.r.reg2|=0x1024;	// Krypteringsövning

  Default.SystemFont=GlobalEmptyString;
  Default.RequestFont=GlobalEmptyString;
  Default.file_pattern=GlobalEmptyString;
  Default.WindowDefault.PubScreen=GlobalEmptyString;
  Default.WindowDefault.FrexxScreenName=GlobalEmptyString;
  Default.KeyMap=GlobalEmptyString;
  Default.directory=GlobalEmptyString;
  Default.WindowDefault.window_title=GlobalEmptyString;
  Default.SharedDefault.filnamn=GlobalEmptyString;
  Default.SharedDefault.path=GlobalEmptyString;
  Default.SharedDefault.macrokey=GlobalEmptyString;
  Default.SharedDefault.comment_end=GlobalEmptyString;
  Default.SharedDefault.face_name=GlobalEmptyString;
  Default.BufStructDefault.fact_name=GlobalEmptyString;


//  Default.color0=0;
//  Default.color1=0;
//  Default.color2=0;
//  Default.color3=0;
//  Default.BufStructDefault.screen_x=0;
//  Default.BufStructDefault.wanted_x=0;
//  Default.BufStructDefault.string_pos=0;
//  Default.BufStructDefault.NextShowBuf=NULL;
//  Default.BufStructDefault.PrevShowBuf=NULL;
//  Default.BufStructDefault.NextBuf=NULL;
//  Default.BufStructDefault.PrevBuf=NULL;
//  Default.BufStructDefault.NextHiddenBuf=NULL;
//  Default.BufStructDefault.PrevHiddenBuf=NULL;
//  Default.BufStructDefault.NextSplitBuf=NULL;
//  Default.BufStructDefault.PrevSplitBuf=NULL;
//  Default.BufStructDefault.view_number=0;
//  Default.BufStructDefault.l_c_len=0;
//  Default.BufStructDefault.left_offset=0;
//  Default.BufStructDefault.namedisplayed=FALSE;
//  Default.BufStructDefault.expunge=FALSE;
//  Default.BufStructDefault.update=FALSE;
//  Default.BufStructDefault.visible=NULL;
//  Default.BufStructDefault.locked=0;
//  Default.SharedDefault.text=NULL;
//  Default.SharedDefault.length=NULL;
//  Default.SharedDefault.line=0;
//  Default.SharedDefault.UndoBuf=NULL;
//  Default.SharedDefault.Undopos=0;
//  Default.SharedDefault.Undomax=0;
//  Default.SharedDefault.Undomem=0;
//  Default.SharedDefault.Undostrpos=0;
//  Default.SharedDefault.changes=0;
//  Default.SharedDefault.taket=0;
//  Default.SharedDefault.name_number=0;
//  Default.SharedDefault.readlock=0;
//  Default.SharedDefault.writelock=NULL;
//  Default.SharedDefault.Locks=NULL;
//  Default.SharedDefault.size=0;
//  Default.SharedDefault.fileblock=NULL;
//  Default.SharedDefault.fileblocklen=0;
//  Default.SharedDefault.fragment=0;
//  Default.SharedDefault.fragmentlen=0;
//  Default.SharedDefault.autosave=FALSE;
//  Default.SharedDefault.asenabled=0;
//  Default.SharedDefault.update=FALSE;
//  Default.SharedDefault.Next=NULL;
//  Default.SharedDefault.Prev=NULL;
//  Default.SharedDefault.LokalInfo=NULL;
//  Default.workstring=NULL;
//  Default.worklength=0;
//  Default.worklengthmax=0;
//  Default.storeworkstring=NULL;
//  Default.storeworklength=NULL;
//  Default.oldShared=NULL;
//  Default.BufStructDefault.Border=NULL;
//  Default.RMB=FALSE;
//  Default.FirstBlock=NULL;
//  Default.workstring=NULL;
//  Default.worklength=0;
//  Default.worklengthmax=0;
//  Default.storeworkstring=NULL;
//  Default.storeworklength=NULL;
//  Default.oldShared=NULL;
//  Default.showpath=FALSE;
//  Default.GlobalInfo=NULL;
//  Default.globalinfoalloc=0;
//  Default.lokalinfoalloc=0;
//  Default.globalinfoantal=0;
//  Default.lokalinfoantal=0;
//  Default.KeyMapseg=NULL;
//  Default.full_path=FALSE;
//  Default.BufStructDefault.Flag.l_c=FALSE;
//  Default.screen_depth=0;
//  Default.window_xpos=0;
//  Default.window_ypos=0;
//  Default.window=FX_SCREEN;
//  Default.search_wildcard=0;
//  Default.search_case=0;
//  Default.search_word=0;
//  Default.search_block=0;
//  Default.FPLdebug=0;
//  Default.iconify=0;
//  Default.fold_margin=0;
//  Default.page_overhead=0;
//  Default.WindowDefault.graphic_card=0;
}

void SecondInit(void)
{
  if (cl_screen)
    Default.WindowDefault.window=FX_SCREEN;
  else if (cl_window)
    Default.WindowDefault.window=FX_WINDOW;
  else if (cl_backdrop)
    Default.WindowDefault.window=FX_BACKDROP;
  if (cl_pubscreen) {
    Dealloc(Default.WindowDefault.PubScreen);
    Default.WindowDefault.PubScreen=cl_pubscreen;
  }
  if (cl_startupfile) {
    Dealloc(Default.StartupFile);
    Default.StartupFile=cl_startupfile;
  }
  if (cl_diskname) {
    Dealloc(Default.diskname);
    Default.diskname=cl_diskname;
  }

  /* init texts */

  sizetext[0] = RetString(STR_REPLACE_GADGET);
  sizetext[1] = RetString(STR_HALF_GADGET);
  sizetext[2] = RetString(STR_FULL_GADGET);
  sizetext[3] = RetString(STR_NEW_WINDOW);
//  sizetext[4] = NULL;

  slidertext[0] = RetString(STR_NONE_GADGET);
  slidertext[1] = RetString(STR_RIGHT_GADGET);
  slidertext[2] = RetString(STR_LEFT_GADGET);
//  slidertext[3] = NULL;

  windowtext[0] = RetString(STR_SCREEN_GADGET);
  windowtext[1] = RetString(STR_WINDOW_GADGET);
  windowtext[2] = RetString(STR_BACKDROP_GADGET);
  windowtext[3] = RetString(STR_WINSCREEN_GADGET);
//  windowtext[4] = NULL;

  windowpostext[0] = RetString(STR_VISIBLE_GADGET);
  windowpostext[1] = RetString(STR_ABSOLUTE_GADGET);
//  windowpostext[2] = NULL;

  blockexisttext[0] = RetString(STR_OFF_GADGET);
  blockexisttext[1] = RetString(STR_MARKING_GADGET);
  blockexisttext[2] = RetString(STR_EXIST_GADGET);
//  blockexisttext[3] = NULL;

  expandpathtext[0] = RetString(STR_OFF_GADGET);
  expandpathtext[1] = RetString(STR_EXPAND_RELATIVE_GADGET);
  expandpathtext[2] = RetString(STR_EXPAND_ALL_GADGET);
//  expandpathtext[3] = NULL;

  saveicontext[0] = RetString(STR_ICON_NEVER); 
  saveicontext[1] = RetString(STR_ICON_ALWAYS);
  saveicontext[2] = RetString(STR_ICON_IF_PARENT);
//  saveicontext[3] = NULL;
}

void __regargs InitWindow(WindowStruct *win)
{
  register BufStruct *Storage=&Default.BufStructDefault;

  BUF(screen_lines) = win->real_window_height;
  BUF(screen_col) = win->real_window_width;
  BUF(left_offset)=systemfont_leftmarg;
  if (win->window_pointer) {
    win->text_start=win->window_pointer->BorderTop;
    if (win->window==FX_SCREEN)
      win->text_start=win->screen_pointer->BarHeight+1;
      
    BUF(screen_lines) -= win->text_start-1;
    if (win->slider==sl_LEFT || !(win->window&FX_WINDOWBIT)) {
//      BUF(screen_col)-=win->window_pointer->BorderLeft; // border width
      if (win->slider==sl_LEFT)
        BUF(left_offset)+=16; // slider width
      else
        BUF(screen_col)-=16; // slider width
    }
    BUF(screen_lines) = BUF(screen_lines)-win->window_pointer->BorderBottom-1;
    BUF(left_offset)+=win->window_pointer->BorderLeft;
/*
    BUF(screen_col) +=-win->window_pointer->BorderLeft-win->window_pointer->BorderRight-
                      SystemFont->tf_BoldSmear*1-SystemFont->tf_XSize/2;
*/
    BUF(screen_col) +=-BUF(left_offset)-win->window_pointer->BorderRight -
                       systemfont_rightmarg;
//    win->window_right_offset=win->window_pointer->Width - win->window_pointer->BorderRight-1;
    win->window_right_offset=BUF(screen_col)+BUF(left_offset)+systemfont_rightmarg-1;
    BUF(screen_lines) = ((BUF(screen_lines))/SystemFont->tf_YSize)-1;
    BUF(screen_col) = (BUF(screen_col))/SystemFont->tf_XSize;
    if (BUF(screen_col)>MAX_CHAR_LINE)
      BUF(screen_col)=MAX_CHAR_LINE;
    win->window_lines=BUF(screen_lines);
    win->window_col=BUF(screen_col);
    win->window_left_offset=BUF(left_offset);
  } else {
    win->window_lines=win->window_height/SystemFont->tf_YSize;
    BUF(screen_lines)=win->window_lines;
    win->window_col=win->window_width/SystemFont->tf_XSize;
    BUF(screen_col)=win->window_col;
  }
  {
    register struct EndPosStruct *mem;
    mem=(struct EndPosStruct *)Remalloc((char *)win->UpdtVars.EndPos, sizeof(struct EndPosStruct)*(win->window_lines+2));
    if (mem)
      win->UpdtVars.EndPos=mem;
    else
      ;	// OBS här måste det hittas minne
  }
  {
    register char *mem;
    register int size=((win->window_lines+1)*((BUF(screen_col)<7?7:BUF(screen_col)+1))+10);
    if (screen_buffer_size<10*size) { /* 10=storleken av det screen_buffer behöver peka på */
      screen_buffer_size=10*size;
      mem=(char *)Remalloc(ScreenBuffer.beginning, screen_buffer_size);
      if (mem) {
        ScreenBuffer.beginning=mem;
        ScreenBuffer.text=ScreenBuffer.beginning;
        ScreenBuffer.control=&ScreenBuffer.text[size];
        ScreenBuffer.colors=(short *)&ScreenBuffer.control[size];
        ScreenBuffer.linebuffer=(int *)&ScreenBuffer.colors[size];
      } else
        ;	// OBS här måste det hittas minne
    }
  }
  {
    register char *mem;
    if (win->window_col>statusbuffer_len) {
      statusbuffer_len=win->window_col;
      mem=(char *)Remalloc(statusbuffer, statusbuffer_len+40);
      if (mem)
        statusbuffer=mem;
      else
        ;	// OBS här måste det hittas minne
    }
  }
  {
    BufStruct *Storage;
    if (win) {
      PrepareRedrawOfWindow(win);  // Make sure all window is redrawn next time.
      Storage=win->NextShowBuf;
      while (Storage) {
        BufLimits(Storage);
        PositionSlider(Storage);
        Storage=BUF(NextShowBuf);
      }
    }
  }
}

void __regargs PrepareRedrawOfWindow(WindowStruct *win)
{
/* Filling the EndPos table for the window to make sure everything is
   redrawn next time */
  int counter;
  for (counter=0; counter<=(win->window_lines); counter++) {
    win->UpdtVars.EndPos[counter].len=win->window_col;
    win->UpdtVars.EndPos[counter].blockstart=-1;
    win->UpdtVars.EndPos[counter].bitplanes=(unsigned char)-1;
    win->UpdtVars.EndPos[counter].eraseline=1;
  }
}

void InitKeys()
{
  Default.key.mul=NULL;

  /********************************************************
                 Default keymap coming up
   *******************************************************

      Amiga|Alt|Ctrl|Shift, Key, Command        */

  AddKey(akFPL|akDEFAULT, 0, NULL,"\r", (int)"Output(\"\\n\");",	&Default.key, NULL);
#ifdef test
  AddKey(akCMD|akDEFAULT, RAW_AMIGA, NULL,"0",	DO_SLASK,	&Default.key, NULL);
  AddKey(akCMD|akDEFAULT, RAW_AMIGA, NULL,"9",	DO_SLASK2,	&Default.key, NULL);
#endif

  AddKey(akCMD|akDEFAULT, 0, NULL,"\b",	DO_BACKSPACE,	&Default.key, NULL);		
  AddKey(akCMD|akDEFAULT, 0, NULL,"\x7f",	DO_DELETE,	&Default.key, NULL);
  AddKey(akCMD|akDEFAULT, 0, NULL,"\x9B""A",	DO_CURSOR_UP,	&Default.key, NULL);
  AddKey(akCMD|akDEFAULT, 0, NULL,"\x9B""B",	DO_CURSOR_DOWN,	&Default.key, NULL);
  AddKey(akCMD|akDEFAULT, 0, NULL,"\x9B""C", DO_CURSOR_RIGHT,&Default.key, NULL);
  AddKey(akCMD|akDEFAULT, 0, NULL,"\x9B""D",	DO_CURSOR_LEFT,	&Default.key, NULL);
  AddKey(akCMD|akDEFAULT, 0, NULL,"\x9B""S",	DO_PAGE_DOWN,	&Default.key, NULL);
  AddKey(akCMD|akDEFAULT, 0, NULL,"\x9B""T",	DO_PAGE_UP,	&Default.key, NULL);
  AddKey(akCMD|akDEFAULT, RAW_CTRL, NULL,"\x9B""C",	DO_RIGHT_WORD,	&Default.key, NULL);
  AddKey(akCMD|akDEFAULT, RAW_CTRL, NULL,"\x9B""D",	DO_LEFT_WORD,	&Default.key, NULL);

							/******************/
}

