/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************************
*
*  Change.h
*
**********/

int __regargs PromptSetting(BufStruct *Storage, BufStruct *copy_to, int type, char *windowname, char **argv, int argc, int mask, int nonmask);
int __regargs Change(BufStruct *Storage, int type, int vald, int newvalue);
int __regargs ChangeAsk(BufStruct *Storage, int vald, int *input);
int __regargs ReDrawSetting(BufStruct *Storage, WORD update);
int __regargs GetSettingName(char *name);
int __regargs LoadSetting(BufStruct *Storage, char *filename, int load);

#define cs_LOCAL 1
#define cs_GLOBAL 2
#define cs_LOCAL_ALL 4
#define cs_STRING 8		// input is given as an string

/********************************************************
 *
 * CopyLineNumberWidth(void);
 *
 * Kopierar linenumberwidth till alla buffrar.
 *******/
void CopyLineNumberWidth(void);

BOOL __regargs CheckSetting(BufStruct *Storage, char *name);

int __regargs Bsearch(void *name, int antal, int __asm (cmp)(register __a0 void *, register __d1 int));
