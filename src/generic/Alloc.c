/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************************
*
*  Allocate.c
*
**********/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef AMIGA
#include <exec/execbase.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <clib/graphics_protos.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <proto/dos.h>
#endif

#include "Buf.h"
#include "Alloc.h"
#include "Block.h"
#include "BufControl.h"
#include "BuildMenu.h"
#include "Command.h"
#include "Edit.h"
#include "FrexxEd_rev.c"
#include "GetFile.h"
#include "Init.h"
#include "Limit.h"
#include "OpenClose.h"
#include "Request.h"
#include "Search.h"
#include "Strings.h"
#include "Undo.h"
#include "UpdtScreen.h"
#include "WindowOutput.h"
#include "Execute.h" /* for the userdata struct */

extern DefaultStruct Default;
extern int allowcleaning;
extern char *firstalloc;
extern int totalalloc;
extern char GlobalEmptyString[];
extern char DebugOpt; /* debug on/off */
extern char *lockedalloc;
extern BOOL freelockedalloc;
extern struct AllocCache alloc_cache[];
extern char buffer[];
extern BOOL cache_allocs;

void FreeLockedAlloc()
{
    char *pointer=lockedalloc;
    lockedalloc=NULL;
    if (freelockedalloc) {
        Dealloc(pointer);
        freelockedalloc=FALSE;
    }
}


/*************************************
 *
 * InitAlloc()
 *
 ***************/

void *InitAlloc()
{
  return (void *)-1;
}

void KillAlloc()
{
}

void * __asm FPLmalloc(register __d0 int size,
                       register __a0 struct UserData *user)
{
  char *ret;
  // FIXME: Is this needed at all?
#ifdef REG_A4
  register long a4 = getreg(REG_A4);
  putreg(REG_A4, user->a4);
#endif

  ret = AllocMem(size, MEMF_ANY);

#ifdef REG_A4
  putreg(REG_A4, a4);
#endif
  return (void *)ret;
}

void __asm FPLfree(register __a1 void *mem,
                   register __d0 int size,
                   register __a0 struct UserData *user)
{
#ifdef REG_A4
  register long a4 = getreg(REG_A4);
  putreg(REG_A4, user->a4);
#endif

  FreeMem((char *)mem, size);

#ifdef REG_A4
  putreg(REG_A4, a4);
#endif
}

/************************************************
*
*  OwnAllocRemember(char **, int)
*
*  Performs a remember-malloc.
*
**********/
char __regargs *OwnAllocRemember(char **remember, int size)
{
  register char *ret;

  if (!size)
    ret=NULL;
  else {
    ret=Malloc(size+4);
    if (ret) {
      *(char **)ret=*remember;
      *remember=ret;
      ret+=4;
    }
  }
  return(ret);
}
/************************************************
*
*  OwnFreeRemember(char **)
*
*  Performs a free from your rememberlist.
*
**********/
void __regargs OwnFreeRemember(char **remember)
{
  register char *mem;

  while (mem=*remember) {
    *remember=*((char **)mem);
    Dealloc(mem);
  }
}

/************************************************
*
*  DeallocLine(BufStruct *, int)
*
*  Performs a free.
*
**********/
void __regargs DeallocLine(BufStruct *Storage, int rad)
{
  register char *mem=RAD(rad);

  if (rad) {
    if (mem<SHS(fileblock) || mem>(SHS(fileblock)+SHS(fileblocklen))) {
      if (mem)
        Dealloc(mem);
    } else {
      SHS(fragment)++;
      SHS(fragmentlen)+=LEN(rad);
    }
  }
  if (allowcleaning) {
    register int fmax=(SHS(line) >> 3)+Default.fragmentmax;
    register int flmax=(SHS(size) >> 3)+Default.fragmentmemmax;
    if (SHS(fragment)>fmax || SHS(fragmentlen)>flmax)
      CleanUp(Storage);
  }
}

