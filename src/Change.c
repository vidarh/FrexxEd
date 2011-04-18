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
#include <libraries/FPL.h>
#include <limits.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/FPL.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/reqtools.h>
#include <proto/wb.h>
#endif

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

extern HistoryStruct SearchHistory;
extern char FrexxEdStarted;	/* Är FrexxEd uppstartad */
extern BlockStruct *BlockBuffer;
extern DefaultStruct Default;
extern FACT *DefaultFact;
extern struct Setting **sets;
extern int antalsets;
extern int Visible;
extern char buffer[];
extern struct rtFileRequester *FileReq; /* Buffrad Filerequester. Bra o ha! */
extern BufStruct *NewStorageWanted;
extern int mouse_x;
extern int mouse_y;
extern srch Search;          /* search structure */
extern struct MsgPort *WBMsgPort;
extern struct AppIcon *appicon;
extern char *appiconname;
extern struct DiskObject AppIconDObj;
extern struct DiskObject *externappicon;
extern int MacroOn;
extern BOOL open_copywb;

extern int UpDtNeeded;
extern int redrawneeded;
extern char GlobalEmptyString[];
extern char bitplanes;
extern void *Anchor;
extern struct UserData userdata;

extern struct Task *filehandlertask;

extern unsigned char *title;

extern struct TextFont *SystemFont;
extern int BarHeight;

/* from filehandler: */
extern char filehandlerstop;
extern struct Task *filehandlertask;
extern struct ProcMsg *filehandlerproc;
/* end of filehandler variables */

extern struct SignalSemaphore LockSemaphore;

#define SETD ((char *)(sets[vald]->offset+((int)&Default)))
#define SETL ((char *)(sets[vald]->offset+((int)Storage)))
#define SETS ((char *)(sets[vald]->offset+((int)Storage->shared)))
#define SETW (Storage->window?((int)Storage->window)+(char *)(sets[vald]->offset):NULL)

/************************************************
*
*  PromptSetting
*
**********/
int __regargs PromptSetting(BufStruct *Storage, BufStruct *copy_to, int type,
                            char *windowname, char **argv, int argc, int mask,
                            int nonmask)
{
  int vald;
  int counter=0;
  int count=0;
  int ret=OK;
  WORD update=0;

  {
    ButtonStruct Buttons;
    ButtonArrayStruct *array;
    if (argc<0)
      argc=0;
    array=(ButtonArrayStruct *)Malloc((antalsets+argc)*sizeof(ButtonArrayStruct));

    if (array) {
      memset(array, 0, (antalsets+argc)*sizeof(ButtonArrayStruct));
      {
        while (counter<antalsets) {
          if ((sets[counter]->mask&mask) && !(sets[counter]->mask&nonmask)) {
            array[count].name=sets[counter]->name;
            array[count].settingnumber=counter;
            array[count].typesfull=sets[counter]->type;
            array[count].types=sets[counter]->type&15;
            array[count].cycletext=sets[counter]->cycle;
            array[count].FPLaddition=sets[counter]->FPLaddition;
            {
              array[count].flags=ChangeAsk(Storage, counter, NULL);
              if ((array[count].types & 15)==ST_STRING)
                array[count].flags=(int)Strdup((char *)array[count].flags);
              count++;
            }

          }
          counter++;
        }
      }
      if (argc>0) {
        for (counter=0; counter<argc; counter++) {
//          int len=strlen(argv[counter]);
          int len=INT_MAX;
          {
            register char *pos;
            pos=strchr(argv[counter], '*');
            if (pos)
              len=pos-argv[counter];
          }
          if (argv[counter][0]=='!') {		// Exclude setting
            register int scan;
            BOOL found=FALSE;
            len--;
            for (scan=0; scan<count; scan++) {
              if (!strncmp(&argv[counter][1], array[scan].name, len)) {
                if ((array[scan].types & 15)==ST_STRING)
                  Dealloc((char *)array[scan].flags);
                count--;
                memcpy(&array[scan], &array[scan+1], (count-scan)*sizeof(ButtonArrayStruct));
                found=TRUE;
                if (len<0)
                  break;
                scan--;
              }
            }
            if (found)
              counter--;
          } else {				// Include setting
            vald=GetSettingName(argv[counter]);
            if (vald>=0) {
              register int scan, check;
              for (scan=0; scan<count; scan++) {
                check=strncmp(argv[counter], array[scan].name, len);
                if (check<=0)
                  break;
              }
              if (check || !count) {
                MoveUp(&array[scan+1], &array[scan], (count-scan)*sizeof(ButtonArrayStruct));
                array[scan].name=sets[vald]->name;
                array[scan].settingnumber=vald;
                array[scan].typesfull=sets[vald]->type;
                array[scan].types=sets[vald]->type&15;
                array[scan].cycletext=sets[vald]->cycle;
                array[scan].flags=ChangeAsk(Storage, vald, NULL);
                if ((array[scan].types & 15)==ST_STRING)
                  array[scan].flags=(int)Strdup((char *)array[scan].flags);
                count++;
              }
            }
          }
        }
      }
      InitButtonStruct(&Buttons);
      Buttons.array=array;
      Buttons.OkCancel=TRUE;
      if (windowname)
        Buttons.toptext=windowname;
      Buttons.antal=count;
      if (Buttons.antal) {
  /* Om 'copy_to' är satt så ska ingen requester tas fram, utan alla settings ska
     kopieras till 'copy_to'-buffern. */
        if (copy_to || ButtonPress(Storage, &Buttons)==0) {
          if (copy_to)
            Storage=copy_to;
          for (counter=0; counter<count; counter++) {
            update|=Change(Storage, type, array[counter].settingnumber,
                           array[counter].flags);
          }
          if (type & cs_LOCAL_ALL)
            update|=SE_ALL|((update & SE_UPDATE)?SE_UPDATEALL:0);
          ret=ReDrawSetting(Storage, update);
        } else
          ret=FUNCTION_CANCEL;
        for (counter=0; counter<count; counter++) {
          if ((array[counter].types & 15)==ST_STRING)
            Dealloc((char *)array[counter].flags);
        }
      }
    } else
      ret=OUT_OF_MEM;
    Dealloc(array);
  }
  return(ret);
}
/*************************************************************
 *
 * Change(BufStruct *Storage, int type, int vald, int newvalue)
 *
 * Ändrar vald setting till nya värdet.
 * 'type' har bara effekt om den är satt till 'cs_LOCAL_ALL' och/eller 'cs_STRING'
 * Returnerar updatevärdet från Setting-strukturen.
 ********/
