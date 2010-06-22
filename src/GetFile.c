/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * GetFile.c
 *
 *********/

#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <dos/dos.h>
#include <dos/dosasl.h>
#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/ppbase.h>
#include <libraries/reqtools.h>
#include <xpk/xpk.h>
#include <proto/xpkmaster.h>
#include <inline/xpkmaster_protos.h>
#include <proto/xpksub.h>
#include <inline/xpksub_protos.h> 
#include <proto/dos.h>
#include <proto/fpl.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/powerpacker.h>
#include <inline/powerpacker_protos.h>
#include <proto/reqtools.h>
#include <proto/utility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stat.h"
#include "Buf.h"
#include "Alloc.h"
#include "Block.h"
#include "BufControl.h"
#include "Command.h"
#include "DoSearch.h"
#include "Edit.h"
#include "FACT.h"
#include "Fold.h"
#include "GetFile.h"
#include "Hook.h"
#include "IDCMP.h"
#include "Icon.h"
#include "Init.h"
#include "Limit.h"
#include "Request.h"
#include "Sort.h"
#include "Undo.h"
#include "UpdtScreen.h"
#include "WindowOutput.h"

extern char **Argv;
extern int Argc;
extern struct rtFileRequester *FileReq; /* Buffrad Filerequester. Bra o ha! */
extern char buffer[];
extern DefaultStruct Default;
extern int _OSERR;
extern int Visible;
extern FACT *DefaultFact;
extern char ReturnBuffer[RET_BUF_LEN];
extern BOOL OwnWindow;
extern struct Library *XpkBase;
extern char DebugOpt;
extern int constant;	// Konstant för maxfillängd utan registrering.
extern char GlobalEmptyString[];
extern BufStruct *NewStorageWanted;

int __regargs GetFile(BufStruct *Storage,  char *path, char *filename)
{
  ReadFileStruct RFS;
  int ret=OK;
  int retlength;
  TextStruct *rettext;
  const int slaskbuf=FALSE;

  RFS.filename=filename;
  RFS.path=path;
  RFS.buffer=ReturnBuffer;
  RFS.realpath=NULL;
  ret=ReadFile(Storage, &RFS);
  SHS(fileblock)=RFS.program;
  SHS(fileblocklen)=RFS.length;
  SHS(fileprotection)=RFS.fileprotection;
  if (ret>=OK) {
    if (!slaskbuf) {
      register int newname=TRUE;
      GiveProtection(SHS(fileprotection), (char *)&SHS(protection));
      strcpy(SHS(pack_type), RFS.pack);
      strcpy(SHS(password), RFS.password);
      if (SHS(name_number)>=1) {
        if (Stricmp(SHS(filnamn), filename))
          CountDownFileNumber(BUF(shared));	// Inte lika namn.
        else
          newname=FALSE;
      }
      Dealloc(SHS(path));
      SHS(path)=Strdup(path);
      Dealloc(SHS(filnamn));
      SHS(filnamn)=Strdup(RFS.buffer);
      if (newname) {
        CheckName(Storage, FALSE, TRUE, NULL);
      }
      memcpy(&SHS(date), &RFS.date, sizeof(struct DateStamp));
      memcpy(&SHS(filecomment), &RFS.comment, 80);
    }
    ret=ExtractLines(SHS(fileblock), SHS(fileblocklen), &rettext, &retlength, Storage);
    if (ret<OK) {
      Dealloc(SHS(fileblock));
      SHS(fileblock)=NULL;
      SHS(fileblocklen)=0;
      SHS(size)=0;
      SHS(line)=0;
    } else {
      Dealloc(SHS(text));
      SHS(text)=rettext;
      SHS(line)=retlength;
      SHS(taket)=SHS(line);
    }
    SHS(type)=type_FILE;
  } else if (ret==CANT_FIND_FILE) {
    if (!slaskbuf) {
      Dealloc(SHS(path));
      Dealloc(SHS(filnamn));
      SHS(path)=Strdup(path);
      SHS(filnamn)=Strdup(filename);
    }
    ret=(int)RetString(STR_EDITING_NEW_FILE);
  }
  return(ret);
}

/**********************************************************************
 *
 * Split()
 *
 * Divides the given string in one directory part and one file name part.
 * Return FALSE if the file name is the same.
 ********/
int __regargs Split(SharedStruct *shared, char *name)
{
  char temp[FILESIZE];
  register int check=Stcgfn(temp, name);
  register int ret;

  ret=Stricmp(temp, shared->filnamn);
  if (ret && shared->name_number>=1)	// A new name is given.
    CountDownFileNumber(shared);
  Dealloc(shared->filnamn);
  if (check) {
    shared->filnamn=Strdup(temp);
  } else
    shared->filnamn=Strdup("");
  Dealloc(shared->path);
  if (Stcgfp(temp, name)) {
    shared->path=Strdup(temp);
  } else
    shared->path=Strdup("");
  DateStamp(&shared->date);

  return(ret);
}

