/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
 *
 * Execute.c
 *
 * This routine handles ALL FPL.library calls, internal FPL handling
 * and environment variable subroutines.
 *
 *******/
#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <fpl/reference.h>
#include <libraries/FPL.h>

#include "IncludeAll.h"

extern BlockStruct *BlockBuffer;
extern FACT *UsingFact;
extern FACT *DefaultFact;
extern struct FrexxEdFunction fred[];
extern int nofuncs;
extern BufStruct *NewStorageWanted;
extern int Visible;
extern int UpDtNeeded;
extern DefaultStruct Default;
extern char buffer[];
extern BOOL fplabort;	/* Allow to abort FPL scripts.  Startup script shouldn't be breakable */
extern struct Setting **sets;
extern antalsets;
extern int ReturnValue;		// Global return value storage.
extern int ErrNo;		// Global Error value storage.
extern char CursorOnOff;
extern srch Search;          /* search structure */

extern struct TextFont *SystemFont;
extern int BarHeight;
extern HistoryStruct SearchHistory;

extern void *Anchor;
extern long ARexxFPL;
extern char FrexxEdStarted;	/* Är FrexxEd uppstartad */
extern struct InputEvent ievent;        /* used for RawKeyConvert() */
extern struct KeyMap *internalkeymap;

extern int recursive;
extern BOOL hook_enabled;
extern struct Setting **sets;
extern char **setting_string_pointer;
extern int *setting_int_pointer;
extern char cursorhidden;

static long __asm fpl_functions(register __a0 struct fplArgument *);
extern struct Library *FPLBase;

extern AREXXCONTEXT RexxHandle;

extern char *fpl_executer;	/* Namn på den funktion som exekverar FPL */
extern ReturnCode fpl_error;
extern BPTR wb_path;

extern char **filelog_list;
extern struct MenuInfo menu;
extern BOOL clear_all_currents;
extern BOOL device_want_control;
extern int undoantal;

extern struct UserData userdata;

extern short important_message_available;
extern int retval_antal;
extern int retval_params[];
extern struct screen_buffer ScreenBuffer;

/*** PRIVATE ***/
static jmp_buf oldfplstackpoint;
static int filelog_maxalloc = 0;
static int filelog_antal=0;

static void ScInverseLine(BufStruct *Storage, struct fplArgument *arg);
static void ScPrintLine(BufStruct *Storage, struct fplArgument *arg);
static int ScPromptInfo(BufStruct *Storage, int argID, struct fplArgument *arg);
static int ScGetFileList(BufStruct *Storage, struct fplArgument *arg);
static int ScMenuRead(struct fplArgument *arg);
static int ScMenuDelete(struct fplArgument *arg);
static int ScGetList(BufStruct *Storage, struct fplArgument *arg);
static int ScFaceRead(struct fplArgument *arg);
static int BSearch(struct fplArgument *arg);
static int ScSort(struct fplArgument *arg);
static long __asm __stackext run_functions(register __a0 struct fplArgument *arg);

#ifndef FREQF_NOFILES
#define FREQF_NOFILES 1
#define FREQF_SAVE 2
#define FREQF_MULTISELECT 4

struct rtFileList {
};
#endif


/**********************************************************************
 *
 * InitFPL()
 *
 * Initializes fpl.library handling. Returns non-zero if anything failed!
 *
 *****/
int InitFPL(char type)
{
  int i=0;
  int ret=0;

  if (!Anchor) {
#ifdef REG_A4
    userdata.a4 = getreg(REG_A4);
#endif

#ifdef FIXME
    Anchor=fplInitTags(fpl_functions,
                       FPLTAG_INTERVAL, (long)StopCheck,
                       FPLTAG_CACHEALLFILES, FPLCACHE_EXPORTS,
                       FPLTAG_USERDATA, &userdata,
                       FPLTAG_INTERNAL_ALLOC, (long)FPLmalloc,
                       FPLTAG_INTERNAL_DEALLOC, (long)FPLfree,
                       FPLTAG_REREAD_CHANGES, TRUE,
                       FPLTAG_HASH_TABLE_SIZE, 311,
                       FPLTAG_IDENTITY, Default.ARexxPort, /* identify us! */
                       FPLTAG_AUTORUN, TRUE,
                       FPLTAG_END);
#endif
  }
  if (!Anchor)
    ret=OUT_OF_MEM;
  
  while (!ret && i<nofuncs) {
    if (fred[i].type==type) {
      ret=fplAddFunction(Anchor,
  		         fred[i].name,
  		         fred[i].ID,
  		         fred[i].ret,
  		         fred[i].format,
  		         NULL);
    }
    i++;
  }
  return(ret);
}

/*****************************
 *
 * ExecuteFPL()
 *
 * NEW STYLE:
 *
 * Mode:
 * EXECUTE_SCRIPT - executes the script the second paramter points to
 * EXECUTE_FILE   - executes the file
 * EXECUTE_CACHED_SCRIPT - executes a script, but enables caching!
 *
 * Returns the error code of the FPL.library calls in the integer
 * the fourth argument points to (if given).
 */

int __regargs
ExecuteFPL(BufStruct *Storage,
           char *program,
           char mode,
           int *end,
           char *interpret)
{
  BPTR lock=NULL;
  BOOL pathextract=FALSE;
  int pathlen;
  char *pathpoint;
  int result=FPL_OK;
  char *caller=fpl_executer;

  fpl_executer=NULL;
  if (program) {
    fpl_error=FPL_OK;
    recursive++;	/* Do not MacroRecord any happenings in command.c */

    userdata.buf = Storage;
    if(mode != EXECUTE_FILE) {
      /* Execute the FPL program!
         This really should have a name sent in a tag */
#ifdef LOG_FILE_EXECUTION
      if (*program=='<') {
        register char *stop=strrchr(program, '>');
        if (stop) {
          if (FindFileExecution(program)>=0) {
            memcpy(&buffer[1000], program+1, stop-program-1);
            buffer[1000+stop-program-1]=0;
            ExecuteFPL(Storage, &buffer[1000], TRUE, &result, interpret);
            if (NewStorageWanted) {
              Storage=NewStorageWanted;
              clear_all_currents=TRUE;
            }
            if (result<0)
              mode=EXECUTE_FILE;
          }
          program=stop+1;
        } else
          mode=EXECUTE_FILE;
      }
#endif
      userdata.buf = Storage;
      if(mode != EXECUTE_FILE) {
        if(mode == EXECUTE_CACHED_SCRIPT) {
          /*
           * Try flushing it first, to remove a previous version!
           */
          fplSendTags(Anchor, FPLSEND_FREEFILE, interpret?interpret:caller,
                              FPLTAG_END);
        }
        result=fplExecuteScriptTags(Anchor, &program, 1,
                                    FPLTAG_CACHEFILE,
                                    (mode==EXECUTE_CACHED_SCRIPT?
                                     FPLCACHE_EXPORTS:FPLCACHE_NONE),
                                    FPLTAG_PROGNAME, interpret?interpret:caller,
                                    FPLTAG_KIDNAP_CACHED, TRUE, /* kidnap caches! */
                                    FPLTAG_USERDATA, &userdata,
                                    FPLTAG_END);
      }
    } else {
      struct Files *list=NULL;
      char *remember=NULL;

      userdata.buf = Storage;
      if (program[0]) {
        LockBuf_release_semaphore(BUF(shared));
        /* execute the FPL program file */
        if(!strchr(program, ':')) {
          pathextract=TRUE;
          pathpoint=Default.FPLdirectory;
        } else {
          lock = Lock(program, ACCESS_READ);
          if (!lock)
            GetFileList(Storage, program, &list, &remember, FALSE);
        }
        result = FPL_OPEN_ERROR;
        do {
          if (lock) {
            char *fullname=Malloc(FILESIZE);
            if (fullname) {
              NameFromLock(lock, fullname, FILESIZE);
              UnLock(lock);
              lock=NULL;
#ifdef LOG_FILE_EXECUTION
              LogFileExecution(program);
#endif
              UnLockBuf_obtain_semaphore(BUF(shared));
              result=fplExecuteFileTags(Anchor, fullname,
                                        FPLTAG_USERDATA, &userdata,
                                        FPLTAG_INTERPRET, interpret,
                                        FPLTAG_END);
              LockBuf_release_semaphore(BUF(shared));
              Dealloc(fullname);
              pathextract=FALSE;
              if (result!=OK)
                break;	// exit while
            }
          } else {
            if (list) {
              lock = Lock(list->name, ACCESS_READ);
              list=list->next;
            } else {
              if (pathextract) {
                pathlen=GetWildWord(pathpoint, buffer);
                if (pathlen) {
                  strcat(buffer, program);
                  lock = Lock(buffer, ACCESS_READ);
                  if (!lock) {
                    if (remember)
                      OwnFreeRemember(&remember);
                    GetFileList(Storage, buffer, &list, &remember, FALSE);
                  }
                  pathpoint+=pathlen;
                } else {
                  break;	// exit while
                }
              } else {
                break; // exit while
              }
            }
          }  
        } while (1);
        UnLockBuf_obtain_semaphore(BUF(shared));
        if (remember)
          OwnFreeRemember(&remember);
      }
    }
    NewStorageWanted = userdata.buf;

    if(Storage==NewStorageWanted)
      NewStorageWanted=NULL;

    recursive--;
    if (result==FPLERR_PROGRAM_STOPPED)
      fpl_error=FPL_EXIT_OK;
  }
  if (end)
    *end=result;

 /* if a new BufStruct if wanted, return NEW_STORAGE ! */
  return(NewStorageWanted?NEW_STORAGE:OK);
}


static longfpl_functions(struct fplArgument *arg)
{
  int ret;
//printf("Stack: %x %x %x =%x\n", Default.task->tc_SPLower, Default.task->tc_SPUpper, getreg(REG_A7), ((int)getreg(REG_A7))-(int)Default.task->tc_SPLower);
  ret=setjmp(oldfplstackpoint);
  if (!ret)
    ret=run_functions(arg);
  return(ret);
}

