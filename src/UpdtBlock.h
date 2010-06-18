/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************************
 *
 *  UpdtBlock.h
 *
 ******/

/************************************************
 *
 *  UpdateBlock(BufStruct *)
 *
 *  Updates your block, relative to the last updating.
 *  Input wanted BufStruct.  No return.
 *********/
void __regargs UpdateBlock(BufStruct *Storage);

/******************************************************
*
*  SetBlockMark(BufStruct *)
*
*  Simply update your block mark.  You have to clear
*  the screen yourself.
*  Input wanted BufStruct pointer.
********/
void __regargs SetBlockMark(BufStruct *Storage, BOOL erase);

/******************************************************
*
*  PasteBlockMark(BufStruct *, ...)
*
*  Paste a block in Bpl2 according to your command.
*
********/
void __regargs PasteBlockMark(BufStruct *Storage, int xstart, int ystart,
                    int xstop, int ystop, int pattern);