/************************************************
*
*  Realloc(char *, int)
*
*  Performs a realloc.
*
**********/
char __regargs *Realloc(char *mem, int size)
{
  register char *ret;

  ret=Remalloc(mem, size);
  if (!ret)
    Dealloc(mem);

  return(ret);
}
/************************************************
*
*  Remalloc(char *, int)
*
*  Performs a remalloc.
*
**********/
char __regargs *Remalloc(char *mem, int size)
{
  register char *ret;
  register int oldsize=1;

  if (mem) {
    if (mem!=(void *)GlobalEmptyString) {
      oldsize=(((AllocStruct *)(((char *)mem)-sizeof(AllocStruct)))->size)-sizeof(AllocStruct);
#ifdef alloctest
      oldsize-=sizeof(PostAllocStruct);
#endif
    }
    if (size>oldsize || size<oldsize-16) {
      ret=Malloc(size);
      if (ret) {
        memcpy(ret, mem, (size<oldsize)?size:oldsize);
//        Move4(ret, mem, (((size<oldsize)?size:oldsize)+3)>>2);
        Dealloc(mem);
      } else {
        if (size<oldsize)
          ret=mem;
      }
    } else
      ret=mem;
  } else
    ret=Malloc(size);

  return(ret);
}
/************************************************
*
*  ReallocLine(SharedStruct *, int rad, int size)
*
*  Performs a realloc.
*
**********/
char __regargs *ReallocLine(SharedStruct *shared, int rad, int size)
{
  char *ret;
  static int lastline=0;
  static int lastsize=0;
  static SharedStruct *lastbuf=NULL;
  
  if (allowcleaning && shared->Entry) {
    register int fmax=(shared->line >> 3)+Default.fragmentmax;
    register int flmax=(shared->size >> 3)+Default.fragmentmemmax;
    if (shared->fragment>=fmax || shared->fragmentlen>=flmax)
      CleanUp(shared->Entry);
  }
  if (shared->text[rad].text<shared->fileblock ||
      shared->text[rad].text>(shared->fileblock+shared->fileblocklen)) {
    ret=(char *)Remalloc(shared->text[rad].text, size);
    if (!ret && size) {
      if (size<=shared->text[rad].length)
        ret=shared->text[rad].text;
      else {
        ret=Malloc(size);	// NULL pointer is OK
        if (ret) {
          memcpy(ret, shared->text[rad].text, size);
          Dealloc(shared->text[rad].text);
        }
      }
    }
  } else {
    shared->fragment++;
    if (size<=shared->text[rad].length) {
      shared->fragmentlen+=shared->text[rad].length-size;
      ret=shared->text[rad].text;
    } else {
      if (shared==lastbuf && rad==lastline && size<=lastsize) {
        ret=shared->text[rad].text;
        shared->fragmentlen-=size-lastsize;
      } else {
        shared->fragmentlen+=shared->text[rad].length;
        ret=Malloc(size);	// NULL pointer is OK
        if (ret)
          memcpy(ret, shared->text[rad].text, shared->text[rad].length);
      }
    }
  }
  return(ret);
}

/***************************************************
*
*  CleanUp(BufStruct *)
*
*  Fill all holes in the fileblock
*  Returns a 'ret' value
*******/
int __regargs CleanUp(BufStruct *Storage)
{
  int rad=1, len;
  char *mem;

  if (SHS(fragmentlen)) {	/* If no fragmentations, no cleanup is required */
    if (BUF(window))
      Status(Storage, RetString(STR_GARBAGE_COLLECTION_IN_PROCESS), 1);
    SHS(fragment)=0;
    SHS(fragmentlen)=0;
    if (!(mem=SHS(fileblock)))
      return(OK);
    do {
      if (RAD(rad)>=SHS(fileblock) && RAD(rad)<=(SHS(fileblock)+SHS(fileblocklen))) {
        if (mem!=RAD(rad)) {
          memcpy(mem, RAD(rad), LEN(rad));
          RAD(rad)=mem;
        }
        mem+=LEN(rad);
      }
      rad++;
    } while (rad<=SHS(line));
    len=mem-SHS(fileblock);
    if (!(mem=Realloc(SHS(fileblock), len+1)) && len)
      return(OUT_OF_MEM);
    mem[len]=0;
    SHS(fileblocklen)=len;
    if (mem!=SHS(fileblock)) {
      register int dif=SHS(fileblock)-mem;
      register int rad=1;
      do {
        if (RAD(rad)>=SHS(fileblock) && RAD(rad)<=(SHS(fileblock)+SHS(fileblocklen)))
          RAD(rad)-=dif;
        rad++;
      } while (rad<=SHS(line));
      SHS(fileblock)=mem;
    }
    if (BUF(window))
      Showname(Storage);
  }
  return(OK);
}

