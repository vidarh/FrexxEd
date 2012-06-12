/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**********************************************************
 *
 * OpenClose.c
 *
 * Open and close the screen and windows. Set up the gadgets.
 *
 *********/

#ifdef AMIGA
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/icon.h>
#include <proto/gadtools.h>
#include <proto/wb.h>
#include <devices/console.h>
#include <devices/keymap.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/semaphores.h>
#include <exec/types.h>
#include <graphics/displayinfo.h>
#include <graphics/gfx.h>
#include <graphics/gfxbase.h>
#include <graphics/view.h>
#include <graphics/gfxmacros.h>
#undef GetOutlinePen
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/preferences.h>
#include <intuition/screens.h>
#include <libraries/gadtools.h>
#ifndef NO_PPACKER
#include <libraries/ppbase.h>
#endif
#include <libraries/reqtools.h>
#include <libraries/locale.h>
#ifndef NO_XPK
#include <xpk/xpk.h>
#include <proto/xpkmaster.h>
#include <proto/xpksub.h>
#endif
#include <proto/graphics.h>
#ifndef NO_PPACKER
#include <proto/powerpacker.h>
#endif
#include <proto/locale.h>
#include <proto/FPL.h>
#include <proto/reqtools.h>
#include <proto/utility.h>
#include <pragmas/FPL_pragmas.h>
#include <utility/tagitem.h>
#include <workbench/workbench.h>
#else
#include "compat.h"
#endif

#include <setjmp.h>
#include <libraries/FPL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "Block.h"
#include "BufControl.h"
#include "BuildMenu.h"
#include "Change.h"
#include "Command.h"
#include "Cursor.h"
#include "Execute.h"
#include "FACT.h"
#include "Face.h"
#include "FrexxEd_rev.c"
#include "GetFont.h"
#include "GetFile.h"
#include "Icon.h"
#include "IDCMP.h"
#include "Init.h"
#include "Limit.h"
#include "MultML.h"
#include "KeyAssign.h"
#include "OpenClose.h"
#include "Rexx.h"
#include "Search.h"
#include "Setting.h"
#include "Slider.h"
#include "Startup.h"
#include "Strings.h"
#include "Timer.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"
#include "WindowOutput.h"
#ifdef USE_FASTSCROLL
#include "fastGraphics.h"
#include "fastGraphics_pragma.h"
#include "fastGraphics_proto.h"
#endif

#include "Palette.h"
#include "Process.h"

extern struct Library *XpkBase;

extern struct Menu *Menus;
extern long FrexxPri;
extern void *Anchor;
extern BOOL hook_enabled;

extern struct SignalSemaphore LockSemaphore;
extern DefaultStruct Default;
//extern struct ExecBase *SysBase;
extern char buffer[];
extern int bufferlen;
extern HistoryStruct SearchHistory;
extern struct TextFont *SystemFont;	/* Font used by the system/screen */
extern struct TextFont *RequestFont;	/* Font used by requsters */
extern struct TextFont *DefaultFont;	/* Pointer for the DefaultFont */
extern int systemfont_leftmarg;
extern int systemfont_rightmarg;
extern BlockStruct *YankBuffer;
extern BlockStruct MacroBuffer;
extern char *KeyMapRemember;
extern char *cl_frexxecute;
extern char *cl_portname;
extern char *cl_pubscreen;
extern char *cl_run;
extern char *cl_init;
extern int cl_iconify;
extern FACT *DefaultFact;
extern BOOL OwnWindow;
extern struct Window *activewindow;	// Current active FrexxEd window
extern int visible_height;
extern int visible_width;

extern FACT *UsingFact;		/* Current using FACT.  A free pointer, changed without notice */

extern char *version;

extern BOOL open_copywb;
extern struct Window *InfoWindow;
extern srch Search;          /* search structure */
extern struct Library *FastGraphicsBase;
extern struct FastGraphicsTable *fast_gfx_table;

#ifdef AMIGA
extern struct Library * WorkbenchBase;
extern struct Library * UtilityBase;
extern struct Library * LocaleBase;
extern struct Library *FPLBase;
extern struct Library *DiskfontBase;
extern struct Library *GadToolsBase;
#endif

extern struct rtFileRequester *FileReq; /* Buffrad Filerequester. Bra o ha! */
extern struct IOStdReq *WriteReq;
extern struct MsgPort *WritePort, *ReadPort;
extern struct MsgPort *WindowPort;
extern struct Process *editprocess;
extern struct Library *ConsoleDevice; /* Changed definition in 6.0! */
extern struct PPBase *PPBase;
extern struct ExecBase *execbase;
extern WORD charbredd, charhojd, baseline;
extern struct Library *LocalBase;
extern struct Catalog *catalog;

extern int Visible;
extern int UpDtNeeded;
extern char *FrexxName;
extern char *screen_title;

extern struct MsgPort *msgport;			/* msgport for STICK */
extern struct MsgPort *msgreply;
extern struct Message msgsend;

extern struct Library *IconBase;
extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct ReqToolsBase *ReqToolsBase;
extern BlockStruct *BlockBuffer;

extern struct screen_buffer ScreenBuffer;
extern struct Window *iconw;

extern int BarHeight;
extern int BorderWidth, BorderHeight;
extern struct TextFont *font;
extern struct TagItem Taggis[];

extern unsigned char *title;

extern struct ExtNewWindow newwindow;

extern struct TextAttr topazfont;
extern int requestfontwidth;		// Kalkulerad bredd på requestfonten.
extern char *statusbuffer;

extern char DebugOpt; /* debug on/off */
extern struct MsgPort *WBMsgPort;
extern struct AppIcon *appicon;
extern char *appiconname;
extern struct DiskObject AppIconDObj;
extern struct DiskObject *externappicon;

extern struct RastPort ButtonRastPort;	//Rastport för att kunna testa med Requestfonten.
extern char *mothername;

extern const int nofuncs;
extern struct FrexxEdFunction fred[];

extern int signalbits; /* Which signal bits to Wait() for! */
extern AREXXCONTEXT RexxHandle;

extern UWORD zoom[];
extern int zoomstate;
extern char *lastprompt;	/* Innehållet i senaste promptningen. */
extern char GlobalEmptyString[];

extern int redrawneeded;
extern WORD sliderhighlite, statuscolor, statustextcolor;
extern char bitplanes;
extern char FrexxEdStarted;	/* Är FrexxEd uppstartad */
extern char ignoreresize;
extern char fastmap1[];
extern char fastmap2[];
extern struct MsgPort *TimerMP;
extern char waitreq;
extern BOOL match_for_fastscroll;
extern jmp_buf return_stackpoint;

extern struct KeyMap *internalkeymap;

/* from filehandler: */
extern char filehandlerstop;
extern struct Task *filehandlertask;
extern struct ProcMsg *filehandlerproc;
/* end of filehandler variables */

static struct Screen *screenwindow; /* Screen to put window on if WINSCREEN is set */
static char *screenwindow_name=NULL;

#ifdef alloctest
extern int total_fpl_alloc;
#endif

#include <assert.h>

extern struct Setting **sets;

extern int ExecVersion;

/*** PRIVATE ***/

