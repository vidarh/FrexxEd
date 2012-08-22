/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * IDCMP.c
 *
 * Handle all IDCMP signals...
 *
 *********/

#include "compat.h"

#ifdef AMIGA
#include <devices/console.h>
#include <devices/inputevent.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <exec/types.h>
#include <graphics/gfxmacros.h>
#undef GetOutlinePen
#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/gadtools.h>
#include <proto/console.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/wb.h>
#include <stdio.h>
#include <string.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#endif

#include <assert.h>
#include <stdio.h>

#include "Buf.h"
#include "Alloc.h"
#include "BuildMenu.h"
#include "BufControl.h"
#include "Change.h"
#include "Command.h"
#include "Cursor.h"
#include "Execute.h"
#include "FACT.h"
#include "Fold.h"
#include "Icon.h"
#include "IDCMP.h"
#include "Hook.h"
#include "Init.h"
#include "KeyAssign.h"
#include "Limit.h"
#include "OpenClose.h"
#include "RawKeys.h"
#include "Request.h"
#include "Rexx.h"
#include "Setting.h"
#include "Timer.h"
#include "UpdtBlock.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"
#include "WindowOutput.h"
#include "Winsign.h"

extern DefaultStruct Default;
extern int Visible;
extern char CursorOnOff;
extern int undoantal;
extern char buffer[];
extern struct TextFont *SystemFont;
extern int systemfont_leftmarg;
extern int systemfont_rightmarg;
extern BOOL fplabort;	/* Allow to abort FPL scripts.  Startup script shouldn't be breakable */
extern int mouse_x;
extern int mouse_y;
extern struct Window *activewindow;	// Current active FrexxEd window
extern struct SignalSemaphore LockSemaphore;
extern int semaphore_count;

extern BufStruct *NewStorageWanted;
extern struct KeyMap *internalkeymap;
extern struct MsgPort *WindowPort;

extern struct MsgPort *WBMsgPort;

extern char *lockedalloc;
extern int signalbits; /* Which signal bits to Wait() for! */

extern AREXXCONTEXT RexxHandle;

extern int UpDtNeeded;
extern char *fpl_executer;	/* Namn p� den funktion som exekverar FPL */
extern char cursorhidden;

extern BOOL quitting;

#define NUM_OF_CHARS 32 /* max number of chars to recieve from keymap */

#define IDCMP1 IDCMP_INTUITICKS|IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MENUPICK|IDCMP_MOUSEMOVE|\
               IDCMP_CHANGEWINDOW|IDCMP_CLOSEWINDOW|IDCMP_GADGETDOWN|IDCMP_GADGETUP|\
               IDCMP_MENUVERIFY|IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW|IDCMP_NEWSIZE

extern struct InputEvent ievent;        /* used for RawKeyConvert() */
extern int zoomstate;
extern char ignoreresize;
extern struct MsgPort *TimerMP;
extern char build_default_menues;

extern BOOL device_has_killed_a_buffer;
extern BOOL clear_all_currents;

extern long fh_locks, fh_opens;

extern struct Task *FrexxEdTask;

extern short important_message_available;

/*** PRIVATE ***/

static struct IntuiMessage *IDCMPmsg=NULL;
static UWORD lastmsgCode=NULL;
static ReturnMsgStruct *firstreturnmsg=NULL;
static int mousex, mousey=0, oldmousex=-1, oldmousey=-1;
static char *Argv[3];
static int Argc;
static int lastrawkeypressed=0;
static int rawkeypressed=0;		/* Mouse or key was pressed */
static int commandflag=NULL;
static char mousebutton=0;
static BOOL idcmpbuffer=FALSE;	/* Om IDCMP-messaged ska replajas */


char windowresized=0;



static void ProcessRexxMsg(BufStruct * Storage)
{
    while (RexxHandle->rmsg=GetARexxMsg(RexxHandle)) {
        fpl_executer=EXECUTED_AREXX;
        
        /* If there is ARexx message: */
        ExecuteFPL(Storage,
                   ARG0(RexxHandle->rmsg), /* program */
                   EXECUTE_SCRIPT,	/* execute string */
                   NULL,		   /* ignore return code */
                   NULL);		   /* no extra parameter */
        
        ReplyARexxMsg(RexxHandle);
        if (NewStorageWanted) {
            Storage=NewStorageWanted;
            CursorOnOff=FALSE;
            clear_all_currents=TRUE;
        }
    } /* while rmsg */
}


struct AppMessage * GetWBMsg() {
    assert(WBMsgPort > 1024);
    return (struct AppMessage *)GetMsg(WBMsgPort);
};

void WaitForIntuiMsg() {
    Wait(1 << WindowPort->mp_SigBit);
};

struct IntuiMessage * GetIntuiMsg() {
    assert(WindowPort > 1024);
    return (struct IntuiMessage *)GetMsg(WindowPort);
};


static int ProcessAppMsg(BufStruct * Storage, int ret, int * command)
{
    struct AppMessage *appmsg;

    fprintf(stderr, "ProcessAppMsg\n");
    while (appmsg = GetWBMsg()) {
        struct WBArg   *argptr;
        int i;
        
        /*
         * The AppMessage type will be MTYPE_APPWINDOW,
         * the ID & userdata are what we supplied when
         * the window was designed as an AppWindow.
         * NumArgs allows us to process the Workbench
         * arguments properly.
         */
        
        /*
         * Get a pointer to the start of the Workbench
         * argument list.
         */
        argptr = appmsg->am_ArgList;
        if (!appmsg->am_NumArgs && !argptr) {
            Argc=2;
            Argv[0]=(char *)FRONTWINDOW->window_pointer;
            Argv[1]=(char *)appmsg->am_UserData;
            *command=DO_ICON_CLICK;
        } else {
            if ((Default.appicon && appmsg->am_Type==AMTYPE_APPICON) ||
                (appmsg->am_Type==AMTYPE_APPWINDOW)) {
                LockBuf_release_semaphore(BUF(shared));
                for (i = 0; i < appmsg->am_NumArgs; i++, argptr++) {
                    /*
                     * The lock will be on the directory in
                     * which the file resides. If there is no
                     * filename, either a volume or window was
                     * dropped on us.
                     */
                    if(!argptr->wa_Name[0] ||
                       !NameFromLock(argptr->wa_Lock, buffer,
                                     MAX_CHAR_LINE-strlen(argptr->wa_Name)))
                        continue;
                    
                    ret = strlen(buffer);
                    if(buffer[ret-1] != '/' && buffer[ret-1] != ':')
                        strcat(buffer, "/"); /* append slash! */
                    
                    strcat(buffer, argptr->wa_Name); /* add filename to path */
                    
                    SendReturnMsg(cmd_ICONDROP, DO_ICON_DROP, buffer, NULL,
                                  (int *)&appmsg->am_UserData);
                    ret = OK;
                }
                UnLockBuf_obtain_semaphore(BUF(shared));
            }
        }
        ReplyMsg((struct Message *) appmsg);
    }
    return ret;
}


void ProcessTimerRequest(BufStruct * Storage) 
{
    FrexxTimerRequest *ftr;
    fprintf(stderr, "ProcessTimerRequest\n");
    while (ftr= TimerGetMsg()) {
        int fplret;

        fpl_executer=EXECUTED_TIMER;
        ftr->flags|=time_ACTIVE;

        ExecuteFPL(Storage,
                   ftr->fpl_program,	/* program */
                   EXECUTE_SCRIPT,	/* execute string */
                   &fplret,		/* ignore return code */
                   NULL);		/* no extra parameter */
        
        if (NewStorageWanted) {
            Storage=NewStorageWanted;
            CursorOnOff=FALSE;
            clear_all_currents=TRUE;
        }

        if (!fplret && (ftr->flags&time_REPEAT))
            ReAddTimerEvent(ftr);
        else
            FreeTimeRequest(ftr);
    }
}


static void UndoDoNothing(BufStruct * Storage, int options) {
    if (undoantal)
        Command(Storage, DO_NOTHING | options, 0, NULL, NULL);  /* For the undo buffer */
}

