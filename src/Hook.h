/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
*
* Hook.h
*
* Hook functions' header.

*******/

#define HOOK_POSTHOOK 1 /* add a post hook! */
#define HOOK_ABS 2      /* "absolute" function. Use the exact hook name as
			   a program without adding the parameters from the
			   original invoke! */

int __regargs RunHook(BufStruct *, int, int, char **, int);
int __regargs AddHook(BufStruct *, char *, char *, char *, char);
int __regargs HookClear(BufStruct *, char *, char *, char *);