static long run_functions(struct fplArgument *arg)
{
  int line;
  int col;
  char *tempbuffer;
  int ret;
  int *ReturnInt;
  int tempint;	// Om ReturnInt måste ha nått att peka på.
  BufStruct *Storage;
  BufStruct *Storage2;
  String result;
  struct UserData *userdata;
  int argID;

  ret=OK;
  ReturnInt=NULL;
  NewStorageWanted=NULL;
  fplSendTags(arg->key, FPLSEND_GETUSERDATA, (long)&userdata, FPLSEND_DONE);
  argID=arg->ID;
  Storage = userdata->buf;
//printf("Command: %d\n", argID);

  if(argID>=0 && !(argID & FPL_CMD)) {
    ret=Command(Storage, argID, arg->argc, (char **)arg->argv, NULL);
    fplSendTags(arg->key, FPLSEND_INT, ReturnValue, FPLSEND_DONE);
  } else {
    if (undoantal)
      Command(Storage, DO_NOTHING|NO_HOOK, 0, NULL, NULL);
    if (argID>=0 && Default.hook[argID&~LOCKINFO_BITS]) {
      if(RunHook(Storage, argID, arg->argc, (char **)arg->argv, 0)) {
        argID=-1;	// Gör ingen posthook.
        ret=FUNCTION_CANCEL;
      }
      if (NewStorageWanted) {
        userdata->buf = NewStorageWanted;
        Storage=NewStorageWanted;
        clear_all_currents=TRUE;
      }
    }
    switch(argID) {
    /***********************************************************************
     *
     * All FrexxEd-FPL functions are coded here below:
     *
     *****/

    case SC_INVERSE_LINE:
      ScInverseLine(Storage, arg);
      break;
    case SC_CLEAN:
      hook_enabled=FALSE;
      ExecuteFPL(Storage, (char *)arg->argv[0], FALSE, &tempint, NULL);
      ReturnInt=&tempint;
      hook_enabled=TRUE;
      break;
 
    case SC_EXISTS:
      LockBuf_release_semaphore(BUF(shared));
      ret = (int)Lock((char *)arg->argv[0], ACCESS_READ);
      if(ret)
        UnLock((BPTR)ret);
      UnLockBuf_obtain_semaphore(BUF(shared));
      ReturnInt = &ret;
      break;

    case SC_FACT_DELETE:
      ret=FACTDelete((char *)arg->argv[0]);
      ReturnInt=&ret;
      break;

    case SC_FACT_CREATE:
      ret=CreateFact((char *)arg->argv[0]);
      ReturnInt=&ret;
      break;
    case SC_RESET_FACT:
      {
        FACT *fact=NULL;
        if (arg->argc>1) {
          fact=FindFACT((char *)arg->argv[1]);
          if (!fact) {
            ret=SYNTAX_ERROR;
            break;
          }
        }
        ret=ResetFACT(fact?fact:DefaultFact, arg->argc?(int)arg->argv[0]:256);
      }
      ReturnInt=&ret;
      break;
      
    case SC_ISUPPER:
    case SC_ISLOWER:
    case SC_ISSPACE:
    case SC_ISWORD:
    case SC_ISSYMBOL:
    case SC_ISOPEN:
    case SC_ISCLOSE:
    case SC_ISTAB:
    case SC_ISNEWLINE:
    case SC_ISSTRING:
      {
        register FACT *fact=BUF(using_fact);
        if (arg->argc>1) {
          fact=FindFACT(arg->argv[1]);
        }
        if (fact) {
          tempint=FactInfo(fact, (int)argID, (int)arg->argv[0]);
          if (tempint>=256) {
            fplSendTags(arg->key, FPLSEND_STRING, buffer,
                        FPLSEND_STRLEN, tempint-256,
                        FPLSEND_DONE);
          } else
            ReturnInt=&tempint;
        }
      }
      break;

    case SC_STRICMP:
      if(arg->argc>2)
        tempint = Strnicmp(arg->argv[0], arg->argv[1], (int)arg->argv[2]);
      else
        tempint = Stricmp(arg->argv[0], arg->argv[1]);
      ReturnInt=&tempint;
      break;

    case SC_CONSTRUCT_INFO:
      tempint=0;
      ret=0;
      if (arg->argc>7) {
        tempint=(int)arg->argv[7];
        ret=arg->format[7];
      }
      ret=ConstructSetting((char *)arg->argv[0],
                           (char *)arg->argv[1],
                           (char *)arg->argv[2],
                           (char *)arg->argv[3],
                           (char *)arg->argv[4],
                           (int)arg->argv[5],
                           (int)arg->argv[6],
                           tempint, ret);
      ReturnInt=&ret;
      break;
    case SC_DELETE_INFO:
      ret=DeleteSetting((char *)arg->argv[0]);
      ReturnInt=&ret;
      break;
    case SC_PROMPT_BUFFER:
    case SC_PROMPT_ENTRY:
      Storage2 = ChooseBuf(Storage, arg->argc>0?(char *)arg->argv[0]:"", arg->argc>1?(int)arg->argv[1]:-1, argID==SC_PROMPT_ENTRY);
      ReturnInt=(int *)&Storage2;
      break;
    case SC_OUTPUT:
      {
        char *argv[2];
        argv[0]=arg->argv[0];
        argv[1]=(char *)FPL_STRING_LENGTH(arg, 0);
        ret=Command(Storage, SC_OUTPUT|NO_HOOK, 2, (char **)&argv, NULL);
      }
      break;
    case SC_DUPLICATE_ENTRY:
      {
        Storage2=Storage;
        if (arg->argc)
          Storage2=CheckBufID((BufStruct *)arg->argv[0]);
        if (Storage2) {
          tempint=(int)DuplicateBuf(Storage2);
          if (!tempint)
            ret=OUT_OF_MEM;
          ReturnInt=&tempint;
        } else
          ret=CANT_FIND_BUFFER;
      }
      break;
    case SC_NEXT_BUF:
    case SC_PREV_BUF:
    case SC_NEXT_ENTRY:
    case SC_PREV_ENTRY:
    case SC_NEXT_SHOW_BUF:
    case SC_PREV_SHOW_BUF:
    case SC_NEXT_HIDDEN_BUF:
    case SC_PREV_HIDDEN_BUF:
      Storage2=Storage;
      if (arg->argc>0 && (int)arg->argv[0]) {
        Storage2=CheckBufID((BufStruct *)arg->argv[0]);
        if (!Storage2)
          Storage2=&Default.BufStructDefault;
      }
      Storage2=GetNewBuf(Storage, Storage2, argID, arg->argc>1?(int)arg->argv[1]:Storage2->shared->type);
      ReturnInt=(int *)&Storage2;
      break;
    case SC_ISFOLD:
      Storage2=Storage;
      line=BUF(curr_line);
      if (arg->argc>0) {
        if (arg->argc>1 && (int)arg->argv[1])
          Storage2=CheckBufID((BufStruct *)arg->argv[1]);
        if (Storage2 && (int)arg->argv[0]>0 && (int)arg->argv[0]<=Storage2->shared->line)
          line=(int)arg->argv[0];
      }
      if (Storage2) {
        tempint=Storage2->shared->text[line].fold_no;
        if (!(Storage2->shared->text[line].flags&FOLD_HIDDEN))
          tempint=-tempint;
        ReturnInt=&tempint;
      }
      break;
    case SC_ACTIVATE:		// 'Activate(int BufferID, int mode, int oldbufferid)'
      if (FRONTWINDOW) {
        BufStruct *Storage3=Storage;
        Storage2=Storage;
        if (arg->argc) {
          Storage2=CheckBufID((BufStruct *)arg->argv[0]);
          if (!Storage2)
            Storage2=Storage;
        }
        if (arg->argc>2)
          Storage3=CheckBufID((BufStruct *)arg->argv[2]);
        if (!Storage3 || !Storage3->window)
          Storage3=FRONTWINDOW->NextShowBuf;
        {
          register int flag=Default.full_size_buf;
          if (arg->argc>1 && (int)arg->argv[1]>=0 && (int)arg->argv[1]<=3)
            flag=(int)arg->argv[1];
          Storage2=Activate(Storage3, Storage2, flag);
        }
        ReturnInt=(int *)&Storage2;
      }
      break;
    case SC_REDRAWSCREEN:
      UpdateAll();
      break;
    case SC_CENTERSCREEN:	// 'CenterScreen(int ViewID)'
      {
        Storage2=Storage;
        if (arg->argc)
          Storage2=CheckBufID((BufStruct *)arg->argv[0]);
        if (Storage2) {
          CenterScreen(Storage2);
          UpdateScreen(Storage2);
          MoveSlider(Storage2);
        } else
          ret=CANT_FIND_BUFFER;
        ReturnInt=&ret;
      }
      break;
    case SC_PRINTLINE:		// 'PrintLine(string, viewrad, BufferID)'
      ScPrintLine(Storage, arg);
      break;
    case SC_NEW_BUF:
      CursorXY(Storage, -1, 0);
      Storage2=MakeNewBuf(Storage);
      if (!Storage2)
        ret=OUT_OF_MEM;
      ReturnInt=(int *)&Storage2;
      break;

#ifdef MULTIPROCESS
    case SC_NEW_PROCESS:
      fplSendTags(arg->key, FPLSEND_INT, StartNewProcess((char *)arg->argv[0]), FPLSEND_DONE);
      break;
#endif

    case SC_RENAME:
      ret=RenameBuf(Storage, arg->argv[0]);
      break;
    case SC_CURRENT_BUF:	// 'CurrentBuffer(int BufferID)'
      if (NewStorageWanted=CheckBufID((BufStruct *)arg->argv[0])) {
        ret=NEW_STORAGE;
        CursorXY(NULL, -1, -1);
        if (NewStorageWanted->window) {
          TestCursorPos(NewStorageWanted);
          if (NewStorageWanted->block_exists) {
            SetBlockMark(NewStorageWanted, FALSE);
          }
        }
        tempint=(int)Storage;
        UpDtNeeded|=UD_SHOW_STATUS;
      } else {
        ret=CANT_FIND_BUFFER;
        tempint=0;
      }
      ReturnInt=&tempint;
      break;
    case SC_STRING2BLOCKAPPEND:            // 'StringToBlockAppend(char *blockname, char *string)'
    case SC_STRING2BLOCK:                  // 'StringToBlock(char *blockname, char *string)'
      {
        register BlockStruct *block=NULL;
        if (arg->argc>1 && arg->argv[1]) {
          block=FindBlock((BufStruct *)arg->argv[1]);
          if (!block) {
            ret=NO_BLOCK_FOUND;
            break;
          }
        }
        ret=String2Block(block, arg->argv[0], FPL_STRING_LENGTH(arg, 0), argID==SC_STRING2BLOCKAPPEND);
        ReturnInt=&ret;
      }
      break;
    case SC_STRING2CLIP:             // 'StringToClip(int unit, char *string)'
	  ret=String2Clip(arg->argv[1], FPL_STRING_LENGTH(arg, 1), (int)arg->argv[0]);
      ReturnInt=&ret;
      break;
    case SC_BLOCK_ALLOC:         // 'BlockAlloc(char *blockname)'
      tempint=(int)AllocBlock(arg->argv[0]);
      ReturnInt=&tempint;
      break;
    case SC_BLOCK_CHANGE:         // 'BlockChange(char *blockname)'
      {
        register BlockStruct *block=Default.FirstBlock;
        if (arg->argc>0 && arg->argv[0])
          block=FindBlock(arg->argv[0]);
        if (!block) {
          ret=NO_BLOCK_FOUND;
          tempint=0;
        } else {
          tempint=(int)BlockBuffer;
          BlockBuffer=block;
        }
        ReturnInt=&tempint;
      }
      break;
/*
    case SC_BLOCK_FREE:          // 'BlockFree(char *blockname)'
      ret=NO_BLOCK_FOUND;
      if (0==strcmp(arg->argv[0], "DefaultBlock"))
        ret=FreeBlock(NULL, arg->argv[0]);
      ReturnInt=&ret;
      break;
*/
    case SC_SETFACT:
      ReturnInt=&ret;
      {
        FACT *fact=NULL;
        tempint=0;
        if (arg->format[0]==FPL_STRARG) {
          fact=FindFACT((char *)arg->argv[0]);
          if (!fact || arg->format[1]!=FPL_INTARG) {
            ret=SYNTAX_ERROR;
            break;
          }
          tempint++;
        }
        ret=SetFACT(fact?fact:DefaultFact, arg->argc-tempint, (char **)&arg->argv[tempint], &arg->format[tempint]);
      }
      break;
    case SC_LOADSTRING:		// 'LoadString(filename)'
      {
        ReadFileStruct RFS;
        memset((char *)&RFS, 0, sizeof(RFS));
        RFS.filename=(char *)arg->argv[0];
        ret=ReadFile(Storage, &RFS);
        if (ret>=OK) {
          fplSendTags(arg->key, FPLSEND_STRING, RFS.program,
                      FPLSEND_STRLEN, RFS.length , FPLSEND_DONE);
          Dealloc(RFS.program);
        }
      }
      break;
    case SC_SAVESTRING:		// 'SaveString(filename, string)'
      ret=SaveString(Storage, (char *)arg->argv[0], (char *)arg->argv[1], FPL_STRING_LENGTH(arg, 1));
      ReturnInt=&ret;
      break;
    case SC_GETVAR:
        tempint = ScGetVar((const char *)arg->argv[0], buffer);
        if (tempint>=0) {
            fplSendTags(arg->key, FPLSEND_STRING, buffer,
                        FPLSEND_STRLEN, tempint, FPLSEND_DONE);
        } else
            ret=CANT_FIND_SETTING;
        break;
    case SC_SETVAR:
        tempint=ScSetVar((char *)arg->argv[0], (char *)arg->argv[1], FPL_STRING_LENGTH(arg, 1));
        ReturnInt=&tempint;
        break;

    case SC_RANDOM:	// 'Random()'
        tempint = ScRandom();
        ReturnInt=&tempint;
        break;
    case SC_GETDATE:	// 'GetDate(int BufferID)'
      {
        if (arg->argc && (int)arg->argv[0]>0)
          Storage2=CheckBufID((BufStruct *)arg->argv[0]);
        if (Storage2) {
            const char * retb = ScGetDate(&Storage2,arg->argc,arg->argv,buffer);
            if (retb) fplSendTags(arg->key, FPLSEND_STRING, buffer, FPLSEND_DONE);
        } else
          ret=CANT_FIND_BUFFER;
      }
      break;
    case SC_EXECUTE_BUF:	// 'ExecuteBuffer(int BufferID)'
      {
        register BufStruct *Storage2=Storage;
        String bufstring;
        if (arg->argc)
          Storage2=CheckBufID((BufStruct *)arg->argv[0]);
        if (Storage2) {
          ret=CompactBuf(Storage2->shared, &bufstring);
          if (ret>=0 && bufstring.length) {
            ((AllocStruct *)bufstring.string-1)->lock++;
            Sprintf(buffer, "Buffer execute_%ld", SHS(buffer_number));
            ret=ExecuteFPL(Storage, bufstring.string, EXECUTE_CACHED_SCRIPT,
                           &tempint, buffer);
            ((AllocStruct *)bufstring.string-1)->lock--;
            if (((AllocStruct *)bufstring.string-1)->lock==(UWORD)0x8000)
              Dealloc(bufstring.string);
          }
        } else {
          ret=-(int)RetString(STR_CANT_FIND_BUFFER);
          tempint=FPL_OPEN_ERROR;
        }
        ReturnInt=&tempint;
      }
      break;
    case SC_EXECUTE_FILE:         // ExecuteFile(char *filnamn)
      tempint=FPL_OPEN_ERROR;
//printf("ExecuteFile: %s\n", (char *)arg->argv[0]);
      if(*(char *)arg->argv[0])
        /* only do this if it wasn't a "" name! */
        ret=ExecuteFPL(Storage,
                       (char *)arg->argv[0],
                       TRUE,
                       &tempint,
                       arg->argc>1?(char *)arg->argv[1]:NULL);
      else
        ret = CANT_FIND_FILE;
      ReturnInt=&tempint;
      break;
    case SC_EXECUTE_LATER:         // ExecuteLater(char *FPLstring)
      {
        if (!SendReturnMsg(cmd_EXECUTE, 0, (char *)arg->argv[0], NULL, NULL))
          ret=OUT_OF_MEM;
        ReturnInt=&ret;
      }
      break;

    case SC_GETBUFFERID:
    case SC_GETENTRYID:
      if (!arg->argc)
        tempint=(int)Storage;
      else
        tempint=(int)FindBuffer(arg->argc, (char **)arg->argv);
      if (tempint && argID==SC_GETBUFFERID)
        tempint=(int)((BufStruct *)tempint)->shared;
      ReturnInt=&tempint;
      break;
    case SC_GETWINDOWID:
      if (!arg->argc)
        tempint=(int)BUF(window);
      else
        tempint=(int)CheckWinID((WindowStruct *)arg->argv);
      ReturnInt=&tempint;
      break;
    case SC_READSET:
      if (((char *)arg->argv[0])[0]) {
        int vald=GetSettingName((char *)arg->argv[0]);
        if (vald>=0) {
          register BufStruct *Storage2=Storage;
          int value;
          if (arg->argc>1) {
            if ((int)arg->argv[1]>=0) {
              Storage2=CheckBufID((BufStruct *)arg->argv[1]);
              if (!Storage2)
                Storage2=&Default.BufStructDefault;
            }
          }
          value=ChangeAsk(Storage2, vald, (arg->argc>2)?(int *)&arg->argv[2]:NULL);
          fplSendTags(arg->key, ((sets[vald]->type&15)==ST_STRING)?FPLSEND_STRING:FPLSEND_INT, value, FPLSEND_DONE);
        } else { /* Error */
          ret=CANT_FIND_SETTING;
        }
      } else {
        if (setting_string_pointer || setting_int_pointer)
          fplSendTags(arg->key, setting_string_pointer?FPLSEND_STRING:FPLSEND_INT, setting_string_pointer?(int)*setting_string_pointer:*setting_int_pointer, FPLSEND_DONE);
      }
      break;
    case SC_CCONVERTSTRING:
      {
        String result;
        String input;

        input.string=(char *)arg->argv[0];
        input.length=FPL_STRING_LENGTH(arg, 0);

        if (ConvertString(UsingFact, &input, &result)) {
          fplSendTags(arg->key, FPLSEND_STRING, (char *)result.string, FPLSEND_STRLEN, result.length, FPLSEND_DONE);
          Dealloc(result.string);
        } else
          ret=SYNTAX_ERROR;
      }
      break;
    case SC_FACTCONVERTSTRING:
      {
        String result;

        ret=FACTConvertString(Storage, (char *)arg->argv[0], FPL_STRING_LENGTH(arg, 0), &result, 0);
        if (ret>=OK) {
          fplSendTags(arg->key, FPLSEND_STRING, (char *)result.string, FPLSEND_STRLEN, result.length, FPLSEND_DONE);
          Dealloc(result.string);
        }
      }
      break;
    case SC_GETKEY:
      {
        register int chars;
        register int flag=gkWAIT;
        if (arg->argc>0) {
          if ((int)arg->argv[0]&1)
            flag=0;
          if ((int)arg->argv[0]&2)
            flag|=gkFULLSYNTAX;
          if ((int)arg->argv[0]&4)
            flag|=gkQUALIFIERS;
        }
        if (GetKey(Storage, flag)<=0) {
          chars=0;
        } else
          chars=buffer[81];
        fplSendTags(arg->key, FPLSEND_STRING, buffer, 
                              FPLSEND_STRLEN, chars, FPLSEND_DONE);
      }
      break;
    case SC_KEYPRESS:
      {
        register char *result;
        if (arg->argc)
          result=FindKey(Storage, (char *)arg->argv[0]);
        else
          result=WaitForKey(Storage);
        fplSendTags(arg->key, FPLSEND_STRING, result, FPLSEND_DONE);
      }
      break;
    case SC_DELAY:
      {
        register int temp;
        line=(int)arg->argv[0];
        do {
          temp=line>10?10:line;
          line-=temp;
          Delay(temp);
        } while(!StopCheck(Storage) && line);
      }
      if(line>0)
        fpl_error = FPLERR_PROGRAM_STOPPED;
      break;

    case SC_PROMPT_FONT:
      ret=PromptFont((arg->argc)?((char *)arg->argv[0]):RetString(STR_SELECT_FONT),
                     (arg->argc>1)?((int)arg->argv[1]):3);
      fplSendTags(arg->key, FPLSEND_STRING, buffer, FPLSEND_DONE);
      break;
    case SC_SCROLL_DOWN:	// ScrollDown(int antal)
      arg->argv[0]=(char *)-(int)arg->argv[0];
    case SC_SCROLL_UP:		// ScrollUp(int antal)
      tempint=(int)arg->argv[0];
      ScrollScreen(Storage, tempint, scroll_NORMAL);
      BUF(cursor_y)=FoldFindLine(Storage, BUF(curr_line));
      if (BUF(cursor_y)==0) {
        BUF(cursor_y)=1;
        BUF(curr_line)=BUF(curr_topline);
      } else {
        if (BUF(cursor_y)<0) {
          BUF(cursor_y)=BUF(screen_lines);
          BUF(curr_line)=FoldMoveLine(Storage, BUF(curr_topline), BUF(cursor_y)-1);;
        }
      }
      MoveSlider(Storage);
      tempint=abs(tempint);
      ReturnInt=&tempint;
      break;

    case SC_GETBLOCK:
      /*
       * Return the block as a string!
       */
      {
        String retstring;
        register BlockStruct *block=NULL;
        if (arg->argc && (int)arg->argv[0]) {
          block=FindBlock((BufStruct *)arg->argv[0]);
          if (!block)
            ret=NO_BLOCK_FOUND;
        }
        if (block) {
          ret=CompactBuf(block, &retstring);
          if (ret>=OK) {
            fplSendTags(arg->key, FPLSEND_STRING, retstring.string,
                        FPLSEND_STRLEN, retstring.length,
                        FPLSEND_DONE);
            break;	// quit case
          }
        }
        ret=Block2String(Storage, block, &tempbuffer, &line);
        if(ret>=OK) {
          fplSendTags(arg->key, FPLSEND_STRING, tempbuffer, FPLSEND_STRLEN, line,
                      FPLSEND_DONE);
          Dealloc(tempbuffer);
        }
      }
      break;

    case SC_CONFIRM:
      /*
       * Display a requester!
       */
      tempint=Ok_Cancel(Storage, (char *)arg->argv[0],
                        (arg->argc>1)?(char *)arg->argv[1]:NULL,
                        (arg->argc>2)?(char *)arg->argv[2]:NULL);
      ReturnInt=&tempint;
      break;

    case SC_GETERRNO:
      tempint=ErrNo;
      ReturnInt=&tempint;
      ret=OK;
      break;

    case SC_SETRETURNMSG:
      {
        register char *res=(char *)arg->argv[0];
        if (arg->format[0]==FPL_INTARG) {
          res=GetRetMsg((int)arg->argv[0]);
        }
        SendReturnMsg(cmd_STATUS | cmd_EGO, 0, res, NULL, NULL);
      }
      break;

    case SC_GETERRORMSG:
      fplSendTags(arg->key, FPLSEND_STRING, GetRetMsg((int)arg->argv[0]), FPLSEND_DONE);
      break;

    case SC_GETINT:
      /*
       * Display a requester to prompt for an integer number.	// OBS, ge möjlighet att ställa in defaultvärde och ändra titeln
       */
      if (arg->argc>1)
          line=(int)arg->argv[1];
      tempint = ScGetInt(arg->argc?(char *)arg->argv[0]:RetString(STR_ENTER_NUMBER), line, (arg->argc>1), (arg->argc>2)?(char *)arg->argv[2]:NULL,!(arg->argc>2));
      if (tempint) {
        ReturnInt=&line;
        ret=OK;
      } else
        ret=FUNCTION_CANCEL;
      break;

    case SC_DISPLAYBEEP:
      DisplayBeep(FRONTWINDOW && FRONTWINDOW->window_pointer?FRONTWINDOW->window_pointer->WScreen:NULL);
      break;
    case SC_GETWORD:	// GetWord(line, string_pos)
      {
        register int line, column;
        column=BUF(string_pos);
        line=BUF(curr_line);
        if (arg->argc && (int)arg->argv[0]>0 && (int)arg->argv[0]<=SHS(line))
          line=(int)arg->argv[0];
        if (arg->argc>1)
          column=(int)arg->argv[1];
        GetWord(Storage, line, column, &result);
        fplSendTags(arg->key, FPLSEND_STRING, result.string,
                    FPLSEND_STRLEN, result.length, FPLSEND_DONE);
      }
      break;

    case SC_GETLINE:
      /*
       * Return current line on specified line!
       */
      Storage2=Storage;
      if (arg->argc>1) {
        Storage2=CheckBufID((BufStruct *)arg->argv[1]);
        if (!Storage2)
          Storage2=Storage;
      }
      line=arg->argc>0?(int)arg->argv[0]:Storage2->curr_line;

      /* fix line */
      if(line>Storage2->shared->line)
        line=Storage2->shared->line;
      else if(line<1)
        line=1;
      fplSendTags(arg->key, FPLSEND_STRING, Storage2->shared->text[line].text,
                  FPLSEND_STRLEN, Storage2->shared->text[line].length, FPLSEND_DONE);
      break;
    case SC_GETBYTE:
      col=arg->argc?(int)arg->argv[0]: BUF(cursor_x)+BUF(screen_x);
      line=arg->argc>1?(int)arg->argv[1]:Storage->curr_line;
      if(line>SHS(line))
        line=SHS(line);
      else if(line<1)
        line=1;
      tempint=Col2Byte(Storage, col, RAD(line), LEN(line));
      ReturnInt=&tempint;
      break;
    case SC_GETCHAR:
      // Return current character under cursor or on specified column!
    case SC_GETCURSOR:
      // Return cursor position for the specified string position!
      col=arg->argc?(int)arg->argv[0]: BUF(string_pos);
      line=arg->argc>1?(int)arg->argv[1]:Storage->curr_line;

      /* fix line */
      if(line>SHS(line) || line<0)
        line=SHS(line);
      else if(line<1)
        line=1;

      /* fix column */
      if(col>LEN(line)+(SHS(line)==line) || col<0)
        col=LEN(line)-1;
      if (argID==SC_GETCHAR)
        fplSendTags(arg->key, FPLSEND_INT, LEN(line)>(signed char)col?RAD(line)[col]:-1, FPLSEND_DONE);
      else {	// SC_GETCURSOR
        tempint=Byte2Col(Storage, col, RAD(line));
        ReturnInt=&tempint;
      }
      break;

    case SC_SETINFO:
      /*
       * Change a setting of specified buffer.
       *  0 = Default
       * -1 = Current
       * -2 = Local All
       */
      {
        register int counter;
        register int updt=0;
        register int vald;
        Storage2=Storage;
        if ((int)arg->argv[0]>=0) {
          Storage2=&Default.BufStructDefault;
          if ((int)arg->argv[0]>0)
            Storage2=CheckBufID((BufStruct *)arg->argv[0]);
          if (!Storage2) {
            ret=CANT_FIND_BUFFER;
            break;
          }
        }
        if (arg->argc>1) {
          for (counter=1; (counter+2)<=arg->argc; counter+=2) {
            if (arg->format[counter]==FPL_STRARG) {
              vald=GetSettingName(arg->argv[counter]);
              if (vald>=0) {
                if (((sets[vald]->type&15)==ST_STRING)?(arg->format[counter+1]==FPL_STRARG):(arg->format[counter+1]==FPL_INTARG))
                  updt|=Change(Storage2, ((int)arg->argv[0]==-2)?cs_LOCAL_ALL:NULL, vald, (int)arg->argv[counter+1]);
              } else
                ret=CANT_FIND_SETTING;
            } else {
              fpl_error=FPLERR_SYNTAX_ERROR;
              ret=SYNTAX_ERROR;
              break;
            }
          }
          if (updt) {
            if ((int)arg->argv[0]==-2)
              updt|=SE_ALL;
            ReDrawSetting(Storage2, updt);
          }
        }
      }
      ReturnInt=&ret;
      break;

    case SC_SETCOPY:
    case SC_PROMPTINFO:
      ret=ScPromptInfo(Storage, argID, arg);
      ReturnInt=&ret;
      break;
    case SC_GETSTRING:
      /*
       * Display a requster to prompt for a string.
       */
      buffer[0]=0;
      if (arg->argc>0)
        strcpy(buffer, (char *)arg->argv[0]);
      tempint = ScGetString(buffer,
                            (arg->argc>1)?(char *)arg->argv[1]:RetString(STR_ENTER_STRING),
                            (arg->argc>2)?(char *)arg->argv[2]:NULL,
                            !(arg->argc>2));
      if (tempint)
        fplSendTags(arg->key, FPLSEND_STRING, buffer, FPLSEND_DONE);
      else
        ret=FUNCTION_CANCEL;
      break;

    case SC_GETFILE:
      /*
       * Display a file requester.
       */
      {
        struct rtFileList *filelist=NULL;
        char *fullname=Malloc(FILESIZE);
        long dims[]={0, -1};
        if (fullname) {
          int flags=0;
          fullname[0]=0;
          if (arg->argc) {
            stccpy(fullname, arg->argv[0], FILESIZE);
            fullname[FILESIZE-1]=0;
          }
          if (arg->argc>3) {
            register int count=0;
            while (((char *)(arg->argv[3]))[count]) {
              switch (((char *)(arg->argv[3]))[count]) {
              case 'd':
              case 'D':
                flags|=FREQF_NOFILES;
                break;
              case 's':
              case 'S':
                flags|=FREQF_SAVE;
                break;
              case 'm':
              case 'M':
                if (arg->argc>4)	// Must be an extra argument
                  flags|=FREQF_MULTISELECT;
                break;
              }
              count++;
            }
          }
          if(PromptFile(Storage, fullname, arg->argc>1?(char *)arg->argv[1]:NULL, arg->argc>2?(char *)arg->argv[2]:NULL, flags, (APTR *)&filelist)) {
            if (flags&FREQF_MULTISELECT) {
              struct rtFileList *filetemp=filelist;
              if (filetemp) {
                struct fplRef ref;
                tempint= fileReqCountFiles(filelist);
                ref.Dimensions=1;
                ref.ArraySize=(long *)&tempint;
                fplReferenceTags(Anchor, (void *)(arg->argv[4]),
                                 FPLREF_ARRAY_RESIZE, &ref,
                                 FPLREF_END);
                filetemp=filelist;

                int tempint = 0;
                do {
                    dims[0]=tempint;
                    strmfp(buffer, fullname, filetemp->Name);
                    fplReferenceTags(Anchor, (void *)(arg->argv[4]),
                                     FPLREF_ARRAY_ITEM, &dims[0],
                                     FPLREF_SET_MY_STRING, buffer,
                                     FPLREF_END);
                    tempint++;
                    filetemp=filetemp->Next;
                } while (filetemp);
              }
              ReturnInt=&tempint;	// Return number of entries.
              fileReqFreeList(filelist);
            } else
                fplSendTags(arg->key, FPLSEND_STRING, fullname, FPLSEND_DONE);
          } else
              ret=FUNCTION_CANCEL;
          Dealloc(fullname);
        }
      }
      break;
    case SC_GETFILELIST:
      tempint=ScGetFileList(Storage, arg);
      ReturnInt=&tempint;
      break;
    case SC_GETLIST:
      tempint=ScGetList(Storage, arg);
      ReturnInt=&tempint;
      break;
    case SC_SETHOOK:
    case SC_SETHOOKPAST:
      /*
       * Add a hook to a FrexxEd function!
       */
      line = AddHook(Storage, arg->argv[0], /* function to hook */
		              arg->argv[1], /* function to call */
			      arg->argc>2?(char *)arg->argv[2]:NULL, /* setting */
			      (argID==SC_SETHOOKPAST)? /* post-hook? */
			      HOOK_POSTHOOK:0);
      fplSendTags(Anchor, FPLSEND_INT, line?UNKNOWN_COMMAND:OK, FPLSEND_DONE);
      break;
    case SC_HOOKCLEAR:
      /*
       * Remove hook(s)
       */

      line = HookClear(Storage,
                       arg->argc>0?(char *)arg->argv[0]:NULL, /* hooked function */
		       arg->argc>1?(char *)arg->argv[1]:NULL, /* hook name/function */
                       arg->argc>2?(char *)arg->argv[2]:NULL);/* setting */

      /* return number of hooks removed */
      fplSendTags(Anchor, FPLSEND_INT, line, FPLSEND_DONE);
      break;

    case SC_STATUS:
      {
        Storage2=Storage;
        if (arg->argc && arg->argv[0])
          Storage2=CheckBufID((BufStruct *)arg->argv[0]);
        if (Storage2) {
          register int type=3;
          if (arg->argc>2)
            type=(int)arg->argv[2];
          if (arg->argc>1 && *(char *)arg->argv[1])
            Status(Storage2, arg->argv[1], type|0x80);
          else {
            if (type&1)
              Showname(Storage2);
            if (type&2)
              Showplace(Storage2);
          }
        }
      }
      break;
    case SC_CURSOR_ACTIVATE:
      tempint=!cursorhidden;
      ReturnInt=&tempint;
      if (arg->argc) {
        if (!arg->argv[0]) {
          if (CursorOnOff)
            CursorXY(Storage, -1, -1);
          cursorhidden=TRUE;
        } else
          cursorhidden=FALSE;
      }
      break;
    case SC_CURSOR_STACK:
      {
#define MAX_CURSOR_STACK 7	// jämn 2-multipel
        static CursorPosStruct cpos[MAX_CURSOR_STACK+1];
        static lastpos=0;
        if (arg->argc && (int)arg->argv[0]>0) {
          lastpos=(lastpos-1)&MAX_CURSOR_STACK;
          if (cpos[lastpos].curr_line)
            RestoreCursorPos(Storage, &cpos[lastpos], TRUE);
        } else {
          SaveCursorPos(Storage, &cpos[lastpos]);
          lastpos=(lastpos+1)&MAX_CURSOR_STACK;
        }
      }
      break;

    case SC_VISIBLE:
      /*
       * Change the Visible flag!
       * 0=off, 1=on, no input=nothing.
       * Return the state of Visible flag before function call
       */
      {
        register BOOL updt=FALSE;
        tempint=(Visible==VISIBLE_ON)?1:0;
        ReturnInt=&tempint;
        if (arg->argc) {
          if (arg->argv[0]) {
            if (Visible!=VISIBLE_ON)
              updt=TRUE;
          } else {
            CursorXY(Storage, -1, -1);
            Visible=VISIBLE_OFF;
          }
          if (!FrexxEdStarted || !FRONTWINDOW ||
              (FRONTWINDOW->window_pointer && FRONTWINDOW->window_pointer->Height<FRONTWINDOW->window_minheight)) {
            Visible=VISIBLE_OFF;
          } else if (updt) {
            ClearVisible();
            CursorXY(Storage, BUF(cursor_x), BUF(cursor_y));
          }
        }
      }
      break;

    case SC_AREXXRESULT:
        ScArexxResult(arg->argc, arg->argv);
        break;
    case SC_AREXXREAD:
      /*
       * Result string to send to ARexx if we were invoke from there!
       */
      if(RexxHandle) {
        char *string;
        if(!GetARexxVar(RexxHandle, (char *)arg->argv[0], &string))
          fplSendTags(arg->key, FPLSEND_STRING, string, FPLSEND_DONE);
        else
          ErrNo = SYNTAX_ERROR;
      }
      break;

    case SC_AREXXSET:
      /*
       * Result string to send to ARexx if we were invoke from there!
       */
      if(RexxHandle) {
        tempint = SetARexxVar(RexxHandle,
                              (char *)arg->argv[0],
                              (char *)arg->argv[1]);
        ReturnInt=&tempint;
      }
      break;

    case SC_AREXXSEND:
      /* the function below takes care of sending the result by itself! */
      SendARexxMsg(Storage,
                   RexxHandle,
                   arg->key,
                   (char *)arg->argv[0],
                   (char *)arg->argv[1],
		   arg->argc>2?(int)arg->argv[2]:0);
      break;

    case SC_FINDPORT:
      line=arg->argc>1?(int)(arg->argv[1])*5:0;
      do {
        Forbid();
        tempint = (int) FindPort(arg->argv[0]);
        Permit();
	if(tempint || !line)
	  break;
	line--;
        Delay(10); /* a 5th of a second */
      } while(!StopCheck(Storage));
      if(line && !tempint)
        fpl_error  = FPLERR_PROGRAM_STOPPED;
      else
        ReturnInt=&tempint;
      break;      

    case SC_CLIP2STRING:
      /*
       * Hämtar en sträng från clipboarden.
       */
      {
        int len;
        char *string;
        if(Clip2String(Storage, &string, &len, (int)arg->argv[0])>=OK)
          fplSendTags(arg->key, FPLSEND_STRING, string,
                                FPLSEND_STRLEN, len,
                                FPLSEND_DONE);
        Dealloc(string);
      }
      break;

    case SC_LOG_SETTING:
      LogSetting((char *)arg->argv[0], (char *)arg->argv[1], (long)arg->argv[2], arg->format[2]);
      break;
    case SC_SYSTEM:
        tempint = ScSystem(Storage,arg);
        ReturnInt=&tempint;
        break;

    case SC_SET_SYSTEM_FONT:	// 'setfont(char *fontnamn)'
    case SC_SET_REQUEST_FONT:
      ret=GetFont(Storage, arg->argv[0], 3-((argID==SC_SET_SYSTEM_FONT)?2:0)-((argID==SC_SET_REQUEST_FONT)?1:0));
      ReturnInt=&ret;
      break;

    case SC_SEARCHSET:		// 'SearchSet(char *flag, char *search, char *replace)'
    case SC_REPLACESET:		// 'ReplaceSet(char *flag, char *search, char *replace)'
      if (arg->argc)
        SetSearch(arg->argv[0], (arg->argc>1)?(char *)arg->argv[1]:NULL, (arg->argc>2)?(char *)arg->argv[2]:NULL);
      else
        ret=SearchAsk(Storage, argID==SC_REPLACESET);
      ReturnInt=&ret;
      break;

    case SC_MENUCLEAR:
      menu_clear();
      break;
    case SC_MENUADD:
      /* TYPE, STRING, FPL (all strings) */
      tempint=MenuAdd(arg->argc, (char **)arg->argv);
      ReturnInt=&tempint;
      break;
    case SC_MENUREAD:
      tempint=ScMenuRead(arg);
      ReturnInt=&tempint;
      break;
    case SC_MENUDELETE:
      tempint=ScMenuDelete(arg);
      ReturnInt=&tempint;
      break;

    case SC_MENUBUILD:
      {
        WindowStruct *wincount;
        wincount=FRONTWINDOW;
        while (wincount) {
          menu_detach(&menu, wincount);
          tempint=menu_attach(wincount);
          wincount=wincount->next;
        }
      }
      ReturnInt=&tempint;
      break;

    case SC_TIMER_START:
      tempint=(int)AddTimerEvent((int)arg->argv[0], (char *)arg->argv[1], (int)arg->argv[2], (int)arg->argv[3]);
      ReturnInt=&tempint;
      break;
    case SC_TIMER_REMOVE:
      FreeTimeRequest((FrexxTimerRequest *)arg->argv[0]);
      break;
    case SC_MATCH_SEARCH:
      {
        String string;
        ret=SearchMatch(Storage, arg->argv[0], arg->argv[1], &string, arg->argc>2?(int)arg->argv[2]:0, arg->argc>3?(char *)arg->argv[3]:"");
        if (ret>=OK) {
          fplSendTags(arg->key, FPLSEND_STRING, string.string,
                                FPLSEND_STRLEN, string.length,
                                FPLSEND_DONE);
          Dealloc(string.string);
        }
      }
      break;

    case SC_SORT:   // Sort(&array, antal, flag);
      /* flag=0-normal, 1-incase, 2-backward (bits) */
      ret=ScSort(arg);
      ReturnInt=&ret;
      break;
    case SC_STCGFP:
    case SC_STCGFN:
      if (argID==SC_STCGFP)
        Stcgfp(buffer, arg->argv[0]);
      else
        Stcgfn(buffer, arg->argv[0]);
      fplSendTags(arg->key, FPLSEND_STRING, buffer, FPLSEND_DONE);
      break;
    case SC_STRMFP:
      strmfp(buffer, arg->argv[0], arg->argv[1]);
      fplSendTags(arg->key, FPLSEND_STRING, buffer, FPLSEND_DONE);
      break;
    case SC_BSEARCH:
      tempint=BSearch(arg);
      ReturnInt=&tempint;
      break;
    case FPL_GENERAL_ERROR:
      /*
       * Interpretation error has occured! Display a window with additional
       * information about the failure.
       */
      {
        char tempbuffer[FPL_ERRORMSG_LENGTH*2];
	void *array[3];
	int len;
        fplGetErrorMsg(Anchor, (long) arg->argv[0], tempbuffer),
        fplSendTags(arg->key,
  		  FPLSEND_GETVIRLINE, &array[0],
  		  FPLSEND_GETPROGNAME, &array[1],
  		  FPLSEND_DONE);
  	
	array[2] = tempbuffer;
  
        if(RunHook(Storage, SC_ERROR, 3, (char **)array, 0))
          ret=FUNCTION_CANCEL;

        if (NewStorageWanted) {
          userdata->buf = NewStorageWanted;
          Storage=NewStorageWanted;
          clear_all_currents=TRUE;
        }
	if(!ret) {
	  len = strlen(tempbuffer);
          Sprintf(&tempbuffer[len],
                  RetString(STR_FPL_ERROR_ADD),
                  array[0],
                  array[1]);

	  /* Display the error message window */
          Ok_Cancel(Storage,
	            tempbuffer,
		    RetString(STR_FPL_HALTED),
		    RetString(STR_OK_GADGET));
	  tempbuffer[len]=0; /* re-terminate the string at the previous end! */
	}
        RunHook(Storage, SC_ERROR, 3, (char **)array, HOOK_POSTHOOK);
        if (NewStorageWanted) {
          userdata->buf =NewStorageWanted;
          Storage=NewStorageWanted;
          clear_all_currents=TRUE;
        }

      }
      fpl_error=FPL_OK;
      break; /* this return code is ignored anyway */
    case SC_REQUEST_WINDOW:
      tempint=RequestWindow(Storage, arg);
      ReturnInt=&tempint;
      break;
    case SC_FOLD:
      Storage2=Storage;
      if (arg->argc>=3 && (int)arg->argv[2]>0)
        Storage2=CheckBufID((BufStruct *)arg->argv[2]);
      if (Storage2) {
        if (arg->argc<2) {
          if (Storage2->block_exists) {
            if (Storage2->block_exists==be_MARKING) {
              Storage2->block_exists=be_NONE;
            }
            Fold(Storage2, Storage2->block_begin_y, Storage2->block_end_y);
          } else
            ret=NO_BLOCK_MARKED;
        } else
          Fold(Storage2, (int)arg->argv[0], (int)arg->argv[1]);
      } else
        ret=CANT_FIND_BUFFER;
      break;
    case SC_FOLD_DELETE:
    case SC_FOLD_SHOW:
    case SC_FOLD_HIDE:
      Storage2=Storage;
      if (arg->argc>1 && (int)arg->argv[1]>0)
        Storage2=CheckBufID((BufStruct *)arg->argv[1]);
      if (Storage2) {
        switch (argID) {
        case SC_FOLD_DELETE:
          UnFold(Storage2, arg->argc>0?(int)arg->argv[0]:Storage2->curr_line);
          break;
        case SC_FOLD_SHOW:
          FoldShow(Storage, arg->argc>0?(int)arg->argv[0]:Storage2->curr_line);
          break;
        case SC_FOLD_HIDE:
          FoldHide(Storage, arg->argc>0?(int)arg->argv[0]:Storage2->curr_line);
          break;
        }
      } else
        ret=CANT_FIND_BUFFER;
      break;
    case SC_STRING_TO_UPPER:
    case SC_STRING_TO_LOWER:
    case SC_STRING_TO_UPPLOW:
      {
        register int len;
        register char *string;
        register char tkn;
        register FACT *fact=BUF(using_fact);
        if (arg->argc>1) {
          fact=FindFACT(arg->argv[1]);
        }
        len=FPL_STRING_LENGTH(arg, 0);
        if (len && fact) {
          string=fplAllocString(Anchor, len);
          if (string) {
            while (len) {
              len--;
              tkn=((char *)arg->argv[0])[len];
              if (argID!=SC_STRING_TO_LOWER &&
                  fact->xflags[tkn] & (factx_LOWERCASE))
                tkn=fact->cases[tkn];
              else if (argID!=SC_STRING_TO_UPPER &&
                       fact->xflags[tkn] & (factx_UPPERCASE))
                tkn=fact->cases[tkn];
              string[len]=tkn;
            }
            fplSendTags(arg->key, FPLSEND_STRING, string,
                                  FPLSEND_STRLEN, FPL_STRING_LENGTH(arg, 0),
                                  FPLSEND_DONTCOPY_STRING, TRUE,
                                  FPLSEND_DONE);
          }
        }
      }
      break;
    case SC_WINDOW_ALLOC:
      {
        WindowStruct *win=NULL;
        ret=OUT_OF_MEM;
        tempint=NULL;
        ReturnInt=&tempint;
        CursorXY(Storage, -1, -1);
        if (arg->argc) {
          Storage2=CheckBufID((BufStruct *)arg->argv[0]);
          if (Storage2 && Storage2->window)
            Storage2=NULL;
          if (arg->argc>1)
            win=CheckWinID((WindowStruct *)arg->argv[1]);
        }
        if (win=CreateNewWindow(win, Storage2, Storage)) {
          tempint=(int)win;
          ret=OK;
        }
      }
      break;
    case SC_WINDOW_OPEN:
      {
        WindowStruct *win=BUF(window);
        ret=SYNTAX_ERROR;
        if (arg->argc)
          win=CheckWinID((WindowStruct *)arg->argv[0]);
        if (win) {
          ret=OUT_OF_MEM;
          if (!OpenUpScreen(win)) {
            UpdateWindow(win);
            ret=OK;
          }
        }
      }
      break;
    case SC_WINDOW_CLOSE:
    case SC_WINDOW_FREE:
      {
        WindowStruct *win=BUF(window);
        ret=FUNCTION_CANCEL;
        if (arg->argc)
          win=CheckWinID((WindowStruct *)arg->argv[0]);
        if (win) {
          CursorXY(NULL, -1, -1);
          CloseMyScreen(win);
          if (argID==SC_WINDOW_FREE)
            FreeWindow(win);
          ret=OK;
        }
      }
      break;
    case SC_NEXT_WINDOW:
    case SC_PREV_WINDOW:
      {
        WindowStruct *win=BUF(window);
        ret=FUNCTION_CANCEL;
        if (arg->argc)
          win=CheckWinID((WindowStruct *)arg->argv[0]);
        if (argID==SC_NEXT_WINDOW) {
          if (win)
            win=win->next;
          if (!win)
            win=FRONTWINDOW;
        } else {
          if (win)
            win=win->prev;
          if (!win) {
            win=FRONTWINDOW;
            while (win->next)
              win=win->next;
          }
        }
        tempint=(int)win;
        ReturnInt=&tempint;
      }
      break;
/*
  {"FaceAdd",           SC_FACEADD,             'I', "IISSs",1} // (faceID, styleID, addstring, flags, endstring) Add a string as a face
  {"FaceGet",           SC_FACEGET,             'I', "Si", 1}   // (face name, mode) create/get faceID with the specified name
  {"FaceStyle",         SC_FACESTYLE,           'I', "SSII",1}  // (style name, mode, fg, bg) create/get styleID
*/
    case SC_FACEADD:
      ReturnInt=&ret;
      ret = FaceAdd((void *)arg->argv[0],
                    (int)arg->argv[1],
                    (char *)arg->argv[2],
                    FPL_STRLEN(arg->argv[2]),
                    (char *)arg->argv[3],
                    arg->argc>4?(char *)arg->argv[4]:NULL,
                    arg->argc>4?FPL_STRLEN(arg->argv[4]):0);
      break;

    case SC_FACEGET:
      ReturnInt=&ret;
      ret = (int)FaceGet((char *)arg->argv[0],
                         arg->argc>1?(int)arg->argv[1]:0);
      break;
      
    case SC_FACESTYLE:
      ReturnInt=&ret;
      ret = (int)FaceStyle((char *)arg->argv[0],
                           (char *)arg->argv[1],
                           (int)arg->argv[2],
                           (int)arg->argv[3],
                           arg->argc>4?(int)arg->argv[4]:0);
      break;
      
    case SC_FACEREAD:
      ReturnInt = &ret;
      ret = ScFaceRead(arg);
      break;
    }

    /* check for and run any post hook */
    ErrNo=ret;	// The returnvalue is the same as the Error value.
    if (argID>=0 && Default.posthook[argID&~LOCKINFO_BITS])
      RunHook(Storage, argID, arg->argc, (char **)arg->argv, HOOK_POSTHOOK);
    Search.lastpos|=BUF(curr_line)+BUF(string_pos);
  }
  if (NewStorageWanted) {
    Storage=NewStorageWanted;
    clear_all_currents=TRUE;
//    ret=OK;
  }
  userdata->buf = Storage;

  ErrNo=ret;	// The returnvalue is the same as the Error value.

  if (ReturnInt)
    fplSendTags(arg->key, FPLSEND_INT, *ReturnInt, FPLSEND_DONE);

  if (BUF(window) && BUF(window)->window_pointer &&
      BUF(window)->window_pointer->Height>=BUF(window)->window_minheight) {
    if (Visible==VISIBLE_ON && BUF(window)->Visible==VISIBLE_ON) {
      if (UpDtNeeded)
        ClearVisible();
//      UpDtNeeded=UD_NOTHING;
      if (!CursorOnOff) {
        CursorOnOff=TRUE;
        CursorXY(Storage, BUF(cursor_x), BUF(cursor_y));
      }
    }
  }
  if (BUF(block_exists)==be_MARKING)
    UpdateBlock(Storage);

  return(fpl_error);
}

