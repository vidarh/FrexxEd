/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
#ifndef BUF_H
/****************************************************************
 *
 * Buf.h
 *
 * Declares BufStruct and a few other *MINOR* things... :-)
 *
 *********/

#include <exec/types.h>
#include <exec/tasks.h>
#include <libraries/dos.h>
#include <libraries/reqtools.h>
#include <intuition/intuition.h>
#include <proto/dos.h>

#include "Strings.h"
#include "Function.h"


#define BUF(x)    (Storage->x)
#define SHS(x)    (Storage->shared->x)
#define RAD(x)    (Storage->shared->text[x].text)
#define LEN(x)    (Storage->shared->text[x].length)
#define FOLD(x)   (Storage->shared->text[x].fold_no)
#define LFLAGS(x) (Storage->shared->text[x].flags)	/* LineFlags */
#define TEXT(x)   (Storage->shared->text[x])
#define FLAG(x)   (Storage->Flag.x)
#define LOC(x)    (((LocalStruct *)(FindTask(NULL)->tc_UserData))->x)

#define FRONTWINDOW Default.WindowDefault.next

/* Struktur för textpekare.  Storleken definieras även i UpdtScreenML.i */
typedef struct {
  char *text;
  int length;
  WORD fold_no;
  WORD flags;
  char current_style;
  char old_style;
  WORD trash;
} TextStruct;


typedef struct { 
  ULONG block_begin_x;  /* begin column of block (x) */
  ULONG block_begin_y;  /* begin line of block (y) */
  ULONG block_end_x;    /* end column of block (x) */
  ULONG block_end_y;    /* end line of block (y) */
  ULONG blocktype;	/* type of block.  Defined in 'Block.h' */
  char block_exists;	/* existance of a block */
 /* These are just dummys, but important! */
  ULONG block_anc_x;      /* anchor (x) position of the block */
  ULONG block_anc_y;      /* anchor (y) position of the block */
  ULONG block_last_x;	/* last block position */
  ULONG block_last_y;
} BlockCutStruct;
/* Defines for block_exists */
#define be_NONE		0	/* No block marked */
#define be_MARKING	1	/* Block is now under marking */
#define be_EXIST	2	/* Block is marked */


typedef struct {	/* for the slider border */
  struct Gadget gadget;
  struct Border border1;
  SHORT pairs1[10];
} FrexxBorder;


struct FrexxSlider {	 		/* bufs slider */
  struct Gadget Slider;
  struct PropInfo SliderSInfo;
  struct Image SliderImage;
  FrexxBorder Border;
};

struct Script { /* FPL program structure */
  char *file;		/* file name of the FPL program */
  char *name;		/* FPL program name */
  char *program;	/* the program in one zero terminated string, or if
			   it's not loaded yet, NULL */
  int len;		/* length of program */
  struct Script *next;	/* pointer to next Script structure */
};

struct Kmap { /* Internal keymap-structure */
  int Qual;	      /* Qualifier info. ORed !!! */
  USHORT Code;        /* RAWKEY values! */
  char *string;       /* Keymap string to invoke function */
  int Command;	      /* Function define from "Command.h"
  			 If linked list: pointer to a text. */
  char *FPLstring;    /* FPL string to execute */
  int flags;          /* Flags defined in 'Init.h' */
  char *depended;     /* Depending setting. */
  struct OwnMenu *menu;/* Connected menu. Set by MenuBuild */
  struct Kmap *mul;   /* Possible linked list of multiple keypresses */
  struct Kmap *father;/* The previous Kmap in the multiple chain */
  struct Kmap *next;  /* next structure */
};

/*
struct BlockStruct {
  char **text;
  int *length;
  int lines;
  int maxlines;
  int blocktype;	// type of block.  Defined in 'Block.h'
  struct BlockStruct *next;
  char name[1];
};

typedef struct BlockStruct BlockStruct;
*/

