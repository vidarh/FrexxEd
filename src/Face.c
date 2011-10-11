/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/*
 * Faces
 */

#include <stdio.h>
#include <string.h>

#include "compat.h"

#ifdef AMIGA
#include <proto/utility.h>
#endif

#include "Buf.h"
#include "Alloc.h"
#include "Face.h"
#include "UpdtScreenC.h"
#include "FACT.h"

#define BG(x) ((x)<<8)
#define FG(x) (x)

#define READ_BG(x) ((x)>>8)
#define READ_FG(x) ((x)&255)

extern DefaultStruct Default;

static struct FaceControl *fc; /* current one */
static struct FaceType facetype[MAX_STYLES]= {
  { cb_NORMAL,    FG(1) | BG(0), FT_STATIC, "Normal"    },
  { cb_REVERSE,   FG(1) | BG(0), FT_STATIC, "Reverse"   },
  { cb_BOLD,      FG(1) | BG(0), FT_STATIC, "Bold"      },
  { cb_ITALIC,    FG(1) | BG(0), FT_STATIC, "Italic"    },
  { cb_UNDERLINE, FG(1) | BG(0), FT_STATIC, "Underline" },
  { cb_NORMAL,    FG(2) | BG(0), FT_STATIC, "Highlight" },
  { cb_NORMAL,    FG(3) | BG(0), FT_STATIC, "Shadow"    },
  { cb_ITALIC,    FG(2) | BG(0), FT_STATIC, "Highitalic"}
};
static int NumOfFaceStyles=8; /* NUMBER of entries above */

static struct Face *screenface[2];
static int mode[2]; /* scan mode */
static char lastchar[2]; /* final character of the end string */
static int facenum; /* current face */

static char *screentext; /* current text line */
static struct screen_buffer *screenstore;
static BufStruct *Storage;
static char spacestat;
static int firstnspace;
static int start_offset;

#define ISWORD(x)	(Storage->using_fact->xflags[x]&factx_CLASS_WORD?1:0)
#define ISNWORD(x)	(!(Storage->using_fact->xflags[x]&factx_CLASS_WORD))

#define ISSYMBOL(x)	(Storage->using_fact->xflags[x]&factx_CLASS_SYMBOL?1:0)
#define ISSPACE(x)	(Storage->using_fact->xflags[x]&factx_CLASS_SPACE?1:0)
#define ISUPPERCASE(x)	(Storage->using_fact->xflags[x]&factx_UPPERCASE?1:0)
#define ISLOWERCASE(x)	(Storage->using_fact->xflags[x]&factx_LOWERCASE?1:0)
#define ISOPEN(x)	(Storage->using_fact->xflags[x]&factx_CLASS_OPEN?1:0)
#define ISCLOSE(x)	(Storage->using_fact->xflags[x]&factx_CLASS_CLOSE?1:0)
#define ISNEWLINE(x)	(Storage->using_fact->flags[x]&fact_NEWLINE)
#define ISTAB(x)	(Storage->using_fact->flags[x]&fact_TAB?1:0)

static long __inline FaceStyleString(char *flags);
static long __inline Faceaddflags(char *flagstr, char *usepipe);

/**********************************************************************
 *
 * int Gethash();
 *
 * Return the hash number for the name received as argument.
 *
 *****/

static __inline unsigned long Gethash(char *name, long len)
{
  unsigned long hash=0;
  while(len--)
    hash=(hash<<1)+(*name++ +1)+(hash&(1<<31)?-2000000000:0);
  return hash;
}

#if 1
#define SetColour(strlen, facenum, store, index)                \
    do {                                                        \
      /*                                                        \
       * This is a found substring!                             \
       */                                                       \
      int count=store-start_offset;                             \
      short pen = facetype[facenum].pen;                        \
      short style = facetype[facenum].style;                    \
      while (--count>=0 &&                                      \
             index-strlen<screenstore->linebuffer[count]) {     \
        screenstore->control[count+start_offset]=style;         \
        screenstore->colors[count+start_offset]=pen;            \
      }                                                         \
    } while(0)
