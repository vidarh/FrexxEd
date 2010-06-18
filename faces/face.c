/*************************************************************************
 *
 * The internal theories:
 * ~~~~~~~~~~~~~~~~~~~~~~
 * We call the areas with different style and/or colours 'faces'.
 *
 * struct FaceControl holds all local face information for a single buffer.
 *
 * struct Face holds information about a single string and which face that
 * string should bring up on screen.
 *
 * struct FaceType defines all faces. There is 256 different faces which can
 * be defined to be any combination of style, background and foreground.
 *
 * The FPL function interface:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * FaceID( name ) will return the ID of the face-setup called 'name'. If name
 * doesn't already exist as a face-setup, it will get created first.
 * FaceAdd( ID, string, flags ) adds a string to the specified face-setup.
 * FaceType( Num, Style, Fg, Bg) sets the specified facetype as specified.
 *
 ************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef UNIX
#include <sys/types.h>
typedef unsigned short WORD;
#define __inline
#define TRUE 1
#define FALSE 0
#else
#include <exec/types.h>
#endif

/* Struktur för textpekare.  Storleken definieras även i UpdtScreenML.i */
typedef struct {
  char *text;
  int length;
  WORD fold_no;
  WORD flags;
  int trash;
} TextStruct;

#define XX(a) {a,0,0,0,0}

#define ERROR 1
#define OK    0

/**********************************************************************
 *
 * Different character defines:
 *
 **********************************************************************/

#define _U (1<<0)  /* upper case */
#define _L (1<<1)  /* lower case */
#define _W (1<<2)  /* also included as a valid identifier character */
#define _N (1<<3)  /* numerical digit 0-9 */
#define _S (1<<4)  /* white space */
#define _C (1<<5)  /* control character */
#define _P (1<<6)  /* punctation characters */
#define _X (1<<7)  /* hexadecimal digit */

/***********************************************************************
 *
 * A bunch of useful macros:
 *
 **********************************************************************/

#define isalpha(c)  ((type+1)[c] & (_U|_L))
#define isupper(c)  ((type+1)[c] & _U)
#define islower(c)  ((type+1)[c] & _L)
#define isdigit(c)  ((type+1)[c] & _N)
#define isxdigit(c) ((type+1)[c] & _X)
#define isalnum(c)  ((type+1)[c] & (_U|_L|_N))
#define isspace(c)  ((type+1)[c] & _S)
#define ispunct(c)  ((type+1)[c] & _P)
#define isprint(c)  ((type+1)[c] & (_U|_L|_N|_P))
#define iscntrl(c)  ((type+1)[c] & _C)
#define isascii(c)  (!((c) & ~127))
#define isgraph(c)  ((type+1)[c] & (_U|_L|_N|_P))
#define toascii(c)  ((c) & 127)
#define toupper(c)  ((type+1)[c] & _L? c-CASE_BIT: c)
#define tolower(c)  ((type+1)[c] & _U? c+CASE_BIT: c)

#define isodigit(c) ((c) >= CHAR_ZERO && (c) <= CHAR_SEVEN)

#define isident(c)  ((type+1)[c] & (_U|_L|_W))
#define isidentnum(c)  ((type+1)[c] & (_U|_L|_W|_N))