int __regargs Load(BufStruct *Storage, char *string, int flag, char *file)
{
  char filename[FILESIZE];
  BufStruct *Storage2;
  int ret=OK;
  int buffers=0;
  char **pointers=NULL;
  int antal=0, counter;
  struct rtFileList *filelist=NULL;
  char *filedir=NULL;
  struct Files *list=NULL;
  char *remember=NULL;
  BufStruct *firststorage=Storage;
  char *Argv[2];		// För GetFile-exception


  FreezeEditor(0);
  filename[0]=0;
  strcpy(buffer, SHS(path));
  if (flag & loadPROMPTFILE) {
    if (file) {
      Stcgfn(filename, file);
      if (!(flag & loadBUFFERPATH)) {
        Stcgfp(buffer, file);
      }
    }
    {
      register int len=strlen(buffer);
      if (len) {
        if (buffer[len-1]=='/')
          buffer[len-1]=0;
      }
    }
    file=NULL;
  }
  if (file && !file[0])
    file=NULL;
  rtChangeReqAttr(FileReq, RTFI_MatchPat, "", TAG_END);
  if (strcmp(FileReq->Dir, buffer))
    rtChangeReqAttr(FileReq, RTFI_Dir, buffer, TAG_END);

  if (!file) {
    struct Screen *screen=NULL;
    if (!FRONTWINDOW || !FRONTWINDOW->window_pointer)
      screen=LockPubScreen(NULL);
    LockBuf_release_semaphore(BUF(shared));
    rtChangeReqAttr(FileReq, RTFI_MatchPat, Default.file_pattern, TAG_END);
    filelist=rtFileRequest(FileReq, filename, string,
                                     !screen?RT_Window:TAG_IGNORE, FRONTWINDOW->window_pointer,
				     !screen?RT_WaitPointer:TAG_IGNORE, OwnWindow,
                                     screen?RT_Screen:TAG_IGNORE, screen,
                                     RTFI_Flags, FREQF_MULTISELECT|FREQF_PATGAD,
				     RT_TextAttr, &Default.RequestFontAttr,
                                     TAG_END);
    if (screen)
      UnlockPubScreen(NULL, screen);  // screen that we opened requester at.
    if (filelist) {
      Dealloc(Default.file_pattern);
      Default.file_pattern=Strdup(FileReq->MatchPat);
    }
    UnLockBuf_obtain_semaphore(BUF(shared));
  }
  if (file || filelist) {
    if (!file)
      strmfp(filename, FileReq->Dir, filelist->Name);
    else
      strcpy(filename, file);
    if (filelist && filelist->Next) {
      char *tempbuffer;
      struct rtFileList *filetemp=filelist;
      filedir=FileReq->Dir;
      antal=0;
      while (filetemp) {
        antal++;
        filetemp=filetemp->Next;
      }
      pointers=(char **)Malloc(sizeof(char *)*antal);
      tempbuffer=Malloc(FILESIZE);
      if (!pointers || !tempbuffer)
        return(OUT_OF_MEM);
      antal=0;
      filetemp=filelist;
      while (filetemp) {
        strmfp(tempbuffer, FileReq->Dir, filetemp->Name);
        if (CheckAndPrompt(Storage, tempbuffer, !(flag&(loadNOQUESTION|loadINCLUDE)), filename)>=OK) {
          pointers[antal]=Strdup(filename);
          antal++;
        }
        filetemp=filetemp->Next;
      }
      Dealloc(tempbuffer);
      if (!antal)
        antal--;
    } else {
      antal=GetFileList(Storage, filename, &list, &remember, !(flag&(loadNOQUESTION|loadINCLUDE)));
      if (antal>0) {
        struct Files *filetemp;
        pointers=(char **)Malloc(sizeof(char *)*antal);
        if (!pointers)
          return(OUT_OF_MEM);
        antal=0;
        filetemp=list;
        while (filetemp) {
          pointers[antal]=filetemp->name;
          antal++;
          filetemp=filetemp->next;
        }
        {
          char **stringarray[2];
          stringarray[0]=pointers;
          stringarray[1]=NULL;
//          ShellSort(stringarray, 1, 1, antal, SORT_NORMAL);
          ShellSort(stringarray, 1, 1, antal, SORT_BACK); //961028
        }
//        strsrt(pointers, antal);
      }
    }
    if (antal<=0) {
      if (antal==0 &&
          !strrchr(filename, '?') && !strrchr(filename, '#') &&
          !strrchr(filename, '|') && !strrchr(filename, '~') &&
          !strrchr(filename, '[') && !strrchr(filename, ']')) {
        if ((flag & loadNEWBUFFER) &&
            !(flag & loadINCLUDE)) {
          register int hookret;
          Argv[0]=buffer;
          Argv[1]=filename+Stcgfp(buffer, filename);
          BUF(locked)++;
          hookret=RunHook(Storage, DO_GETFILE, 2, (char **)&Argv, NULL);
          BUF(locked)--;
          if (hookret) {
            ret=FUNCTION_CANCEL;
          } else {
            Clear(Storage, FALSE);
            Split(BUF(shared), filename);
            CheckName(Storage, FALSE, TRUE, filename);
            ret=(int)RetString(STR_EDITING_NEW_FILE);
            BUF(locked)++;
            RunHook(Storage, DO_GOTFILE, 2, (char **)&Argv, NULL);
            BUF(locked)--;
            NewStorageWanted=NULL;
          }
        }
      } else {
        if (!antal)		// antal är -1 om funktionen cancelerades.
          ret=OPEN_ERROR;
        else
          ret=FUNCTION_CANCEL;
      }
      antal=0;
    } else for (counter=0; counter<antal; counter++) {
      ret=OK;
      if (buffers || (flag & loadINCLUDE)) {
        if (!(Storage2=MakeNewBuf(Storage)))
          ret=OUT_OF_MEM;
      } else {
        Storage2=Storage;
      }

      if (ret>=OK) {
        if ((flag & loadNEWBUFFER)) {
          register int hookret;
          Argv[0]=buffer;
          Argv[1]=pointers[counter]+Stcgfp(buffer, pointers[counter]);
          BUF(locked)++;
          Storage2->locked++;
          hookret=RunHook(Storage2, DO_GETFILE, 2, (char **)&Argv, NULL);
          Storage2->locked--;
          BUF(locked)--;
          NewStorageWanted=NULL;
          if (hookret) {
            ret=FUNCTION_CANCEL;
          }
        }

        if (ret>=OK) {
          if (!buffers)
            Clear(Storage2, FALSE);
          if (!(flag & loadNOINFO)) {
            Sprintf(buffer, RetString(STR_LOADING), Storage2->shared->filnamn);
            Status(firststorage, buffer, 0);
          }
          {
            char *filnamn, *pathnamn;
            filnamn=pointers[counter]+Stcgfp(buffer, pointers[counter]);
            pathnamn=Strdup(buffer);
            ret=GetFile(Storage2, pathnamn, filnamn);
            if (flag & loadINCLUDE) {
              if (ret>=OK) {
                ret=OK;
                ret=BlockPaste(Storage, undoNORMAL, Storage2->shared, bp_NORMAL);
                Free(Storage2, TRUE, FALSE);
              }
            } else {
              Argv[0]=pathnamn;
              Argv[1]=filnamn;
              BUF(locked)++;
              Storage2->locked++;
              RunHook(Storage2, DO_GOTFILE, 2, (char **)&Argv, NULL);
              Storage2->locked--;
              BUF(locked)--;
              NewStorageWanted=NULL;
            }
            Dealloc(pathnamn);
          }
        }
        if (ret>=OK) {
          ret=OK;
          buffers++;
        } else {
          if ((flag & loadNEWBUFFER)) {
            if (buffers)
              Free(Storage2, FALSE, FALSE);
          }
        }
      }
    }
    if (pointers && filelist && filelist->Next) {
      while (--antal>=0)
        Dealloc(pointers[antal]);
    }
    Dealloc(pointers);
  } else 
    ret=FUNCTION_CANCEL;
  ActivateEditor();
  rtFreeFileList(filelist);
  OwnFreeRemember(&remember);
  if (ret==OK) {
    Sprintf(ReturnBuffer, RetString(STR_LOADING_READY), SHS(filnamn), SHS(size));
    ret=(int)ReturnBuffer;
  }
  return(ret);
}

