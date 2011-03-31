/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************************
*
*  Allocate.h
*
**********/

#define SIZE_POOL	  0x6000  /* size of the memory pools */
#define SIZE_MAX_IN_POOL  0x100   /* maximum memory allocation size to put in
				     a memory pool! */

#include "compat.h"
#include "Buf.h"

/************************************************
*
*  Alloc(int)
*
*  Performs a malloc.
*
**********/
#ifndef alloctest
char __regargs *Myalloc(int size);
void  *Mymalloc(int size);
void  Mydealloc(void *mem);
void  *Mystrdup(char *str);
void  *MyMemdup(char *str, int len);
#define Alloc(x) Myalloc(x)
#define Malloc(x) Mymalloc(x)
#define Dealloc(x) Mydealloc(x)
#define Strdup(x) Mystrdup(x)
#define Memdup(x, y) MyMemdup(x, y)
#else
char  *Myalloc(int size, int line, char *file);
void  *Mymalloc(int size, int line, char *file);
void  Mydealloc(void *mem, int line, char *file);
void  *Mystrdup(char *str, int line, char *file);
void  *MyMemdup(char *str, int len, int line, char *file);
#define Alloc(x) Myalloc(x, __LINE__, __FILE__)
#define Malloc(x) Mymalloc(x, __LINE__, __FILE__)
#define Dealloc(x) Mydealloc(x, __LINE__, __FILE__)
#define Strdup(x) Mystrdup(x, __LINE__, __FILE__)
#define Memdup(x, y) MyMemdup(x, y, __LINE__, __FILE__)
#endif

#ifndef POOLS

void * __asm AsmAllocPooled(register __a0 void *,
                            register __d0 ULONG,
                            register __a6 struct ExecBase *);

void * __asm AsmCreatePool(register __d0 ULONG,
                           register __d1 ULONG,
                           register __d2 ULONG,
                           register __a6 struct ExecBase *);

void __asm AsmDeletePool(register __a0 void *,
                         register __a6 struct ExecBase *);

void __asm AsmFreePooled(register __a0 void *,
                         register __a1 void *,
                         register __d0 ULONG,
                         register __a6 struct ExecBase *);

void *InitAlloc(void);
#endif
void KillAlloc(void);

// Defined in Execute.h
struct UserData;

void __asm FPLfree(register __a1 void *mem,
                   register __d0 int size,
                   register __a0 struct UserData *);
void * __asm FPLmalloc(register __d0 int size,
                       register __a0 struct UserData *);

char  *OwnAllocRemember(char **remember, int size);
void  OwnFreeRemember(char **remember);

/************************************************
*
*  Dealloc(char *)
*
*  Performs a free.
*
**********/

/************************************************
*
*  DeallocLine(BufStruct *, int)
*
*  Performs a free.
*
**********/
void  DeallocLine(BufStruct *Storage, int rad);

/************************************************
*
*  Realloc(char *, int)
*
*  Performs a realloc.
*
**********/
char  *Realloc(char *mem, int size);
char  *Remalloc(char *mem, int size);

/************************************************
*
*  ReallocLine(SharedStruct *, int rad, int size)
*
*  Performs a realloc.
*
**********/
char  *ReallocLine(SharedStruct *shared, int rad, int size);

/***************************************************
*
*  CleanUp(BufStruct *)
*
*  Fyll igen alla hål i fileblocket.
*  Returnerar ett 'ret'värde.
*******/
int  CleanUp(BufStruct *Storage);

/***************************************************
*
*  CompactBuf(SharedStruct *, String *)
*
*  Lägger hela texten i en klump.
*  Ge en pekare för resultatet.  (if NULL, omorganisera valda buffern).
*  Returnerar ett ret-värde.
*
*******/
int  CompactBuf(SharedStruct *shared, String *retstring);

typedef struct {
  int size;
  UWORD lock;
#ifdef alloctest
  char *Next;
  char *Prev;
  int check;
  char *file;
  int line;
  int number;
  int premungwall;
#endif
} AllocStruct;

#ifdef alloctest
typedef struct {
  int postmungwall;
} PostAllocStruct;

#define MUNGWALL_PREVALUE  0x65872abc
#define MUNGWALL_POSTVALUE 0x72fab1de

void  CheckAllocs(char *firstalloc);
#endif


#define ALLOC_CACHE_MAXSIZE 512
#define ALLOC_CACHE_LENGTH  16
#define ALLOC_CACHE_MAXSTORE 10
struct AllocCache {
  struct AllocCache *pekare;
};


void  FreeCache(void);

char  *FreeXMem(int size, char *order);

