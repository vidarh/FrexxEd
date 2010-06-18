/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/******************************************************************************
 *                          FREXXWARE ENTERPRISES
 * ----------------------------------------------------------------------------
 *
 * Project: FrexxEd
 * $Source: regex.c $
 * $Revision: 2.0 $
 * $Date: 1.12.94 $
 * $Author: Daniel Stenberg $
 * $State: Experimental $
 *
 * Regular expresion routines for FrexxEd or stand-alone program. Compile
 * without the FREXXED symbol to create a stand-alone executable.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef FREXXED
#include <proto/FPL.h>
#include "Buf.h"
#include "Alloc.h"
#include "FACT.h"
#include "Fold.h"
#include "Search.h"
#include "regex.h"

extern FACT *UsingFact;
extern void *Anchor;

#define ISWORD(x)	(UsingFact->xflags[x]&factx_CLASS_WORD?1:0)
#define ISNWORD(x)	(!(UsingFact->xflags[x]&factx_CLASS_WORD))

#define ISSYMBOL(x)	(UsingFact->xflags[x]&factx_CLASS_SYMBOL?1:0)
#define ISSPACE(x)	(UsingFact->xflags[x]&factx_CLASS_SPACE?1:0)
#define ISUPPERCASE(x)	(UsingFact->xflags[x]&factx_UPPERCASE?1:0)
#define ISLOWERCASE(x)	(UsingFact->xflags[x]&factx_LOWERCASE?1:0)
#define ISOPEN(x)	(UsingFact->xflags[x]&factx_CLASS_OPEN?1:0)
#define ISCLOSE(x)	(UsingFact->xflags[x]&factx_CLASS_CLOSE?1:0)
#define ISNEWLINE(x)	(UsingFact->flags[x]&fact_NEWLINE)
#define ISTAB(x)	(UsingFact->flags[x]&fact_TAB?1:0)

signed char STATIC CheckChar(unsigned char, re_CharCode);
re_CharCode STATIC SetChar(unsigned char);

#else
#define Malloc malloc
#define Dealloc free
#define Remalloc realloc

#define ISWORD(x)  (SYNTAX(x) == Sword)
#define ISNWORD(x) (SYNTAX(x) != Sword)
#define ISNEWLINE(x) (x=='\n')
void STATIC REGARGS printstring(signed char *, int );
void STATIC REGARGS printchar (signed char);

typedef struct {
  char *text;
  int length;
} TextStruct;

#include "regex.h"

#endif

#ifdef FREXXED
re_CharCode STATIC SetChar(unsigned char letter)
{
  switch(letter) {
    case '(':
      return CHAR_OPEN;
    case ')':
      return CHAR_CLOSE;
    case 'W':
      return CHAR_WORD;
    case ' ':
      return CHAR_SPACE;
    case '!':
      return CHAR_SYMBOL;
    case 'N':
      return CHAR_NEWLINE;
    case 'T':
      return CHAR_TAB;
    case 'U':
      return CHAR_UCASE;
    case 'L':
      return CHAR_LCASE;
    default:
      /*
       * Illegal character, never match this!!
       */
      return CHAR_ILLEGAL_TYPE;
  }
}

signed char STATIC CheckChar(unsigned char letter, re_CharCode flag)
{
  switch(flag) {
    case CHAR_WORD:
      return (signed char)ISWORD(letter);
    case CHAR_SYMBOL:
      return (signed char)ISSYMBOL(letter);
    case CHAR_SPACE:
      return (signed char)ISSPACE(letter);
    case CHAR_UCASE:
      return (signed char)ISUPPERCASE(letter);
    case CHAR_LCASE:
      return (signed char)ISLOWERCASE(letter);
    case CHAR_OPEN:
      return (signed char)ISOPEN(letter);
    case CHAR_CLOSE:
      return (signed char)ISCLOSE(letter);
    case CHAR_NEWLINE:
      return (signed char)(ISNEWLINE(letter)?1:0);
    case CHAR_TAB:
      return (signed char)ISTAB(letter);
    default:
      return FALSE;
  }
}

#endif


void STATIC REGARGS insert_jump (signed char , signed char *, signed char *, signed char *);
void STATIC REGARGS store_jump (signed char *, signed char , signed char *);
int INLINE STATIC REGARGS bcmp_translate (unsigned char *, unsigned char *,
                                          int, FACT *);
STATIC INLINE signed char REGARGS PointAdd1(struct re_position *);
STATIC INLINE signed char REGARGS PointSub1(struct re_position *);
int STATIC REGARGS AppendToString(signed char ** , int * , signed char * , int );
STATIC signed char REGARGS PointTo(struct re_position * , int );


/* These are the command codes that appear in compiled regular expressions, one
   per byte.
   Some command codes are followed by argument bytes.
   A command code can specify any interpretation whatever for its arguments.
   Zero-bytes may appear in the compiled regular expression. */

enum regexpcode
  {
    unused,
    exactn,    /* followed by one byte giving n, and then by n literal bytes */
    begline,   /* fails unless at beginning of line */
    endline,   /* fails unless at end of line */
    jump,	 /* followed by two bytes giving relative address to jump to */
    on_failure_jump, /* followed by two bytes giving relative address of place
                        to resume at in case of failure. */
    finalize_jump,   /* Throw away latest failure point and then jump to
                        address. */
    maybe_finalize_jump, /* Like jump but finalize if safe to do so.
			    This is used to jump back to the beginning
			    of a repeat.  If the command that follows
			    this jump is clearly incompatible with the
			    one at the beginning of the repeat, such that
			    we can be sure that there is no use backtracking
			    out of repetitions already completed,
			    then we finalize. */
    dummy_failure_jump,  /* jump, and push a dummy failure point.
			    This failure point will be thrown away
			    if an attempt is made to use it for a failure.
			    A + construct makes this before the first repeat.  */
    anychar,	 /* matches any one character */
    charset,     /* matches any one char belonging to specified set.
		    First following byte is # bitmap bytes.
		    Then come bytes for a bit-map saying which chars are in.
		    Bits in each byte are ordered low-bit-first.
		    A character is in the set if its bit is 1.
		    A character too large to have a bit in the map
		    is automatically not in the set */
    charset_not, /* similar but match any character that is NOT one of those specified */
    start_memory, /* starts remembering the text that is matched
		    and stores it in a memory register.
		    followed by one byte containing the register number.
		    Register numbers must be in the range 0 through NREGS. */
    stop_memory, /* stops remembering the text that is matched
		    and stores it in a memory register.
		    followed by one byte containing the register number.
		    Register numbers must be in the range 0 through NREGS. */
    duplicate,    /* match a duplicate of something remembered.
		    Followed by one byte containing the index of the memory register. */
    before_dot,	 /* Succeeds if before dot */
    at_dot,	 /* Succeeds if at dot */
    after_dot,	 /* Succeeds if after dot */
    begbuf,      /* Succeeds if at beginning of buffer */
    endbuf,      /* Succeeds if at end of buffer */
    wordchar,    /* Matches any word-constituent character */
    notwordchar, /* Matches any char that is not a word-constituent */
    wordbeg,	 /* Succeeds if at word beginning */
    wordend,	 /* Succeeds if at word end */
    wordbound,   /* Succeeds if at a word boundary */
    notwordbound, /* Succeeds if not at a word boundary */
    syntaxspec,  /* Matches any character whose syntax is specified.
		    followed by a byte which contains a syntax code, Sword or such like */
    notsyntaxspec, /* Matches any character whose syntax differs from the specified. */
    grid,	/* match any character */
    on_failure_continue_on_next
  };

#ifndef NULL
#define NULL (char *)0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#if 1
#define DEBUG(x,y)
#define DEBUGCHAR(x,y)
#else
#define DEBUG(x, y) FPrintf(Output(), "*** LINE %ld, %s %ld\n", __LINE__, x, y);
#define DEBUGCHAR(x) fprintf(stderr, "%c", x);
#endif

#define isxdigit(x) ((toupper(x) >= 'A' && toupper(x) <= 'F') || \
		     isdigit(x))
#define toupper(x) ((x) & ~ 'a' - 'A')

#define isdigit(x) ((x) >= '0' && (x) <= '9')

#define isodigit(x) ((x) >= '0' && (x) <= '7')

#ifndef FREXXED
int main (int, signed char **);
#endif

