
#ifdef AMIGA
#include <clib/exec_protos.h>
#include <clib/intuition_protos.h>
#include <dos/dos.h>
#include <dos/dosasl.h>
#include <exec/types.h>
#include <graphics/gfxbase.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>

#ifndef NO_PPACKER
#include <libraries/ppbase.h>
#endif

#include <libraries/reqtools.h>

#ifndef NO_XPK
#include <xpk/xpk.h>
#include <proto/xpkmaster.h>
#include <inline/xpkmaster_protos.h>
#include <proto/xpksub.h>
#include <inline/xpksub_protos.h> 
#endif

#include <proto/dos.h>
#include <proto/FPL.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#ifndef NO_PPACKER
#include <proto/powerpacker.h>
#include <inline/powerpacker_protos.h>
#endif

#include <proto/reqtools.h>
#include <proto/utility.h>
#else
#define NO_XPK
#define NO_PPACKER
#include "compat.h"
#endif

#include <libraries/FPL.h>
#include "Buf.h"
#include "GetFile.h"
#include "Limit.h"

extern char *ReturnBuffer;
extern char buffer[];
extern BOOL OwnWindow;
extern DefaultStruct Default;
extern struct rtFileRequester *FileReq; /* Buffrad Filerequester. Bra o ha! */

void fileReqSetMatchPat(const char * str) {
    rtChangeReqAttr(FileReq, RTFI_MatchPat, str, TAG_END);
}

void fileReqSetDir(const char * buffer) {
    if (strcmp(FileReq->Dir, buffer))
        rtChangeReqAttr(FileReq, RTFI_Dir, buffer, TAG_END);
}

void fileReqFilelistNotEmpty(void * filelist) {
    return filelist && ((struct rtFileList *)filelist)->Next;
}

void fileReqFreeFileList(void * filelist) {
    rtFreeFileList(filelist);
}

struct rtFileList * fileReqRequest(BufStruct * Storage,char *filename, char * string) {
    struct rtFileList * filelist = NULL;
    struct Screen *screen=NULL;
    if (!FRONTWINDOW || !FRONTWINDOW->window_pointer)
        screen=LockPubScreen(NULL);
    LockBuf_release_semaphore(BUF(shared));
    fileReqSetMatchPat(Default.file_pattern);
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
    return filelist;
}

void fileReqGetPath(struct rtFileList * filelist,char * filename) {
    strmfp(filename, FileReq->Dir, filelist->Name);
}

int fileReqCountFiles(struct rtFileList * filetemp) {
    int antal=0;
    while (filetemp) {
        antal++;
        filetemp=filetemp->Next;
    }
    return antal;
}

void fileReqFreeList(struct rtFileList * filelist) {
    rtFreeFileList(filelist);
}

char * fileReqName(struct rtFileList * filetemp) {
    return filetemp->Name;
}

char * fileReqPath(char * tempbuffer, struct rtFileList * filetemp) {
    strmfp(tempbuffer, FileReq->Dir, filetemp->Name);
}

void fileReqXpk() {
#ifndef NO_XPK
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
#endif
}

void fileReqPPacker() {
#ifndef NO_PPACKER
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
#endif
}

void fileReqCompress() {
#if !defined(NO_PPACKER) && !defined(NO_XPK)
    if (!strcasecmp("PP20", packmode)) {
        fileReqPPacker();
    }  else {
        fileReqXpk();
    }
#endif
}

int fileReqCheckFileModifiedOnDisk(char * file, BufStruct * Storage, char * text) {
    struct FileInfoBlock fileinfo;
    int ret = OK;
    BPTR lock;
    if (!file && SHS(date.ds_Days)!=-1 &&
        (lock=Lock((UBYTE *)text, ACCESS_READ))) {
        int fi=FALSE;
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
    return ret;
}

int fileReqSetAttributes(BufStruct * Storage,char * falsefilename, char * text, char * allocation) {
    int slask;
    BPTR lock;
    LONG error = 0;
    int ret = OK;
    struct FileInfoBlock fileinfo;
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
    return ret;
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
int ReadFile(BufStruct *Storage, ReadFileStruct *RFS) {
  struct FileHandle *fileread;
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

  struct FileInfoBlock fileinfo;
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
		RFS->fileprotection=fileinfo.fib_Protection;
		memcpy(&RFS->comment, fileinfo.fib_Comment, 80);
		memcpy(&RFS->date, &fileinfo.fib_Date, sizeof(struct DateStamp));
      }
    } else
      ret=OPEN_ERROR;
    UnLock((BPTR)lock);
    if (fileinfo.fib_EntryType<0) {
      if (ret==OK) {
#ifndef NO_XPK
        struct XpkFib fileinfo;
#endif
        if (!Default.rwed_sensitive && (RFS->fileprotection & S_IREAD)) {
          SetProtection(buffer, 0);
        } else {
          if (RFS->fileprotection & S_IREAD)
            ret=READ_PROTECTED;
        }
        if (ret>=OK) {
#ifndef NO_XPK
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
          } else
#endif
	  /* .. else */ if (!(program=Malloc(size+1)))
            ret=OUT_OF_MEM;
          else if (fileread=(struct FileHandle *)Open((UBYTE *)buffer, MODE_OLDFILE)) {
            fileret=Read((BPTR)fileread, (APTR)program, (long)size);
            Close((BPTR)fileread);
            if (fileret==-1) {
              ret=OPEN_ERROR;
            } else {
#ifndef NO_PPACKER
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
#endif
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
	  /* FIXME: This is broken: It assumes that (int)ReturnBuffer will always result in a positive integer */
      //ret=(int)ReturnBuffer;
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

/*******************************************************************
*
*  namebuffer är en pekare var det riktiga namnet ska lagras (inkl path).
*
************/
int CheckAndPrompt(BufStruct *Storage,
                   char *name,
                   int checkname,
                   char *namebuffer)
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
