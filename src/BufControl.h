/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * BufControl.h
 *
 * Control routines for BufStruct
 *
 *********/

/***********************************************
 *
 *  NextShowBuf( BufStruct *)
 *  PrevShowBuf( BufStruct *)
 *  NextBuf( BufStruct *)
 *  NextBuf( BufStruct *)
 *  NextHiddenBuf( BufStruct *)
 *  PrevHiddenBuf( BufStruct *)
 *
 *  Returns the pointer for the next/prev shown/not shown buffer.
 *  Will not activate it!!!
 *  Input the buffer pointer.
 *
 ***********/
WindowStruct __regargs *NextWindow(WindowStruct *win);
WindowStruct __regargs *PrevWindow(WindowStruct *win);
BufStruct __regargs *NextView(BufStruct *Storage);
BufStruct __regargs *PrevView(BufStruct *Storage);
BufStruct __regargs *NextHiddenEntry(BufStruct *Storage);
BufStruct __regargs *PrevHiddenEntry(BufStruct *Storage);
BufStruct __regargs *NextEntry(BufStruct *Storage);
BufStruct __regargs *PrevEntry(BufStruct *Storage);
BufStruct __regargs *NextBuf(BufStruct *Storage);
BufStruct __regargs *PrevBuf(BufStruct *Storage);

void __regargs BufLimits(BufStruct *Storage);

/***********************************************
 *
 *    Activate(BufStruct *, BufStruct *, int)
 *
 * Activate chosen buffer.
 * Input: current buffer, wanted buffer.
 * Return the active BufStruct pointer.
 *
 ***************/
BufStruct __regargs *Activate(BufStruct *Storage, BufStruct *Storage3, int flag);

/***********************************************
 *
 *  Free( BufStruct *, int)
 *
 *  Deallocate a buffer.
 *  Input a pointer to the BufStruct to delete
 *  and if you want something in return.
 *  Return a new BufStruct to use.
 *
 ***********/
BufStruct __regargs *Free(BufStruct *Storage, BOOL returnwanted, BOOL hook);

void __regargs FreeLokalInfo(SharedStruct *shared);
/***********************************************
 *
 *  MakeNewBuf(BufStruct *)
 *
 *  Allocate a new buffer in the buffer list and activates it!
 *  Input the currunt BufStruct pointer.
 *  Returns the BufStruct pointer.
 *
 ***********/
BufStruct __regargs *MakeNewBuf(BufStruct *Storage3);

/***********************************************
 *
 *  Init( BufStruct *from,BufStruct *to)
 *
 *  Inits the buffer you desire using 'from-buffer' as default.
 *  Return OK/FALSE.
 ***********/
BOOL __regargs Init(BufStruct *Storage2, BufStruct *Storage);

/***********************************************
 *
 *  Clear( BufStruct *)
 *
 *  Clears the buffer you desire.
 *
 ***********/
void __regargs Clear(BufStruct *Storage, BOOL do_undo);

BufStruct __regargs *ChooseBuf(BufStruct *Storage, char *title, int type, int entries);

BufStruct __regargs *ReSizeBuf(BufStruct *CurrentStorage, BufStruct *Storage, BufStruct *NextHdnStorage, int lines);

BufStruct __regargs *RemoveBuf(BufStruct *Storage);

void __regargs FixMarg(BufStruct *Storage);

/*************************************************
*
*  BufStruct *DuplicateBuf(BufStruct *Storage)
*
*  Duplicerar önskad buffert och aktiverar den med
*  halva sizen av den nuvarande.
**********/
BufStruct __regargs *DuplicateBuf(BufStruct *Storage2);


/* Activate flags */
#define acREPLACE 0
#define acHALF_SIZE 1
#define acFULL_SIZE 2
#define acNEW_WINDOW 3

BufStruct __regargs *FindBuffer(int Argc, char **Argv);

void __regargs LockBuf_release_semaphore(SharedStruct *shared);
void __regargs UnLockBuf_obtain_semaphore(SharedStruct *shared);

BufStruct * __regargs CheckBufID(BufStruct *BufferID);
WindowStruct * __regargs CheckWinID(WindowStruct *winID);
WindowStruct *__regargs FindWindow(struct Window *window);
void __regargs HeadWindow(WindowStruct *win);

int __regargs CountDownFileNumber(SharedStruct *Shared);

BufStruct __regargs *GetNewBuf(BufStruct *Storage, BufStruct *Storage2, int argID, int type);

BufStruct __regargs *DeleteEntry(BufStruct *Storage, BOOL returnwanted);

int __regargs DeleteBuffer(SharedStruct *shared);

void __regargs AttachBufToWindow(WindowStruct *win, BufStruct *Storage);