/***************************************************
*
*  CompactBuf(SharedStruct *, String *)
*
*  Compacts the entire text to one contiguous area
*  Give a pointer for the result (NULL is ok)
*  Returns a ret-value
*
*******/
int __regargs CompactBuf(SharedStruct *shared, String *retstring)
{
  int ret=OK;
  int temp_allowcleaning;
  int rad=1;
  char *mem, *mem2;

  if (shared->size==shared->fileblocklen && !shared->fragmentlen &&
      shared->text[1].text==shared->fileblock) {
      if (retstring) {
          retstring->string=shared->fileblock;
          retstring->length=shared->size;
      }
  } else {
    if (!(mem2=mem=Malloc(shared->size+1)))
      return(OUT_OF_MEM);
  
    mem[shared->size]=0;		/* Clear last character */
    temp_allowcleaning=allowcleaning;
    allowcleaning=FALSE;
    do {
      memcpy(mem, shared->text[rad].text, shared->text[rad].length);
      DeallocLine(shared->Entry, rad);
      shared->text[rad].text=mem;
      mem+=shared->text[rad].length;
      rad++;
    } while (rad<=shared->line);

    allowcleaning=temp_allowcleaning;
    shared->fragment=0;
    shared->fragmentlen=0;
    Dealloc(shared->fileblock);
    shared->fileblock=mem2;
    shared->fileblocklen=shared->size;
    if (retstring) {
      retstring->string=mem2;
      retstring->length=shared->size;
    }
  }
  return(ret);
}

/************************************************
*
*  Dealloc(char *)
*
*  Performs a free.
*
**********/
void __regargs Mydealloc(void *mem)
{
#ifdef DEBUGTEST
  if (DebugOpt) {
    if (GlobalEmptyString[0])
      Ok_Cancel(NULL, "Someone changed the global empty string", "", NULL);
  }
#else
  GlobalEmptyString[0]=0;
#endif

  if (mem && mem!=(void *)GlobalEmptyString) {
    if (lockedalloc==(char *)mem) {
      freelockedalloc=TRUE;
    } else {
      register int size;
      mem=(void *)(((int)mem)-sizeof(AllocStruct));
      size=((AllocStruct *)(char *)mem)->size;
      if (((AllocStruct *)mem)->lock&0x7fff)
        ((AllocStruct *)mem)->lock|=0x8000;
      else {
          if (cache_allocs && size<ALLOC_CACHE_MAXSIZE) {
              ((struct AllocCache *)mem)->pekare=alloc_cache[size>>3].pekare;
              alloc_cache[size>>3].pekare=(struct AllocCache *)mem;
          } else {
              FreeMem((char *)mem, size);
              totalalloc-=size;
          }
      }
    }
  }
}


void *Mymalloc(int size) {
    if (size > 131072)    fprintf(stderr,"Mymalloc %ld\n", size);
    char *ret=NULL;
    if (size>=0) {
        size=(size+sizeof(AllocStruct)+8)&~7;	// OBS ska kunna vara +7

        if (size<ALLOC_CACHE_MAXSIZE && alloc_cache[size>>3].pekare) {
            ret=(char *)alloc_cache[size>>3].pekare;
            alloc_cache[size>>3].pekare=((struct AllocCache *)ret)->pekare;
        } else {
            ret=(char *)AllocMem(size, MEMF_ANY);
      }
        if (ret) {
            totalalloc+=size;
            ((AllocStruct *)ret)->size=size;
            ((AllocStruct *)ret)->lock=0;
            ret+=sizeof(AllocStruct);
        }
    }
    return(ret);
}


/* Allocate the memory and copy, plus add a zero at the end */
void __regargs *MyMemdup(char *str, int len)
{
  register char *ret=NULL;

  if (str) {
    if (len) {
      ret=Mymalloc(len+1);
    }
    if (ret) {
      memcpy(ret, str, len);
      ret[len]=0;
    } else
      ret=GlobalEmptyString;

  }
  return(ret);
}

void *Mystrdup(char *str)
{
  char *ret=GlobalEmptyString;
  if (str) {
    ret=MyMemdup(str, strlen(str));
  }
  return(ret);
}

void FreeCache() {
    int count;
    for (count=0; count<ALLOC_CACHE_MAXSIZE/8; count++) {
        struct AllocCache *next;
        struct AllocCache *pek=alloc_cache[count].pekare;
        while (pek) {
            next=pek->pekare;
            FreeMem((char *)pek, (count)<<3);
            pek=next;
        }
    }
    cache_allocs=FALSE;
}
