/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * KeyAssign.c
 *
 *********/

struct KeyStruct {
  char *string;
  UWORD Qual;
  struct KeyStruct *next;
  char rawkey;
};

typedef struct KeyStruct KeyStruct;

int __regargs KeyAssign(BufStruct *Storage, char *string, char *function,
                        char *depended, int flag);

	/* Used if no string is given */
#define kaINPUT 	1
#define kaREQUEST	2
#define kaMENU		4	// Assign is called from BuildMenu.

struct Kmap __regargs *GetKeyMap(struct Kmap *firstkmap, UWORD Qualifier, int string, int rawkey);	// Note: The 'string' argument is a 'char *'.  The type is changed as an workaround of a SAS/C optimize bug.  / Kjelle 940904

/*****************************************************************
 *  KeySequence(struct Kmap *val, int flag)
 *
 *  Convert the Kmap to a string sequence
 *  Return: a string to be Dealloc().
 *********/
char __regargs *KeySequence(struct Kmap *val, int flag);

#define ksKEYSEQ TRUE	// KeySequence = 'Amiga Shift key'
#define ksMENU   FALSE	// MenuSequence= 'AS-key'

int PrintQualifiers(char *place, int Qual, int flag);

int __regargs MacroAssign(BufStruct *Storage, BlockStruct *block,
                          char *macroname, char *keysequence, int norequest);

int __regargs ConvertString(FACT *fact, String *string, String *result);

/*********************************************************
 *
 * TemplateString(char *string, KeyStruct *firstkey)
 *
 * Convert a plain key stroke text to a chain och KeyStructs.
 * Input:  The string.
 *         KeyStruct pointer to attach the first key.
 * Result: A ret value.
 *         A chain to the 'firstkey' to be Deallocated.
 *******/
int __regargs TemplateString(char *string, KeyStruct *firstkey);

/********************************************************
 *
 * DeleteKey(BufStruct *Storage, char *string)
 *
 * Delete the first key with given key stroke.
 * If no string is given, an input from the keyboard is taken.
 *****/
int __regargs DeleteKey(BufStruct *Storage, char *string);

/********************************************************
 *
 * FindKey(char *string)
 *
 * Find the first key with given key stroke.
 * Returns the FPL-program.
 *****/
char __regargs *FindKey(BufStruct *Storage, char *string);

/****************************************************
 *
 * DeleteKmap(struct Kmap *)
 *
 * Delete a KeyMap with its multiples
 *
 *********/
void __regargs DeleteKmap(struct Kmap *Kmap);

struct Kmap *AddKey(int flags, int qual, UWORD code, char *string, int cmd, struct Kmap *kmap, char *depended);

#define akCMD		1	/* cmd is a command */
#define akFPL		2	/* cmd is a pointer to a FPLstring */
#define akFPLALLOC	4	/* cmd is a pointer to a FPLstring to be Strdup */
#define akDEFAULT	8	/* cmd is a defaultcommand */
#define akMENU		16	/* cmd is a menucommand */