int REGARGS STATIC
AppendToString(signed char **result_PPC,
               int *reslen_PI,
               signed char *append_PC,
               int applen_I)
{
  signed char *temp_PC;
  if(!applen_I) /* no string length! */
    return 0;
  
  if(!*reslen_PI) {
    *result_PPC = (signed char *)Malloc(applen_I);
    if(!*result_PPC)
      return 1;
  }
  else {
    temp_PC = (signed char *)Remalloc(*result_PPC, applen_I + *reslen_PI);
    if(!temp_PC) {
      Dealloc(*result_PPC);
      *result_PPC=NULL;
      return 1;
    }
    *result_PPC = temp_PC;
  }

  memcpy(*result_PPC + *reslen_PI, append_PC, applen_I);

  *reslen_PI += applen_I;
  return 0;
}

signed char * REGARGS
  re_replace(TextStruct *text_PPC,
             struct re_registers *regs_PS,
             signed char *replace_PC,
             int *retlen_PI)
{
  signed char *result_PC= NULL;
  int result_I=0; /* length of result */
  
  signed char *endofstring_PC=&replace_PC[strlen(replace_PC)];
  
  signed char touse;
  int len; /* generic length variable */
  
  signed char *ptr_PC;
  signed char *replptr_PC = replace_PC;
#ifdef FREXXED
  signed char *buffer;
#endif

  
  while(*replptr_PC && (ptr_PC = strchr(replptr_PC, '\\'))) {
    touse = ptr_PC[1];
    switch(touse) {
    case '&':
      touse= '0';
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':

      touse -= '0'; /* to makea a real number from it */

      if(regs_PS->start[touse]==-1) {
        replptr_PC = ptr_PC+2;
        break;
      }

      /*
       * Get the string from 'replace_PC' to 'ptr_PC'.
       * This string should first be C-to-raw converted and then
       * appended to the created replace-string!
       *
       * This version currently does not C-to-raw convert any string.
       */
      len = ptr_PC - replace_PC;

#ifdef FREXXED
      buffer = Malloc(len+1);
      if(buffer) {
        *ptr_PC = 0;
        len = fplConvertString(Anchor, replace_PC, buffer);
        *ptr_PC = '\\';
        if(AppendToString(&result_PC, &result_I, buffer, len)) {
          Dealloc(buffer);
          return NULL;
        }
        Dealloc(buffer);
      } else
        return NULL;
#else
      if(AppendToString(&result_PC, &result_I, replace_PC, len))
        return NULL;
#endif
      
      replptr_PC = ptr_PC +2; /* pass the \X too! */
      
      replace_PC = replptr_PC;
      
      /*
       * Now, get the \X - string and append that string to the created
       * result string. This string is already raw and should not be
       * converted in any way!
       */
      
      if(regs_PS->start_line[touse] == regs_PS->end_line[touse]) {
        /*
         * Starts and ends on the same line, nice...!
         */
        if(AppendToString(&result_PC,
                          &result_I,
                          (signed char *)text_PPC[ regs_PS->start_line[touse] ].text +
                          regs_PS->start[touse],
                          regs_PS->end[touse] - regs_PS->start[touse]))
          return NULL;
      }
      else {
        /*
         * The \X - string is a multi-line string. Go through each
         * line and add them as separate strings.
         */
        int line = regs_PS->start_line[touse];
        int end_line = regs_PS->end_line[touse];
        
        if(AppendToString(&result_PC,
                          &result_I,
                          (signed char *)text_PPC[ line ].text + regs_PS->start[touse],
                          text_PPC[ line ].length - regs_PS->start[touse]))
          return NULL;
        
        while(++line != end_line) {
          if(AppendToString(&result_PC,
                            &result_I,
                            (signed char *)text_PPC[line].text,
                            text_PPC[line].length))
            return NULL;
        }
        if(AppendToString(&result_PC,
                          &result_I,
                          (signed char *)text_PPC[ line ].text,
                          regs_PS->end[touse]))
          return NULL;
      }
      
      break;
    case 0:
      replptr_PC++;
      break;
      
    default:
      replptr_PC = ptr_PC+2;
      break;
    }
  }
  if(!result_PC) {
    len = strlen(replace_PC);
  }
  else {
    if(replace_PC != endofstring_PC) {
      len = endofstring_PC - replace_PC;
    } else
      len = 0;
  }
  /*
   * C-to-raw convert this before appending!
   */
#ifdef FREXXED
  if(len) {
    buffer = Malloc(len+1);
    if(buffer) {
      len = fplConvertString(Anchor, replace_PC, buffer);
      if(AppendToString(&result_PC, &result_I, buffer, len)) {
        Dealloc(buffer);
        return NULL;
      }
      Dealloc(buffer);
    }
  }
#else
  if(AppendToString(&result_PC, &result_I, replace_PC, len))
    return NULL;
#endif
  
  *retlen_PI = result_I;
  return result_PC;
}


signed char REGARGS
PointInit(struct re_textdata *text, /* pointer to re_position struct */
          struct SearchStruct *searchinfo,
	  unsigned int start_line, /* start line */
	  unsigned int start_col,  /* start column */
	  TextStruct *array, /* buffer array */
	  unsigned int lines,  /* number of lines */
          int end_line,       /* end line */
          int end_col,        /* end column */
          signed char forward) /* TRUE - forward, FALSE backward */
{
  /*
   * Forward search -
   * start line/col must be before end line/col
   *
   * Backward search -
   * start line/col must be after end line/col
   */
  if(start_line == end_line && start_col == end_col)
    /*
     * No search on the last position!
     */
    return FALSE;

  text->pos.data = text; /* set up pointer */
  
  text->pos.ptr = (unsigned char *)&array[start_line].text [start_col];
  text->pos.line = start_line;
  text->pos.finnished=FALSE;
  text->buf_start = array[0].text;
  if (!(array[lines-1].length)) /* Don't look at the last line if it's empty. */
    lines--;
  text->buf_end = array[lines-1].text + array[lines-1].length;
  text->text = array;
  text->lines = lines;

  if(forward) {
    text->start_line = start_line;
    text->start_col = start_col;
    text->end_line = end_line;
    text->end_col = end_col;
  }
  else {
    text->start_line = end_line;
    text->start_col = end_col;
    text->end_line = start_line;
    text->end_col = start_col;
  }

  text->pos.right = array[start_line].length - start_col;
  text->pos.col = start_col;
  text->pos.searchinfo=searchinfo;
  if(end_line == start_line) {
    if(forward) {
      text->pos.right = text->end_col - text->pos.col;
    }
    else {
      text->pos.right = 0;
      text->pos.col -= text->start_col;
    }
  }
  return TRUE;
}

STATIC INLINE signed char REGARGS
PointAdd1(struct re_position *now)
{
  if(--now->right>0) {
    now->ptr++;
    return TRUE;
  }
  return PointTo(now, 1);
}

STATIC INLINE signed char REGARGS
PointSub1(struct re_position *now)
{
  if(now->col-->0) {
    now->ptr--;
    return TRUE;
  }
  return PointTo(now, -1);
}

STATIC signed char REGARGS
PointTo(struct re_position *now,
        int change)
{
  register int a = now->line;
  signed char returnvalue=TRUE;
  int column = (int) (now->ptr - now->data->text[a].text);
  if(change>0) {  

    if(a >= now->data->end_line) {
      if(a > now->data->end_line)
        return FALSE;
      returnvalue=FALSE; // move foward, but return error anyway (last char)
    }
    column += change;

    if(column >= (int)now->data->text[a].length) {
      column -= now->data->text[a].length;
      change = column;
      a++;
      if (!(now->data->searchflags&INSIDE_FOLD)) {
        while (a<=now->data->end_line && (now->data->text[a].flags&FOLD_HIDDEN)) {
          a++;
          if(a > now->data->end_line)
            return FALSE;
        }
      }
      if(a > now->data->end_line || !now->data->text[a].length) {
//        if(a > now->data->end_line)
          a--;
        now->col=now->data->text[a].length;
        if (now->col) { /* 961012 */
          now->col++;
          now->ptr++;
        }
        now->right=1;
        if (now->finnished)
          return FALSE;
        now->finnished=TRUE;
        return returnvalue;
//        change = now->data->text[a].length - now->data->end_col;
      }
    }
    now->right = now->data->text[a].length - change; /* chars left to scan to the right */
    now->col = change;

  }
  else if(change < 0) {
    if(!column) {
      a--;
      if (!(now->data->searchflags&INSIDE_FOLD)) {
        while (a<=now->data->start_line && (now->data->text[a].flags&FOLD_HIDDEN))
          a--;
      }
      if( a< now->data->start_line )
        return FALSE;
      column = now->data->text[a].length-1;
      change = column;
      if(a == now->data->start_line) {
        change = now->data->text[a].length - now->data->start_col;
      }
    }
    now->right = now->data->text[a].length - change; /* chars left to scan to the right */
    now->col = change;
    if((signed int)now->col<0)
      /*
       * This is a negative == !allowed column!
       */
     return FALSE;
  }
  else {
    change = column;
    now->right = now->data->text[a].length - change; /* chars left to scan to the right */
    now->col = change;

    if(a == now->data->end_line) {
      now->right = now->data->end_col - column + 1;
	  if(column < 0)
		return FALSE;
    }
    if(a == now->data->start_line) {
      now->col = column - now->data->start_col;
    }
  }
  
  now->line = a;
  now->ptr = now->data->text[a].text + column;
  now->finnished=FALSE;

  return returnvalue;
}

