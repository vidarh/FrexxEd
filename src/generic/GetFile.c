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

#ifndef AMIGA
#define NO_XPK
#define NO_PPACKER
#endif

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
#ifdef AMIGA
#include "Icon.h"
#endif
#include "Init.h"
#include "Limit.h"
#include "Request.h"
#include "Sort.h"
#include "Undo.h"
#include "UpdtScreen.h"
#include "WindowOutput.h"

#define FREQF_SAVE 2

extern char **Argv;
extern int Argc;
extern char buffer[];
extern DefaultStruct Default;
extern int _OSERR;
extern int Visible;
extern FACT *DefaultFact;
extern char ReturnBuffer[RET_BUF_LEN];
extern struct Library *XpkBase;
extern char DebugOpt;
extern int constant;	// Konstant för maxfillängd utan registrering.
extern char GlobalEmptyString[];
extern BufStruct *NewStorageWanted;

static const char * GetFile(BufStruct *Storage,  char *path, char *filename) {
    ReadFileStruct RFS;

    const char * ret=OK;
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
            int newname=TRUE;
            GiveProtection(SHS(fileprotection), (char *)&SHS(protection));
            strcpy(SHS(pack_type), RFS.pack);
            strcpy(SHS(password), RFS.password);
            if (SHS(name_number)>=1) {
                if (strcasecmp(SHS(filnamn), filename))
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
        ret=RetString(STR_EDITING_NEW_FILE);
    }
    return ret;
}

/**********************************************************************
 *
 * Split()
 *
 * Divides the given string in one directory part and one file name part.
 * Return FALSE if the file name is the same.
 ********/
int Split(SharedStruct *shared, char *name) {
    char temp[FILESIZE];
    int check=Stcgfn(temp, name);
    int ret;

    ret=stricmp(temp, shared->filnamn);
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
    } else {
        shared->path=Strdup("");
    }
    DateStamp(&shared->date);
    
    return(ret);
}

static void sortFileNames(char ** pointers, int antal) {
    char **stringarray[2];
    stringarray[0]=pointers;
    stringarray[1]=NULL;
    ShellSort(stringarray, 1, 1, antal, SORT_BACK);
}

