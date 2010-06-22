/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
*
* ClipBoard.h
*
* ClipBoard functions!
*
*******/

int __asm String2Clip(register __a0 char *string, register __d0 int stringlen, register __d1 int clipno);
int __regargs Clip2String(BufStruct *Storage, char **retstring, int *retlen, int clipno);
