/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/*******************************
 *
 *  FrexxEd_rev.c
 *
 ******/

#define COMPILE_DATE "(17.11.96)"
#define COMPILE_TIME "10:15:07"
#define COMPILE_REVISION 0
#define COMPILE_VERSION 2
#define PROGRAM_NAME "FrexxEd"
#define PROGRAM_VER "2.0"

#define FREXXED_VER PROGRAM_NAME" "PROGRAM_VER

#ifndef BUF_H
	/* This 'ifdef' is here to make it possible to include this file
	   as an ordinary header file. */
char *version={FREXXED_VER" "COMPILE_DATE};
int version_int=COMPILE_VERSION*10000 + COMPILE_REVISION;
char *FrexxName=FREXXED_VER;
#ifdef FINAL_VERSION
char *screen_title="%s Copyright © 1992-1996 by FrexxWare!";
#else
char *screen_title="%s Copyright © 1992-1996 by FrexxWare!";
#endif
#endif

