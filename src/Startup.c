/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Startup.c
 *
 * Submain-routines for the entire shit!
 *
 *********/

#include "compat.h"

#ifdef AMIGA
#include <dos/rdargs.h>
#include <exec/execbase.h>
#include <exec/ports.h>
#include <exec/tasks.h>
#include <graphics/rastport.h>
#include <graphics/gfxmacros.h>
#undef GetOutlinePen
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <workbench/startup.h>
#include <libraries/dos.h>
#include <libraries/FPL.h>
#include <libraries/reqtools.h>
#include <proto/dos.h>
#include <proto/FPL.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/reqtools.h>
#include <proto/icon.h>

#include <dos/dostags.h> /* for the CreateNewProc() tags! */
#endif

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "BufControl.h"
#include "BuildMenu.h"
#include "Change.h"
#include "Command.h"
#include "Startup.h"
#include "Execute.h"
#include "GetFile.h"
#include "IDCMP.h"
#include "Init.h"
#include "Limit.h"
#include "OpenClose.h"
#include "Request.h"
#include "Setting.h"
#include "UpdtScreen.h"

#include "FileHandler.h"
#include "Process.h"

extern void *Anchor;
extern DefaultStruct Default; /* the default values of a BufStruct */
extern struct NewScreen newscreen;
extern char buffer[];
extern long FrexxPri;
extern char *cl_frexxecute, *cl_run, *cl_portname, *cl_init, *cl_pubscreen, *cl_startupfile, *cl_execute;
extern char *cl_diskname;
extern int cl_column, cl_line, cl_copywb, cl_double;
extern int cl_omit, cl_screen, cl_window, cl_backdrop, cl_iconify;
extern struct RDArgs *argsptr;
extern struct ExecBase *SysBase;
//extern LocalStruct Local;
extern char FrexxEdStarted;	/* Är FrexxEd uppstartad */
extern char DebugOpt; /* Debug option on/off */
extern struct Window *InfoWindow;
extern char *ReturnBuffer;
extern struct Library *IconBase;
extern struct MenuInfo menu;
extern char *mothername;
extern BufStruct *NewStorageWanted;
extern int UpDtNeeded;
extern BOOL cache_allocs;
extern struct AllocCache alloc_cache[];
extern BOOL open_copywb;
extern char **FromWorkbench;
extern jmp_buf return_stackpoint;

static BufStruct *real_main(IPTR *opts);
static void __regargs ReadInitFPL(BufStruct *Storage);

/* filehandlerstuff: */
extern struct ProcMsg *filehandlerproc;
extern struct Task *FrexxEdTask;

int secondmain(IPTR *opts, char **fromwb)
{
  BufStruct *Storage;
  if (!setjmp(return_stackpoint)) {
    FromWorkbench=fromwb;
    Storage=real_main(opts);
    if (Storage)
      IDCMP(Storage);
#ifdef DEBUGTEST
    if(DebugOpt)
      FPrintf(Output(), "CloseAll\n");
#endif
//    CloseAll(NULL);
    CloseFrexxEd(NULL);
  }
  return 0;
}


char *InitFrexxEd()
{
  Default.olddirectory=(void *)-1;	// Pre-init.  Annars kan det krascha.
#ifdef AMIGA
  {
    FrexxEdTask=FindTask(NULL);
    mothername=FrexxEdTask->tc_Node.ln_Name;
    FrexxEdTask->tc_Node.ln_Name="FrexxEd";
  }
#endif

  if(!InitAlloc()) /* init memory pools! */
    return(RetString(STR_GET_MEMORY));

  cache_allocs=TRUE;

  {
    register char *ret;  
    ret=OpenLibraries();

    InitDefaultBuf();     /* init defaultBUF */
    if (ret)
      return(ret);
  }

  if (InitDefaultSetting()<OK)
    return(RetString(STR_GET_MEMORY));
  Default.task=FindTask(NULL);
//  Default.task->tc_UserData=&Local;
  return(NULL);
}


