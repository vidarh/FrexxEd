/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 *  fh_packets.c
 *
 *  Replies to all kinds of filehandler-packets.
 *
 *********/

#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/FPL.h>
#include <exec/memory.h>
#include <string.h>
#include <stdlib.h> /* for the atoi() */

#include "Buf.h"
#include "Block.h" /* for the String2Block() function */
#include "BufControl.h" /* for the FindBuffer() function */
#include "Edit.h"
#include "GetFile.h" /* for the GiveProtection() function */
#include "IDCMP.h"
#include "Limit.h"
#include "Strings.h" /* for the Sprintf() */
#include "Undo.h"
#include "FH_packets.h"

extern struct DeviceList *device;
extern struct MsgPort *filehandlerport;
extern DefaultStruct Default; /* the default values of a BufStruct */
extern char buffer[];
extern long fh_locks;
extern long fh_opens;
extern char filehandlerstop; /* prevent new Lock() or Open() */

static __regargs struct SharedStruct *FindBufferBSTR(long lock, long bstrname);

static __regargs char *findcolon(char *source, int len)
{
  while(len-- && *source != ':')
    source++;
  return *source==':'?source:NULL;
}

/***************************************************************************
 ** Findfile()
 **
 ** Returns SharedStruct of the specified file
 **
 ********/
static __regargs struct SharedStruct *Findfile (char *p, int len)
{
  unsigned char *point[2];
  struct BufStruct *b;
  int fileno=1;
  point[0] = buffer;
  point[1] = (char *)0; /* always search for file 1 at first! */
  len = len<FILESIZE-1?len:FILESIZE-1;
  strncpy(buffer, p, len);
  buffer[len]=0;
  b = FindBuffer(2, point);
  if(!b) {
    char *comma = strrchr(buffer, ',');
    if(!strcmp(buffer, NOFILENAME)) {
      buffer[0]=0; /* search for empty string */
      if(b = FindBuffer(2, point))
        return b->shared;
    }
    if(comma) {
      fileno=fplStrtol(comma+1, 10, NULL);
      *comma = '\0'; /* zero terminate to find proper name! */
      if(fileno > 1) {
        point[1] = (char *) fileno;
        b = FindBuffer(2, point);
        if(!b && !strcmp(buffer, NOFILENAME)) {
          buffer[0] = 0;  /* search for empty string */
          b = FindBuffer(2, point);
        }
        if(b && b->shared->name_number!=fileno)
          b = NULL;
      }
    }
  }
  return b?b->shared:NULL;
}

/***************************************************************************
 ** IsRootLock()
 **
 ** Returns TRUE if the FileLock was a lock to the F: root node.
 **
 ********/
static __regargs BOOL IsRootLock(struct FileLock *l)
{
  struct FrexxLock *fl;
  fl = (struct FrexxLock *)l->fl_Key;
  return (BOOL)((fl->Buffer == NULL)?TRUE:FALSE);
}

/***************************************************************************
 ** AllocFrexxLock()
 **
 ** Allocates a FrexxLock structure and initializes it.
 **
 ********/
static __regargs __inline struct FrexxLock *AllocFrexxLock(struct SharedStruct *Buffer)
{
  struct FrexxLock *fl;
  if (fl = AllocMem(sizeof(struct FrexxLock), MEMF_PUBLIC)) {
    fl->Buffer = Buffer;
    fl->Next = NULL;
  }
  return fl;
}

/***************************************************************************
 ** FreeFrexxLock()
 **
 ** Frees a FrexxLock structure.
 **
 ********/
static __regargs __inline void FreeFrexxLock(struct FrexxLock *fl)
{
  if(fl->Buffer) {
    fl->Buffer->locked--;
  }
  FreeMem(fl, sizeof(struct FrexxLock));
}

/***************************************************************************
 ** AllocLock()
 **
 ** Allocates a FileLock and a FrexxLock structure and fills them in.
 **
 ********/
static __regargs struct FileLock *AllocLock(struct SharedStruct *Buf, LONG Access)
{
   struct FileLock *fl;

   if (!(fl = AllocMem(sizeof(struct FileLock), MEMF_PUBLIC | MEMF_CLEAR)))
   {
      return (NULL);
   }

   if (!(fl->fl_Key = (LONG) AllocFrexxLock(Buf)))
   {
      FreeMem(fl, sizeof(struct FileLock));

      return (NULL);
   }

   fl->fl_Access = Access;
   fl->fl_Task = filehandlerport;
   fl->fl_Volume = MKBADDR(device);
   return (fl);
}

/***************************************************************************
 ** FreeLock()
 **
 ** Frees a FileLock structure with its accompanying FrexxLock structure.
 **
 ********/
static __regargs __inline void FreeLock(struct FileLock *fl)
{
   FreeFrexxLock((struct FrexxLock *)fl->fl_Key);
   FreeMem(fl, sizeof(struct FileLock));
}

/***************************************************************************
 ** CopyLock()
 **
 ** Makes an exact copy of a FileLock structure, including the FrexxLock.
 **
 ********/
static __regargs __inline struct FileLock *CopyLock(struct FileLock *fl)
{
   struct FileLock *fl2;
   struct FrexxLock *l, *l2;

   l = (struct FrexxLock *)fl->fl_Key;
   if(fl2 = AllocLock(l->Buffer, fl->fl_Access)) {
     l2 = (struct FrexxLock *)fl2->fl_Key;
     l2->Next = l->Next;
     if(l2->Buffer)
       l2->Buffer->locked++; /* lock this once more! */
   }
   return fl2;
}