static int clipri=-200;
static APTR oldwindowptr = NULL;
static UWORD DRI[]={ 65535 };

static void TestScreenMode(WindowStruct *win);
static void AdjustBufsInWindow(WindowStruct *win);
static char *OpenMyScreen(WindowStruct *win);

/*************/

/*
  If FrexxEd can't close a screen, the pointer is added to a list,
  and it will be attempted freed later.
*/
static BOOL CleanUpScreens(struct Screen *add)
{
    struct screen_list {
        struct screen_list *next;
        struct Screen *pointer;
    };
    static struct screen_list *list = 0;
    struct screen_list *temp;
    if (add) {
        temp=list;	// Hindra att samma pointer läggs upp två gånger
        while (temp) {
            if (temp->pointer==add)
                return TRUE;
        }
        temp=Malloc(sizeof(struct screen_list));
        if (temp) {
            temp->pointer=add;
            temp->next=list;
            list=temp;
        }
    } else {
        struct screen_list *prev=NULL;
        temp=list;
        while (temp) {
            if (CloseScreen(temp->pointer)) {
                if (prev)
                    prev->next=temp->next;
                else
                    list=temp->next;
                Dealloc(temp);
                temp=list;
            }
            prev=temp;
            if (temp)
                temp=temp->next;
        }
    }
}

static BOOL FrexxEdCloseScreen(struct Screen *screen)
{
  while (!CloseScreen(screen)) {
    if (0==rtEZRequest(RetString(STR_FREXXED_CAN_CONTINUE_BEFORE), RetString(STR_TRY_AGAIN), NULL, NULL,
			RT_Screen, screen,
			RT_TextAttr, &Default.RequestFontAttr,
			RTEZ_ReqTitle, RetString(STR_IMPORTANT),
			TAG_END)) {
      CleanUpScreens(screen);  // Addera till flushlistan.
      return FALSE;
	}
  }
  return TRUE;
}

static void __regargs CloseScreenWindow(void)
{
  if (screenwindow) {
    FrexxEdCloseScreen(screenwindow);
    Dealloc(screenwindow_name);
    screenwindow=NULL;
    screenwindow_name=NULL;
  }
}

void FirstOpen() {
  editprocess = (struct Process *)FindTask(NULL);
  oldwindowptr = editprocess->pr_WindowPtr;

  InitRastPort(&ButtonRastPort);

  appiconname=Strdup("FrexxEd");

#ifdef AREXX
  RexxHandle=InitARexx(cl_portname);
  Default.ARexxPort=RexxHandle?RexxHandle->PortName:""; /* Get ARexx port name */
#endif

#ifdef FIXME
  if (InitFPL(1)) CloseAll(RetString(STR_INIT_FPL));
#endif

#ifdef AMIGA
  /* Create the message port to which Workbench can send messages */
  if(!(WBMsgPort = CreateMsgPort()) ) CloseAll(RetString(STR_CREATE_MESSAGE_PORT));

  /* create reply port and io block for reading from console */
  if (!(ReadPort=CreateMsgPort())) CloseAll(RetString(STR_CREATE_READ_PORT));

  /* create reply port and io block for reading from console */
  if (!(WindowPort=CreateMsgPort())) CloseAll(RetString(STR_CREATE_READ_PORT));

  /* create reply port and io block for writing to console */
  if (!(WritePort=CreateMsgPort())) CloseAll(RetString(STR_CREATE_WRITE_PORT));
  WriteReq=(struct IOStdReq *)CreateIORequest(WritePort, (LONG)sizeof(struct IOStdReq));
  if(!WriteReq) CloseAll(RetString(STR_CREATE_WRITE_REQUEST)); 

  if (OpenDevice("console.device", -1, (struct IORequest *)WriteReq, 1))
    CloseAll(RetString(STR_OPEN_CONSOLE_DEVICE));
  ConsoleDevice=(struct Library *)WriteReq->io_Device;
#endif

  InitKeys();

#ifdef FIXME
  if (!(DefaultFact=InitFACT())) CloseAll(RetString(STR_GET_MEMORY));
#endif

  Default.BufStructDefault.using_fact=UsingFact=DefaultFact;

  LoadKeyMap();

  BlockBuffer=AllocBlock("DefaultBlock");
  if (!BlockBuffer) CloseAll(RetString(STR_GET_MEMORY));
  BlockBuffer->freeable=FALSE;
  BlockBuffer->type|=type_HIDDEN;
  Default.FirstBlock=BlockBuffer;

  YankBuffer=AllocBlock("YankBuffer");
  if (!YankBuffer) CloseAll(RetString(STR_GET_MEMORY));
  YankBuffer->freeable=FALSE;
  YankBuffer->type|=type_HIDDEN;

  Search.buf.fastmap = fastmap1;
  Search.second_buf.fastmap = fastmap2;
  Search.flags=FORWARD|PROMPT_REPLACE|LIMIT_WILDCARD;
  Default.search_flags=Search.flags;
  Search.lastsearchstring=GlobalEmptyString;

  title=Malloc(140);
  if (!title) CloseAll(RetString(STR_GET_MEMORY));
  title[0]=0;
  externappicon = GetDiskObject(FREXXED_ICONDIR FREXXED_APPICON);

  InitSemaphore(&LockSemaphore);
  ObtainSemaphore(&LockSemaphore);
}

static void TestScreenMode(WindowStruct *win)
{
  ULONG mode;
  if (win->screen_pointer) {
    register struct DisplayInfo *display=(struct DisplayInfo *)buffer;
    mode=GetVPModeID(&win->screen_pointer->ViewPort);
    GetDisplayInfoData(0, (void *)display, sizeof(struct DisplayInfo),
                       DTAG_DISP, mode);
    if (display->PropertyFlags&DIPF_IS_FOREIGN)  // returnerar om skärmentypen är okänd.
      win->graphic_card=TRUE;
  }
}


void LastOpen() {
    char *ret;
    WindowStruct *win;
    
    if (FrexxPri>-128)
        Default.taskpri=FrexxPri;
    clipri=SetTaskPri((struct Task *)editprocess, Default.taskpri);
    
    win=MakeNewWindow(&Default.WindowDefault);
    
    if (ret=OpenMyScreen(win)) {
        fprintf(stderr,"OpenMyScreen failed\n");
        CloseAll(ret);
    }
        fprintf(stderr,"OpenMyScreen succeeded\n");    
    if (cl_iconify || win->iconify) {
        if (Iconify(NULL)!=OK) CloseAll(RetString(STR_OPEN_SCREEN));
    }
    
    fprintf(stderr,"HERE 3\n");
#ifdef FIXME
    if (InitFPL(0)) CloseAll(RetString(STR_INIT_FPL));
#endif
    
    if (Default.windows_opened && !ScreenBuffer.beginning)
        CloseAll(RetString(STR_GET_MEMORY));
        fprintf(stderr,"HERE 4\n");
    PrintScreenInit();
    InitColors(win);
        fprintf(stderr,"HERE 5\n");
}


