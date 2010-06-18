/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/*****************************************
*
* Cursor.h
*
*****/

/*****************************************
*
* CursorUD(BufStruct *);
*
* Handles all cursor and screen limits when moving cursor up and down.
*
*****/
void __regargs CursorUD(BufStruct *Storage);

/*****************************************
*
* CursorLR(BufStruct *);
*
* Handles all cursor and screen limits when moving cursor left and right.
*
*****/
int __regargs CursorLR(BufStruct *Storage);

/*****************************************
*
* int CursorUp (BufStruct *);
*
* Call CursorUp() on every cursorup event. It returns (int) whether the
* screen has scrolled or not.
*
*****/
int __regargs CursorUp(BufStruct *Storage);

/*****************************************
*
* int CursorDown (BufStruct *);
*
* Call CursorDown() on every cursordown event. It returns (int) whether the
* screen has scrolled or not.
*
*****/
int __regargs CursorDown(BufStruct *Storage);

/**************************************************************
 *
 * MoveCursor(BufStruct *Storage, int antalcounter)
 *
 * Flytta cursorn (x) antal steg i sidled.
 *
 * Returnera antal steg som flyttats.
 *********/
int __regargs MoveCursor(BufStruct *Storage, int antalcounter);


typedef struct {
  int curr_topline;     /* which line is at the top of the screen */
  int cursor_x;         /* in which column is the cursor */
  int cursor_y;         /* in which line is the cursor */
  int curr_line;        /* in which line is the cursor */
  int screen_x;         /* x-position of the screen */
  int wanted_x;
  int string_pos;
} CursorPosStruct;

void __regargs SaveCursorPos(BufStruct *Storage, CursorPosStruct *cpos);
void __regargs RestoreCursorPos(BufStruct *Storage, CursorPosStruct *cpos, BOOL testc);