/***************************************************************************
 ** GetBufferFromLock()
 **
 ** Returns a pointer to the buffer associated with a FileLock.
 **
 ********/
static __regargs struct SharedStruct *GetBufferFromLock(struct FileLock *fl)
{
   struct FrexxLock *l;

   l = (struct FrexxLock *)fl->fl_Key;
   return (l->Buffer);
}

/***************************************************************************
 ** GetNextBufferFromLock()
 **
 ** Returns a pointer to the next buffer to be examined (only for root locks).
 ********/
static __regargs __inline struct SharedStruct *GetNextBufferFromLock(struct FileLock *fl)
{
   struct FrexxLock *l;

   l = (struct FrexxLock *)fl->fl_Key;
   while(l->Next && (l->Next->type & type_HIDDEN))
     l->Next = l->Next->Next;
   return (l->Next);
}

/***************************************************************************
 ** InitForExNext()
 **
 ** Initializes the NextBuf pointer for ExamineNext() (only for root locks).
 **
 ********/
static __regargs __inline void InitForExNext(struct FileLock *fl)
{
   struct FrexxLock *l;

   l = (struct FrexxLock *)fl->fl_Key;
   l->Next = Default.SharedDefault.Next;
}

/***************************************************************************
 ** StepToNextBuffer()
 **
 ** Updates the NextBuf pointer to the next buffer (only for root locks).
 **
 ********/
static __regargs __inline void StepToNextBuffer(struct FileLock *fl)
{
   struct FrexxLock *l;

   l = (struct FrexxLock *)fl->fl_Key;
   do {
     l->Next = l->Next->Next;
   } while ( l->Next && (l->Next->type & type_HIDDEN));
}

/***************************************************************************
 ** LockBuffer()
 **
 ** Searches for a buffer and creates a FileLock to it if it exists.
 **
 ********/
static __regargs __inline struct FileLock *LockBuffer(BPTR DirLock, BSTR Name, LONG Mode)
{
   int x;
   char *p;
   LONG len;
   struct FileLock *dl = NULL;
   struct FileLock *fl = NULL;
   struct SharedStruct *b;

   p = (char *)BADDR(Name) + 1;
   len = *(char *)BADDR(Name);

   if (dl = BADDR(DirLock)) {	/* Is there a Directory Lock? */
      if (!IsRootLock(dl)) {	/* Really a FrexxEd Root Lock? */
	 return NULL;		/* No, return NULL */
      }
   }
   else {
     for (x = 0; x < len; x++) {
       if (p[x] == ':') {       /* Is there a ':' in the path? */
         p = p + x + 1;	        /* Then set p past it */
         len = len - x - 1;	/* Adjust string length */
         break;                 /* And break the loop */
       }
     }
   }

   if (len == 0) {		/* Did he want a lock on FREXXED: alone? */
     fl = AllocLock(NULL, Mode);
   }
   else {
     if (b = Findfile(p, len) ) {
       b->locked++;
       fl = AllocLock(b, Mode);
     }
   }
   return fl;
}


static __regargs __inline struct FrexxHandle *AllocFrexxHandle(BPTR DirLock, BSTR Name, BOOL update)
{
   char *p;
   LONG len;
   struct SharedStruct *b;
   struct FileLock *dl = NULL;
   struct FrexxHandle *fh = NULL;

   p = (char *)BADDR(Name) + 1;
   len = *(char *)BADDR(Name);

   if (dl = BADDR(DirLock)) {	/* Is there a Directory Lock? */
      if (!IsRootLock(dl)) {	/* Really a FrexxEd Root Lock? */
	 return (NULL);		/* No, return NULL */
      }
   }

   b = FindBufferBSTR(DirLock, Name);
   if (b || update) {
     if(update && !b) {
       struct BufStruct *bs = MakeNewBuf(NULL);
       if(bs) {
         b = bs->shared;
         Split(b, p);
         CheckName(bs, FALSE, TRUE, NULL);
         b->fileprotection = 0;
         GiveProtection(0, b->protection);
       }
     }
     if (b && (fh = AllocMem(sizeof(struct FrexxHandle), MEMF_PUBLIC))) {
       fh->Buffer = b;
       fh->Line = 1;
       fh->Byte = 0;
       fh->Pos = 0;
       fh->Len = b->size;
       fh->write = update; /* if 'update', then allow writing! */
       b->locked++;  /* lock it for our use! */
     }
   }
   return fh;
}


static __regargs struct SharedStruct *FindBufferBSTR(long lock, long bstrname)
{
  char *p2;
  char *p = (char *)BADDR(bstrname) + 1;
  int len = *(char *)BADDR(bstrname);

  if(p2 = findcolon(p, len)) /* Set p to the first character after */
  {			     /* a ':', if any is present in the path */
     len -= p2-p+1;
     p = p2 + 1;
  }

  return len?Findfile(p, len):NULL;
}