/************************************************************************
 * 
 *  Splitting out the IDCMP handling
 *
 ************************************************************************/

static void handle_SELECTDOWN(BufStruct * Storage, const int activated, char * resize, BufStruct ** resizeStorage) {
    if (activated) return;

    /* IDCMP for the button pressed condition */
    if (!mousebutton) {
        BufStruct *Storage2=BUF(window)->NextShowBuf;
        mousebutton=1;
        while (Storage2 && (Storage2->top_offset>(mousey+1) ||
                            (Storage2->top_offset+Storage2->screen_lines-2)<mousey))
            Storage2=Storage2->NextShowBuf;
        if (Storage2) {
            if (Storage==Storage2) {
                rawkeypressed=MOUSELEFT;
                ModifyIDCMP(BUF(window)->window_pointer, IDCMP_INTUITICKS|IDCMP1);
            } else {
                UndoDoNothing(Storage,0);
                Storage=Activate(Storage, Storage2, Default.full_size_buf);
                Showstat(Storage);
                oldmousex=-1;
                oldmousey=-1;
                CursorXY(Storage, -1, 0);
                mousebutton=2;
            }
        } else {
            BufStruct *Storage2=BUF(window)->NextShowBuf;
            while (Storage2 &&
                   (Storage2->top_offset+Storage2->screen_lines-1)!=mousey)
                Storage2=Storage2->NextShowBuf;
            if (Storage2) {
                *resize=mousey<BUF(window)->window_lines+1 && mousey>=0;
                *resizeStorage=Storage2;
                oldmousey=BUF(cursor_y)-1;
            }
        }
    } else if (mousebutton==1) {
        if (mousex==oldmousex && mousey==oldmousey &&
            (mousey-BUF(top_offset)<BUF(uppermarg) ||
             BUF(screen_lines)-mousey-BUF(top_offset)<BUF(lowermarg))) {
            rawkeypressed=MOUSELEFT|MOUSEDRAG;
        }
    }
}



static void handle_MENUPICK(BufStruct * Storage) {
    UWORD menunumber=IDCMPmsg->Code;
    struct MenuItem *item;
    while (menunumber != MENUNULL) {
        struct OwnMenu *own;
        char *program;
        item=ItemAddress(BUF(window)->menus, menunumber);
        own=((struct OwnMenu *)GTMENUITEM_USERDATA(item));
        program=own->FPL_program;
        if (own->settingname) {
            Sprintf(buffer, "SetInfo(-1,\"%s\",%ld);", own->settingname, (item->Flags&CHECKED)?1:0);
            program=buffer;
        }
        SendReturnMsg(cmd_MENUPICK, menunumber, program, EXECUTED_MENU, NULL);
        menunumber=item->NextSelect;
    }
}

static void handle_MOUSEMOVE(BufStruct * Storage, int resize) {
    if (!resize && mousebutton==1) {
        if (!(IDCMPmsg->Qualifier&(IEQUALIFIER_LEFTBUTTON|IEQUALIFIER_MIDBUTTON|IEQUALIFIER_RBUTTON)) &&
            !((IDCMPmsg->Qualifier&RAW_AMIGA) && (IDCMPmsg->Qualifier&RAW_ALT))) {
            mousebutton=2;
        } else {
            if ((mousex!=oldmousex || mousey!=oldmousey) &&
                (BUF(curr_topline)==1 || mousey>=BUF(top_offset)+BUF(uppermarg)) &&
                (FoldMoveLine(Storage, BUF(curr_topline), BUF(screen_lines))-1==SHS(line) || mousey<BUF(top_offset)+BUF(screen_lines)-BUF(lowermarg)-1)) {
                rawkeypressed=(lastrawkeypressed&(MOUSELEFT|MOUSERIGHT|MOUSEMIDDLE))|MOUSEDRAG;
            }
        }
    }
}


/* Structure introduced to facilitate breakup of the IDCMP function */

struct CmdState {
    int activated;
    int ret;       /* variable to recieve exit code from commands */
    int err;       /* Secondary error code to check subroutine response before updating ret */
    int command;   /* which command the key should perform */
    BufStruct *CommandStorage; /* Which Storage to send to Command() */
    char resize;
    int lasty;
    struct Border * border;
};


static void FrexxDrawBorder(BufStruct * Storage, struct Border * border, WORD lasty)
{
    SetWrMsk((struct RastPort *)BUF(window)->window_pointer->RPort, 0x03);
    DrawBorder((struct RastPort *)BUF(window)->window_pointer->RPort,
               border,
               0,
               SystemFont->tf_YSize * lasty+BUF(window)->text_start);
}

static void handle_MOUSEBUTTONS(BufStruct *Storage, struct CmdState * state, BufStruct * resizeStorage) {
    if (!BUF(window)) return;
    switch(IDCMPmsg->Code) {
    case SELECTUP:
        if (!state->activated) {
            ModifyIDCMP(BUF(window)->window_pointer, IDCMP1);
            /* IDCMP for the button released condition */
            oldmousex=-1;
            if (state->resize) {
                BufStruct *Storage2=Storage, *Storage3;
                if (state->lasty>=0 && state->lasty<=BUF(window)->window_lines) {
                    FrexxDrawBorder(Storage,state->border,state->lasty);
                }
                if (state->lasty<0) state->lasty=0;
                if (state->lasty>BUF(window)->window_lines) 
                    state->lasty=BUF(window)->window_lines;
                UndoDoNothing(Storage,0);
                if (state->lasty-resizeStorage->top_offset+1!=resizeStorage->screen_lines) {
                    register int tempvisible=Visible;
                    Visible=VISIBLE_OFF;
                    Storage3=ReSizeBuf(Storage2, resizeStorage, NULL, state->lasty-resizeStorage->top_offset+1);
                    assert(Storage3 > 1024);
                    Storage=Activate(Storage3, Storage3, Default.full_size_buf);
                    SetScreen(Storage, Col2Byte(Storage, BUF(cursor_x)+BUF(screen_x),
                                                RAD(BUF(curr_line)),
                                                LEN(BUF(curr_line))),
                              BUF(curr_line));
                    Visible=tempvisible;
                    RefreshGList(BUF(window)->window_pointer->FirstGadget, BUF(window)->window_pointer, NULL, -1);
                    UpdateAll();
                }
                state->lasty=-2;
            } else
                rawkeypressed=MOUSELEFT|MOUSEUP;
            state->resize=mousebutton=0;
        }
        break;

    case SELECTDOWN:
        handle_SELECTDOWN(Storage,state->activated,&state->resize,&resizeStorage);
        break;

    case MIDDLEDOWN:
        if (!state->resize) {
            rawkeypressed=MOUSEMIDDLE;
            mousebutton=1;
            ModifyIDCMP(BUF(window)->window_pointer, IDCMP_INTUITICKS|IDCMP1);
        }
        break;

    case MIDDLEUP:
        if (!state->resize) {
            rawkeypressed=MOUSEMIDDLE|MOUSEUP;
            mousebutton=0;
            ModifyIDCMP(BUF(window)->window_pointer, IDCMP1);
        }
        break;

    case MENUUP:
        if(state->resize || (Default.RMB && 
                             ((IDCMPmsg->Code==MENUHOT && (IDCMPmsg->MouseY<0 ||
                                                     IDCMPmsg->MouseY>=BUF(window)->text_start)) ||
                        mousebutton))) {
            if (!state->resize) {
                rawkeypressed=MOUSERIGHT|MOUSEUP;
                ModifyIDCMP(BUF(window)->window_pointer, IDCMP1);
            }
            mousebutton=0;
        }
        break;
    }
}