const char type[257] = { /* Character type codes */
   _C, /* -1 == regular ANSI C eof character */
   _C,    _C,	  _C,	 _C,    _C,    _C,    _C,    _C, /* 00		*/
   _C,    _S,	  _S,	 _C,    _C,    _S,    _C,    _C, /* 08		*/
   _C,    _C,	  _C,	 _C,    _C,    _C,    _C,    _C, /* 10		*/
   _C,    _C,	  _C,	 _C,    _C,    _C,    _C,    _C, /* 18		*/
   _S,    _P,     _P,	 _P,    _P,    _P,    _P,    _P, /* 20	!"#$%&' */
   _P,    _P,     _P,    _P,    _P,    _P,    _P,    _P, /* 28 ()*+,-./ */
 _N|_X, _N|_X, _N|_X, _N|_X, _N|_X, _N|_X, _N|_X, _N|_X, /* 30 01234567 */
 _N|_X, _N|_X,    _P,    _P,    _P,    _P,    _P,    _P, /* 38 89:;<=>? */
   _P, _U|_X,  _U|_X, _U|_X, _U|_X, _U|_X, _U|_X,    _U, /* 40 @ABCDEFG */
   _U,    _U,	  _U,	 _U,    _U,    _U,    _U,    _U, /* 48 HIJKLMNO */
   _U,    _U,	  _U,	 _U,    _U,    _U,    _U,    _U, /* 50 PQRSTUVW */
   _U,    _U,	  _U,	 _P,    _P,    _P,    _P, _P|_W, /* 58 XYZ[\]^_ */
   _P, _L|_X,  _L|_X, _L|_X, _L|_X, _L|_X, _L|_X,    _L, /* 60 `abcdefg */
   _L,    _L,	  _L,	 _L,    _L,    _L,    _L,    _L, /* 68 hijklmno */
   _L,    _L,	  _L,	 _L,    _L,    _L,    _L,    _L, /* 70 pqrstuvw */
   _L,    _L,	  _L,	 _P,    _P,    _P,    _P,   000, /* 78 xyz{|}~	*/
  000,   000,	 000,	000,   000,   000,   000,   000, /* 80 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* 88 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* 90 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* 98 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* A0 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* A8 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* B0 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* B8 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* C0 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* C8 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* D0 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* D8 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* E0 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* E8 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* F0 	        */
  000,   000,	 000,	000,   000,   000,   000,   000, /* F8 	        */
};


struct FaceType {
  char style;  /* BOLD / ITALICS / UNDERLINE / NORMAL */
  char fgpen;
  char bgpen;
};

struct Face {
  struct Face *next; /* next face-string in the list */
  char facetype; /* which style/colour type that should be used, there can
                    only be 256 different, referenced at least at the moment
                    by a mere number between 0 to 255. */
  unsigned long hash; /* for faster string checks, two strings should be
                         _very_ unlikely to have the same hash number */
  long flags;    /* extra knowledge about this */
  long strlen;
  long strlen2;
  char string[1];
  char string2[1]; /* if there is something that ends this type, only valid
                      for FACE_STARTING */
};
#define FACE_WORDONLY 1  /* classic, only this defined word should be marked
                            and considered */
#define FACE_ANYWHERE 2  /* this string can be anywhere */
#define FACE_BACKSLASH 4 /* this area can be escaped with a '\' letter which
                            will make the following letter to be ignored,
                            to fully support i.e C quoted strings */
#define FACE_1STNONSPC 8 /* this is only valid if its the first non-space
                            characters of a line */
#define FACE_OBEYIMPOR 16 /* This flag will make all strings that are marked
                             as 'important' get marked even if this first
                             string has been found and we're searching for
                             the ending one */
#define FACE_IMPORTANT 32 /* this is a more important style ;) */

#define MAX_STRING_LENGTH 80 /* maximum length of any face-string */
#define MAX_CONTINUATIONS 255 /* maximum amount of strings that begin a style
				 that ends with another string */

#define FACETABLE_SIZE 128 /* the larger the faster, though larger than 256
                              is pointless, and larger than 128 will probably
                              never get noticed. */

struct FaceControl {
  /*
   * General pointer to the next face control setup.
   */
  struct FaceControl *next;

  /***************************************************************
   * Local array for each face-setup.
   * It contains non-zero in the entry if the character is the last character
   * of a face-control word.
   ****************************************************************/
  char exist[256];
/* FACE_WORDONLY 1 classic, only this defined word should be marked
                   and considered */
/* FACE_ANYWHERE 2 this string can be anywhere */
#define FC_SEVERAL   4 /* more than one control-string ends with this
                          character */
#define FC_IMPORTANT 8 /* this is a more prioritized string */

  /***************************************************************
   * Array with struct pointers.
   * The array is only FACETABLE_SIZE items big.
   * Non-zero entry means a pointer to a "struct Face" holding information
   * about that single string and its desired colour/style setup.
   ***************************************************************/
  struct Face *strings[FACETABLE_SIZE];

  /***************************************************************
   * The length of the shortest string added. Lines shorter than
   * this cannot change anything.
   ***************************************************************/
  long shortest;

  /***************************************************************
   * The total amount of control strings added.
   ***************************************************************/
  long numofstrings;