int RequestWindow(BufStruct *Storage, struct fplArgument *arg)
{
#define MAX_STRING_LENGTH 1000
  int tempint;

  ButtonStruct Buttons;
  ButtonArrayStruct *array;
  char *remember=NULL;
  int ret;
  int col;
  int width=0;

  ret=OUT_OF_MEM;
  tempint=0;
  array=(ButtonArrayStruct *)OwnAllocRemember(&remember, (arg->argc/2)*sizeof(ButtonArrayStruct));
  if (array) {
    register int count, antal=0;
    int value;
    char **listarray=NULL;
    char *listarraystring=NULL;
    char *listarraystring2=NULL;
    char *listarraystringname1=NULL;
    char *listarraystringname2=NULL;
    int listarrayantal=-1;
    int listarraystore;
    int listarraystore2;

    memset(array, 0, (arg->argc/2)*sizeof(ButtonArrayStruct));
    InitButtonStruct(&Buttons);
    Buttons.array=array;
    Buttons.OkCancel=TRUE;
    Buttons.toptext=(char *)arg->argv[0];
    count=1;
    if (count<arg->argc && arg->format[count]==FPL_INTARG) {
      if ((int)arg->argv[count]>=0) {
        Buttons.string_width=(int)arg->argv[count];
        width=0;
      }
      count++;
    }
    for (; count<arg->argc; count++) {
      ret=SYNTAX_ERROR;
      if (arg->format[count]==FPL_STRARG) {
        array[antal].name=(char *)arg->argv[count];
        count++;
        if (count>=arg->argc)
          break;
        if (arg->format[count]==FPL_STRARG) {
          int type=ST_NOSAVE|ST_GLOBAL|ST_BOOL|ST_USERDEFINED;
          ExtractType(arg->argv[count], &type, &tempint, &tempint, &tempint);
          count++;
          if (count>=arg->argc)
            break;
          array[antal].settingnumber=(int)arg->argv[count];
          array[antal].cycletext=NULL;
          array[antal].typesfull=type;
          array[antal].types=type&15;
          array[antal].FPLaddition=NULL;
          if ((type&15)==ST_STRING && arg->format[count]==FPL_STRVARARG) {
            fplReferenceTags(Anchor, arg->argv[count],
                             FPLREF_GET_STRING, (char **)&value,
                             FPLREF_DONE);
              array[antal].flags=(int)Strdup((char *)value);
              ret=OK;
          } else if ((type&15)!=ST_STRING && arg->format[count]==FPL_INTVARARG) {
            fplReferenceTags(Anchor, arg->argv[count],
                             FPLREF_GET_INTEGER, (int *)&value,
                             FPLREF_DONE);
            array[antal].flags=*(int *)value;
            if ((type&15)==ST_CYCLE) {
              count++;
              if (count>=arg->argc)
                break;
              if (arg->format[count]==FPL_STRARG) {
                register char *pos=(char *)arg->argv[count];
                register int no=1;
                while (*pos) {
                  if (*pos=='|')
                    no++;
                  pos++;
                }
                if (array[antal].flags>no)
                  array[antal].flags=0;
                array[antal].cycletext=(char **)OwnAllocRemember(&remember, strlen(arg->argv[count])+no*sizeof(char *)+8);
                if (array[antal].cycletext) {
                  register char **start=array[antal].cycletext;
                  register char *store=((char *)array[antal].cycletext)+(no+1)*sizeof(char *);
                  pos=(char *)arg->argv[count];
                  *start=store;
                  start++;
                  do {
                    if (*pos=='|') {
                      *store=0;
                      store++;
                      *start=store;
                      start++;
                    } else {
                      *store=*pos;
                      store++;
                    }
                  } while (*pos++);
                  *start=NULL;
                } else
                  break;
              } else
                break;
            }
            ret=OK;
          } else if ((type&15)==ST_ARRAY && arg->format[count]==FPL_STRARRAYVARARG) {
            struct fplRef ref;
            long size;
             // Array
            fplReferenceTags(Anchor, arg->argv[count],
                             FPLREF_ARRAY_INFO, &ref,
                             FPLREF_DONE);
            if(1 == ref.Dimensions) {
              count++;
              if (count>=arg->argc)
                break;
              if (arg->format[count]==FPL_STRVARARG) {
                fplReferenceTags(Anchor, arg->argv[count],
                                 FPLREF_GET_STRING, (char **)&value,
                                 FPLREF_DONE);
                listarraystore=(int)arg->argv[count];
                count++;
                if (count>=arg->argc)
                  break;
                if (arg->format[count]==FPL_INTARG) {
                  size=ref.ArraySize[0];
                  if ((int)arg->argv[count]>=0 && size>(int)arg->argv[count])
                    size=(int)arg->argv[count];
                  listarray=(char **)OwnAllocRemember(&remember, size*sizeof(char *));
                  if (listarray || !size) {
                    listarraystring=(char *)OwnAllocRemember(&remember, MAX_STRING_LENGTH);
                    if (listarraystring) {
                      long dims[]={0, -1};
                      listarraystring[0]=0;
                      listarraystringname1=array[antal].name;
                      if (value)
                        stccpy(listarraystring, (char *)value, MAX_STRING_LENGTH);
                      listarrayantal=size;
                      for(col=0; col<listarrayantal; col++) {
                        dims[0]=col;
                        fplReferenceTags(Anchor, arg->argv[count-2],
                                         FPLREF_ARRAY_ITEM, &dims[0],
                                         FPLREF_GET_STRING, &listarray[col],
                                         FPLREF_DONE);
                      }
                      antal--;	// Don't count this.
                      ret=OK;
                    } else
                      break;
                  } else
                    break;
                } else
                  break;
              } else
                break;
            } else
              break;
          } else if ((type&15)==ST_ARRAY && arg->format[count]==FPL_STRVARARG &&
                      listarrayantal>=0) {
            fplReferenceTags(Anchor, arg->argv[count],
                             FPLREF_GET_STRING, (char **)&value,
                             FPLREF_DONE);
            listarraystore2=(int)arg->argv[count];
            listarraystring2=OwnAllocRemember(&remember, MAX_STRING_LENGTH);
            if (listarraystring2) {
              listarraystring2[0]=0;
              listarraystringname2=array[antal].name;
              if (value)
                stccpy(listarraystring2, (char *)value, MAX_STRING_LENGTH);
              ret=OK;
            }

          } else
            break;
        } else
          break;
      } else
        break;
      antal++;
    }
    tempint=0;
    if (ret==OK) {
      Buttons.antal=antal;
      if (antal && listarrayantal<0) {
        if (ButtonPress(Storage, &Buttons)==0)
          tempint=1;
      } else {
         if (listarrayantal>=0) {
           register int retval;
           retval=Reqlist(Storage,
                          REQTAG_ARRAY, listarray, listarrayantal,
                          REQTAG_BUTTON, &Buttons,
                          REQTAG_TITLE, Buttons.toptext,
                          REQTAG_STRING1, listarraystring, MAX_STRING_LENGTH,
                          REQTAG_STRING2, listarraystring2, MAX_STRING_LENGTH,
                          REQTAG_STRINGNAME1, listarraystringname1,
                          REQTAG_STRINGNAME2, listarraystringname2,
                          REQTAG_WIDTH, width,
                          REQTAG_END);
           if (retval>=0 || (retval!=rl_CANCEL && retval!=rl_ERROR))
             tempint=1;
         } else
           fpl_error=FPLERR_SYNTAX_ERROR;
      }
    }
    {
      register int count;
      for (count=0; count<antal; count++) {
        if (tempint) {
          if (array[count].types==ST_STRING) {
            fplReferenceTags(Anchor, (void *)(array[count].settingnumber),
                             FPLREF_SET_MY_STRING, (char *)array[count].flags,
                             FPLREF_END);
          } else {
            fplReferenceTags(Anchor, (void *)(array[count].settingnumber),
                             FPLREF_SET_INTEGER, array[count].flags,
                             FPLREF_END);
          }
        }
        if ((array[count].types & 15)==ST_STRING)
          Dealloc((char *)array[count].flags);
      }
      if (listarrayantal>=0 && tempint) {
        fplReferenceTags(Anchor, (void *)(listarraystore),
                         FPLREF_SET_MY_STRING, listarraystring,
                         FPLREF_END);
        if (listarraystring2) {
          fplReferenceTags(Anchor, (void *)(listarraystore2),
                           FPLREF_SET_MY_STRING, listarraystring2,
                           FPLREF_END);
        }
      }
    }
    OwnFreeRemember(&remember);
  }
  return(tempint);
}