int Load(BufStruct *Storage, char *string, int flag, char *file) {
    char filename[FILESIZE];
    BufStruct *Storage2;
    int ret=OK;
    int buffers=0;
    char **pointers=NULL;
    int antal=0, counter;

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
            if (!(flag & loadBUFFERPATH)) Stcgfp(buffer, file);
        }
        int len=strlen(buffer);
        if (len && buffer[len-1]=='/') buffer[len-1]=0;
        file=NULL;
    }
    if (file && !file[0]) file=NULL;

    fileReqSetMatchPat("");
    fileReqSetDir(buffer);

    void *filelist=NULL;
    if (!file) filelist = fileReqRequest(Storage,filename,string);
        
    if (file || filelist) {
        if (!file) fileReqGetPath(filelist,filename);
        else strcpy(filename, file);
        if (fileReqFilelistNotEmpty(filelist)) {
            antal  = fileReqCountFiles(filelist);

            pointers=(char **)Malloc(sizeof(char *)*antal);
            char * tempbuffer=Malloc(FILESIZE);
            if (!pointers || !tempbuffer) return(OUT_OF_MEM);
            antal=0;
            struct rtFileList * filetemp=filelist;
            while (filetemp) {
                fileReqPath(tempbuffer, filetemp);
                if (CheckAndPrompt(Storage, tempbuffer, !(flag&(loadNOQUESTION|loadINCLUDE)), filename)>=OK) {
                    pointers[antal]=Strdup(filename);
                    antal++;
                }
                filetemp=filetemp->Next;
            }
            Dealloc(tempbuffer);
            if (!antal) antal--;
        } else {
            antal=GetFileList(Storage, filename, &list, &remember, !(flag&(loadNOQUESTION|loadINCLUDE)));
            if (antal>0) {
                struct Files *filetemp;
                pointers=(char **)Malloc(sizeof(char *)*antal);
                if (!pointers) return(OUT_OF_MEM);
                antal=0;
                filetemp=list;
                while (filetemp) {
                    pointers[antal]=filetemp->name;
                    antal++;
                    filetemp=filetemp->next;
                }
                sortFileName(pointers,antal);
            }
        }
        if (antal<=0) {
            if (antal==0 &&
                !strrchr(filename, '?') && !strrchr(filename, '#') &&
                !strrchr(filename, '|') && !strrchr(filename, '~') &&
                !strrchr(filename, '[') && !strrchr(filename, ']')) {
                if ((flag & loadNEWBUFFER) &&
                    !(flag & loadINCLUDE)) {
                    int hookret;
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
                ret = antal ? FUNCTION_CANCEL : OPEN_ERROR;
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
                
                if (ret==OK) {
                    if ((flag & loadNEWBUFFER)) {
                        int hookret;
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
                    
                    if (ret==OK) {
                        if (!buffers) Clear(Storage2, FALSE);
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
        if (pointers && fileReqFileListNotEmpty(filelist)) {
            while (--antal>=0)
                Dealloc(pointers[antal]);
        }
        Dealloc(pointers);
    } else 
        ret=FUNCTION_CANCEL;
    ActivateEditor();
    fileReqFreeFileList(filelist);
    OwnFreeRemember(&remember);
    return(ret);
}

int Save(BufStruct *Storage, int flag, char *string, char *file, char *packmode)
{
  struct FileHandle *filewrite;
  String fileblock={NULL, 0};
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
  if (ret >= OK) ret = fileReqCheckFileModifiedOnDisk(file,Storage,text);

  if (ret>=OK) {
      BOOL resave=TRUE;		// Save the file normal
      if (Default.safesave) {
          SetProtection(falsefilename, 0);
          DeleteFile(falsefilename);
      }
      
      if (packmode[0]) {
          BOOL fine=FALSE;
          if (FoldCompactBuf(BUF(shared), &fileblock)==OK) {
              // FIXME: arguments / return
              fileReqCompress();
          }
          if (!fine) {
              if (!Ok_Cancel(Storage, RetString(STR_CANT_CRUNCH_BUFFER), NULL, NULL)) {
                  ret=FUNCTION_CANCEL;
                  resave=FALSE;
              }
          }
      }
      
      if (ret>=OK && resave) {	// Save it normally (pack failed)
          struct FileHandle * filewrite=(struct FileHandle *)Open(falsefilename, MODE_NEWFILE);
          if (filewrite) {
              Sprintf(filename, RetString(STR_SAVING_TEXT), text);
              Status(Storage, filename, 0);
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
          } else
              ret=OPEN_ERROR;
      }
      if (ret>=OK) {
          
          ret = fileReqSetAttributes(Storage,falsefilename,text, allocation);

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
                      if(!Strcasecmp(&buffer[0], &buffer[FILESIZE])) {
                          /* We're in a root directory! */
                          parent=TRUE;
                      } else {
                          if(!CheckIcon(&buffer[FILESIZE]))
                              /* parent had no icon! */
                              parent = FALSE;
                      }
                      UnLock(lock);
                  }
              }

              if(parent) MakeIcon(text);
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
          /* FIXME: Not sure if this is actually used, but
             it's risky and not portable, so removing it for now
             Sprintf(ReturnBuffer, RetString(STR_WROTE), text); */
          ret=OK;
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
int CheckName(BufStruct *Storage, int requester, int update, char *name) {
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

    SharedStruct *Shared2=Default.SharedDefault.Next;
    while (Shared2) {
        if (Shared2!=Shared) {
            if (!strcmp(Shared2->filnamn, name)) {
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
int ExtractLines(char *text, int length, TextStruct **rettext, int *lines, BufStruct *can_be_folded) {
  unsigned char *mem=text;
  int len=length;
  char *flags=DefaultFact->flags;
  int rows=0;
  TextStruct *slasktext;
  int line=0;
  int size=0;
  int fold_no=0, fold_flags=0;
  BOOL fold_found;
  unsigned char *next_foldstart=text+length+1;
  unsigned char *next_foldend=text+length+1;
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
          register unsigned char *temppek=next_foldend+sizeof(FOLD_STRING_END)-1;
          while (--maxlen>=0 && *(temppek++)!='.');
          temppek+=fold_end_len;
          if (temppek-1<=mem) { // -1 to accept last row
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
          unsigned char *temppek=next_foldstart+sizeof(FOLD_STRING_BEGIN)-1;
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
  }
  if (can_be_folded)
    can_be_folded->shared->size=size;
  *rettext=slasktext;
  *lines=line;
  return(OK);
}

void GiveProtection(int fileprotection, char *output)
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
int GetFileList(BufStruct *Storage,
                UBYTE *wildcard,    /* pattern pointer! */
                struct Files **list, /* list to read names from! */
                char **remember, /* allocremember pointer pointer */
                int checkname)
{
    int ret=OK;
    int number=0;
    char *namebuffer;
    
    LockBuf_release_semaphore(BUF(shared));
    
    namebuffer=Malloc(FILESIZE);
    if (namebuffer) {
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
                char plats[sizeof(struct AnchorPath)+FILESIZE];
                struct AnchorPath *file=(struct AnchorPath *)&plats;
                memset(&plats, 0, sizeof(plats));
                file->ap_BreakBits=0;
                file->ap_Strlen=FILESIZE;     /* we want the entire path! */
                
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

int RenameBuf(BufStruct *Storage, char *name)
{
  if (Split(BUF(shared), name))
    CheckName(Storage, FALSE, TRUE, NULL);	// New name given.
  SHS(date.ds_Days)=-1;
  BUF(namedisplayed)=FALSE;
  Showstat(Storage);
  return(OK);
}

/*  Change the current directory */
void ChangeDirectory(char *dir)
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