char *OpenLibraries()
{
#ifdef AMIGA
  if (!(IntuitionBase = (struct IntuitionBase *)OpenLibrary("intuition.library", LIB_REV)))
	return("open intuition.library");
  
  if (!(GfxBase= (struct GfxBase *)OpenLibrary("graphics.library", LIB_REV)))
    return("open graphics.library");
  
  if (!(GadToolsBase = OpenLibrary("gadtools.library", 36L)))
    return("open gadtools.library");

  if (!(DiskfontBase = OpenLibrary("diskfont.library", 36L)))
    CloseAll("open diskfont.library");

  if (!(ReqToolsBase= (struct ReqToolsBase *)
        OpenLibrary(REQTOOLSNAME, REQTOOLSVERSION)))
    return("open reqtools.library V38+");

  if (!(FileReq = (struct rtFileRequester *)rtAllocRequestA(RT_FILEREQ, NULL)))
    return(RetString(STR_GET_MEMORY));
  
#ifndef NO_PPACKER
  PPBase = (struct PPBase *)OpenLibrary ("powerpacker.library", 20);
#endif
#ifndef NO_XPK
  XpkBase = (struct Library *)OpenLibrary (XPKNAME, 0);
#endif

  if (!(WorkbenchBase = OpenLibrary("workbench.library", 36)))
    CloseAll("open workbench.library");

  if (!(UtilityBase = OpenLibrary("utility.library", 36)))
    CloseAll("open utility.library");

  LocaleBase = OpenLibrary("locale.library", 38);
  if(LocaleBase) {
    catalog = OpenCatalogA(NULL, "FrexxEd.catalog", NULL);
  }

  if (!(IconBase = OpenLibrary("icon.library", 36)))
    CloseAll("open icon.library");

#ifdef NO_STATIC_FPL
  FPLBase=OpenLibrary("fpl.library", 13); /* open fpl.library version 13 */
  if (!FPLBase || (FPLBase->lib_Version==13 && FPLBase->lib_Revision<0))
    return("open fpl.library v13");
#endif
#ifdef USE_FASTSCROLL
  FastGraphicsBase=OpenLibrary(FastGfxName, 1);
  if (!FastGraphicsBase)
    FastGraphicsBase=OpenLibrary("ProgDir:libs/"FastGfxName, 1);
#endif
  if (InitTimer()<OK) CloseAll("open timer.device");
#endif
  return(NULL);
}


/****************************************************************
 *
 * Close down all libraries for this task.  A special requester
 * can appear if a string is given.
 ********/
void CloseLibraries(char *string)
{
  if (string && IntuitionBase) {
      char *sstring;
      sstring=(char *)Malloc(strlen(string)+40);
      if (sstring) {
          struct EasyStruct req = {
              sizeof(struct EasyStruct),0,
              FREXXED_VER, NULL, NULL
          };
          req.es_TextFormat=sstring;
          req.es_GadgetFormat=RetString(STR_OK_GADGET);
          
          Sprintf(sstring, RetString(STR_FREXXED_COULDNT), string);
          EasyRequestArgs(NULL, &req, NULL, NULL);
          
          Dealloc(sstring);
      }
  }
  FreeTimer();

  if (FileReq)
    rtFreeRequest((APTR)FileReq);

#ifdef AMIGA
#ifdef USE_FASTSCROLL
  if(FastGraphicsBase)
    CloseLibrary(FastGraphicsBase);
#endif
  if (ReqToolsBase)
    CloseLibrary((struct Library *)ReqToolsBase);

  if(IconBase)      CloseLibrary(IconBase);
  if(catalog)       CloseCatalog(catalog);
  if(LocaleBase)    CloseLibrary(LocaleBase);
  if(UtilityBase)   CloseLibrary((struct Library *)UtilityBase);
  if(WorkbenchBase) CloseLibrary((struct Library *)WorkbenchBase);
#ifdef NO_STATIC_FPL
  if (FPLBase) {
    CloseLibrary(FPLBase);
  }
#endif
  if (DiskfontBase)  CloseLibrary((struct Library *)DiskfontBase);
  if (GadToolsBase)  CloseLibrary((struct Library *)GadToolsBase);
  if (PPBase)        CloseLibrary ((struct Library *)PPBase);
  if (XpkBase)       CloseLibrary ((struct Library *)XpkBase);
  if (GfxBase)       CloseLibrary((struct Library *)GfxBase);
  if (IntuitionBase) CloseLibrary((struct Library *)IntuitionBase);
#endif
}


/**********************************************************************
 *
 * CloseAll()
 *
 * Close the scren, the windows, the buffers and everything else and
 * then exit this process!
 *
 ****/

void CloseAll(char *string)
{
    CloseFrexxEd(string);
    longjmp(return_stackpoint, 1);
}

