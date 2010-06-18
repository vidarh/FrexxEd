/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * UpdtScreen.h
 *
 *********/

/*****************************************
*
* UpdateScreen (BufStruct *);
*
* Simply redraws the screen according to the (BufStruct *) values.
*
*****/
void __regargs UpdateScreen(BufStruct *Storage);

/*****************************************
*
* UpdateDupScreen (BufStruct *);
*
* Redraws all screens duplicated by the (BufStruct *).
*
*****/
void __regargs UpdateDupScreen(BufStruct *Storage2);

/*****************************************
*
* UpdateLines (BufStruct *, int editrad);
*
* Update chosen buffer from line (int).
*
*****/
void __regargs UpdateLines (BufStruct *Storage2, int editrad);

/*****************************************
*
* UpdateAll (void);
*
* Simply redraws all bufs according to the BufStruct values.
*
*****/
void UpdateAll(void);
void UpdateWindow(WindowStruct *win);

void __regargs CenterScreen(BufStruct *Storage);

/*****************************************
*
* int Col2Byte(BufStruct *, int, char *, int length);
*
* Returns (int) where in the string (char *) with length (int)
* you should peek if you want the string from column (int).
*
*****/
int __regargs Col2Byte(BufStruct *Storage, int var, char *text, int length);

/*****************************************
*
* int Byte2Col(BufStruct *, int, char *);
*
* Returns (int) the screen length of (int) bytes of the string (char *).
*
*****/
int __regargs Byte2Col(BufStruct *Storage, int var, char *text);

/*****************************************
*
* PrintLine (BufStruct *, int line);
*
* Prints a line on the screen.
*
*****/
void __regargs PrintLine(BufStruct *Storage2, int editrad);

/*****************************************
*
* int CheckPos(BufStruct *);
*
* Moves the screen if the BUF(cursor_x) is out of line in any direction.
* Returns TRUE if the screen was updated, otherwise FALSE;
*
*****/
int __regargs CheckPos(BufStruct *Storage);

int __regargs TestAllCursorPos(void);

/*****************************************
*
* int TestCursorPos(BufStruct *);
*
* Test if the cursor and blockmark are in valid places.
* Return TRUE if the screen was updated.
*****/
int __regargs TestCursorPos(BufStruct *Storage);


/*****************************************
*
* SetScreen(BufStruct *, int, int);
*
* Set cursor at string position x (int) and line y (int).
* Returns TRUE if screen is updated.
*
*****/
int __regargs SetScreen(BufStruct *Storage, int x, int y);

void __regargs UpdtDupStat(BufStruct *Storage);
void __regargs Showstat(BufStruct *Storage);
void ShowAllstat(void);
void __regargs Showname(BufStruct *Storage);
void __regargs Showplace(BufStruct *Storage);
void __regargs Showerror(BufStruct *Storage, int error);


void ClearVisible(void);

void UpdateFace(void);

#define UD_NOTHING		0
#define UD_REDRAW_BORDER	(1<<0)
#define UD_REDRAW_VIEWS		(1<<1)
#define UD_CLEAR_SCREEN		(1<<2)
#define UD_SHOW_STATUS		(1<<3)
#define UD_REDRAW_SETTING	(1<<4)
#define UD_CHECK_FACT		(1<<5)
#define UD_MOVE_SLIDER		(1<<6)

#define UD_REDRAW_ENTRY		(1<<7)
#define UD_REDRAW_BLOCK		(1<<8)
#define UD_CLEAR_ENTRY		(1<<9)

#define UD_REDRAW_BUFFS		(1<<10)

#define UD_FACE_UPDATE		(1<<11)