#else
void SetColour(strlen, facenum, store, index)                  
{                                                       
  /*                                                       
   * This is a found substring!                            
   */                                                      
  int count=store-start_offset;                          
  short pen = facetype[facenum].pen;                       
  short style = facetype[facenum].style;                   
  while (--count>=0 &&                                        
         index-strlen<screenstore->linebuffer[count]) {    
    screenstore->control[count+start_offset]=style;        
    screenstore->colors[count+start_offset]=pen;           
  }                                                        
}
#endif
struct FaceControl * __regargs InitFace(char *name)
{
  struct FaceControl *facecontrol;
  facecontrol = Malloc(sizeof(struct FaceControl)+strlen(name));
  if(facecontrol) {
    memset(facecontrol, 0, sizeof(struct FaceControl));
    facecontrol->shortest=9999;
    facecontrol->conts=Malloc(sizeof(struct Face *));
    if(!facecontrol->conts) {
      Dealloc(facecontrol);
      return NULL;
    }
    facecontrol->conts[0]=NULL; /* this first is never really used */
    strcpy(facecontrol->name, name);
    {
      SharedStruct *shared=Default.SharedDefault.Next;
      while (shared) {
        if (!Stricmp(name, shared->face_name)) {
          shared->face=facecontrol;
          shared->face_updated_line=0;
          {
            BufStruct *count=shared->Entry;
            while (count) {
              count->face_top_updated_line=0;
              count->face_bottom_updated_line=0;
              count=count->NextSplitBuf;
            }
          }
          face_update=TRUE;
        }
        shared=shared->Next;
      }
    }
#if 0
    AddFaceString(facecontrol, 0, "while", 5, NULL, 0, ADD_WORDONLY);
    AddFaceString(facecontrol, 0, "if",    2, NULL, 0, ADD_WORDONLY);
    AddFaceString(facecontrol, 0, "int",   3, NULL, 0, ADD_WORDONLY);
    AddFaceString(facecontrol, 0, "for",   3, NULL, 0, ADD_WORDONLY);
    AddFaceString(facecontrol, 0, "do",    2, NULL, 0, ADD_WORDONLY);
    AddFaceString(facecontrol, 0, "else",  4, NULL, 0, ADD_WORDONLY);
    AddFaceString(facecontrol, 1, "/*",    2, "*/", 2, ADD_IMPORTANT);
    AddFaceString(facecontrol, 1, "\"",    1, "\"", 1, ADD_BACKSLASH);
    AddFaceString(facecontrol, 0, "#",     1, "\n", 1, ADD_BACKSLASH|
                                                       ADD_1STNONSPC|
                                                       ADD_OBEYIMPO);
#endif
  }
  return facecontrol;
}

/*
 * Add a string to a facecontrol.
 */