#define COLUMN(x) ((x)->ptr - (x)->data->text[(x)->line].text)

#ifndef bcopy
#define bcopy(x,y,z) memcpy((y),(x),(z))
#endif

/* Maximum size of failure stack.  Beyond this, overflow is an error.  */

#define RE_MAX_FAILURES 2000

/* To test, compile without -DFREXXED.
 This testable feature turns this into a self-contained program
 which reads a pattern, describes how it compiles,
 then reads a string and searches for it.  */

#ifndef FREXXED

/*
 * Define the syntax stuff, so we can do the \<...\> things.
 */

#ifndef Sword /* must be non-zero in some of the tests below... */
#define Sword 1
#endif

#define SYNTAX(c) re_syntax_table[c]

STATIC unsigned char re_syntax_table[256];

STATIC void REGARGS
init_syntax_once (void)
{
   int c;
   static int done = 0;

   if (done)
     return;

   memset (re_syntax_table, 0, sizeof re_syntax_table);

   for (c = 'a'; c <= 'z'; c++)
     re_syntax_table[c] = Sword;

   for (c = 'A'; c <= 'Z'; c++)
     re_syntax_table[c] = Sword;

   for (c = '0'; c <= '9'; c++)
     re_syntax_table[c] = Sword;

   done = 1;
}

#endif

/* Number of failure points to allocate space for initially,
 when matching.  If this number is exceeded, more space is allocated,
 so it is not a hard limit.  */

#ifndef NFAILURES
#define NFAILURES 80
#endif /* NFAILURES */

/* width of a byte in bits */

#define BYTEWIDTH 8

#ifndef SIGN_EXTEND_CHAR
#define SIGN_EXTEND_CHAR(x) (x)
#endif

/* re_compile_pattern takes a regular-expression string
   and converts it into a buffer full of byte commands for matching.

  PATTERN   is the address of the pattern string (zero terminated)
  BUFP	    is a  struct re_pattern_buffer *  which points to the info
	    on where to store the byte commands.
	    This structure contains a  char *  which points to the
	    actual space, which should have been obtained with Malloc.
	    re_compile_pattern may use  realloc  to grow the buffer space.

  The number of bytes of commands can be found out by looking in
  the  struct re_pattern_buffer  that bufp pointed to,
  after re_compile_pattern returns.
*/

#define PATPUSH(ch) (*b++ = (signed char) (ch))

#define PATFETCH(c) \
 {if (!*p) \
    return RE_PREMATURE_END_OF_RE; \
  c = * (unsigned char *) p++; \
  if (fact && (fact->xflags[c]&factx_UPPERCASE)) \
    c = fact->cases[c]; }

#define EXTEND_BUFFER \
  { signed char *old_buffer = bufp->buffer;   \
    if (bufp->allocated == (1<<16))    \
      return RE_TOO_BIG;               \
    bufp->allocated *= 2;              \
    if (bufp->allocated > (1<<16))     \
      bufp->allocated = (1<<16);       \
    if (!(bufp->buffer = (signed char *) Remalloc (bufp->buffer, bufp->allocated))) \
      return RE_OUT_OF_MEMORY;         \
    c = bufp->buffer - old_buffer;     \
    b += c;                            \
    if (fixup_jump)                    \
      fixup_jump += c;                 \
    if (laststart)                     \
      laststart += c;                  \
    begalt += c;                       \
    if (pending_exact)                 \
      pending_exact += c;              \
  }

re_RetCode REGARGS
re_compile_pattern (signed char *pattern,
		    struct re_pattern_buffer *bufp)
{
  signed char *b = bufp->buffer;
  signed char *p = pattern;
  unsigned c, c1;
  signed char *p1;
  FACT *fact= bufp->fact;
  
  /* address of the count-byte of the most recently inserted "exactn" command.
     This makes it possible to tell whether a new exact-match character
     can be added to that command or requires a new "exactn" command. */
  
  signed char *pending_exact = 0;
  
  /* address of the place where a forward-jump should go
     to the end of the containing expression.
     Each alternative of an "or", except the last, ends with a forward-jump
     of this sort. */
  
  signed char *fixup_jump = 0;
  
  /* address of start of the most recently finished expression.
     This tells postfix * where to find the start of its operand. */
  
  signed char *laststart = 0;
  
  /* In processing a repeat, 1 means zero matches is allowed */
  
  signed char zero_times_ok;
  
  /* In processing a repeat, 1 means many matches is allowed */
  
  signed char many_times_ok;
  
  /* address of beginning of regexp, or inside of last \( */
  
  signed char *begalt = b;
  
  /* Stack of information saved by \( and restored by \).
     Four stack elements are pushed by each \(:
     First, the value of b.
     Second, the value of fixup_jump.
     Third, the value of regnum.
     Fourth, the value of begalt.  */
  
  int stackb[40];
  int *stackp = stackb;
  int *stacke = stackb + 40;
  int *stackt;
  
  /* Counts \('s as they are encountered.  Remembered for the matching \),
     where it becomes the "register number" to put in the stop_memory command */
  
  int regnum = 1;
  
  bufp->fastmap_accurate = 0;
  
#ifndef FREXXED
  /*
   * Initialize the syntax table.
   */
  init_syntax_once();
#endif
  
  if (bufp->allocated == 0) {
    bufp->allocated = 28;
    if (bufp->buffer)
      /* EXTEND_BUFFER loses when bufp->allocated is 0 */
      bufp->buffer = (signed char *) Remalloc (bufp->buffer, 28);
    else
      /* Caller did not allocate a buffer.  Do it for him */
      bufp->buffer = (signed char *) Malloc (28);
    if (!bufp->buffer)
      return RE_OUT_OF_MEMORY;
    begalt = b = bufp->buffer;
  }
  
