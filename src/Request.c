/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Request.c
 *
 * Commmunicate with the user. One-way and both ways...
 *
 *********/

#ifdef AMIGA
#include <clib/console_protos.h>
#include <clib/gadtools_protos.h>
#include <clib/reqtools_protos.h>
#include <devices/console.h>
#include <exec/execbase.h>
#include <exec/semaphores.h>
#include <exec/io.h>
#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <graphics/gfxmacros.h>
#undef GetOutlinePen
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/reqtools.h>
#ifndef __AROS__
#include <inline/reqtools_protos.h>
#endif
#include <proto/reqtools.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/diskfont.h>
#include <libraries/reqtools.h>
#else
#include "compat.h"
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "RawKeys.h"
#include "Reqlist.h"
#include "Alloc.h"
#include "BufControl.h"
#include "Button.h"
#include "Change.h"
#include "Command.h"
#include "Cursor.h"
#include "GetFile.h"
#include "IDCMP.h"
#include "Init.h"
#include "OpenClose.h"
#include "Request.h"
#include "Setting.h"
#include "Sort.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"
#include "Winsign.h"
#include "WindowOutput.h"

#if 0	// We use reqtools instead.
static UWORD chip clock[36] = {
    0x0000, 0x0000,    0x0400, 0x07c0,    0x0000, 0x07c0,    0x0100, 0x0380,    0x0000, 0x07e0,    0x07c0, 0x1ff8,    0x1ff0, 0x3fec,    0x3ff8, 0x7fde,    0x3ff8, 0x7fbe,    0x7ffc, 0xff7f,    0x7efc, 0xffff,    0x7ffc, 0xffff,    0x3ff8, 0x7ffe,    0x3ff8, 0xfffe,    0x1ff0, 0x3ffc,    0x07c0, 0x1ff8,    0x0000, 0x07e0,    0x0000, 0x0000
};
#endif

extern struct ExecBase *SysBase;
extern UpdateStruct UpdtVars;
extern struct IntuitionBase *IntuitionBase;
extern struct ConsoleDevice *ConsoleDevice;
extern struct MsgPort *ReadPort;
extern DefaultStruct Default;
extern char CursorOnOff;
extern struct TextFont *RequestFont;
extern int Visible;
extern struct rtFileRequester *FileReq;
extern struct ReqToolsBase *ReqToolsBase;
extern BOOL OwnWindow;
extern int radmodulo;
extern struct TextFont *SystemFont;	/* Font used by the system/screen */
extern int UpDtNeeded;
extern char buffer[];
extern char waitreq;
extern struct SignalSemaphore LockSemaphore;

/**********************************************************************
 *
 * int PromptFile(BufStruct *, char *, char *)
 *
 * Returns TRUE if the user selected a file name. The entire path
 * is stored in the buffer (char *) points to, this buffer is also
 * used to set the defaultname and path.
 * If a Storage is given the path will be found there.
 * The flags shall be in reqtools filerequester style.
 *
 *****/
int __regargs PromptFile(BufStruct *Storage, char *path, char *header, char *patter, int flags, APTR *liststore)
{
  char *filename=buffer;
  int ret;
  struct rtFileList *filelist;

  FreezeEditor(0);
  LockBuf_release_semaphore(Storage?BUF(shared):NULL);
  rtChangeReqAttr(FileReq, RTFI_MatchPat, patter?patter:Default.file_pattern, TAG_END);
  if (Storage && !strchr(path,':') && !strchr(path,'/')) {
    if (strcmp(FileReq->Dir, SHS(path))) {
      rtChangeReqAttr(FileReq, RTFI_Dir, SHS(path), TAG_END);
    }
  } else {
    Stcgfp(filename, path);
    if (strcmp(FileReq->Dir, filename)) {
      rtChangeReqAttr(FileReq, RTFI_Dir, filename, TAG_END);
    }
  }
  Stcgfn(filename, path);
  {
    struct Screen *screen=NULL;
    if (!FRONTWINDOW || !FRONTWINDOW->window_pointer)
      screen=LockPubScreen(NULL);
    filelist=rtFileRequest(FileReq, filename, header?header:RetString(STR_FILE_NAME),
                      RTFI_Flags, FREQF_PATGAD|flags,
                      !screen?RT_Window:TAG_IGNORE, FRONTWINDOW->window_pointer,
		      !screen?RT_WaitPointer:TAG_IGNORE, OwnWindow,
                      screen?RT_Screen:TAG_IGNORE, screen,
                      RT_TextAttr, &Default.RequestFontAttr,
                      TAG_END);
    if (screen)
      UnlockPubScreen(NULL, screen);  // screen that we opened requester at.
  }
  if (filelist) {
    if (!patter) {
      Dealloc(Default.file_pattern);
      Default.file_pattern=Strdup(FileReq->MatchPat);
    }
    if (flags&FREQF_MULTISELECT) {
      *liststore=filelist;
      strcpy(path, FileReq->Dir);
    } else
      strmfp(path, FileReq->Dir, filename);
    ret=TRUE;
  } else
    ret=FALSE;
  UnLockBuf_obtain_semaphore(Storage?BUF(shared):NULL);
  ActivateEditor();
  return(ret);
}