static int ScSort(struct fplArgument *arg)
{
  int ret=OUT_OF_MEM;
  int col;
  char **array;
  int *length;
  char **stringarray[2];
  long dims[]={0, -1};
  struct fplRef ref;
  int antal;
   // Array
  fplReferenceTags(Anchor, arg->argv[0],
                   FPLREF_ARRAY_INFO, &ref,
                   FPLREF_DONE);
  antal=ref.ArraySize[0];
  if (arg->argc>1 && (int)arg->argv[1]>0 && antal>(int)arg->argv[1])
    antal=(int)arg->argv[1];
  array=(char **)Malloc(antal*sizeof(char *));
  if (array) {
    length=(int *)Malloc(antal*sizeof(int *));
    if (length) {
      for(col=0; col<antal; col++) {
        dims[0]=col;
        fplReferenceTags(Anchor, arg->argv[0],
                         FPLREF_ARRAY_ITEM, &dims[0],
                         FPLREF_GET_STRING, &array[col],
                         FPLREF_DONE);
        length[col]=FPL_STRLEN(array[col]);
      }
      stringarray[0]=array;
      stringarray[1]=(char **)length;
      ShellSort(stringarray, 1, 1, antal, arg->argc>2?(int)arg->argv[2]:0);
      for (col=0; col<antal; col++) {
        register char *string=fplAllocString(Anchor, length[col]);
        if (string) {
          memcpy(string, array[col], length[col]);
          dims[0]=col;
        }
        array[col]=string;
      }
      for (col=0; col<antal; col++) {
        dims[0]=col;
        fplReferenceTags(Anchor, (void *)(arg->argv[0]),
                         FPLREF_ARRAY_ITEM, &dims[0],
                         FPLREF_SET_STRING, array[col],
                         FPLREF_END);
      }
      ret=OK;
      Dealloc(length);
    }
    Dealloc(array);
  }
  return(ret);
}
static int __asm bsearch_cmp_array(register __a0 void *arg, register __d1 int count)
{
  char *string;
  int ret;
  long dims[2];
  dims[0]=count;
  dims[1]=-1;
  fplReferenceTags(Anchor, (void *)(((struct fplArgument *)arg)->argv[0]),
                   FPLREF_ARRAY_ITEM, &dims[0],
                   FPLREF_GET_STRING, &string,
                   FPLREF_END);
  if (((struct fplArgument *)arg)->argc>3 && ((int)((struct fplArgument *)arg)->argv[3])&1)
    ret=Stricmp(string, ((struct fplArgument *)arg)->argv[1]);
  else
    ret=strcmp(string, ((struct fplArgument *)arg)->argv[1]);
  if (((struct fplArgument *)arg)->argc>3 && ((int)((struct fplArgument *)arg)->argv[3])&2)
    ret=-ret;
  return(ret);
}