void handle_TICKS(BufStruct * Storage,struct CmdState * state) {
    fprintf(stderr,"Tick tock\n");
    if (face_update && Visible==VISIBLE_ON) UpdateFace();
    if (ignoreresize)
        ignoreresize--;
    if (state->activated) {
        state->activated--;
    } else if (mousebutton==1 && activewindow==IDCMPmsg->IDCMPWindow) {
        if (!(IDCMPmsg->Qualifier&(IEQUALIFIER_LEFTBUTTON|IEQUALIFIER_MIDBUTTON|IEQUALIFIER_RBUTTON)) &&
            !((IDCMPmsg->Qualifier&RAW_AMIGA) && (IDCMPmsg->Qualifier&RAW_ALT))) {
            mousebutton=2;
        } else {
            if (!state->resize) {
                if ((mousex!=oldmousex || mousey!=oldmousey) ||
                    (mousex<BUF(leftmarg) && BUF(screen_x)) ||
                    mousex>BUF(screen_col)-BUF(rightmarg) ||
                    (BUF(curr_topline)>1 && mousey<BUF(top_offset)+BUF(uppermarg)) ||
                    (FoldMoveLine(Storage, BUF(curr_topline), mousey)<=SHS(line) && mousey>=BUF(top_offset)+BUF(screen_lines)-BUF(lowermarg)-1)) {
                    rawkeypressed=(lastrawkeypressed&(MOUSELEFT|MOUSERIGHT|MOUSEMIDDLE))|MOUSEDRAG;
                }
            }
        }
    } else {
        if (!idcmpbuffer && IDCMPmsg->IDCMPWindow!=BUF(window)->window_pointer) {
            NewStorageWanted=state->CommandStorage->window->ActiveBuffer;
            WindowActivated(Storage, IDCMPmsg);
            state->command=DO_NOTHING_RETMSG;
        }
        if (activewindow==IDCMPmsg->IDCMPWindow)
            mousebutton=0;
        if (!windowresized) {
            ModifyIDCMP(IDCMPmsg->IDCMPWindow, IDCMP1);
        }
    }
    if (!idcmpbuffer && windowresized) {
        WindowStruct *win;
        windowresized=0;
        win=FRONTWINDOW;
        while (win) {
            if (win->window_resized) {
                windowresized++;
                win->window_resized--;
                if (!win->window_resized)
                    state->ret=ReSizeWindow(win->NextShowBuf);
            }
            win=win->next;
        }
    }
}



void ProcessRetMsg(BufStruct * Storage, ReturnMsgStruct *retmsg, struct CmdState * state) {
    int tmperr;
    switch(retmsg->command) {
    case cmd_DEVICE:
        Storage->shared->current++;
        RunHook(Storage, DO_FILEHANDLER_EXCEPTION, 2, (char **)&retmsg->args, NULL);
        Storage->shared->current--;
        if (NewStorageWanted)
            state->ret=NEW_STORAGE;
        state->command=DO_NOTHING_RETMSG;
        break;

    case cmd_AUTOSAVE:
        state->CommandStorage=CheckBufID((BufStruct *)retmsg->retvalue);
        if (state->CommandStorage && state->CommandStorage->shared->asenabled) {
            state->CommandStorage->shared->current++;
            state->CommandStorage->shared->asenabled=0;
            RunHook(state->CommandStorage, DO_AUTOSAVE, 1, (char **)&state->CommandStorage, NULL);
            state->CommandStorage->shared->current--;
            if (NewStorageWanted)
                state->ret=NEW_STORAGE;
        }
        state->command=DO_NOTHING_RETMSG;
        break;

    case cmd_MENUPICK:
        BUF(locked)++;
        SHS(current)++;
        Default.commandcount++;
        Argv[0]=(char *)(MENUNUM(retmsg->retvalue)+1);
        Argv[1]=(char *)(ITEMNUM(retmsg->retvalue)+1);
        Argv[2]=(char *)(SUBNUM(retmsg->retvalue))+1;
        if ((int)Argv[2]==32)
            Argv[2]=NULL;
        
        {
            int temp;
            temp=RunHook(state->CommandStorage, DO_MENU_SELECTED, 3, (char **)&Argv, NULL);
            BUF(locked)--;
            SHS(current)--;
            if (temp)
                break;	// MenuSelect is aborted
        }
        /* !!!! Falling down !!!! */
    case cmd_EXECUTE:
        state->command=DO_EXECUTE_STRING|NO_HOOK;
        Argc=1;
        Argv[0]=retmsg->string;
        lockedalloc=(char *)retmsg;
        fpl_executer=retmsg->caller;
        break;
    case cmd_STATUS:
        if (*(char *)retmsg->string)
            Status(Storage, (char *)retmsg->string, 3);
        else
            Showstat(Storage);
        break;
    case cmd_KEY:
    case cmd_IDCMP_MSG:
        if (BUF(window)->window_pointer) {
            IDCMPmsg=(struct IntuiMessage *)retmsg->string;
            idcmpbuffer=TRUE;
        }
        break;
    case cmd_RET:
        state->ret=retmsg->retvalue;
        Showerror(Storage, state->ret);
        break;
    case cmd_NEWSTORAGE:
        Storage=(BufStruct *)retmsg->retvalue;
        break;
    case cmd_STRINGCMD:
        state->command=retmsg->retvalue;
        Argc=1;
        Argv[0]=retmsg->string;
        break;
    case cmd_ICONDROP:
        state->command=retmsg->retvalue;
        Argc=2;
        Argv[0]=retmsg->string;
        Argv[1]=(char *)retmsg->args[0];
        break;
    case cmd_WINDOWSIZE:
        handle_WINDOWSIZE(Storage, state->CommandStorage, retmsg, &Argv);
        break;
    case cmd_MENUCLEAR:
        if ((tmperr = handle_MENUCLEAR(state->ret)) != OK) state->ret = tmperr;
        break;
    }
}

static int Resize(BufStruct * Storage, struct Border * border, int lasty) {
    WORD coords[10];
    int newy;
    newy=(IDCMPmsg->MouseY-BUF(window)->text_start);
    newy=(newy-((newy<0)?SystemFont->tf_YSize:0))/SystemFont->tf_YSize;
    if (newy != lasty && BUF(window)) {
        if (lasty>=0 && lasty<=BUF(window)->window_lines) {
            FrexxDrawBorder(Storage,border,lasty);
        }
        coords[0]=BUF(left_offset);
        coords[6]=coords[0];
        coords[8]=coords[0];
        coords[2]=coords[0]+BUF(screen_col)*SystemFont->tf_XSize-1;
        coords[4]=coords[2];
        coords[1]=0;
        coords[3]=0;
        coords[5]=SystemFont->tf_YSize-1;
        coords[7]=coords[5];
        coords[9]=1;
        border->XY=coords;
        lasty=newy;
        if (lasty>=0 && lasty<=BUF(window)->window_lines) {
            FrexxDrawBorder(Storage,border,lasty);
        }
    }
    return lasty;
}

static void DeleteAllBufs(BufStruct * Storage) {
  Visible=VISIBLE_OFF;
  char loop=2;
  ClearWindow(BUF(window));
  do {
      BufStruct *Storage=Default.BufStructDefault.NextBuf;
      BufStruct *Storage2;
      while (Storage) {
          Visible=VISIBLE_OFF;
          UpDtNeeded|=UD_REDRAW_BORDER|UD_REDRAW_VIEWS|UD_CLEAR_SCREEN|UD_SHOW_STATUS|UD_MOVE_SLIDER|UD_REDRAW_ENTRY|UD_REDRAW_BLOCK|UD_CLEAR_ENTRY|UD_REDRAW_BUFFS|UD_REDRAW_SETTING;
          Storage2=BUF(NextBuf);

          if (SHS(freeable) || SHS(shared)>1)
              Free(Storage, FALSE, TRUE);

          Storage=Storage2;
      }
      if (loop==2) {
          RunHook(Default.FirstBlock->Entry, DO_EXIT, 0, NULL, NULL); // Exit-hook
          Default.FirstBlock->freeable=TRUE;
          Free(Default.FirstBlock->Entry, FALSE, TRUE);
          Default.FirstBlock=NULL;
      }
      loop--;
  } while (loop);
}