static BufStruct *real_main(IPTR *opts)
{
  BufStruct *Storage;
  BufStruct *activeStorage;
  int buffers;
  char **files;
  int ret;
  char nofilehandler=FALSE;

  {

    CacheClearU();

    FirstOpen();
    if (!cl_init || cl_init[0])
      LoadSetting(NULL, cl_init?cl_init:(char *)-1, TRUE);
    if (cl_copywb)
      CloneWB(&Default.WindowDefault);  // Klona WB till default

    if(cl_diskname && !strcmp(cl_diskname, "off")) {
      nofilehandler=TRUE;
      Dealloc(cl_diskname); /* free memory */
      cl_diskname=NULL; /* clear */
    }
    SecondInit();

    LastOpen();
    Storage=MakeNewBuf(NULL);
    if (!Storage)
      CloseAll(RetString(STR_GET_MEMORY));
    if (FRONTWINDOW)
      AttachBufToWindow(FRONTWINDOW, Storage);

    FrexxEdStarted=1;	// FrexxEd is now considered started.

    if (!cl_omit) {
#ifdef DEBUGTEST
      if(DebugOpt)
        FPrintf(Output(), "Execute FPL\n");
#endif
      ReadInitFPL(Storage);
    }
  
    FrexxEdStarted=2;	// FrexxEd have read the init files

#ifdef DEBUGTEST
    if(DebugOpt)
      FPrintf(Output(), "Load files\n");
#endif
      /* are there files as arguments?? */
    files=(char **)opts[opt_FILE];
    {
      char **sptr=files;
      activeStorage=Storage;
      buffers = 0;
      while (sptr && *sptr) {
        if (buffers)
          Storage=MakeNewBuf(NULL);
        ret=Load(Storage, NULL, loadNEWBUFFER, *sptr);	//loadNOQUESTION borttaget
        if (ret<OK && buffers) {
          Free(Storage, FALSE, TRUE);
        } else {
          if (ret>=OK)
            buffers++;
          activeStorage=Activate(activeStorage, Storage, Default.full_size_buf);
          if (ret<OK)
            Showerror(activeStorage, ret);
          else
            Showstat(activeStorage);
        }
        sptr++;
      } /* while(sptr... */
    }

#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "Loaded\n");
#endif
#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "Update screen\n");
#endif
    UpdateAll();

    cl_iconify=FALSE;
  
    ExecuteFPL(Storage, cl_execute, TRUE, NULL, NULL);
    Dealloc(cl_execute);

    if (BUF(window)->window_pointer)
      RefreshWindowFrame(BUF(window)->window_pointer);
    FrexxEdStarted=3;	// FrexxEd has executed the startup file.
    ClearVisible();

#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "Start IDCMP\n");
#endif
    cache_allocs=FALSE;
  }

  if(!nofilehandler && Default.filehandler ) {
    filehandlerproc = start_process(FileHandler, Default.taskpri,
                                    4000, FILE_HANDLER_PROCESS);
    /* if we fail, simply no filehandler! */
  }
  else
    Default.filehandler=FALSE; /* switch off for real! */
  return(Storage);
}