  while (*p) {   /* (p != pend) */
    if (b - bufp->buffer > bufp->allocated - 10)
      /* Note that EXTEND_BUFFER clobbers c */
      EXTEND_BUFFER;
    
    PATFETCH (c);
    
    switch (c) {
    case '$':
      /* $ means succeed if at end of line, but only in special contexts.
         If randomly in the middle of a pattern, it is a normal character. */
      if (!*p || ISNEWLINE(*p)
          || *p == ')'
          ||  *p == '|') {
        PATPUSH (endline);
        break;
      }
      goto normal_char;
      
    case '^':
      /* ^ means succeed if at beg of line,
         but only if no preceding pattern. */
      if (laststart && !ISNEWLINE(p[-2]))
        goto normal_char;
      else
        PATPUSH (begline);
      break;
      
    case '+':
    case '?':
      /* fall through */
    handle_plus:
    case '*':
      /* If there is no previous pattern, char not special. */
      if (!laststart)
        goto normal_char;

      /* If there is a sequence of repetition chars,
         collapse it down to equivalent to just one.  */
      zero_times_ok = 0;
      many_times_ok = 0;
      while (1) {
        zero_times_ok |= c != '+';
        many_times_ok |= c != '?';
        if (!*p)
          break;
        PATFETCH (c);
        if (c == '*')
          ;
        else if (c == '+' || c == '?')
          ;
        else {
          p--;
          break;
        }
      }

      /* Star, etc. applied to an empty pattern is equivalent
         to an empty pattern.  */
      if (!laststart)
        break;
      
      /* Now we know whether 0 matches is allowed,
         and whether 2 or more matches is allowed.  */
      if (many_times_ok) {
        /* If more than one repetition is allowed,
           put in a backward jump at the end.  */
        store_jump (b, maybe_finalize_jump, laststart - 3);
        b += 3;
      }
      insert_jump (on_failure_jump, laststart, b + 3, b);
      pending_exact = 0;
      b += 3;
      if (!zero_times_ok) {
        /* At least one repetition required: insert before the loop
           a skip over the initial on-failure-jump instruction */
        insert_jump (dummy_failure_jump, laststart, laststart + 6, b);
        b += 3;
      }
      break;
      
    case '.':
      laststart = b;
      PATPUSH (anychar);
      break;
    case '#':
      pending_exact = 0;
      do {
        PATFETCH(c);
      } while (c=='?');
      p--;
      if (c)
        insert_jump (on_failure_continue_on_next, b, b, b);
      b += 3;
//      PATPUSH (grid);
      break;
      
    case '[':
      while (b - bufp->buffer
             > bufp->allocated - 3 - (1 << BYTEWIDTH) / BYTEWIDTH)
        /* Note that EXTEND_BUFFER clobbers c */
        EXTEND_BUFFER;
      
      laststart = b;
      if (*p == '^')
        PATPUSH (charset_not), p++;
      else
        PATPUSH (charset);
      p1 = p;
      
      PATPUSH ((1 << BYTEWIDTH) / BYTEWIDTH);
      /* Clear the whole map */
      memset (b, 0, (1 << BYTEWIDTH) / BYTEWIDTH);
      /* Read in characters and ranges, setting map bits */
      while (1) {
        PATFETCH (c);
        if (c == ']' && p != p1 + 1) break;
        if (*p == '-' && p[1] != ']') {
          PATFETCH (c1);
          PATFETCH (c1);
          while (c <= c1)
            b[c / BYTEWIDTH] |= 1 << (c % BYTEWIDTH), c++;
        }
        else {
          b[c / BYTEWIDTH] |= 1 << (c % BYTEWIDTH);
        }
      }
      /* Discard any bitmap bytes that are all 0 at the end of the map.
         Decrement the map-length byte too. */
      while ((int) b[-1] > 0 && b[b[-1] - 1] == 0)
        b[-1]--;
      b += b[-1];
      break;
      
    case '(':
      if (stackp == stacke)
        return RE_NESTING_TOO_DEEP;
      if (regnum < RE_NREGS) {
        PATPUSH (start_memory);
        PATPUSH (regnum);
      }
      *stackp++ = b - bufp->buffer;
      *stackp++ = fixup_jump ? fixup_jump - bufp->buffer + 1 : 0;
      *stackp++ = regnum++;
      *stackp++ = begalt - bufp->buffer;
      fixup_jump = 0;
      laststart = 0;
      begalt = b;
      break;
              
    case ')':
      if (stackp == stackb)
        return RE_UNMATCHED_CLOSE_PAREN;
      begalt = *--stackp + bufp->buffer;
      if (fixup_jump)
        store_jump (fixup_jump, jump, b);
      if (stackp[-1] < RE_NREGS) {
        PATPUSH (stop_memory);
        PATPUSH (stackp[-1]);
      }
      stackp -= 2;
      fixup_jump = 0;
      if (*stackp)
        fixup_jump = *stackp + bufp->buffer - 1;
      laststart = *--stackp + bufp->buffer;
      break;
      
    case '|':
      insert_jump (on_failure_jump, begalt, b + 6, b);
      pending_exact = 0;
      b += 3;
      if (fixup_jump)
        store_jump (fixup_jump, jump, b);
      fixup_jump = b;
      b += 3;
      laststart = 0;
      begalt = b;
      break;
      
    case '\\':
    case '/':
      if (!*p)
        return RE_INVALID_REGULAR_EXPRESSION;
      c = * (unsigned char *) p++;
      switch (c) {

#ifdef emacs
      case '=':
        PATPUSH (at_dot);
        break;
#endif /* emacs */
#ifdef FREXXED
      case 's':	
        laststart = b;
        PATPUSH (syntaxspec);
        PATFETCH (c);
        PATPUSH (SetChar(c));
        break;
        
      case 'S':
        laststart = b;
        PATPUSH (notsyntaxspec);
        PATFETCH (c);
        PATPUSH (SetChar(c));
        break;
#endif
      case 'w':
        laststart = b;
        PATPUSH (wordchar);
        break;
        
      case 'W':
        laststart = b;
        PATPUSH (notwordchar);
        break;
        
      case '<':
        PATPUSH (wordbeg);
        break;
        
      case '>':
        PATPUSH (wordend);
        break;
        
      case 'b':
        PATPUSH (wordbound);
        break;
        
      case 'B':
        PATPUSH (notwordbound);
        break;
        
      case '`':
        PATPUSH (begbuf);
        break;
        
      case '\'':
        PATPUSH (endbuf);
        break;
        
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        c1 = c - '0';
        if (c1 >= regnum)
          goto normal_char;
        for (stackt = stackp - 2;  stackt > stackb;  stackt -= 4)
          if (*stackt == c1)
            goto normal_char;
        laststart = b;
        PATPUSH (duplicate);
        PATPUSH (c1);
        break;
        
      case 'x':
      case 'X':
        {
          int a;
          for(a=c=0; *p && isxdigit(*p) && a<2; a++, p++) {
            c = (c<<4) + (isdigit(*p)?*p-'0':toupper(*p)-'A'+10);
          }
        }
        goto normal_char;
        
      case '0':
        {
          int a;
          for(a=c=0; *p && isodigit(*p) && a<3; a++, p++) {
            c = ((c<<3) + (*p - '0'))&255;
          }
        }
        goto normal_char;
        
      case 't':
        c = '\t';
        goto normal_char;
        
      case 'n':
        c = '\n';
        goto normal_char;
        
      case '+':
      case '?':
        /* fall through! */
        
      default:
      normal_backsl:
        /* You might think it would be useful for \ to mean
           not to translate; but if we don't translate it
           it will never match anything.  */
        if (fact && (fact->xflags[c]&factx_UPPERCASE))
          c = fact->cases[c];
        goto normal_char;
      }
      break;
      
    default:
    case '\n':
    normal_char:
      if (!pending_exact || pending_exact + *pending_exact + 1 != b
          || *pending_exact == 0177 || *p == '*' || *p == '^'
          || (*p == '+' || *p == '?')) {
        laststart = b;
        PATPUSH (exactn);
        pending_exact = b;
        PATPUSH (0);
      }
      PATPUSH (c);
      (*pending_exact)++;
    }
  }
  
  if (fixup_jump)
    store_jump (fixup_jump, jump, b);
  
  if (stackp != stackb)
    return RE_UNMATCHED_OPEN_PAREN;
  
  bufp->used = b - bufp->buffer;
  return RE_OK;

}

/* Store where `from' points a jump operation to jump to where `to' points.
  `opcode' is the opcode to store. */

void REGARGS STATIC
store_jump (signed char *from, signed char opcode, signed char *to)
{
  from[0] = opcode;
  from[1] = (to - (from + 3)) & 0377;
  from[2] = (to - (from + 3)) >> 8;
}

/* Open up space at char FROM, and insert there a jump to TO.
   CURRENT_END gives te end of the storage no in use,
   so we know how much data to copy up.
   OP is the opcode of the jump to insert.

   If you call this function, you must zero out pending_exact.  */

void REGARGS STATIC
insert_jump (signed char op, signed char *from, signed char *to, signed char *current_end)
{
  signed char *pto = current_end + 3;
  signed char *pfrom = current_end;
  while (pfrom != from)
    *--pto = *--pfrom;
  store_jump (from, op, to);
}

/* Given a pattern, compute a fastmap from it.
   The fastmap records which of the (1 << BYTEWIDTH) possible characters
   can start a string that matches the pattern.
   This fastmap is used by re_search to skip quickly over totally implausible
   text.

   The caller must supply the address of a (1 << BYTEWIDTH)-byte data area
   as bufp->fastmap.
   The other components of bufp describe the pattern to be used.  */

void REGARGS
re_compile_fastmap (bufp)
     struct re_pattern_buffer *bufp;
{
  signed char *fastmap = bufp->fastmap;
  unsigned char *p = (unsigned char *) bufp->buffer;
  unsigned char *pend = p + bufp->used;
  int j;
#ifdef FREXXED
  int k;
#endif
  FACT *fact= bufp->fact;
  
  unsigned char *stackb[NFAILURES];
  unsigned char **stackp = stackb;
  
  memset (fastmap, 0, (1 << BYTEWIDTH));
  bufp->fastmap_accurate = 1;
  bufp->can_be_null = 0;
  