static int BSearch(struct fplArgument *arg)
{
  int ret=-1;
  struct fplRef ref;
  int antal;
   // Array
  fplReferenceTags(Anchor, arg->argv[0],
                   FPLREF_ARRAY_INFO, &ref,
                   FPLREF_DONE);
  antal=ref.ArraySize[0];
  if (arg->argc>2 && (int)arg->argv[2]>0 && antal>(int)arg->argv[2])
    antal=(int)arg->argv[2];
  if (antal)
    ret=Bsearch(arg, antal, &bsearch_cmp_array);
  return(ret);
}
#ifdef LOG_FILE_EXECUTION
int LogFileExecution(char *file)
{
  int count=-1;
  int pos;

  do {
    count++;
    buffer[count]=ToUpper(file[count]);
  } while(file[count]);
  if ((pos=FindFileExecution(buffer))>=0) {
    if (filelog_maxalloc<=filelog_antal) {
      register char **temp;
      temp=(char **)Remalloc((char *)filelog_list, (filelog_maxalloc+20)*sizeof(char *));
      if (!temp)
        return(OUT_OF_MEM);
      filelog_list=temp;
      filelog_maxalloc+=18;
    }
    filelog_antal++;
    MoveUp4(&filelog_list[pos+1], &filelog_list[pos], filelog_antal-pos);
    filelog_list[pos]=Strdup(buffer);
  }
  return(OK);
}