void CloseFrexxEd(char *string)
{
    fprintf(stderr,"CloseFrexxEd '%s'\n",string);
    Visible=VISIBLE_OFF;
    hook_enabled=FALSE;
    if (editprocess) /* restore old system request windowpointer */
        editprocess->pr_WindowPtr = oldwindowptr;

#ifdef AMIGA
    if (ReadPort) DeleteMsgPort(ReadPort);
    if (WriteReq) {
        if (WriteReq->io_Device)
            CloseDevice((struct IORequest *)WriteReq);
        DeleteIORequest(WriteReq);
    }
    if (WritePort) DeleteMsgPort(WritePort);
#endif
        fprintf(stderr,"XXXXXXXXXXXXXXXXXX ==========\n");
        assert(sets != 0);

    /* free the entire linked lists of menu information! */
    menu_delete(&menu, menu.ownmenu);

    CleanupAllFaces(); /* Delete all faces */

    if (InfoWindow) CloseWindow(InfoWindow);
    if(appicon) RemoveAppIcon(appicon);
    if(externappicon) FreeDiskObject(externappicon);

#ifdef AMIGA
    if(WBMsgPort) {
        struct Message *msg;
        while (msg=GetMsg(WBMsgPort))
            ReplyMsg(msg);
        DeleteMsgPort(WBMsgPort);
    }
#endif

  {
    while (FRONTWINDOW) {
      if(FRONTWINDOW->appicon) {
        RemoveAppIcon(FRONTWINDOW->appicon);
        FRONTWINDOW->appicon=NULL;
      }
      CloseMyScreen(FRONTWINDOW);
      FreeWindow(FRONTWINDOW);
    }
    CloseScreenWindow();
    CleanUpScreens(NULL);  // Rensa upp alla kvarglömda skärmar
  }
	/* Wait until all sub processes are finnished */
#ifdef MULTIPROCESS
//  if (Default)
    while (Default.processes>1);
#endif

  if (DefaultFont) CloseFont(DefaultFont);
  if (WindowPort)  DeleteMsgPort(WindowPort);
  if (Default.KeyMapseg) UnLoadSeg(Default.KeyMapseg);
  if (Default.olddirectory!= -1) UnLock(CurrentDir(Default.olddirectory));

#ifndef POOL_DEALLOC

  /*
   * Free all hooks !
   */
  if (Default.hook) {
    register int a;
    register struct FrexxHook *hook, *hookn;
    for(a=0; a<MAX_COMMANDS; a++) {
      hook=Default.hook[a];
      while(hook) {
        hookn=hook->next;
        Dealloc(hook->name);
        Dealloc(hook);
        hook=hookn;
      }
    }
  }
  if (Default.posthook) {
      struct FrexxHook *hook, *hookn;
      int a;
      for(a=0; a<MAX_COMMANDS; a++) {
          hook=Default.posthook[a];
          while(hook) {
              hookn=hook->next;
              Dealloc(hook->name);
              Dealloc(hook);
              hook=hookn;
          }
      }
  }

  if (Default.FirstBlock) {
    Default.FirstBlock->freeable=TRUE;
    Free(Default.FirstBlock->Entry, FALSE, FALSE);
  }
  if (YankBuffer) {
    YankBuffer->freeable=TRUE;
    Free(YankBuffer->Entry, FALSE, FALSE);
  }
  DeleteSetting(NULL);	// Delete all settings
#ifdef LOG_FILE_EXECUTION
  DeleteFileExecutionList();
#endif

  /* Deleata SearchHistoryn */
  while(SearchHistory.strings)
    Dealloc(SearchHistory.text[--SearchHistory.strings]);
  Dealloc((char *)SearchHistory.text);

  Dealloc(Search.buf.buffer);
  Dealloc(Search.second_buf.buffer);
  Dealloc(Search.buffer);
  Dealloc(Search.secondbuffer);
  Dealloc(Search.realbuffer);
  Dealloc(Search.repbuffer);
  Dealloc(Search.realrepbuffer);
  Dealloc(Search.lastsearchstring);

  Dealloc(statusbuffer);

  Visible=VISIBLE_OFF;

  Dealloc(lastprompt);

  ClearBlock(&MacroBuffer);

  Dealloc(ScreenBuffer.beginning);
  Dealloc(title);
  Dealloc(cl_init);

  while (GetReturnMsg(0));	/* Töm ReturnMsg-porten */

  if (DefaultFact) {
    register FACT *factcount=DefaultFact;
    do {
      factcount=ClearFACT(factcount);
    } while (factcount);
  }

  Dealloc(appiconname);

//  if (Default)
  {
    Dealloc(Default.directory);
    Dealloc(Default.status_line);
    Dealloc(Default.GlobalInfo);
    Dealloc(Default.SharedDefault.LokalInfo);
    Dealloc(Default.font.ta_Name);
    Dealloc(Default.RequestFontAttr.ta_Name);
    Dealloc(Default.defaultfile);
    Dealloc(Default.WindowDefault.PubScreen);
    Dealloc(Default.WindowDefault.FrexxScreenName);
    Dealloc(Default.KeyMap);
  
    Dealloc(Default.SystemFont);
    Dealloc(Default.RequestFont);
    Dealloc(Default.StartupFile);
    Dealloc(Default.workstring);
    Dealloc(Default.FPLdirectory);
    Dealloc(Default.file_pattern);

    Dealloc(Default.diskname);
    Dealloc(Default.WindowDefault.window_title);

    Dealloc(Default.SharedDefault.comment_begin);
    Dealloc(Default.SharedDefault.comment_end);
    Dealloc(Default.SharedDefault.filnamn);
    Dealloc(Default.SharedDefault.path);
    Dealloc(Default.SharedDefault.macrokey);
    Dealloc(Default.SharedDefault.face_name);
    Dealloc(Default.BufStructDefault.fact_name);
    menu.ownmenu=NULL;
    while (Default.key.mul)
      DeleteKmap(Default.key.mul);
  }
  DeleteLogSetting();

  FreeCache();

		// End if to POOLS
#endif
  if (Anchor)
    fplFree(Anchor);
  FreeARexx(RexxHandle);
  CloseLibraries(string);

#ifdef alloctest
#ifndef POOL_DEALLOC
  {
    extern char *firstalloc;
    register char *mem=firstalloc;
    while (mem) {
      FPrintf(Output(), "\n\a"FREXXED_VER":Dealloc forgotten file '%s' l %ld, no %ld! ",
              ((AllocStruct *)mem)->file,
              ((AllocStruct *)mem)->line,
              ((AllocStruct *)mem)->number);
      {
        int count=0;
        for (count=0; count<10; count++) {
          char tkn=((char *)(&(((AllocStruct *)mem)[1])))[count];
          if (tkn>32 && tkn<127)
            FPrintf(Output(), "%c", tkn);
          else
            FPrintf(Output(), " %02lx", tkn);
        }
      }
      FPrintf(Output(), "\n");
      mem=((AllocStruct *)mem)->Next;
    }
  }
#endif
#endif

  if (clipri>-200)
    SetTaskPri((struct Task *)editprocess, clipri);
  FindTask(NULL)->tc_Node.ln_Name=mothername;

#ifdef USE_FASTSCROLL
  if (match_for_fastscroll) {
    if (fast_gfx_table) {
      fgFree(fast_gfx_table);
      fast_gfx_table=NULL;
    }
  }
#endif

  if(filehandlerproc) {
    /*
     * We might actually get here before the filehandler is 100%
     * started and then we don't have any task to signal. Then
     * we simply set its termination variable. It will finally
     * make the filehandler surrender!!!
     *
     */
    if(filehandlertask)
      Signal(filehandlertask, SIGBREAKF_CTRL_C);
    else
      filehandlerstop=TRUE;
    wait_process(filehandlerproc); /* wait here until it dies! */
  }

  KillAlloc(); /* removes all memory pools! */
}

void PrintScreenInit(void)
{
    if (SystemFont) {
        charhojd=SystemFont->tf_YSize;
        charbredd=SystemFont->tf_XSize;
        baseline=SystemFont->tf_Baseline;
    } else fprintf(stderr,"FIXME: No SystemFont\n");
}

static void CreateScreenName(WindowStruct * win, char * buffer) {
    struct List *list=LockPubScreenList();
    int counter=0;
    do {
        counter++;
        Sprintf(buffer, "FrexxEdScreen%ld", counter);
    } while (list && FindName(list, buffer));
    if (list) UnlockPubScreenList();
}


