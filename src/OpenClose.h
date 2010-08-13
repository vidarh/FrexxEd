/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * OpenClose.h
 *
 *********/

#include "compat.h"

void FirstOpen(void);
void LastOpen(void);

char __regargs *OpenLibraries(void);
void __regargs CloseLibraries(char *message);

char __regargs *OpenMyScreen(WindowStruct *win);

void CloseAll(char *string);
void CloseFrexxEd(register __a0 char *string);

void PrintScreenInit(void);

void CloseDownFrexxEd(int);
void CloseMyScreen(WindowStruct *win);

int OpenUpScreen(WindowStruct *win);

int OpenIcon(void);

struct Window __regargs *FixWindow(BufStruct *Storage, char *string);

void SetupMinWindow(WindowStruct *win);

void __regargs GetDimension(ULONG mode);

void __regargs CalcPlanes(WindowStruct *win);

int __regargs Iconify(WindowStruct *specific_window);
int __regargs Deiconify(WindowStruct *specific_window);
void __regargs OpenAppIcon(void);
void __regargs CloseAppIcon(void);
int __regargs CloneWB(WindowStruct *win);
long __regargs FixRGB32(ULONG *cols);
void __regargs SetColors(struct Screen *sc, int col, int red, int green, int blue);
void __regargs CopyColors(struct Screen *sc, WindowStruct *win);
void InitColors(WindowStruct *win);
void __regargs TestScreenMode(WindowStruct *);
void __regargs AdjustBufsInWindow(WindowStruct *win);

/*************************************************************
*
* ResetColors(int col);
*
* Reset all colors to the Workbench default colors.
*
******/
void __regargs ResetColors(int col, WindowStruct *win);

WindowStruct __regargs *MakeNewWindow(WindowStruct *def);
void __regargs FreeWindow(WindowStruct *win);
WindowStruct *CreateNewWindow(WindowStruct *defwin, BufStruct *newstorage, BufStruct *Storage);
void __regargs CheckWindowSize(WindowStruct *win);


/* Flags for WindowStruct */
#define WINDOW_FLAG_REOPEN 1
#define WINDOW_FLAG_FREEZED 2

int __regargs LoadKeyMap(void);

