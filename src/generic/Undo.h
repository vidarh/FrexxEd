/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************************
*
*  Undo.h
*
**********/

#define undoNORMAL 1
#define undoAPPENDBEFORE 2
#define undoAPPENDAFTER 4
#define undoONLYBACKSPACES 8
#define undoONLYTEXT 16
#define undoNEWTEXT 32		// Byt ut hela texten

/************************************************
*
*  UndoAdd(BufStruct *, char *, int multiple, int len)
*
*  Add a string to the undo buffer of length 'int len'.
*  Multiple input=TRUE.
*  If multinput=undoONLYBACKSPACES then (char *) is the number of BS.
*  Don't forget to use undoONLYTEXT if possible.
*  Returns TRUE if everything was OK.
*********/
int __regargs UndoAdd(BufStruct *Storage, char *text, int multinput, int len);

/************************************************
*
*  UndoAddTop(SharedStruct *)
*
*  Increase the top of the undo buffer and realloc if necessary.
*
**********/
void __regargs UndoAddTop(SharedStruct *shared);

/************************************************
*
*  FreeUndoMem(BufStruct *, int)
*
*  Free (int) bytes from the wanted undo buffer.
*  Return:  TRUE=all OK, FALSE=can't free mem.
*
**********/
int __regargs FreeUndoMem(SharedStruct *shared, int mem);

/************************************************
*
*  FreeUndoLines(BufStruct *, int)
*
*  Free (int) line from the wanted undo buffer.
*  Return:  TRUE=all OK, FALSE=can't free mem.
*
**********/
int __regargs FreeUndoLines(SharedStruct *shared, int rader);

/************************************************
*
*  Undo(BufStruct *, int)
*
*  Perform a real undo. Say if you want it to continue down in the
*  undo list (TRUE) or go to the end (FALSE).
*  Returns a std-error massage.
*
**********/
int __regargs Undo(BufStruct *Storage, int cont);

UndoStruct __regargs *UndoAlloc(SharedStruct *shared);

void __regargs UndoNoChanged(SharedStruct *Storage);

void __regargs UndoFree(SharedStruct *shared, UndoStruct *undo);

void StoreWorkString(void);