static char *OpenMyScreen(WindowStruct *win)
{
  struct NewWindow newwindow = {
    0, 0,                        /* left and top edge */
    0,                           /* width */
    0,                           /* height */
    3, 1,                        /* detail-, blockpen */
    NULL,		         /* IDCMP */
    WFLG_BACKDROP|WFLG_BORDERLESS|WFLG_ACTIVATE|WFLG_REPORTMOUSE, /* flags */
    NULL,                        /* gadget */
    NULL,                        /* image */
    NULL,                        /* title */
    NULL,                        /* (Screen *) */
    NULL,                        /* bitmap */
    NULL,NULL,                     /* minsizes */
    65535, 65535,                /* maxsizes */
    CUSTOMSCREEN                 /* type */
  };
  char *pubscreenname;
  static char firstopen=TRUE;

  {
    win->ps_frontmost=FALSE;
    win->pubscreen=LockPubScreen(pubscreenname=win->PubScreen);
    if (!win->pubscreen) {
      if (!Stricmp(pubscreenname, "frontmost")) {
        win->pubscreen=IntuitionBase->FirstScreen;
        win->ps_frontmost=TRUE;
      } else {
        pubscreenname="";
        win->pubscreen=LockPubScreen(NULL);
        if (!win->pubscreen)
          return(RetString(STR_GET_PUBLIC_SCREEN));
      }
    }
  }
  if (!win->ownscreen && win->screen_pointer)
    UnlockPubScreen(NULL, win->screen_pointer);

  if (open_copywb) CloneWB(win);

  if ((!open_copywb || !SystemFont) && firstopen) {

    if (GetFont(NULL, Default.SystemFont, 1)<0) {
      Dealloc(Default.font.ta_Name);
      memcpy(&Default.font, &topazfont, sizeof(struct TextAttr));
      Default.font.ta_Name=Strdup(topazfont.ta_Name);
      DefaultFont=OpenFont(&Default.font);
      Dealloc(Default.SystemFont);
      Default.SystemFont=Strdup("topaz.font 8");
    }

    SystemFont=DefaultFont;

    if (GetFont(NULL, Default.RequestFont, 2)<0) {
      Dealloc(Default.RequestFontAttr.ta_Name);
      memcpy(&Default.RequestFontAttr, &topazfont, sizeof(struct TextAttr));
      Default.RequestFontAttr.ta_Name=Strdup(topazfont.ta_Name);
      RequestFont=OpenFont(&Default.RequestFontAttr);
      requestfontwidth=8;
      Dealloc(Default.RequestFont);
      Default.RequestFont=Strdup("topaz.font 8");
    }
  }

  win->real_screen_height=win->screen_height;
  win->real_screen_width=win->screen_width;
  win->real_window_height=win->window_height;
  win->real_window_width=win->window_width;
  win->real_window_xpos=win->window_xpos;
  win->real_window_ypos=win->window_ypos;

  if (!cl_iconify && !win->iconify) {
    if (win->window!=FX_SCREEN) {
      newwindow.Height=win->real_window_height;
      newwindow.Width=win->real_window_width;
      newwindow.TopEdge=win->real_window_ypos;
      newwindow.LeftEdge=win->real_window_xpos;
  
      if (win->window_position==WINDOW_VISIBLE) {
        newwindow.TopEdge-= win->pubscreen->ViewPort.DyOffset; /* offset is negative */
        newwindow.LeftEdge-=win->pubscreen->ViewPort.DxOffset;/* offset is negative */
      }
    } else {
      newwindow.Height=win->real_screen_height;
      newwindow.Width=win->real_screen_width;
    }
    if (!open_copywb && win->pubscreen) {
      register ULONG mode=GetVPModeID(&win->pubscreen->ViewPort);
      GetDimension(mode);
    }
    open_copywb=FALSE;
    Sprintf(title, screen_title, FrexxName);
  }

  /* open screen */
  if (!cl_iconify && !win->iconify) {
    if (win->window==FX_SCREEN || win->window==FX_WINSCREEN) {
     if (win->window==FX_WINSCREEN && screenwindow) {
         win->screen_pointer=screenwindow;
         Dealloc(win->FrexxScreenName);
         win->FrexxScreenName=Strdup(screenwindow_name);
         win->ownscreen=FALSE;
      } else {
         int screen_depth=win->screen_depth;

         CreateScreenName(win,buffer);

         win->screen_pointer = OpenScreenTags(NULL,
    			    SA_DisplayID, win->DisplayID,
    			    SA_Height, win->real_screen_height,
    			    SA_Width, win->real_screen_width,
    			    SA_Type, CUSTOMSCREEN,
    			    SA_PubName, buffer,
    			    SA_Font, &Default.RequestFontAttr, //screenfont,
    			    SA_AutoScroll, win->autoscroll,
    			    SA_Overscan, win->OverscanType,
    			    SA_Interleaved, TRUE,
    			    SA_Depth, screen_depth,
    			    SA_LikeWorkbench, TRUE,
                  (ExecVersion >= 39) ? TAG_END :
    	    /* Below only for v37 and below */
    			    SA_DetailPen, 3,
    			    SA_BlockPen, 1,
    			    SA_Pens, (ULONG)&DRI,
    			    TAG_DONE);

         if(win->screen_pointer) {
             /* FIXME: Warn using RetString(STR_OPEN_SCREEN) if !win->screen_pointer */

             if (win->window==FX_WINSCREEN) {
                 screenwindow=win->screen_pointer;
                 screenwindow_name=Strdup(buffer);
             }
    
             Dealloc(win->FrexxScreenName);
             win->FrexxScreenName=Strdup(buffer);
             GetDimension(win->DisplayID);
             
             if (win->window==FX_WINSCREEN)
                 win->ownscreen=FALSE;
             else
                 win->ownscreen=TRUE;
             
             PubScreenStatus(win->screen_pointer, 0);
             if (win->pubscreen) {
                 if (!win->ps_frontmost)
                     UnlockPubScreen(NULL, win->pubscreen);
                 win->pubscreen=NULL;
             }

             TestScreenMode(win);
             assert(win->screen_pointer > 1000);
             win->real_screen_height=win->screen_pointer->Height;
             win->real_window_height=win->screen_pointer->Height;
             win->real_screen_width=win->screen_pointer->Width;
             win->real_window_width=win->screen_pointer->Width;
         } else win->window = FX_WINDOW; /* Fallback */
     
     }
    }
    if (win->window!=FX_SCREEN && win->window!=FX_WINSCREEN) {
        /* Open on existing pubscreen */
        win->real_screen_height=visible_height;
        win->real_window_height=visible_height;
        win->real_screen_width=visible_width;
        win->real_window_width=visible_width;
        win->screen_pointer=win->pubscreen;
        Dealloc(win->FrexxScreenName);
        win->FrexxScreenName=Strdup(pubscreenname);
    }

    if (win->window==FX_SCREEN) {
      newwindow.Height=win->real_window_height;
      newwindow.Width=win->real_window_width;
    }
  
    /* attach window to the screen */
    newwindow.Screen = win->screen_pointer;
  
    BarHeight=0;
    if (win->window!=FX_BACKDROP)
      BarHeight=win->screen_pointer->BarHeight;

    SetupMinWindow(win);
    if (win->window!=FX_SCREEN) {
      if (win->real_window_width<win->window_minwidth)
        newwindow.Width=win->window_width=win->window_minwidth;
      if (win->real_window_height<win->window_minheight) {
        newwindow.Height=win->real_window_height=win->window_minheight;
      }
      if ((win->window&FX_WINDOWBIT) && win->autoresize &&
          BorderWidth>=0 && BorderHeight>=0) {
        register int dx;
        register int dy;
        dx=(newwindow.Width - BorderWidth -
            systemfont_leftmarg-systemfont_rightmarg)%SystemFont->tf_XSize;
        newwindow.Width-=dx;
        dy=(newwindow.Height - BorderHeight)%SystemFont->tf_YSize;
        newwindow.Height-=dy;
      }
    }
    newwindow.MinWidth=win->window_minwidth;
    newwindow.MinHeight=win->window_minheight;
  } else {
      win->screen_pointer=win->pubscreen;
  }
  if (win->visualinfo)
    FreeVisualInfo(win->visualinfo);
  if (!(win->visualinfo=GetVisualInfo(win->screen_pointer, TAG_END)))
    return(RetString(STR_GET_VISUALINFO));

  if (!cl_iconify && !win->iconify) {
  /* open window */
    if (win->window&FX_WINDOWBIT) {
      newwindow.Flags=WFLG_ACTIVATE|WFLG_REPORTMOUSE|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_CLOSEGADGET|WFLG_NEWLOOKMENUS;
    }
    {
      char *str=win->window_title;
      char *str_s=Default.WindowDefault.window_title;
      if (!*str_s)
        str_s=title;
      if (!*str)
        str=str_s;
#if 1 /* FIXME: Unsure if this is meant to run if the registration key has been found or if it hasn't. Test */
#warning Remnants of keyfile stuff
      //if (Default.BufStructDefault.reg.reg) {
        str=title;
        str_s=title;
		//}
#endif
      if (!(win->window_pointer=(struct Window *)
                                    OpenWindowTags(&newwindow,
                                                   WA_AutoAdjust, TRUE,
                                                   WA_NoCareRefresh, TRUE,
                                                   WA_NewLookMenus, TRUE,
                                                   WA_ScreenTitle, str_s,
              /* Stop om window==backdrop */                                               
                                !(win->window&FX_WINDOWBIT)?TAG_DONE:WA_SizeGadget, TRUE,
                                                   WA_Title, str,
                                                   WA_SizeBRight, TRUE,
                                                   WA_SizeBBottom, FALSE,
                                                   WA_Zoom,  (ULONG)&zoom,
                                                  /* WA_SimpleRefresh, TRUE, */
                                                   TAG_DONE))) {
        return(RetString(STR_OPEN_WINDOW));
      }
    }
    Default.windows_opened++;
    waitreq=0;
    win->real_window_width=win->window_pointer->Width;
    win->real_window_height=win->window_pointer->Height;
  
    if (win->window==FX_BACKDROP)
	  WindowToFront(win->window_pointer);
    zoomstate=win->window_pointer->Flags&WFLG_ZOOMED;
    activewindow=win->window_pointer;
    win->window_pointer->UserData="FrexxEd";
  
    SetupMinWindow(win);
  } else {
    win->iconify=TRUE;
    win->Visible=VISIBLE_ICONIFIED;
  }
  if (win->window!=FX_SCREEN && win->window!=FX_WINSCREEN) {
    struct PubScreenNode *psnode;
    pubscreenname="";
    if ( win->screen_pointer->Flags & WBENCHSCREEN )
      pubscreenname = "Workbench" ;
    else {
      if ( win->screen_pointer->Flags & PUBLICSCREEN ) {
        psnode=(struct PubScreenNode *)LockPubScreenList();
        while ((psnode) && !(pubscreenname[0])) {
          if ((psnode->psn_Flags != PSNF_PRIVATE) && (psnode->psn_Screen == win->screen_pointer)) {
            pubscreenname = psnode->psn_Node.ln_Name;
            break;
          }
          psnode = (struct PubScreenNode *)psnode->psn_Node.ln_Succ;
        }
        UnlockPubScreenList();                
      }
    }
    Dealloc(win->FrexxScreenName);
    win->FrexxScreenName=Strdup(pubscreenname);
  }

  if(win->window_pointer && win->appwindow &&
     win->window!=FX_SCREEN && win->window!=FX_WINSCREEN) {
    win->appwindow_pointer = AddAppWindow(0, NULL, win->window_pointer, WBMsgPort, NULL);
  }  
  OpenAppIcon();

  win->redrawneeded=NULL;
  if (!cl_iconify && !win->iconify) {
    CopyWindowPos(win);
    if (win->window_pointer->Height<win->window_minheight) {
      Visible=VISIBLE_OFF;
      UpDtNeeded|=UD_REDRAW_SETTING;
      redrawneeded|=SE_REINIT;
      win->redrawneeded|=SE_REINIT;
    } else if ((win->window&FX_WINDOWBIT) && win->autoresize) {
        int dx, dy;
        BarHeight=win->window_pointer->BorderTop-1;
        BorderWidth=win->window_pointer->BorderLeft + win->window_pointer->BorderRight;
        BorderHeight=BarHeight+win->window_pointer->BorderBottom+1;
        
        assert(SystemFont->tf_XSize);
        assert(SystemFont->tf_YSize);
        
        dx=(win->real_window_width - BorderWidth -
            systemfont_leftmarg-systemfont_rightmarg)%SystemFont->tf_XSize;
        dy=(win->real_window_height-BorderHeight)%SystemFont->tf_YSize;
        win->real_window_height=win->window_pointer->Height-dy;
        win->real_window_width=win->window_pointer->Width-dx;
        if (dx || dy) {
            SizeWindow(win->window_pointer, -dx, -dy);
            ignoreresize=2;
        }
    }
  }

  {
    struct DrawInfo *dri;
    dri=GetScreenDrawInfo(win->screen_pointer);
    assert(dri);
    sliderhighlite=dri->dri_Pens[SHINEPEN];
    statuscolor=dri->dri_Pens[FILLPEN];
    statustextcolor=dri->dri_Pens[FILLTEXTPEN];
  }

  /* attach the menus to the window */
  if(menu_attach(win))  // 960605
    return(RetString(STR_CREATE_MENUS));

  if (!cl_iconify && !win->iconify) {

    SetFont(win->window_pointer->RPort, SystemFont);

    if (editprocess)
      editprocess->pr_WindowPtr = (APTR)win->window_pointer;

    win->window_pointer->UserPort=WindowPort;  /* Set up shared port */

    ModifyIDCMP(win->window_pointer, IDCMP_RAWKEY|IDCMP_MOUSEMOVE|IDCMP_MOUSEBUTTONS|IDCMP_INTUITICKS|IDCMP_NEWSIZE|
              IDCMP_CHANGEWINDOW|IDCMP_CLOSEWINDOW|IDCMP_MENUPICK|IDCMP_GADGETDOWN|IDCMP_GADGETUP|
                IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW|(!firstopen?IDCMP_MENUVERIFY:0));

    OwnWindow=TRUE;
  }
  InitWindow(win);
  AdjustBufsInWindow(win);
  firstopen=FALSE;

#ifdef AREXX
  signalbits=ARexxSignal(RexxHandle) |
             (1 << WBMsgPort->mp_SigBit) |
             SIGBREAKF_CTRL_C |
             SIGBREAKF_CTRL_D |	// CTRL-D is sent as a dummy from the file handler.
             SIGBREAKF_CTRL_E |	// CTRL-E is sent as deiconify.
             (1 << TimerMP->mp_SigBit) |
             (1 << WindowPort->mp_SigBit);
#endif

  return(NULL);
}