int __regargs Save(BufStruct *Storage, int flag, char *string, char *file, char *packmode)
{
  struct FileHandle *filewrite;
  String fileblock={NULL, 0};
  __aligned struct FileInfoBlock fileinfo;
  BPTR lock;
  int counter, ret=OK;
  int slask;
  LONG error=0;
  char *allocation;
  char *text;

  char *temp1, *temp2, *temp3, *temp4;
  char *path, *name;
  char *filename;
  char *falsefilename;

  allocation=Malloc((FILESIZE+40)*5);
  if (!allocation)
    return(OUT_OF_MEM);
  text=allocation;
  temp1=allocation+(FILESIZE+40)*1;
  temp2=allocation+(FILESIZE+40)*2;
  temp3=allocation+(FILESIZE+40)*3;
  temp4=allocation+(FILESIZE+40)*4;

  path=temp1;
  name=temp2;
  filename=temp3;
  falsefilename=temp4;

  if (!packmode)
    packmode=SHS(pack_type);

  if ((!file || !file[0]) && !SHS(filnamn[0])) { // Can't save a file with the default name.
    temp3[0]=0;
    if (!PromptFile(Storage, temp3, RetString(STR_SAVE_NONAME), NULL, FREQF_SAVE, NULL)) {
      Dealloc(allocation);
      return(FUNCTION_CANCEL);
    }
    file=temp3;
    Split(BUF(shared), file);
    CheckName(Storage, FALSE, TRUE, NULL);
  }

  if (!file || !file[0]) {
    path=SHS(path);
    name=SHS(filnamn);
  } else {
    Stcgfp(path, file);
    Stcgfn(name, file);
  }
  strmfp(text, path, name);
  if (Default.safesave) {
    strmfp(falsefilename, path, "FredTemp.");
    strcat(falsefilename, name);
  } else
    strcpy(falsefilename, text);

		/* Compare date */
  LockBuf_release_semaphore(BUF(shared));
  if (ret>=OK && !file && SHS(date.ds_Days)!=-1 &&
      (lock=Lock((UBYTE *)text, ACCESS_READ))) {
    register int fi=FALSE;
    if (Examine(lock, &fileinfo)) {
      fi=TRUE;
      if (CompareDates(&SHS(date), &fileinfo.fib_Date)>0) {
        BUF(locked)++;
        if (RunHook(Storage, DO_FILECHANGED, 0, NULL, NULL)) {
          ret=FUNCTION_CANCEL;
        }
        BUF(locked)--;
      }
    }
    UnLock(lock);
    if (ret>=OK) {
      if (!Default.rwed_sensitive) {
	if (fi && (fileinfo.fib_Protection & S_IWRITE)) {
          SetProtection(text, 0);
        }
      } else {
	if ((fi && (fileinfo.fib_Protection & S_IWRITE)) ||
	    (SHS(fileprotection)&S_IWRITE)) {
	  ret=WRITE_PROTECTED;
	}
	if (fi && (fileinfo.fib_Protection & S_IDELETE))
	  ret=DELETE_PROTECTED;
      }
    }
  }

  if (ret>=OK) {
    BOOL resave=TRUE;		// Save the file normal
    if (Default.safesave) {
      SetProtection(falsefilename, 0);
      DeleteFile(falsefilename);
    }

    if (packmode[0]) {
      BOOL fine=FALSE;
      if (FoldCompactBuf(BUF(shared), &fileblock)==OK) {
        if (!Stricmp("PP20", packmode)) {
          if (PPBase) {
            APTR crunchinfo;
            ULONG crunchlen;
            char *filestring=Malloc(fileblock.length);
  
            if (filestring) {
              memcpy(filestring, fileblock.string, fileblock.length);
              Status(Storage, RetString(STR_CRUNCHING_BUFFER), 0);
              if (crunchinfo=ppAllocCrunchInfo(CRUN_FAST, SPEEDUP_BUFFSMALL, NULL, NULL)) {
                crunchlen=ppCrunchBuffer(crunchinfo, filestring, fileblock.length);
        
                if (crunchlen && crunchlen!=PP_BUFFEROVERFLOW) {
                  filewrite=(struct FileHandle *)Open(falsefilename, MODE_NEWFILE);
                  if (filewrite) {
                    if (!ppWriteDataHeader((BPTR)filewrite, CRUN_FAST, FALSE, NULL))
                      ret=OPEN_ERROR;
                    else {
                      fine=TRUE;
                      resave=FALSE;
                      Sprintf(filename, RetString(STR_SAVING_TEXT), text);
                      if (Write((BPTR)filewrite, (APTR)filestring, crunchlen)<0)   /* Spara filen */
                        ret=OPEN_ERROR;
                    }
                    Close((BPTR)filewrite);
                  } else
                    ret=OPEN_ERROR;
                }

                Dealloc(filestring);
                if (crunchinfo)
                  ppFreeCrunchInfo(crunchinfo);
              }
            }
          }
        } else {
          if (XpkBase) {
          /* xpk packing selected! */
            Sprintf(filename, RetString(STR_XPKSAVING_TEXT), text);
            Status(Storage, filename, 0);
            fine=TRUE;
            ret=XpkPackTags(XPK_InBuf, fileblock.string,
                            XPK_InLen, fileblock.length,
                            XPK_PackMethod, packmode,
                            XPK_Password, SHS(password),
                            XPK_OutName, falsefilename,
                            XPK_GetError, buffer,
                            TAG_DONE);
            if(ret) {
              ret=OK;
              strcat(buffer, RetString(STR_SAVE_IT_NORMAL_ADD));
              if (!Ok_Cancel(Storage, buffer, RetString(STR_XPK_ERROR), NULL)) {
                ret=FUNCTION_CANCEL;
                resave=FALSE;
              }
            } else
              resave=FALSE;
          }
        }
      }
      if (!fine) {
        if (!Ok_Cancel(Storage, RetString(STR_CANT_CRUNCH_BUFFER), NULL, NULL)) {
          ret=FUNCTION_CANCEL;
          resave=FALSE;
        }
      }

    }
    if (ret>=OK && resave) {	// Save it normally (pack failed)
      filewrite=(struct FileHandle *)Open(falsefilename, MODE_NEWFILE);
      if (filewrite) {
        Sprintf(filename, RetString(STR_SAVING_TEXT), text);
        Status(Storage, filename, 0);
//inte bra vid autosave        FreezeEditor(0);
        if (!(flag & saveCLEANUP) || FoldCompactBuf(BUF(shared), &fileblock)!=OK) {
          for (counter=1; counter<=SHS(line); counter++) {
            if (Write((BPTR)filewrite, (APTR)RAD(counter), LEN(counter))<0) {
              ret=OPEN_ERROR;
              break;
            }
          }
        } else {
          if (Write((BPTR)filewrite, (APTR)fileblock.string, fileblock.length)<0)
            ret=OPEN_ERROR;
        }
        Close((BPTR)filewrite);
//inte bra vid autosave (hör ihop med Freeze'n:        ActivateEditor();
      } else
        ret=OPEN_ERROR;
    }
    if (ret>=OK) {
      if (lock=Lock((UBYTE *)falsefilename, ACCESS_READ)) {
        SetProtection(falsefilename, SHS(fileprotection));
        SetComment(falsefilename, (char *)&SHS(filecomment));
        UndoNoChanged(BUF(shared));
        if (Default.safesave) {
          if (!(slask=DeleteFile(text)))
            error=IoErr();
          if (slask || error==205) {
            error=0;
            if (!Rename(falsefilename, text)) {
              Dealloc(allocation);
              return(-(int)RetString(STR_SAVING_TROUBLE));
            }
          } else
            ret=OPEN_ERROR;
        }
        if (ret>=OK) {
          if (Examine(lock, &fileinfo))
            memcpy(&SHS(date), &fileinfo.fib_Date, sizeof(struct DateStamp));
        } else
          SHS(date.ds_Days)=-1;
        UnLock(lock);
      }
      if(ret>=OK && SHS(SaveInfo)) {
        BPTR parent = TRUE;
        BPTR lock;
        STRPTR ptr;
        if(SHS(SaveInfo) == 2) {
          lock = Lock(text, ACCESS_READ); /* lock file (again) */
          if(lock) {
            parent = ParentDir(lock);
            NameFromLock(parent, &buffer[0], FILESIZE);
            ptr = PathPart(&buffer[0]);
            *ptr = 0; /* end the string there! */

            NameFromLock(lock, &buffer[FILESIZE], FILESIZE);
            ptr = PathPart(&buffer[FILESIZE]);
            *ptr = 0; /* end the string there! */
            if(!Stricmp(&buffer[0], &buffer[FILESIZE])) {
              /*
               * We're in a root directory!
               */
              parent=TRUE;
            } else {
              if(!CheckIcon(&buffer[FILESIZE]))
                /* parent had no icon! */
                parent = FALSE;
            }
            UnLock(lock);
          }
        }
        if(parent)
          MakeIcon(text);
      }
    } else
      DeleteFile(falsefilename);

    if  (error || (ret<OK && ret!=FUNCTION_CANCEL)) {
      if (!error)
        error=IoErr();
      if (error) {
        Fault(error, RetString(STR_WRITE_ERROR), ReturnBuffer, 120);
        ret=-(int)ReturnBuffer;
      } else {
        if (Default.safesave)
          DeleteFile(falsefilename);
      }
    }
    if (ret>=OK) {
      SHS(changes)=0;
      if (SHS(shared)!=1)
        UpdtDupStat(Storage);
      Sprintf(ReturnBuffer, RetString(STR_WROTE), text);
      ret=(int)ReturnBuffer;
    }
  }
  UnLockBuf_obtain_semaphore(BUF(shared));
  Dealloc(allocation);
  if (fileblock.string!=SHS(fileblock))
    Dealloc(fileblock.string);
  else
    CompactBuf(BUF(shared), NULL);
  return(ret);
}

