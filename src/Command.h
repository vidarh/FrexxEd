/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
*
* Command.h
*
*******/

#define OUTPUT_UNDO_MAX 40

#define MAX_PROMPT_LEN 80
#define MAX_ARGUMENTS 8

#define FN_NORMAL   0
#define FN_DEFAULT1 1

#define cmflag_POINTER  1       // Argv innhåller i hemlighet x, y-koordinater

/****************************************************************
 *
 * int Command(BufStruct * int)
 *
 * Command does what you tell it to do (int). Returns the exit code (int).
 * All commands are listed above as defines.
 *
 *********/
int __regargs Command(BufStruct *Storage, int command, int Argc, char **Argv, int flags);

/**************************************************
*
*  AppendYank(char *, int)
*
* Append the string (char *) with length (int) to the YankBuffer.
* This function will Dealloc the input string.
*
* No return.
********/
void __regargs AppendYank(char *string, int stringlen);

BOOL __regargs MakeCall(cmd command, int Argc, char **Argv);
