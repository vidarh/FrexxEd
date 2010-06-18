/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * GetFile.h
 *
 *********/
#include <dos/dos.h>

typedef struct Files {
  char *name;
  struct Files *next;
};

int __regargs GetFile(BufStruct *Storage,  char *path, char *filename);

int __regargs Split(SharedStruct *shared, char *name);

int __regargs Load(BufStruct *Storage, char *string, int flag, char *filename);

/* To use when loading with wildcards: */
#define loadNEWBUFFER	1 /* Every wildbuffer is a new buffer */
#define loadINCLUDE	2 /* Load all wildbuffers to one new buffer */
#define loadNOINFO	4 /* Do not display info on the origin status line */
#define loadNOQUESTION	8 /* Do not ask if you should find the same name */
#define loadPROMPTFILE	16/* Display a file requester even if filename is given. */
#define loadBUFFERPATH  32/* Take the path from the current buffer. */

int __regargs Save(BufStruct *Storage, int flag, char *string, char *filename, char *packmode);

#define saveASK		1 /* Set up a filerequester */
#define saveCLEANUP	2 /* Clean up the buffer for faster disk access */

int __regargs SaveString(BufStruct *Storage, char *filename, char *string, int len);

int __regargs GetFileNames(BufStruct *Storage, char *wildcard, char ***retpoint, int *retantal, char **retnames);

int __regargs CheckName(BufStruct *Storage, int requester, int update, char *name);

/*****************************************************
 *
 * ExtractLines(char *text, int length, char **rettext, int **retlength, int *lines)
 *
 * Extract the lines from a textblock.
 *
 * Input: Return a array of (char **) in (char ***) which is up to you to Dealloc.
 *
 * Return: A ret value.
 **********/
int ExtractLines(char *text, int length, TextStruct **rettext, int *lines, BufStruct *can_be_folded);

typedef struct {
  char *filename;	/* filename */
  char *path;		/* path */
  char *buffer;		/* buffer to store real filename or error message */
  char *realpath;	/* buffer to store real pathname. */

  char *program;	/* program buffer */
  int length;		/* length of file */
  LONG fileprotection;	/* Protection flags for file. */
  struct DateStamp date;/* Date file last saved */
  char password[80];	/* encryption password */
  char comment[80];	/* File comment */
  char pack[5];		/* file was packed with PowerPacker */
} ReadFileStruct;

/**********************************************************************
 *
 * int ReadFile(BufStruct *Storage, ReadFileStruct *RFS)
 *
 * Reads a file into memory and store the information in a ReadFileStruct.
 * Give a (BufStruct *) if you want some information to be output on the screen.
 *
 * If anything goes wrong, no program will be available and no freeing
 * is needed by the caller of this function.
 *
 * Return: a ret-value.
 *******/
int __regargs ReadFile(BufStruct *Storage, ReadFileStruct *RFS);

void __regargs GiveProtection(int protectionbits, char *output);

int __regargs GetFileList(BufStruct *Storage,
			  UBYTE *wildcard,    /* pattern pointer! */
			  struct Files **list, /* list to read names from! */
			  char **remember, /* allocremember pointer pointer */
			  int checkname);

int __asm CheckAndPrompt(register __a5 BufStruct *Storage,
                         register __a0 char *name,
                         register __d0 int checkname,
                         register __a1 char *namebuffer);

int __regargs Stcgfn(char *name, char *filename);
int __regargs Stcgfp(char *path, char *filename);
int __regargs RenameBuf(BufStruct *Storage, char *name);

int __regargs GetWildWord(char *from, char *store);

void __regargs ChangeDirectory(char *dir);