#ifdef LOG_FILE_EXECUTION
int FindFileExecution(char *file)
{
  int vald;

  vald=0;
  if (*file=='<')
    file++;
  while(file[vald] && file[vald]!='>') {
    buffer[vald]=ToUpper(file[vald]);
    vald++;
  }
  buffer[vald]=0;
  vald=0;
  {
    register int step=(filelog_antal+1)>>1;
    register int counter=step-1;
  
    while (step) {
      register int result=strcmp(filelog_list[counter], buffer);
      if (step>1)
        step++;
      step=step >> 1;
      if (!result) {
        vald=-1;
        step=0;
      } else {
        if (result>0) {
          counter-=step;
          if (counter<0)
            counter=0;
        } else {
          counter+=step;
          if (counter>=filelog_antal)
            counter=filelog_antal-1;
        }
      }
    }
  }
  return(vald);
}
#endif

#ifndef POOL_DEALLOC
void DeleteFileExecutionList()
{
  int counter;
  for (counter=0; counter<filelog_antal; counter++)
    Dealloc(filelog_list[counter]);
  Dealloc(filelog_list);
}
#endif
#endif //LOG_FILE_EXECUTION

static void ScInverseLine(BufStruct *Storage, struct fplArgument *arg)
{
  if (Visible==VISIBLE_ON && BUF(window) && BUF(window)->Visible==VISIBLE_ON) {
    int rad, kolumn, lngd=0;

    CursorXY(Storage, -1, 0);
    kolumn=0;
    rad=BUF(cursor_y);
    if (arg->argc>0) {
      if ((int)arg->argv[0]>0)
        rad=(int)arg->argv[0];
      if (arg->argc>1) {
        lngd=(int)arg->argv[1];
        if (arg->argc>2) {
          if ((int)arg->argv[2]>0)
            kolumn=(int)arg->argv[2]-1;
          else
            kolumn=BUF(cursor_x)-1;
        }
      }
    }
    rad--;
    if (rad<BUF(screen_lines)) {
      rad+=BUF(top_offset)-1;
      if (kolumn<BUF(window)->UpdtVars.EndPos[rad].len) {
        if (!lngd || lngd+kolumn>BUF(window)->UpdtVars.EndPos[rad].len)
          lngd=BUF(window)->UpdtVars.EndPos[rad].len-kolumn;
        SetDrMd(BUF(window)->window_pointer->RPort, COMPLEMENT);
        SetWrMsk(BUF(window)->window_pointer->RPort, 0x01);
        RectFill(BUF(window)->window_pointer->RPort,
                 SystemFont->tf_XSize*kolumn+BUF(left_offset),
                 SystemFont->tf_YSize*rad+BUF(window)->text_start,
                 SystemFont->tf_XSize*(lngd+kolumn)-1+BUF(left_offset),
                 SystemFont->tf_YSize*(rad+1)-1+BUF(window)->text_start);
      }          
    }
  }
}