/********************************************************************
 *
 * int Ok_Cancel(BufStruct *Storage, char *messy, char *title, char *answer)
 *
 * Returns (int) true or false on the question (char *).
 *
 ********/
int __regargs Ok_Cancel(BufStruct *Storage, char *messy, char *title, char *answer)
{
//#define EASY_REQ

  int i;

#ifdef EASY_REQ
  struct EasyStruct easy={sizeof(struct EasyStruct), 0, 0, "%s", "%s"};
  void *asluse=NULL;
#endif
      
  if (!answer)
    answer=RetString(STR_YESNO);
  if (!title)
    title=RetString(STR_FREXXED_REQUEST);
  FreezeEditor(3);
  LockBuf_release_semaphore(Storage?BUF(shared):NULL);
  if (OwnWindow && CursorOnOff==TRUE)
    CursorXY(NULL, -2, 0);

#ifdef EASY_REQ
  if (SysBase->LibNode.lib_Version == 40) {
/*
    Forbid();
    asluse= (void *)FindName(&SysBase->LibList, "egs.library");
    Permit();
  }
  if(asluse) {
*/
    easy.es_Title = title;
    i = (int)EasyRequest(FRONTWINDOW->window_pointer, &easy, NULL, messy, answer);
  } else
#endif
  {
    struct Screen *screen=NULL;
    if (!FRONTWINDOW || !FRONTWINDOW->window_pointer)
      screen=LockPubScreen(NULL);
	i = 0;
    i=(int)rtEZRequest("%s", answer, NULL, &messy, 
					   !screen?RT_Window:TAG_IGNORE, FRONTWINDOW->window_pointer,
			   !screen?RT_WaitPointer:TAG_IGNORE, OwnWindow,
                           screen?RT_Screen:TAG_IGNORE, screen,
			   RT_TextAttr, &Default.RequestFontAttr,
			   RTEZ_ReqTitle, title,
			   TAG_END);
    if (screen)
      UnlockPubScreen(NULL, screen);  // screen that we opened requester at.
  }
  UnLockBuf_obtain_semaphore(Storage?BUF(shared):NULL);
  ActivateEditor();
  return(i);
}

/*************************************************************
 *
 *  FreezeEditor(int mode)
 *
 *  Will make all gadgets inactive.
 *  mode=1, mode=2 busypointer.
 *
 *  mode=3 no sprite.
 *
 ******/
void __regargs FreezeEditor(int mode)
{
  WindowStruct *win=FRONTWINDOW;

  if (OwnWindow) {
    if (!waitreq) {
      while (win) {
        if (win->window_pointer) {
          struct Gadget *gadget=win->window_pointer->FirstGadget;
          while (gadget) {
            if (gadget->GadgetType==GTYP_PROPGADGET) {
              OffGadget(gadget, win->window_pointer, NULL);
            }
            gadget=gadget->NextGadget;
          }
          if (!(win->window&FX_WINDOWBIT))
            RefreshGList(win->window_pointer->FirstGadget, win->window_pointer, NULL, -1);
          ModifyIDCMP(win->window_pointer, IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW|IDCMP_CHANGEWINDOW);
          win->window_lock_id=rtLockWindow(win->window_pointer);
          CursorXY(NULL, -2, 0);
          win->flags|=WINDOW_FLAG_FREEZED;
        }
        win=win->next;
      }
    }
    waitreq++;
  }
}
/*************************************************************
 *
 *  ActivateEditor()
 *
 *  Will make all gadgets active.
 *
 **********/

