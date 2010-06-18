/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
*
* Edit.h
*
*******/

/*******************************************
*
*  SWAP4(a,b)
*
*  Will swap the 4byte value in 'a' with 'b'.
*******/
#define SWAP4(a,b) {  register int temp, *c=(int *)&a, *d=(int *)&b;  temp=*c;  *c=*d;  *d=temp; }
#define SWAP1(a,b) {  register char temp, *c=(char *)&a, *d=(char *)&b;  temp=*c;  *c=*d;  *d=temp; }

#define TOLOWER(x) ((((x)>='A')&&((x)<='Z'))?((x)-'A'+'a'):(x))
#define TOUPPER(x) ((((x)>='a')&&((x)<='z'))?((x)-'a'+'A'):(x))

/*******************************************
*
*  MoveUp((char *)To, (char *)From ,int len)
*
*  A built in function to move mem upwards.  Opposite to memcpy.
*******/
			/* moving bytes */
#define MoveUp(a, b, c) { register int cc=(c); register char *aa=((char *)a)+(cc), *bb=((char *)b)+(cc); while (--cc>=0) *(--aa)=*(--bb); }
			/* moving words */
#define MoveUp2(a, b, c) { register int cc=(c); register SHORT *aa=((SHORT *)a)+(cc), *bb=((SHORT *)b)+(cc); while (--cc>=0) *(--aa)=*(--bb); }
			/* moving longwords */
#define MoveUp4(a, b, c) {  register int cc=(c);  if (cc) { register int *aa=(int *)(((int *)a)+cc), *bb=(int *)(((int *)b)+cc);  do { *(--aa)=*(--bb); } while (--cc>0);     } }

//#define Move4(a, b, c) {  register int cc=(c);  if (cc) { register int *aa=(int *)(a), *bb=(int *)(b);  do { *(aa++)=*(bb++); } while (--cc>0);     } }

/*************************************************************
*
* char *Deleteline(BufStruct *, int *);
*
* Performs a delete_line and returns a pointer to the character deleted
* with length at 'int *'.
* You have to free the return pointer yourself.
*********/
char __regargs *Deleteline(BufStruct *Storage, int *lenstore);

/*************************************************************
*
* int Backspace(BufStruct *);
*
* Performs a backspace and returns the character deleted.
*
*********/
int __regargs Backspace(BufStruct *Storage);

/***********************************************
*
*  BackspaceNr(BufStruct *, char *, int)
*
*  Perform (int) backspaces.
*  Return the undo-string in (char *).
*******/
int __regargs BackspaceNr(BufStruct *Storage, char *retstring, int antal);

/***********************************************
*
*  BackspaceUntil(BufStruct *, int, int, char **, int *)
*
*  Perform backspaces until given string-position.
*  Return the undo-string in (char **) with length (int *).
*******/
int __regargs BackspaceUntil(BufStruct *Storage, int xstop, int ystop, char **retstring, int *retlength);

/*************************************************************
*
* int OutputText(BufStruct *, char *, int, int, String *);
*
* Inserts a text-string (no backspaces) with length (int) 
* with insertmode if wanted (int).  If insert=FALSE the retstring
* will be placed in the (String *).
* Returns number of chars.
*
*********/
int __regargs OutputText(BufStruct *Storage, char *output, int length, int insert, String *getstring);

int __regargs GetWord(BufStruct *Storage, int line, int column, String *result);

/*************************************************************
*
* int WCfwd(BufStruct *);
*
* Returns number of characters to be played with by other routines that handles
* entire words. (delete word, right word, etc)
*
*********/
int __regargs WCfwd(BufStruct *Storage);

/*************************************************************
*
* int WCbwd(BufStruct *);
*
* Returns number of characters to be played with by other routines that handles
* entire words. (backspace word, left word, etc)
*
*********/
int __regargs WCbwd(BufStruct *Storage);

/*************************************************************
*
* char *Del2Eol(BufStruct *, int *);
*
* Deletes the rest of the line and returns a char pointer to
* the deleted string with length at (int *).
*
*********/
char __regargs *Del2Eol(BufStruct* Storage, int *lenstore);

/******************************************************
*
*  InsertLine(BufStruct *, int, int)
*
*  To be called when a line is inserted in a duplicated buffer.
*********/
void __regargs InsertLine(BufStruct *Storage2, int rad, int antal);

/******************************************************
*
*  DeleteLine(BufStruct *, int, int)
*
*  To be called when a line is deleted in a duplicated buffer.
*********/
void __regargs DeleteLine(BufStruct *Storage2, int rad, int antal);

int __regargs ExpandText(SharedStruct *shared, int wanted);

int __regargs InsertText(BufStruct *Storage, char *text, int length, int position);

void __regargs Pos2LineByte(SharedStruct *,
                           long,    /* position */
                           long *,  /* is this line */
                           long *); /* and this byte position */