  /***************************************************************
   * The total amount of "continuation" strings added.
   * That is strings that begin a mode/style.
   ***************************************************************/
  long numofconts;

  /***************************
   * Name of the face setup to.
   **************************/
  char name[1];
};

static unsigned long Gethash(char *name, long len);
void PrintText(TextStruct *textinfo, int lines);
void FaceReport(struct FaceControl *facecontrol);
long ScanBuffer(struct FaceControl *facecontrol,
                TextStruct *textinfo,
                long lines);

long scanline;
long lastline;
                
/*
 * Add a string to a facecontrol.
 */
long AddFaceString(struct FaceControl *facecontrol,
                   char facetype,
                   char *string,
                   long strlen,
                   char *string2,
                   long strlen2,
                   long flags)
#define ADD_WORDONLY   1 /* otherwise ANYWHERE */
#define ADD_1STNONSPC  2
#define ADD_BACKSLASH  4
#define ADD_OBEYIMPO   8
#define ADD_IMPORTANT 16

{
  struct Face *face;
  face = malloc( sizeof(struct Face) + strlen + strlen2);
  if(face) {
    char lastchar = string[strlen-1];
    
    memcpy(face->string, string, strlen);
    face->string[strlen]=0; /* zero terminate for easier access */
    face->strlen = strlen;
    
    facecontrol->numofstrings++; /* increase number of things added */
    if(string2 && strlen2) {
      facecontrol->numofconts++; /* increase number of continuation things */
      if(facecontrol->numofconts==MAX_CONTINUATIONS) {
        /* we can only take a certain amount of these kind since it will
           be using data space in the struct stored for each line in a
           single buffer */
        free(face);
        return ERROR;
      }
      memcpy(face->string2+strlen, string2, strlen2);
    }
    face->string2[strlen2+strlen]=0; /* zero terminate for easier access */
    face->strlen2 = strlen2;
    
    face->hash = Gethash(string, strlen);
    face->flags = (flags&ADD_1STNONSPC?FACE_1STNONSPC:0)|
                    (flags&ADD_BACKSLASH?FACE_BACKSLASH:0)|
                      (flags&ADD_OBEYIMPO?FACE_OBEYIMPOR:0)|
                        (flags&ADD_WORDONLY?FACE_WORDONLY:FACE_ANYWHERE)|
                          (flags&ADD_IMPORTANT?FACE_IMPORTANT:0);
    face->facetype = facetype;

    /* fix that shortest string stuff */
    if(facecontrol->shortest>strlen)
      facecontrol->shortest = strlen;
    
    /* if there already is a string ending with the same character as we
       do... */
    if(facecontrol->exist[ lastchar ])
      facecontrol->exist[ lastchar ] |= FC_SEVERAL; /* we're not alone */

    /* set the ->exist flag depending on the flags set for the string */
    facecontrol->exist[ lastchar ] |=
      (flags&ADD_WORDONLY?FACE_WORDONLY:FACE_ANYWHERE)|
        (flags&ADD_IMPORTANT?FC_IMPORTANT:0);

    /* link ourself to the chain of face-strings */
    face->next = facecontrol->strings[lastchar%FACETABLE_SIZE];
    facecontrol->strings[lastchar%FACETABLE_SIZE] = face;
  }
  else
    return ERROR;
  return OK;
}

struct Face * __inline
MatchWord(TextStruct *ti,
          struct Face *face,
          long col,
          long firstnspace,
          long flags)
{
  if(col<ti->length-1 && !isidentnum(ti->text[ col + 1 ])) {
    /*
     * We're at the end of a word, get the entire word and its
     * checksum
     */
    long n;
    long i = col;
    unsigned long hash;
    n=0;
    do {
      n++;
      if(i-1 < 0 ||
         !isidentnum(ti->text[ i-1 ]) ) {
        break;
      }
      i--;
    } while(1);
    /* word is 'n' bytes long starting at 'i' */
    if(n<=MAX_STRING_LENGTH) {
      /*
       * This isn't longer than maxlength, cause if it is, we won't
       * have to bother to check the word.
       */
      hash = Gethash(&ti->text[i], n);
      do {
        if(face->flags&FACE_WORDONLY &&
           face->hash == hash &&
           !memcmp(face->string, &ti->text[i], n) &&
           face->flags & flags) {
             
          if (face->flags&FACE_1STNONSPC &&
              i != firstnspace)
            continue;
          printf("-WORDONLY--> %s\n", face->string);
          return face;
        }
      } while(face=face->next);
    }
  }
  return NULL;
}