/**********************************************
*
*  void CloseMyScreen()
*
*  Close down screen.
*
******************/

void CloseMyScreen(WindowStruct *win)
{
  BufStruct *Storage;

  Storage=win->NextShowBuf;

  if (win->window_pointer && Storage) {
    do {
      RemoveGadget(win->window_pointer, &BUF(slide.Slider));
      RemoveGadget(win->window_pointer, &BUF(slide.Border.gadget));
      BUF(slider_on)=FALSE;
      Storage=BUF(NextShowBuf);
    } while(Storage);
  }
  if (win->visualinfo)
    FreeVisualInfo(win->visualinfo);
  if(win->appwindow_pointer) {
    RemoveAppWindow(win->appwindow_pointer);
    win->appwindow_pointer=NULL;
  }

  if (win->window_pointer) {
    if (win->menus) {
      struct Menu *oldmenu=win->menus;
      menu_detach(&menu, win);
      FreeMenus(oldmenu);
    }

#ifdef AMIGA
    {
      /* Remove outstanding IDCMP messages for this window */
      /* Amiga ROM KRML, side 255 */
      struct IntuiMessage *msg;
      struct Node *succ;
      Forbid();
      msg=(struct IntuiMessage *)WindowPort->mp_MsgList.lh_Head;
      while (msg && (succ = msg->ExecMessage.mn_Node.ln_Succ)) {
        if (msg->IDCMPWindow==win->window_pointer) {
          Remove((struct Node *)msg);
          ReplyMsg((struct Message *)msg);
        }
        msg=(struct IntuiMessage *)succ;
      }
      win->window_pointer->UserPort=NULL;
      ModifyIDCMP(win->window_pointer, NULL);
      Permit();
      CloseWindow(win->window_pointer);
      win->window_pointer=NULL;
      Default.windows_opened--;
    }
#endif
    activewindow=NULL;
  }

  if (win->pubscreen && !win->ps_frontmost)
    UnlockPubScreen(NULL, win->pubscreen);
  if (win->screen_pointer) {
    if (win->ownscreen)
      FrexxEdCloseScreen(win->screen_pointer);
    {
      WindowStruct *wincount=FRONTWINDOW;
      while (wincount && 
             (!wincount->window_pointer || wincount->window!=FX_WINSCREEN))
        wincount=wincount->next;
      if (!wincount)
        CloseScreenWindow();
    }
  }
  Dealloc(win->UpdtVars.EndPos);

  CleanUpScreens(NULL);  // Rensa upp alla kvarglömda skärmar

  win->UpdtVars.EndPos=NULL;
  win->visualinfo=NULL;
  win->screen_pointer=NULL;
  win->ownscreen=FALSE;
  win->ps_frontmost=FALSE;
  win->pubscreen=NULL;
}