int __regargs Change(BufStruct *Storage, int type, int vald, int newvalue)
{
  int ret=0;
  WORD settingtype;
  BOOL changed=FALSE;
  char *dummy;
  int oldvalue;
  BufStruct *recursive_storage;
  BOOL all_windows=FALSE;

  if (type & (cs_LOCAL_ALL|cs_GLOBAL))
    Storage=&Default.BufStructDefault;

  if (vald>=0 && !(sets[vald]->type & ST_READ)) {
    if (!(type & cs_LOCAL_ALL) &&
        (sets[vald]->type & ST_ALL_WINSCREEN) &&
        BUF(window) && BUF(window)->window==FX_WINSCREEN) {
      /* Om fönstret är WinScreen så ska alla WinScreen-buffrar ändras */
      all_windows=TRUE;
      Storage=&Default.BufStructDefault;
      while (Storage && (!BUF(window) || BUF(window)->window!=FX_WINSCREEN)) {
        Storage=BUF(NextBuf);
      }
    }
    settingtype=sets[vald]->type & 15;
    if (settingtype != ST_STRING) {
      if (type & cs_STRING)
        newvalue=fplStrtol((char *)newvalue, 10, &dummy);
      if (sets[vald]->max || sets[vald]->min) {
        if (newvalue<sets[vald]->min)
          newvalue=sets[vald]->min;
        else if (newvalue>sets[vald]->max)
          newvalue=sets[vald]->max;
      }
    }

    if (sets[vald]->type & ST_GLOBAL) {			/* Global */
      char *mempos=SETD;
      if (sets[vald]->type & ST_USERDEFINED)		/* Userdefined */
        mempos=(char *)&Default.GlobalInfo[sets[vald]->offset];
      oldvalue=ChangeAsk(Storage, vald, NULL);
      if (settingtype==ST_BOOL ||
          settingtype==ST_CYCLE) {
        if ((char)newvalue!=(char)oldvalue) {
          if (newvalue<0 && settingtype==ST_BOOL)
            newvalue=1-oldvalue;
          ret=sets[vald]->update;
          *((char *)mempos)=(char)newvalue;
          changed=TRUE;
        }
      } else if (settingtype==ST_NUM) {
        if (newvalue!=oldvalue) {
          ret=sets[vald]->update;
          *((int *)mempos)=(int)newvalue;
          changed=TRUE;
        }
      } else if (settingtype==ST_STRING) {
        if (strcmp((char *)newvalue, (char *)oldvalue)) {
          ret=sets[vald]->update;
          if (sets[vald]->max)
            stccpy((char *)mempos, (char *)newvalue, sets[vald]->max);
          else {
            Dealloc(*(char **)mempos);
            *((char **)mempos)=Strdup((char *)newvalue);
          }
          changed=TRUE;
        }
      }
      if (changed) {
        switch(sets[vald]->setinstruct) {
        case SEUP_LINECOUNT:
          {
            register BufStruct *Storage=&Default.BufStructDefault;
            do {
              if (FLAG(l_c))
                BUF(l_c_len)=Default.l_c_len_store;
              BUF(fold_len)=Default.fold_margin;
              Storage=BUF(NextBuf);
            } while (Storage);
          }
          break;
        case SEUP_DIRECTORY:
          ChangeDirectory(Default.directory);
          rtChangeReqAttr(FileReq, RTFI_Dir, "", TAG_END);
          break;
        case SEUP_PRIO:
          if (FrexxEdStarted)
            SetTaskPri(FindTask(NULL), newvalue);
          if (filehandlertask)
            SetTaskPri(filehandlertask, newvalue);
          break;
        case SEUP_NEWKEYMAP:
          LoadKeyMap();
          break;
        case SEUP_APPICON:
          if (newvalue) {
            OpenAppIcon();
          } else {
            CloseAppIcon();
          }
          break;
        case SEUP_SEARCH_ALL_FLAG:
          Search.flags=newvalue;
          ConvertSearchFlags();
          break;
        case SEUP_FPLDEBUG:
          fplResetTags(Anchor, FPLTAG_DEBUG, newvalue, FPLTAG_END);
          break;
        case SEUP_SEARCH_FLAG:
          {
            register int searchflag=0;
            if (Default.search_limit)
              searchflag|=LIMIT_WILDCARD;
            if (Default.search_wildcard)
              searchflag|=WILDCARD;
            if (Default.search_word)
              searchflag|=ONLY_WORDS;
            if (Default.search_case)
              searchflag|=CASE_SENSITIVE;
            if (Default.search_block)
              searchflag|=INSIDE_BLOCK;
            if (Default.search_forward)
              searchflag|=FORWARD;
            if (Default.search_prompt)
              searchflag|=PROMPT_REPLACE;
            Search.flags=searchflag;
          }
          break;
        case SEUP_FILEHANDLER:
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
          break;

/*
 ^^^^^^^ ^^^^^^^ ^^^^^^^ ^^^^^^^ ^^^^^^^ ^^^^^^^ ^^^^^^^ ^^^^^^^
  Här Danne kan du lägga in dina switchar med 'SEUP_'-enumar (Setting.h)
  Här ska enbart de globala settingarna finnas.

*/
        }
        if (sets[vald]->FPLnotify && sets[vald]->FPLnotify[0]) {
          register BufStruct *Storage2=Storage;
          if (Storage==&Default.BufStructDefault)
            Storage2=Default.BufStructDefault.NextBuf;
          recursive_storage=userdata.buf;
          Storage2->locked++;
          ExecuteFPL(Storage2, sets[vald]->FPLnotify, FALSE, NULL, NULL);
          Storage2->locked--;
          userdata.buf=recursive_storage;
          NewStorageWanted=NULL;
        }
      }
    } else {						/* Local/Shared */
      char *mempos;
      do {
        if (!(sets[vald]->type & ST_NOSAVE) ||
            Storage!=&Default.BufStructDefault) {
          oldvalue=ChangeAsk(Storage, vald, NULL);
          mempos=SETL;
          if (sets[vald]->type & ST_SHARED)
            mempos=SETS;
          if (sets[vald]->type & ST_WINDOW)
            mempos=SETW;
          if (mempos) {
            changed=FALSE;
            if (sets[vald]->type & ST_CALCULATE) {		/* Kalkulera ut resultatet */
              switch(sets[vald]->offset) {
              case SECALC_FULLNAME:
                if (Storage!=&Default.BufStructDefault) {
                  if (strcmp((char *)newvalue, (char *)oldvalue)) {
                    RenameBuf(Storage, (char *)newvalue);
                    changed=TRUE;
                    ret=sets[vald]->update;
                  }
                }
                break;
              case SECALC_COLUMN:
                if (Storage!=&Default.BufStructDefault) {
                  if (newvalue!=oldvalue) {
                    changed=TRUE;
                    ret=sets[vald]->update;
                    BUF(string_pos)=Col2Byte(Storage, newvalue, RAD(BUF(curr_line)), LEN(BUF(curr_line)));
                    BUF(cursor_x)=Byte2Col(Storage, BUF(string_pos), RAD(BUF(curr_line)))-BUF(screen_x);
                  }
                }
                break;
              case SECALC_LINE:
                if (Storage!=&Default.BufStructDefault) {
                  if (newvalue!=oldvalue) {
                    changed=TRUE;
                    ret=sets[vald]->update;
                    BUF(curr_topline)=newvalue-BUF(cursor_y)+1;
                  }
                }
                break;
              }
            } else {
              if (sets[vald]->type & ST_USERDEFINED)		/* Userdefined */
                mempos=(char *)&Storage->shared->LokalInfo[sets[vald]->offset];
              if (settingtype==ST_BOOL ||
                  settingtype==ST_CYCLE) {
                if ((char)newvalue!=(char)oldvalue) {
                  if (newvalue<0 && settingtype==ST_BOOL)
                    newvalue=1-oldvalue;
                  ret=sets[vald]->update;
                  *((char *)mempos)=(char)newvalue;
                  changed=TRUE;
                }
              } else if (settingtype==ST_NUM) {
                if (newvalue!=oldvalue) {
                  ret=sets[vald]->update;
                  *((int *)mempos)=(int)newvalue;
                  changed=TRUE;
                }
              } else if (settingtype==ST_STRING) {
                if (strcmp((char *)newvalue, (char *)oldvalue)) {
                  ret=sets[vald]->update;
                  if (ret & SE_PROTECTION) {
                    register LONG bits=0x0f;
                    register int count;
                    register char *string=(char *)newvalue;
                    ret-=SE_PROTECTION;
                    for (count=0; count<8; count++) {
                      if (string) {
                        if (!string[count])
                          count=8;
                        else {
                          switch(string[count] | 32) {
                          case 's':
                            bits|=S_ISCRIPT;
                            break;
                          case 'p':
                            bits|=S_IPURE;
                            break;
                          case 'a':
                            bits|=S_IARCHIVE;
                            break;
                          case 'r':
                            bits&=~S_IREAD;
                            break;
                          case 'w':
                            bits&=~S_IWRITE;
                            break;
                          case 'e':
                            bits&=~S_IEXECUTE;
                            break;
                          case 'd':
                            bits&=~S_IDELETE;
                            break;
                          }
                        }
                      }
                    }
                    SHS(fileprotection)=bits;
                    GiveProtection(bits, (char *)&SHS(protection));
                  } else {
                    if (sets[vald]->max)
                      stccpy((char *)mempos, (char *)newvalue, sets[vald]->max);
                    else {
                      Dealloc(*(char **)mempos);
                      *((char **)mempos)=Strdup((char *)newvalue);
                    }
                  }
                  changed=TRUE;
                }
              }
            }
            if (changed) {
              switch(sets[vald]->setinstruct) {
              case SEUP_LINECOUNT:
                BUF(l_c_len)=0;
                if (FLAG(l_c))
                  BUF(l_c_len)=Default.l_c_len_store;
                break;
              case SEUP_CHANGES:
                UndoNoChanged(BUF(shared));
                break;
              case SEUP_STRINGPOS:
                {
                  register int line=BUF(curr_line);
                  if (BUF(string_pos)>LEN(line)-(line!=SHS(line)))
                    BUF(string_pos)=LEN(line)-(line!=SHS(line));
                  TestCursorPos(Storage);
                }
                break;
              case SEUP_CURSOR_X:
                {
                  register int line=BUF(curr_line);
                  
                  BUF(string_pos)=Col2Byte(Storage, BUF(cursor_x)+BUF(screen_x),
                                           RAD(line), LEN(line)+(line==SHS(line)));
                  BUF(cursor_x)=Byte2Col(Storage, BUF(string_pos), RAD(line))-BUF(screen_x);
                  TestCursorPos(Storage);
                }
                break;
              case SEUP_CURSORMOVE:
                TestCursorPos(Storage);
                break;
              case SEUP_PROTECTION_BITS:
                GiveProtection(SHS(fileprotection), (char *)&SHS(protection));
                break;
              case SEUP_RESIZE:
                if (Storage!=&Default.BufStructDefault && BUF(window)) {
                  Visible=VISIBLE_OFF;
                  *((int *)mempos)=(int)oldvalue;
                  if (BUF(NextShowBuf))
                    ReSizeBuf(Storage, BUF(NextShowBuf), NULL, newvalue-BUF(top_offset)+BUF(NextShowBuf)->screen_lines);
                  else
                    ReSizeBuf(Storage, Storage, NULL, newvalue-BUF(top_offset)+BUF(window)->window_lines);
                }
                break;
              case SEUP_FACT_CHANGE:
                BUF(using_fact)=DefaultFact;
                {
                  register FACT *factcount=FindFACT((char *)newvalue);
                  if (factcount)
                    BUF(using_fact)=factcount;
                }
                break;
              case SEUP_FACE_CHANGE:
                SHS(face)=FaceGet((char *)newvalue, FACEGET_CHECK);
                SHS(face_updated_line)=0;
                {
                  BufStruct *count=SHS(Entry);
                  while (count) {
                    count->face_top_updated_line=0;
                    count->face_bottom_updated_line=0;
                    count=count->NextSplitBuf;
                  }
                }
                face_update=TRUE;
                break;
              case SEUP_APPWINDOW:
                {
                  WindowStruct *win=BUF(window);
                  { //bort 960905 while (win) {
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
                }
                break;
              case SEUP_SLIDERCHANGE:
                {
                  BufStruct *StorageCount=BUF(window)->NextShowBuf;
                  while (StorageCount) {
                    RemoveBufGadget(StorageCount);
                    StorageCount=StorageCount->NextShowBuf;
                  }
                }
                break;
              case SEUP_TITLE_CHANGE:
#if 0
#warning Remnants of registration keyfile stuff - unsure if it should stay or go
                if (BUF(reg.reg)) {
#else
				if (0) {
#endif
                  Dealloc(BUF(window->window_title));
                  BUF(window->window_title)=GlobalEmptyString;
                } else {
                  WindowStruct *wincount=FRONTWINDOW;
                  while (wincount) {
                    if (wincount->window_pointer) {
                      char *str=wincount->window_title;
                      char *str2=Default.WindowDefault.window_title;
                      if (!str2)
                        str2=title;
                      if (!*str)
                        str=str2;
                      SetWindowTitles(wincount->window_pointer, str, str2);
                    }
                    wincount=wincount->next;
                  }
                }
                break;
    /*
              case SEUP_xxxx:
                break;
     ^^^^^^^ ^^^^^^^ ^^^^^^^ ^^^^^^^ ^^^^^^^ ^^^^^^^ ^^^^^^^ ^^^^^^^ ^^^^^^^
      Här Danne kan du lägga in dina switchar med 'SEUP_'-enumar (Setting.h)
      Här ska enbart de lokala settingarna finnas.
    
    */
              default:
                break;
              }
    
              if (sets[vald]->FPLnotify && sets[vald]->FPLnotify[0] &&
                  Storage!=&Default.BufStructDefault) {
                recursive_storage=userdata.buf;
                BUF(locked)++;
                ExecuteFPL(Storage, sets[vald]->FPLnotify, FALSE, NULL, NULL);
                BUF(locked)--;
                userdata.buf=recursive_storage;
                NewStorageWanted=NULL;
              }
            }
          }
          if (BUF(window))
            BUF(window)->redrawneeded|=ret;
        }
        Storage=BUF(NextBuf);
        if (all_windows) {
          while (Storage && (!BUF(window) || BUF(window)->window!=FX_WINSCREEN)) {
            Storage=BUF(NextBuf);
          }
        }
      } while (((type & cs_LOCAL_ALL) || all_windows) && Storage);
    }
  }
  return(ret);
}

int __regargs ChangeAsk(BufStruct *Storage, int vald, int *input)
{
  int temp;
  int ret=0;
  WORD settingtype=sets[vald]->type & 15;

  if (sets[vald]->type & ST_CALCULATE) {		/* Kalkulera ut resultatet */
    switch(sets[vald]->offset) {
    case SECALC_COLOUR:
      ret=-1;
      if (input && BUF(window) && BUF(window)->screen_pointer) {
        if (SysBase->LibNode.lib_Version < 39) {
          ret=GetRGB4(BUF(window)->screen_pointer->ViewPort.ColorMap, *input);
        } else {
          ULONG cols[12];
          GetRGB32(BUF(window)->screen_pointer->ViewPort.ColorMap, *input, 3, cols);
          ret=FixRGB32(cols);
        }
      }
      break;
    case SECALC_FULLNAME:
      strmfp(buffer, SHS(path), SHS(filnamn));
      ret=(int)buffer;
      break;
    case SECALC_COLUMN:
      ret=BUF(cursor_x)+BUF(screen_x);
      break;
    case SECALC_LINELEN:
      if (input) {
        temp=*input;
        if (temp>SHS(line))
          temp=SHS(line);
        else if (temp<1)
          temp=1;
      } else
        temp=BUF(curr_line);
      if (Storage->shared->text)
        ret=LEN(temp);
      break;
    case SECALC_BLOCK_NAME:
      ret=(int)BlockBuffer;
      break;
    case SECALC_MOUSE_X:
      ret=(int)mouse_x;
      break;
    case SECALC_MOUSE_Y:
      ret=(int)mouse_y;
      break;
    case SECALC_SEARCHBUFFER:
      ret=0;
      if (input && (temp=*input)>0) {
        if (temp<=SearchHistory.strings)
          ret=(int)SearchHistory.text[temp-1];
      } else
        ret=(int)Search.realbuffer;
      if (!ret)
        ret=(int)GlobalEmptyString;
      break;
    case SECALC_REPLACEBUFFER:
      ret=(int)Search.realrepbuffer;
      break;
    case SECALC_MACRO:
      ret=(int)MacroOn;
      break;
    }
  } else {
    register char *mempos=SETL;			/* Lokal */
    if (sets[vald]->type & ST_SHARED)		/* Shared */
      mempos=SETS;
    if (sets[vald]->type & ST_WINDOW)		/* Window */
      mempos=SETW;
    if (sets[vald]->type & ST_USERDEFINED)	/* Lokal userdefined */
      mempos=(char *)&SHS(LokalInfo[sets[vald]->offset]);
    if (sets[vald]->type & ST_GLOBAL) {		/* Global */
      mempos=SETD;
      if (sets[vald]->type & ST_USERDEFINED)	/* Global userdefined */
        mempos=(char *)&Default.GlobalInfo[sets[vald]->offset];
    }
    if (mempos) {
      if (settingtype==ST_BOOL ||
          settingtype==ST_CYCLE)
        ret=(int)(*((char *)mempos));
      else if (settingtype==ST_NUM)
        ret=(int)(*((int *)mempos));
      else if (settingtype==ST_STRING) {
        if (sets[vald]->max)
          ret=(int)(mempos);
        else
          ret=(int)(*((char **)mempos));
      } 
    }
  }
  if (settingtype==ST_STRING && !ret)
    ret=(int)GlobalEmptyString;

  return(ret);
}

int __regargs ReDrawSetting(BufStruct *Storage, WORD update)
{
  int ret=OK, tempvisible=Visible;
  BufStruct *tempstorage=Storage;
  WindowStruct *win=FRONTWINDOW;

  if (Storage && BUF(window))
    win=BUF(window);

  if (!FrexxEdStarted ||
      !win || Visible!=VISIBLE_ON ||
      (win->Visible!=VISIBLE_ON && !(update&SE_ALL)
      )) {
    redrawneeded|=update;
    win->redrawneeded|=update;
    UpDtNeeded|=UD_REDRAW_SETTING;
    return FAILED;
  }

  if (update & SE_ALL) {
    win=FRONTWINDOW;
    Storage=NULL;
  }
  while (win) {
    if (!Storage)
      Storage=win->NextShowBuf;
    if (update & SE_REOPEN) {
      if (Storage && Storage!=&Default.BufStructDefault) {  // Uppdatera ej om det är defaultbufferten.
        redrawneeded=0;
        win->redrawneeded=0;
        CursorXY(NULL, -1, -1);
        if (BUF(window) && BUF(window)->window_pointer) {
          if (BUF(window)->window==FX_WINSCREEN) {
            WindowStruct *wincount=FRONTWINDOW;
            while (wincount) {
              wincount->flags&=~WINDOW_FLAG_REOPEN;
              if (wincount->window==FX_WINSCREEN &&
                  wincount->window_pointer) {
                CloseMyScreen(wincount);
                wincount->flags|=WINDOW_FLAG_REOPEN;
                wincount->redrawneeded&=~SE_REOPEN;
              }
              wincount=wincount->next;
            }
            wincount=FRONTWINDOW;
            while (wincount) {
              if (wincount->flags&WINDOW_FLAG_REOPEN) {
                wincount->flags&=~WINDOW_FLAG_REOPEN;
                while (!OpenUpScreen(wincount)) {
                  if (!(Ok_Cancel(Storage, RetString(STR_RETRY_OPEN_SCREEN), NULL, NULL)))
                    wincount->window=FX_WINDOW;
                }
              }
              wincount=wincount->next;
            }
          } else {
            CloseMyScreen(BUF(window));
            while (!OpenUpScreen(BUF(window)))
              if (!(Ok_Cancel(Storage, RetString(STR_RETRY_OPEN_SCREEN), NULL, NULL)))
                BUF(window)->window=FX_WINDOW;
            ret=WINDOW_REOPENED;
            SendReturnMsg(cmd_RET, ret, NULL, NULL, NULL);
          }
        }
      }
    } else {
      if (update & SE_FONTCHANGED) {
        GetFont(Storage, Default.SystemFont, 1);
        GetFont(Storage, Default.RequestFont, 2);
        update&=~SE_FONTCHANGED;
      }
      if (update & SE_REINIT) {
        if (win!=&Default.WindowDefault &&
            ((win->real_window_height>win->window_minheight &&
              win->real_window_height>win->window_pointer->MinHeight) ||
             (win->window&FX_WINDOWBIT))) {
          CursorXY(NULL, -1, -2);
          InitWindow(win);
          PrintScreenInit();
          InitColors(win);
  
          ClearWindow(win);
          Visible=VISIBLE_OFF;
          Storage=NULL;
          do {
            register int size;
            if (!Storage) {
              Storage=win->NextShowBuf;
              size=BUF(screen_lines);
              if (win->window_lines-BUF(top_offset)>BUF(screen_lines))
                size=win->window_lines-BUF(top_offset);
            } else
              size=BUF(screen_lines);
            ReSizeBuf(Storage, Storage, NULL, size);
            BUF(namedisplayed)=FALSE;
            Storage=BUF(NextShowBuf);
          } while(Storage);
          Storage=win->NextShowBuf;
          do {
            AddBufGadget(Storage); // 960905
            FixMarg(Storage);
            InitSlider(Storage);
            TestCursorPos(Storage);
            Storage=BUF(NextShowBuf);
          } while(Storage);
          Storage=tempstorage;
          if (Storage && Storage!=&Default.BufStructDefault && !BUF(window) &&
              !NewStorageWanted) {
            NewStorageWanted=win->NextShowBuf;
            ret=NEW_STORAGE;
          }
          Visible=tempvisible;
          UpdateAll();
          ShowAllstat();
          if (win->move_screen>(win->window_col>>1))
            win->move_screen=win->window_col>>1;
          {
            register BufStruct *Storage2=(Storage && BUF(window))?BUF(window->NextShowBuf):win->NextShowBuf;
            while (Storage2) {
              Storage2->lowermarg=Default.BufStructDefault.lowermarg;
              Storage2->uppermarg=Default.BufStructDefault.uppermarg;
              FixMarg(Storage2);
              Storage2=Storage2->NextShowBuf;
            }
          }
          if (win && win->window_pointer) {
            if (win->window&FX_WINDOWBIT)
              RefreshWindowFrame(win->window_pointer);
            else
              RefreshGList(win->window_pointer->FirstGadget, win->window_pointer, NULL, -1);
          }
        }
      } else {
        if (win!=&Default.WindowDefault && (update & SE_RESIZE_WINDOW) &&
            win->window!=FX_SCREEN) {
          if (BUF(window->window_pointer) &&
              Storage!=&Default.BufStructDefault) {  // Uppdatera ej om det är defaultbufferten.
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
        }
        if (win!=&Default.WindowDefault && (update & SE_RETHINK)) {
          InitWindow(win);
          PrintScreenInit();
          InitColors(win);
        } else if (update & SE_UPDATEALL) {
          CopyLineNumberWidth();
          UpdateAll();
          ShowAllstat();
        } else if (update & SE_UPDATE) {
          if (Storage && Storage!=&Default.BufStructDefault) {  // Uppdatera ej om det är defaultbufferten.
            CopyLineNumberWidth();
            TestCursorPos(Storage);
            UpdateDupScreen(Storage);
            UpdtDupStat(Storage);
          }
        }
      }
    }
    win->redrawneeded=0;
    if (update & SE_ALL) {
      win=win->next;
      Storage=NULL;
    } else
      win=NULL;
  }
  return(ret);
}

static int __asm bsearch_cmp_gsn(register __a0 void *name, register __d1 int count)
{
  return(strcmp(sets[count]->name, (char *)name));
}
int __regargs Bsearch(void *name, int antal, int __asm (cmp)(register __a0 void *, register __d1 int))
{
  int vald=-1;
  register int step=(antal+1)>>1;
  register int counter=step;

  step++;
  while (step) {
    register int result;
    result=cmp(name, counter);
    if (step>1)
      step++;
    step=step >> 1;
    if (!result) {
      vald=counter;
      step=0;
    } else {
      if (result>0) {
        if (counter<=0)
          step=0;
        counter-=step;
        if (counter<0)
          counter=0;
      } else {
        if (counter==antal-1)
          step=0;
        counter+=step;
        if (counter>=antal)
          counter=antal-1;
      }
    }
  }
  return(vald);
}

int __regargs GetSettingName(char *name)
{
  return(Bsearch(name, antalsets, &bsearch_cmp_gsn));
}
/****************************************************
 *
 *  LoadSetting(char *filename, int load)
 *
 * Load/save a default file.
 * If Storage==NULL it will handle a global file.
 * If filename==NULL a requester will appear.
 *
 * Return a ret value.
 *******/
int __regargs LoadSetting(BufStruct *Storage, char *filename, int load)
{
  __aligned BPTR fp;
  int count;
  int ret=OK;
  char file[FILESIZE];
  String result;
  String input;

  if (!Storage)	// Storage är vilken buffer som ska användas som default.
    Storage=&Default.BufStructDefault;

  strcpy(file, Default.defaultfile);
  if (!filename || ((int)filename>0 && !filename[0])) {
    if (!PromptFile(NULL, file, RetString(STR_SAVE_SETTING), NULL, NULL, NULL))
      return(FUNCTION_CANCEL);
  } else {
    if ((int)filename>0)
      strcpy(file, filename);
    if (!strchr(file, '/') && !strchr(file, ':')) {
      register struct FileLock *lock;
      LockBuf_release_semaphore(BUF(shared));
      if (!(lock=(struct FileLock *)Lock(file, ACCESS_READ))) {
        strmfp(buffer, "ProgDir:FPL", file);
        strcpy(file, buffer);
        if (!(lock=(struct FileLock *)Lock(file, ACCESS_READ))) {
          ret=CANT_FIND_FILE;
        } else
          UnLock((BPTR)lock);
      } else
        UnLock((BPTR)lock);
      UnLockBuf_obtain_semaphore(BUF(shared));
    }
  }

  if (ret==OK) {
    Dealloc(Default.defaultfile);
			/* Store the new default file name */
    Default.defaultfile=Strdup(file);
    if (load) {
      register BOOL fstarted=FrexxEdStarted;
      FrexxEdStarted=FALSE;
      ExecuteFPL(Storage, file, TRUE, &count, NULL);
      FrexxEdStarted=fstarted;
      if (count==FPL_OK)
        open_copywb=FALSE;

      if (fstarted)
        ReDrawSetting(Storage, SE_REOPEN);
    } else {
      LockBuf_release_semaphore(BUF(shared));
      if (fp=Open(file, MODE_NEWFILE)) {
        FPuts(fp, "Visible(0);\n");
        for (count=0; count<antalsets; count++) {
          if (!(sets[count]->type & ST_NOSAVE)) {
            if (sets[count]->type & ST_USERDEFINED) {
              Sprintf(buffer, "LogSetting(%s,", sets[count]->FPLsave_string);
            } else {
              Sprintf(buffer, "SetInfo(0,\"%s\",", sets[count]->name);
            }
            FPuts(fp, buffer);
            if ((sets[count]->type & 15)==ST_STRING) {
              register char *tempstring=(char *)ChangeAsk(Storage, count, NULL);
              FPuts(fp, "\"");
              input.string=tempstring;
              input.length=strlen(tempstring);
              if (ConvertString(DefaultFact, &input, &result)) {
                if (result.length)
                  FPuts(fp, result.string);
                Dealloc(result.string);
              }
              FPuts(fp, "\");\n");
            } else {
              Sprintf(buffer, "%ld);\n", ChangeAsk(Storage, count, NULL));
              FPuts(fp, buffer);
            } 
          }
        }
        Close(fp);
      } else
        ret=OPEN_ERROR;
      UnLockBuf_obtain_semaphore(BUF(shared));
    }
  }
  return(ret);
}

/********************************************************
 *
 * CopyLineNumberWidth(void);
 *
 * Kopierar linenumberwidth till alla buffrar.
 *******/
void CopyLineNumberWidth(void)
{
  BufStruct *Storage=&Default.BufStructDefault;

  while (Storage) {
    if (BUF(l_c_len))
      BUF(l_c_len)=Default.l_c_len_store;
    BUF(fold_len)=Default.fold_margin;
    FixMarg(Storage);
    Storage=BUF(NextBuf);
  }
}


BOOL __regargs CheckSetting(BufStruct *Storage, char *name)
{
  BOOL ret;
  signed char lastret=-1;
  BOOL inverse;
  register int vald;
  int len;
  char *namepoint;

  while (len=GetWildWord(name, buffer)) {
    ret=FALSE;
    inverse=FALSE;

    namepoint=buffer;
    if (*namepoint=='!') {
      namepoint++;
      inverse=TRUE;
    }
    
    vald=GetSettingName(namepoint);
    if (vald>=0) {
      register int result=ChangeAsk(Storage, vald, NULL);
      if (result) {
        ret=TRUE;
        if ((sets[vald]->type&15)==ST_STRING)
          ret=((char *)result)[0];
      }
    }
    if (inverse)
      ret=ret?FALSE:TRUE;
    if (lastret>=0) {
      switch (name[-1]) {
      case '|':
        ret=(ret|lastret);
        break;
      case '&':
        ret=(ret&lastret);
        break;
      }
    }
    lastret=ret;
    name+=len;
  }
  return(ret);
}