static void ScPrintLine(BufStruct *Storage, struct fplArgument *arg)
{
  int len;
  BufStruct *Storage2=Storage;
  TextStruct text;

  if (arg->argc==3) {
    Storage2=CheckBufID((BufStruct *)arg->argv[3]);
    if (!Storage2 && FRONTWINDOW)
      Storage2=PrevView(FRONTWINDOW->NextShowBuf);
  }
  if (Storage2 &&
      Storage2->window &&
      (int)arg->argv[1]<=Storage2->screen_lines &&
      (int)arg->argv[1]>=0) {
    text.text=(char *)arg->argv[0];
    text.length=FPL_STRING_LENGTH(arg, 0);
    len=UpdtOneLineC(Storage2, &text, &ScreenBuffer, (int)arg->argv[1],
                     Storage2->curr_topline+(int)arg->argv[1]);
    SystemPrint(Storage2, &ScreenBuffer, len, (int)arg->argv[1]);
  }
}

static int ScPromptInfo(BufStruct *Storage, int argID, 
                                  struct fplArgument *arg)
      /*
       * Prompt for settings of specified buffer or globals.
       *  0 = Default
       * -1 = Current
       * -2 = Local All
       */
{
  int type=cs_LOCAL;
  int ret;
  BufStruct *Storage2=Storage;
  BufStruct *copy_to=NULL;

  if ((int)arg->argv[0]>=0) {
    Storage2=&Default.BufStructDefault;
    if ((int)arg->argv[0]>0)
      Storage2=CheckBufID((BufStruct *)arg->argv[0]);
    if (!Storage2) {
      return(CANT_FIND_BUFFER);
    }
  }
  {
    register int mask=~(MS_GLOBAL|MS_LOCAL), nonmask=MS_READ|MS_HIDDEN;
    register char *windowname=NULL;
    if ((int)arg->argv[0]==0) {
      type=cs_GLOBAL;
      mask|=MS_GLOBAL;
      nonmask|=MS_LOCAL;
    } else if ((int)arg->argv[0]==-1) {
      type=cs_LOCAL;
      mask|=MS_LOCAL;
      nonmask|=MS_GLOBAL;
    } else if ((int)arg->argv[0]==-2) {
      type=cs_LOCAL_ALL;
      mask|=MS_LOCAL;
      nonmask|=MS_GLOBAL;
      Storage2=&Default.BufStructDefault;
    }
    if (arg->argc>1) {
      switch (argID) {
      case SC_PROMPTINFO:
        if (*(char *)arg->argv[1])
          windowname=(char *)arg->argv[1];
        break;
      case SC_SETCOPY:
        copy_to=Storage;
        if ((int)arg->argv[1]>=0) {
          copy_to=&Default.BufStructDefault;
          if ((int)arg->argv[1]>0)
            copy_to=CheckBufID((BufStruct *)arg->argv[1]);
          if (!copy_to) {
            return(CANT_FIND_BUFFER);
          }
        }
      }
      if (arg->argc>2) {
        if ((arg->format[2]==FPL_INTARG && (int)arg->argv[2]) ||
            (arg->format[2]==FPL_STRARG && *(char *)arg->argv[2])) {
          if (arg->format[2]==FPL_STRARG)
            mask=GiveMaskNumber(arg->argv[2], NULL);
          else
            mask=(int)arg->argv[2];
          if (arg->argc>3) {
            if (arg->format[3]==FPL_STRARG)
              nonmask=GiveMaskNumber(arg->argv[3], NULL);
            else
              nonmask=(int)arg->argv[3];
          }
        }
      }
    }
    ret=PromptSetting(Storage2, copy_to, type, windowname, (char **)&arg->argv[4], arg->argc-4, mask, nonmask);
  }
  return(ret);
}