/**********************************************
*
*  void OpenUpScreen()
*
*  Open screen.
*
******************/
int OpenUpScreen(WindowStruct *win)
{
  BufStruct *Storage;
  char *ret;

  if (ret=OpenMyScreen(win)) {
    CloseMyScreen(win);
    return(FALSE);
  }

  InitColors(win);
  PrintScreenInit();
  Storage=win->NextShowBuf;
  Visible=VISIBLE_OFF;
  do {
    PositionSlider(Storage);
    ShortInitSlider(Storage);
    if (!BUF(slider_on)) {
      AddBufGadget(Storage);
    } else {
      AddGadget(win->window_pointer, &BUF(slide.Slider), 0);
      AddGadget(win->window_pointer, &BUF(slide.Border.gadget), 0);
    }
    BUF(slider_on)=TRUE;
    Storage=BUF(NextShowBuf);
  } while(Storage);
  AdjustBufsInWindow(win);
  UpdateAll();
//  ClearVisible();
  if (win->window_pointer) {
    ModifyIDCMP(win->window_pointer, IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MENUPICK|IDCMP_GADGETDOWN|IDCMP_GADGETUP|
	        IDCMP_MOUSEMOVE|IDCMP_MENUVERIFY|IDCMP_CHANGEWINDOW|IDCMP_CLOSEWINDOW|
	        IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW|IDCMP_INTUITICKS);
  }
  return(TRUE);
}

static void AdjustBufsInWindow(WindowStruct *win)
{
  if (win && win->window_pointer &&
      win->window_pointer->Height>win->window_minheight &&
      win->window_pointer->Height>=win->window_pointer->MinHeight) {
    BufStruct *Storage=win->NextShowBuf;
    while (Storage) {
      ReSizeBuf(Storage, Storage, NULL, BUF(screen_lines)?BUF(screen_lines):win->window_lines);
      FixMarg(Storage);
      if (BUF(slider_on)) {
        InitSlider(Storage);
        RefreshGList(&BUF(slide.Border.gadget), win->window_pointer, NULL, 2);
      }
      Storage=BUF(NextShowBuf);
    }
  }
}

struct Window __regargs *FixWindow(BufStruct *Storage, char *string)
{
  struct Window *retwindow;
  int len=strlen(string);
  int xpos, ypos, xwidth, ywidth;
  WindowStruct *win;

  win=FRONTWINDOW;
  if (BUF(window) && BUF(window)->window_pointer)
    win=BUF(window);

  SetFont(&ButtonRastPort, RequestFont);
  xwidth=TextLength(&ButtonRastPort, string, len)+16;

  ywidth=RequestFont->tf_YSize*2;

  if (win) {
    xpos=win->window_pointer->LeftEdge+win->window_pointer->Width-xwidth-20;
    ypos=win->window_pointer->TopEdge+BarHeight+1;
  }

  if (retwindow=OpenWindowTags(NULL, WA_Left, xpos,
				     WA_Top, ypos,
				     WA_Width, xwidth,
				     WA_Height, ywidth,
				     WA_PubScreen, win?win->screen_pointer:NULL,
				     WA_DetailPen, 1,
				     WA_BlockPen, 0,
				     TAG_END)) {
    SetFont(retwindow->RPort, RequestFont);
    Move(retwindow->RPort, 8, RequestFont->tf_YSize+(RequestFont->tf_YSize>>2));
    SetAPen(retwindow->RPort, 1);
    SetBPen(retwindow->RPort, 0);
    Text(retwindow->RPort, string, len);
  }
  return(retwindow);
}


void SetupMinWindow(WindowStruct *win) 
{
    assert(win);
    assert(SystemFont);

    win->window_minheight=BarHeight;
    win->window_minwidth=SystemFont->tf_XSize*17;
    if (win->window_pointer)
        win->window_minheight+=win->window_pointer->BorderBottom+SystemFont->tf_YSize*2;
    else
        win->window_minheight+=12;
    
    CheckWindowSize(win);
}

void GetDimension(ULONG mode)
{
  struct DimensionInfo *dim=(struct DimensionInfo *)buffer;
  GetDisplayInfoData(0, (void *)dim, sizeof(struct DimensionInfo),
                     DTAG_DIMS, mode);
  visible_height=dim->TxtOScan.MaxY+1;
  visible_width=dim->TxtOScan.MaxX+1;
}

int Iconify(WindowStruct *specific_window)
{
  int ret=OK;
  WindowStruct *win=specific_window;
  if (!win)
    win=FRONTWINDOW;
  CursorXY(NULL, -1, -1);
  while (win) {
    Visible=VISIBLE_ICONIFIED;
    if (win->window_pointer)
      CloseMyScreen(win);
    win->iconify=TRUE;
    win->Visible=VISIBLE_ICONIFIED;
    if (specific_window) {
      char *name=DEFAULTNAME;
      if (specific_window->NextShowBuf &&
          specific_window->NextShowBuf->shared->filnamn[0])
        name=specific_window->NextShowBuf->shared->filnamn;
      specific_window->appicon =
          AddAppIcon(NULL, (long)specific_window,
                     name,
                     WBMsgPort, NULL,
                     externappicon?externappicon:&AppIconDObj, NULL);
    }
    win=win->next;
    if (specific_window)
      win=NULL;
  }
  Visible=VISIBLE_ICONIFIED;
  if (!specific_window) {
    Default.WindowDefault.iconify=TRUE;
    OpenAppIcon();
  }
  return(ret);
}

