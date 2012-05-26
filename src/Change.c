/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************************
*
*  Change.c
*
**********/

#include "compat.h"

#ifdef AMIGA
#include <devices/keymap.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <exec/types.h>
#include <exec/semaphores.h>
#include <graphics/gfxmacros.h>
#undef GetOutlinePen
#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include <libraries/reqtools.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/FPL.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/reqtools.h>
#include <proto/wb.h>
#endif

#include <libraries/FPL.h>
#include <limits.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "BufControl.h"
#include "Button.h"
#include "Change.h"
#include "Command.h"
#include "Cursor.h"
#include "Edit.h"
#include "Execute.h"
#include "Face.h"
#include "FACT.h"
#include "GetFile.h"
#include "GetFont.h"
#include "Icon.h"
#include "Init.h"
#include "IDCMP.h"
#include "KeyAssign.h"
#include "Limit.h"
#include "OpenClose.h"
#include "Request.h"
#include "Search.h"
#include "Setting.h"
#include "Slider.h"
#include "Undo.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"
#include "WindowOutput.h"
#include "Process.h" /* for the wait_process() prototype */
#include "FileHandler.h" /* for the FileHandler() prototype */

extern DefaultStruct Default;
extern FACT *DefaultFact;
extern struct rtFileRequester *FileReq; /* Buffrad Filerequester. Bra o ha! */
extern struct MsgPort *WBMsgPort;
extern struct AppIcon *appicon;
extern char *appiconname;
extern struct DiskObject AppIconDObj;
extern struct DiskObject *externappicon;
extern char FrexxEdStarted;	/* Är FrexxEd uppstartad */

extern char bitplanes;

extern struct Task *filehandlertask;

extern struct TextFont *SystemFont;
extern int BarHeight;

/* from filehandler: */
extern char filehandlerstop;
extern struct Task *filehandlertask;
extern struct ProcMsg *filehandlerproc;
/* end of filehandler variables */

extern struct SignalSemaphore LockSemaphore;

#define SETL ((char *)(sets[vald]->offset+((int)Storage)))
#define SETS ((char *)(sets[vald]->offset+((int)Storage->shared)))
#define SETW (Storage->window?((int)Storage->window)+(char *)(sets[vald]->offset):NULL)


void SetDirectory() {
    ChangeDirectory(Default.directory);
    rtChangeReqAttr(FileReq, RTFI_Dir, "", TAG_END);
};

void SetTaskPriority(int newvalue) {
    if (FrexxEdStarted)  SetTaskPri(FindTask(NULL), newvalue);
    if (filehandlertask) SetTaskPri(filehandlertask, newvalue);
}


void SetFileHandle(int newvalue) {
    if (newvalue) {
        if(filehandlerproc && !filehandlertask) {
            wait_process(filehandlerproc);
            filehandlerproc=NULL;
        }
        else if(filehandlerstop) {
            /* the filehandler is still alive, but have been signaled
               to die */
            filehandlerstop=FALSE; /* make it live again! */
        }
        if(!filehandlerproc && FrexxEdStarted>=2) {
            filehandlerproc = start_process(FileHandler, Default.taskpri,
                                            4000, FILE_HANDLER_PROCESS);
            if(filehandlerproc) {
                ReleaseSemaphore(&LockSemaphore);
                ObtainSemaphore(&LockSemaphore);
            }
        }
    } else {
        /* Här Danne!  Filehandlern slås av! */
        if(filehandlerproc && !filehandlerstop && filehandlertask)
            Signal(filehandlertask, SIGBREAKF_CTRL_C);
        /* we've signaled it to die as soon as possible! */
    }
}

void SetAppWindow(BufStruct * Storage,int newvalue) {
    WindowStruct *win=BUF(window);
    if (win->window_pointer) {
        if (newvalue) {
            if (win->window!=FX_SCREEN)
                win->appwindow_pointer = AddAppWindow(0, 0, win->window_pointer, WBMsgPort, 0);
        } else {
            if (win->appwindow_pointer) {
                RemoveAppWindow(win->appwindow_pointer);
                win->appwindow_pointer=NULL;
            }
        }
    }
    win=win->next;
}

void SetChangedFont(BufStruct * Storage) {
    GetFont(Storage, Default.SystemFont, 1);
    GetFont(Storage, Default.RequestFont, 2);
}

int CalcColour(BufStruct * Storage,int * input) {
    int ret=-1;
    if (input && BUF(window) && BUF(window)->screen_pointer) {
        if (SysBase->LibNode.lib_Version < 39) {
            ret=GetRGB4(BUF(window)->screen_pointer->ViewPort.ColorMap, *input);
        } else {
            ULONG cols[12];
            GetRGB32(BUF(window)->screen_pointer->ViewPort.ColorMap, *input, 3, cols);
            ret=FixRGB32(cols);
        }
    }
}

void ResizeWindow(BufStruct * Storage) {
    int dx;
    int dy;
    int xpos, ypos;
    dx=BUF(window)->window_pointer->Width - BUF(window)->window_width;
    dy=BUF(window)->window_pointer->Height - BUF(window)->window_height;
    if (BUF(window)->autoresize) {
        dx+=(BUF(window)->window_width -
             BUF(window)->window_pointer->BorderLeft -
             BUF(window)->window_pointer->BorderRight - 
             SystemFont->tf_BoldSmear-SystemFont->tf_XSize/2) %SystemFont->tf_XSize;
        dy+=(BUF(window)->window_height-(BUF(window)->text_start-1)-BUF(window)->window_pointer->BorderBottom-1)%SystemFont->tf_YSize;
            }
    xpos=BUF(window)->window_xpos;
    ypos=BUF(window)->window_ypos;
    if (BUF(window)->window_position==WINDOW_VISIBLE &&
        BUF(window)->screen_pointer) {
        xpos-=BUF(window)->screen_pointer->ViewPort.DxOffset;/* offset is negative */
        ypos-=BUF(window)->screen_pointer->ViewPort.DyOffset; /* offset is negative */
    }
    ChangeWindowBox(BUF(window)->window_pointer,
                    xpos,
                    ypos,
                    BUF(window)->window_pointer->Width-dx,
                    BUF(window)->window_pointer->Height-dy);
    //              SizeWindow(BUF(window)->window_pointer, -dx, -dy);
    
}


