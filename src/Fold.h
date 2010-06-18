/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Fold.h
 *
 *********/

int __regargs Fold(BufStruct *Storage, int startline, int endline);
int __regargs UnFold(BufStruct *Storage, int line);
int __regargs FoldShow(BufStruct *Storage, int line);
int __regargs FoldHide(BufStruct *Storage, int line);
void __regargs AdjustCursorUp(BufStruct *Storage);
void __regargs AdjustCursorDown(BufStruct *Storage);
int __regargs FoldNearestLine(BufStruct *Storage, int line);
int __regargs FoldFindLine(BufStruct *Storage, int line);
int __regargs FoldMoveLine(BufStruct *Storage, int line, int move);
int __regargs FoldCompactBuf(SharedStruct *shared, String *retstring);
int __regargs FoldDiff(BufStruct *Storage, int line1, int line2);

#define FOLD_HIDDEN 1
#define FOLD_BEGIN 2

#define FOLD_STRING_BEGIN "FrexxStartFold"
#define FOLD_STRING_END "FrexxEndFold"
#define FOLD_STRING_OVERHEAD 6	/* Maximum overhead */