typedef struct {
  int toprad;
  int buffer;
  int radcounter;
  WORD counter;
  int textline_no;
  int screenline_no;
  int lineslen;
  int lines;
  int linesdummy;
  int len;
  WORD returnsub;
  struct EndPosStruct *EndPos;
  struct FACT *UsingFact;
  WORD fold_depth;
  WORD fold_flags;
  LONG start_text;
  char line_fixed;
} UpdateStruct;

typedef struct
{
  int len;
  char *string;
} UndoStruct;


typedef struct
{
  int length;
  char *string;
} String;

struct SharedStruct
{
/*-----------------------------------------------------------------*/
/* This is the main file headers, don't change the order */
  TextStruct *text;     /* pointer to all TextStructure pointers */
  int line;             /* total number of lines */
  int taket;            /* number of allocated lines */
  int size;		/* Total buffer size (number of bytes) */
/*-----------------------------------------------------------------*/
  int face_updated_line;/* Hur långt face-uppdateringen kommit */
  struct FaceControl *face;
  char *face_name;
  UndoStruct **UndoBuf;
  int Undopos;          /* the structure number we are undoing */
  int Undomax;          /* max structures allocated in undo buffer */
  int Undomem;		/* memory allocated by the undo buffer */
  int Undostrpos;       /* string position of the current undotext */
  int Undotop;          /* top of undo array */
  int changes;          /* changes since last save */
  char *filnamn;        /* name of the buffer (without path) */
  char *path;           /* path of the buffer (without filename) */
  char *fileblock;	/* pointer to the block where the origin file
			   is stored. */
  struct DateStamp date;/* Date file last saved */
  int fileblocklen;	/* length of the file block */
  int filetime;		/* Time of file creation. Used to see if the file has
			   been changed by something else when FrexxEd saves. */
  LONG fileprotection;	/* Protection flags for file. */
  char protection[8];	/* String for the protection bits */
  char filecomment[80];	/* File Comment, 80 chars */
  int fragment;		/* number of holes in the fileblock */
  int fragmentlen;	/* memory lost by fragmentation */
  int shared;		/* number of buffers sharing this structure */
  int name_number;	/* number of file */
  int readlock;		/* Number of read locks on this file */
  struct Task *writelock;/* Task that locked this file for write (NULL=no lock) */
  struct LockStruct *Locks;/* Lock on this file */
  int type;		/* Type of buffer.  Bit 0-File, 1-Macro, 2-Block. */ 
#define type_FILE	(1 << 0)
#define type_MACRO	(1 << 1)
#define type_BLOCK	(1 << 2)
#define type_HIDDEN	(1 << 3)
  char pack_type[5];    /* Packing type string, no string == no packing */
  char SaveInfo;        /* Create a '.info'-file when saving. */
  char Undo;		/* Undo on/off */
  WORD update;		/* Update is needed (0-1) */
  struct SharedStruct *Next;/* Linked list of all SharedStructs */
  struct SharedStruct *Prev;
  struct BufStruct *Entry;/* Pointer to a BufStruct to use to get this SharedStruct */
  int *LokalInfo;	/* Array of the userdefined LokalInfo settings */
  char autosave;	/* AutoSave hook shall be executed */
  char asenabled;	/* Autosave is enabled */
  char freeable;	/* Buffer can be freed */
  char password[80];	/* encryption password */
  char *macrokey;	/* Macro key */
  int buffer_number;	/* Unikt buffernummer */
  int fold_depth;
  WORD locked;		/* SharedStruct is locked */
  WORD current;		/* Can't be quit from the file handler */
  char *comment_begin;
  char *comment_end;
  char writeprotected;
};

typedef struct SharedStruct SharedStruct;
typedef struct SharedStruct BlockStruct;