__regargs long ReplyToPacket(struct DosPacket *Packet,
                             struct DeviceList *dl,
                             long *res1,
                             long *res2,
                             long out)
{
  int msgargs[2];
  struct FileLock *lock;
  struct SharedStruct *b;

  switch (Packet->dp_Type) {
  case ACTION_IS_FILESYSTEM:
    *res1 = DOSTRUE;
    *res2 = 0;
    break;

  case ACTION_LOCATE_OBJECT:
/***********************************************************************
 **	ACTION_LOCATE_OBJECT	Lock(...)
 **
 **	ARG1:	LOCK -	Lock on directory to which ARG2 is relative
 **	ARG2:	BSTR -	Name (possibly with a path) of object to lock
 **	ARG3:	LONG -	Mode: ACCESS_READ/SHARED_LOCK or
 **			ACCESS_WRITE/EXCLUSIVE_LOCK
 **
 **	RES1:	LOCK -	Lock on requested object or 0 to indicate failure
 **	RES2:	CODE -	Failure code if RES1 = 0
 ***********************************************************************/
    DEB(FPrintf(out, " Directory: %lx, ", BADDR(Packet->dp_Arg1)));
    DEB(FPrintf(out, " Name: \"%b\", ", Packet->dp_Arg2));
    DEB(FPrintf(out, " Mode: %ld %s\n", Packet->dp_Arg3, (Packet->dp_Arg3 == -1) ? "SHARED_LOCK/ACCESS_READ" : "EXCLUSIVE_LOCK/ACCESS_WRITE"));

    lock = filehandlerstop? NULL:
      LockBuffer(Packet->dp_Arg1,
                 (BSTR)Packet->dp_Arg2,
                 Packet->dp_Arg3);
    if(lock) {
      DEB(FPrintf(out, " Lock at %xl\n", lock));
      *res1 = MKBADDR(lock);
      *res2 = 0;
      fh_locks++;
    }
    else {
      *res1 = DOSFALSE;
      *res2 = ERROR_OBJECT_NOT_FOUND;
    }
    break;

  case ACTION_FREE_LOCK:
/***********************************************************************
 **	ACTION_FREE_LOCK	UnLock(...)
 **
 **	ARG1:	LOCK -	Lock to free
 **
 **	RES1:	BOOL -	DOSTRUE
 ***********************************************************************/
    FreeLock((struct FileLock *)BADDR(Packet->dp_Arg1));
    DEB(FPrintf(out, " Freed lock at %xl\n", BADDR(Packet->dp_Arg1)));
    *res1 = DOSTRUE;
    *res2 = 0;
    fh_locks--;
    break;

/***********************************************************************
 **	ACTION_EXAMINE_OBJECT	Examine(...)
 **
 **	ARG1:	LOCK -	Lock on object to examine
 **	ARG2:	BPTR -	FileInfoBlock to fill in
 **
 **	RES1:	BOOL -	Success/Failure (DOSTRUE/DOSFALSE)
 **	RES2:	CODE -	Failure code if RES1 = DOSFALSE
 ***********************************************************************/
  case ACTION_EXAMINE_OBJECT:
    {
      struct FileInfoBlock *f;
      struct FileLock *fl;
   
      fl = (struct FileLock *)BADDR(Packet->dp_Arg1);
      DEB(FPrintf(out, " Lock: %lx\n", fl));
   
      f = (struct FileInfoBlock *)BADDR(Packet->dp_Arg2);
      DEB(FPrintf(out, " FileInfoBlock: %lx\n", f));
      f->fib_DiskKey = 0;
   
      if (IsRootLock(fl))
      {
         f->fib_DirEntryType = ST_USERDIR;
         strcpy(f->fib_FileName, BADDR(device->dl_Name));
         f->fib_Protection = 0;
         f->fib_EntryType = f->fib_DirEntryType;
         f->fib_Size = 0;
         f->fib_NumBlocks = 0;
         f->fib_Date = device->dl_VolumeDate;
         strcpy(f->fib_Comment, "FrexxEd root");
         /* Initialize NextBuf pointer for ExamineNext() */
         InitForExNext(fl);
      }
      else
      {
         char *name;
   
         b = GetBufferFromLock(fl);
         f->fib_DirEntryType = ST_FILE;
         name = b->filnamn[0]?b->filnamn:NOFILENAME;
         if(b->name_number>1 && strlen(name)<31) {
           Sprintf(f->fib_FileName + 1, "%s,%ld",
                   name, b->name_number);
         }
         else
           strcpy(f->fib_FileName + 1, name);
         f->fib_FileName[0] = strlen(f->fib_FileName + 1);
         f->fib_Protection = b->fileprotection;
         f->fib_EntryType = f->fib_DirEntryType;
         f->fib_Size = b->size;
         f->fib_NumBlocks = f->fib_Size;
         f->fib_Date = b->date;
         strcpy(f->fib_Comment + 1, b->filecomment);
         f->fib_Comment[0] = strlen(f->fib_Comment + 1);
      }
      *res1 = DOSTRUE;
      *res2 = 0;
    }
    break;

  case ACTION_FINDINPUT:
  case ACTION_FINDUPDATE:
/***********************************************************************
 **	ACTION_FINDINPUT	Open(..., MODE_OLDFILE)
 **
 **	ARG1:	BPTR -	FileHandle to fill in
 **	ARG2:	LOCK -	Lock to directory that ARG3 is relative to
 **	ARG3:	BSTR -	Name of file to be opened (relative to ARG2)
 **
 **	RES1:	BOOL -	Success/Failure (DOSTRUE/DOSFALSE)
 **	RES2:	CODE -	Failure code if RES1 = DOSFALSE
 ***********************************************************************/
   {
     struct FileHandle *fh;
     struct FrexxHandle *f;
  
     fh = BADDR(Packet->dp_Arg1);
     DEB(FPrintf(out, " File Handle: $%lx\n", fh));
     DEB(FPrintf(out, " Directory: $%lx\n", BADDR(Packet->dp_Arg2)));
     DEB(FPrintf(out, " File Name: %b\n", Packet->dp_Arg3));
  
     fh->fh_Type = filehandlerport;
     fh->fh_Arg1 = 0;
  
     if (f = filehandlerstop?NULL:
           AllocFrexxHandle(Packet->dp_Arg2, Packet->dp_Arg3,
                            ACTION_FINDUPDATE == Packet->dp_Type)) {
       fh->fh_Arg1 = (LONG) f;
       *res1 = DOSTRUE;
       *res2 = 0;
       fh_opens++;
       msgargs[0]=(int)f->Buffer;
       msgargs[1]=EXCEPTION_TYPE_OPEN;
       SendReturnMsg(cmd_DEVICE|cmd_IMPORTANT, OK, NULL, EXECUTED_HANDLER, msgargs);
     }
     else {
       *res1 = DOSFALSE;
       *res2 = ERROR_OBJECT_NOT_FOUND;
     }
   }
   break;

  case ACTION_DISK_INFO:
/***********************************************************************
 **	ACTION_DISK_INFO	Info(...)
 **
 **	ARG1:	BPTR -	Pointer to an InfoData structure to fill in
 **
 **	RES1:	BOOL -	Success/Failure (DOSTRUE/DOSFALSE)
 ***********************************************************************/
  case ACTION_INFO:
/***********************************************************************
 **	ACTION_INFO	<sendpkt only>
 **
 **	ARG1:	LOCK -	Lock on volume
 **	ARG2:	BPTR -	Pointer to an InfoData structure to fill in
 **
 **	RES1:	BOOL -	Success/Failure (DOSTRUE/DOSFALSE)
 ***********************************************************************/
    {
      struct InfoData *i;
      long size=0;
      b = Default.SharedDefault.Next;
      while(b) {
        size += b->size;
        b = b->Next;
      }

      if(Packet->dp_Type == ACTION_DISK_INFO) {   
        i = (struct InfoData *)BADDR(Packet->dp_Arg1);
        DEB(FPrintf(out, " Replied to DISK_INFO\n"));
      }
      else {
        i = (struct InfoData *)BADDR(Packet->dp_Arg2);
        DEB(FPrintf(out, " Replied to INFO\n"));
      }
      i->id_NumSoftErrors=0;
      i->id_UnitNumber = 1;
      i->id_DiskState = ID_VALIDATED;
      i->id_NumBlocks = size?size:1; /* never return 0 sized disk! */
      i->id_NumBlocksUsed = size;
      i->id_BytesPerBlock = 1;
      i->id_DiskType = ID_FFS_DISK;
      i->id_VolumeNode = MKBADDR(device);
      i->id_InUse = 0;
      *res1 = DOSTRUE;
      *res2 = 0;
    }
    break;

  case ACTION_EXAMINE_NEXT:
/***********************************************************************
 **	ACTION_EXAMINE_NEXT	ExNext(...)
 **
 **	ARG1:	LOCK -	Lock on directory being examined
 **	ARG2:	BPTR -	FileInfoBlock to fill in
 **
 **	RES1:	Success/Failure (DOSTRUE/DOSFALSE)
 **	RES2:	Failure code if RES1 = DOSFALSE
 ***********************************************************************/
    {
      struct FileInfoBlock *f;
      struct FileLock *fl;
   
      fl = (struct FileLock *)BADDR(Packet->dp_Arg1);
      DEB(FPrintf(out, " Lock: %lx\n", fl));
   
      /* Find the node structure */
      b = GetNextBufferFromLock(fl);
   
      if (b)
      {
         char *name;
         DEB(FPrintf(out, " Found buffer: %s\n", b->filnamn));
         f = (struct FileInfoBlock *)BADDR(Packet->dp_Arg2);
         f->fib_DiskKey = 0;
         f->fib_DirEntryType = ST_FILE;
         name = b->filnamn[0]?b->filnamn:NOFILENAME;
         if(b->name_number>1 && strlen(name)<31) {
           Sprintf(f->fib_FileName + 1, "%s,%ld",
                   name, b->name_number);
         }
         else
           strcpy(f->fib_FileName + 1, name);
         f->fib_FileName[0] = strlen(f->fib_FileName + 1);
         f->fib_Protection = b->fileprotection;
         f->fib_EntryType = f->fib_DirEntryType;
         f->fib_Size = b->size;
         f->fib_NumBlocks = f->fib_Size;
         f->fib_Date = b->date;
         strcpy(f->fib_Comment + 1, b->filecomment);
         f->fib_Comment[0] = strlen(f->fib_Comment + 1);
         StepToNextBuffer(fl);
         *res1 = DOSTRUE;
         *res2 = 0;
      }
      else
      {
         *res1 = DOSFALSE;
         *res2 = ERROR_NO_MORE_ENTRIES;
      }
    }
    break;

  case ACTION_COPY_DIR:
/***********************************************************************
 **	ACTION_COPY_DIR		DupLock(...)
 **
 **	ARG1:	LOCK -	Lock to duplicate
 **
 **	RES1:	LOCK -	Duplicated lock or 0 to indicate failure
 **	RES2:	CODE -	Failure code if RES1 = 0
 ***********************************************************************/
    if(!filehandlerstop) {
      struct FileLock *l, *l2;
    
      l2 = (struct FileLock *)BADDR(Packet->dp_Arg1);
      if(l = CopyLock(l2)) {
        DEB(FPrintf(out, " Returning copy $%lx of lock $%lx\n", l, l2));
        *res1 = MKBADDR(l);
        *res2 = 0;
        fh_locks++;
      }
      else {
        *res1 = DOSFALSE;
        *res2 = ERROR_DISK_FULL;
      }
    }
    else {
      *res1 = DOSFALSE;
      *res2 = ERROR_OBJECT_NOT_FOUND;
    }
    break;

  case ACTION_SAME_LOCK:
/***********************************************************************
 **	ACTION_SAME_LOCK    SameLock(...)
 **
 **	ARG1:	LOCK -	Lock to check
 **     ARG2:   LOCK -  Lock to check
 **
 **	RES1:	LONG -	LOCK_SAME / LOCK_SAME_HANDLER / LOCK_DIFFERENT
 **	RES2:	CODE -	Failure code if RES1 = LOCK_DIFFERENT
 ***********************************************************************/
    {
       struct FileLock *l1, *l2;
    
       l1 = (struct FileLock *)BADDR(Packet->dp_Arg1);
       l2 = (struct FileLock *)BADDR(Packet->dp_Arg2);
       DEB(FPrintf(out, " Compares copy $%lx with lock $%lx\n", l1, l2));

       if(l1->fl_Task == l2->fl_Task &&
          l1->fl_Task == filehandlerport) {
         *res2 = 0; /* no error code */
         *res1 = LOCK_SAME_HANDLER;
         if( GetBufferFromLock(l1) == GetBufferFromLock(l2) ) {
           *res1 = LOCK_SAME;
           DEB(FPrintf(out, " SAME!!!\n"));
         }
#if DEBUG_PACKETS>0
         else {
           DEB(FPrintf(out, " Same handler!\n"));
         }
#endif
       }
       else {
         *res1 = LOCK_DIFFERENT;
         *res2 = ERROR_OBJECT_NOT_FOUND;
         DEB(FPrintf(out, " Different!\n"));
       }
    }
    break;

  case ACTION_READ:  
/***********************************************************************
 **	ACTION_READ	Read(...)
 **
 **	ARG1:	ARG1 -	fh_Arg1 field of opened FileHandle
 **	ARG2:	APTR -	Buffer to put data into
 **	ARG3:	LONG -	Number of bytes to read
 **
 **	RES1:	LONG -	Number of bytes read. o indicates EOF.
 **			-1 indicates ERROR
 **	RES2:	CODE -	Failure code if RES1 = -1
 ***********************************************************************/
    {
      struct FrexxHandle *fh;
      char *p;
      int x, len;
   
      fh = (struct FrexxHandle *)Packet->dp_Arg1;
      b = fh->Buffer;
      p = (char *)Packet->dp_Arg2;
      len = Packet->dp_Arg3;
   
      DEB(FPrintf(out, " File length: %ld\n", fh->Len));
      if (fh->Pos == fh->Len)
      {
         *res1 = *res2 = 0;
      }
      else
      {
         DEB(FPrintf(out, " Buffer: %s\n", b->filnamn));
         DEB(FPrintf(out, " Target: $%lx\n", p));
         DEB(FPrintf(out, " Length: %ld\n", len));
         for (x = 0; (x < len) && (fh->Pos < fh->Len); x++)
         {
           p[x] = b->text[fh->Line].text[fh->Byte];
           if(++fh->Byte >= b->text[fh->Line].length) {
             fh->Byte=0;
             fh->Line++;
           }
           fh->Pos++;
         }
         DEB(FPrintf(out, " Bytes delivered (and return code): %ld\n", x));
         *res1 = x;
         *res2 = 0;
#if DEBUG_PACKETS>0
         {
           int a;
           DEB(FPrintf(out, " The first %ld bytes:\n '", x>70?70:x));
           for (a = 0; (a < x) && (a<70); a++)
           {
             DEB(FPrintf(out, "%lc", (int)p[a]));
           }
           DEB(FPrintf(out, "'\n"));
         }
#endif
      }
      msgargs[0]=(int)b;
      msgargs[1]=EXCEPTION_TYPE_READ;
      SendReturnMsg(cmd_DEVICE|cmd_EGO|cmd_IMPORTANT, OK, NULL, EXECUTED_HANDLER, msgargs);
    }
    break;

  case ACTION_SEEK:
/***********************************************************************
 **	ACTION_SEEK	Seek(...)
 **
 **	ARG1:	ARG1 -	fh_Arg1 field of the opened FileHandle
 **	ARG2:	LONG -	New position
 **	ARG3:	LONG -	Mode: OFFSET_BEGINNING, OFFSET_END, or
 **			OFFSET_CURRENT
 **
 **	RES1:	LONG -	Old position. -1 indicates an error
 **	RES2:	CODE -	Failure code if RES1 = -1
 ***********************************************************************/
    {
      struct FrexxHandle *fh;
      LONG Pos;
      LONG Mode;
      LONG OldPos;
   
      fh = (struct FrexxHandle *)Packet->dp_Arg1;
      Pos = (LONG) Packet->dp_Arg2;
      Mode = (LONG) Packet->dp_Arg3;
      DEB(FPrintf(out, " Pos: %ld\n", Pos));
      DEB(FPrintf(out, " Mode: %s\n",
                  Mode==OFFSET_BEGINNING?"OFFSET_BEGINNING":
                  Mode==OFFSET_CURRENT?"OFFSET_CURRENT":
                  Mode==OFFSET_END?"OFFSET_END":"_unknown_"));
   
      DEB(FPrintf(out, " Current position: %ld\n", fh->Pos));
      DEB(FPrintf(out, " Current file size: %ld\n", fh->Len));

      OldPos = fh->Pos;
      switch(Mode) {
        case OFFSET_BEGINNING:
          fh->Pos = Pos;
          break;
        case OFFSET_CURRENT:
          fh->Pos += Pos;
          break;
        case OFFSET_END:
          fh->Pos = fh->Len-Pos;
          break;
      }

      if(fh->Pos> fh->Len ||
         fh->Pos < 0) {
        /* ERROR! */
        if(fh->Pos < 0)
          fh->Pos = 0;
        else
          fh->Pos = fh->Len;
        *res1 = -1;
        *res2 = ERROR_SEEK_ERROR;
      }
      else {
        *res1 = OldPos;
        *res2 = 0;
        Pos2LineByte(fh->Buffer, fh->Pos, &fh->Line, &fh->Byte);
      }
      DEB(FPrintf(out, " Returns %ld\n", *res1));
    }
    break;

  case ACTION_WRITE:
/***********************************************************************
 **	ACTION_WRITE	Write(...)
 **
 **	ARG1:	ARG1 -	fh_Arg1 field of opened FileHandle
 **	ARG2:	APTR -	Buffer with data to write
 **	ARG3:	LONG -	Number of bytes to write
 **
 **	RES1:	LONG -	Number of bytes written
 **			-1 indicates ERROR
 **	RES2:	CODE -	Failure code if RES1 != ARG3
 ***********************************************************************/
    {
      struct FrexxHandle *fh;
      char *p;
      int len;
      int ret;
   
      fh = (struct FrexxHandle *)Packet->dp_Arg1;
      b = fh->Buffer;
      p = (char *)Packet->dp_Arg2;
      len = Packet->dp_Arg3;
   
      DEB(FPrintf(out, " File original length: %ld\n", fh->Len));
      DEB(FPrintf(out, " File pos: %ld\n", fh->Pos));
      DEB(FPrintf(out, " File name: %s\n", b->filnamn));
      DEB(FPrintf(out, " Data Length: %ld\n", len));

#if DEBUG_PACKETS>0
      {
        int a;
        DEB(FPrintf(out, " The first %ld bytes:\n '", len>70?70:len));
        for (a = 0; (a < len) && (a<70); a++)
        {
          DEB(FPrintf(out, "%lc", (int)p[a]));
        }
        DEB(FPrintf(out, "'\n"));
      }
#endif

#if 0
      if(!fh->write)
        ret=WRITE_PROTECTED;
      else {
#endif
        if (fh->Pos==fh->Len)
          ret=String2Block((BlockStruct *)b, p, len, 1); /* append == TRUE */
        else
          ret=InsertText(b->Entry, p, len, fh->Pos); /* insert */
#if 0
      }
#endif
      switch(ret) {
      case WRITE_PROTECTED:
        *res1 = -1;
        *res2 = ERROR_WRITE_PROTECTED;
        DEB(FPrintf(out, " ERROR_WRITE_PROTECTED\n"));
        break;
      /* case OUT_OF_MEM: */
      default:
        if (ret<OK) {
          *res1 = -1;
          *res2 = ERROR_NO_FREE_STORE;
          DEB(FPrintf(out, " ERROR_NO_FREE_STORE\n"));            
          break;
        } /* falling down */
      case OK: /* OK */
        *res2 = 0;
        *res1 = len; /* everything is OK */
        fh->Pos += len; /* move our "position" */
        fh->Len = b->size; /* get the size of the file! */
        Pos2LineByte(fh->Buffer, fh->Pos, &fh->Line, &fh->Byte);
        DateStamp(&b->date);
        msgargs[0]=(int)b;
        msgargs[1]=EXCEPTION_TYPE_WRITE;
        SendReturnMsg(cmd_DEVICE|cmd_EGO|cmd_IMPORTANT, OK, NULL, EXECUTED_HANDLER, msgargs);
        break;
      }
    }
    break;

  case ACTION_SET_PROTECT:
/***********************************************************************
 **	ACTION_SET_PROTECT
 **
 **	ARG1:	ARG1 -	Unused
 **	ARG2:	LOCK -	Lock to which ARG3 is relative to
 **	ARG3:	BSTR -	bstring of the object name
 **     ARG4:   LONG -  Protection 32-bits
 **
 **	RES1:	BOOL -	DOSTRUE/DOSFALSE
 **	RES2:	CODE -	Failure code if RES1 = DOSFALSE
 ***********************************************************************/
    b = FindBufferBSTR(Packet->dp_Arg2,  /* LOCK */
                       Packet->dp_Arg3); /* BSTR filename */
    if(b) {
      long protection = Packet->dp_Arg4;

      DEB(FPrintf(out, " File: \"%s\"\n", b->filnamn));
      DEB(FPrintf(out, " New bits: %lx\n", protection));

      b->fileprotection = protection;
      GiveProtection(protection, b->protection);
      *res1 = DOSTRUE;
      *res2 = 0;
    }
    else {
      *res1 = DOSFALSE;
      *res2 = 0;
    }
    break;

  case ACTION_SET_COMMENT:
/***********************************************************************
 **	ACTION_SET_COMMENT
 **
 **	ARG1:	ARG1 -	Unused
 **	ARG2:	LOCK -	Lock to which ARG3 is relative to
 **	ARG3:	BSTR -	bstring of the object name
 **     ARG4:   BSTR -  Comment (max 79 characters)
 **
 **	RES1:	BOOL -	DOSTRUE/DOSFALSE
 **	RES2:	CODE -	Failure code if RES1 = DOSFALSE
 ***********************************************************************/
    {
      b = FindBufferBSTR(Packet->dp_Arg2,  /* LOCK */
                         Packet->dp_Arg3); /* BSTR filename */
      if(b) {
        char *comment = (char *)BADDR(Packet->dp_Arg4) + 1;
        int cmntlen = *(char *)BADDR(Packet->dp_Arg4);

        cmntlen = cmntlen>79?79:cmntlen;

        DEB(FPrintf(out, " File: \"%s\"\n", b->filnamn));
        DEB(FPrintf(out, " New comment: '%b'\n", Packet->dp_Arg4));

        memcpy(b->filecomment, comment, cmntlen);
        b->filecomment[cmntlen] = 0;
        *res1 = DOSTRUE;
        *res2 = 0;
      }
      else {
        DEB(FPrintf(out, " No such file!\n"));
        *res1 = DOSFALSE;
        *res2 = ERROR_OBJECT_NOT_FOUND;
      }
    }
    break;

  case ACTION_SET_DATE:
/***********************************************************************
 **	ACTION_SET_DATE
 **
 **	ARG1:	ARG1 -	Unused
 **	ARG2:	LOCK -	Lock to which ARG3 is relative to
 **	ARG3:	BSTR -	bstring of the object name
 **	ARG4:	DateStamp pointer
 **
 **	RES1:	BOOL -	DOSTRUE/DOSFALSE
 **	RES2:	CODE -	Failure code if RES1 = DOSFALSE
 ***********************************************************************/
    {
      b = FindBufferBSTR(Packet->dp_Arg2,  /* LOCK */
                         Packet->dp_Arg3); /* BSTR filename */
      if(b) {
        struct DateStamp *ds = (struct DateStamp *)Packet->dp_Arg4;

        DEB(FPrintf(out, " File: \"%b\"\n", Packet->dp_Arg3));
        DEB(FPrintf(out, " Days/minute/tick: %ld-%ld-%ld\n",
                    ds->ds_Days, ds->ds_Minute, ds->ds_Tick));

        b->date = *ds;
        *res1 = DOSTRUE;
        *res2 = 0;
      }
      else {
        *res1 = DOSFALSE;
        *res2 = ERROR_WRITE_PROTECTED;
      }
    }
    break;

  case ACTION_FINDOUTPUT:
/***********************************************************************
 **	ACTION_FINDOUTPUT	Open(..., MODE_NEWFILE)
 **
 **	ARG1:	BPTR -	FileHandle to fill in
 **	ARG2:	LOCK -	Lock to directory that ARG3 is relative to
 **	ARG3:	BSTR -	Name of file to be opened (relative to ARG2)
 **
 **	RES1:	BOOL -	Success/Failure (DOSTRUE/DOSFALSE)
 **	RES2:	CODE -	Failure code if RES1 = DOSFALSE
 ***********************************************************************/
   if(!filehandlerstop) {
     struct FileHandle *fh;
     struct FrexxHandle *f;
  
     b = FindBufferBSTR(Packet->dp_Arg2,  /* LOCK */
                        Packet->dp_Arg3); /* BSTR filename */

     fh = BADDR(Packet->dp_Arg1);
     DEB(FPrintf(out, " File Handle: $%lx\n", fh));
     DEB(FPrintf(out, " Directory: $%lx\n", BADDR(Packet->dp_Arg2)));
     DEB(FPrintf(out, " File name: %b\n", Packet->dp_Arg3));
  
     fh->fh_Type = filehandlerport;
     fh->fh_Arg1 = 0;

     *res2 = ERROR_OBJECT_IN_USE;
     if(b) {
       if (!b->changes) {
         FreeUndoLines(b, b->Undotop+1);
         Clear(b->Entry, FALSE);
         msgargs[0]=(int)b;
         msgargs[1]=EXCEPTION_TYPE_OPEN;
         SendReturnMsg(cmd_DEVICE|cmd_EGO|cmd_IMPORTANT, OK, NULL, EXECUTED_HANDLER, msgargs);
       } else
         b=NULL;
     }
     else {
       struct BufStruct *bs = MakeNewBuf(NULL);
       if(bs) {
         char *p2, *p = (char *)BADDR(Packet->dp_Arg3) + 1;
         long len = *(char *)BADDR(Packet->dp_Arg3);
         if(p2 = findcolon(p, len)) { /* Set p to the first character after */
                                      /* a ':', if any is present in the path */
           len -= p2-p+1;
           p = p2 + 1;
         }
         memcpy(buffer, p, len);
         buffer[len]=0;
         b = bs->shared;
         Split(b, buffer);
         CheckName(bs, FALSE, TRUE, NULL);
         b->fileprotection = 0;
         GiveProtection(0, b->protection);
         msgargs[0]=(int)b;
         msgargs[1]=EXCEPTION_TYPE_CREATE;
         SendReturnMsg(cmd_DEVICE|cmd_IMPORTANT, OK, NULL, EXECUTED_HANDLER, msgargs);
       }
       else
         *res2 = ERROR_DISK_FULL; /* kind of anyway! ;) */
     }

     if(b && (f = AllocMem(sizeof(struct FrexxHandle), MEMF_PUBLIC))) {
       f->Buffer = b;
       f->Line = 1;
       f->Byte = 0;
       f->Pos = 0;
       f->Len = 0;
       f->write = 1; /* allow writing to it! */

       b->locked++; /* lock it */

       fh->fh_Arg1 = (LONG) f;
       *res1 = DOSTRUE;
       *res2 = 0;

       DEB(FPrintf(out, " Opened new file!\n"));
       fh_opens++;
     }
     else {
       *res1 = DOSFALSE;
     }
   }
   else {
     *res1 = DOSFALSE;
     *res2 = ERROR_OBJECT_NOT_FOUND;
   }
   break;

  case ACTION_END:
/***********************************************************************
 **	ACTION_END	Close()
 **
 **	ARG1:	ARG1 -	FileHandle
 **
 **	RES1:	BOOL -	Success/Failure (DOSTRUE/DOSFALSE)
 **	RES2:	CODE -	Failure code if RES1 = DOSFALSE
 ***********************************************************************/
    {
      struct FrexxHandle *fh = (struct FrexxHandle *)Packet->dp_Arg1;

      fh->Buffer->locked--;

      DEB(FPrintf(out, " Closed file handle %lx!\n", fh));
      DEB(FPrintf(out, " Name: '%s'!\n", fh->Buffer->filnamn));

      msgargs[0]=(int)fh->Buffer;
      msgargs[1]=EXCEPTION_TYPE_CLOSE;
      SendReturnMsg(cmd_DEVICE|cmd_EGO|cmd_IMPORTANT, OK, NULL, EXECUTED_HANDLER, msgargs);

      FreeMem( fh, sizeof(struct FrexxHandle) );

      *res1 = DOSTRUE;
      *res2 = 0;
      fh_opens--;
    }
    break;

  case ACTION_RENAME_OBJECT:
/***********************************************************************
 **	ACTION_RENAME_OBJECT
 **
 **	ARG1:	LOCK -	Lock to which ARG2 is relative to
 **	ARG2:	BSTR -	name of the file (relative to ARG1)
 **	ARG3:	LOCK -	lock to dest dir
 **     ARG4:   BSTR -  bstring to file name
 **
 **	RES1:	BOOL -	Success/Failure (DOSTRUE/DOSFALSE)
 **	RES2:	CODE -	Failure code if RES1 = DOSFALSE
 ***********************************************************************/
    b = FindBufferBSTR(Packet->dp_Arg1,  /* LOCK */
                       Packet->dp_Arg2); /* BSTR filename */

    *res1 = DOSFALSE;
    *res2 = ERROR_OBJECT_NOT_FOUND;

    if(b) {
      struct SharedStruct *dest;

      DEB(FPrintf(out, " Source file : \"%s\"\n",
                  b->filnamn));

      dest = FindBufferBSTR(Packet->dp_Arg3,  /* LOCK */
                            Packet->dp_Arg4); /* BSTR filename */
      if(!dest) {
        /* destination file did not exist! */
        
        char *p2, *p = (char *)BADDR(Packet->dp_Arg4) + 1;
        long len = *(char *)BADDR(Packet->dp_Arg4);
        if(p2 = findcolon(p, len)) /* Set p to the first character after */
        {	                     /* a ':', if any is present in the path */
          len -= p2-p+1;
          p = p2 + 1;
        }

        strncpy(buffer, p, len);
        buffer[len] = 0;
        DEB(FPrintf(out, " Dest name: %s\n", buffer));
        Split(b, buffer);
        CheckName(b->Entry, FALSE, TRUE, NULL);
        *res1 = DOSTRUE;
        *res2 = 0;
      }
      else {
        *res2 = ERROR_OBJECT_EXISTS;
      }
    }
    break;

  case ACTION_DELETE_OBJECT:
/***********************************************************************
 **	ACTION_DELETE_OBJECT
 **
 **	ARG1:	LOCK -	Lock to which ARG3 is relative to
 **	ARG2:	BSTR -	bstring of the object name (possible dir)
 **
 **	RES1:	BOOL -	Success/Failure (DOSTRUE/DOSFALSE)
 **	RES2:	CODE -	Failure code if RES1 = DOSFALSE
 ***********************************************************************/
    b = FindBufferBSTR(Packet->dp_Arg1,  /* LOCK */
                       Packet->dp_Arg2); /* BSTR filename */
    if(b) {
      DEB(FPrintf(out, " Delete file : \"%s\"\n", b->filnamn));
      *res1 = DOSTRUE;
      *res2 = 0;
      if (!DeleteBuffer(b)) {
        *res1 = DOSFALSE;
        *res2 = ERROR_OBJECT_IN_USE;
        DEB(FPrintf(out, " File in use, can't delete!\n"));
      } else {
        msgargs[0]=(int)b;
        msgargs[1]=EXCEPTION_TYPE_DELETE;
        SendReturnMsg(cmd_DEVICE|cmd_IMPORTANT, OK, NULL, EXECUTED_HANDLER, msgargs);
        DEB(FPrintf(out, " File is deleted!\n"));
      }
    }
    else {
      *res1 = DOSFALSE;
      *res2 = ERROR_OBJECT_NOT_FOUND;

      DEB(FPrintf(out, " File \"%b\" was not found!\n", Packet->dp_Arg2));
    }
   break;

/***********************************************************************
 **	ACTION_PARENT	Parent(...)
 **
 **	ARG1:	LOCK -	Lock on object to get the parent of
 **
 **	RES1:	LOCK -	Parent lock
 **	RES2:	Failure code if RES1 = 0
 ***********************************************************************/
  case ACTION_PARENT:
    if(!filehandlerstop) {
       struct FileLock *l, *fl;
    
       l = (struct FileLock *)BADDR(Packet->dp_Arg1);
       DEB(FPrintf(out, " Lock: $%lx\n", l));
    
       if (IsRootLock(l))
       {
         DEB(FPrintf(out, " Already at root level, Returning NULL\n"));
         *res1 = NULL;
         *res2 = 0;
       }
       else
       {
          if (fl = AllocLock(NULL, l->fl_Access))
          {
            DEB(FPrintf(out, " Returning root lock: $%lx\n", fl));
    	    *res1 = MKBADDR(fl);
            *res2 = 0;
            fh_locks++;
          }
          else
          {
    	    DEB(FPrintf(out, "*** Out of memory!!!\n"));
    	    *res1 = 0;
            *res2 = ERROR_NO_FREE_STORE;
          }
       }
    }
    else {
      *res1 = DOSFALSE;
      *res2 = ERROR_OBJECT_NOT_FOUND;
    }
    break;

  default:
    *res1 = DOSFALSE;
    *res2 = ERROR_ACTION_NOT_KNOWN;
    break;
  }
  return 0; /* only zero at the moment */
}