int __regargs SaveString(BufStruct *Storage, char *filename, char *string, int len)
{
  struct FileHandle *filewrite;
  int ret=OK;
  LONG error;

  LockBuf_release_semaphore(BUF(shared));
  if (filewrite = (struct FileHandle *)Open(filename, MODE_NEWFILE)) {
    if (Write((BPTR)filewrite, (APTR)string, len)<0)
      ret=OPEN_ERROR;
    Close((BPTR)filewrite);
  } else
    ret=OPEN_ERROR;
  if  (ret<OK) {
    error=IoErr();
    if (error) {
      Fault(error, RetString(STR_WRITE_ERROR), ReturnBuffer, 120);
      ret=-(int)ReturnBuffer;
    }
  }
  UnLockBuf_obtain_semaphore(BUF(shared));
  return(ret);
}

/*********************************************************
 *
 *  CheckName(BufStruct *Storage, int requester)
 *
 *  Testar om det finns en buffer i minnet som har samma namn.
 *  Om 'requester'=TRUE så utlöses 'SameName'-hooken, svaras
 *  det OK så sätts bufferflaggor.
 *  Returnerar ett ret-värde.
 *********/
int __regargs CheckName(BufStruct *Storage, int requester, int update, char *name)
{
  SharedStruct *Shared=NULL;
  SharedStruct *Sharedfoundlast=NULL;
  int maxvalue=0;
  int ret=OK;
  char *path;

  if (Storage)
    Shared=BUF(shared);
  if (!name) {
    name=SHS(filnamn);
    path=SHS(path);
  } else {
    Stcgfn(buffer+FILESIZE*2, name);
    Stcgfp(buffer+FILESIZE, name);
    name=buffer+FILESIZE*2;
    path=buffer+FILESIZE;
  }

//  if(name[0])  {
  if (1) {
    {
      register SharedStruct *Shared2=Default.SharedDefault.Next;
      while (Shared2) {
        if (Shared2!=Shared) {
          if (!Stricmp(Shared2->filnamn, name)) {
            if (Shared2->name_number>maxvalue)
              maxvalue=Shared2->name_number;
            if (!Stricmp(Shared2->path, path))
              Sharedfoundlast=Shared2;
            if (!Sharedfoundlast)
              Sharedfoundlast=Shared2;
          }
        }
        Shared2=Shared2->Next;
      }
    }
  
    if (Sharedfoundlast) {
      if (requester) {
        char *Argv[2];
        strmfp(buffer, path, name);
        Argv[0]=buffer;
        Argv[1]=(char *)Sharedfoundlast;
        BUF(locked)++;
        if (RunHook(Storage, DO_SAMENAME, 2, Argv, NULL)) {
          ret=FUNCTION_CANCEL;
        }
        BUF(locked)--;
      }
      if (ret>=OK && update) {
        if (maxvalue==0)
          Sharedfoundlast->name_number=maxvalue=1;
        Shared->name_number=maxvalue+1;
      }
    }
  } else
    Shared->name_number=0;
  return(ret);
}