struct LockStruct
{
  struct LockStruct *Next;	/* Anchor */
  struct LockStruct *Prev;
  SharedStruct *shared;		/* Buffer that is locked */
  struct Task *task;		/* Task that locked this file */
  int type;			/* type of lock (read/write) */
};
typedef struct LockStruct LockStruct;


typedef struct
{
  char l_c;		/* Flag for line number */
  char insert_mode; 	/* insert mode */
} Flags;

struct BufStruct
{
  /*                           WARNING !!!
             DO NOT CHANGE ANYTHING BETWEEN THE "====" MARKS !!!
    =================================================================== */
  SharedStruct *shared;	/* pointer to shared struct */
  int screen_lines;     /* number of possible lines in the window */
  int screen_col;       /* number of possible columns in the window */
  int tabsize;          /* size of tab */ 
  int curr_topline;     /* which line is at the top of the screen */
  int cursor_x;         /* in which column is the cursor */
  int cursor_y;         /* in which line is the cursor */
  int screen_x;         /* x-position of the screen
                           Good to know when editing lines
                           longer than screen_col... */

  int rightmarg;        /* how long from the right edge the screen will
                           shift 'move_screen' characters to the left */
  int leftmarg;         /* like above, but vice versa */
  int uppermarg;	/* copy of the default margs */
  int lowermarg;
  int left_offset;	/* Bufferposition relativly the backdrop window... */
  int top_offset;	/* ...counted i cursor-steps */
  int l_c_len;		/* length of line number counting. */
  WORD fold_len;		/* width of fold. */
  /* =================================================================== */
  ULONG block_begin_x;        /*	begin column of block (x)		*/
  ULONG block_begin_y;       /*	begin line of block (y)		       */
  ULONG block_end_x;        /*	end column of block (x)		      */
  ULONG block_end_y;       /*	end line of block (y)		     */
  ULONG blocktype;	  /*	type of block.  Defined in 'Block.h'*/
  char  block_exists;    /* existance of a block		   */
  ULONG block_anc_x;    /* anchor (x) position of the block       */
  ULONG block_anc_y;   /* anchor (y) position of the block       */
  ULONG block_last_x; /* last block position                    */
  ULONG block_last_y;
  /* ============ Don't touch, it's a copy of BlockCutStruct ==== */
  int curr_line;	/* current line */
  int face_top_updated_line;/* Översta säkra uppdaterade face-raden */
  int face_bottom_updated_line; /* Nedersta säkra uppdaterade face-raden */

  int move_screen;	/* copy of the default move_screen */
  union {
    int reg;
    struct {
      WORD reg1;
      WORD reg2;
    } r;
  } reg;		/* Registered version */
  struct BufStruct *NextShowBuf;	/* next BufStruct on screen*/
  struct BufStruct *PrevShowBuf; 	/* previous BufStruct on screen*/

  struct BufStruct *NextBuf;   		/* next BufStruct */
  struct BufStruct *PrevBuf;   		/* previous BufStruct */
  struct BufStruct *NextHiddenBuf; 	/* next hidden buf */
  struct BufStruct *PrevHiddenBuf; 	/* previous hidden buf */
  struct BufStruct *NextSplitBuf;   	/* next duplicated BufStruct */
  struct BufStruct *PrevSplitBuf;   	/* previous duplicated BufStruct */
  int view_number;			/* number of view */
  struct FrexxSlider slide; 		/* bufs slider */
  int string_pos;       /* which byte in the string do you stand on */
  int wanted_x;		/* the cursor_x-position the cursor should have if
			   the line was long enough. */
  char expunge;		/* Buffer is to be freed, as soon as possible */

  Flags Flag;           /* All strange flags. See above! */
  char namedisplayed;	/* Flag telling if the name is displayed (bit 0) or statusline (bit 1) */
  WORD update;		/* Update is needed */
  short locked;		/* This buffer cannot be freed */
  struct FACT *using_fact;
  struct WindowStruct *window; /* current window */
  char *fact_name;
  char visible_bitplanes;
  char text_using_bitplanes;

