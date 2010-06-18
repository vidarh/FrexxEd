/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */

struct FaceType {
  char  style; /* BOLD / ITALICS / UNDERLINE / NORMAL */
  short pen;   /* fg*256+bg */
  char flags;
  char *name;  /* pointer to name, see flags if alloced or static */
};
#define FT_STATIC 1 /* this has a static name */

struct Face {
  struct Face *next; /* next face-string in the list */
  char facetype; /* which style/colour type that should be used, there can
                    only be 256 different, referenced at least at the moment
                    by a mere number between 0 to 255. */
  unsigned long hash; /* for faster string checks, two strings should be
                         _very_ unlikely to have the same hash number */
  char num;      /* if this is a continued kind, this contains our "number"
                    1-255  (0 is for default mode) */ 
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


 /* used by the scanner routine: */
#define FACE_CONTINUE  64
#define FACE_SKIPNEXT  128

#define FACESTRING_LENGTH 80 /* maximum length of any face-string */
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

  /***************************************************************
   * Array of struct Face pointers to the actual continue-face with
   * the corresponding number. [1] points to the first continued
   * face, [2] to the second, etc. [0] is the original and considered
   * original mode.
   * This should be realloced to increase on each new continue-face
   * that is added!
   ***************************************************************/
  struct Face **conts;
  
  /***************************
   * Name of the face setup to.
   **************************/
  char name[1];
};

/*
 *  Defines for AddFaceString()
 */

#define ADD_WORDONLY   1 /* otherwise ANYWHERE */
#define ADD_1STNONSPC  2
#define ADD_BACKSLASH  4
#define ADD_OBEYIMPO   8
#define ADD_IMPORTANT 16


void __regargs StringMatchInit(BufStruct *storage,
                    char *text,                 /* buffer line */
                    struct screen_buffer *dest, /* destination storage */
                    int offset,                 /* start offset */
                    int line);			/* line */
int __regargs StringMatchEnd(BufStruct *storage,
                             int line); /* returns TRUE if changed */
void __regargs StringMatch(char byte, /* character in text */
                           int index, /* byte position in line */
                           int store); /* store position index */
struct FaceControl * __regargs InitFace(char *name);
long __regargs AddFaceString(struct FaceControl *facecontrol,
                   char facetype,
                   char *string,
                   long strlen,
                   char *string2,
                   long strlen2,
                   long flags);
struct FaceControl *FaceGet(char *facename, long mode);

#define FACEGET_CREATE 1
#define FACEGET_CHECK  0

#define MAX_STYLES 256 /* maximum amount of different text styles */
int __regargs FaceStyle(char *, char *, int, int, int );
int __regargs FaceAdd(struct FaceControl *fc,
                      int style,
                      char *addstring,
                      long addlen,
                      char *flagstr,
                      char *endstring,
                      long endlen);
void __regargs CleanupAllFaces(void);

char **FaceListFaces(int *faces);
char **FaceListStyles(int *faces);
char *GetFaceStyle(char *style, int *fg, int *bg);

