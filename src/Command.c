/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
*
* Command.c
*
* This routine handles ALL FrexxEd internal functions.
*
*******/

#ifdef AMIGA
#include <dos/datetime.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <intuition/screens.h>
#include <libraries/dos.h>
#include <libraries/reqtools.h>
#include <limits.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/FPL.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/reqtools.h>
#include <proto/utility.h>
#include <proto/graphics.h>
#undef GetOutlinePen
#include <graphics/gfxmacros.h>
#include <graphics/rastport.h>

#endif

#include <stdio.h>
#include <string.h>

#include "IncludeAll.h"

extern char outputundo[OUTPUT_UNDO_MAX];

extern BlockStruct *BlockBuffer;
extern BufStruct *NewStorageWanted;
extern char CursorOnOff;
extern int Visible;
extern char buffer[];
extern int bufferlen;
extern struct rtFileRequester *FileReq;
extern srch Search;          /* search structure */
extern DefaultStruct Default;

extern int last_command;
extern BlockStruct *YankBuffer;
extern BlockStruct MacroBuffer;
extern int undoantal;
extern int totalalloc;
extern int MacroOn;		/* Macro på (1) eller av (0) */
extern struct FrexxEdFunction fred[];
extern const int nofuncs;
extern int ReturnValue;		// Global return value storage.
extern int UpDtNeeded;
extern struct TextFont *RequestFont;	/* Font used by requsters */
extern int requestfontwidth;		// Kalkulerad bredd på requestfonten.

extern struct Setting **sets;
extern antalsets;
extern int recursive;

extern struct Window *InfoWindow;

extern char *KeyMapRemember;
extern unsigned char *title;

extern char *fpl_executer;
extern FACT *DefaultFact;
extern BOOL clear_all_currents;
extern int ErrNo;		// Global Error value storage.

/*** PRIVATE ***/

static BOOL yankcontinue=FALSE;			/* Shall the yank buffer expand */
static int lastoutput=0;
static int undoOPbsantal=0;
static int undoOPantal=0;
static int undoBSantal=0;
static int undoDLantal=0;

/***************/