  char slider_on;	/* slider attached */
};

struct FrexxHook {
  struct FrexxHook *next;
  struct FrexxEdFunction *func; /* function descriptor */
  char *name;	/* name of the program to call (strdup()'ed) */
  char flags;	/* see defines below */
  char settings[1]; /* dependent setting */
};

typedef struct BufStruct BufStruct;

typedef struct WindowStruct {
  ULONG DisplayID;	/* Display ID for your screen */
  char *PubScreen;	/* Public screen to put FrexxEd on */
  char *FrexxScreenName;/* The name of the screen FrexxEd is on */
  char autoscroll;	/* AutoScroll on/off */
  ULONG OverscanType;
  char window_position;	/* How to place the window (visible/absolut) */
#define WINDOW_VISIBLE 0
#define WINDOW_ABSOLUT 1
  int screen_height;	/* Screen height. */
  int screen_width;	/* Screen width. */
  int screen_depth;	/* Screen depth. */
  int window_height;	/* Window height. */
  int window_width;	/* Window width. */
  int window_minheight;	/* Window minheight. */
  int window_minwidth;	/* Window minwidth. */
  int window_xpos;	/* Window xpos. */
  int window_ypos;	/* Window ypos. */
  int window_lines;	/* Maximum visible lines */
  int window_col;	/* Maximum visible columns */
  int window_left_offset;	/* left offset */
  int window_right_offset;	/* right offset */

  int real_screen_height;	/* Screen height. */
  int real_screen_width;	/* Screen width. */
  int real_window_height;	/* Window height. */
  int real_window_width;	/* Window width. */
  int real_window_xpos;	/* Window xpos. */
  int real_window_ypos;	/* Window ypos. */
  char autoresize;	/* Automatic resize of the window to fit cursorposition */

  int color0;		/* RGB-value. */
  int color1;
  int color2;
  int color3;

  char appwindow;	/* Should FrexxEd open as an appwindow */

  int move_screen;	/* number of character positions the screen should
			   move at least, when hitting the right or the
			   left margin. */

  char *window_title;
  int Views;            /* Number of entries visible */
  char slider;		/* slider off, right, left */
#define sl_OFF   0	/* slider position */
#define sl_RIGHT 1
#define sl_LEFT  2
  int block_pen;
  int cursor_pen;
  int colour_status_info;

  char window;		/* Startup FrexxEd as a window. (0-3)*/
#define FX_SCREEN 0
#define FX_WINDOW 1
#define FX_BACKDROP 2
#define FX_WINSCREEN 3

#define FX_WINDOWBIT 1
  struct Window *window_pointer; /* Window pointer */
  struct Screen *screen_pointer; /* Screen pointer */
  struct Screen *pubscreen; /* PubScreen pointer */
  struct AppWindow *appwindow_pointer;

  struct WindowStruct *next;
  struct WindowStruct *prev;
  struct BufStruct *NextShowBuf;
  struct BufStruct *ActiveBuffer;

  void *visualinfo;		/* VisualInfo for GadTools */

  BOOL ps_frontmost; /* Fönstret har kapat en screen */
  BOOL ownscreen;	/* Tells if a screen is opened */

  UpdateStruct UpdtVars;

  struct Menu *menus;

  char *appiconname;
  struct AppIcon *appicon;

  char iconify;
  char Visible; /* Storleken på fönstret räcker för att skriva ut med */

  int UpDtNeeded;
  int redrawneeded;
  char window_resized;

  char graphic_card;

  int top_border_height;
  int text_start;

  int flags; /* Declare in OpenClose.h */

  APTR window_lock_id;  /* Return value from rtLockWindow */
};

typedef struct WindowStruct WindowStruct;