/*****************************************************
 *
 * ExtractLines(char *text, int length, char **rettext, int **retlength, int *lines)
 *
 * Extract the lines from a textblock.
 *
 * Input: Return a array of (char **) in (char ***) which is up to you to Dealloc.
 *
 * Return: A ret value.
 **********/
int ExtractLines(char *text, int length, TextStruct **rettext, int *lines, BufStruct *can_be_folded)
{
  register char *mem=text;
  register len=length;
  register char *flags=DefaultFact->flags;
  register int rows=0;
  TextStruct *slasktext;
  int line=0;
  int size=0;
  int fold_no=0, fold_flags=0;
  BOOL fold_found;
  char *next_foldstart=text+length+1;
  char *next_foldend=text+length+1;
  String search_beg_text, search_beg_string;
  String search_end_text, search_end_string;
  int fold_start_len=0, fold_end_len=0, copy_len, see_level=0;
  char *dummy;

  if (can_be_folded && Default.fold_save) {
    fold_found=FALSE;
    search_beg_text.string=text;
    search_end_text.string=text;
    search_beg_text.length=length;
    search_end_text.length=length;
    search_beg_string.string=FOLD_STRING_BEGIN;
    search_beg_string.length=sizeof(FOLD_STRING_BEGIN)-1;
    next_foldstart=FindTextInString(&search_beg_text, &search_beg_string);
    if (next_foldstart) {
      search_end_string.string=FOLD_STRING_END;
      search_end_string.length=sizeof(FOLD_STRING_END)-1;
      do {
        next_foldend=FindTextInString(&search_end_text, &search_end_string);
        if (!next_foldend)
          next_foldend=text+length+1;
      } while (next_foldend<next_foldstart);
    } else {
      next_foldstart=next_foldend;
      next_foldend=text+length+1;
    }
  }

  if (DefaultFact->newlinecheck) {
    if (DefaultFact->newlinecheck==1) {
      register char newline=DefaultFact->newlinechar;
      if (len && (*mem)==newline)
        rows++;
      while (--len>0) {
        if (*(++mem)==newline)
          rows++;
      }
    } else {
      mem--;
      while (len>0) {
        if (flags[*(++mem)] & fact_NEWLINE)
          rows++;
        len--;
      }
    }
  } else
    rows=0;
  slasktext=(TextStruct *)Malloc(sizeof(TextStruct)*(rows+2));
  if (!slasktext) {
    return(OUT_OF_MEM);
  }
  memset(slasktext, 0, sizeof(TextStruct)*(rows+2));
  if (!rows) {
    line++;
    slasktext[line].text = text;
    slasktext[line].length = length;
    size=length;
  } else {
    mem=text-1;
    rows++;
    while (--rows>=0) {
      line++;
      slasktext[line].text = mem;
      if (rows)
        while (!(flags[*(++mem)] & fact_NEWLINE));
      else
        mem=text+length-1;
      size+=slasktext[line].length=mem-slasktext[line].text;
      slasktext[line].text++;
      if (can_be_folded) {
        if (fold_flags&FOLD_BEGIN) {
          fold_flags=0;
          if (fold_no>see_level)
            fold_flags=FOLD_HIDDEN;
        }
        while (mem>=next_foldend) {
          register int maxlen=16;
          register char *temppek=next_foldend+sizeof(FOLD_STRING_END)-1;
          while (--maxlen>=0 && *(temppek++)!='.');
          temppek+=fold_end_len;
          if (temppek-1<=mem) { // -1 för att acceptera sista raden.
            copy_len=mem-(next_foldend-fold_start_len);
            memcpy(next_foldend-fold_start_len, temppek, copy_len);
            copy_len=temppek-(next_foldend-fold_start_len);
            slasktext[line].length-=copy_len;
            size-=copy_len;
            if (mem>=next_foldstart)
              next_foldstart-=copy_len;
            if (fold_no) {
              fold_no--;
              if (!fold_no) {
                fold_flags=0;
                see_level=0;
                fold_flags&=~FOLD_HIDDEN;
              } else if (fold_no<=see_level) {
                if (fold_no<see_level)
                  see_level--;
                fold_flags&=~FOLD_HIDDEN;
              }
            }
            search_end_text.length+=search_end_text.string-slasktext[line].text;
            search_end_text.string-=search_end_text.string-slasktext[line].text;
          }
          next_foldend=FindTextInString(&search_end_text, &search_end_string);
          if (!next_foldend)
            next_foldend=text+length+1;
        }
        if (mem>=next_foldstart) {
          register char *temppek=next_foldstart+sizeof(FOLD_STRING_BEGIN)-1;
          if (!fold_found) {
            {
              register int maxlen=8;
              while (--maxlen>0 && *(temppek++)!='.');
              if (temppek<mem) {
                fold_start_len=fplStrtol(temppek, 10, &dummy);
                Dealloc(can_be_folded->shared->comment_begin);
                can_be_folded->shared->comment_begin=Memdup(next_foldstart-fold_start_len, fold_start_len);
                maxlen=8;
                while (--maxlen>0 && *(temppek++)!='.');
                if (temppek<mem) {
                  fold_end_len=fplStrtol(temppek, 10, &dummy);
                  maxlen=8;
                  while (--maxlen>0 && *(temppek++)!='.');
                  if (temppek<=mem) {
                    Dealloc(can_be_folded->shared->comment_end);
                    can_be_folded->shared->comment_end=Memdup(temppek, fold_end_len);
                    fold_found=TRUE;
                  }
                }
              }
            }
          } else {
            register int maxlen=16;
            while (--maxlen>=0 && *(temppek++)!='.');
            while (--maxlen>=0 && *(temppek++)!='.');
            while (--maxlen>=0 && *(temppek++)!='.');
          }
          if (*(temppek-2)==',') {
            see_level++;
            fold_flags&=~FOLD_HIDDEN;
          }
          temppek+=fold_end_len;
          if (temppek<=mem) {
            copy_len=mem-(next_foldstart-fold_start_len);
            memcpy(next_foldstart-fold_start_len, temppek, copy_len);
            copy_len=temppek-(next_foldstart-fold_start_len);
            slasktext[line].length-=copy_len;
            size-=copy_len;
            fold_no++;
            fold_flags|=FOLD_BEGIN;
            search_beg_text.length+=search_end_text.string-slasktext[line].text;
            search_beg_text.string-=search_end_text.string-slasktext[line].text;
          }
          do {
            next_foldstart=FindTextInString(&search_beg_text, &search_beg_string);
            if (!next_foldstart)
              next_foldstart=text+length+1;
          } while (next_foldstart<mem);
        }
      }
      slasktext[line].fold_no=fold_no;
      slasktext[line].flags=fold_flags;
      slasktext[line].current_style=0;
      slasktext[line].old_style=0;
    }
/*
    if ((flags[slasktext[line].text[slasktext[line].length-1]] & fact_NEWLINE)) {
      slasktext[++line].length=text+length-mem-1;
      size+=slasktext[line].length;
      if(slasktext[line].length)
        slasktext[line].text = mem+1;
      else
        slasktext[line].text = NULL;
      slasktext[line].fold_no=fold_no;
      slasktext[line].flags=fold_flags;
    }
*/
  }
  if (can_be_folded)
    can_be_folded->shared->size=size;
  *rettext=slasktext;
  *lines=line;
  return(OK);
}