int Command(BufStruct *Storage, int command, int Argc, char **Argv, int flags)
{
  static int lastcounter=-1;
  static BOOL lastinsertmode=-1;
  static CursorPosStruct oldbackspace;
  static BufStruct *lastbuffer=NULL;

  int slask, slaskis, len, ret=OK, rek=0;
  int antalcounter=1;
  BOOL storemacro=FALSE;
  BOOL writeprotected=FALSE;
  BOOL hookfunction=TRUE;
#ifdef MULTIPROCESS
  LockStruct buflock;
#endif
  BufStruct *Storage2;
  char *retstring;
  int editrad;
  BOOL execute=TRUE;	// Execute this command.  Can be switch by the hook.
  int localReturnValue;

  if (command&NO_HOOK) {
    hookfunction=FALSE;
    command&=~NO_HOOK;
    if (!Storage) {
      Storage=lastbuffer;
      if (!Storage)
        return OK;
    }
  }
  lastbuffer=Storage;

  editrad=BUF(curr_line);

  recursive++;

  localReturnValue=0;	// Clear the return value.

/* Det första argumentet är alltid en varvräknare för de funktioner som
   är upprepbara */
  if (Argc)
    antalcounter=(int)Argv[0];

  if (undoantal) {
    if ((undoOPantal || undoOPbsantal) && (command!=SC_OUTPUT || FLAG(insert_mode)!=lastinsertmode)) {
      SHS(changes)--;
      UndoAdd(Storage, (char *)undoOPantal, undoNORMAL | undoONLYBACKSPACES, 0);
      if (undoOPbsantal)
        UndoAdd(Storage, outputundo, undoAPPENDBEFORE, undoOPbsantal);
      lastoutput=0;
      undoantal=0;
      undoOPantal=0;
      undoOPbsantal=0;
    } else if (undoBSantal) {
      if (command!=DO_BACKSPACE || antalcounter>1) {
        SHS(changes)--;
        UndoAdd(Storage, &outputundo[OUTPUT_UNDO_MAX-undoBSantal], undoNORMAL|undoONLYTEXT, undoBSantal);
        lastoutput=0;
        undoantal=0;
        undoBSantal=0;
      } else
        SaveCursorPos(Storage, &oldbackspace);
    } else if (undoDLantal && (command!=DO_DELETE || (Argc && (int)Argv[0]>1))) {
      SHS(changes)--;
      UndoAdd(Storage, outputundo, undoNORMAL|undoONLYTEXT, undoDLantal);
      lastoutput=0;
      undoantal=0;
      undoDLantal=0;
    }
  }
//  if (command!=last_command && Default.commandcount!=lastcounter)
  if (!(command&YANK_COMMAND) && command!=DO_EXECUTE_STRING) {
//  if (command!=last_command)
    yankcontinue=FALSE;
  }
  lastcounter=Default.commandcount;

  if (InfoWindow) {
    CloseWindow(InfoWindow);
    InfoWindow=NULL;
  }

  if (MacroOn && recursive==1) {
    if (MacroOn==1 && command!=DO_MACRO_RECORD && command>DO_NOT_MACRO_RECORD_ABOVE_THIS_SETTING)
         /* Se till att vi lagrar funktionen när den är klar */
      storemacro=TRUE;
    MacroOn++;
  }

  /* Check if there is any hook to the function we're about to invoke */
  if (hookfunction && (Default.hook[command&~LOCKINFO_BITS])) {
    if(RunHook(Storage, command, Argc, Argv, 0)) {
      execute=FALSE;
      ret=FUNCTION_CANCEL;
    }
    if (NewStorageWanted) {
      Storage=NewStorageWanted;
      clear_all_currents=TRUE;
    }
  }

  if (execute) {
    if ((command&LOCK_WRITE) && (SHS(writeprotected) || (Default.rwed_sensitive && (SHS(fileprotection)&S_IWRITE)))) {
      ret=WRITE_PROTECTED;
      writeprotected=TRUE;
    }
    {
      switch(command) {
      case DO_ABOUT:
        Sprintf(buffer,
                    RetString(STR_ABOUT_TEXT),
		    "Daniel Stenberg", "Kjell Ericson",
                    Default.ARexxPort?Default.ARexxPort:RetString(STR_NONE),
                    FRONTWINDOW?FRONTWINDOW->FrexxScreenName:"",
                    AvailMem(MEMF_CHIP),
                    AvailMem(MEMF_FAST),
                    totalalloc,
                    Default.diskname,
                    PROGRAM_VER
            );
        Ok_Cancel(Storage, buffer, FREXXED_VER, RetString(STR_OK_GADGET));
        break;
      case DO_ASSIGN_KEY:           // 'AssignKey(function, keystring, depended)'
        {
          int flag=NULL;
          if (Argc<2 || !Argv[1][0])
            flag=kaINPUT;
          if (Argc<1 || !Argv[0][0])
            flag=kaREQUEST;
            
          ret=KeyAssign(Storage, Argc>1?Argv[1]:NULL, Argc>0?Argv[0]:NULL, Argc>2?Argv[2]:NULL, flag);
          localReturnValue=ret;
        }
        break;
      case DO_BACKSPACE:
        if (!writeprotected) {
          if (antalcounter>0) {	// If counter>0, execute the command
            CursorPosStruct cpos;
            int countdown=antalcounter;
            char *string=Malloc(1+antalcounter);
            slaskis=0;
            if (string) {
              do {
                slask=Backspace(Storage);
                if (slask!=-1) {
                  string[--countdown]=(char)slask;
                  slaskis++;
                }
              } while (--antalcounter);
              if (slaskis) {
                localReturnValue=slaskis;
                if (slaskis>1) {
                  UndoAdd(Storage, &string[countdown], undoNORMAL|undoONLYTEXT, slaskis);
                } else {
                  register int shar;
                  if (BUF(using_fact)->xflags[string[countdown]] & factx_CLASS_SPACE)
                    shar=3;
                  else if (BUF(using_fact)->xflags[string[countdown]] & factx_CLASS_WORD)
                    shar=2;
                  else
                    shar=1;
                  if (!undoantal)
                    SHS(changes)++;
                  if ((lastoutput && lastoutput!=shar && lastoutput!=3) || undoBSantal+1>OUTPUT_UNDO_MAX) {
                    SaveCursorPos(Storage, &cpos);
                    RestoreCursorPos(Storage, &oldbackspace, FALSE);
                    UndoAdd(Storage, &outputundo[OUTPUT_UNDO_MAX-undoBSantal], undoNORMAL|undoONLYTEXT, undoBSantal);
                    RestoreCursorPos(Storage, &cpos, FALSE);
                    SHS(changes)--;
                    undoantal=0;
                    undoBSantal=0;
                  }
                  lastoutput=shar;
                  /* OBS Flytta cursorn ett steg åt höger */
                  outputundo[OUTPUT_UNDO_MAX-1-undoBSantal]=string[countdown];
                  undoBSantal++;
                  undoantal++;
                }
              }
              Dealloc(string);
            } else
              ret=OUT_OF_MEM;
          }
        }
        break;
      case DO_BACKSPACE_WORD:
        if (!writeprotected) {
          if (antalcounter>0) {	// If counter>0, execute the command
            do {
              if (slask=WCbwd(Storage)) {
                char *retstring=Malloc(slask);
                if (retstring) {
                  ReturnValue+=slask;
                  Visible=VISIBLE_OFF;
                  ret=BackspaceNr(Storage, retstring, slask);
                  if (ret<OK)
                    break;  // Error, exit
                  UndoAdd(Storage, retstring, undoNORMAL|undoONLYTEXT, slask);
                  Dealloc(retstring);
                }
              }
            } while (--antalcounter);
          }
        }
        break;
      case DO_BLOCK_MARK:                   // 'BlockMark(void)'
      case DO_BLOCK_MARK_COL:               // 'BlockMarkRect(void)'
      case DO_BLOCK_MARK_LINE:              // 'BlockMarkLine(void)'
        {
          int type=(command==DO_BLOCK_MARK_COL)?bl_COLUMNAR:(command==DO_BLOCK_MARK_LINE?bl_LINE:bl_NORMAL);
          SetBlockMark(Storage, TRUE);
          if (Argc>=3 && (int)Argv[3]>0) {
            BUF(blocktype)=type; // 951023, testa
            if (Argc>=5) {
              PlaceBlockMark(Storage, NULL, (int)Argv[1], (int)Argv[2], 
                             (int)Argv[3], (int)Argv[4]);
            } else
              PlaceBlockMark(Storage, NULL, (int)Argv[1], (int)Argv[2], 
                             BUF(cursor_x)+BUF(screen_x),
                             BUF(curr_line));
          } else if (BUF(block_exists)==be_NONE) {
            BUF(block_begin_x)=BUF(block_end_x)=BUF(block_anc_x)=
               BUF(cursor_x)+BUF(screen_x);
            BUF(block_begin_y)=BUF(block_end_y)=BUF(block_anc_y)=
               BUF(curr_line);
      
            BUF(block_last_x)=BUF(block_last_y)=-1;
          }
          if (Argc>2 || Argc==1) {
            if ((int)Argv[0]>=0 && (int)Argv[0]<=2) {
              BUF(block_exists)=(int)Argv[0];
            }
          } else if ((BUF(block_exists)==be_EXIST ||
                      BUF(block_exists)==be_MARKING) &&
                     BUF(blocktype)!=type) {
            BUF(blocktype)=type;
          } else if (BUF(block_exists)==be_EXIST ||
                     (BUF(block_exists)==be_MARKING &&  
                      BUF(block_begin_x)==BUF(block_end_x) &&
                      BUF(block_begin_y)==BUF(block_end_y))) {
            BUF(block_exists)=be_NONE;
          } else if (BUF(block_exists)==be_NONE) {
            BUF(block_exists)=be_MARKING;
          } else
            BUF(block_exists)=be_EXIST;
          BUF(blocktype)=type;
          if (BUF(block_exists))
            SetBlockMark(Storage, FALSE);
          CursorXY(Storage, -1, 0);
        }
        break;
      case DO_BLOCK_MOVE:                   // 'BlockMove(int steps)'
        if (!writeprotected) {
          if (Argc && antalcounter)
            ret=BlockMove(Storage, antalcounter, TRUE);
          else
            ret=BlockMovement(Storage);
        }
        localReturnValue=ret;
        break;
      case DO_BLOCK_CUT:                    // 'BlockCut(char *blockname, ... x1, y1, x2, y2)'
      case DO_BLOCK_CUT_APPEND:
      case DO_BLOCK_DELETE:
        if (!writeprotected) {
      case DO_BLOCK_COPY:
      case DO_BLOCK_COPY_APPEND:
          if (!BUF(block_exists) && Argc<5)
            ret=NO_BLOCK_MARKED;
          else {
            register int special=0;
            BlockStruct *block=NULL;
            if (command!=DO_BLOCK_DELETE && Argc && Argv[0]) {
              block=FindBlock((BufStruct *)Argv[0]);
              if (!block) {
                ret=NO_BLOCK_FOUND;
                break;
              }
            }
            if ((command==DO_BLOCK_COPY_APPEND)||(command==DO_BLOCK_CUT_APPEND))
              special|=bl_APPEND;
            if ((command==DO_BLOCK_CUT)||(command==DO_BLOCK_CUT_APPEND))
              special|=bl_CUT|bl_UNDOADD;
  	  if (command==DO_BLOCK_DELETE)
  	    special|=bl_NOSTORE|bl_CUT|bl_UNDOADD;
  
            if (Argc==5 && (int)Argv[1]>0) {
              BlockCutStruct BlockCut;
              PlaceBlockMark(Storage, &BlockCut,
                             (int)Argv[1], (int)Argv[2], 
                             (int)Argv[3], (int)Argv[4]);
              BlockCut.block_exists=be_EXIST;
              BlockCut.blocktype=special;
              ret=BlockCopy(Storage, block, special, &BlockCut);
            } else
              ret=BlockCopy(Storage, block, special, NULL);
          }
        }
        localReturnValue=ret;
        break;
      case DO_BLOCK_PASTE:                  // 'BlockPaste(char *blockname)'
      case DO_BLOCK_PASTE_COLUMN:           // 'BlockPasteRectangle(char *blockname)'
        if (!writeprotected) {
          register BlockStruct *block=NULL;
          if (Argc) {
            if ((int)Argv[0]==-1)
              block=BlockBuffer;
            else if (Argv[0]) {
              block=FindBlock((BufStruct *)Argv[0]);
              if (!block) {
                ret=NO_BLOCK_FOUND;
                break;
              }
            }
          }
          {
            register int type=bp_NORMAL;
            if (command==DO_BLOCK_PASTE_COLUMN)
              type=bp_COLUMN;
            ret=BlockPaste(Storage, undoNORMAL, block, type);
          }
        }
        localReturnValue=ret;
        break;
      case DO_BLOCK_TO_UPPLOW:              // 'BlockChangeCase(char *blockname)'
      case DO_BLOCK_TO_LOWER:               // 'BlockToLower(char *blockname)'
      case DO_BLOCK_TO_UPPER:               // 'BlockToUpper(char *blockname)'      
        {
          register BlockStruct *block=NULL;
          register int type;
          if (Argc && Argv[0]) {
            block=FindBlock((BufStruct *)Argv[0]);
            if (!block) {
              ret=NO_BLOCK_FOUND;
              break;
            }
          }
          if (command==DO_BLOCK_TO_UPPLOW)
            type=bc_CHANGE_CASE;
          else if (command==DO_BLOCK_TO_LOWER)
            type=bc_LOWER_CASE;
          else
             type=bc_UPPER_CASE;
          BlockCase(Storage, block, type);
        }
        localReturnValue=ret;
        break;
      case DO_BLOCK_SORT:           // 'BlockSort(char *blockname, int column)'
        {
          register BlockStruct *block=NULL;
          if (Argc && Argv[0]) {
            block=FindBlock((BufStruct *)Argv[0]);
            if (!block) {
              ret=NO_BLOCK_FOUND;
              break;
            }
          }
          if (ret>=OK)
            ret=BlockSort(Storage, block, (Argc>1)?(int)Argv[1]:-1,
                          (Argc>2)?(int)Argv[2]:0);
        }
        localReturnValue=ret;
        break;
      case DO_CLEAR:		// Clear(BufferID)
        if (!writeprotected) {
          register BufStruct *Storage2=Storage;
          if (Argc>0)
            Storage2=CheckBufID((BufStruct *)Argv[0]);
          if (Storage2) {
            Clear(Storage2, TRUE);
            UpdateScreen(Storage2);
            InitSlider(Storage2);
          } else
            ret=CANT_FIND_BUFFER;
        }
        localReturnValue=ret;
        break;
      case DO_CLONEWB:
        Storage2=Storage;
        if (Argc) {
          if ((int)Argv[0]==0)
            Storage2=&Default.BufStructDefault;
          if ((int)Argv[0]>0)
            Storage2=CheckBufID((BufStruct *)Argv[0]);
        }
        if (Storage2 && Storage2->window) {
          ret=CloneWB(Storage2->window);
          if (ret>=OK)
            ret=ReDrawSetting(Storage, SE_REOPEN);
        }
        break;
      case DO_COLOR_ADJUST:
        if (Argc==4 && BUF(window))
          SetColors(BUF(window)->screen_pointer, (int)Argv[0], (int)Argv[1], (int)Argv[2], (int)Argv[3]);
        if (!BUF(window) || !BUF(window->window_pointer) || //no window
            (Argc!=4 && PaletteRequest(Storage)==-1)) {  //cancelled
          ret=FUNCTION_CANCEL;
          break;
                    /* Continue !!! */
      case DO_COLOR_RESET:
          if (BUF(window))
            ResetColors(Argc?(int)Argv[0]:-1, BUF(window));
        }
        CopyColors(BUF(window)->screen_pointer, BUF(window));
        localReturnValue=ret;
        break;

	  case DO_COLOR_SAVE:
		if (BUF(window)) {
		  /* FIXME: Call function here to iterate over all the colors of the current
			 screen palette and save them */
		}
      case DO_CURSOR_LEFT:
        antalcounter=-antalcounter;
      case DO_CURSOR_RIGHT:
        localReturnValue=MoveCursor(Storage, antalcounter);
        CursorXY(Storage, BUF(cursor_x), BUF(cursor_y));
        break;
      case DO_CURSOR_UP:
      case DO_CURSOR_DOWN:
        if (antalcounter) {
          if (antalcounter<0) {
            antalcounter=-antalcounter;
            if (command==DO_CURSOR_UP)
              command=DO_CURSOR_DOWN;
            else
              command=DO_CURSOR_UP;
          }
          if (command==DO_CURSOR_UP) {
            do {
              BUF(cursor_y)--;
              BUF(curr_line)--;
              CursorUp(Storage);
            } while (--antalcounter);
          } else {
            do {
              BUF(cursor_y)++;
              BUF(curr_line)++;
              CursorDown(Storage);
            } while (--antalcounter);
          }
          CursorUD(Storage);
          CursorXY(Storage, BUF(cursor_x), BUF(cursor_y));
          localReturnValue=editrad-BUF(curr_line);
        }
        break;
      case DO_DEICONIFY:
        {
          register WindowStruct *specific_window=NULL;
          if (Argc && (long)Argv[0]) {
            if ((long)Argv[0]==-1)
              specific_window=BUF(window);
            else
              specific_window=CheckWinID((WindowStruct *)Argv[0]);
            if (!specific_window) {
              localReturnValue=SYNTAX_ERROR;
              break;
            }
          }
          ret=Deiconify(specific_window);
          localReturnValue=ret;
        }
        break;
      case DO_DELETE:
        if (!writeprotected) {
          BUF(wanted_x)=0;
          if (antalcounter>0) {	// If counter>0, execute the command
            retstring=(char *)Malloc(1+antalcounter);
            if (retstring) {
              while ((!rek || (rek<antalcounter)) && (editrad!=SHS(line) || BUF(string_pos)!=LEN(editrad))) {
                int blockexists=0;
                if (BUF(string_pos)==LEN(editrad) && editrad==SHS(line))
                  break;
                if (BUF(block_exists) &&
                    BUF(block_end_y)==editrad &&
                    BUF(block_end_x)>BUF(cursor_x)+BUF(screen_x)) {
                      blockexists=Col2Byte(Storage, BUF(block_end_x), RAD(editrad), LEN(editrad));
                }
                retstring[rek]=*(RAD(editrad)+BUF(string_pos));
                if (!(BUF(using_fact)->flags[retstring[rek]] & fact_NEWLINE)) {
          
                  movmem(RAD(editrad)+BUF(string_pos)+1, RAD(editrad)+BUF(string_pos), LEN(editrad)-BUF(string_pos)-1);
                  RAD(editrad)=ReallocLine(BUF(shared), editrad, LEN(editrad)-1);
                  LEN(editrad)--;
                  SHS(size)--;
                  PrintLine(Storage, editrad);
                  if (blockexists) {
                      BUF(block_end_x)=Byte2Col(Storage, blockexists-1, RAD(editrad));
                      BUF(block_anc_x)=BUF(block_end_x);
                    SetBlockMark(Storage, FALSE);
                  }
                } else {                    /* delete last in line */
                  int slask;
                  if ((SHS(line)==1) || (editrad==SHS(line)))
                    break;
                  {
                    register char *tempmem;
                    tempmem=ReallocLine(BUF(shared), editrad, LEN(editrad)+LEN(editrad+1)-1);
                    if (!tempmem)
                      break;  // while()
                    RAD(editrad)=tempmem;
                  }
                  memcpy(RAD(editrad)+LEN(editrad)-1, RAD(editrad+1), LEN(editrad+1));
                  if (editrad+2<=SHS(line) && (LFLAGS(editrad+1)&FOLD_BEGIN) &&
                      FOLD(editrad+1)==FOLD(editrad+2))
                    LFLAGS(editrad+2)=LFLAGS(editrad+1);
                  LEN(editrad)+=LEN(editrad+1)-1;
                  DeallocLine(Storage, editrad+1);
                  for(slask=editrad+1; slask<SHS(line); slask++)
                    TEXT(slask)=TEXT(slask+1);
                  SHS(size)--;
                  SHS(line)--;
                  DeleteLine(Storage, editrad, 1);
                  if (CursorOnOff) {
                    CursorXY(Storage, -1, -1);
                  }
                  UpdateLines(Storage, editrad);
                  if (BUF(block_exists)) {
                    SetBlockMark(Storage, FALSE);
                  }
                }
                rek++;
              }
              if (rek) {
                localReturnValue=rek;
                if (rek>1) {
                  UndoAdd(Storage, retstring, undoNORMAL|undoONLYTEXT, rek);
                } else {
                  register int shar;
                  if (BUF(using_fact)->xflags[retstring[0]] & factx_CLASS_SPACE)
                    shar=3;
                  else if (BUF(using_fact)->xflags[retstring[0]] & factx_CLASS_WORD)
                    shar=2;
                  else
                    shar=1;
                  if (!undoDLantal)
                    SHS(changes)++;
                  if ((lastoutput && lastoutput!=shar && shar!=3) || undoDLantal+1>OUTPUT_UNDO_MAX) {
                    UndoAdd(Storage, outputundo, undoNORMAL|undoONLYTEXT, undoDLantal);
                    undoantal=0;
                    undoDLantal=0;
                  }
                  lastoutput=shar;
                  outputundo[undoDLantal]=retstring[0];
                  undoDLantal++;
                  undoantal++;
                }
              }
              if ((Argc?(int)Argv[0]:1)>1 && rek)
                AppendYank(retstring, rek);
              else
                Dealloc(retstring);
            }
          }
        }
        break;
      case DO_DELETE_LINE:          // delete_line(int antal)
        if (!writeprotected) {
          if (antalcounter>0) {	// If counter>0, execute the command
            char *rad;
            int retlength;
            int undovar=undoNORMAL|undoONLYTEXT;
    
            if (CursorOnOff)
              CursorXY(Storage, -1, -1);
    
            do {
              rad=(char *)Deleteline(Storage, &retlength);
              if (retlength) {
                UndoAdd(Storage, rad, undovar, retlength);
                AppendYank(rad, retlength);
                undovar=undoAPPENDBEFORE;
                ReturnValue++;
              } else
                break;	// Om raden var 0 bytes lång, så SKA det ha varit den sista och då breakar vi.
            } while (--antalcounter);
          }
        }
        break;
      case DO_DELETE_TO_EOL:
        if (!writeprotected) {
          char *retstring;
          int retlength;
          retstring=(char *)Del2Eol(Storage, &retlength);
          if (retstring) {
            localReturnValue=retlength;
            UndoAdd(Storage, retstring, undoNORMAL|undoONLYTEXT, retlength);
            AppendYank(retstring, retlength);
            ret=OK;
          } else
            ret=NOTHING_TO_DELETE;
        }
        break;
      case DO_DELETE_WORD:          // delete_word(int antal)
        if (!writeprotected) {
          if (antalcounter>0) {	// If counter>0, execute the command
            while (--antalcounter>=0 && (slask=WCfwd(Storage))) {
              Visible=VISIBLE_OFF;
              ret=Command(Storage, DO_DELETE, 1, (char **)&slask, NULL);
            }
            localReturnValue=(int)Argv[0]-antalcounter;
          }
        }
        break;
      case DO_DELETE_KEY:           // 'DeleteKey(keystring)'
        DeleteKey(Storage, Argc>0?Argv[0]:NULL);
        break;
      case DO_EXECUTE_STRING:
        if (!fpl_executer)
          fpl_executer=EXECUTED_STRING;
        if (Argc) {
          register BufStruct *Storage2=Storage;
          if (Argc>1) {
            BUF(locked)++;
            Storage2=CheckBufID((BufStruct *)Argv[1]);
            ret=CANT_FIND_BUFFER;
          }
          if (Storage2)
            ret=ExecuteFPL(Storage2, Argv[0], FALSE, &slask, NULL);
          if (Argc>1) {
            BUF(locked)--;
            if (!NewStorageWanted || NewStorageWanted==Storage2)
              NewStorageWanted=Storage;
          }
        }
        localReturnValue=slask;
        break;
      case DO_GOTO_LINE:            // goto_line(int rad, int kolumn)
        if (Argc) {
          SetScreen(Storage, (Argc==2)?(int)Argv[1]:0, (int)Argv[0]);
          localReturnValue=FALSE;
          if (BUF(curr_line)==(int)Argv[0]) {
            if (Argc!=2 || BUF(string_pos)==(int)Argv[1])
              localReturnValue=TRUE;		// Position was valid
          }
        } else {
          buffer[0]=0;
          if (rtGetString(buffer,MAX_CHAR_LINE, RetString(STR_GOTO_LINE), NULL, RTGL_Width, requestfontwidth*20, RT_TextAttr, &Default.RequestFontAttr, (BUF(window) && BUF(window)->screen_pointer?RT_Screen:TAG_IGNORE), BUF(window)?BUF(window)->screen_pointer:NULL, TAG_END)) {
            char *pek=buffer;
            slask=fplStrtol(pek, 0, &pek);
            slaskis=fplStrtol(pek, 0, &pek);
            if (slask>SHS(line))
              slask=SHS(line);
            else if (slask<=0)
              slask=1;
            SetScreen(Storage, slaskis, slask);
          } else
            ret=FUNCTION_CANCEL;
        }
        break;
      case DO_INFORMATION:
        if (Argc)
          InfoWindow=FixWindow(Storage, Argv[0]);
        break;
      case DO_ICONIFY:
        {
          register WindowStruct *specific_window=NULL;
          if (Argc && (long)Argv[0]) {
            if ((long)Argv[0]==-1)
              specific_window=BUF(window);
            else
              specific_window=CheckWinID((WindowStruct *)Argv[0]);
            if (!specific_window) {
              localReturnValue=SYNTAX_ERROR;
              break;
            }
          }
          ret=Iconify(specific_window);
          localReturnValue=ret;
        }
        break;
      case DO_ICON_DROP:
        {
          WindowStruct *win;
          win=CheckWinID((WindowStruct *)Argv[1]);
          if (win) {
            Storage=win->ActiveBuffer;
            NewStorageWanted=Storage;
          }
          Argc--;
        }
        /* Fall ner */
      case DO_INSERT_FILE:                  // inser_file(char *filnamn)
        if (!writeprotected) {
          {
            register char *filename=NULL;
            register char *title=RetString(STR_PICK_FILE_INCLUDE);
            register int flag=loadINCLUDE;
            if (Argc) {
              if (Argc>=2 && Argv[1][0])
                title=Argv[1];
              if (Argc<=2)
                filename=Argv[0];
              else {
                filename=Argv[2];
                flag|=loadPROMPTFILE;
              }
            }
            if (!filename || !filename[0]) {
              flag|=loadPROMPTFILE|loadBUFFERPATH;
            }
            ret=Load(Storage, title, flag, filename);
          }
        }
        localReturnValue=ret;
        break;
      case DO_LAST_CHANGE:          // 'goto_change(int)'
        if (Default.oldShared==BUF(shared))
          StoreWorkString();
        {
          int pos=SHS(Undopos);
          if (Argc)
            pos-=(int)Argv[0];
          if (SHS(Undopos)>0 && pos<=SHS(Undopos) && pos>0) {
            int xpos=0, ypos=0, count;
            char *string=SHS(UndoBuf[pos-1])->string+1;
            char sign;
  
            localReturnValue=SHS(Undopos)-pos;
      
            sign=*(string++);
            for (count=sign-1; count!=-1; count--)
              xpos=xpos*256+((UBYTE)(*(string+count)));
        
            string+=sign;
            sign=*(string++);
            for (count=sign-1; count!=-1; count--)
              ypos=ypos*256+((UBYTE)(*(string+count)));
            SetScreen(Storage, xpos, ypos);
          }
        }
        break;
      case DO_LOAD:                         // load(char *filnamn)
        {
          register char *filename=NULL;
          register char *title=RetString(STR_LOAD_FILE);
          register int flag=loadNEWBUFFER;
          if (Argc) {
            if (Argc>=2 && Argv[1][0])
              title=Argv[1];
            if (Argc<=2)
              filename=Argv[0];
            else {
              filename=Argv[2];
              flag|=loadPROMPTFILE;
            }
          }
          if (!filename || !filename[0]) {
            filename=SHS(filnamn);
            flag|=loadPROMPTFILE|loadBUFFERPATH;
          }
          ret=Load(Storage, title, flag, filename);
        }
        if (ret>=OK) {
          if (SHS(shared)!=1) {             /* Rätta alla splittade buffrar */
            register BufStruct *Storage2=Storage;
            while (Storage2->PrevSplitBuf) 		   // Hitta första splitten.
              Storage2=Storage2->PrevSplitBuf;		  //
            do {					 //
              if (Storage2!=Storage) {			//
                TestCursorPos(Storage2);	       //
                UpdateScreen(Storage2);		      //
                InitSlider(Storage2);		     //
                Showstat(Storage2);		    //
              }					   //
              Storage2=Storage2->NextSplitBuf;	  // Uppdaterar alla splittar.
            } while (Storage2);			 //
          }					//
        }
        if (BUF(window)) {
          UpdateScreen(Storage);
          InitSlider(Storage);
        }
        localReturnValue=ret;
        break;
      case DO_MACRO_RECORD:
        if (!MacroOn) {
          ClearBlock(&MacroBuffer);
          MacroOn=1;
          ret=(int)RetString(STR_BEGIN_RECORDING);
          if (BUF(window) && BUF(window)->window_pointer) {
            SetWindowTitles(BUF(window)->window_pointer, (char *)ret, (char *)ret);
          }
        } else {
          WindowStruct *win=FRONTWINDOW;
          while (win) {
            if (win->window_pointer) {
              char *str=win->window_title;
              char *str_s=Default.WindowDefault.window_title;
              if (!*str)
                str=str_s;
#if 1
#warning Remnants of registration keyfile stuff - unsure if it should stay or go
              if (1) { //BUF(reg.reg)) {
                str=title;
                str_s=title;
              }
#endif
              SetWindowTitles(win->window_pointer, str, str_s);
            }
            win=win->next;
          }
          ret=MacroAssign(Storage, &MacroBuffer, Argc>1?Argv[1]:NULL, (Argc>2)?Argv[2]:NULL, Argc?(int)Argv[0]:0);
          MacroOn=0;
        }
        localReturnValue=ret;
        break;
      case DO_MATCH_PAREN:
        if(!MatchChar(Storage))
          ret=NO_MATCH;
        localReturnValue=ret;
        break;
      case DO_MAXIMIZE_BUF:
        if (BUF(window)) {
          register BufStruct *Storage0=BUF(window->NextShowBuf);
          register int temp=Visible;
          if (CursorOnOff)
            CursorXY(Storage, -1, -1);
          Storage2=Storage;
          if (Argc) {
            Storage2=CheckBufID((BufStruct *)Argv[0]);
            if (!Storage2)
              Storage2=Storage;
          }
          Visible=VISIBLE_OFF;
          while (Storage0) {
            if (Storage0!=Storage2) {
              RemoveBuf(Storage0);
              Storage0=BUF(window)?BUF(window->NextShowBuf):FRONTWINDOW->NextShowBuf;
            } else
              Storage0=Storage0->NextShowBuf;
          }
          Visible=temp;
          UpdateWindow(BUF(window));
        }
        ret=OK;
        break;
      case DO_MOUSE_LEFT:
        if (flags&cmflag_POINTER)
          Argc=2;
        if (Argc>=2) {
          register int slaskis=BUF(screen_x)+(int)Argv[0];
          register int slask=FoldMoveLine(Storage, BUF(curr_topline), (int)Argv[1]-1);
          if (slask>SHS(line))
            slask=SHS(line);
          else if (slask<=0)
            slask=1;
          SetScreen(Storage, Col2Byte(Storage, slaskis, RAD(slask), LEN(slask)+(slask==SHS(line))), slask);
        }
        break;
      case SC_OUTPUT:
        if (!writeprotected) {
          String getstring;
          BUF(wanted_x)=0;
          if (Argc>1) {
            len=(int)Argv[1];
            Argc--;
          } else if (!(len=strlen(Argv[0])))
            break;
          if (len==1) {
            register int shar;
            if (BUF(using_fact)->xflags[Argv[0][0]] & factx_CLASS_SPACE)
              shar=3;
            else if (BUF(using_fact)->xflags[Argv[0][0]] & factx_CLASS_WORD)
              shar=2;
            else
              shar=1;
            if ((lastoutput && lastoutput!=shar && shar!=3) || undoOPantal+1>OUTPUT_UNDO_MAX) {
              SHS(changes)--;
              UndoAdd(Storage, (char *)undoOPantal, undoNORMAL | undoONLYBACKSPACES, 0);
              if (undoOPbsantal)  //(!FLAG(insert_mode))
                UndoAdd(Storage, outputundo, undoAPPENDBEFORE, undoOPbsantal);
              undoantal=0;
              undoOPantal=0;
              undoOPbsantal=0;
              SHS(changes)++;
            } else if (!undoOPantal && !undoOPbsantal)
              SHS(changes)++;
            lastoutput=shar;
            slask=OutputText(Storage, (char *)Argv[0], 1, FLAG(insert_mode), &getstring);
            if (!FLAG(insert_mode)) {
              register int slask2=getstring.length;
              while (--slask2>=0) {
                outputundo[undoOPbsantal]=getstring.string[slask2];
                undoOPbsantal++;
              }
            }
            undoOPantal+=slask;
            undoantal+=slask;
            Dealloc(getstring.string);
          } else {
            if (undoOPantal || undoOPbsantal) {
              SHS(changes)--;
              UndoAdd(Storage, (char *)undoOPantal, undoNORMAL | undoONLYBACKSPACES, 0);
              if (!FLAG(insert_mode))
                UndoAdd(Storage, outputundo, undoAPPENDBEFORE, undoOPbsantal);
              undoOPantal=0;
              undoOPbsantal=0;
            } 
            slask=OutputText(Storage, (char *)Argv[0], len, FLAG(insert_mode), &getstring);
            if (slask)
              UndoAdd(Storage, (char *)slask, undoNORMAL | undoONLYBACKSPACES, 0);
            if (!FLAG(insert_mode))
              UndoAdd(Storage, getstring.string, undoAPPENDBEFORE|undoONLYTEXT, getstring.length);
            Dealloc(getstring.string);
            lastoutput=0;
            undoantal=0;
          }
        }
        break;
      case DO_PAGE_UP:              // page_up(int antal)
      case DO_PAGE_DOWN:            // page_down(int antal)
        {
          int update=FALSE;    
          int oldtopline=BUF(curr_topline);
          int pagerader=BUF(screen_lines)-Default.page_overhead;
          if (pagerader<=0)
            pagerader=BUF(screen_lines);
          if (antalcounter) {
            if (antalcounter<0) {
              antalcounter=-antalcounter;
              if (command==DO_PAGE_UP)
                command=DO_PAGE_DOWN;
              else
                command=DO_PAGE_UP;
            }
            if (command==DO_PAGE_UP) {
              do {
                BUF(curr_topline)=FoldMoveLine(Storage, BUF(curr_topline), -pagerader);
                if (BUF(curr_topline)!=oldtopline)
                  update=TRUE;
                BUF(curr_line)=FoldMoveLine(Storage, BUF(curr_line), -pagerader);
                BUF(cursor_y)=FoldFindLine(Storage, BUF(curr_line));
              } while (--antalcounter);
            } else {
              do {
                BUF(curr_topline)=FoldMoveLine(Storage, BUF(curr_topline), pagerader);
                if (FoldFindLine(Storage, SHS(line))!=-1)
                  BUF(curr_topline)=FoldMoveLine(Storage, SHS(line), -(BUF(screen_lines)-1));
                if (BUF(curr_topline)!=oldtopline)
                  update=TRUE;
                BUF(curr_line)=FoldMoveLine(Storage, BUF(curr_line), pagerader);
                BUF(cursor_y)=FoldFindLine(Storage, BUF(curr_line));
              } while (--antalcounter);
            }
            CursorUD(Storage);
            if (update) {
              UpdateScreen(Storage);
              MoveSlider(Storage);
            } else
              CursorXY(Storage, BUF(cursor_x), BUF(cursor_y));
          }
        }
        break;
      case DO_PROMPT:
        ret=Prompt(Storage);
        localReturnValue=ret;
        break;
      case DO_QUIT:                 // 'Kill(int BufferID)'
        {
          register int repeat=1;
          if (Argc) {
            Storage2=CheckBufID((BufStruct *)Argv[0]);
            if (Storage2) {
              if (Storage2!=(BufStruct *)Argv[0])
                repeat=Storage2->shared->shared;
            }
          } else
            Storage2=Storage;
          if (Storage2) {
            while (--repeat>=0) {
              NewStorageWanted=Free(Storage2, TRUE, TRUE);
              if (!NewStorageWanted)
                ret=QUIT_COMMAND;
              localReturnValue=(int)NewStorageWanted;
              if (NewStorageWanted) {
                if (Storage==NewStorageWanted)
                  Storage=NewStorageWanted;
                UpdateAll();
                InitSlider(NewStorageWanted);
                NewStorageWanted->namedisplayed=FALSE;
                ret=NEW_STORAGE;
                if (Storage!=Storage2) {
                  NewStorageWanted=NULL;
                  ret=OK;
                }
              }
              if (repeat)
                Storage2=((SharedStruct *)Argv[0])->Entry;
            }
          } else
            ret=CANT_FIND_BUFFER;
        }
        break;
      case DO_QUIT_ALL:
        ret=QUIT_COMMAND;
        SendReturnMsg(cmd_RET, ret, NULL, NULL, NULL);
        localReturnValue=ret;
        break;
      case DO_REMOVE_BUF:
        if (BUF(window)) {
          if (NewStorageWanted = RemoveBuf(Storage)) {
            UpdateAll();
            ret=NEW_STORAGE;
          }
        }
        localReturnValue=ret;
        break;
      case DO_RESIZE_BUF:		// resize_view(int storlek, int bufferid)
        Storage2=Storage;
        if (Argc>1)
          Storage2=CheckBufID((BufStruct *)Argv[1]);
        if (Storage2 && Storage2->window) {
          slask =(Argc?(int)Argv[0]:Storage2->screen_lines);
          if (Argc || rtGetLong((ULONG *)&slask, RetString(STR_ENTER_DESIRED_VALUE),
                      NULL, RTGL_Width, requestfontwidth*20, RTGL_Min, 0,
                      RTGL_Max, Storage2->window->window_lines,
                      RTGL_ShowDefault, FALSE, TAG_END)) {
            NewStorageWanted=ReSizeBuf(Storage, Storage2, NULL, slask);
            ret=NEW_STORAGE;
            localReturnValue=Storage2->screen_lines;
            if (!Storage2->window)
              localReturnValue=0;
            else
              TestCursorPos(Storage2);
            UpdateAll();
          } else
            ret=FUNCTION_CANCEL;
        }
        break;
      case DO_REPLACE:              // 'replace(int prompt, char *search, char *replace)'
        if (!writeprotected)
          ret=Replace(Storage, Argc, Argv);
        localReturnValue=ret;
        break;
      case DO_RIGHT_WORD:            // right_word(int antal)
      case DO_LEFT_WORD:            // left_word(int antal)
        {
          register int antalsteps=0;
          if (command==DO_LEFT_WORD)
            antalcounter=-antalcounter;
          if (antalcounter>0) {
            while (--antalcounter>=0 && (slask=WCfwd(Storage))) {
              antalsteps+=slask;
              ret=Command(Storage, DO_CURSOR_RIGHT, 1, (char **)&slask, NULL);
            }
          } else if (antalcounter<0) {
            while (++antalcounter<=0 && (slask=WCbwd(Storage))) {
              antalsteps+=slask;
              ret=Command(Storage, DO_CURSOR_LEFT, 1, (char **)&slask, NULL);
            }
          }
          localReturnValue=antalsteps;
        }
        break;
      case DO_SAVE:                         // save(char *filnamn, packmode)
        ret=Save(Storage, saveCLEANUP, NULL, Argc?Argv[0]:NULL, Argc>1?Argv[1]:NULL);
        localReturnValue=ret;
        break;
      case DO_SEARCH:               // 'search(char *string)'
        ret=SearchFor(Storage, Argc, Argv);
        localReturnValue=ret;
        break;
      case DO_SET_SCREEN:
        Storage2=Storage;
        if (Argc) {
          if ((int)Argv[0]==0)
            Storage2=&Default.BufStructDefault;
          if ((int)Argv[0]>0)
            Storage2=CheckBufID((BufStruct *)Argv[0]);
        }
        if (Storage2)
          ret=ScreenModeReq(Storage2);
        localReturnValue=ret;
        break;
      case DO_SET_SAVE:		// (filnamn)
        Storage2=&Default.BufStructDefault;
        if (Argc>1) {
          if ((int)Argv[1]==-1)
            Storage2=Storage;
          if ((int)Argv[1]>0)
            Storage2=CheckBufID((BufStruct *)Argv[1]);
        }
        if (Storage2)
          ret=LoadSetting(Storage2, Argc?(char *)Argv[0]:NULL, FALSE);
        else
          ret=CANT_FIND_BUFFER;
        localReturnValue=ret;
        break;
      case DO_SLIDER:
        ScrollScreen(Storage, 0, scroll_SLIDER);
        CursorUD(Storage);
        break;
      case DO_UNDO:			// 'undo(int antal)'
      case DO_UNDO_RESTART:
        if (!writeprotected) {
          register BOOL notfirst=(command==DO_UNDO);
          if (antalcounter>0) {	// If counter>0, execute the command
            do {
              ret=Undo(Storage, notfirst);
              notfirst=TRUE;
            } while (ret>=OK && --antalcounter);
            MoveSlider(Storage);
          }
          localReturnValue=(int)Argv[0]-antalcounter;
        }
        break;
      case DO_YANK:			// 'yank(int antal)'
        if (!writeprotected) {
          if (antalcounter) {
            do {
              ret=BlockPaste(Storage, undoNORMAL, YankBuffer, bp_NORMAL);
            } while (ret>=OK && --antalcounter);
          }
        }
        localReturnValue=ret;
        break;
      case DO_ICON_CLICK:
        {
          WindowStruct *win=NULL;
          if ((long)Argv[1]) {
            if ((long)Argv[1]==-1)
              win=BUF(window);
            else
              win=CheckWinID((WindowStruct *)Argv[1]);
          }
          if ((long)Argv[1]==0 || (win && win->iconify)) {
            Command(Storage, DO_DEICONIFY, Argc-1, &Argv[1], flags);
            break;
          }
        }
  	/* Fall through */
      case DO_WINDOWFRONT:
        {
          WindowStruct *win=BUF(window);
          if (Argc>1 && (long)Argv[1])
            win=CheckWinID((WindowStruct *)Argv[1]);
          if (win && win->window_pointer) {
            if (command==DO_WINDOWFRONT && Argc && (((int)Argv[0])==-1)) {
              if (!(win->window&FX_WINDOWBIT)) {
                if (win->screen_pointer->NextScreen) {
                  ScreenToFront(win->screen_pointer);
                  if (win->screen_pointer->FirstWindow)
                    ActivateWindow(win->screen_pointer->FirstWindow);
                }
              } else {
                WindowToBack(win->window_pointer);
                if (win->window_pointer->NextWindow)
                  ActivateWindow(win->window_pointer->NextWindow);
              }
            } else {
              if (command==DO_ICON_CLICK || !Argc || !(((int)Argv[0])&2)) {
                ScreenToFront(win->screen_pointer);
                if (win->window!=FX_SCREEN)
                  WindowToFront(win->window_pointer);
              }
              if (command==DO_ICON_CLICK || !Argc || !(((int)Argv[0])&1))
                ActivateWindow(win->window_pointer);
            }
          }
        }
        break;
      case DO_CLOSE_WINDOW: //dummy
        break;
  
#ifdef test
      case DO_SLASK:                /* Amiga 0 */
        {
          char *string="Testing!";
          Command(Storage, DO_INFORMATION, 1, (char **)&string, NULL);
          Argc=0;
        }
        break;
      case DO_SLASK2:               /* Amiga 9 */
#ifdef alloctest
        CheckAllocs(NULL);
#endif
#if 0
        {
          register counter;
          SystemTags("date", TAG_DONE);
          for (counter=0; counter<1000; counter++)
            UpdtLinesC(Storage, buffer, BUF(curr_topline), BUF(screen_lines), 0);
          SystemTags("date", TAG_DONE);
          for (counter=0; counter<1000; counter++)
            UpdtLinesML(&BUF(window)->UpdtVars, Storage, buffer, BUF(curr_topline), BUF(screen_lines), BUF(top_offset));
          SystemTags("date", TAG_DONE);
        }
#endif
        break;
#endif
      default:
        ret=UNKNOWN_COMMAND;
        break;
      }
      if (ret==WRITE_PROTECTED)
        DisplayBeep(FRONTWINDOW && FRONTWINDOW->window_pointer?FRONTWINDOW->window_pointer->WScreen:NULL);

    }
    /* Check if there is any post hook to the function just invoked */
  }

  /* Check if there is any post hook to the function we just invoked */
  if (hookfunction && Default.posthook[command&~LOCKINFO_BITS]) {
    ErrNo=ret;	// The returnvalue is the same as the Error value.
    RunHook(Storage, command, Argc, Argv, HOOK_POSTHOOK);
    if (NewStorageWanted) {
      Storage=NewStorageWanted;
      clear_all_currents=TRUE;
    }
  }

    /* Här ser vi till att lagra ett macro efter funktionen är utförd,
       så att alla funktioner UTOM den sista lagras */
    /* OBSERVERA att man inte kan spela in argument av typ 'optional' */
  if (MacroOn > 1 && recursive==1) {	// Om MacroOn är 1, så ska inte funktionen lagras eftersom den är den första */
    MacroOn--;
    if (storemacro) {
      if (MakeCall(command, Argc, Argv)) {
        String2Block(&MacroBuffer, buffer, strlen(buffer), TRUE);
      }
    }
  }

  last_command=command;
  ReturnValue=localReturnValue;

  if (recursive>0)
    recursive--;
  Search.lastpos|=BUF(curr_line)+BUF(string_pos);
  lastinsertmode=FLAG(insert_mode);
  return(ret);
}

