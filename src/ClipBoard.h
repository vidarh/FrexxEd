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

#define STRING2CLIP (register __a0 char *, register __d0 int, register __d1 int)
int __asm String2Clip STRING2CLIP;

int __regargs Clip2String(BufStruct *Storage, char **retstring, int *retlen, int clipno);
