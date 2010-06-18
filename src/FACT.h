/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************
 *
 *  FACT.h
 *
 *********/

#ifndef FACT_H
#define FACT_H

#define FACT_EOF		0
#define FACT_NO_EOL		1
#define FACT_FOLDED_CHAR	2
#define FACT_FOLDUP_CHAR	3
#define FACT_NOFOLD_CHAR	4
#define FACT_NON_CHAR		5
#define FACT_EXTRA_STRINGS	6	// max


struct FACT {
  char *flags;		/* The eight most important flags */
  int *xflags;		/* Extended flags */
  char **strings;	/* String to output */
  UBYTE *length;	/* Length of string (max 255) */
  char *delimit;	/* Delimiter char */
  char *cases;		/* Opposit case	*/
  char newlinechar;	/* Charachter to use when to make a newline */
  char tabchar;		/* Charachter to use when to make a tab */
  char newlinecheck;	/* Number of newline characters that exists (255=256) */
  char *name;
  struct FACT *next;
  String extra[FACT_EXTRA_STRINGS];
};
typedef struct FACT FACT;


#define fact_NEWLINE 	 (1 << 0)	/* A new line char */
#define fact_TAB 	 (1 << 1)	/* A tab char */
#define fact_STRING 	 (1 << 2)	/* Ouput a string */

#define factx_CLASS_SPACE	(1 << 0)
#define factx_CLASS_WORD	(1 << 1)
#define factx_CLASS_SYMBOL	(1 << 2)
#define factx_CLASS_PUNCT	(1 << 3)
#define factx_CLASS_OPEN	(1 << 4)
#define factx_CLASS_CLOSE	(1 << 5)
#define factx_CLASS_ESCAPE	(1 << 6)
#define factx_UPPERCASE		(1 << 7)
#define factx_LOWERCASE		(1 << 8)
#define factx_DEFAULT		(1 << 9)
#define factx_ALLOCSTRING	(1 << 10)

FACT __regargs *InitFACT(void);
void __regargs CheckFACT(FACT *fact);
FACT __regargs *ClearFACT(FACT *fact);
int __regargs SetFACT(FACT *fact, int Argc, char **Argv, char *format);
int __regargs FACTConvertString(BufStruct *Storage, char *string, int len, String *result, int flag);
int __regargs FactInfo(FACT *fact, int type, short tkn);
BOOL __regargs CheckNotStarted(void);
int __regargs ResetFACT(FACT *fact, int tecken);
int __regargs CreateFact(char *name);
FACT __regargs *FindFACT(char *name);
int __regargs FACTDelete(char *name);
void __regargs NewFACT(FACT *newfact, FACT *oldfact);

/* Defines for control byte in output strings */

#define cb_NORMAL 	0
#define cb_REVERSE	1

#define cb_BOLD		2
#define cb_ITALIC	4
#define cb_UNDERLINE	8

#define cb_NEWLINE 	16
#define cb_EOS		32

#endif