typedef struct 
{
  BufStruct BufStructDefault;
  SharedStruct SharedDefault;
  WindowStruct WindowDefault;
  int l_c_len_store;	/* length to store in l_c_len. */
  int fold_margin;	/* width to use for fold marks. */
  struct TextAttr font; /* font structure */
  struct TextAttr RequestFontAttr; /* RequestFont structure */
  char *SystemFont;	/* Text syntax for the font */
  char *RequestFont;
  char *StartupFile;	/* FPL-file to execute at beginning */
  int Undo_maxline;
  int Undo_maxmemory;
  char unpack;		/* automatic unpack of packed buffers on/off */
  int taskpri;		/* taskpriority */
  char full_size_buf;   /* should a new window take maximum size (0-2)*/
  int num_of_key;       /* number of keymap entries */
  struct Kmap key;      /* The internal keymap that controls all functions.
                           This is an empty structure used to find the chain */
  char *ARexxPort;	/* Name of the ARexx port for this FrexxEd. */
  int fragmentmax;	/* Max number of fragments before GC */
  int fragmentmemmax;	/* Max fragmented memory before GC */

  char RMB;		/* Programmable (R)ight (M)ouse (B)utton on/off */
  char safesave;	/* Use a tempfile to save the file to (0-1) */
  /*
   * when a new buffer is loaded...
   */
  
  char rwed_sensitive;  /* Displays a warning if the W(rite) or D(elete) flag
			   isn't set. (0-1)*/
  char *defaultfile;	/* Default file */
  BlockStruct *FirstBlock;

  struct FrexxHook *hook[MAX_COMMANDS]; /* points to an array of pointers to the internal
			      hooks! */
  struct FrexxHook *posthook[MAX_COMMANDS]; /* points to an array of pointers to the post-
			          hooks! */
  char *workstring;
  int worklength;
  int worklengthmax;
  char **storeworkstring;
  int *storeworklength;
  SharedStruct *oldShared;
  char showpath;	/* Path is to be shown with the file name (0-1)*/
  int *GlobalInfo;	/* Array of the userdefined GlobalInfo settings */
  int globalinfoalloc;	/* Number of allocated userdefined GlobalInfo */
  int lokalinfoalloc;	/* Number of allocated userdefined LokalInfo */
  int globalinfoantal;	/* Number of used userdefined GlobalInfo */
  int lokalinfoantal;	/* Number of used userdefined LokalInfo */
  int Entries;          /* Number of entries */
  int Buffers;          /* Number of buffers */
  BPTR KeyMapseg;	/* Segment for the keymap */
  char *KeyMap;		/* Name of the keymap */
  char appicon;		/* Should FrexxEd open an appicon */
  char full_path;	/* Filens 'absoluta' path ska tas ut vid laddning. */
  char fold_save;
  int autosaveintervall;/* AutoSave intervall. */
  int commandcount;	/* Räknare inför varje Command() anrop. */
  char *language;       /* language in use */
  int search_flags;
  char search_limit;
  char search_wildcard;
  char search_case;
  char search_word;
  char search_block;
  char search_forward;
  char search_prompt;
  char search_fold;

  int version_int;
  char *version_string;

  char *directory;	/* Current directory */
  BPTR olddirectory;
  struct Task *task;

  char *FPLdirectory;
  char *status_line;
  char FPLdebug;
  char fast_scroll;

  char *diskname;      /* Name of the mounted filehandler, set to NULL to
                          prevent mounting */
  char filehandler;

  char *file_pattern;
  int page_length;

  int page_overhead; // överlappning vid page up/down

  char req_ret_mode;

  int windows_allocated;
  int windows_opened;
} DefaultStruct;


struct LocalStruct {
  char *LinkerDB;		/* LinkerDB pointer */
  int processnumber;		/* Which process is this */
};

typedef struct LocalStruct LocalStruct;


#define VISIBLE_ON		0
#define VISIBLE_OFF		1
#define VISIBLE_ICONIFIED	2

#define BUF_H
#endif	//BUF_H