int __regargs Deiconify(WindowStruct *specific_window)
{
  int ret=OUT_OF_MEM;
  WindowStruct *win=specific_window;
  if (!win)
    win=FRONTWINDOW;
  while (win) {
    if (!win->window_pointer && win->iconify) {
      win->iconify=FALSE;
      if (OpenUpScreen(win)) {
        win->iconify=FALSE;
        ret=OK;
        if (win->appicon) {
          RemoveAppIcon(win->appicon);
          win->appicon=NULL;
        }
      }
    } else
      ret=OK;
    win=win->next;
    if (specific_window)
      win=NULL;
  }
  if (Visible==VISIBLE_ICONIFIED)
    Visible=VISIBLE_OFF;
  if (!specific_window) {
    Default.WindowDefault.iconify=FALSE;
    CloseAppIcon();
  }
  return(ret);
}

void __regargs OpenAppIcon(void)
{
  if(!appicon && (Default.appicon || Default.WindowDefault.iconify)) {
    appicon = AddAppIcon(NULL, NULL, appiconname, WBMsgPort, NULL,
    			 externappicon?externappicon:&AppIconDObj, NULL);
  }  
}

void __regargs CloseAppIcon(void)
{
  if (!Default.appicon && !Default.WindowDefault.iconify) {
    if(appicon) {
      RemoveAppIcon(appicon);
      appicon=NULL;
    }
  }
}


int CloneWB(WindowStruct *win)
{
  struct Screen *temp_pubscreen=win->pubscreen;
  int ret=OUT_OF_MEM;
  if (!temp_pubscreen) {
    temp_pubscreen=LockPubScreen("Workbench");
    if (!temp_pubscreen)
      temp_pubscreen=LockPubScreen(NULL);
  }
  if (temp_pubscreen) {
      ULONG mode=GetVPModeID(&temp_pubscreen->ViewPort);
      GetDimension(mode);
      win->DisplayID=mode;
      SetSystemFont();
      win->screen_height=visible_height;
      win->window_height=visible_height;
      win->screen_width=visible_width;
      win->window_width=visible_width;

    if (temp_pubscreen && temp_pubscreen->RastPort.BitMap) {
        win->screen_depth=temp_pubscreen->RastPort.BitMap->Depth;
    } else {
        // Fallback mainly for non-Amiga version.
        win->screen_depth = 4;
    }
    
    win->window_ypos=0;
    win->window_xpos=0;
    CopyColors(temp_pubscreen, win);
    ret=OK;
    if (!win->pubscreen)
      UnlockPubScreen(NULL, temp_pubscreen);
  }
  return(ret);
}

WindowStruct __regargs *MakeNewWindow(WindowStruct *def)
{
  WindowStruct *win;

  if (!def)
    def=&Default.WindowDefault;

  win=(WindowStruct *)Malloc(sizeof(WindowStruct));
  if (win) {
    *win=*def; /* Kopiera def */
    while (def->next) /* Lägg sist i listan */
      def=def->next;
    win->PubScreen=Strdup(win->PubScreen);
    win->FrexxScreenName=NULL;
    win->Views=0;
    win->window_title=Strdup(win->window_title);
    win->window_pointer=NULL;
    win->screen_pointer=NULL;
    win->pubscreen=NULL;
    win->appwindow_pointer=NULL;
    win->UpdtVars.EndPos=NULL;
    win->NextShowBuf=NULL;
    win->ActiveBuffer=NULL;
    win->visualinfo=NULL;
    win->menus=NULL;
    win->appiconname=NULL;
    win->appicon=NULL;
    win->ps_frontmost=FALSE;
    win->FrexxScreenName=NULL;
    win->ownscreen=FALSE;
    win->window_resized=0;
    win->flags=0;
    win->next=def->next;
    win->prev=NULL;
    win->iconify=FALSE;
    if (def!=&Default.WindowDefault)
      win->prev=def;
    def->next=win;
    Default.windows_allocated++;
  }
  return win;
}

void __regargs FreeWindow(WindowStruct *win)
{
  BufStruct *Storage;

  Storage=win->NextShowBuf;
  while (Storage) {
    register BufStruct *temp=BUF(NextShowBuf);
    BUF(NextShowBuf)=NULL;
    BUF(PrevShowBuf)=NULL;
    BUF(window)=NULL;
    Storage=temp;
  }
  Dealloc(win->UpdtVars.EndPos);
  Dealloc(win->PubScreen);
  Dealloc(win->window_title);
  Dealloc(win->FrexxScreenName);
  if (win->prev)
    win->prev->next=win->next;
  else
    Default.WindowDefault.next=win->next;
  if (win->next)
    win->next->prev=win->prev;

  Dealloc(win);
  Default.windows_allocated--;
}

WindowStruct *CreateNewWindow(WindowStruct *defwin, BufStruct *newstorage, BufStruct *Storage)
{
    WindowStruct *win=NULL;
    if (!newstorage || newstorage->window) {
        newstorage=GetNewBuf(Storage, Storage, SC_NEXT_HIDDEN_BUF, type_FILE);
        if (!newstorage || newstorage->window)
            newstorage=MakeNewBuf(Storage);
    }

    if (newstorage) {
        win=MakeNewWindow(defwin);
        if (win) {
            AttachBufToWindow(win, newstorage);
        }
    }
    assert(win->ActiveBuffer > 1024);
    return win;
}


/* Checkar om fönstret är skrivbart i */
void __regargs CheckWindowSize(WindowStruct *win)
{
  if (win->window_pointer &&
      ((win->real_window_height>win->window_minheight &&
        win->real_window_height>win->window_pointer->MinHeight) ||
       (win->window_height>win->window_minheight &&
        win->window_height>win->window_pointer->MinHeight))) {
    win->Visible=VISIBLE_ON;
  } else {
    win->Visible=VISIBLE_OFF;
  }
}


int __regargs LoadKeyMap()
{
  if (Default.KeyMapseg)
    UnLoadSeg(Default.KeyMapseg);
  Default.KeyMapseg=NULL;
  internalkeymap=NULL;

  if (Default.KeyMap[0]) {
    Default.KeyMapseg=LoadSeg(Default.KeyMap);
    if (!Default.KeyMapseg) {
      Sprintf(buffer, "Devs:keymaps/%s", Default.KeyMap);
      Default.KeyMapseg=LoadSeg(buffer);
    }
    if (Default.KeyMapseg) {
      internalkeymap = &((struct KeyMapNode *)(((ULONG *)BADDR(Default.KeyMapseg))+1))->kn_KeyMap;
    }
  }
  return(Default.KeyMapseg);
}