  while (p) {
    if (p == pend) {
      bufp->can_be_null = 1;
      break;
    }
    switch ((enum regexpcode) *p++) {
    case exactn:
      fastmap[p[1]] = 1;
      if (fact && (fact->xflags[p[1]]&(factx_UPPERCASE|factx_LOWERCASE)))
        fastmap[fact->cases[p[1]]] = 1;
      break;
      
    case begline:
    case before_dot:
    case at_dot:
    case after_dot:
    case begbuf:
    case endbuf:
    case wordbound:
    case notwordbound:
    case wordbeg:
    case wordend:
      continue;
      
    case endline:
      {
        register int count;
        for (count=0; count<256; count++) {
          if (ISNEWLINE(count)) {
            fastmap[count] = 1;
            if (fact && (fact->xflags[count]&(factx_UPPERCASE|factx_LOWERCASE)))
              fastmap[fact->cases[count]] = 1;
          }
        }
      }
      if (bufp->can_be_null != 1)
        bufp->can_be_null = 2;
      break;
      
    case finalize_jump:
    case maybe_finalize_jump:
    case jump:
    case dummy_failure_jump:
      bufp->can_be_null = 1;
      j = *p++ & 0377;
      j += SIGN_EXTEND_CHAR (*(signed char *)p) << 8;
      p += j + 1;		/* The 1 compensates for missing ++ above */
      if (j > 0)
        continue;
      /* Jump backward reached implies we just went through
         the body of a loop and matched nothing.
         Opcode jumped to should be an on_failure_jump.
         Just treat it like an ordinary jump.
         For a * loop, it has pushed its failure point already;
         if so, discard that as redundant.  */
      if ((enum regexpcode) *p != on_failure_jump)
        continue;
      p++;
      j = *p++ & 0377;
      j += SIGN_EXTEND_CHAR (*(signed char *)p) << 8;
      p += j + 1;		/* The 1 compensates for missing ++ above */
      if (stackp != stackb && *stackp == p)
        stackp--;
      continue;
      
    case on_failure_jump:
      j = *p++ & 0377;
      j += SIGN_EXTEND_CHAR (*(signed char *)p) << 8;
      p++;
      *++stackp = p + j;
      continue;
      
    case start_memory:
    case stop_memory:
      p++;
      continue;
      
    case duplicate:
      bufp->can_be_null = 1;
      fastmap['\n'] = 1;
    case anychar:
      for (j = 0; j < (1 << BYTEWIDTH); j++)
        if (!ISNEWLINE(j))
          fastmap[j] = 1;
      if (bufp->can_be_null)
        return;
      /* Don't return; check the alternative paths
         so we can set can_be_null if appropriate.  */
      break;

    case grid:
      for (j = 0; j < (1 << BYTEWIDTH); j++) {
//        if (!ISNEWLINE(j))
          fastmap[j] = 1;
      }
      if (bufp->can_be_null)
        return;
      /* Don't return; check the alternative paths
         so we can set can_be_null if appropriate.  */
      break;

    case wordchar:
      for (j = 0; j < (1 << BYTEWIDTH); j++)
        if (ISWORD(j))
          fastmap[j] = 1;
      break;
      
    case notwordchar:
      for (j = 0; j < (1 << BYTEWIDTH); j++)
        if (ISNWORD(j))
          fastmap[j] = 1;
      break;
      
#ifdef FREXXED
    case syntaxspec:
      k = *p++;
      for (j = 0; j < (1 << BYTEWIDTH); j++)
        if (CheckChar(j, k))
          fastmap[j] = 1;
      break;
      
    case notsyntaxspec:
      k = *p++;
      for (j = 0; j < (1 << BYTEWIDTH); j++)
        if (!CheckChar(j, k))
          fastmap[j] = 1;
      break;
#endif
      
    case charset:
      for (j = *p++ * BYTEWIDTH - 1; j >= 0; j--)
        if (p[j / BYTEWIDTH] & (1 << (j % BYTEWIDTH))) {
          fastmap[j] = 1;
          if (fact && (fact->xflags[j]&(factx_UPPERCASE|factx_LOWERCASE)))
            fastmap[fact->cases[j]] = 1;
        }
      break;
      
    case charset_not:
      /* Chars beyond end of map must be allowed */
      for (j = *p * BYTEWIDTH; j < (1 << BYTEWIDTH); j++)
        fastmap[j] = 1;
        if (fact && (fact->xflags[j]&(factx_UPPERCASE|factx_LOWERCASE)))
          fastmap[fact->cases[j]] = 1;
      
      for (j = *p++ * BYTEWIDTH - 1; j >= 0; j--)
        if (!(p[j / BYTEWIDTH] & (1 << (j % BYTEWIDTH)))) {
          fastmap[j] = 1;
          if (fact && (fact->xflags[j]&(factx_UPPERCASE|factx_LOWERCASE)))
            fastmap[fact->cases[j]] = 1;
        }
      break;
    }
    
    /* Get here means we have successfully found the possible starting characters
       of one path of the pattern.  We need not follow this path any farther.
       Instead, look at the next alternative remembered in the stack. */
    if (stackp != stackb)
      p = *stackp--;
    else
      break;
  }
}

int REGARGS
re_search (pbufp, range, regs, p)
     struct re_pattern_buffer *pbufp;
     int range;
     struct re_registers *regs;
     struct re_position *p;
{
  
  /* RANGE is the number of places to try before giving up.
     If RANGE is negative, the starting positions tried are
     STARTPOS, STARTPOS - 1, etc.
     
     The value returned is the position at which the match was found,
     or -1 if no match was found,
     or -2 if error (such as failure stack overflow).  */
  
  signed char *fastmap = pbufp->fastmap;
  FACT *fact= pbufp->fact;
  int val;
  struct re_position pos;
  
  /* Update the fastmap now if not correct already */
  if (fastmap && !pbufp->fastmap_accurate)
    re_compile_fastmap (pbufp);
  
  /* Don't waste time in a long search for a pattern
     that says it is anchored.  */
  if (pbufp->used > 0 && (enum regexpcode) pbufp->buffer[0] == begbuf
      && range > 0) {
    range = 1;
  }
#ifdef FREXXED
  else if(range>0 && !p->searchinfo->find_direct) {
    if(!PointAdd1(p))
      return -1;
    range--;
  } else if(range<0) {
    if(!PointSub1(p))
      return -1;
    range++;
  }
#endif
  while (1) {
    /* If a fastmap is supplied, skip quickly over characters
       that cannot possibly be the start of a match.
       Note, however, that if the pattern can possibly match
       the null string, we must test it at each starting point
       so that we take the first null string we get.  */
    
    if (fastmap && pbufp->can_be_null != 1) {
      if (range > 0) {
        if (fact) {
          while(!fastmap[*p->ptr]) {
            if(!PointAdd1(p) ||
               !--range) {
              return -1;
            }
          }
        }
        else {
          while(!fastmap[*p->ptr]) {
            if(!PointAdd1(p) ||
               !--range) {
              return -1;
            }
          }
        }
      }
      else if (range<0) {
        if (fact) {
          while(!fastmap[*p->ptr]) {
            if(!PointSub1(p) ||
               !++range) {
              return -1;
            }
          }
        }
        else {
          while(!fastmap[*p->ptr]) {
            if(!PointSub1(p) ||
               !++range) {
              return -1;
            }
          }
        }
        if(!PointTo(p, 0)) /* reset the struct ->right value! */
		  break;
      }
    }
    
    if (p->finnished || (range >= 0 && !p->ptr && fastmap && pbufp->can_be_null == 0))
      return -1;
    pos = *p;
    val = re_match (pbufp, regs, &pos);
    
    /* Propagate error indication if worse than mere failure.  */
    if (val == -2)
      return -2;
    /* Return position on success.  */
    if (0 <= val)
      return 0;
    
    if (!range)
      break;
    if (range > 0) {
      range--;
      if (-3 == val)	// quit if no possibility to match
        return -1;
      if(!PointAdd1(p))
        break;
    }
    else {
      range++;
      if (-3 == val) {	// smart match if no possibility to match
        if (p->data->end_line!=p->line) {
          p->data->end_line=p->line+1;
          p->data->end_col=0;
        }
      }
      if(!PointSub1(p))
        break;
    }
  }
  return -1;
}

#define NO_RETRY 2

