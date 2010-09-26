/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * GetFont.c
 *
 * Change the system font.
 *
 *********/

#include <devices/console.h>
#include <dos/dos.h>
#include <exec/io.h>
#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <libraries/diskfont.h>
#include <libraries/dos.h>
#include <libraries/reqtools.h>
#include <prefs/font.h>
#include <prefs/prefhdr.h>
#include <proto/diskfont.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/FPL.h>
#include <proto/graphics.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>
#include <proto/reqtools.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "BufControl.h"
#include "BuildMenu.h"
#include "Change.h"
#include "Command.h"
#include "Cursor.h"
#include "GetFont.h"
#include "IDCMP.h"
#include "Init.h"
#include "Limit.h"
#include "Request.h"
#include "Slider.h"
#include "OpenClose.h"
#include "UpdtScreen.h"
#include "WindowOutput.h"
#ifdef USE_FASTSCROLL
#include "fastGraphics.h"
#include "fastGraphics_pragma.h"
#include "fastGraphics_proto.h"
#endif

extern struct Library *FastGraphicsBase;
extern struct IntuitionBase *IntuitionBase;
extern char buffer[];
extern int Visible;
extern DefaultStruct Default;
extern struct TextAttr topazfont;
extern struct IOStdReq *WriteReq;
extern struct ConsoleDevice *ConsoleDevice;
extern struct MsgPort *WritePort;
extern int BarHeight;
extern struct RastPort ButtonRastPort;	//Rastport för att kunna testa med Requestfonten.
extern struct FastGraphicsTable *fast_gfx_table;

#define CURSOROFF "\x9b\x30\x20\x70"
#define CURSORON  "\x9b\x20\x70"

extern struct TextFont *SystemFont;	/* Font used by the system/screen */
extern struct TextFont *RequestFont;	/* Font used by requsters */
extern struct TextFont *DefaultFont;	/* Font used by requsters */
extern int requestfontwidth;		// Kalkulerad bredd på requestfonten.
extern BOOL match_for_fastscroll;
extern int systemfont_leftmarg;
extern int systemfont_rightmarg;

/***************************************************
 *
 *  GetFont(BufStruct *, char *font, int flag)
 *
 *  Will make (char *) to the system/request font.
 *  Font shall be written like "topaz 8"
 *  Set 'flag' to '1' if you want to change the systemfont, 
 *  '2' for the requestfont and '3' for both.
 *  If no BufStruct is given, no change will be done on the window.
 *
 *  Return a ret value.
 ******/
