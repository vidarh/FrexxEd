/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/*********************************************************
 *
 * Function.h
 *
 */

#define NO_HOOK	   (1<<15) /* No hook shall be trapped on this funktion */
#define LOCK_READ  (1<<14) /* A read lock must be performed before command execution */
#define LOCK_WRITE (1<<13) /* A write lock must be performed before command execution */
#define FPL_CMD	   (1<<12) /* This command can only be called from a FPL script */
#define YANK_COMMAND (1<<11) /* This command append its deletion to the yank buffer */

#define LOCKINFO_BITS (NO_HOOK|LOCK_WRITE|LOCK_READ|FPL_CMD|YANK_COMMAND)

typedef enum {	/* Commands without any lock */
  DO_NOTHING,		/* Tom */
  DO_NOTHING_RETMSG,	/* Tom */

  DO_AUTOSAVE,
  DO_BUFFER_KILL,
  DO_CLOSE_WINDOW,
  DO_EVENT,
  DO_EXIT,
  DO_FILECHANGED,
  DO_GETFILE,
  DO_GOTFILE,
  DO_ICON_CLICK,
  DO_MENU_SELECTED,
  DO_NEWWINDOWSIZE,
  DO_SAMENAME,
  DO_SLIDER,
  DO_FILEHANDLER_EXCEPTION,

  DO_NOT_MACRO_RECORD_ABOVE_THIS_SETTING,

  DO_ABOUT,		/* Tar fram about-f�nstret */
  DO_ASSIGN_KEY,	/* Assigna en funktion till en tangent */
  DO_BLOCK_SORT,	/* Sortera raderna i blocket */
  DO_BLOCK_TO_LOWER,	/* �ndra alla tecken i blocket till lower case */
  DO_BLOCK_TO_UPPER,	/* �ndra alla tecken i blocket till upper case */
  DO_BLOCK_TO_UPPLOW,	/* �ndra alla tecken i blocket */
  DO_CLONEWB,
  DO_COLOR_ADJUST,	/* F�r�ndra f�rgerna p� screenen */
  DO_COLOR_RESET,	/* Kopiera f�rgerna fr�n workbenchen */
  DO_DEICONIFY,		/* Deiconify FrexxEd */
  DO_DELETE_KEY,	/* Delete en tangentfunktion */
  DO_EXECUTE_STRING,	/* Exekvera en FPL-str�ng */
  DO_ICONIFY,		/* Iconify FrexxEd */
  DO_INFORMATION,
  DO_LOAD,		/* Ladda in en fil �ver den gamla bufferten */
  DO_MACRO_RECORD,
  DO_MOUSE_LEFT,
  DO_PROMPT,		/* Ta fram prompt-requestern */
  DO_QUIT,		/* D�da bufferten */
  DO_QUIT_ALL,		/* D�da editorn */
  DO_REMOVE_BUF,	/* Ta bort viewen */
  DO_RESIZE_BUF,	/* �ndra storlek p� viewen */
  DO_SET_SAVE,		/* Spara defaultsettings */
  DO_SET_SCREEN,
  DO_SLASK,  /* temporary */
  DO_SLASK2, /* temporary */
  DO_STATUS,
  DO_WINDOWFRONT,
  MAX_NOLOCK,

	/* Commands with a read lock needed */
  LOCK_READ1=LOCK_READ+(MAX_NOLOCK&~LOCKINFO_BITS), /* dummy */

  DO_BLOCK_COPY,	/* Kopierar markerade blocket till block-bufferten */
  DO_BLOCK_COPY_APPEND,	/* Kopierar markerade blocket till slutet av block-bufferten */
  DO_BLOCK_MARK,	/* S�tt p�/av blockmarkeringen */
  DO_BLOCK_MARK_COL,	/* S�tt p�/av kolumn-blockmarkeringen */
  DO_BLOCK_MARK_LINE,	/* S�tt p�/av rad-blockmarkeringen */
  DO_CURSOR_DOWN,	/* Flytta ner cursorn */
  DO_CURSOR_LEFT,	/* Flytta cursorn till v�nster */
  DO_CURSOR_RIGHT,	/* Flytta cursorn till h�ger */
  DO_CURSOR_UP,		/* Flytta upp cursorn */
  DO_GOTO_LINE,		/* G� till �nskad rad */
  DO_LAST_CHANGE,	/* G� till senaste �ndringen */
  DO_LEFT_WORD,		/* G� till slutet av f�rra ordet */
  DO_MAXIMIZE_BUF,	/* Maximera buffern */
  DO_MATCH_PAREN,	/* G� till matchande parantes */
  DO_PAGE_DOWN,		/* Flytta ner cursorn en sida */
  DO_PAGE_UP,		/* Flytta upp cursorn en sida */
  DO_RIGHT_WORD,	/* G� till n�sta ord */
  DO_SAVE,		/* Spara bufferten */
  DO_SEARCH,		/* G�r search med nuvarande search buffert */
  MAX_READLOCK,

	/* Commands with a write lock needed */
  LOCK_WRITE1=LOCK_WRITE+(MAX_READLOCK&~LOCKINFO_BITS), /* dummy */

  DO_BACKSPACE,		/* G�r en backspace */
  DO_BACKSPACE_WORD,	/* Backspace till b�rjan av ordet */
  DO_BLOCK_CUT,		/* Flyttar markerade blocket till block-bufferten */
  DO_BLOCK_CUT_APPEND,	/* Flyttar markerade blocket till slutet av block-bufferten */
  DO_BLOCK_DELETE,	/* Deletea ett markerat block */
  DO_BLOCK_MOVE,	/* Flyttar markerade blocket i sidled */
  DO_BLOCK_PASTE,	/* Inserta blocket */
  DO_BLOCK_PASTE_COLUMN,	/* Inserta blocket */
  DO_CLEAR,		/* T�m bufferten */
  DO_DELETE_WORD,	/* Deletea ordet du st�r p� */
  DO_ICON_DROP,		/* An icon was dropped in a FrexxEd window */
  DO_INSERT_FILE,	/* Inserta en fil i bufferten */
  DO_REPLACE,		/* Replace */
  DO_UNDO,		/* G�r undo */
  DO_UNDO_RESTART,	/* Resettar l�sning i undo-bufferten */
  DO_YANK,		/* Inserta inneh�llet i yank-bufferten */
  MAX_WRITELOCK,

	/* Commands called only from FPL */
  FPL_CMD1=FPL_CMD+(MAX_WRITELOCK&~LOCKINFO_BITS),

  SC_ACTIVATE,		/* Aktiverar en buffer */
  SC_AREXXREAD,		/* l�s variabel */
  SC_AREXXRESULT,	/* s�tt return-variabeln */
  SC_AREXXSEND,		/* skicka meddelande till port */
  SC_AREXXSET,		/* s�tt variabel */
  SC_BLOCK_ALLOC,	/* Allokerar en ny block-bufferten */
  SC_BLOCK_CHANGE,	/* Byter aktuella block-bufferten */
  SC_BLOCK_FREE,	/* Fria en blockbuffert */
  SC_BSEARCH,
  SC_CCONVERTSTRING,	/* Convertarar en str�ng till C-syntax */
  SC_CENTERSCREEN,
  SC_CLEAN,
  SC_CLIP2STRING,	/* Flyttar ClipBoarden till en str�ng*/
  SC_CONFIRM,
  SC_CONSTRUCT_INFO,
  SC_COPY_SETTING,
  SC_CURRENT_BUF,	/* Byt s� att en viss buffert �r den aktiva */
  SC_CURRENT_WINDOW,	/* Byt s� att ett visst f�nster �r den aktiva */
  SC_CURSOR_ACTIVATE,
  SC_CURSOR_STACK,
  SC_DELAY,
  SC_DELETE_INFO,
  SC_DISPLAYBEEP,
  SC_DUPLICATE_ENTRY,
  SC_ERROR,		/* FPL error exception */
  SC_EXECUTE_BUF,
  SC_EXECUTE_FILE,
  SC_EXECUTE_KEY_STROKE,
  SC_EXECUTE_LATER,
  SC_EXISTS,
  SC_FACEADD,           /* add a string as a face    */
  SC_FACEGET,           /* create a main-face        */
  SC_FACEREAD,          /* read face info */
  SC_FACESTYLE,         /* create/get a style/colour */
  SC_FACTCONVERTSTRING,
  SC_FACT_CREATE,
  SC_FACT_DELETE,
  SC_FINDPORT,		/* finns porten? */
  SC_FOLD,
  SC_FOLD_DELETE,
  SC_FOLD_HIDE,
  SC_FOLD_SHOW,
  SC_GETBLOCK,
  SC_GETBUFFERID,
  SC_GETBYTE,
  SC_GETCHAR,
  SC_GETCURSOR,
  SC_GETDATE,
  SC_GETENTRYID,
  SC_GETERRNO,
  SC_GETERRORMSG,
  SC_GETFILE,
  SC_GETFILELIST,
  SC_GETINT,
  SC_GETKEY,
  SC_GETLINE,
  SC_GETLIST,
  SC_GETSTRING,
  SC_GETVAR,
  SC_GETWINDOWID,
  SC_GETWORD,
  SC_HOOKCLEAR,                /* ta bort hook! */
  SC_INVERSE_LINE,
  SC_ISCLOSE,
  SC_ISFOLD,
  SC_ISLOWER,
  SC_ISNEWLINE,
  SC_ISOPEN,
  SC_ISSPACE,
  SC_ISSTRING,
  SC_ISSYMBOL,
  SC_ISTAB,
  SC_ISUPPER,
  SC_ISWORD,
  SC_KEYPRESS,
  SC_LOADSTRING,	/* Ladda in en fil till en str�ng */
  SC_LOG_SETTING,
  SC_MATCH_SEARCH,	/* Matcha en search-str�ng och returnera replacestr�ngen. */
  SC_MENUADD,
  SC_MENUBUILD,
  SC_MENUCLEAR,
  SC_MENUDELETE,
  SC_MENUREAD,
  SC_NEW_BUF,		/* Initera en ny buffert */
  SC_NEW_PROCESS,	/* Initera en ny process */
  SC_NEXT_BUF,		/* G� till n�sta buffert i kedjan */
  SC_NEXT_ENTRY,	/* G� till n�sta entry i kedjan */
  SC_NEXT_HIDDEN_BUF,	/* G� till n�sta buffert som inte syns */
  SC_NEXT_SHOW_BUF,	/* G� till viewen under den aktiva */
  SC_NEXT_WINDOW,
  SC_PREV_BUF,		/* G� till f�rra bufferten i kedjan */
  SC_PREV_ENTRY,	/* G� till f�rra entryt i kedjan */
  SC_PREV_HIDDEN_BUF,	/* G� till f�rra bufferten som inte syns */
  SC_PREV_SHOW_BUF,	/* G� till viewen �ver den aktiva */
  SC_PREV_WINDOW,
  SC_PRINTLINE,
  SC_PROMPTINFO,
  SC_PROMPT_BUFFER,	/* V�lj en buffert */
  SC_PROMPT_ENTRY,	/* V�lj ett entry! */
  SC_PROMPT_FONT,	/* K�r upp en fontrequester */
  SC_RANDOM,		/* H�mta ett pseudo-randomv�rde */
  SC_READSET,		/* L�s av en int setting */
  SC_REDRAWSCREEN,
  SC_RENAME,		/* �ndra namn p� bufferten */
  SC_REPLACESET,	/* Replace med requester alt argument */
  SC_REQUEST_WINDOW,
  SC_RESET_FACT,
  SC_SAVESTRING,	/* Spara en fil till disk */
  SC_SCROLL_DOWN,	/* Scrolla ner viewen */
  SC_SCROLL_UP,		/* Scrolla upp viewen */
  SC_SEARCHSET,		/* Search med requester alt argument */
  SC_SETCOPY,		/* Kopiera en setting */
  SC_SETFACT,		/* �ndra i FACT'en */
  SC_SETHOOK,
  SC_SETHOOKPAST,
  SC_SETINFO,		/* �ndra en setting */
  SC_SETRETURNMSG,
  SC_SETVAR,
  SC_SET_REQUEST_FONT,	/* Byt request-font */
  SC_SET_SYSTEM_FONT,	/* Byt system-font */
  SC_SORT,		/* Sortera en array */
  SC_STATUS,
  SC_STCGFN,
  SC_STCGFP,
  SC_STRICMP,		/* stricmp() clone */
  SC_STRING2BLOCK,	/* Flyttar en str�ng till ett block */
  SC_STRING2BLOCKAPPEND,/* Appendar en str�ng till ett block */
  SC_STRING2CLIP,	/* Flyttar en str�ng till ClipBoarden */
  SC_STRING_TO_LOWER,
  SC_STRING_TO_UPPER,
  SC_STRING_TO_UPPLOW,
  SC_STRMFP,
  SC_SYSTEM,		/* System kommando */
  SC_TIMER_REMOVE,
  SC_TIMER_START,
  SC_VISIBLE,
  SC_WINDOW_ALLOC,
  SC_WINDOW_CLOSE,
  SC_WINDOW_FREE,
  SC_WINDOW_OPEN,
  MAX_FPL_CMD,

	/* Commands called only from FPL but need a WRITE lock */
  FPL_CMD2=FPL_CMD+LOCK_WRITE+(MAX_FPL_CMD&~LOCKINFO_BITS),
  SC_OUTPUT,		/* Anropar DO_OUTPUT men tar ur l�ngden p� str�ngen */
  MAX_FPL_CMD2,

  YANK_CMD1=YANK_COMMAND+LOCK_WRITE+(MAX_FPL_CMD2&~LOCKINFO_BITS),
  DO_DELETE,		/* Deletea tecknet du st�r p� */
  DO_DELETE_LINE,	/* Deletea raden du st�r p� */
  DO_DELETE_TO_EOL,	/* Deletea allt till h�ger om cursorn */
  MAX_YANK_CMD2,

  MAX_COMMANDS=MAX_YANK_CMD2&~LOCKINFO_BITS /* always the last */
} cmd;