static void ClearAllCurrents() {
    if (clear_all_currents) {
        SharedStruct *shared=&Default.SharedDefault;
        while (shared) {
            shared->current=0;
            shared=shared->Next;
        }
        clear_all_currents=FALSE;
    }
}


static void handle_IDCMP(BufStruct * Storage, struct CmdState * state, BufStruct * resizeStorage)
{
    switch(IDCMPmsg->Class) {
    case IDCMP_INTUITICKS:
        handle_TICKS(Storage, state);
        if (activewindow==IDCMPmsg->IDCMPWindow)
            break;
        /* FALLING THROUGH, FALLING THROUGH */

    case IDCMP_INACTIVEWINDOW:

    case IDCMP_ACTIVEWINDOW:
        if (WindowActivated(Storage, IDCMPmsg)) {
            ModifyIDCMP(IDCMPmsg->IDCMPWindow, IDCMP1|IDCMP_INTUITICKS);
            state->activated=2;
        }
        if (NewStorageWanted)
            state->command=DO_NOTHING_RETMSG;
        break;

    case IDCMP_NEWSIZE:
        {
            WindowStruct *win=FindWindow(IDCMPmsg->IDCMPWindow);
            if (win && win->NextShowBuf)
                ReSizeWindow(win->NextShowBuf);
        }
        break;

    case IDCMP_MENUVERIFY:
        if (IDCMPmsg->Code==MENUHOT) {
            if(state->resize || (Default.RMB && 
                                IDCMPmsg->MouseY>=BUF(window)->text_start ||
                                mousebutton)) {
                if (state->resize) {
                    if (state->lasty>=0 && state->lasty<=BUF(window)->window_lines) {
                        FrexxDrawBorder(Storage,state->border,state->lasty);
                    }
                    state->resize=0;
                    mousebutton=2;
                    state->lasty=-2;
                } else {
                    rawkeypressed=MOUSERIGHT;
                    mousebutton=1;
                    ModifyIDCMP(BUF(window)->window_pointer, IDCMP_INTUITICKS|IDCMP1);
                }
                IDCMPmsg->Code=MENUCANCEL;
            } else {
                if (InitializeMenu(Storage)) {
                    //                ResetMenuStrip(BUF(window)->window_pointer, menu.menus);  //Den h�r borde g�ras, men verkar inte beh�vas.  Dessutom funkar inte MagicMenu om den finns.
                }
            }
        }
        break;
    case IDCMP_MENUPICK:
        handle_MENUPICK(Storage);
        break;
    case IDCMP_CLOSEWINDOW:
        state->command=DO_CLOSE_WINDOW;
        break;
    case IDCMP_GADGETDOWN:
        state->CommandStorage=(BufStruct *)GadgetAddress(Storage, IDCMPmsg);
        state->command=DO_SLIDER;
        break;
    case IDCMP_MOUSEMOVE:
        handle_MOUSEMOVE(Storage,state->resize);
        break;
    case IDCMP_MOUSEBUTTONS:
        handle_MOUSEBUTTONS(Storage,&state, resizeStorage);
        break;
    case IDCMP_CHANGEWINDOW:
        handle_CHANGEWINDOW(FindWindow(IDCMPmsg->IDCMPWindow));
        break;
    case IDCMP_RAWKEY:
        if (
            (IDCMPmsg->Code&128) ||
            (IDCMPmsg->Code>=RAWC_LEFT_SHIFT &&
             IDCMPmsg->Code<=RAWC_MIDDLE_MOUSE));
        else {
            mousebutton=0;
            rawkeypressed=RAWPRESSED;
        }
        break;
    default:
        break;
    }
}

/***********************************************************************
 *
 * IDCMP()
 *
 * This is the main loop function.
 *
 * Waits for all signals (menus, keys, ARexx, etc) and calls all
 * functions!
 *
 * NOTE: All ARexx functions work perfect even if the rexx.library couldn't
 * be opened! (The ARexx simply won't work then!)
 *
 ******/

