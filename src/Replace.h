/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************
*
*       Replace.h
*
********/
/********************************************
*
*   Replace(Storage, int)
*
* variant=0 ger promptning.
* variant=1 ger continue.
* variant=2 ger engångsreplace.
*********************/
int __regargs Replace(BufStruct *Storage, int argc, char **argv);

/***********************************************************
 *
 * int ReplaceWord(...)
 *
 * Replacear ett ord med ett annat.  sstringlen adderas till slutpositionen.
 * Returnar antalet relativt deletade tecken.
 ******/
int __regargs ReplaceWord(BufStruct *Storage, int xpos, int rad, int xstop, int ystop,
                int sstringlen, char *rstring, int rstringlen, int undoadd);

typedef struct {
  char *firstpos;
  char *currentread, *currentwrite;
  char *newtext;
  int newsize;
} turbostruct;

int __regargs ReplaceTurboWord(BufStruct *Storage,
                               SearchStruct *SearchInfo,
                               turbostruct *TT);

int __regargs BeginTurbo(BufStruct *Storage, turbostruct *TT, SearchStruct *SearchInfo);