static int ScGetFileList(BufStruct *Storage, struct fplArgument *arg)
{
  int antal, count;
  char *remember=NULL;
  struct Files *list=NULL;
  long dims[]={0, -1};

  antal=GetFileList(Storage, (char *)arg->argv[0], &list, &remember, FALSE);
  if (antal) {
    struct fplRef ref;
    ref.Dimensions=1;
    ref.ArraySize=(long *)&antal;
    fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                     FPLREF_ARRAY_RESIZE, &ref,
                     FPLREF_END);
    count=antal;
    while (count>0) {
      count--;
      dims[0]=count;
      fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                       FPLREF_ARRAY_ITEM, &dims[0],
                       FPLREF_SET_MY_STRING, list->name,
                       FPLREF_END);
      list=list->next;
    }
    OwnFreeRemember(&remember);
  }
  return(antal);
}


static int ScMenuRead(struct fplArgument *arg)
{
  struct menu_position mp={0, 0, 0};
  struct OwnMenu *item;
  int ret=FAILED;

  mp.titlenum=(int)arg->argv[4];
  if (arg->argc>5) {
    mp.itemnum=(int)arg->argv[5];
    if (arg->argc>6)
      mp.subitemnum=(int)arg->argv[6];
  }
  if (menu.ownmenu) {
    item=find_menu(menu.ownmenu, &mp);
    if (item) {
      ret=OK;
      {			// Title
        char *str;
        switch (item->nm_Type) {
        case NM_TITLE:
          str=MENU_TITLE;
          if (mp.itemnum||mp.subitemnum)
            ret=FAILED;
          break;
        case NM_ITEM:
          str=MENU_ITEM;
          if (mp.subitemnum)
            ret=FAILED;
          break;
        case NM_SUB:
          str=MENU_SUBITEM;
          break;
        }
        fplReferenceTags(Anchor, arg->argv[0],
                         FPLREF_SET_MY_STRING, str,
                         FPLREF_END);
      }
		// string
      fplReferenceTags(Anchor, arg->argv[1],
                       FPLREF_SET_MY_STRING, (int)item->nm_Label>NULL?item->nm_Label:MENU_BAR,
                       FPLREF_END);
		// FPL program
      fplReferenceTags(Anchor, arg->argv[2],
                       FPLREF_SET_MY_STRING, item->FPL_program?item->FPL_program:"",
                       FPLREF_END);
		// Key
      {
        char *key=NULL;
        if (item->keypress)
          key=KeySequence(item->keypress, ksKEYSEQ);
        fplReferenceTags(Anchor, arg->argv[3],
                         FPLREF_SET_MY_STRING, key?key:"",
                         FPLREF_END);
        Dealloc(key);
      }
    }
  }
  return(ret);
}


static int ScMenuDelete(struct fplArgument *arg)
{
  struct menu_position mp={0, 0, 0};
  int ret=SYNTAX_ERROR;

  mp.titlenum=(int)arg->argv[0];
  if (arg->argc>1) {
    mp.itemnum=(int)arg->argv[1];
    if (arg->argc>2)
      mp.subitemnum=(int)arg->argv[2];
  }
  if (menu.ownmenu) {
    MenuDelete(&mp);
    ret=OK;
  }
  return(ret);
}

static int ScFaceRead(struct fplArgument *arg)
{
   int fg;
   int bg;
   char *style;
   style = GetFaceStyle(arg->argv[0], &fg, &bg); /* get style */
   if(style) {
     fplReferenceTags(Anchor, arg->argv[1],
                      FPLREF_SET_MY_STRING, style,
                      FPLREF_END);
     fplReferenceTags(Anchor, arg->argv[2],
                      FPLREF_SET_INTEGER, fg,
                      FPLREF_END);
     fplReferenceTags(Anchor, arg->argv[3],
                      FPLREF_SET_INTEGER, bg,
                      FPLREF_END);     
   }
   else
     return NO_MATCH;
   return 0;
}

#define GETLIST_FACT      "FACT"
#define GETLIST_SEARCH    "search"
#define GETLIST_SETTING   "setting"
#define GETLIST_SYMBOLS   "symbols"
#define GETLIST_EXTRA     "extra"
#define GETLIST_FACE      "face"
#define GETLIST_FACESTYLE "facestyle"
#define GETLIST_FACEWORD  "faceword"

static int ScGetList(BufStruct *Storage, struct fplArgument *arg)
{
  int antal=0, count;
  struct fplRef ref;
  long dims[]={0, -1};
  ref.Dimensions=1;

  if ((!strcmp(arg->argv[0], GETLIST_FACESTYLE) ||
       !strcmp(arg->argv[0], GETLIST_FACE)) &&
      arg->format[1]==FPL_STRARRAYVARARG) {
    char **list;
    if(!strcmp(arg->argv[0], GETLIST_FACE))
      list = FaceListFaces(&antal);
    else
      list = FaceListStyles(&antal);
    if(antal) {
      ref.ArraySize=(long *)&antal;
      fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                       FPLREF_ARRAY_RESIZE, &ref,
                       FPLREF_END);
      count=0;
      while (count<antal) {
        dims[0]=count;
        fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                         FPLREF_ARRAY_ITEM, &dims[0],
                         FPLREF_SET_MY_STRING, list[count],
                         FPLREF_END);
        count++;
      }
      Dealloc(list);
    }
    return antal;	/* array entries */
  }
  
  if (!strcmp(arg->argv[0], GETLIST_FACT) &&
      arg->format[1]==FPL_STRARRAYVARARG) {
    register FACT *factcount=DefaultFact;
    while (factcount) {
      factcount=factcount->next;
      antal++;
    }
    ref.ArraySize=(long *)&antal;
    fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                     FPLREF_ARRAY_RESIZE, &ref,
                     FPLREF_END);
    factcount=DefaultFact;
    count=0;
    while (factcount) {
      dims[0]=count;
      fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                       FPLREF_ARRAY_ITEM, &dims[0],
                       FPLREF_SET_MY_STRING, factcount->name,
                       FPLREF_END);
      count++;
      factcount=factcount->next;
    }
    return(antal);	// Returnera antal medlemmar
  }
  if (!strcmp(arg->argv[0], GETLIST_SEARCH) &&
      arg->format[1]==FPL_STRARRAYVARARG) {
    ref.ArraySize=(long *)&SearchHistory.strings;
    fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                     FPLREF_ARRAY_RESIZE, &ref,
                     FPLREF_END);
    for (count=0; count<SearchHistory.strings; count++) {
      dims[0]=count;
      fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                       FPLREF_ARRAY_ITEM, &dims[0],
                       FPLREF_SET_MY_STRING, SearchHistory.text[count],
                       FPLREF_END);
    }
    return(SearchHistory.strings);
  }
  if (!strcmp(arg->argv[0], GETLIST_SETTING) &&
      arg->format[1]==FPL_STRARRAYVARARG) {
    int mask=~(MS_GLOBAL|MS_LOCAL), nonmask=MS_READ|MS_HIDDEN;
    void *extraarray=NULL;
    if (arg->argc>2) {
      if ((arg->format[2]==FPL_INTARG && (int)arg->argv[2]) ||
          (arg->format[2]==FPL_STRARG && *(char *)arg->argv[2])) {
        if (arg->format[2]==FPL_STRARG)
          mask=GiveMaskNumber(arg->argv[2], NULL);
        else
          mask=(int)arg->argv[2];
        if (arg->argc>3) {
          if (arg->format[3]==FPL_STRARG)
            nonmask=GiveMaskNumber(arg->argv[3], NULL);
          else
            nonmask=(int)arg->argv[3];
        }
      }
      if (arg->argc>4 && arg->format[4]==FPL_STRARRAYVARARG) {
        extraarray=arg->argv[4];
      }
    }
    ref.ArraySize=(long *)&antalsets;
    fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                     FPLREF_ARRAY_RESIZE, &ref,
                     FPLREF_END);
    if (extraarray) {
      fplReferenceTags(Anchor, extraarray,
                       FPLREF_ARRAY_RESIZE, &ref,
                       FPLREF_END);
    }
    antal=0;
    for (count=0; count<antalsets; count++) {
      if ((sets[count]->mask&mask) && !(sets[count]->mask&nonmask)) {
        dims[0]=antal;
        fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                         FPLREF_ARRAY_ITEM, &dims[0],
                         FPLREF_SET_MY_STRING, sets[count]->name,
                         FPLREF_END);
        if (extraarray) {
          register char *type=buffer;
          switch (sets[count]->type&15) {
          case ST_BOOL:
            *type++='b';
            break;
          case ST_STRING:
            *type++='s';
            break;
          case ST_CYCLE:
            *type++='c';
            break;
          case ST_NUM:
            *type++='i';
            break;
          }
          if (sets[count]->type&ST_GLOBAL)
            *type++='g';
          if (sets[count]->type&ST_SHARED)
            *type++='l';
          if (!(sets[count]->type&ST_NOSAVE))
            *type++='w';
          if (sets[count]->type&ST_READ)
            *type++='r';
          if (sets[count]->type&ST_HIDDEN)
            *type++='h';
          if (sets[count]->type&ST_USERDEFINED)
            *type++='u';
          *type=0;
          fplReferenceTags(Anchor, extraarray,
                           FPLREF_ARRAY_ITEM, &dims[0],
                           FPLREF_SET_MY_STRING, buffer,
                           FPLREF_END);
        }
        antal++;
      }
    }
    ref.ArraySize=(long *)&antal;
    fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                     FPLREF_ARRAY_RESIZE, &ref,
                     FPLREF_END);
    if (extraarray) {
      fplReferenceTags(Anchor, extraarray,
                       FPLREF_ARRAY_RESIZE, &ref,
                       FPLREF_END);
    }
    return(antal);
  }
  if (!strcmp(arg->argv[0], GETLIST_SYMBOLS) &&
      arg->format[1]==FPL_STRARRAYVARARG) {
    struct fplSymbol *sym;
    if (!fplSendTags(Anchor, FPLSEND_GETSYMBOL_FUNCTIONS, &sym, FPLSEND_DONE))
      antal=sym->num;
    
      ref.ArraySize=(long *)&antal;
      fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                       FPLREF_ARRAY_RESIZE, &ref,
                       FPLREF_END);
      for (count=0; count<antal; count++) {
        dims[0]=count;
        fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                         FPLREF_ARRAY_ITEM, &dims[0],
                         FPLREF_SET_MY_STRING, sym->array[count],
                         FPLREF_END);
      }
    return antal;
  }
  if (!strcmp(arg->argv[0], GETLIST_EXTRA) &&
      arg->format[1]==FPL_INTARRAYVARARG) {
    antal=retval_antal;
    ref.ArraySize=(long *)&antal;
    fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                     FPLREF_ARRAY_RESIZE, &ref,
                     FPLREF_END);
    for (count=0; count<antal; count++) {
      dims[0]=count;
      fplReferenceTags(Anchor, (void *)(arg->argv[1]),
                       FPLREF_ARRAY_ITEM, &dims[0],
                       FPLREF_SET_INTEGER, retval_params[count],
                       FPLREF_END);
    }
    return antal;
  }
  return(0);
}