void IDCMP(BufStruct *Storage)
{
    struct CmdState state;

  ULONG signals;

  BufStruct *resizeStorage=NULL;
  struct Border border={ 0, 0, 1, 0, COMPLEMENT, 5, NULL, NULL};
  ReturnMsgStruct *retmsg=NULL;

  int wbmsg_signal_bits=0;
  int rexx_signal_bits=0;
  int timer_signal_bits=0;

  state.CommandStorage=Storage;/* Which Storage to send to Command() */
  state.activated = 0;
  state.resize = FALSE;
  state.lasty = -2;
  state.border = &border;

  if (RexxHandle)
      rexx_signal_bits=(1L << (RexxHandle->ARexxPort->mp_SigBit));
  if (WBMsgPort)
      wbmsg_signal_bits=(1 << WBMsgPort->mp_SigBit);
  if (TimerMP)
      timer_signal_bits=(1 << TimerMP->mp_SigBit);

  if (!menu.array_size) {
      if (SetDefaultMenu(&menu)) /* set up the entire internal menu structure list */
          CloseAll(RetString(STR_GET_MEMORY));
      assert(menu.array_size);
  }
  
  WindowStruct *wincount;
  wincount=FRONTWINDOW;
  while (wincount) {
      menu_attach(wincount);
      wincount=wincount->next;
  }

  CursorOnOff=TRUE;
  CursorXY(Storage, BUF(cursor_x), BUF(cursor_y));

  fplabort=TRUE;

  semaphore_count=1;

  while (1) {		/* Can only be exited with ret==QUIT_COMMAND */
      state.command=DO_NOTHING;
      commandflag=NULL;
      Argc=0;		/* Clear the argument */
      state.ret=OK;
      state.CommandStorage=Storage;
      IDCMPmsg=NULL;
      rawkeypressed=0;
      signals=0;
      
      if (!retmsg) {
          ClearAllCurrents();

          IDCMPmsg=GetIntuiMsg();

          if (!IDCMPmsg) {
              semaphore_count=0;
              SHS(current)=0;
              ReleaseSemaphore(&LockSemaphore);
              signals=Wait(signalbits);
              ObtainSemaphore(&LockSemaphore);
              semaphore_count++;
              if (device_has_killed_a_buffer) {
                  if (!CheckBufID(Storage)) {
                      Storage=Default.BufStructDefault.NextBuf;
                      if (FRONTWINDOW)
                          Storage=FRONTWINDOW->NextShowBuf;
                  }
                  device_has_killed_a_buffer=FALSE;
              }
              SHS(current)++;

              if (signals & (SIGBREAKF_CTRL_C|SIGBREAKF_CTRL_E)) {
                  if (signals & SIGBREAKF_CTRL_C)	// quit
                      state.command=DO_QUIT_ALL;
                  if (signals & SIGBREAKF_CTRL_E) {	// deiconify.
                      if (BUF(window) && BUF(window)->iconify) {
                          DeIconify(Storage);
                          state.command=DO_NOTHING_RETMSG;
                      }
                  }
              } else {
                  IDCMPmsg=GetIntuiMsg();
              }
          }
      }

      if (retmsg || IDCMPmsg) {
          if (retmsg) ProcessRetMsg(Storage,retmsg,&state);

          if (IDCMPmsg) {
              if (!idcmpbuffer && BUF(window)->window_pointer!=IDCMPmsg->IDCMPWindow) {
                  WindowStruct *win=FindWindow(IDCMPmsg->IDCMPWindow);
                  if (win && win->NextShowBuf) {
                      state.command=DO_NOTHING_RETMSG;
                      state.CommandStorage=win->NextShowBuf;
                  }
              }
              lastmsgCode=IDCMPmsg->Code;
              mousex=(IDCMPmsg->MouseX-BUF(left_offset))/SystemFont->tf_XSize-BUF(l_c_len)-BUF(fold_len);
              mousey=(IDCMPmsg->MouseY-BUF(window)->text_start)/SystemFont->tf_YSize;
              handle_IDCMP(Storage, &state,resizeStorage);
          }
          
          
          if (rawkeypressed) {
              state.command=ExamineKeyPress(Storage);
          }

          
          if (state.resize && IDCMPmsg) {
              state.lasty = Resize(Storage, &border, state.lasty);
          }

          if (IDCMPmsg) {
              if (!idcmpbuffer) {
                  ReplyMsg((struct Message *)IDCMPmsg);
              }
              idcmpbuffer=FALSE;
              IDCMPmsg=NULL;
          }
      } /* if IDCMPmsg */
      
      
      if (signals & timer_signal_bits) {	/* TIMER ACTION */
          ProcessTimerRequest(Storage);
      }

      if (signals & rexx_signal_bits) {
          ProcessRexxMsg(Storage);
      }
      
      if (signals & wbmsg_signal_bits) {
          state.ret = ProcessAppMsg(Storage,state.ret,&state.command);
      }


      if (state.command>DO_NOTHING_RETMSG) {
          if (state.resize) {
              if (BUF(window)->window_pointer && state.lasty>=0 && state.lasty<=BUF(window)->window_lines) {
                  FrexxDrawBorder(Storage, &border, state.lasty);
              }
              state.resize=FALSE;
              state.lasty=-1;
          }

          {
              int temp=0;
              Default.commandcount++;

              if (Default.hook[DO_EVENT&~LOCKINFO_BITS]) {
                  temp = RunEventHook(state.CommandStorage,(char **)&state.command,0);
              }


              if (!temp) {
                  state.ret=Command(state.CommandStorage, state.command, Argc, (char **)&Argv, commandflag); /* here runs *ALL* functions!!! */
                  FreeLockedAlloc();
              }

              if (NewStorageWanted)
                  state.CommandStorage=NewStorageWanted;

              if (Default.posthook[state.command&~LOCKINFO_BITS]) {
                  RunEventHook(state.CommandStorage,(char **)&state.command,HOOK_POSTHOOK);
              }
          }
      }
      

      if (state.ret==QUIT_COMMAND) {
          if(fh_locks || fh_opens) {
              while (GetReturnMsg(cmd_RET));
              Sprintf(buffer, RetString(STR_CANT_QUIT), fh_locks + fh_opens);
              Ok_Cancel(Storage, buffer, NULL, RetString(STR_OK_GADGET));
              state.ret = OK;
          }
          else
              break;		/* Exit fr�n IDCMP-loopen */
      }
      

      if (NewStorageWanted) {
          Storage=NewStorageWanted;
          clear_all_currents=TRUE;
          NewStorageWanted=NULL;
          CursorOnOff=FALSE;
          state.ret=OK;
      }
      
      
      if (LFLAGS(BUF(curr_line))&FOLD_HIDDEN) {
          UndoDoNothing(Storage,NO_HOOK);
          AdjustCursorUp(Storage);
          TestCursorPos(Storage);
      }
      

      if (!BUF(window) || !BUF(window)->window_pointer) {
          WindowStruct *win=FRONTWINDOW;

          if (!win) {
              win=CreateNewWindow(NULL, NULL, Storage);
              if (win) {
                  assert(win->ActiveBuffer > 1024);
                  if (!OpenUpScreen(win))
                      UpdateWindow(win);
              }
          }
          
          while (win && !win->window_pointer)
              win=win->next;
          
          if (win) {
              assert(win->ActiveBuffer > 1024);
              Storage=Activate(Storage, win->ActiveBuffer, Default.full_size_buf);
          }
      }
    
      if (BUF(window)) {
          assert(Storage > 1024);
          BUF(window)->ActiveBuffer=Storage;
          if (BUF(window)->prev)
              HeadWindow(BUF(window));
      }
      
      retmsg=GetReturnMsg(0);
      if (BUF(window) && BUF(window)->window_pointer) {
          if (BUF(window)->Visible==VISIBLE_ON ||
              (UpDtNeeded & UD_REDRAW_SETTING)) {
              if (Visible!=VISIBLE_ON) {
                  CursorOnOff=TRUE;
                  ClearVisible();
                  CursorXY(Storage, -1, -1);
              } else {
                  if (UpDtNeeded)
                      ClearVisible();
              }
              if (state.command!=DO_NOTHING) {
                  if (state.ret>=OK) {
                      if (state.ret>LAST_CODE)   /* Utg�r fr�n att ingen str�ng ligger under address LAST_CODE :-)*/
                          Status(Storage, (char *)state.ret, 3);
                      else {
                          if (BUF(namedisplayed) & 1)
                              Showplace(Storage);
                          else
                              Showstat(Storage);
                      }
                  } else
                      Showerror(Storage, state.ret);
              }
              if (!CursorOnOff || state.command==DO_NOTHING_RETMSG) {
                  cursorhidden=FALSE;
                  CursorOnOff=TRUE;
                  CursorXY(Storage, BUF(cursor_x), BUF(cursor_y));
              }
              if (BUF(block_exists)==be_MARKING)
                  UpdateBlock(Storage);
          } else
              Visible=VISIBLE_OFF;	// F�nstret �r f�r litet, allt uppdateras senare.
      } else {
          if (BUF(window))
              BUF(window)->iconify=TRUE;
          Visible=VISIBLE_ICONIFIED;
      }
  } /* while EVIG */
  
  quitting=TRUE;

  DeleteAllBufs(Storage);

  ReleaseSemaphore(&LockSemaphore);
}

/*******************************************
 *
 *  int GetKey(BufStruct *Storage, int flags)
 *
 *  Take one char from the keyboard and return it to you.
 *  You can also read in 'buffer' if the keypress result in more key strokes.
 *  buffer[81] contains the length of the string.
 *  Read in 'buffer+100' for the entire IntuiMessage.
 *  Flags defined in IDCMP.h.
 *  The CAPS_LOCK is always stripped.
 *  Return RawKey value.
 **********/
