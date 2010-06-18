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

#include <exec/execbase.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <clib/graphics_protos.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <proto/dos.h>

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

extern struct ExecBase *SysBase;
extern DefaultStruct Default;
extern int allowcleaning;
extern char *firstalloc;
extern int totalalloc;
extern char GlobalEmptyString[];
extern struct Library *XpkBase;
extern struct Library *PPBase;
extern int MacroOn;		/* Macro på (1) eller av (0) */
extern BlockStruct *YankBuffer;
extern BlockStruct MacroBuffer;
extern srch Search;          /* search structure */
extern HistoryStruct SearchHistory;
extern char DebugOpt; /* debug on/off */
extern char *lockedalloc;
extern BOOL freelockedalloc;
extern struct MenuInfo menu;
extern struct AllocCache alloc_cache[];
extern char buffer[];
extern BOOL cache_allocs;

#ifdef alloctest
//#define FPL_ALLOCTEST
int total_fpl_alloc=0;
char *fpl_firstalloc=NULL;
#endif

#ifdef POOL_ALLOC

static void *MemoryPool=NULL;

#endif

/*************************************
 *
 * InitAlloc()
 *
 ***************/

void *InitAlloc()
{
#ifdef POOL_ALLOC
  MemoryPool = AsmCreatePool ( MEMF_ANY, SIZE_POOL, SIZE_MAX_IN_POOL, SysBase );
  return MemoryPool;
#else
  return (void *)-1;
#endif
}

void KillAlloc()
{
#ifdef POOL_ALLOC
  if(MemoryPool)
    AsmDeletePool(MemoryPool, SysBase);
#endif
}

void * __asm FPLmalloc(register __d0 int size,
                       register __a0 struct UserData *user)
{
  char *ret;
  register long a4 = getreg(REG_A4);
  putreg(REG_A4, user->a4);
#ifdef  FPL_ALLOCTEST
CheckAllocs(fpl_firstalloc);
  size+=sizeof(PostAllocStruct);
  size=(size+sizeof(AllocStruct)+8)&~7;	// OBS ska kunna vara +7
#endif

#ifdef POOL_ALLOC
  ret = AsmAllocPooled(MemoryPool, size, SysBase);

#else
  ret = AllocMem(size, MEMF_ANY);
#endif

#ifdef  FPL_ALLOCTEST
if (ret) memset(ret, 0, size);  //DEBUG

  if (ret) {
    static int nummer=0;
    total_fpl_alloc+=size;
    ((AllocStruct *)ret)->size=size;
    ((AllocStruct *)ret)->lock=0;
    if (fpl_firstalloc)
      ((AllocStruct *)fpl_firstalloc)->Prev=ret;
    ((AllocStruct *)ret)->Prev=NULL;
    ((AllocStruct *)ret)->Next=fpl_firstalloc;
    fpl_firstalloc=ret;
    ((AllocStruct *)ret)->check=*(int *)"K&B";
    ((AllocStruct *)ret)->line=0;
    ((AllocStruct *)ret)->file="FPL";
    ((AllocStruct *)ret)->number=nummer++;
    ((AllocStruct *)ret)->premungwall=MUNGWALL_PREVALUE;
    ((PostAllocStruct *)(ret+size-sizeof(PostAllocStruct)))->postmungwall=MUNGWALL_POSTVALUE;
    ret+=sizeof(AllocStruct);
  }
  if (ret)
    *((int *)(((char *)ret)+((size-4)&~3)))=MUNGWALL_POSTVALUE;
#endif
  putreg(REG_A4, a4);
  return (void *)ret;
}

