#ifndef __FREXXED_REGEX_H
#define __FREXXED_REGEX_H

/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */

#ifdef __SASC
#define REGARGS __regargs
#define INLINE  __inline
#else
#define REGARGS
#define INLINE
#endif

#ifdef FREXXED
typedef enum {
  CHAR_WORD,
  CHAR_SYMBOL,
  CHAR_SPACE,
  CHAR_UCASE,
  CHAR_LCASE,
  CHAR_OPEN,
  CHAR_CLOSE,
  CHAR_NEWLINE,
  CHAR_TAB,

  CHAR_ILLEGAL_TYPE
} re_CharCode;

#include "Buf.h"
#endif


#define STATIC static

/* Define number of parens for which we record the beginnings and ends.
   This affects how much space the `struct re_registers' type takes up.  */
#ifndef RE_NREGS
#define RE_NREGS 10
#endif

/* This data structure is used to represent a compiled pattern. */

struct re_pattern_buffer {
  signed char *buffer;	    /* Space holding the compiled pattern commands. */
  int allocated;	/* Size of space that  buffer  points to */
  int used;		    /* Length of portion of buffer actually occupied */
  signed char *fastmap;	/* Pointer to fastmap, if any, or zero if none.
					   re_search uses the fastmap, if there is one,
					   to skip quickly over totally implausible characters */
  struct FACT *fact;	/* Translate table to apply to all characters before comparing.
					   Or zero for no translation.
					   The translation is applied to a pattern when it is compiled
					   and to data when it is matched. */
  signed char fastmap_accurate;  /* Set to zero when a new pattern is stored,
							 set to one when the fastmap is updated from it. */
  signed char can_be_null;   /* Set to one by compiling fastmap
						 if this pattern might match the null string.
						 It does not necessarily match the null string
						 in that case, but if this is zero, it cannot.
						 2 as value means can match null string
						 but at end of range or before a character
						 listed in the fastmap.  */
  };

/* Structure to store "register" contents data in.

   Pass the address of such a structure as an argument to re_match, etc.,
   if you want this information back.

   start[i] and end[i] record the string matched by \( ... \) grouping i,
   for i from 1 to RE_NREGS - 1.
   start[0] and end[0] record the entire string matched. */

struct re_registers
  {
    int start[RE_NREGS];
    int end[RE_NREGS];
#ifndef OLD
	int start_line[RE_NREGS];
	int end_line[RE_NREGS];
#endif
  };

struct re_position {
  unsigned char *ptr;       /* current pos */
  unsigned int line;        /* current line */
  unsigned int right;       /* chars to the right on this line */
  unsigned int col;         /* chars to the left on this line */
  struct re_textdata *data; /* pointer to the text information */
  struct SearchStruct *searchinfo;
  char finnished;
};

struct re_textdata {
  struct re_position pos;   /* top level position information */
  unsigned char *buf_start; /* start of buffer */
  unsigned char *buf_end;   /* end of buffer */
  TextStruct *text;         /* data array */
  unsigned int lines;       /* number of lines */
  int end_line;	            /* last line */
  int end_col;		    /* last column */
  int start_line;           /* start line */
  int start_col;            /* start column */
  int searchflags;	    /* search flags */
};  

typedef enum {
  RE_OK,  /* returns OK */
  RE_INVALID_REGULAR_EXPRESSION,
  RE_UNMATCHED_OPEN_PAREN,
  RE_UNMATCHED_CLOSE_PAREN,
  RE_PREMATURE_END_OF_RE,
  RE_NESTING_TOO_DEEP,
  RE_TOO_BIG,
  RE_OUT_OF_MEMORY,

  RE_NUM_OF_RETCODES
} re_RetCode;
  

signed char * REGARGS re_replace(TextStruct * ,
				 struct re_registers * ,
				 signed char * ,
				 int * );

signed char REGARGS
PointInit(struct re_textdata *text, /* pointer to re_position struct */
          struct SearchStruct *searchinfo,
	  unsigned int line, /* start line */
	  unsigned int col,  /* start column */
	  TextStruct *array, /* buffer array */
	  unsigned int lines,  /* number of lines */
          int end_line,       /* end line */
          int end_col,        /* end column */
          signed char forward); /* TRUE - forward, FALSE backward */


re_RetCode REGARGS re_compile_pattern(signed char * ,
                                      struct re_pattern_buffer * );

void REGARGS re_compile_fastmap(struct re_pattern_buffer * );

int REGARGS re_search(struct re_pattern_buffer * ,
                      int,
                      struct re_registers *,
                      struct re_position *);

int REGARGS re_match(struct re_pattern_buffer *,
                     struct re_registers *,
                     struct re_position *);


#endif