/**********************************************************************
 *
 * int ReadFile(BufStruct *Storage, ReadFileStruct *RFS)
 *
 * Reads a file into memory and store the information in a ReadFileStruct.
 * Give a (BufStruct *) if you want some information to be output on the screen.
 *
 * If anything goes wrong, no program will be available and no freeing
 * is needed by the caller of this function.
 *
 * Return: a ret-value.
 *******/
int __regargs ReadFile(BufStruct *Storage, ReadFileStruct *RFS)
{
  struct FileHandle *fileread;
  struct FileInfoBlock fileinfo;
  BPTR lock;
  int ret=OK;
  int size=0;
  int fileret;
  char *program=NULL;
  char text[FILESIZE+40];

  RFS->pack[0]='\0';
  RFS->password[0]='\0';

  LockBuf_release_semaphore(Storage?BUF(shared):NULL);

  if (RFS->path)
    strmfp(buffer, RFS->path, RFS->filename);
  else
    strcpy(buffer, RFS->filename);

  if (Storage) {
    Sprintf(text, RetString(STR_LOADING), RFS->filename);
    Status(Storage, text, 0);
  }

  if (lock=Lock((UBYTE *)buffer, ACCESS_READ)) {
    if (Examine((BPTR)lock, &fileinfo)) {
      if (fileinfo.fib_EntryType<0) {
        if (RFS->realpath && Default.full_path) {
          if (Default.full_path==2 || !strchr(buffer+1, ':')) {
            NameFromLock((BPTR)lock, RFS->realpath, FILESIZE);
            Stcgfp(RFS->realpath, RFS->realpath);
          } else
            Stcgfp(RFS->realpath, buffer);
        }
        size=fileinfo.fib_Size;
        if ((size+constant>67217+435261) && Storage && BUF(reg.reg)) {
          SendReturnMsg(cmd_RET, UNREG_VERSION, NULL, NULL, NULL);
//          ret=OPEN_ERROR;
        }
//        else 
        {
          RFS->fileprotection=fileinfo.fib_Protection;
          memcpy(&RFS->comment, fileinfo.fib_Comment, 80);
          memcpy(&RFS->date, &fileinfo.fib_Date, sizeof(struct DateStamp));
        }
      }
    } else
      ret=OPEN_ERROR;
    UnLock((BPTR)lock);
    if (fileinfo.fib_EntryType<0) {
      if (ret==OK) {
        struct XpkFib fileinfo;
        if (!Default.rwed_sensitive && (RFS->fileprotection & S_IREAD)) {
          SetProtection(buffer, 0);
        } else {
          if (RFS->fileprotection & S_IREAD)
            ret=READ_PROTECTED;
        }
        if (ret>=OK) {
          if(Default.unpack && XpkBase &&
             !XpkExamineTags(&fileinfo, XPK_InName, buffer, TAG_DONE) &&
             fileinfo.xf_Type == XPKTYPE_PACKED) {
            buffer[200]=0;
            if(fileinfo.xf_Flags & XPKFLAGS_PASSWORD) {
              rtGetString(&buffer[200], 79, /* only 79 character password! */
                          RetString(STR_ENTER_PASSWORD_TITLE), NULL,
      	              RTGS_TextFmt, RetString(STR_ENTER_PASSWORD),
      	              TAG_END);
            }
            size = fileinfo.xf_ULen;
            if(program=Malloc(size+XPK_MARGIN)) {
              if(XpkUnpackTags(XPK_InName, buffer, XPK_OutBuf, program,
    			   XPK_Password, &buffer[200],
                               XPK_OutBufLen, size+XPK_MARGIN, TAG_DONE))
                ret = OPEN_ERROR;
              else {
                strcpy(RFS->pack, fileinfo.xf_Packer);
                strcpy(RFS->password, &buffer[200]);
              }
            } else
              ret=OUT_OF_MEM;
          } else if (!(program=Malloc(size+1)))
            ret=OUT_OF_MEM;
          else if (fileread=(struct FileHandle *)Open((UBYTE *)buffer, MODE_OLDFILE)) {
            fileret=Read((BPTR)fileread, (APTR)program, (long)size);
            Close((BPTR)fileread);
            if (fileret==-1) {
              ret=OPEN_ERROR;
            } else {
              char *prog;
              if (Default.unpack && PPBase) {
                if (size>=4 && (*(int *)program)==(*(int *)"PP20")) {
                  int newsize=(*(int *)((int)program+size-4)) >> 8;
                  prog=Malloc(newsize+ 8);
                  if (prog) {
                    ppDecrunchBuffer((UBYTE *)((int)program+size), prog, (ULONG *)((int)program+4), DECR_COL1);	/* OBS errorhantering */
                    Dealloc(program);
                    program=prog;
                    size=newsize;
                    strcpy(RFS->pack, "PP20");
                  } else {
                    Dealloc(program);
                    ret=OUT_OF_MEM;
                  }
                }
              }
            }
          } else
            ret=OPEN_ERROR;
          if (!Default.rwed_sensitive && (RFS->fileprotection & S_IREAD)) {
            SetProtection(buffer, RFS->fileprotection);
          }
        }
      } else 
        ret=OPEN_ERROR;
    }
    if (ret==OPEN_ERROR && RFS->buffer) {
      int error;
      if (error=IoErr()) {
        Fault(error, RetString(STR_OPEN_ERROR), RFS->buffer, 120);
        ret=-(int)buffer;
      }
    }
  } else
    ret=CANT_FIND_FILE;
  if(ret<OK) {
    Dealloc(program);
    program=NULL;
    size=0;
    fileinfo.fib_FileName[0]=0;
  } else {
    if (program)
      program[size]=0;
    if (Storage) {
      Sprintf(ReturnBuffer, RetString(STR_LOADING_READY), buffer, size);
      Status(Storage, ReturnBuffer, 0);
      ret=(int)ReturnBuffer;
    }
  }
  if (RFS->buffer) {
    RFS->buffer[0]=0;
    if (fileinfo.fib_EntryType<0)
      strcpy(RFS->buffer, fileinfo.fib_FileName);
  }
  RFS->program=program;
  RFS->length=size;

  UnLockBuf_obtain_semaphore(Storage?BUF(shared):NULL);

  return(ret);
}
void __regargs GiveProtection(int fileprotection, char *output)
{
  if (fileprotection & S_IARCHIVE)
    *output++='a';
  if (fileprotection & S_ISCRIPT)
    *output++='s';
  if (fileprotection & S_IPURE)
    *output++='p';
  if (!(fileprotection & S_IREAD))
    *output++='r';
  if (!(fileprotection & S_IWRITE))
    *output++='w';
  if (!(fileprotection & S_IEXECUTE))
    *output++='e';
  if (!(fileprotection & S_IDELETE))
    *output++='d';
  *output=0;
}