void __asm FPLfree(register __a1 void *mem,
                   register __d0 int size,
                   register __a0 struct UserData *user)
{
  register long a4 = getreg(REG_A4);
  putreg(REG_A4, user->a4);
#ifdef  FPL_ALLOCTEST
  CheckAllocs(fpl_firstalloc);
  {
    mem=(void *)(((int)mem)-sizeof(AllocStruct));
    size=((AllocStruct *)(char *)mem)->size;
    if (((AllocStruct *)mem)->check!=*(int *)"K&B") {
      int count=0;
      while(count<1000000)
        *(int *)0xdff180=count++;
      if (((AllocStruct *)mem)->check!=*(int *)"KKK")
        Sprintf(buffer, "\n\a"FREXXED_VER": Dealloc error from FPL!\n");
      else
        Sprintf(buffer, "\n\a"FREXXED_VER": Dubble dealloc from FPL!\n");
      Ok_Cancel(NULL, buffer, "", NULL);
      return;
    }
  }
  {
    register AllocStruct *AS=(AllocStruct *)mem;
    AS->check=*(int *)"KKK";
  
    if (AS->Next)
      ((AllocStruct *)AS->Next)->Prev=AS->Prev;
    if (AS->Prev)
      ((AllocStruct *)AS->Prev)->Next=AS->Next;
    else
      fpl_firstalloc=AS->Next;
    {
      register AllocStruct *AS=(AllocStruct *)mem;
      if ((AS->premungwall!=MUNGWALL_PREVALUE)) {
        int count=0;
        while(count<1000000)
          *(int *)0xdff180=count++;
        Sprintf(buffer, "\n\aFPL MUNGWALL PRE ERROR allocated from %s %ld", AS->file, AS->line);
        Ok_Cancel(NULL, buffer, "", NULL);
        putreg(REG_A4, a4);
        return;
      }
      if ((((PostAllocStruct *)(((char *)mem)+size-sizeof(PostAllocStruct)))->postmungwall!=MUNGWALL_POSTVALUE)) {
        int count=0;
        while(count<1000000)
          *(int *)0xdff180=count++;
        Sprintf(buffer, "\n\aFPL MUNGWALL POST ERROR allocated from %s %ld", AS->file, AS->line);
        Ok_Cancel(NULL, buffer, "", NULL);
        putreg(REG_A4, a4);
        return;
      }
    }
  }
  memset(mem, 0x55, size);
  ((char *)mem)[size-1]=0;
  total_fpl_alloc-=size;
#endif

#ifdef POOL_ALLOC
  AsmFreePooled(MemoryPool, mem, size, SysBase);
#else
  FreeMem((char *)mem, size);
#endif
  putreg(REG_A4, a4);
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
*  Fyll igen alla hål i fileblocket.
*  Returnerar ett 'ret'värde.
*******/
int __regargs CleanUp(BufStruct *Storage)
{
  int rad=1, len;
  char *mem;

  if (SHS(fragmentlen)) {	/* Om ingen fragmentering, så behövs inget rensas upp. */
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
*  Lägger hela texten i en klump.
*  Ge en pekare för resultatet (NULL accepteras).
*  Returnerar ett ret-värde.
*
*******/
int __regargs CompactBuf(SharedStruct *shared, String *retstring)
{
  int ret=OK;
  int temp_allowcleaning;
  int rad=1;
  char *mem, *mem2;

  if (shared->size==shared->fileblocklen && !shared->fragmentlen &&
      shared->text[1].text==shared->fileblock) {	/* Dessa 6 rader är omändrat 17/10-93, Borde fungera */
    if (retstring) {
      retstring->string=shared->fileblock;
      retstring->length=shared->size;
    }
  } else {
    if (!(mem2=mem=Malloc(shared->size+1)))
      return(OUT_OF_MEM);
  
    mem[shared->size]=0;		/* Töm sista tecknet */
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
#ifndef alloctest
void __regargs Mydealloc(void *mem)
#else
void __regargs Mydealloc(void *mem, int line, char *file)
#endif
{
//  static int nummer=0;
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
#ifdef alloctest
      {
        if (((AllocStruct *)mem)->check!=*(int *)"K&B") {
          if (((AllocStruct *)mem)->check!=*(int *)"KKK")
            FPrintf(Output(), "\n\a"FREXXED_VER": Dealloc error from '%s' line %ld!\n", file, line);
          else
            FPrintf(Output(), "\n\a"FREXXED_VER": Dubble dealloc from '%s' line %ld!\n", file, line);
          line=*(char *)-1;
          return;
        }
      }
#endif
      if (((AllocStruct *)mem)->lock&0x7fff)
        ((AllocStruct *)mem)->lock|=0x8000;
      else {
#ifdef alloctest
        register AllocStruct *AS=(AllocStruct *)mem;
        AS->check=*(int *)"KKK";
#if 0 // DEBUG, find allocation
{
  char *count=firstalloc;
  char *last=firstalloc;
  while (count && count!=(char *)AS) {
    if (((AllocStruct *)count)->check!=*(int *)"K&B" ||
        ((((AllocStruct *)count)->premungwall!=MUNGWALL_PREVALUE))) {
      FPrintf(Output(), "\n\aALLOCATION DESTROYED freed from %s %ld\n", file, line);
      FPrintf(Output(), "LAST ONE allocated from %s %ld", ((AllocStruct *)last)->file, ((AllocStruct *)last)->line);
      ((AllocStruct *)last)->Next=NULL;
      return;
    }
    last=count;
    count=(char *)(((AllocStruct *)count)->Next);
  }
  if (!count)
    FPrintf(Output(), "\n\aALLOCATION MISSING freed from %s %ld\n", file, line);
}
#endif
        if (AS->Next)
          ((AllocStruct *)AS->Next)->Prev=AS->Prev;
        if (AS->Prev)
          ((AllocStruct *)AS->Prev)->Next=AS->Next;
        else
          firstalloc=AS->Next;
        {
          register AllocStruct *AS=(AllocStruct *)mem;
          if ((AS->premungwall!=MUNGWALL_PREVALUE)) {
            line=*(char *)-2;
            FPrintf(Output(), "\n\aINTERNAL MUNGWALL PRE ERROR allocated from %s %ld", AS->file, AS->line);
            FPrintf(Output(), "\n\aINTERNAL MUNGWALL PRE ERROR freed from %s %ld\n", file, line);
            return;
          }
          if ((((PostAllocStruct *)(((char *)mem)+size-sizeof(PostAllocStruct)))->postmungwall!=MUNGWALL_POSTVALUE)) {
            line=*(char *)-3;
            FPrintf(Output(), "\n\aINTERNAL MUNGWALL POST ERROR allocated from %s %ld", AS->file, AS->line);
            FPrintf(Output(), "\n\aINTERNAL MUNGWALL POST ERROR freed from %s %ld\n", file, line);
            return;
          }
        }
#endif
//      if (size==-1091576148)
//        size=size*1;
//        printf("\nFREE   %X, %d, %s, %d", (int)mem, size, AS->file, AS->line);
        if (cache_allocs && size<ALLOC_CACHE_MAXSIZE) {
          ((struct AllocCache *)mem)->pekare=alloc_cache[size>>3].pekare;
          alloc_cache[size>>3].pekare=(struct AllocCache *)mem;
        } else {
#ifdef alloctest
          memset(mem, 0x55, size);
          ((char *)mem)[size-1]=0;
#endif
#ifdef POOL_ALLOC
          AsmFreePooled(MemoryPool, mem, size, SysBase);
#else
          FreeMem((char *)mem, size);
#endif
          totalalloc-=size;
        }
      }
    }
  }
}


#ifndef alloctest
void __regargs *Mymalloc(int size)
#else
void __regargs *Mymalloc(int size, int line, char *file)
#endif
{
  register char *ret=NULL;
#ifdef alloctest
  static int nummer=0;
#endif
  if (size>=0) {
#ifdef alloctest
    size+=sizeof(PostAllocStruct);
#endif
    size=(size+sizeof(AllocStruct)+8)&~7;	// OBS ska kunna vara +7

    if (size<ALLOC_CACHE_MAXSIZE && alloc_cache[size>>3].pekare) {
      ret=(char *)alloc_cache[size>>3].pekare;
      alloc_cache[size>>3].pekare=((struct AllocCache *)ret)->pekare;
    } else {
#ifdef POOL_ALLOC
      ret=(char *)AsmAllocPooled(MemoryPool, size, SysBase);
#else
      ret=(char *)AllocMem(size, MEMF_ANY);
#endif
    }
//    if (nummer==1465)
//      nummer+=0;
//    printf("\nALLOC  %X, %d, %s, %d", (int)ret, size, file, line);
    if (ret) {
      totalalloc+=size;
      ((AllocStruct *)ret)->size=size;
      ((AllocStruct *)ret)->lock=0;
#ifdef alloctest
      if (firstalloc)
        ((AllocStruct *)firstalloc)->Prev=ret;
      ((AllocStruct *)ret)->Prev=NULL;
      ((AllocStruct *)ret)->Next=firstalloc;
      firstalloc=ret;
      ((AllocStruct *)ret)->check=*(int *)"K&B";
      ((AllocStruct *)ret)->line=line;
      ((AllocStruct *)ret)->file=file;
      ((AllocStruct *)ret)->number=nummer++;
      ((AllocStruct *)ret)->premungwall=MUNGWALL_PREVALUE;
      ((PostAllocStruct *)(ret+size-sizeof(PostAllocStruct)))->postmungwall=MUNGWALL_POSTVALUE;
#endif
      ret+=sizeof(AllocStruct);
    }
#ifdef alloctest
    else {
      FPrintf(Output(), "\n\a"FREXXED_VER": couldn't allocate %ld bytes!\n", size);
      FPrintf(Output(), "         Called from '%s' line %ld!\n", file, line);
      line=*(char *)-4;
    }
#endif
  }
#ifdef alloctest
    else {
    FPrintf(Output(), "\n\a"FREXXED_VER": couldn't allocate %ld bytes!\n", size);
    FPrintf(Output(), "         Called from '%s' line %ld!\n", file, line);
    line=*(char *)-5;
  }
#endif
  return(ret);
}


/* Allokera minnet och kopiera, plus lägg en nolla på slutet */
#ifdef alloctest
void __regargs *MyMemdup(char *str, int len, int line, char *file)
#else
void __regargs *MyMemdup(char *str, int len)
#endif
{
  register char *ret=NULL;

  if (str) {
    if (len) {
#ifdef alloctest
      ret=Mymalloc(len+1, line, file);
#else
      ret=Mymalloc(len+1);
#endif
    }
    if (ret) {
      memcpy(ret, str, len);
      ret[len]=0;
    } else
      ret=GlobalEmptyString;

  }
  return(ret);
}

#ifdef alloctest
void __regargs *Mystrdup(char *str, int line, char *file)
#else
void __regargs *Mystrdup(char *str)
#endif
{
  register char *ret=GlobalEmptyString;
  if (str) {
#ifdef alloctest
    ret=MyMemdup(str, strlen(str), line, file);
#else
    ret=MyMemdup(str, strlen(str));
#endif
  }
  return(ret);
}

#ifndef POOL_DEALLOC
void __regargs FreeCache()
{
  register int count;
  for (count=0; count<ALLOC_CACHE_MAXSIZE/8; count++) {
    register struct AllocCache *next;
    register struct AllocCache *pek=alloc_cache[count].pekare;
    while (pek) {
      next=pek->pekare;
#ifdef POOL_ALLOC
      AsmFreePooled(MemoryPool, pek, (count)<<3, SysBase);
#else
      FreeMem((char *)pek, (count)<<3);
#endif
      pek=next;
    }
  }
  cache_allocs=FALSE;
}
#endif


#ifdef alloctest

void __regargs CheckAllocs(char *input_firstalloc)
{
  char *count=input_firstalloc;
  AllocStruct *prev=NULL;
  int size;
  int num=0;
/*
  if (GlobalEmptyString[0]) {
    int count=0;
    while(count<1000000)
      *(int *)0xdff180=count++;
    FPrintf(Output(), "\n\aINTERNAL MUNGWALL ERROR in GlobalEmptyString\n");
  }
*/
  while (count) {
    num++;
    size=((AllocStruct *)count)->size;
    if ((((AllocStruct *)count)->premungwall!=MUNGWALL_PREVALUE) ||
        (((PostAllocStruct *)(count+size-sizeof(PostAllocStruct)))->postmungwall!=MUNGWALL_POSTVALUE)) {
      int count=0;
      while(count<1000000)
        *(int *)0xdff180=count++;
      Sprintf(buffer, "\n\aINTERNAL MUNGWALL ERROR allocated from %s %ld num %ld order %ld\n", ((AllocStruct *)count)->file, ((AllocStruct *)count)->line, ((AllocStruct *)count)->number, num);
      Ok_Cancel(NULL, buffer, "", NULL);
      FPrintf(Output(), buffer);
      break;
    }
    prev=(AllocStruct *)count;
    count=((AllocStruct *)count)->Next;
  }
}

#endif