struct Face * __inline
MatchAnyWhere(TextStruct *ti,
              struct Face *face,
              long col,
              long firstnspace,
              long flags)
{
  do {
    if(face->flags&FACE_ANYWHERE &&
       col + 1 >= face->strlen &&
       !memcmp(face->string,
               &ti->text[ col - face->strlen + 1],
               face->strlen) &&
       face->flags&flags) {
      if (face->flags&FACE_1STNONSPC &&
          col != firstnspace)
        continue;
      printf("-ANYWHERE--> %s\n", face->string);
      return face; /* found it! */
    }
  } while(face=face->next);
  return NULL; /* none found */
}

/*
 * Search for a specific string in the line from the specific position
 */
long __inline FindEnd(TextStruct **ti,
                      struct FaceControl *fc,
                      long *count,
                      char *string2,
                      long strlen2,
                      long flags)
{
  register char *pnt;
  while(*count+strlen2 <= (*ti)->length ) {
    pnt = &(*ti)->text[*count];
    if(flags&FACE_BACKSLASH && '\\' == *pnt) {
      (*count)+=2;
      continue;
    }

    if(flags&FACE_OBEYIMPOR &&
       fc->exist[ *pnt ] & FC_IMPORTANT) {
      char hit=FALSE;
      struct Face *face = fc->strings[ *pnt % FACETABLE_SIZE];

      if(fc->exist[ *pnt ] & FACE_WORDONLY) {
        if(face = MatchWord((*ti), face, *count, -1, FACE_IMPORTANT))
          hit=TRUE;
      }
      if(!hit &&
         fc->exist[ *pnt ] & FACE_ANYWHERE) {
        /*
         * We may have matched some word right *now*.
         * Check all FACE_ANYWHERE strings on the text in the buffer.
         */
        if(face = MatchAnyWhere((*ti), face, *count, -1, FACE_IMPORTANT))
          hit=TRUE;
      }
      if(hit && face->strlen2) {
        /*
         * If we found a match and this turns out to be a "starting"
         * string, find the end of it!
         */
        (*count)++; /* pass the ending letter of the init string */
        while(scanline<lastline) {
          if(FindEnd(ti, fc, count,
                     face->string2+face->strlen, face->strlen2,
                     face->flags)) {
            *count+=face->strlen2;
            break;
          }
          scanline++;
          (*ti)++;
          (*count)=0; /* restart at column 0 */
        }

      }
    }

    if(!memcmp(string2, pnt, strlen2)) {
      printf("-FOUNDEND--> %s\n", string2);
      return 1;
    }
    (*count)++;
  }
  return 0; /* next line please */
}

long ScanBuffer(struct FaceControl *fc,
                TextStruct *ti,
                long lines)
{
  struct Face *face;
  char hit;
  long firstnspace;
  char spacestat;
  register char byte;
  lastline = lines;
  for(scanline=0; scanline<lastline; scanline++, ti++) {
    spacestat = FALSE;
    firstnspace = 0;
    if(fc->shortest <= ti->length) {
      long count;
      for(count=fc->shortest-1;
          count < ti->length;
          count++) {
        byte = ti->text[ count ];
        if(!spacestat && !isspace(byte)) {
          spacestat = TRUE;
          firstnspace = count;
        }
        if( fc->exist[byte] ) {
          char code = fc->exist[byte];
          face=fc->strings[byte%FACETABLE_SIZE];
          hit = FALSE;
          if(code & FACE_WORDONLY) {
            if(face = MatchWord(ti, face, count, firstnspace, ~0))
              hit=TRUE;
          }
          if(!hit &&
             code & FACE_ANYWHERE) {
            /*
             * We may have matched some word right *now*.
             * Check all FACE_ANYWHERE strings on the text in the buffer.
             */
            if(face = MatchAnyWhere(ti, face, count, firstnspace, ~0))
              hit=TRUE;
          }
          if(hit && face->strlen2) {
            /*
             * If we found a match and this turns out to be a "starting"
             * string, find the end of it!
             */
            count++; /* pass the ending letter of the init string */
            while(scanline<lastline) {
              if(FindEnd(&ti, fc, &count,
                         face->string2+face->strlen, face->strlen2,
                         face->flags)) {
                count+=face->strlen2-1;
                break;
              }
              scanline++;
              ti++;
              count=0; /* restart at column 0 */
            }

          }
        }
      }
    }
  }
  return 0;
}

