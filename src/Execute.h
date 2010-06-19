/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************************
*
* Execute.h
*
* Function prototypes
*
*/
/**********************************************************************
 *
 * InitFPL()
 *
 * Initializes fpl.library handling. Returns non-zero if anything failed!
 *
 *****/
int __regargs InitFPL(char type);

/*****************************
*
* ExecuteFPL()
*
* Mode:
* EXECUTE_SCRIPT - executes the script the second paramter points to
* EXECUTE_FILE   - executes the file
* EXECUTE_CACHED_SCRIPT - executes a script, but enables caching!
*
* Returns the error code of the FPL.library calls in the integer
* the fourth argument points to.
*/

#define EXECUTE_SCRIPT        0 /* (old: FALSE) */
#define EXECUTE_FILE          1 /* (old: TRUE)  */
#define EXECUTE_CACHED_SCRIPT 2

int __regargs ExecuteFPL(BufStruct *, char *, char, int *, char *);

struct UserData {
  BufStruct *buf;
  long a4;
};

struct SprintfData {
  long a4;
  char *text;
  long size;
  long pos;
};

struct fplArgument;

long __asm StopCheck(register __a0 void *);
long __asm __stackext run_functions(register __a0 struct fplArgument *arg);
int __regargs RequestWindow(BufStruct *Storage, struct fplArgument *arg);
int __regargs ScSort(struct fplArgument *arg);
int __regargs BSearch(struct fplArgument *arg);

#ifdef LOG_FILE_EXECUTION
int __regargs LogFileExecution(char *file);
int __regargs FindFileExecution(char *file);
void __regargs DeleteFileExecutionList(void);
#endif
