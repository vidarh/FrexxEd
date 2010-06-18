/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
*
* Block.h
*
* Support block functions!
*
*******/

/*********************************************************
*
*  BlockCopy(BufStruct *, BlockStruct *, int flag, BlockCutStruct *blockcut)
*
*  Copy your block to wanted block buffer (NULL = BlockBuffer).
*  Flags are defined in 'Block.h'.
*  Function clears block_exists flag.
*  Input:  wanted BufStruct pointer.
*  Return: a 'ret' value.
*
***********/
int __regargs BlockCopy(BufStruct *Storage, BlockStruct *block, int special, BlockCutStruct *blockcut);

/*********************************************************
*
*  BlockPaste(BufStruct *, int undosort, BlockStruct *block, int flag)
*
*  Paste wanted block (NULL = BlockBuffer).
*  Tell (int) how to access the undo buffer.
*  Tell (int) if the block is to be erased.
*  Return: a ret value.
*
***********/
int __regargs BlockPaste(BufStruct *Storage, int undosort, BlockStruct *block, int flag);
#define bp_NORMAL	0
#define bp_KILL		1	// Fungerar inte med bp_COLUMN
#define bp_COLUMN	2
#define bp_LINE		3


/*************************************************************
*
*  int Block2String(BufStruct *, BlockStruct *block, char **, int *) 
*
*  Copy the marked/given block to a string, returned in (char **) and len
*  in (int *).
*  Return a ret value.
************/
int __regargs Block2String(BufStruct *Storage, BlockStruct *block, char **retstring, int *retlen);

int __regargs String2Block(BlockStruct *block, char *string, int stringlen, char append);

/*************************************************************
*
*  int BlockSort(BufStruct *, BlockStruct *block, int pos, int flags)
*
*  Sort the marked/given block.  If nothing marked/given: the BlockBuffer.
*  Start sorting at field (int pos).
*  Return a ret value.
************/
int __regargs BlockSort(BufStruct *Storage, BlockStruct *block, int pos, int flags);

/*************************************************************
*
*  int BlockCase(BufStruct *, BlockStruct *block, int flag)
*
*  Change the case of the marked/given block.
*  Defines for flag in Block.h
*  Return a ret value.
************/
int __regargs BlockCase(BufStruct *Storage, BlockStruct *block, int flag);
#define bc_CHANGE_CASE 1
#define bc_UPPER_CASE 2
#define bc_LOWER_CASE 3

void __regargs ClearBlock(BlockStruct *block);

BlockStruct __regargs *AllocBlock(char *name);

int __regargs FreeBlock(BlockStruct *block, char *name);

BlockStruct __regargs *FindBlock(BufStruct *name);

/*************************************************************
 *
 *  BlockMove(BufStruct *, int move, int undoadd)
 *
 *  Move a block int steps rigth(+) or left(-).
 *  Return a retvalue.
 * 
 *  Obs:  Will only consider a block from the first line to the last -1.
 ***********/
int __regargs BlockMove(BufStruct *Storage, int move, int undoadd);

int __regargs BlockMovement(BufStruct *Storage);

/* blocktype definition */

#define bl_NORMAL 1
#define bl_COLUMNAR 2
#define bl_LINE 3


/* block copy definition */

		/* Append to block */
#define bl_APPEND	4
		/* Cut block */
#define bl_CUT		8
		/* Don't alloc new lines */
#define bl_NOALLOC	16
		/* Store in the UndoBuffer */
#define bl_UNDOADD	32
		/* No copy to the BlockBuffer */
#define bl_NOSTORE	64
		/* Append to UndoBuffer */
#define bl_UNDOAPPEND	128	/* Används bara vid bl_NORMAL */

void __regargs PlaceBlockMark(BufStruct *Storage, BlockCutStruct *blockcut, int x1, int y1, int x2, int y2);

