/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/***********************************************
 *
 *  Init.h
 *
 ***********/

/***********************************************
 *
 *  InitDefaultBuf()
 *
 *  Init for the Default.BufStructDefault.
 *
 ***********/
void InitDefaultBuf(void);
void SecondInit(void);

void __regargs PrepareRedrawOfWindow(WindowStruct *win);

void InitKeys(void);

void __regargs InitWindow(WindowStruct *win);

