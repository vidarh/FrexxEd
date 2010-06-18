/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Limit.h
 *
 * General FrexxEd limitations.
 *
 *********/

#define ALLOC_OVERHEAD 50		/* Overhead of line allocation */
#define ALLOC_STEP 3			/* Number of lines to alloc at buffer creation */
#define KEYFILE "FrexxEd:FrexxEd.key"	/* Key file name */
#define DEFAULTNAME "*noname*"		/* Default startup buffer name */
#define LIB_REV 37			/* Lowest kickstart version supported */
#define MAX_CHAR_LINE 2048		/* Buffer size */
#define RET_BUF_LEN 200			/* Lenght of the return msg buffer */
#define CRYPT_REVISION 1
#define FILESIZE 256


#define EXECUTED_KEY	"Key execution"
#define EXECUTED_MENU	"Menu execution"
#define EXECUTED_PROMPT	"Prompt execution"
#define EXECUTED_AREXX	"ARexx execution"
#define EXECUTED_STRING	"String execution"
#define EXECUTED_BUTTON	"Button execution"
#define EXECUTED_TIMER	"Timer execution"
#define EXECUTED_HANDLER "Handler execution"

#ifdef DEBUGTEST
#define DEBUGLINE { if (DebugOpt) printf("%s, %d\n", __FILE__, __LINE__); }
#else
#define DEBUGLINE
#endif