int GetKey(BufStruct *Storage, int flags)
{
  struct IntuiMessage *msg;
  int chars;
  int ret=-1;
  int stop=FALSE;
  int pos=0;
  WindowStruct *win=FRONTWINDOW;
  if (Storage && BUF(window))
    win=BUF(window);

  if (win && win->window_pointer) {
    if (!(flags & gkNOCHACHED) && (msg=(struct IntuiMessage *)GetReturnMsg(cmd_KEY))) {
      msg=(struct IntuiMessage *)(((ReturnMsgStruct *)msg)->string);
      ievent.ie_Code=msg->Code;
      if (flags&gkFULLSYNTAX) {
        pos=PrintQualifiers(buffer, msg->Qualifier, ksKEYSEQ);
        msg->Qualifier=0;
      }
      msg->Qualifier=msg->Qualifier&(~RAW_CAPS);
      ievent.ie_Qualifier=msg->Qualifier;
      if (flags & gkSTRIP) {
        ievent.ie_Qualifier=msg->Qualifier&RAW_SHIFT;
        msg->Qualifier=msg->Qualifier&(~RAW_SHIFT);
      }
      ievent.ie_Qualifier=msg->Qualifier;
      ievent.ie_EventAddress=*((APTR *)msg->IAddress);
      chars=RawKeyConvert(&ievent, buffer+pos, 80, internalkeymap); //! Why 80 here and "NUM_OF_CHARS" (== 32) elsewhere?
      buffer[chars+pos]=0;
      ret=msg->Code;
      buffer[81]=chars;
      memcpy(buffer+100, (char *)msg, sizeof(struct IntuiMessage));
    } else {
      buffer[0]=0;
      if (flags & gkWAIT) {
          while (msg= GetIntuiMsg())
              ReplyMsg((struct Message *)msg);
      }
      while (!stop) {
        if (flags & gkWAIT) {
          ModifyIDCMP(win->window_pointer, IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MENUVERIFY|IDCMP_CHANGEWINDOW|
                      IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW);
          Wait(1 << WindowPort->mp_SigBit);
        } else
          stop=TRUE;
        assert(WindowPort > 1024);
        while (msg=GetIntuiMsg()) {
            switch(msg->Class) {
            case IDCMP_INTUITICKS:
                if (ignoreresize)
                    ignoreresize--;
                break;
            case IDCMP_CHANGEWINDOW:
                handle_CHANGEWINDOW(FindWindow(msg->IDCMPWindow));
                break;
            case IDCMP_NEWSIZE:
                {
                    WindowStruct *win=FindWindow(msg->IDCMPWindow);
                    if (win && win->NextShowBuf) {
                        ReSizeWindow(win->NextShowBuf);
                    }
                }
                break;
            case IDCMP_ACTIVEWINDOW:
            case IDCMP_INACTIVEWINDOW:
                WindowActivated(Storage, msg);
                break;
            case IDCMP_MENUVERIFY:
                msg->Code=MENUCANCEL;
                stop=flags&gkNOCHACHED;
            if (stop)
              ret=0;
            break;
          case IDCMP_MOUSEBUTTONS:
            if (flags&gkNOCHACHED) {
              if (msg->Code==SELECTDOWN ||
                  msg->Code==MIDDLEDOWN ||
                  msg->Code==SELECTUP ||
                  msg->Code==MIDDLEUP ||
                  msg->Code==MENUUP) {
                stop=TRUE;
                ret=0;
              }
            }
            break;
          case IDCMP_RAWKEY:
            if ((flags & gkNOTEQ) && lastmsgCode==msg->Code)
              break;
            lastmsgCode=msg->Code;
            if ((msg->Code&128) ||
                (msg->Code>=RAWC_LEFT_SHIFT &&
                 msg->Code<=RAWC_MIDDLE_MOUSE &&
                 !(flags&gkQUALIFIERS)))
              break;
            ievent.ie_Code=msg->Code;
            if (flags&gkFULLSYNTAX) {
              pos=PrintQualifiers(buffer, msg->Qualifier, ksKEYSEQ);
              msg->Qualifier=0;
            }
            msg->Qualifier=msg->Qualifier&(~RAW_CAPS);
            ievent.ie_Qualifier=msg->Qualifier;
            if (flags & gkSTRIP) {
              ievent.ie_Qualifier=msg->Qualifier&RAW_SHIFT;
              msg->Qualifier=msg->Qualifier&(~RAW_SHIFT);
            }
            ievent.ie_EventAddress= *((APTR *)msg->IAddress);
            stop=TRUE;
            chars=RawKeyConvert(&ievent, buffer+pos, 80, internalkeymap);
            if (chars>=0) {
              chars+=pos;
              buffer[chars]=0;
              ret=msg->Code;
            }
            buffer[81]=chars;
          }
          if (stop)
            memcpy(buffer+100, (char *)msg, sizeof(struct IntuiMessage));
          ReplyMsg((struct Message *)msg);
          if (stop)
            break;
        }
      }
      if (flags & gkWAIT) {
        ModifyIDCMP(win->window_pointer, IDCMP1);
      }
    }
  }
  return(ret);
}

int __regargs ReSizeWindow(BufStruct *Storage)
{
  int msgreturned=TRUE;
  register int ret=OK;
  WindowStruct *win=NULL;
  int args[2];

  if (Storage && BUF(window))
    win=BUF(window);

  if (win && win->window_pointer && win->window!=FX_SCREEN) {
    win->window_resized=0;
    CopyWindowPos(win);
    {
      register oldx=win->window_xpos;
      register oldy=win->window_ypos;
      win->window_xpos=win->real_window_xpos;
      win->window_ypos=win->real_window_ypos;
      if (win->window_position==WINDOW_VISIBLE) {
        win->window_xpos+=win->screen_pointer->ViewPort.DxOffset; /* offset is negative */
        win->window_ypos+=win->screen_pointer->ViewPort.DyOffset;/* offset is negative */
      }
      if (oldx!=win->window_xpos || oldy!=win->window_ypos)
        msgreturned=FALSE;
    }
    args[1]=(int)win;
    if (win->real_window_width!=win->window_pointer->Width ||
        win->real_window_height!=win->window_pointer->Height) {
      CursorXY(NULL, -1, -1);
      msgreturned=TRUE;
      win->window_width=win->window_pointer->Width;
      if (win->window_pointer->Height<=win->window_minheight ||
          win->window_pointer->Height==win->window_pointer->MinHeight) {
        win->real_window_width=win->window_pointer->Width;
        win->real_window_height=win->window_pointer->Height;
        win->window_height=win->window_pointer->Height;
        InitWindow(win);
        ClearWindow(win);
        RefreshWindowFrame(win->window_pointer);
        Visible=VISIBLE_OFF;
        UpDtNeeded|=UD_CLEAR_SCREEN|UD_REDRAW_BORDER|UD_REDRAW_VIEWS;
        win->UpDtNeeded|=UD_CLEAR_SCREEN|UD_REDRAW_BORDER|UD_REDRAW_VIEWS;
//          SizeWindow(window->window_pointer, win->window_minwidth-window->window_pointer->Width, win->window_minheight-window->window_pointer->Height);
        args[0]=1;
        SendReturnMsg(cmd_WINDOWSIZE|cmd_EGO, 1, NULL, NULL, args);
      } else {
        register int dx=0;
        register int dy=0;
  
        if (win->autoresize) {
          if (win->window_pointer->Width>win->window_pointer->MinWidth)
            dx=(win->window_pointer->Width -
                win->window_pointer->BorderLeft -
                win->window_pointer->BorderRight -
                systemfont_leftmarg-systemfont_rightmarg)%SystemFont->tf_XSize;
          if (win->window_pointer->Height>win->window_pointer->MinHeight && win->window_pointer->Height>win->window_minheight)
            dy=(win->window_pointer->Height-(BUF(window)->text_start-1)-win->window_pointer->BorderBottom-1)%SystemFont->tf_YSize;
        }
        if (dx || dy) {
          SizeWindow(win->window_pointer, -dx, -dy);
        } else {
          win->real_window_width=win->window_pointer->Width;
          win->real_window_height=win->window_pointer->Height;
          win->window_height=win->window_pointer->Height;
          ret=ReDrawSetting(win->NextShowBuf, SE_REINIT);
          args[0]=1;
          SendReturnMsg(cmd_WINDOWSIZE|cmd_EGO, 1, NULL, NULL, args);
        }
      }
    }

    if (zoomstate!=(win->window_pointer->Flags&WFLG_ZOOMED)) {
      zoomstate=win->window_pointer->Flags&WFLG_ZOOMED;
      args[0]=2;
      SendReturnMsg(cmd_WINDOWSIZE|cmd_EGO, 2, NULL, NULL, args);
    } else {
      if (!msgreturned) {
        args[0]=0;
        SendReturnMsg(cmd_WINDOWSIZE|cmd_EGO, 0, NULL, NULL, args);
      }
    }
  }
  return(ret);
}

/* command is defined in 'Buf.h' */
BOOL __regargs SendReturnMsg(int command, int retvalue, char *string, char *caller, int *args)
{
    ReturnMsgStruct *retmsg;
    ReturnMsgStruct *first=firstreturnmsg;
    int len=0;

  if (command&cmd_IMPORTANT) {
    command&=~cmd_IMPORTANT;
    Signal(FrexxEdTask, SIGBREAKF_CTRL_D); /* inform FrexxEd */
    important_message_available++;
  }

  if (first) {
    if (command & cmd_EGO) {
        ReturnMsgStruct *prev=NULL;
        command&=~cmd_EGO;
        do {
            ReturnMsgStruct *next=first->next;
            if (first->command==command && !first->sent &&
                (!args || (args[0]==first->args[0] && args[1]==first->args[1]))) {
                if (prev) {
                    prev->next=first->next;
                } else
                    firstreturnmsg=first->next;
                Dealloc(first);
            } else
                prev=first;
            first=next;
        } while (first);
        first=prev;
    } else {
        while (first->next)
            first=first->next;
    }
  }
  
  if (command==cmd_KEY || command==cmd_IDCMP_MSG) {
      len=sizeof(struct IntuiMessage);
  } else {
      if (string)
          len=strlen(string)+1;
  }
  
  retmsg=(ReturnMsgStruct *)Malloc(sizeof(ReturnMsgStruct)+len);
  if (!retmsg)
    return(FALSE);

  retmsg->retvalue=retvalue;
  retmsg->caller=caller;
  retmsg->command=command & ~cmd_EGO;
  retmsg->sent=FALSE;
  retmsg->next=NULL;
  retmsg->string[0]=0;
  if (len) {
    memcpy(retmsg->string, string, len);
  }
  if (args) {
    retmsg->args[0]=args[0];
    retmsg->args[1]=args[1];
  }

  if (command==(cmd_STATUS|cmd_EGO)) {
    retmsg->next=firstreturnmsg;
    firstreturnmsg=retmsg;
  } else {
    if (first) {
      first->next=retmsg;
    } else
      firstreturnmsg=retmsg;
  }

  return(TRUE);
}