int REGARGS
re_match (pbufp, regs, d)
     struct re_pattern_buffer *pbufp;
     struct re_registers *regs;
     struct re_position *d;
{
  /* Match the pattern described by PBUFP
     
     If pbufp->fastmap is nonzero, then it had better be up to date.
     
     -1 is returned if there is no match.
     -2 is returned if there is an error (such as match stack overflow).
     -3 is returned if there is possibility to retry a match.
     Otherwise the value is the length of the substring which was matched.  */
  unsigned char add1=FALSE;
  
  unsigned char *p = (unsigned char *) pbufp->buffer;
  unsigned char *pend = p + pbufp->used;
  
  unsigned char dunk;
  
  int mcnt;
  FACT *fact= pbufp->fact;
  
  /* Failure point stack.  Each place that can handle a failure further down
     the line pushes a failure point on this stack.  It consists of two
     char *'s and one int.
     The first one pushed is where to resume scanning the pattern;
     the second pushed is where to resume scanning the strings and the third
     is the line number to resume scanning on.
     If the data pointer is zero, the failure point is a "dummy".
     If a failure happens and the innermost failure point is dormant,
     it discards that failure point and tries the next one. */
  
  unsigned char *initial_stack[3 * NFAILURES];
  unsigned char **stackb = initial_stack;
  unsigned char **stackp = stackb, **stacke = &stackb[3 * NFAILURES];
  signed char stackb_malloc=0;
  
  /* Information on the "contents" of registers.
     These are pointers into the input strings; they record
     just what was matched (on this attempt) by some part of the pattern.
     The start_memory command stores the start of a register's contents
     and the stop_memory command stores the end.
     
     At that point, regstart[regnum] points to the first character in the register,
     regend[regnum] points to the first character beyond the end of the register,
     regstart_seg1[regnum] is true iff regstart[regnum] points into string1,
     and regend_seg1[regnum] is true iff regend[regnum] points into string1.  */
  
  unsigned char *regstart[RE_NREGS];
  unsigned char *regend[RE_NREGS];
  
  unsigned int regstart_line[RE_NREGS], regend_line[RE_NREGS];
  struct re_position startd;

  signed char failed=FALSE;
  
  /* Initialize \) text positions to -1
     to mark ones that no \( or \) has been seen for.  */
  
  for (mcnt = 0; mcnt < sizeof (regend) / sizeof (*regend); mcnt++)
    regend[mcnt] = (unsigned char *) -1;
  
  /* `p' scans through the pattern as `d' scans through the data. */
  
  startd = *d;
  
  /* This loop loops over pattern commands.
     It exits by returning from the function if match is complete,
     or it drops through if match fails at this starting point in the input data. */
  