typedef enum {

 LAST_ERROR =  -254,

 FAILED			=-27,
 WORKBENCH_NOT_RUNNING  =-26,
 DELETE_PROTECTED 	=-25,
 WRITE_PROTECTED	=-24,
 READ_PROTECTED		=-23,
 UNREG_VERSION		=-22,
 CANT_ALLOC_SETTING	=-21,
 WRONG_FILE_NAME	=-20,
 CANT_FIND_BUFFER	=-19,
 CANT_LOCK_BUFFER	=-18,
 IFF_ERROR		=-17,
 NO_BLOCK_FOUND		=-16,
 CANT_FIND_FILE		=-15,
 FPL_NOT_FOUND		=-14,
 CANT_FIND_SETTING	=-13,
 WRONG_BLOCKTYPE	=-12,
 NO_MATCH		=-11,
 FUNCTION_CANCEL	=-10,
 CANT_FIND_MORE		=-9,
 EMPTY_BLOCKBUFFER	=-8,
 OPEN_ERROR		=-7,
 NOTHING_TO_DELETE	=-6,
 NO_BLOCK_MARKED	=-5,
 NOTHING_TO_UNDO	=-4,
 SYNTAX_ERROR		=-3,
 OUT_OF_MEM		=-2,
 UNKNOWN_COMMAND	=-1,
 OK			=0,
 QUIT_COMMAND,
 NEW_STORAGE,
 NO_KEYREPEAT,
 WINDOW_REOPENED,

 LAST_CODE
} RetCode;

typedef struct FrexxEdFunction {
  char *name;
  cmd ID;
  char ret;
  char *format;
  char type;	// Initieringen ska ske i 'f�rsta gruppen'
};