/**********************************************************************
 *
 * int GetFileList(wildcard, list, remember);
 *
 * Returns number of names matching the wildcard. The names are readable in
 * the struct List the second argument points to.
 *
 * After reading, free all memory used by this function by a
 * OwnFreeRemember(remember); call which takes the same pointer pointer as
 * argument as this function's fourth argument.
 *
 ********/
int __regargs GetFileList(BufStruct *Storage,
			  UBYTE *wildcard,    /* pattern pointer! */
			  struct Files **list, /* list to read names from! */
			  char **remember, /* allocremember pointer pointer */
			  int checkname)
{
  __aligned char plats[sizeof(struct AnchorPath)+FILESIZE];
  struct AnchorPath *file=(struct AnchorPath *)&plats;
  int ret=OK;
  int number=0;
  char *namebuffer;

  LockBuf_release_semaphore(BUF(shared));

  namebuffer=Malloc(FILESIZE);
  if (namebuffer) {
    memset(&plats, 0, sizeof(plats));
    file->ap_BreakBits=0;
    file->ap_Strlen=FILESIZE;     /* we want the entire path! */
  
    *list=NULL;
    namebuffer[0]=0;
    ret=CheckAndPrompt(Storage, wildcard, checkname, namebuffer);
    if (ret!=FUNCTION_CANCEL) {
      if (ret==OK) {
        *list=(struct Files *)OwnAllocRemember(remember, sizeof(struct Files));
        if (*list) {
          (*list)->name=OwnAllocRemember(remember, strlen(namebuffer)+1);
          if ((*list)->name) {
            strcpy((*list)->name, namebuffer);
            (*list)->next=NULL;
            number++;
          }
        }
      } else {
        if(!MatchFirst(wildcard, file)) {	/* Start matching pattern */
          struct Files *newfile;
          do {
            if (file->ap_Info.fib_DirEntryType<0) {
              ret=CheckAndPrompt(Storage, file->ap_Buf,
                                 checkname, namebuffer);
              if (ret!=CANT_FIND_FILE) {
                number++;
                newfile=(struct Files *)OwnAllocRemember(remember, sizeof(struct Files));
                if (newfile) {
                  newfile->name=OwnAllocRemember(remember, strlen(namebuffer)+1);
                  if (newfile->name) {
                    strcpy(newfile->name, namebuffer);
                    newfile->next=*list;
                    *list=newfile;
                  }
                }
              }
            }
          } while(!MatchNext(file));
          MatchEnd(file);
        }
      }
    } else
      number--;		// returnera minus som cancel.
    Dealloc(namebuffer);
  }
  UnLockBuf_obtain_semaphore(BUF(shared));
  return(number);
}
/*******************************************************************
*
*  namebuffer är en pekare var det riktiga namnet ska lagras (inkl path).
*
************/
int __asm CheckAndPrompt(register __a5 BufStruct *Storage,
                         register __a0 char *name,
                         register __d0 int checkname,
                         register __a1 char *namebuffer)
{
  int ret=OK;

  struct FileInfoBlock fileinfo;
  struct FileLock *lock;

  LockBuf_release_semaphore(BUF(shared));
  if (lock=(struct FileLock *)Lock((UBYTE *)name, SHARED_LOCK)) {
    if (Examine((BPTR)lock, &fileinfo)) {
      if (Default.full_path==2 ||
          (Default.full_path==1 && !strchr(name, ':'))) {
        NameFromLock((BPTR)lock, namebuffer, FILESIZE);
      } else {
        buffer[0]=0;
        switch (name[strlen(name)-1]) {
        case ':':	//  skippa ':' och '/'
        case '/':
          break;
        default:
          if (strchr(name, ':') || strchr(name, '/'))
            Stcgfp(buffer, name);
          break;
        }
        strmfp(namebuffer, buffer, fileinfo.fib_FileName);
      }
      if (fileinfo.fib_DirEntryType>=0) {
        register int len=strlen(namebuffer)-1;
        if (namebuffer[len]!=':' && namebuffer[len]!='/') {
          namebuffer[len+1]='/';
          namebuffer[len+2]=0;
        }
      }
      if (checkname) {
        ret=CheckName(Storage, TRUE, FALSE, namebuffer);
      }
    }
    UnLock((BPTR)lock);
  } else
    ret=CANT_FIND_FILE;
  UnLockBuf_obtain_semaphore(BUF(shared));
  return(ret);
}