BOOL SendReturnValue(int ret) {
    return SendReturnMsg(cmd_RET, ret, NULL, NULL, NULL);
}

ReturnMsgStruct *GetReturnMsg(int command)
{
  ReturnMsgStruct *retmsg=firstreturnmsg;

  while (retmsg && retmsg->sent) {
    ReturnMsgStruct *next=retmsg->next;
    Dealloc(retmsg);
    retmsg=next;
  }
  firstreturnmsg=retmsg;

  if (command) {
    while (retmsg) {
      ReturnMsgStruct *next=retmsg->next;
      if (!retmsg->sent && retmsg->command==command)
        break;
      retmsg=next;
    }
  }

  if (retmsg)
    retmsg->sent=TRUE;
  assert(retmsg == 0 || retmsg > 1024);
  return(retmsg);
}

void CopyWindowPos(struct WindowStruct *win)
{
  win->real_window_xpos=win->window_pointer->LeftEdge;
  win->real_window_ypos=win->window_pointer->TopEdge;
}

BOOL WindowActivated(BufStruct *Storage, struct IntuiMessage *msg)
{
  BOOL ret=FALSE;
  CursorXY(NULL, -1, -1);
  if (Storage && BUF(window)) {
      if (msg->IDCMPWindow->Flags&WFLG_WINDOWACTIVE) {
          if(!BUF(window->appwindow_pointer) && BUF(window->appwindow) && BUF(window->window)!=FX_SCREEN)
              BUF(window->appwindow_pointer) = AddAppWindow(0, NULL, BUF(window->window_pointer), WBMsgPort, NULL);

          AddFrexxAppIcon();
          ret=TRUE;
          {
              WindowStruct *win=FindWindow(msg->IDCMPWindow);
              if (win) {
                  NewStorageWanted=win->ActiveBuffer;
                  activewindow=msg->IDCMPWindow;
                  CursorXY(NULL, -3, 0);
              }
          }
      } else {
          activewindow=NULL;
          CursorXY(NULL, -2, 0);
      }
  } else {
      /* OBS: Cacha activeringen */
  }
  return ret;
}

void __chkabort(void) { }	// Disable break.

int SmartInput(int qual)
{
  if (qual&RAW_AMIGA) qual|=RAW_AMIGA;
  if (qual&RAW_CTRL)  qual|=RAW_CTRL;
  if (qual&RAW_ALT)   qual|=RAW_ALT;
  if (qual&RAW_SHIFT) qual|=RAW_SHIFT;
  return(qual&RAW_QUAL);
}


int ConvertKeys(char * here) {
    int chars = RawKeyConvert(&ievent, here, NUM_OF_CHARS, internalkeymap);
    here[chars] = '\0';
    return chars;
}

