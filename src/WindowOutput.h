/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * WindowOutput.h
 *
 *********/

void __regargs SystemPrint(BufStruct *Storage, struct screen_buffer *buf, int len, int line);

/********************************************************************
 *
 * Status(BufStruct *, char *, int mode);
 *
 * Writes the text (char *) on the status line.
 * Mode: 0 - hela raden
 *       1 - namnet
 *       2 - infot
 *
 ********/
void __regargs Status(BufStruct *, char *, int);

/*****************************************
*
* ScrollScreen(BufStruct *, int dif, int special);
*
* Scroll the screen <dif> steps.  Up (-) and down (+).
* No return.
*
*****/
void __regargs ScrollScreen(BufStruct *Storage, int dif, int special);

#define scroll_NORMAL 0
#define scroll_SLIDER 1

void __regargs ClearWindow(WindowStruct *win);

/*****************************************
*
* CursorXY (BufStruct *, int, int);
*
* Places the cursor at the coordinates (int, int).
* x=-1 makes cursor invisible.
*
*****/
void __regargs CursorXY(BufStruct *Storage, int x, int y);