void ActivateEditor(void)
{
  WindowStruct *win=FRONTWINDOW;

  if (OwnWindow) {
    if (waitreq>0) {
      waitreq--;
      if (waitreq==0) {
        while (win) {
          if (win->flags&WINDOW_FLAG_FREEZED) {
            win->flags&=~WINDOW_FLAG_FREEZED;
            rtUnlockWindow(win->window_pointer, win->window_lock_id);  // Remove due to a bug with rtFontRequest
            CursorXY(NULL, -3, 0);
            ModifyIDCMP(win->window_pointer, IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MENUPICK|IDCMP_GADGETDOWN|
			IDCMP_GADGETUP|IDCMP_MENUVERIFY|IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW|IDCMP_CHANGEWINDOW|IDCMP_CLOSEWINDOW|IDCMP_INTUITICKS);
            {
              struct Gadget *gadget=win->window_pointer->FirstGadget;
            
              while (gadget) {
                if (gadget->GadgetType==GTYP_PROPGADGET) {
                  OnGadget(gadget, win->window_pointer, NULL);
                }
                gadget=gadget->NextGadget;
              }
              if (!(win->window&FX_WINDOWBIT))
                RefreshGList(win->window_pointer->FirstGadget, win->window_pointer, NULL, -1);
            }
          }
          win=win->next;
        }
      }
    }
  }
}

/********************************************************
 *
 * int ScreenModeReq(BufStruct *Storage)
 *
 * Will change the current screenmode, with help of a request.
 *
 * Return a ret-value.
 *****/
int ScreenModeReq(BufStruct *Storage)
{
  struct rtScreenModeRequester *scrmodereq;
  int ret=OK;
  int reopen=NULL;
  struct Screen *screen=NULL;

  if (!BUF(window))
    return FUNCTION_CANCEL;

  if (!FRONTWINDOW || !FRONTWINDOW->window_pointer)
    screen=LockPubScreen(NULL);

  if (scrmodereq = rtAllocRequestA (RT_SCREENMODEREQ, NULL)) {
    LockBuf_release_semaphore(BUF(shared));
    rtChangeReqAttr(scrmodereq,
			RTSC_DisplayID, BUF(window)->DisplayID,
			RTSC_OverscanType, BUF(window)->OverscanType,
			RTSC_DisplayWidth, BUF(window)->screen_width,
			RTSC_DisplayHeight, BUF(window)->screen_height,
			RTSC_DisplayDepth, BUF(window)->screen_depth,
			RTSC_MinDepth, 0,
			RTSC_MaxDepth, 16,
			RTSC_AutoScroll, BUF(window)->autoscroll);
    if (!(rtScreenModeRequest(scrmodereq, RetString(STR_PICK_SCREEN_MODE),
				RTSC_Flags, SCREQF_SIZEGADS|SCREQF_AUTOSCROLLGAD|SCREQF_OVERSCANGAD|SCREQF_DEPTHGAD,
                                !screen?RT_Window:TAG_IGNORE, FRONTWINDOW->window_pointer,
				!screen?RT_WaitPointer:TAG_IGNORE, OwnWindow,
                                screen?RT_Screen:TAG_IGNORE, screen,
				TAG_END))) {
      ret=FUNCTION_CANCEL;
    } else {
      reopen|=Change(Storage, cs_LOCAL, GetSettingName("display_id"), scrmodereq->DisplayID);
      reopen|=Change(Storage, cs_LOCAL, GetSettingName("overscan"), scrmodereq->OverscanType);
      reopen|=Change(Storage, cs_LOCAL, GetSettingName("screen_width"), scrmodereq->DisplayWidth);
      reopen|=Change(Storage, cs_LOCAL, GetSettingName("screen_height"), scrmodereq->DisplayHeight);
      reopen|=Change(Storage, cs_LOCAL, GetSettingName("screen_depth"), scrmodereq->DisplayDepth);
      reopen|=Change(Storage, cs_LOCAL, GetSettingName("autoscroll"), scrmodereq->AutoScroll);
    }
    rtFreeRequest(scrmodereq);
    UnLockBuf_obtain_semaphore(BUF(shared));
    if (reopen && BUF(window)->window_pointer && (BUF(window)->window==FX_SCREEN || BUF(window)->window==FX_WINSCREEN))
      ret=ReDrawSetting(Storage, reopen);
  } else
    ret=OUT_OF_MEM;

  if (screen)
    UnlockPubScreen(NULL, screen);  // screen that we opened requester at.
  return(ret);
}