void ParseArg(char *string, IPTR *opts)
{
  struct RDArgs rdargs;
  struct RDArgs *result;

  if(string) {
    memset(opts, 0, opt_COUNT*sizeof(long));
    /* only do this if we have a string to parse! */
    memset(&rdargs, 0, sizeof(struct RDArgs));

    rdargs.RDA_Source.CS_Buffer = string;
    rdargs.RDA_Source.CS_Length = strlen(string);
    result = ReadArgs((UBYTE *)TEMPLATE, opts, &rdargs);
  }
  if(!string || result) {
    if(opts[opt_BACKDROP]) {
      cl_backdrop=TRUE;
      cl_screen=FALSE;
      cl_window=FALSE;
    } else if(opts[opt_SCREEN]) {
      cl_backdrop=FALSE;
      cl_screen=TRUE;
      cl_window=FALSE;
    } else if(opts[opt_WINDOW]) {
      cl_backdrop=FALSE;
      cl_screen=FALSE;
      cl_window=TRUE;
    }

    if(opts[opt_COPYWB])
      cl_copywb=TRUE;

    if(opts[opt_INIT]) {
      Dealloc(cl_init);
      cl_init=Strdup((char *)opts[opt_INIT]);
    }
    if(opts[opt_EXECUTE]) {
      Dealloc(cl_execute);
      cl_execute=Strdup((char *)opts[opt_EXECUTE]);
    }
    if(opts[opt_OMIT])
      cl_omit=TRUE;

#ifdef _DEBUGTEST
    if(opts[opt_DEBUG])
      DebugOpt=TRUE;
#endif

    if(opts[opt_ICONIFY])
      cl_iconify=TRUE;

    if(opts[opt_PORTNAME]) {
      Dealloc(cl_portname);
      cl_portname=Strdup((char *)opts[opt_PORTNAME]);
    }
    if(opts[opt_DISKNAME]) {
      Dealloc(cl_diskname);
      cl_diskname=Strdup((char *)opts[opt_DISKNAME]);
    }
    if(opts[opt_DIRECTORY]) {
      Dealloc(Default.directory);
      Default.directory=Strdup((char *)opts[opt_DIRECTORY]);
      ChangeDirectory(Default.directory);
    }
    if(opts[opt_STARTUP]) {
      Dealloc(cl_startupfile);
      cl_startupfile=Strdup((char *)opts[opt_STARTUP]);
    }
    if (((LONG *)opts[opt_PRIO]))
      FrexxPri=*((LONG *)opts[opt_PRIO]);

    if(opts[opt_PUBSCREEN]) {
      Dealloc(cl_pubscreen);
      cl_pubscreen=Strdup((char *)opts[opt_PUBSCREEN]);
    }
    if(string)
      FreeArgs(result);
  }
}
void __regargs ReadInitFPLInfoWindow(BufStruct *Storage, char *text, ReturnCode result)
{
  if (result!=FPL_OK && result!=FPL_EXIT_OK) {
    {
      register char *temppek=text;
      while (*temppek) {
        switch (*temppek) {
        case '#':
        case '?':
        case '|':
        case '(':
        case '[':
          return;
        }
        temppek++;
      }
    }
    Sprintf(buffer, RetString(STR_ERROR_WITH_FPL_FILE), text);
    Ok_Cancel(NULL, buffer, RetString(STR_OPEN_ERROR), RetString(STR_OK_GADGET));

/*
    char tempbuffer[200];
    if (InfoWindow) {
      CloseWindow(InfoWindow);
      InfoWindow=NULL;
    }
    Sprintf(tempbuffer, RetString(STR_ERROR_WITH_FPL_FILE), text);
    InfoWindow=FixWindow(Storage, tempbuffer);
*/
  }
}
/***********************************************************
 *
 * Will execute the startup FPL program.
 *
 *****/
static void __regargs ReadInitFPL(BufStruct *Storage)
{
  int antalfiles;
  struct Files *list=NULL;
  char *remember=NULL;
//  BPTR lock;
  int result;

  if (Default.StartupFile[0]) {
    antalfiles=GetFileList(Storage, Default.StartupFile, &list, &remember, FALSE);
  
    if (antalfiles>0) {
      register struct Files *templist;
      templist=list;
      while (templist) {
        ExecuteFPL(Storage, templist->name, TRUE, &result, NULL);
        ReadInitFPLInfoWindow(Storage, templist->name, result);
        templist=templist->next;
      }
      OwnFreeRemember(&remember);
    } else {
      char tempfile[FILESIZE];
      char *startname=Default.StartupFile;
      int namelen;
  
      while (namelen=GetWildWord(startname, tempfile)) {
        startname+=namelen;
        result=-1;
  
        ExecuteFPL(Storage, tempfile, TRUE, &result, NULL);
/* bortkommenterat 950222, ingen nytta...
        if (lock=Lock((UBYTE *)tempfile, ACCESS_READ)) {
          UnLock(lock);
          ExecuteFPL(Storage, tempfile, TRUE, &result, NULL);
        } else {
          ExecuteFPL(Storage, tempfile, TRUE, &result, NULL);
        }
*/
        ReadInitFPLInfoWindow(Storage, tempfile, result);
      }
    }
  }
}