int __regargs GetFont(BufStruct *Storage, char *fullfontname, int flag)
{

  struct TextFont *newfont;
  struct TextAttr font;
  int ret=OK;
  int tempvisible;
  char *slask;
  char *fontname;
  WindowStruct *win;

  win=FRONTWINDOW;
  if (Storage && BUF(window))
    win=BUF(window);

  ret=-(int)RetString(STR_CANT_INSTALL_FONT);

  if (!FRONTWINDOW || !fullfontname || !fullfontname[0])
    return(FUNCTION_CANCEL);

  {
    int count=strlen(fullfontname)-1;
    while (fullfontname[count]==' ')
      count--;
    while (fullfontname[count]!=' ')
      count--;
    font.ta_YSize=fplStrtol(&fullfontname[count+1], 10, &slask);
    if (!font.ta_YSize)
      return(FUNCTION_CANCEL);
    if (font.ta_YSize<1)  // Tillåt inte size 0 på fonten
      return ret;
    fontname=Malloc(count+1);
    if (!fontname)
      return(OUT_OF_MEM);
    stccpy(fontname, fullfontname, count+1);
//    fontname[count]=0;
    font.ta_Name=fontname;
    font.ta_Style=0;
  }
  if (Storage)
    CursorXY(win->NextShowBuf, -1, -1);

  font.ta_Flags=FPF_DESIGNED;
  newfont=OpenFont(&font);
  if (!newfont || newfont->tf_YSize!=font.ta_YSize) {
    if (newfont)
      CloseFont(newfont);
    font.ta_Flags=0;
    if (!(newfont=OpenDiskFont(&font)))
      newfont=OpenFont(&font);
  }
  if (newfont &&
      (newfont->tf_YSize<win->screen_height>>3) &&
      (newfont->tf_XSize<win->screen_width>>3)) {
    if (flag & 2) {
      RequestFont=newfont;
      Dealloc(Default.RequestFontAttr.ta_Name);
      memcpy(&Default.RequestFontAttr, &font, sizeof(struct TextAttr));
      Default.RequestFontAttr.ta_Name=Strdup(Default.RequestFontAttr.ta_Name);
      if (fullfontname!=Default.RequestFont) {
        Dealloc(Default.RequestFont);
        Default.RequestFont=Strdup(fullfontname);
      }
      if (Storage) {
        {
          WindowStruct *wincount;
          wincount=FRONTWINDOW;
          while (wincount) {
            menu_detach(&menu, wincount);
            menu_attach(wincount);
            wincount=wincount->next;
          }
        }
      }
      {
        register int counter;
        for (counter=0; counter<25; counter++) {
          buffer[counter]=counter+'A';
          buffer[counter+25]=counter+'a';
//          buffer[counter+50]=counter+32;
        }
        SetFont(&ButtonRastPort, RequestFont);
        SetSoftStyle(&ButtonRastPort, 0, FSF_UNDERLINED|FSF_BOLD|FSF_ITALIC);
        counter=TextLength(&ButtonRastPort, buffer, 50);
        requestfontwidth=(counter+49)/50;
      }
      ret=(int)RetString(STR_NEW_FONT_INSTALLED);
    }
    if (flag & 1) {
      if (!(newfont->tf_Flags & FPF_PROPORTIONAL)) {
        if (DefaultFont)
          CloseFont(DefaultFont);
        DefaultFont=newfont;
        Dealloc(Default.font.ta_Name);
        memcpy(&Default.font, &font, sizeof(struct TextAttr));
        Default.font.ta_Name= Strdup(font.ta_Name);
        SystemFont=newfont;
        if (fullfontname!=Default.SystemFont) {
          Dealloc(Default.SystemFont);
          Default.SystemFont=Strdup(fullfontname);
        }
        {
          struct TextExtent textextent;
          SetFont(&ButtonRastPort, SystemFont);
          SetSoftStyle(&ButtonRastPort, FSF_UNDERLINED|FSF_BOLD|FSF_ITALIC, FSF_UNDERLINED|FSF_BOLD|FSF_ITALIC);
          TextExtent(&ButtonRastPort, "A", 1, &textextent);
          systemfont_leftmarg=-textextent.te_Extent.MinX;
          systemfont_rightmarg=textextent.te_Extent.MaxX-SystemFont->tf_XSize+1;
        }
        win=FRONTWINDOW;
        while (win) {
          win->window_minheight=SystemFont->tf_YSize*3+BarHeight;
          win->window_minwidth=SystemFont->tf_XSize*5;
          ClearWindow(win);
#ifdef USE_FASTSCROLL
          if (FastGraphicsBase) {
            if (match_for_fastscroll) {
              if (fast_gfx_table) {
                fgFree(fast_gfx_table);
                fast_gfx_table=NULL;
              }
            }
            match_for_fastscroll=FALSE;
            if (newfont->tf_YSize==8 && newfont->tf_XSize==8) {
  //            SetFont(&ButtonRastPort, SystemFont);
  //            fast_gfx_table=fgAlloc(&ButtonRastPort);
              match_for_fastscroll=TRUE;
            }
          }
#endif
          if (Storage) {
            if (win->window_pointer)
              SetFont(win->window_pointer->RPort, SystemFont);
            ActivateEditor();
            InitWindow(win);
            PrintScreenInit();
            win->window_height++;
            ReSizeWindow(Storage);
            PrintScreenInit();
            Storage=win->NextShowBuf;
            tempvisible=Visible;
            Visible=VISIBLE_OFF;
            do {
              ReSizeBuf(Storage, Storage, NULL, BUF(screen_lines));
              FixMarg(Storage);
              InitSlider(Storage);
              Storage=BUF(NextShowBuf);
            } while(Storage);
            if (win->window_pointer)
              RefreshGList(win->window_pointer->FirstGadget, win->window_pointer, NULL, -1);
            Storage=win->NextShowBuf;
            Visible=tempvisible;
          }
          SetupMinWindow(win);
          win=win->next;
        }
        UpdateAll();
        ShowAllstat();
        ret=(int)RetString(STR_NEW_FONT_INSTALLED);
      }
    }
    ActivateEditor();
  }

  Dealloc(fontname);
  return(ret);
}

void __regargs SetSystemFont()
{
  struct IFFHandle *iffhandle;
  struct StoredProperty *sp;
  struct ContextNode *cn;
  LONG ifferror;
  int foundcount=0;

  if (IFFParseBase=OpenLibrary("iffparse.library", 0L)) {
    if (iffhandle=AllocIFF()) {
      if (iffhandle->iff_Stream=(LONG)Open("env:sys/font.prefs", MODE_OLDFILE)) {
        InitIFFasDOS(iffhandle);
        PropChunk(iffhandle, ID_PREF, ID_FONT);
        if (OpenIFF(iffhandle, IFFF_READ)==0) {
          while (1) {
            
            ifferror=ParseIFF(iffhandle, IFFPARSE_STEP);
            if (ifferror!=IFFERR_EOC) {
              if (!ifferror) {
                cn=CurrentChunk(iffhandle);
                if (cn->cn_ID!=ID_PRHD && cn->cn_ID!=ID_FORM) {
                  sp=FindProp(iffhandle, ID_PREF, ID_FONT);
                  if (sp) {
                    Sprintf(buffer, "%s %ld", ((struct FontPrefs *)sp->sp_Data)->fp_Name, ((struct FontPrefs *)sp->sp_Data)->fp_TextAttr.ta_YSize);
                    if (foundcount>=1)
                      GetFont(NULL, buffer, foundcount);
                    foundcount++;
                    if (foundcount==3)
                      break;	// End loop
                  }
                }
              } else
                break;	// End loop
            }
          }
        }
        Close(iffhandle->iff_Stream);
      }
      FreeIFF(iffhandle);
    }
    CloseLibrary(IFFParseBase);
    IFFParseBase=NULL;
  }
}