/**************************************************
*
*  AppendYank(char *, int)
*
* Append the string (char *) with length (int) to the YankBuffer.
* This function will Dealloc the input string.
*
* No return.
********/
void __regargs AppendYank(char *string, int stringlen)
{
  if (stringlen) {
    String2Block(YankBuffer, string, stringlen, yankcontinue);
    yankcontinue=TRUE;
    UpdateDupScreen(YankBuffer->Entry);
  }
  Dealloc(string);
}

/* Converts the command to an FPL string, stored in buffer. */
BOOL __regargs MakeCall(cmd command, int Argc, char **Argv)
{
  BOOL result=FALSE;
  register int function;
  for (function=0; function<nofuncs; function++) {
    if (fred[function].ID==command)
      break;
  }
  buffer[0]=0;
  if (function<nofuncs && fred[function].type<10) {
    strcpy(buffer, fred[function].name);
    strcat(buffer, "(");
    if (Argc && fred[function].format) {
      register int count=0;
      char type=fred[function].format[0];
      String string;
      String fnuttstring;
      while (Argc>count) {
        if (count)
          strcat(buffer, ", ");
        if (type=='s' || type=='S') {
          strcat(buffer, "\"");
          string.string=Argv[count];
          if (command==SC_OUTPUT) {
            Argc=1;
            string.length=(int)Argv[1];
          } else
            string.length=strlen(Argv[count]);
          if (!ConvertString(DefaultFact, &string, &fnuttstring))
            break;
          strcat(buffer, fnuttstring.string);
          strcat(buffer, "\"");
          Dealloc(fnuttstring.string);
        } else if (type=='i' || type=='I') {
          Sprintf(buffer+strlen(buffer), "%ld", (int)Argv[count]);
        }
        count++;
        if (fred[function].format[count]!='>')
          type=fred[function].format[count];
      }
    }
    strcat(buffer, ");\n");
    result=TRUE;
  }
  return(result);
}