long __regargs AddFaceString(struct FaceControl *facecontrol,
                             char facetype,
                             char *string,
                             long strlen,
                             char *string2,
                             long strlen2,
                             long flags)
{
  struct Face *face;  
  face = Malloc( sizeof(struct Face) + strlen + strlen2);
  if(face) {
    char lastchar = string[strlen-1];
    
    memcpy(face->string, string, strlen);
    face->string[strlen]=0; /* zero terminate for easier access */
    face->strlen = strlen;
    
    facecontrol->numofstrings++; /* increase number of things added */
    if(string2 && strlen2) {
      void *tmp;
      if(facecontrol->numofconts==MAX_CONTINUATIONS) {
        /* we can only take a certain amount of these kind since it will
           be using data space in the struct stored for each line in a
           single buffer */
        Dealloc(face);
        return SYNTAX_ERROR;
      }
      ++facecontrol->numofconts; /* increase number of continuation faces */
      memcpy(face->string2+strlen, string2, strlen2);
      tmp = Realloc((char *)facecontrol->conts, sizeof(struct Face *) *
                    (facecontrol->numofconts + 1) /* plus the default */ );
      if(!tmp) {
        Dealloc(face);
        return OUT_OF_MEM;        
      }
      facecontrol->conts = (struct Face **)tmp;
      facecontrol->conts[facecontrol->numofconts] = face;
      face->num = facecontrol->numofconts; /* set face num in face struct */
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
  else {
    return OUT_OF_MEM;
  }
  return OK;
}

struct Face * MatchWord(struct Face *face,
          long col,
          long flags)
{
  if(!ISWORD(screentext[ col + 1 ])) {
    /*
     * We're at the end of a word, get the entire word and its
     * checksum
     */
    long n=col;
    while(--col >= 0 && ISWORD(screentext[ col ]) );
    n -= col++; /* we have the length */

    /* word is 'n' bytes long starting at 'col' */
    if(n<=FACESTRING_LENGTH) {
      /*
       * This isn't longer than maxlength, cause if it is, we won't
       * have to bother to check the word.
       */
      unsigned long hash = Gethash(&screentext[col], n);
      do {
        if(((face->flags&(FACE_WORDONLY|flags)) == (FACE_WORDONLY|flags)) &&
           face->hash == hash &&
           !memcmp(face->string, &screentext[col], n)) {
             
          if (face->flags&FACE_1STNONSPC &&
              col != firstnspace)
            continue;
          return face;
        }
      } while(face=face->next);
    }
  }
  return NULL;
}

struct Face * __inline
MatchAnyWhere(struct Face *face,
              long col,
              long flags)
{
  do {
    if(((face->flags&(FACE_ANYWHERE|flags)) == (FACE_ANYWHERE|flags)) &&
       col + 1 >= face->strlen &&
       !memcmp(face->string,
               &screentext[ col - face->strlen + 1],
               face->strlen)) {
      if (face->flags&FACE_1STNONSPC &&
          col != firstnspace)
        continue;
      return face; /* found it! */
    }
  } while(face=face->next);
  return NULL; /* none found */
}

void StringMatchInit(BufStruct *storage,
		     char *text,                 /* buffer line */
		     struct screen_buffer *dest, /* destination storage */
		     int offset,                 /* start offset */
		     int line)			/* line */
{
  screentext = text;
  screenstore = dest;
  Storage = storage;
  spacestat = FALSE;
  firstnspace = 0;
  start_offset = offset;
  fc = Storage->shared->face;
  if(storage->shared->text[line-1].old_style) {
    facenum=1;
    screenface[0]=fc->conts[storage->shared->text[line-1].old_style];
    mode[0] = screenface[0]->flags|FACE_CONTINUE;
    lastchar[0] = screenface[0]->string2[screenface[0]->strlen+screenface[0]->strlen2-1];
  }
  else
    facenum = 0;
    
  if(storage->shared->text[line-1].current_style) {
    screenface[facenum]=fc->conts[storage->shared->text[line-1].current_style];
    mode[facenum] = screenface[facenum]->flags|FACE_CONTINUE;
    lastchar[facenum] = screenface[facenum]->string2[screenface[facenum]->strlen+screenface[facenum]->strlen2-1];
  }
  else {
    mode[facenum]=0;
  }
}

int __regargs StringMatchEnd(BufStruct *Storage,
                             int line)
{
  char old_old =  BUF(shared->text[line].old_style);
  char old_curr = BUF(shared->text[line].current_style);
  BUF(shared->text[line].current_style)=mode[facenum]&FACE_CONTINUE?screenface[facenum]->num:0;
  BUF(shared->text[line].old_style)=facenum?screenface[0]->num:0;
  if (old_old!=BUF(shared->text[line].old_style) ||
      old_curr!=BUF(shared->text[line].current_style))
    return FALSE;
  return TRUE;
}


void __regargs
StringMatch(char byte, /* character in text */
            int index, /* byte position in line */
            int store) /* store position index */
{
  long flags = 0;
  if(!spacestat && !ISSPACE(byte)) {
    spacestat = TRUE;
    firstnspace = index;
  }
  if( mode[facenum] & FACE_CONTINUE ) {
    if(mode[facenum] & FACE_BACKSLASH &&
       ('\\' == byte ||
        mode[facenum] & FACE_SKIPNEXT)) {
      /*
       * If backslash-aware is 'ON', then do the following if we're on
       * a backslash or the letter following it.
       */
      if(mode[facenum] & FACE_SKIPNEXT)
        /* we're on the byte after a backslash */
        mode[facenum] &= ~FACE_SKIPNEXT;
      else
        /* we're supposed to skip next letter */
        mode[facenum] |= FACE_SKIPNEXT;
        
      /* still the same mode here */
      if(-1 != store)
        SetColour(1, screenface[facenum]->facetype, store, index);
      return;       
    }
    else if(mode[facenum] & FACE_OBEYIMPOR &&
            fc->exist[ byte ] & FC_IMPORTANT) {
      flags = FACE_IMPORTANT; /* it better be important! ;) */
    }
    else if(byte == lastchar[facenum] &&
            index >= screenface[facenum]->strlen2-1 &&
            !memcmp(&screenface[facenum]->string2[screenface[facenum]->strlen],
                    &screentext[index - screenface[facenum]->strlen2 + 1],
                    screenface[facenum]->strlen2)) {
      mode[facenum]=0;
      if(-1 != store)
        SetColour(screenface[facenum]->strlen2,
                  screenface[facenum]->facetype,
                  store,
                  index);
      if(facenum)
        --facenum;
      return;
    }
    else {
      /* still the same mode here */
      if(-1 != store)
        SetColour(1, screenface[facenum]->facetype, store, index);
      return;
    }
  }
  if( fc->exist[byte] ) {
    char code = fc->exist[byte];
    struct Face *face = fc->strings [ byte % FACETABLE_SIZE ];
    
    if(code & FACE_WORDONLY &&
       (face = MatchWord(face, index, flags)))
      ;
    else if(code & FACE_ANYWHERE &&
            /*
             * We may have matched some word right *now*.
             * Check all FACE_ANYWHERE strings on the text in the buffer.
             */
            (face = MatchAnyWhere(face, index, flags)) )
      ;
    else {
      if(mode[facenum] & FACE_CONTINUE) {
        /* still the same mode here */
        if(-1 != store)
          SetColour(1, screenface[facenum]->facetype, store, index);
      }
      return;
    }

    if(-1 != store)
      SetColour(face->strlen, face->facetype, store, index);

    if(face->strlen2) {
      if(FACE_IMPORTANT == flags)
        ++facenum;
      /*
       * If this turns out to be a "starting" string!
       */
      mode[facenum]      |= FACE_CONTINUE|face->flags;
      lastchar[facenum]   = face->string2[face->strlen+face->strlen2-1];
      screenface[facenum] = face;
    }

  }
}

/*
  FPL USAGE:
  
  int face = FaceGet("c-mode", 1); // create one if missing
*/

struct FaceControl *Allfaces; /* main linked list pointer */

struct FaceControl *FaceGet(char *facename, long mode)
{
  struct FaceControl *fc = Allfaces;
  struct FaceControl *old=NULL;
  struct FaceControl *new;
  if(fc) {
    do {
      if(!Stricmp(facename, fc->name)) {
        return fc;
      }
      old = fc;
    } while(fc = fc->next);
  }
  if(FACEGET_CHECK == mode)
    return NULL;
  new = InitFace(facename);
  if(new) {
    if(old)
      old->next = new;
    else
      Allfaces = new;
  }
  return new;
}

char **FaceListFaces(int *faces)
{
  struct FaceControl *fc = Allfaces;
  long num=0;
  char **list;

  while(fc) {
    fc = fc->next;
    num++;
  }
  if(num) {
    list = Malloc(sizeof(char *) * num);
    if(list) {
      num=0;
      fc = Allfaces;
      while(fc) {
        list[num++]=fc->name;
        fc = fc->next;
      }
    }
    else
      num=0;
  }
  *faces = num;
  return list;
}

/* static struct FaceType facetype[MAX_STYLES]= { */

char **FaceListStyles(int *faces)
{
  char **list;

  list = Malloc(sizeof(char *) * NumOfFaceStyles);
  if(list) {
    register int i;
    for(i=0; i< NumOfFaceStyles; i++)
      list[i] = facetype[i].name;
    *faces = NumOfFaceStyles;
  }
  else {
    *faces = 0;
    list = NULL;
  }
  return list;
}

char *GetFaceStyle(char *style, int *fg, int *bg)
{
  static char buffer[80];
  int i=0;

  do {
    if(!Stricmp(style, facetype[i].name)) {
      char *pnt=buffer;
      int num=0;
     
      strcpy(buffer, "normal"); /* default non-styled style */
      
      if(facetype[i].style&cb_REVERSE) {
	strcpy(pnt, "reverse");
	num++;
	pnt += strlen(pnt);
      }
      if(facetype[i].style&cb_BOLD) {
	if(num++)
	  *pnt++='|';
	strcpy(pnt, "bold");
	pnt += strlen(pnt);
      }
      if(facetype[i].style&cb_ITALIC) {
	if(num++)
	  *pnt++='|';
	strcpy(pnt, "italic");
	pnt += strlen(pnt);
      }
      if(facetype[i].style&cb_UNDERLINE) {
	if(num++)
	  *pnt++='|';
	strcpy(pnt, "underline");
	pnt += strlen(pnt);
      }
      *fg = READ_FG(facetype[i].pen);
      *bg = READ_BG(facetype[i].pen);
      return buffer;
    }
  } while(++i<MAX_STYLES && facetype[i].name);
  return NULL;
}

/*
  FPL USAGE:
  
  Get the style named "c-keywords" OR create one that is bold with
  foreground pen 3 and background pen 0. If none matched and none
  could be created, it will return the style of the best match and
  no style will be named like this.
  
    style = FaceStyle("c-keywords", "bold", 3, 0);
    
*/

#define FACE_REWRITE 1

int __regargs FaceStyle(char *name, char *flags, int fg, int bg, int opts)
{
  int i=0;
  long style;
  short pens;
  char found=FALSE;
  do {
    if(!Stricmp(name, facetype[i].name)) {
      if(opts & FACE_REWRITE) {
        found = TRUE;
        break;
      }
      return i;
    }
  } while(++i<MAX_STYLES && facetype[i].name);

  style = FaceStyleString(flags);
  pens = FG(fg) | BG(bg);
  
  if(!found) {
    if(MAX_STYLES == i) {
      /* all types are in use, and we couldn't find the one we wanted,
         try scanning for one that has the pens and style all right
  
         if not this routine should try getting one that fits almost!
         (left TODO) */
      i=0;
      do {
        if((pens == facetype[i].pen) &&
           (style == facetype[i].style))
          return i;
      } while(++i<MAX_STYLES);
      
      return 0; /* the built-in, normal type */
    }
    facetype[i].name  = Strdup(name);
    ++NumOfFaceStyles;
  }
  facetype[i].pen   = pens;
  facetype[i].style = style;
  return i;
}

static long __inline FaceStyleString(char *flags)
{
  long result=0;
  char *pnt=flags;
  do {
    if(!Strnicmp("Bold", pnt, 4)) {
      result |= cb_BOLD;
      pnt+=4;
    } else if(!Strnicmp("Normal", pnt, 6)) {
      result |= cb_NORMAL;
      pnt+=6;
    } else if(!Strnicmp("Italic", pnt, 6)) {
      result |= cb_ITALIC;
      pnt+=6;
    } else if(!Strnicmp("Underline", pnt, 9)) {
      result |= cb_UNDERLINE;
      pnt+=9;
    } else if(!Strnicmp("Reverse", pnt, 7)) {
      result |= cb_REVERSE;
      pnt+=7;
    } else
      ++pnt;
  } while('|' == *pnt++);
  
  return result;
}


/*
  FPL USAGE:

  FaceAdd(face,  // add word(s) to this face
          style, // use the c-keywords style
                 // Specify all words we want to add to this face with this
                 // particular style, we can of course add more words at a
                 // later time
          "for|do|while|else|if|int|long|short|char|extern",
          "word" // these are word-only matches, that must be surrounded
                 // with non-word letters to get recognized
         );
*/

int __regargs FaceAdd(struct FaceControl *fc,
                      int style,
                      char *addstring,
                      long addlen,
                      char *flagstr,
                      char *endstring,
                      long endlen)
{
  struct FaceControl *fco=Allfaces;
  long flags;
  char usepipe=FALSE;
  char *end;
  char *start;
  int ret;
  while(fco) {
    if(fc == fco)
      break;
    fco = fco->next;
  }
  if(!fco)
    /* illegal face specified! */
    return 0;
  flags = Faceaddflags(flagstr, &usepipe);

  if(endstring || usepipe) {
    /* one single string */
    /*
       Example use:
    
      AddFaceString(facecontrol, 0, "while", 5, NULL, 0, ADD_WORDONLY);
    */
    ret = AddFaceString(fc, style, addstring, addlen,
                        endstring, endlen, flags);
  }
  else {
    /* possibly multiple strings */
    start = addstring;
    do {
      end = strchr(start, '|');
      if(!end)
        end = strchr(start, '\0');
      if(end-start)
        ret = AddFaceString(fc, style, start, end-start, NULL, 0, flags);
      start = end+1;
    } while (!ret && ('|' == *end));
  }
  return ret;
}

/*
 FLAGS for FaceAdd():
 ====================
 usepipe   - the pipe ('|') letter is a part of the actual string
 word      - the string(s) must match as a word-only
 anywhere  - matches wherever it appears in the text (default)
 strong    - a strong string. Will conquer weak ones.
 weak      - a weak string. Will be conquered by strong ones.
 1nonspace - must be the first non-whitespace on a line to match
 backslash - a letter following a backslash will be ignored
*/

static __inline long Faceaddflags(char *flagstr, char *usepipe)
{
  long result=0;
  char *pnt=flagstr;
  do {
    if(!Strnicmp("usepipe", pnt, 7)) {
      *usepipe=TRUE;
      pnt+=7;
    } else if(!Strnicmp("word", pnt, 4)) {
      result |= ADD_WORDONLY;
      pnt+=4;
    } else if(!Strnicmp("anywhere", pnt, 8)) {
      result &= ~ADD_WORDONLY;
      pnt+=8;
    } else if(!Strnicmp("strong", pnt, 6)) {
      result |= ADD_IMPORTANT;
      pnt+=6;
    } else if(!Strnicmp("weak", pnt, 4)) {
      result |= ADD_OBEYIMPO;
      pnt+=4;
    } else if(!Strnicmp("1nonspace", pnt, 9)) {
      result |= ADD_1STNONSPC;
      pnt+=9;
    } else if(!Strnicmp("backslash", pnt, 9)) {
      result |= ADD_BACKSLASH;
      pnt+=9;
    } else
      ++pnt;
  } while('|' == *pnt++);
  
  return result;
}

void __regargs CleanupAllFaces()
{
  struct FaceControl *fc = Allfaces;
  struct FaceControl *next=NULL;
  int i;
  if(fc) {
    do {
      next = fc->next;
      for(i=0; i<FACETABLE_SIZE; ++i) {
        struct Face *facestr = fc->strings[i];
        struct Face *facenxt;
        facestr = fc->strings[i];
        while(facestr) {
          facenxt = facestr->next;
          Dealloc(facestr);
          facestr = facenxt;
        }
      }
      Dealloc(fc->conts); /* remove conts pointer allocation */
      Dealloc(fc);        /* remove the full face */
    } while(fc = next);
  }
  for(i=0; i<MAX_STYLES; ++i) {
    if(facetype[i].name && !(facetype[i].flags&FT_STATIC)) {
      Dealloc(facetype[i].name);
    }
  }
}