void InitFace(struct FaceControl *facecontrol)
{
  memset(facecontrol, 0, sizeof(struct FaceControl));
  facecontrol->shortest=9999;
}

int main(int argc, char **argv)
{
  TextStruct textinfo[]={
    XX("  #define killerninjainthewoods --- /*\n"),
	XX("    */ for int ---- \n"),
    XX("int oldret;\n"),
    XX("for(;undoantal;)\n"),
    XX("  Command(Storage, DO_NOTHING|NO_HOOK, 0, NULL, NULL);\n"),
    XX("tempprg=Malloc(tempprglen);\n"),
    XX("else {\n"),
    XX("  ninja++;\n"),
    XX("}\n"),
    XX("\n"),
    XX("if (tempprg) {\n"),
    XX("  tempprg[0]='\\0';\n"),
    XX("while(!(hook->flags&HOOK_ABS)) {\n"),
    XX("  /* this is not an \"absolute\" hook! */\n"),
    XX("  register char *tempprgend;\n"),
    XX("  len=hook->func->format? strlen(hook->func->format) : 0;\n"),
    XX("}\n"),
    XX("Sprintf(tempprg, \"%64\\\"s/* */ (\", name); /*for only*/\n"),
    XX("tempprgend=tempprg+65;\n")
  };
  struct FaceControl facecontrol;
  struct FaceType facetype[256];

  InitFace(&facecontrol);

  PrintText((TextStruct *)&textinfo[0], sizeof(textinfo)/sizeof(textinfo[0]));

  AddFaceString(&facecontrol, 0, "while", 5, NULL, 0, ADD_WORDONLY);
  AddFaceString(&facecontrol, 0, "if",    2, NULL, 0, ADD_WORDONLY);
  AddFaceString(&facecontrol, 0, "int",   3, NULL, 0, ADD_WORDONLY);
  AddFaceString(&facecontrol, 0, "for",   3, NULL, 0, ADD_WORDONLY);
  AddFaceString(&facecontrol, 0, "do",    2, NULL, 0, ADD_WORDONLY);
  AddFaceString(&facecontrol, 0, "else",  4, NULL, 0, ADD_WORDONLY);
  AddFaceString(&facecontrol, 0, "/*",    2, "*/", 2, ADD_IMPORTANT);
  AddFaceString(&facecontrol, 0, "\"",    1, "\"", 1, ADD_BACKSLASH);
  AddFaceString(&facecontrol, 0, "#",     1, "\n", 1, ADD_BACKSLASH|
                                                      ADD_1STNONSPC|
                                                      ADD_OBEYIMPO);

  FaceReport(&facecontrol);
  
  ScanBuffer(&facecontrol,
             &textinfo[0],
             sizeof(textinfo)/sizeof(textinfo[0]));

  return 0;
}

/**********************************************************************
 *
 * int Gethash();
 *
 * Return the hash number for the name received as argument.
 *
 *****/

static unsigned long Gethash(char *name, long len)
{
  unsigned long hash=0;
  while(len--)
    hash=(hash<<1)+(*name++ +1)+(hash&(1<<31)?-2000000000:0);
  return hash;
}


void PrintText(TextStruct *textinfo, int lines)
{
  int i;
  printf("TEXT:\n");
  for(i=0; i<lines; i++, textinfo++) {
    printf("%2d %s", i, textinfo->text);
    textinfo->length = strlen(textinfo->text);
  }
}

void FaceReport(struct FaceControl *facecontrol)
{
  int i;
  int count=0;
  struct Face *face;
  for(i=0; i<256; i++) {
    if(facecontrol->exist[i]) {
      face = facecontrol->strings[i];
      count = 0;
      while(face) {
        ++count;
        face=face->next;
      }
      printf("Strings end with '%c' %d times\n", i, count);
    }
  }
}