int ExamineKeyPress(BufStruct *Storage)
{
    static unsigned char rawkeybuf1[NUM_OF_CHARS]; /* This recieves the keymap string and is the result string */
    static int LastSeconds=0, LastMicros=0; /* Time for the last mousepress */
    BOOL keyconverted=FALSE;
    unsigned char rawkeybuf2[NUM_OF_CHARS];/* This recieves the keymap string */
    unsigned char rawkeybuf3[NUM_OF_CHARS];/* This recieves the keymap string */
    char *here, *here2, *here3;		/* Points to the rawkeybufs */
    int qual;
    int command=DO_NOTHING;
    struct Kmap *val;
    int chars;     /* number of chars recived from RawKeyConvert() */
    int rightchar;
    BOOL raus=FALSE;
    WindowStruct *win=FRONTWINDOW;
    
    val=Default.key.mul;
    if (BUF(window)) win=BUF(window);

    while(val) {
        if (!keyconverted) {
            if (rawkeypressed==RAWPRESSED) {
                here=rawkeybuf1;
                here2=rawkeybuf2;
                here3=rawkeybuf3;
                ievent.ie_Code=IDCMPmsg->Code;
                ievent.ie_Qualifier=IDCMPmsg->Qualifier&(~RAW_CAPS);
                ievent.ie_EventAddress= *((APTR *)IDCMPmsg->IAddress);
                chars = ConvertKeys(here);
                ievent.ie_Code=IDCMPmsg->Code;
                ievent.ie_Qualifier=NULL;
                ievent.ie_EventAddress= *((APTR *)IDCMPmsg->IAddress);
                ConvertKeys(here2);
                here3[0]='\0';
                if (IDCMPmsg->Qualifier&(RAW_SHIFT)) {
                    ievent.ie_Code=IDCMPmsg->Code;
                    ievent.ie_Qualifier=IDCMPmsg->Qualifier&(RAW_SHIFT);
                    ievent.ie_EventAddress= *((APTR *)IDCMPmsg->IAddress);
                    ConvertKeys(here3);
                }
                mouse_x=-1;
                mouse_y=-1;
            } else {
                Argv[0]=(char *)mousex+1;
                Argv[1]=(char *)mousey-BUF(top_offset)+2;
                commandflag=cmflag_POINTER;
                mouse_x=(int)Argv[0];
                mouse_y=(int)Argv[1];
                if ((rawkeypressed&(MOUSELEFT|MOUSEMIDDLE|MOUSERIGHT))==(lastrawkeypressed&(MOUSELEFT|MOUSEMIDDLE|MOUSERIGHT))) {
                    if (rawkeypressed&MOUSEUP)
                        rawkeypressed|=lastrawkeypressed&(MOUSEDOUBLEPRESS|MOUSETRIPPLEPRESS);
                    else {
                        if (mousex==oldmousex && mousey==oldmousey &&
                            !(rawkeypressed&MOUSEDRAG)) {
                            if (DoubleClick(LastSeconds, LastMicros, IDCMPmsg->Seconds, IDCMPmsg->Micros)) {
                                if (lastrawkeypressed & MOUSEDOUBLEPRESS)
                                    rawkeypressed=rawkeypressed|MOUSETRIPPLEPRESS;
                                else
                                    rawkeypressed|=MOUSEDOUBLEPRESS;
                            }
                        }
                    }
                }
                here=here2=here3=buffer;
                strcpy(buffer, "Mouse");
                if (rawkeypressed & MOUSERIGHT)
                    strcat(buffer, "Right");
                else if (rawkeypressed & MOUSEMIDDLE)
                    strcat(buffer, "Middle");
                else if (rawkeypressed & MOUSELEFT)
                    strcat(buffer, "Left");
                
                if (rawkeypressed & MOUSEUP)
                    strcat(buffer, "Up");
                else if (rawkeypressed & MOUSEDRAG)
                    strcat(buffer, "Drag");
                else if (rawkeypressed & MOUSEDOUBLEPRESS)
                    strcat(buffer, "Double");
                else if (rawkeypressed & MOUSETRIPPLEPRESS)
                    strcat(buffer, "Triple");
                else {
                    LastSeconds=IDCMPmsg->Seconds;
                    LastMicros=IDCMPmsg->Micros;
                }
                oldmousex=mousex;
                oldmousey=mousey;
                if ((IDCMPmsg->Qualifier&RAW_AMIGA) &&
                    (IDCMPmsg->Qualifier&RAW_ALT)) {
                    IDCMPmsg->Qualifier&=~(RAW_AMIGA|RAW_ALT);
                }
                lastrawkeypressed=rawkeypressed;
            }
            keyconverted=TRUE;
            qual=SmartInput(IDCMPmsg->Qualifier);
        }
        if ((val->Qual&RAW_AMIGA)==(qual&RAW_AMIGA)) {
            rightchar=0;
            if (((val->Qual&RAW_QUAL)==qual) && !(val->Qual&(~RAW_QUAL))) {
                if (val->string) {
                    if (strcmp(val->string, here2)==0)
                        rightchar=1;
                } else if (val->Code==IDCMPmsg->Code)
                    rightchar=1;
            } else {
                if (val->string) {
                    if (!(val->Qual&(RAW_SHIFT)) && qual&(RAW_SHIFT) && (val->Qual&(~RAW_SHIFT))==(qual&(~RAW_SHIFT))) {
                        if (here3[0] && strcmp(val->string, here3)==0)
                            rightchar=2;
                    } else {
                        if (!(val->Qual&(~RAW_AMIGA))) {
                            if (here[0] && strcmp(val->string, here)==0)
                                rightchar=2;
                        }
                    }
                }
            }
            
            if (rightchar) {
                if(!val->mul) {
                    register BOOL fine;
                    if (!val->depended || !val->depended[0])
                        fine=TRUE;
                    else
                        fine=CheckSetting(Storage, val->depended);
                    
                    if (fine) {
                        if (val->FPLstring) {		/* Exekvera en FPLstr�ng */
                            Argv[0]=val->FPLstring;
                            lockedalloc=val->FPLstring;
                            Argc=1;
                            command=DO_EXECUTE_STRING|NO_HOOK;
                            fpl_executer=EXECUTED_KEY;
                        } else
                            command=val->Command;		/* Vanligt kommando */
                        if (!(BUF(namedisplayed)&1))
                            Showstat(Storage);
                        
                        break;		// Break 'while(val)'
                    } else
                        val=val->next;
                } else {
                    ModifyIDCMP(win->window_pointer, IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MENUVERIFY|IDCMP_ACTIVEWINDOW|
                                IDCMP_INACTIVEWINDOW|IDCMP_CHANGEWINDOW);
                    buffer[0]=0;
                    if(val->Command)
            strcpy(buffer, (char *)val->Command);
          strcat(buffer, RetString(STR_WAITING_FOR_ANOTHER_KEY));
          Status(Storage, buffer, 3);
          commandflag=NULL;
          raus=FALSE;
          while(!raus) {
            if (!idcmpbuffer && IDCMPmsg) {
              ReplyMsg((struct Message *)IDCMPmsg);
            }
            idcmpbuffer=FALSE;
            if (IDCMPmsg=(struct IntuiMessage *)GetReturnMsg(cmd_KEY)) {
              IDCMPmsg=(struct IntuiMessage *)(((ReturnMsgStruct *)IDCMPmsg)->string);
              idcmpbuffer=TRUE;
            } else {
                IDCMPmsg=GetIntuiMsg();
                if (!IDCMPmsg) {
                    WaitForIntuiMsg();
                    IDCMPmsg=GetIntuiMsg();
              }
            }
            if (IDCMPmsg) {
              switch(IDCMPmsg->Class) {
              case IDCMP_CHANGEWINDOW:
                  handle_CHANGEWINDOW(FindWindow(IDCMPmsg->IDCMPWindow));
                  break;
              case IDCMP_NEWSIZE:
                  {
                      WindowStruct *win=FindWindow(IDCMPmsg->IDCMPWindow);
                      if (win && win->NextShowBuf) {
                          ReSizeWindow(win->NextShowBuf);
                      }
                  }
                  break;
              case IDCMP_ACTIVEWINDOW:
              case IDCMP_INACTIVEWINDOW:
                  WindowActivated(Storage, IDCMPmsg);
                  break;
              case IDCMP_RAWKEY:
                  if(!(IDCMPmsg->Code&128) &&
                     (IDCMPmsg->Code<RAWC_LEFT_SHIFT || IDCMPmsg->Code>RAWC_MIDDLE_MOUSE)) {
                      rawkeypressed=RAWPRESSED;
                      val=val->mul;
                      raus=TRUE;
                      keyconverted=FALSE;
                  }
                  break;
              case IDCMP_MOUSEBUTTONS:
                  switch(IDCMPmsg->Code) {
                  case SELECTUP:
                      /* IDCMP for the button released condition */
                      rawkeypressed=MOUSELEFT|MOUSEUP;
                      mousebutton=0;
                      break;
                  case SELECTDOWN:
                      /* IDCMP for the button pressed condition */
                      if (mousebutton!=1) {
                          rawkeypressed=MOUSELEFT;
                          mousebutton=1;
                          val=val->mul;
                          raus=TRUE;
                          keyconverted=FALSE;
                      }
                      break;
                  case MIDDLEDOWN:
                      if (mousebutton!=1) {
                          rawkeypressed=MOUSEMIDDLE;
                          mousebutton=1;
                          val=val->mul;
                          raus=TRUE;
                          keyconverted=FALSE;
                      }
                      break;
                  case MIDDLEUP:
                      rawkeypressed=MOUSEMIDDLE|MOUSEUP;
                      mousebutton=0;
                      break;
                  case MENUUP:
                      rawkeypressed=MOUSERIGHT|MOUSEUP;
                      mousebutton=0;
                      break;
                  }
                  break;
              case IDCMP_MENUVERIFY:
                  IDCMPmsg->Code=MENUCANCEL;
                  if (mousebutton!=1) {
                      rawkeypressed=MOUSERIGHT;
                      mousebutton=1;
                      val=val->mul;
                      raus=TRUE;
                      keyconverted=FALSE;
                      memcpy(&buffer[200], IDCMPmsg, sizeof(struct IntuiMessage));
                      ReplyMsg((struct Message *)IDCMPmsg);
                      IDCMPmsg=(struct IntuiMessage *)&buffer[200];
                      idcmpbuffer=TRUE;
                  }
                  break;
              case IDCMP_INTUITICKS:
                  if (ignoreresize)
                      ignoreresize--;
              default:             /* 960121 */
                  if (!idcmpbuffer) {
                      ReplyMsg((struct Message *)IDCMPmsg);
                  }
                  idcmpbuffer=FALSE;
                  IDCMPmsg=NULL;
                  break;
              }
            }
          }
                }
            } else
                val=val->next;
        } else
            val=val->next;
    }
    if (rawkeypressed==RAWPRESSED && !val && !raus &&
        !(IDCMPmsg->Qualifier&RAW_AMIGA)) {
        here=rawkeybuf1;
        if(chars) {
            if (IDCMPmsg->Qualifier & RAW_CAPS) {
                ievent.ie_Code=IDCMPmsg->Code;
                ievent.ie_Qualifier=IDCMPmsg->Qualifier;
                ievent.ie_EventAddress= *((APTR *)IDCMPmsg->IAddress);
                chars = ConvertKeys(here);
            }
            Argv[0]=here;
            Argv[1]=(char *)chars;
            Argc=2;
            command=SC_OUTPUT;
        }
    } else if (raus) {
        ModifyIDCMP(win->window_pointer, IDCMP1|(mousebutton?IDCMP_INTUITICKS:0));
        if (command==DO_NOTHING)
            Status(Storage, RetString(STR_NO_FUNCTION_ASSIGNED_TO_KEY), 3);
    }
    return (command);
}


char * WaitForKey(BufStruct *Storage)
{
    int command;
    char *result=buffer;
    
    GetKey(Storage, gkWAIT);
    IDCMPmsg=(struct IntuiMessage *)&buffer[100];
    idcmpbuffer=TRUE;
    Argv[0]=(char *)1;
    rawkeypressed=RAWPRESSED;
    command=ExamineKeyPress(Storage);
    if (command==(DO_EXECUTE_STRING|NO_HOOK))
        result=Argv[0];
    else
        MakeCall(command, Argc, Argv);
    return(result);
}