int __regargs PaletteRequest(BufStruct *Storage)
{
  int ret;
  struct Screen *screen=NULL;
  if (!FRONTWINDOW || !FRONTWINDOW->window_pointer)
    screen=LockPubScreen(NULL);
  LockBuf_release_semaphore(BUF(shared));

  ret=rtPaletteRequest(RetString(STR_MAKE_YOUR_CHOICE), NULL,
                                 !screen?RT_Window:TAG_IGNORE, FRONTWINDOW->window_pointer,
				 !screen?RT_WaitPointer:TAG_IGNORE, OwnWindow,
                                 screen?RT_Screen:TAG_IGNORE, screen,
                                 TAG_END);

  if (screen)
    UnlockPubScreen(NULL, screen);  // screen that we opened requester at.
  UnLockBuf_obtain_semaphore(BUF(shared));

  return ret;
}

/***************************************************
 *
 *  PromptFont(char *reqstring)
 *
 *  Put up a font requester.
 *  Set 'flag' to '1' if you want to change the systemfont, 
 *  '2' for the requestfont and '3' for both.
 *  Return a 'ret'-value, and resultstring in 'buffer'.
 *********/
int __regargs PromptFont(char *reqstring, int flag)
{
  struct rtFontRequester *fontreq;
  struct TextAttr *font;
  struct TextFont *newfont;
  BOOL loop=TRUE;

  buffer[0]=0;
/* OBS, borde man inte kunna byta font även om inget fönster finns //Kjer 960127 */
  if (!FRONTWINDOW || !(fontreq=rtAllocRequest(RT_FONTREQ, NULL)))
    return(OUT_OF_MEM);
  FreezeEditor(1);
  while (loop) {
    rtChangeReqAttr(fontreq,
                    RTFO_FontName, flag>1?Default.RequestFontAttr.ta_Name:Default.font.ta_Name,
                    RTFO_FontHeight, flag>1?Default.RequestFontAttr.ta_YSize:Default.font.ta_YSize, TAG_END);
    if (!(rtFontRequest(fontreq, reqstring,
                        (FRONTWINDOW->screen_pointer?RT_Screen:TAG_IGNORE), FRONTWINDOW->screen_pointer,
                        RTFO_Flags, (flag>1)?FREQF_SCALE:FREQF_FIXEDWIDTH|FREQF_SCALE,
                        RT_TextAttr, &Default.RequestFontAttr,
                        RTFO_MaxHeight, FRONTWINDOW->screen_pointer->Height>>3,  //OBS finns alltid screen_pointer?
		        TAG_END))) {
      ActivateEditor();
      rtFreeRequest(fontreq);
      return(FUNCTION_CANCEL);
    }
    font=&fontreq->Attr;
    if (font->ta_Flags & AFF_DISK)
      newfont=OpenDiskFont(font);
    else
      newfont=OpenFont(font);
    if (!newfont)
      reqstring=RetString(STR_COULDNT_OPEN_FONT);
    else if ((newfont->tf_YSize>FRONTWINDOW->screen_pointer->Height>>3) ||
              (newfont->tf_XSize>FRONTWINDOW->screen_pointer->Width>>3))
      reqstring=RetString(STR_FONT_TOO_BIG);
    else
      loop=FALSE;
    if (newfont)
      CloseFont(newfont);
  }
  Sprintf(buffer, "%s %ld", font->ta_Name, font->ta_YSize);
 
  if (fontreq)
    rtFreeRequest(fontreq);
  ActivateEditor();

  return(OK);
}