  while (1) {
    if (p == pend) {
      /* End of pattern means we have succeeded! */
      /* If caller wants register contents data back,
         convert it to indices */
      if (regs) {
        regs->start[0] = (int)(startd.ptr - startd.data->text[startd.line].text);
        regs->start_line[0] = startd.line;
        regs->end[0] = (int)(d->ptr - d->data->text[d->line].text);
        regs->end_line[0] = d->line;
DEBUG("Start1", regs->start[mcnt]);
DEBUG("End1", regs->end[mcnt]);
        for (mcnt = 1; mcnt < RE_NREGS; mcnt++) {
          if (regend[mcnt] == (unsigned char *) -1) {
            regs->start[mcnt] = -1;
            regs->end[mcnt] = -1;
            continue;
          }
          regs->start[mcnt] = (int)(regstart[mcnt] -
                                    d->data->text[regstart_line[mcnt]].text);
          regs->start_line[mcnt] = regstart_line[mcnt];
          regs->end[mcnt] = (int)(regend[mcnt] -
                                  d->data->text[regend_line[mcnt]].text);
DEBUG("End2", regs->end[mcnt]);
          regs->end_line[mcnt] = regend_line[mcnt];
        }
        
      }
      if(stackb_malloc)
        Dealloc(stackb);
      return (int)d->ptr;
      
    }
    if (add1) {
      add1=FALSE;
DEBUG("PointAdd", (int)(d->ptr - d->data->text[d->line].text));
      if (!PointAdd1(d))
        goto fail;
    }
    
    /* Otherwise match next pattern command */

    switch ((enum regexpcode) *p++) {
      
      /* \( is represented by a start_memory, \) by a stop_memory.
         Both of those commands contain a "register number" argument.
         The text matched within the \( and \) is recorded under that number.
         Then, \<digit> turns into a `duplicate' command which
         is followed by the numeric value of <digit> as the register number. */
      
    case start_memory:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      regstart[*p] = d->ptr;
      regstart_line[*p] = d->line;
      p++;
      break;
      
    case stop_memory:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      regend[*p] = d->ptr;
      regend_line[*p] = d->line;
      p++;
      break;
      
    case duplicate:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      {
        int regno = *p++;   /* Get which register to match against */
        struct re_position d2 = *d;
        unsigned char *dend2;
        unsigned int dend2line;
        
        /* Don't allow matching a register that hasn't been used.
           This isn't fully reliable in the current version,
           but it is better than crashing.  */
        /* Convex fix:  only check for equal to -1 */
        if ((int) regend[regno] == -1) {
          failed=TRUE;
          break;
        }
        d2.ptr = regstart[regno];
        d2.line = regstart_line[regno];
        
        dend2 = regend[regno];
        dend2line = regend_line[regno];
        
        while (1) {
          /* At end of register contents => success */
          if (d2.ptr == dend2)
            break;
          
          if(dend2line != d2.line) {
            mcnt = d2.data->text[d2.line].text + d2.data->text[d2.line].length -
              d2.ptr;
          }
          else
            mcnt = dend2 - d2.ptr;
          
          /* Compare that many; failure if mismatch, else skip them. */
          
          if (bcmp_translate (d->ptr, d2.ptr, mcnt, fact)) {
            failed=TRUE;
            break;
          }
	  /*
	   * These following PointTo()s don't change line!
	   */
          PointTo(&d2, mcnt);
          if(!PointTo(d, mcnt)) {
            failed=TRUE;
            break;
          }
        }
      }
      break;
      
    case anychar:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      /* Match anything but a newline.  */
      if (ISNEWLINE(*d->ptr))
        failed=TRUE;
#if 1 //960917
      if (!PointAdd1(d))
        goto fail;
#else
      add1=TRUE;
#endif
      break;

    case charset:
    case charset_not:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      {
        /* Nonzero for charset_not */
        int not = 0;
        register int c;
        if (*(p - 1) == (unsigned char) charset_not)
          not = 1;
        
        c = *d->ptr;
        if (fact && (fact->xflags[c]&factx_UPPERCASE))
          c = fact->cases[c];

        if (c < *p * BYTEWIDTH
            && p[1 + c / BYTEWIDTH] & (1 << (c % BYTEWIDTH)))
          not = !not;
        
        p += 1 + *p;
        
        if (!not)
          failed=TRUE;
        add1=TRUE;
        break;
      }
      
    case begline:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if(COLUMN(d) > 0 || d->line > 0) {
	dunk = (COLUMN(d)>0?
		d->ptr[-1]:
		d->data->text[d->line-1].text [ d->data->text[d->line-1].length -1 ] );
	if (!ISNEWLINE(dunk))
	  failed=TRUE;
      }
      else if(COLUMN(d)<0)
        failed = TRUE;
      break;
      
    case endline:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if(!ISNEWLINE(*d->ptr))
	failed=TRUE;
      break;
      
      /* "or" constructs ("|") are handled by starting each alternative
         with an on_failure_jump that points to the start of the next alternative.
         Each alternative except the last ends with a jump to the joining point.
         (Actually, each jump except for the last one really jumps
         to the following jump, because tensioning the jumps is a hassle.) */
      
      /* The start of a stupid repeat has an on_failure_jump that points
         past the end of the repeat text.
         This makes a failure point so that, on failure to match a repetition,
         matching restarts past as many repetitions have been found
         with no way to fail and look for another one.  */
      
      /* A smart repeat is similar but loops back to the on_failure_jump
         so that each repetition makes another failure point. */
      
    case on_failure_continue_on_next:
    case on_failure_jump:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if (stackp == stacke) {
        unsigned char **stackx;
        if ((stacke - stackb) > RE_MAX_FAILURES) {
          if(stackb_malloc)
            Dealloc(stackb);
          return -2;
        }
        stackx = (unsigned char **) Malloc (2 * (stacke - stackb)
                                            * sizeof (signed char *));
        bcopy (stackb, stackx, (stacke - stackb) * sizeof (signed char *));
        stackp = stackx + (stackp - stackb);
        stacke = stackx + 2 * (stacke - stackb);
        if(stackb_malloc)
          Dealloc(stackb);
        else
          stackb_malloc =1;
        stackb = stackx;
      }
      if (((enum regexpcode) *(p-1))==on_failure_continue_on_next)
        goto on_failure_continue_on_next__jump;
      mcnt = *p++ & 0377;
      mcnt += SIGN_EXTEND_CHAR (*(signed char *)p) << 8;
      p++;
      *stackp++ = mcnt + p;
      *stackp++ = d->ptr;
      *stackp++ = (unsigned char *)d->line;
      break;

    on_failure_continue_on_next__jump:
      mcnt = *p++ & 0377;
      mcnt += SIGN_EXTEND_CHAR (*(signed char *)p) << 8;
      p++;
      if (d->right==1) {
        if ((d->data->searchflags&LIMIT_WILDCARD)) {
          failed=TRUE;
          break;
        }
        if (d->line+1<d->data->end_line || d->data->end_col) {
          *stackp++ = mcnt + p;
          *stackp++ = d->data->text[d->line+1].text;
          *stackp++ = (unsigned char *)d->line+1;
        } else {
          failed=NO_RETRY;
          break;
        }
      } else {
        *stackp++ = mcnt + p;
        *stackp++ = d->ptr +1;
        *stackp++ = (unsigned char *)d->line;
      }
      break;
      
      /* The end of a smart repeat has an maybe_finalize_jump back.
         Change it either to a finalize_jump or an ordinary jump. */
      
    case maybe_finalize_jump:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      mcnt = *p++ & 0377;
      mcnt += SIGN_EXTEND_CHAR (*(signed char *)p) << 8;
      p++;
      /* Compare what follows with the begining of the repeat.
         If we can establish that there is nothing that they would
         both match, we can change to finalize_jump */
      if (p == pend)
        p[-3] = (unsigned char) finalize_jump;
      else if (*p == (unsigned char) exactn
               || *p == (unsigned char) endline) {
        int c = *p == (unsigned char) endline ? '\n' : p[2];
        unsigned char *p1 = p + mcnt;
        /* p1[0] ... p1[2] are an on_failure_jump.
           Examine what follows that */
        if (p1[3] == (unsigned char) exactn && p1[5] != c)
          p[-3] = (unsigned char) finalize_jump;
        else if (p1[3] == (unsigned char) charset
                 || p1[3] == (unsigned char) charset_not) {
          int not = p1[3] == (unsigned char) charset_not;
          if (c < p1[4] * BYTEWIDTH
              && p1[5 + c / BYTEWIDTH] & (1 << (c % BYTEWIDTH)))
            not = !not;
          /* not is 1 if c would match */
          /* That means it is not safe to finalize */
          if (!not)
            p[-3] = (unsigned char) finalize_jump;
        }
      }
      p -= 2;
      if (p[-1] != (unsigned char) finalize_jump) {
        p[-1] = (unsigned char) jump;
        goto nofinalize;
      }
      
      /* The end of a stupid repeat has a finalize-jump
         back to the start, where another failure point will be made
         which will point after all the repetitions found so far. */
      
    case finalize_jump:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      stackp -= 3;
      
    case jump:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
    nofinalize:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      mcnt = *p++ & 0377;
      mcnt += SIGN_EXTEND_CHAR (*(signed char *)p) << 8;
      p += mcnt + 1;	/* The 1 compensates for missing ++ above */
      break;
      
    case dummy_failure_jump:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if (stackp == stacke) {
        unsigned char **stackx
          = (unsigned char **) Malloc (2 * (stacke - stackb)
                                       * sizeof (signed char *));
        bcopy (stackb, stackx, (stacke - stackb) * sizeof (signed char *));
        stackp = stackx + (stackp - stackb);
        stacke = stackx + 2 * (stacke - stackb);
        if(stackb_malloc)
          Dealloc(stackb);
        else
          stackb_malloc=1;
        stackb = stackx;
      }
      *stackp++ = 0;
      *stackp++ = 0;
      *stackp++ = (unsigned char *)-1;
      goto nofinalize;
      
    case wordbound:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if (COLUMN(d) == 0 && d->line == 0)
        break;
      dunk = (COLUMN(d)>0?
              d->ptr[-1]:
              d->data->text[d->line-1].text [ d->data->text[d->line-1].length -1 ] );
      if ( (ISWORD(dunk)) != (ISWORD(*d->ptr)))
        break;
      failed=TRUE;
      break;
      
    case notwordbound:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if (COLUMN(d)== 0 && d->line == 0) {
        failed=TRUE;
      }
      else {
	dunk = (COLUMN(d)>0?
		d->ptr[-1]:
		d->data->text[d->line-1].text [ d->data->text[d->line-1].length -1 ] );
	if (ISWORD (dunk) != ISWORD(*d->ptr))
	    failed=TRUE;
      }
      break;
      
    case wordbeg:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if (ISNWORD(*d->ptr)) { /* Next char not a letter */
        failed=TRUE;
      }
      else {
	if(COLUMN(d) != 0 || d->line != 0) {
	  dunk = (COLUMN(d)>0?
		  d->ptr[-1]:
		  d->data->text[d->line-1].text [ d->data->text[d->line-1].length -1 ] );
	  if (ISWORD(dunk))  /* prev char was letter */
	    failed=TRUE;
	}
      }
      break;
      
    case wordend:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if(COLUMN(d) != 0 || d->line != 0) {
	dunk = (COLUMN(d)>0?
		d->ptr[-1]:
		d->data->text[d->line-1].text [ d->data->text[d->line-1].length -1 ] );
	if (ISNWORD(dunk))  /* prev char not letter */
	  ;
	else if (ISNWORD(*d->ptr)) /* Next char not a letter */
	  break;
      }
      failed=TRUE;
      break;
      
#ifdef emacs
    case before_dot:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if (PTR_CHAR_POS (d) + 1 >= point)
        failed=TRUE;
      break;
      
    case at_dot:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if (PTR_CHAR_POS (d) + 1 != point) {
        failed=TRUE;
      }
      break;
      
    case after_dot:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if (PTR_CHAR_POS (d) + 1 <= point) {
        failed=TRUE;
      }
      break;
      
    case wordchar:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      mcnt = (int) Sword;
      goto matchsyntax;

    case notwordchar:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      mcnt = (int) Sword;
      goto matchnotsyntax;

#else      
#ifdef FREXXED
    case syntaxspec:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      failed = TRUE; /* to start toggle */
    case notsyntaxspec:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      mcnt = *p++;
      failed ^= CheckChar(*d->ptr, mcnt);
//      if(!failed)
//        failed=TRUE;
      add1=TRUE;
      break;
#endif
    case wordchar:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if (ISNWORD(*d->ptr))
	failed=TRUE;
      add1=TRUE;
      break;
      
    case notwordchar:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if (ISWORD(*d->ptr))
        failed=TRUE;
      add1=TRUE;
      break;
#endif /* not emacs */
      
    case begbuf:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if(d->ptr != d->data->buf_start)
	failed=TRUE;
      break;
      
    case endbuf:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      if (d->ptr != d->data->buf_end)
	failed=TRUE;
      break;
      
    case exactn:
DEBUG("Case", (int)(d->ptr - d->data->text[d->line].text));
      /* Match the next few pattern characters exactly.
         mcnt is how many characters to match. */
      mcnt = *p++;
      if (fact) {
        do {
DEBUG("Loop", (int)(d->ptr - d->data->text[d->line].text));
DEBUG("PointAdd", (int)(d->ptr - d->data->text[d->line].text));
          if ((*d->ptr!=*p &&
               (!(fact->xflags[*d->ptr]&(factx_UPPERCASE|factx_LOWERCASE)) || (fact->cases[*d->ptr] != *p)))
              || (!PointAdd1(d) && mcnt!=1)) {
DEBUG("failed", (int)(d->ptr - d->data->text[d->line].text));
            failed=TRUE;
            p++;
            break;
          }
          p++;
        } while (--mcnt);
      }
      else {
        do {
DEBUG("PointAdd", (int)(d->ptr - d->data->text[d->line].text));
          if((*d->ptr != *p++) ||
             (!PointAdd1(d) && mcnt!=1)) {
            failed=TRUE;
            break;
          }
        } while (--mcnt);
      }
      break;
    }
    if(!failed)
      continue; /* Successfully matched one pattern command; keep matching */
    /* Jump here if any matching operation fails. */
  fail:
    if (stackp != stackb) {
      /* A restart point is known.  Restart there and pop it. */
      if (!stackp[-2]) {  /* data pointer is null */
        /* If innermost failure point is dormant,
           flush it and keep looking */
        stackp -= 3;
        goto fail;
      }
      d->line = (unsigned int)*--stackp; /* GET LINE HERE!!!! */
      d->ptr = *--stackp;
      PointTo(d, 0); /* reset values */
      p = *--stackp;
    }
    else
      break;   /* Matching at this starting point really fails! */
    failed = FALSE;
    add1=FALSE;
  }
  if(stackb_malloc)
    Dealloc(stackb);
  d->finnished=FALSE;
  if (failed==NO_RETRY)
    return -3;
  return -1;         /* Failure to match */
}