int __regargs Stcgfn(char *name, char *filename)
{
  register int len=0;

  if (filename) {
    while (*filename && len<FILESIZE) {/* Vänta till namnet är slut */
      name[len]=*filename;	/* Kopiera namnet */
      len++;			/* Öka längden av resultatet */
      if (*filename==':' || *filename=='/') /* Kolla efter ':' eller '/' */
        len=0;			/* Börja om kopieringen från början */
      filename++;			/* Öka på räknaren av namnet */
    }
  }
  name[len]=0;			/* Nollbyte på slutet */
  return(len);			/* Returnera längden av filnamnet */
}

int __regargs Stcgfp(char *path, char *filename)
{
  register int len=0;
  register int lastend=0;

  if (filename) {
    while (*filename && len<FILESIZE) {/* Vänta till namnet är slut */
      path[len]=*filename;	/* Kopiera namnet */
      len++;			/* Öka längden av resultatet */
      if (*filename==':' || *filename=='/') /* Kolla efter ':' eller '/' */
        lastend=len;		/* Spara positionen för slut av möjlig path */
      filename++;			/* Öka på räknaren av namnet */
    }
  }
  path[lastend]=0;		/* Nollbyte på slutet */
  return(lastend);		/* Returnera längden av pathen */
}


int __regargs RenameBuf(BufStruct *Storage, char *name)
{
  if (Split(BUF(shared), name))
    CheckName(Storage, FALSE, TRUE, NULL);	// New name given.
  SHS(date.ds_Days)=-1;
  BUF(namedisplayed)=FALSE;
  Showstat(Storage);
  return(OK);
}

/* Returns number of chars to skip (includes the '|' and '&', so to say) */
int __regargs GetWildWord(char *from, char *store)
{
  register int len=0;
  
  while (*from) {
    len++;
    if (*from=='|' || *from=='&')
      break;
    *store++=*from++;
  }
  *store=0;
  return(len);
}


/*  Change the current directory */
void __regargs ChangeDirectory(char *dir)
{
  register BPTR newlock;
  register BOOL store=TRUE;
  newlock=Lock(dir, ACCESS_READ);
  if (!newlock) {
    if (Default.olddirectory>=0) {
      newlock=Default.olddirectory;
      store=FALSE;
    }
  }
  if (newlock) {
    register BPTR oldlock;
    oldlock=CurrentDir(newlock);
    if (Default.olddirectory<0)
      Default.olddirectory=oldlock;
    else {
      if (store)
        UnLock(oldlock);
    }
  }
}