int INLINE REGARGS STATIC
bcmp_translate (s1, s2, len, fact)
     unsigned char *s1, *s2;
     int len;
     FACT *fact;
{
  if(fact)  {
    while (--len>=0) {
      if ( *s1!=*s2 && 
          (!(fact->xflags[*s1]&(factx_UPPERCASE|factx_LOWERCASE)) ||
           (fact->cases[*s1++] != *s2++)))
        return 1;
    }
    return 0;
  }
  return memcmp(s1, s2, len);
}

#ifndef FREXXED

#include <stdio.h>

/* Indexed by a character, gives the upper case equivalent of the character */

static unsigned char upcase[0400] = 
  { 000, 001, 002, 003, 004, 005, 006, 007,
    010, 011, 012, 013, 014, 015, 016, 017,
    020, 021, 022, 023, 024, 025, 026, 027,
    030, 031, 032, 033, 034, 035, 036, 037,
    040, 041, 042, 043, 044, 045, 046, 047,
    050, 051, 052, 053, 054, 055, 056, 057,
    060, 061, 062, 063, 064, 065, 066, 067,
    070, 071, 072, 073, 074, 075, 076, 077,
    0100, 0101, 0102, 0103, 0104, 0105, 0106, 0107,
    0110, 0111, 0112, 0113, 0114, 0115, 0116, 0117,
    0120, 0121, 0122, 0123, 0124, 0125, 0126, 0127,
    0130, 0131, 0132, 0133, 0134, 0135, 0136, 0137,
    0140, 0101, 0102, 0103, 0104, 0105, 0106, 0107,
    0110, 0111, 0112, 0113, 0114, 0115, 0116, 0117,
    0120, 0121, 0122, 0123, 0124, 0125, 0126, 0127,
    0130, 0131, 0132, 0173, 0174, 0175, 0176, 0177,
    0200, 0201, 0202, 0203, 0204, 0205, 0206, 0207,
    0210, 0211, 0212, 0213, 0214, 0215, 0216, 0217,
    0220, 0221, 0222, 0223, 0224, 0225, 0226, 0227,
    0230, 0231, 0232, 0233, 0234, 0235, 0236, 0237,
    0240, 0241, 0242, 0243, 0244, 0245, 0246, 0247,
    0250, 0251, 0252, 0253, 0254, 0255, 0256, 0257,
    0260, 0261, 0262, 0263, 0264, 0265, 0266, 0267,
    0270, 0271, 0272, 0273, 0274, 0275, 0276, 0277,
    0300, 0301, 0302, 0303, 0304, 0305, 0306, 0307,
    0310, 0311, 0312, 0313, 0314, 0315, 0316, 0317,
    0320, 0321, 0322, 0323, 0324, 0325, 0326, 0327,
    0300, 0301, 0302, 0303, 0304, 0305, 0306, 0307,
    0310, 0311, 0312, 0313, 0314, 0315, 0316, 0317,
    0320, 0321, 0322, 0323, 0324, 0325, 0326, 0327,
    0330, 0331, 0332, 0333, 0334, 0335, 0336, 0367,
    0340, 0341, 0342, 0343, 0344, 0345, 0346, 0377
  };

int main (argc, argv)
     int argc;
     signed char **argv;
{
  signed char pat[80];
  struct re_pattern_buffer buf;
  int i;
  signed char fastmap[(1 << BYTEWIDTH)];
  
  struct re_registers Regs;
  
  signed char *array_data[]= {
    "this is A text pile\n",
    "that we're gonna (use) when\n",
    "these \"routines\" is remade to ",
    "work with good old arrays"
    };
  static unsigned int array_length[]= { 0, 0, 0, 0};
  
  struct re_textdata pos;
  /* Allow a command argument to specify the style of syntax.  */
  
  int a;
  for(a=0; a<4; a++) {
    array_length[a] = (unsigned int)strlen((signed char *)array_data[a]);
    printf("%s", array_data[a]);
  }
  putchar ('\n');
  
  buf.allocated = 40;
  buf.buffer = (signed char *) Malloc (buf.allocated);
  buf.fastmap = fastmap;
#ifdef CASE_INSENTIVITE
  buf.translate = upcase;
#else
  buf.translate = NULL;
#endif
  
  while (1) {
    gets (pat);
    
    if (*pat) {
      re_compile_pattern (pat, &buf);
#if 0
      for (i = 0; i < buf.used; i++)
        printchar (buf.buffer[i]);
      
      putchar ('\n');
      
      printf ("%d allocated, %d used.\n", buf.allocated, buf.used);
#endif
      re_compile_fastmap (&buf);
#if 0
      printf ("Allowed by fastmap: ");
      for (i = 0; i < (1 << BYTEWIDTH); i++)
        if (fastmap[i]) printchar (i);
      putchar ('\n');
#endif
    }
    else {
      printf("Enter pattern!\n");
      continue;
    }
#if 1
    PointInit(&pos,
              0, /* set start line search position, */
              0, /* set start column search position, */
              (unsigned char **)array_data, /* array */
              array_length, /* text lengths */
              4,         /* num of lines */
              3,
              strlen(array_data[3]),
              TRUE /* search forward! */
              );
#else
    PointInit(&pos,
              0, /* set start line search position, */
              4, /* set start column search position, */
              (unsigned char **)array_data, /* array */
              array_length, /* text lengths */
              4,         /* num of lines */
              0,
              0,
              FALSE /* search forward! */
              );
#endif
    
    i = re_search(&buf, 120, &Regs, &pos.pos);
    
    if (i > -1) {
      printf("Match at line %d column %d -> line %d column %d!\n",
             Regs.start_line[0],
             Regs.start[0],
             Regs.end_line[0],
             Regs.end[0]
             );
      {
        int a=1;
        signed char *replace_PC;
        while(Regs.start[a]!=-1) {
          printf("paren #%d at line %d column %d -> line %d column %d!\n",
                 a,
                 Regs.start_line[a],
                 Regs.start[a],
                 Regs.end_line[a],
                 Regs.end[a]
                 );
          a++;
        }
        printf("Enter replace string:\n");
        gets (pat);	/* Now read the string to match against */
        
        replace_PC = re_replace((unsigned char **)array_data,
                                array_length, &Regs, pat, &i);
        if(replace_PC) {
          printf("Replace string:\n'");
          printstring(replace_PC, i);
          printf("'\n");
          Dealloc(replace_PC);
        }
      }
    }
    else
      printf ("No match!\n");
  }
}

#ifdef NOTDEF
print_buf (bufp)
     struct re_pattern_buffer *bufp;
{
  int i;

  printf ("buf is :\n----------------\n");
  for (i = 0; i < bufp->used; i++)
    printchar (bufp->buffer[i]);
  
  printf ("\n%d allocated, %d used.\n", bufp->allocated, bufp->used);
  
  printf ("Allowed by fastmap: ");
  for (i = 0; i < (1 << BYTEWIDTH); i++)
    if (bufp->fastmap[i])
      printchar (i);
  printf ("\nAllowed by translate: ");
  if (bufp->translate)
    for (i = 0; i < (1 << BYTEWIDTH); i++)
      if (bufp->translate[i])
	printchar (i);
  printf ("\nfastmap is%s accurate\n", bufp->fastmap_accurate ? "" : "n't");
  printf ("can %s be null\n----------", bufp->can_be_null ? "" : "not");
}
#endif

void REGARGS STATIC printchar (signed char c)
{
  if (c < 0x20 || c >= 0x7f)
      printf("\\x%02x", c);
  else
      putchar (c);
}

void REGARGS STATIC printstring(signed char *string, int len)
{
    while(len--) {
	printchar(*string++);
    }
}

#endif /* test */
