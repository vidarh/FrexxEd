/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */

#include <devices/console.h>
#include <devices/keymap.h>
#include <dos/rdargs.h>
#include <exec/types.h>
#include <exec/semaphores.h>
#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <libraries/dos.h>
#include <libraries/gadtools.h>
#include <libraries/reqtools.h>
#include <libraries/locale.h>
#include <libraries/FPL.h>
#include <workbench/workbench.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <utility/tagitem.h>
#include <exec/types.h>
#include <string.h>

#include "Buf.h"

#include "Alloc.h"
#include "Command.h"
#include "Declare.h"
#include "Execute.h"
#include "Fact.h"
#include "FrexxEd_rev.c"
#include "Function.h" /* the struct FrexxEdFunction */
#include "IDCMP.h"
#include "Limit.h"
#include "Prompt.h"
#include "Rexx.h"
#include "Search.h"
#include "Setting.h"
#include "Init.h"
#include "UpdtScreenC.h"
#include "Buildmenu.h" /* for struct MenuInfo */
#ifdef USE_FASTSCROLL
#include "fastGraphics.h"
#endif

/************************ former allocated structures */
struct AllocCache alloc_cache[ALLOC_CACHE_MAXSIZE/8];
DefaultStruct Default; /* the default values of a BufStruct */
char buffer[MAX_CHAR_LINE];	/* temp buffer */
char fastmap1[256];
char fastmap2[256];
struct RastPort ButtonRastPort;	//Rastport f�r att kunna testa med Requestfonten.


/****************************** Alloc.c *********************************/
int allowcleaning=TRUE;
BOOL cache_allocs=FALSE;
char *firstalloc=NULL;		/* only for debug use */
int totalalloc=0;
char GlobalEmptyString[]="";
char *lockedalloc=NULL;
BOOL freelockedalloc=FALSE;


/****************************** BufControl.c ****************************/
int bufferlen=0;
int editmax=0;	/* how many buffers we have */
int buffer_number=0;

/****************************** Buildmenu.c *****************************/

struct MenuInfo menu={NULL, 0, NULL, 0, 0};
char build_default_menues=2;
struct OwnMenu *menu_settingchain=NULL;

/****************************** Button.c ********************************/

char **setting_string_pointer=NULL;
int *setting_int_pointer=NULL;


/****************************** Comm.c **********************************/
char *statusbuffer=NULL;
long statusbuffer_len=0;
char ReturnBuffer[RET_BUF_LEN]="";
char waitreq=0;

/****************************** Command.c *******************************/
int ReturnValue=0;	// Global return value storage.
int ErrNo=0;		// Global Error value storage.
char outputundo[OUTPUT_UNDO_MAX];
int last_command=DO_NOTHING;
BOOL yankcontinue=FALSE;			/* Shall the yank buffer expand */
BlockStruct *YankBuffer;
BlockStruct MacroBuffer={ NULL, NULL, 0, 0, 0, NULL };
int lastoutput=0;
int undoantal=0;
int undoOPbsantal=0;
int undoOPantal=0;
int undoBSantal=0;
int undoDLantal=0;
int MacroOn=0;		/* Macro p� (1) eller av (0) */
int recursive=0;
struct Window *InfoWindow=NULL;

/***************************** DoSearch.c ********************************/

/***************************** Editor.c ********************************/
long __stack=8000;
long __STKNEED=3500;
jmp_buf oldfplstackpoint;
jmp_buf return_stackpoint;

char *cl_frexxecute=NULL, *cl_run=NULL, *cl_portname=NULL, *cl_init=NULL, *cl_pubscreen=NULL, *cl_startupfile=NULL, *cl_execute;
char *cl_diskname=NULL;
int cl_column=0, cl_line=0, cl_copywb=FALSE, cl_double=FALSE;
int cl_omit=FALSE, cl_screen=FALSE, cl_window=FALSE, cl_backdrop=FALSE, cl_debug, cl_iconify=FALSE;
struct RDArgs *argsptr;
/*
LocalStruct Local={
  NULL,
  0
};
*/

char FrexxEdStarted=0;	/* �r FrexxEd uppstartad */
char **FromWorkbench=NULL;       /* started from workbench? argv is placed here */
char DebugOpt=FALSE; /* debug option on */
char *mothername=NULL;		/* Name of the mother process */

/***************************** Execute.c ********************************/
int retval_antal=0;
int retval_params[2];

char **filelog_list=NULL;
int filelog_maxalloc=0;
int filelog_antal=0;

int fpl_error=0;
ReturnCode fpl_executer=NULL;	/* Namn p� den funktion som exekverar FPL */
struct UserData userdata;

BOOL hook_enabled=TRUE;

struct Library *FPLBase = NULL;

void *Anchor=NULL;

/***************************** Fact.c ********************************/
FACT *DefaultFact=NULL;

/***************************** FileHandler.c ********************************/
struct ProcMsg *filehandlerproc;
struct MsgPort *filehandlerport;
struct Task *filehandlertask;
struct DeviceList *device;
char filehandlerstop=FALSE; /* set TRUE when CTRL_C received! */

long fh_locks=0;
long fh_opens=0;
/***************************** GetFont.c ******************************/
				/* Font used by the system/screen */
struct FastGraphicsTable *fast_gfx_table=NULL;
struct TextFont *SystemFont=NULL;
struct TextFont *RequestFont=NULL;	/* Font used by requsters */
struct TextFont *DefaultFont=NULL;	/* Font used by requsters */
int systemfont_leftmarg=0;
int systemfont_rightmarg=0;
int requestfontwidth=0;		// Kalkylerad bredd p� requestfonten.

/***************************** IDCMP.c ********************************/

short important_message_available=0;
BOOL device_want_control=FALSE;
BOOL device_has_killed_a_buffer=FALSE;
BOOL clear_all_currents=FALSE;
int zoomstate=0;
int devicelock=0;
BOOL frexxedrunning=TRUE;
int signalbits=0; /* Which signal bits to Wait() for! */
BOOL fplabort=FALSE;		/* Allow to abort FPL scripts.  Startup script shouldn't be breakable */
ReturnMsgStruct *firstreturnmsg=NULL;
BufStruct *NewStorageWanted=NULL;
struct IntuiMessage *IDCMPmsg=NULL;
UWORD lastmsgCode=NULL;
struct InputEvent ievent = {	/* used for RawKeyConvert() */
  NULL,
  IECLASS_RAWKEY,
  0,0,0
};
int mouse_x=-1;		/* Mouse position from last input, -1==no mouse pressed */
int mouse_y=-1;		/*   - "" - */
struct Window *activewindow=NULL;	// Current active FrexxEd window
struct KeyMap *internalkeymap=NULL;

/***************************** Init.c **********************************/

struct PropInfo VertSliderSInfo = {			/* Default */
  AUTOKNOB|FREEVERT|PROPNEWLOOK|PROPBORDERLESS,       /* PropInfo flags */
  65535,65535,             /* horizontal and vertical pot values */
  65535,65535,             /* horizontal and vertical body values */
   0,0,0,0,0,0
};

struct Image SliderImage = {			/* Default */
  0,0,7,1,0, NULL,0x0000,0x0000, NULL
};

struct Gadget VertSlider = {			/* Default */
  NULL, -20, 20, 16, 20,
  NULL,
  RELVERIFY|GADGIMMEDIATE,
  PROPGADGET,                   /* Proportional Gadget */
  NULL, 			  /* Slider Imagry */
  NULL,NULL,NULL,
  NULL,			  /* SpecialInfo structure */
  1, /* SLIDER,*/
  NULL
};

FrexxBorder borderdef = {
  {
    NULL, 0, 0, 8, 0,
    GADGHBOX,
    NULL, //RELVERIFY,
    BOOLGADGET,
    NULL,
    NULL,NULL,NULL,
    NULL,
    0,
    NULL
  },
  {
    -1, -1,
    2, 0,
//    JAM1, 5,
    JAM1, 2,
    NULL, NULL
  },
  {
    0,0,0,0,0,0,0,0,0,0
  },
};

/***************************** KeyAssign.c *****************************/

const char hextab[]={ '0','1','2','3','4','5','6','7',
                      '8','9','a','b','c','d','e','f'};


const char *converttable[][2]={
  {"F1",	"\xfb\x9b""0~"},
  {"F2",	"\xfb\x9b""1~"},
  {"F3",	"\xfb\x9b""2~"},
  {"F4",	"\xfb\x9b""3~"},
  {"F5",	"\xfb\x9b""4~"},
  {"F6",	"\xfb\x9b""5~"},
  {"F7",	"\xfb\x9b""6~"},
  {"F8",	"\xfb\x9b""7~"},
  {"F9",	"\xfb\x9b""8~"},
  {"F10",	"\xfb\x9b""9~"},
  {"F11",	"\xfb\x9b""10~"},
  {"F12",	"\xfb\x9b""11~"},
  {"F13",	"\xfb\x9b""12~"},
  {"F14",	"\xfb\x9b""13~"},
  {"F15",	"\xfb\x9b""14~"},
  {"F16",	"\xfb\x9b""15~"},
  {"F17",	"\xfb\x9b""16~"},
  {"F18",	"\xfb\x9b""17~"},
  {"F19",	"\xfb\x9b""18~"},
  {"F20",	"\xfb\x9b""19~"},
  {"Del",	"\xfb\x7f"},
  {"Delete",	"\xfb\x7f"},
  {"Help",	"\xfb\x9b?~"},
  {"Up",	"\xfb\x9b""A"},
  {"Down",	"\xfb\x9b""B"},
  {"Right",	"\xfb\x9b""C"},
  {"Left",	"\xfb\x9b""D"},
  {"Esc",	"\xfb\x1b"},
  {"Escape",	"\xfb\x1b"},
  {"Enter",	"\x43"},
  {"Return",	"\x44"},
  {"Tab",	"\xfb\t"},
  {"Bspc",	"\xfb\b"},
  {"Backspace",	"\xfb\b"},
  {"Spc",	"\xfb "},
  {"Space",	"\xfb "},
  {" ",		"\xfb "},
  {"num0",	"\x0f"},
  {"num1",	"\x1d"},
  {"num2",	"\x1e"},
  {"num3",	"\x1f"},
  {"num4",	"\x2d"},
  {"num5",	"\x2e"},
  {"num6",	"\x2f"},
  {"num7",	"\x3d"},
  {"num8",	"\x3e"},
  {"num9",	"\x3f"},
  {"num[",	"\x5a"},
  {"num]",	"\x5b"},
  {"num/",	"\x5c"},
  {"num*",	"\x5d"},
  {"num-",	"\x4a"},
  {"num+",	"\x5e"},
  {"num.",	"\x3c"}
};

const int converttablelength=sizeof(converttable)/((sizeof(char *)*2));

/***************************** OpenClose.c *****************************/

int semaphore_count=0;
struct Task *FrexxEdTask=NULL;
struct SignalSemaphore LockSemaphore;
BOOL open_copywb=TRUE;
char ignoreresize=0;
BOOL quitting=FALSE;
WORD sliderhighlite, statuscolor, statustextcolor;
int visible_height=0;
int visible_width=0;

char *appiconname=NULL;

static USHORT Image1Data[] = {
  0x00FF, 0xFFFF, 0xFFC0, 
  0x0080, 0x0FFC, 0x0000, 
  0x01C0, 0x07F8, 0x0000, 
  0x0360, 0x03F0, 0x0000, 
  0x06F0, 0x01E0, 0x0000, 
  0x07F0, 0x00C0, 0x0000, 
  0x07F0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x01C0, 0x00C0, 0x0000, 
  0x01C0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x0DF8, 0x00C0, 0x0000, 
  0x13FC, 0x00C0, 0x0000, 
  0x2FFE, 0x00C0, 0x0000, 
  0x2C1E, 0x00C1, 0xF000, 
  0x780F, 0x00C1, 0x0000, 
  0x7007, 0x00C1, 0x0000, 
  0x6003, 0x00C1, 0xE000, 
  0x6003, 0x00C1, 0x0000, 
  0x6003, 0x00C1, 0x1040, 
  0x2002, 0x00C1, 0x0880, 
  0x2002, 0x00C1, 0x0500, 
  0x1004, 0x00C0, 0x0200, 
  0x09C8, 0x00C0, 0x0500, 
  0x07F0, 0x00C0, 0x0880, 
  0x0470, 0x00C0, 0x1040, 
  0x0DF8, 0x00C0, 0x0000, 
  0x0FF8, 0x00C0, 0x0000, 
  0x0FF8, 0x00C0, 0x0000, 
  0x07F0, 0x80C0, 0x0000, 
  0x07F0, 0x80C0, 0x2000, 
  0x01C0, 0x40C7, 0xE000, 
  0x0000, 0x40C1, 0xE000, 
  0x0000, 0x3001, 0xE000, 
  0x0000, 0x0C06, 0x2000, 
  0x0000, 0x03F8, 0x0000, 

  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0080, 0x0000, 0x0000, 
  0x0100, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0007, 0xFFF0, 
  0x0200, 0x0007, 0xFFF0, 
  0x0C00, 0x0007, 0xFFF0, 
  0x1000, 0x0007, 0xFFF0, 
  0x1000, 0x0006, 0x0FF0, 
  0x0000, 0x0006, 0xFFF0, 
  0x0000, 0x0006, 0xFFF0, 
  0x0000, 0x0006, 0x1FF0, 
  0x0000, 0x0006, 0xFFF0, 
  0x0000, 0x0006, 0xEFB0, 
  0x0000, 0x0006, 0xF770, 
  0x0000, 0x0006, 0xFAF0, 
  0x0000, 0x0007, 0xFDF0, 
  0x0000, 0x0007, 0xFAF0, 
  0x0000, 0x0007, 0xF770, 
  0x0380, 0x0007, 0xEFB0, 
  0x0200, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
};

struct Image AppIcon1 = {
  0, 0, 44, 38, 2, &Image1Data[0], 3, 0, NULL
};

static USHORT Image2Data[] = {
  0x00FF, 0xFFFF, 0xFFC0, 
  0x0080, 0x0FFC, 0x0000, 
  0x01C0, 0x07F8, 0x0000, 
  0x0360, 0x03F0, 0x0000, 
  0x06F0, 0x01E0, 0x0000, 
  0x07F0, 0x00C0, 0x0000, 
  0x07F0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x01C0, 0x00C0, 0x0000, 
  0x01C0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x07F0, 0x00C0, 0x0000, 
  0x1BFC, 0x00C0, 0x0000, 
  0x27FE, 0x00C0, 0x0000, 
  0x5FFF, 0x00C1, 0xDBC0, 
  0x580F, 0x00C0, 0x0000, 
  0xF007, 0x80C1, 0xD980, 
  0xE003, 0x80C0, 0x0000, 
  0xC001, 0x80C1, 0xBD00, 
  0xC001, 0x80C0, 0x0000, 
  0xC001, 0x80C1, 0xDE80, 
  0x4001, 0x00C0, 0x0000, 
  0x4001, 0x00C1, 0xBAC0, 
  0x2002, 0x00C0, 0x0000, 
  0x1004, 0x00C1, 0xB000, 
  0x0000, 0x00C0, 0x0000, 
  0x0000, 0x00C0, 0x0000, 
  0x00E0, 0x00C0, 0x03C0, 
  0x03F8, 0x00C0, 0x0000, 
  0x0238, 0x00C0, 0x0000, 
  0x06FC, 0x00C0, 0x0000, 
  0x07FC, 0x00C0, 0x0000, 
  0x07FC, 0x00C0, 0x0000, 
  0x03F8, 0x00C0, 0x0000, 
  0x03F8, 0x0000, 0x0000, 
  0x00E0, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 

  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0080, 0x0000, 0x0000, 
  0x0100, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0007, 0xFFF0, 
  0x0400, 0x0007, 0xFFF0, 
  0x1800, 0x0007, 0xFFF0, 
  0x2000, 0x0007, 0xFFF0, 
  0x2000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x01C0, 0x0000, 0x0000, 
  0x0100, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
};

struct Image AppIcon2 = {
  0, 0, 44, 38, 2, &Image2Data[0], 3, 0, NULL
};

struct DiskObject AppIconDObj =
{
    NULL,                                        /* Magic Number */
    NULL,                                        /* Version */
    {                                            /* Embedded Gadget Structure */
        NULL,                                    /* Next Gadget Pointer */
        0, 0, 44, 39,                            /* Left,Top,Width,Height */
        GADGHIMAGE,                              /* Flags */
        NULL,                                    /* Activation Flags */
        NULL,                                    /* Gadget Type */
        (APTR) & AppIcon1,                       /* Render Image */
        (APTR) & AppIcon2,                       /* Select Image */
        NULL,                                    /* Gadget Text */
        NULL,                                    /* Mutual Exclude */
        NULL,                                    /* Special Info */
        0,                                       /* Gadget ID */
        NULL,                                    /* User Data */
    },
    WBAPPICON,                                   /* Icon Type */
    NULL,                                        /* Default Tool */
    NULL,                                        /* Tool Type Array */
    NO_ICON_POSITION,                            /* Current X */
    NO_ICON_POSITION,                            /* Current Y */
    NULL,                                        /* Drawer Structure */
    NULL,                                        /* Tool Window */
    NULL                                         /* Stack Size */
};

struct DiskObject *externappicon;

BOOL OwnWindow=FALSE;	/* This task is the owner of a FrexxEd window */

long FrexxPri=-200;

struct Menu *Menus = NULL;

FACT *UsingFact=NULL;		/* Current using FACT.  A free pointer, changed without notice */

srch Search;          /* search structure */
struct Library *FastGraphicsBase=NULL;
struct Library *DiskfontBase=NULL;
struct Library *GadToolsBase = NULL;
struct rtFileRequester *FileReq=NULL; /* Buffrad Filerequester. Bra o ha! */
struct IOStdReq *WriteReq=NULL;
struct MsgPort *WritePort=NULL, *ReadPort=NULL;
struct MsgPort *WindowPort=NULL;
struct MsgPort *WBMsgPort=NULL;
struct Process *editprocess = NULL;
struct Library *ConsoleDevice=NULL; /* Changed definition in 6.0! */
struct PPBase *PPBase = NULL;
struct Library *XpkBase = NULL;
struct ExecBase *execbase;
struct IntuitionBase *IntuitionBase=NULL;
struct GfxBase *GfxBase=NULL;
struct ReqToolsBase *ReqToolsBase=NULL;
struct AppIcon *appicon=NULL;
struct Library *LocaleBase=NULL;
struct Catalog *catalog=NULL;
struct Library *IconBase=NULL;

int Visible=VISIBLE_OFF;
char CursorOnOff=FALSE;	/* Tells the state of the cursor */
char cursorhidden=FALSE;

int clipri=-200;
struct MsgPort *msgport;			/* msgport f�r STICK */
struct MsgPort *msgreply;
struct Message msgsend = {
    NULL,
    NULL,
    NT_MESSAGE,
    0,
    NULL,
  NULL,
  sizeof(struct MsgPort)
};

//struct Screen *screen=NULL;
//struct WindowStruct *window=NULL;
BlockStruct *BlockBuffer;

APTR oldwindowptr = NULL;
struct screen_buffer ScreenBuffer;
long screen_buffer_size=0;
struct Window *iconw=NULL;

int BarHeight=0;
int BorderWidth=-1, BorderHeight=-1;
struct TextFont *font=NULL;
SHORT charbredd, charhojd, baseline;
/*
UWORD DRI[]={
 3,1, 1,2,
 1,3, 1,0,
 2,8, 65535
};
*/
UWORD DRI[]={ 65535 };

UWORD zoom[]={
  50, 0,
  0, 0
};

unsigned char *title=NULL;

struct TextAttr topazfont = {
  "topaz.font",
  TOPAZ_EIGHTY,
  FS_NORMAL,
  FPF_ROMFONT
};


/**************************** Prompt.c ******************************/
char *lastprompt=NULL;	/* Inneh�llet i senaste promptningen. */

#define SETS(x) (offsetof(SharedStruct, x))
#define SETL(x) (offsetof(BufStruct, Flag) + offsetof(Flags,x))
#define SETB(x) (offsetof(BufStruct, x))
#define SETG(x) (offsetof(DefaultStruct, x))
#define SETW(x) (offsetof(WindowStruct, x))

/* Typ=10 inneb�r att det �r en hook och inte anropas fr�n FPL. */
/* Typ=1 inneb�r att funktionen finns vid uppstart. */
/* Typ=0 inneb�r att funktionen finns EFTER uppstart. */



struct FrexxEdFunction fred[]={
//  {"NewProcess",	SC_NEW_PROCESS,		'I', "S", 0},	// (FPLstr�ng)

/* Dessa funktioner ligger fr�mst, eftersom de enbart �r hookfunktioner.
   De g�r snabbare att hitta dem om de ligger f�rst. */

  {"AllEvents",		DO_EVENT,		'I', "I", 10},  // (command) Function man ska hooka f�r alla h�ndelser. Input �r commandofunktionsv�rdet.
  {"AutoSave",		DO_AUTOSAVE,		'I', "I", 10},	// (EntryID)  AutoSave ska g�ras p� den h�r filen.
  {"BufferKill",	DO_BUFFER_KILL,		'I', "I", 10},	// (BufferID) Exception innan en buffer killas. Kan inte avbrytas.
  {"Exit",		DO_EXIT,		'I', "", 10},	// () FrexxEd ska avslutas (ej avbrytbar).
  {"FPLError",		SC_ERROR,		'I', "ISS", 10},// (rad, program, felmeddelande) Ett FPL fel har intr�ffat.
  {"FileChanged",	DO_FILECHANGED,		'I', "", 10},   // () Buffern som f�rs�kte sparas fanns i nyare format.
  {"GetFile",		DO_GETFILE,		'I', "SS", 10},	// (path, filename) Exception innan 'filename' laddas.  R�tt buffer �r aktiv.
  {"GotFile",		DO_GOTFILE,		'I', "SS", 10},	// (path, filename) Exception efter 'filename' laddas.  R�tt buffer �r aktiv.
  {"HandlerAction",	DO_FILEHANDLER_EXCEPTION,'I', "II", 10},// (BufferID, typ) Exception n�r filehandlern har r�rts. Kan inte avbrytas.  
  {"IconClick",		DO_ICON_CLICK,		'I', "II", 10},	// (deiconified, windowID) Dubbelt ikon-tryck.  Input om FrexxEd �r ikonifierat (0) eller �ppen (icke noll).
  {"IconDrop",		DO_ICON_DROP,		'I', "SI", 10},	// (filnamn, windowID) Icon droppa.  (Default �r insertfile) Ret: retval
  {"MenuSelect",	DO_MENU_SELECTED,	'I', "III", 10},// (titlenum, itemnum, subitemnum) Exception n�r man drar valt ett menyval.  
  {"ResizeWindow",	DO_NEWWINDOWSIZE,	'I', "II", 10},	// (typ, winID) Exception n�r f�nsterstorleken �ndras.  I=0 n�r f�nstret flyttas, I=1 f�r resize, I=2 f�r zoomning.  
  {"SameName",		DO_SAMENAME,		'I', "SI", 10},	// (newfile, oldbufferid) 'newfile' som ska laddas in har samma namn som den buffer man f�r bufferID't till.  Hela filnamnet ges.
  {"SliderDrag",	DO_SLIDER,		'I', "", 10},	// () Exception n�r man drar i slidern. Kan inte avbrytas.  
  {"CloseGadget",	DO_CLOSE_WINDOW,	'I', "", 10},	// () Exception n�r f�nstret st�ngts.

// H�r kommer de vanliga funktionerna.

  {"ARexxRead",		SC_AREXXREAD,		'S', "S", 0},	// (name) Read ARexx variable
  {"ARexxResult",	SC_AREXXRESULT,		'I', "Is", 0},	// (error, string) Skicka tillbaka resultat till ARexx. Error = non-zero skickar RC_ERROR tillbaka till Arexx, 
  {"ARexxSend",		SC_AREXXSEND,		'S', "SSi", 0},	// (port, string, timeout) Send message to ARexx port, timeout in seconds for reply. no timeout means don't wait for reply
  {"ARexxSet",		SC_AREXXSET,		'I', "SS", 0},	// (name, string) Set ARexx variable to string
  {"About",		DO_ABOUT,		'I', NULL, 0},	// ()
  {"Activate",		SC_ACTIVATE,		'I', "iii", 0},	// (BufferID, mode, oldbufferid)	Ploppar upp buffern s� att den syns.  Finns den, s� ploppas inget.  Ret: BufferID.  Mode:  Inget arg s� anv�nds Defaultsetting, 0=replace, 1=half, 2=full, 3=NewWindow.
  {"AssignKey",		DO_ASSIGN_KEY,		'I', "sss", 1},	// (function, keystring, depended setting) AssignKey.  Ret: ret-value.  
  {"BSearch",		SC_BSEARCH,		'I', "BSii", 1},// (array, s�kstr�ng, storlek, flags)	S�k i arrayen efter s�kstr�ngen (flags enligt 'Sort()').  Ret Position i arrayen, eller -1 om den inte hittar.
  {"Backspace",		DO_BACKSPACE,		'I', "i", 0},	// (Antal)		Returna antalet backspace.
  {"BackspaceWord",	DO_BACKSPACE_WORD,	'I', "i", 0},	// (AntalOrd)		Returna antalet backspace.
  {"BlockChange",	SC_BLOCK_CHANGE,	'I', "i", 0},	// (blockid)		Byt aktivt block, casesensitive.  Tom eller ingen str�ng ger "DefaultBlock".  Ret-value.
  {"BlockCopy",		DO_BLOCK_COPY,		'I', "iiiii", 0},	// (blockid, x1, y1, x2, y2) Kopierar markering till blockbuffer alternativt namngivet block (""=current).  Kan �ven ge koordinater.  Ret-value.
  {"BlockCopyAppend",	DO_BLOCK_COPY_APPEND,	'I', "iiiii", 0},	// (blockid, x1, y1, x2, y2) KopAppend markering till blockbuffer alternativt namngivet block (""=current).  Kan �ven ge koordinater.  Ret-value.
  {"BlockCreate",	SC_BLOCK_ALLOC,		'I', "S", 1},	// (blocknamn)		Skapa en ny blockbuffer.  Om den finns h�nder inget.  Returnerar BufferID.  
  {"BlockCut",		DO_BLOCK_CUT,		'I', "iiiii", 0},	// (blockid, x1, y1, x2, y2) Cuttar markering till blockbuffer alternativt namngivet block (""=current).  Kan �ven ge koordinater.  Ret-value.
  {"BlockCutAppend",	DO_BLOCK_CUT_APPEND,	'I', "iiiii", 0},	// (blockid, x1, y1, x2, y2) Cuttar markering till blockbuffer alternativt namngivet block (""=current).  Kan �ven ge koordinater.  Ret-value.
  {"BlockDelete",	DO_BLOCK_DELETE,	'I', "iiiii", 0},	// (blockid, x1, y1, x2, y2) Deletea markering.  Kan �ven ge koordinater.  blockid �r ett dummyv�rde. Ret-value.
  {"BlockMark",		DO_BLOCK_MARK,		'I', "iiiii", 0},	// (on/off/release/ignore, x1, y1, x2, y2)		Togglar blockmarkering.  Inget return.
  {"BlockMarkLine",	DO_BLOCK_MARK_LINE,	'I', "iiiii", 0},	// (on/off/release/ignore, x1, y1, x2, y2)		Togglar blockmarkering.  Inget return.
  {"BlockMarkRect",	DO_BLOCK_MARK_COL,	'I', "iiiii", 0},	// (on/off/release/ignore, x1, y1, x2, y2)		Togglar blockmarkering.  Inget return.
  {"BlockMove",		DO_BLOCK_MOVE,		'I', "i", 0},	// (number of steps)	Flyttar det markerade blocket i sidled (+-).  Om inget input ges sk�ter man skiten med tangenter, max sk�rmbredd. Returnerar Ret-value.
  {"BlockPaste",	DO_BLOCK_PASTE,		'I', "i", 0},	// (blockname)	Inserta ett valt/markerat/default block i pos.  Ret-value.
  {"BlockPasteRect",	DO_BLOCK_PASTE_COLUMN,	'I', "i", 0},	// (blockname)	Inserta ett valt/markerat/default block rektangul�rt i pos.  Ret-value.
  {"BlockSort",		DO_BLOCK_SORT,		'I', "iii", 0},	// (blockname, column, flags)	Sortera valt/markerat/default block p� kolumn.  (Bit 0 i flags ger case insensitive, Bit 1 ger backward) Default 0.  Ret.value.
  {"CConvertString",	SC_CCONVERTSTRING,	'S', "S", 1},	// (str�ng)	Konvertera en str�ng till C-syntax.  Ret C-str�ng.
  {"CenterView",	SC_CENTERSCREEN,	'I', "i", 0},	// (ViewID)	Centera viewn.	Retvalue.
  {"Check",		SC_EXISTS,		'I', "Ss", 0},	// (filnamn) returnerar non-zero om filen finns!
  {"Clean",		SC_CLEAN,		'I', "S", 1},	// (FPl-skript)	Exekvera given skript utan att n�gon hook enablas. Inget returnv�rde.  
  {"Clear",		DO_CLEAR,		'I', "i", 0},	// (BufferID)	T�m anget/aktiv buffer.	Retvalue.
  {"ClipToString",	SC_CLIP2STRING,		'S', "i", 1},	// (unit)	ret: ClipString
  {"CloneWB",		DO_CLONEWB,		'I', "i", 0},	// (winid)	Kopiera Default-pubscreenen till settingarna.	(0=current, -1=default)  Om inget input s� tas current. Retvalue.
  {"ColorAdjust",	DO_COLOR_ADJUST,	'I', "iiii", 0},	// (colno, red, gree, blue)
  {"ColorReset",	DO_COLOR_RESET,		'I', "i", 1},	// ()  Resetta f�rgerna enligt wb. Argumentet �r bitvis, dvs 1=f�rg1, 2=f�rg2, -1=alla f�rger. Default=-1.
  {"ConstructInfo",	SC_CONSTRUCT_INFO,	'I', "SSSSSIIo", 1},// (name, FPLstring, FPLaddition, type, cyclestring, min, max, default)  Skapa en ny Info-variabel. Ret-value.
  {"CopyInfo",		SC_SETCOPY,		'I', "IIOOs>", 0},// (BufferID, CopyToBufferID, mask, nonmask, settings...) Kopierar de �nskade maskningen eller settingarna till en annan buffer.  Samma system som PromptInfo  
  {"CurrentBuffer",	SC_CURRENT_BUF,		'I', "I", 0},	// (BufferID)	Byt current buf.  Om den �r den currenta vid returnet s� blir den synlig.  Ret gamla current buffer eller NULL.
  {"CursorActive",	SC_CURSOR_ACTIVATE,	'I', "i", 0},	// (off/on)	St�ng av cursorn. ret:cursorstatus.  Inget h�nder om inget arg ges.
  {"CursorDown",	DO_CURSOR_DOWN,		'I', "i", 0},	// (AntalSteg)	ret: number of steps relativt senste positionen
  {"CursorLeft",	DO_CURSOR_LEFT,		'I', "i", 0},	// (antal)	ret: number of steps relativt senste positionen
  {"CursorLeftWord",	DO_LEFT_WORD,		'I', "i", 0},	// (antal)	ret: number of steps relativt senste positionen
  {"CursorRight",	DO_CURSOR_RIGHT,	'I', "i", 0},	// (antal)	ret: number of steps relativt senste positionen
  {"CursorRightWord",	DO_RIGHT_WORD,		'I', "i", 0},	// (AntalOrd)	ret: number of steps relativt senste positionen
  {"CursorStack",	SC_CURSOR_STACK,	'I', "i", 0},	// (get/store)	Sparar eller h�mtar cursorpositionen.  Store=-1(default), Get=(+1), allt annat odefinierat.
  {"CursorUp",		DO_CURSOR_UP,		'I', "i", 0},	// (antal)	ret: number of steps relativt senste positionen
  {"Deiconify",		DO_DEICONIFY,		'I', "i", 0},	// (windowID) Deikonifiera FrexxEd.
  {"Delay",		SC_DELAY,		'I', "I", 1},	// (50th of a second) Delay(x), Inget ret.  Kan avbrytas med ESC.
  {"Delete",		DO_DELETE,		'I', "i", 0},	// (AntalTecken) ret: number of steps
  {"DeleteEol",		DO_DELETE_TO_EOL,	'I', NULL, 0},	// () Ret antal tecken.
  {"DeleteInfo",	SC_DELETE_INFO,		'I', "S", 0},	// (settingnamn)	ret: retvalue  Deletear namngiven setting. Kan inte ta bort en default-setting.  Man f�r inte ta bort en setting vars FPL-skript exekveras.
  {"DeleteKey",		DO_DELETE_KEY,		'I', "s", 0},	// (keystring)  Inget ret.
  {"DeleteLine",	DO_DELETE_LINE,		'I', "i", 0},	// (AntalRader)	ret: number of lines
  {"DeleteWord",	DO_DELETE_WORD,		'I', "i", 0},	// (AntalOrd)	ret: number of words
  {"DisplayBeep",	SC_DISPLAYBEEP,		'I', NULL, 0},	// ()  Blinka lilla stj�rm d�r.
  {"DownCase",		DO_BLOCK_TO_LOWER,	'I', "i", 0},	// (blockid)  Downcase block.  Retvalue.
  {"DuplicateEntry",	SC_DUPLICATE_ENTRY,	'I', "i", 0},	// (BufferID) Duplicera entryt (gamla split_view). ret: Nytt BufferID (EntryID).
  {"ExecuteBuffer",	SC_EXECUTE_BUF,		'I', "i", 0},	// (BufferID) Exekvera en buffer.  Retvalue.
  {"ExecuteFile",	SC_EXECUTE_FILE,	'I', "Ss", 1},	// (filnamn, main) Exekvera en fil som FPL-program.  'main' �r en optional main-funktion i filen. Retvalue.
  {"ExecuteLater",	SC_EXECUTE_LATER,	'I', "S", 0},	// (FPL-str�ng) Exekvera str�ngen direkt man kommit tillbaka till IDCMPc.  Retvalue.
  {"ExecuteString",	DO_EXECUTE_STRING,	'I', "Si", 1},	// (FPL-str�ng, bufferid) Exekvera en str�ng.  Retvalue.  SKA LIGGA I COMMAND.c.
  {"FACT",		SC_SETFACT,		'I', "Oo>", 1},	// ([namn], tecken, tags) �ndra FACT:en! ; ret: ret value.
  {"FACTClear",		SC_RESET_FACT,		'I', "is", 1},	// (tecken, namn) �ndra ett FACT-tecken till default.  tecken==256 eller inget input ger alla tecken.   ret: ret value.
  {"FACTConvertString",	SC_FACTCONVERTSTRING,	'S', "S", 0},	// (str�ng)	Konvertera en str�ng till FACT-syntax.  Ret FACT-str�ng.
  {"FACTCreate",	SC_FACT_CREATE,		'I', "S", 1},	// (namn) Konstruera en ny FACT-struktur. ret: ret value.
  {"FACTDelete",	SC_FACT_DELETE,		'I', "S", 1},	// (namn) Ta bort en FACT-struktur. Ret: ret value.
  {"FACTString",	SC_ISSTRING,		'S', "Is", 1},	// (tecken) Returnerar str�ngen.
  {"FaceAdd",           SC_FACEADD,             'I', "IISSs",1},// (faceID, styleID, addstring, flags, endstring) Add a string as a face
  {"FaceGet",           SC_FACEGET,             'I', "Si", 1},  // (face name, mode) create/get faceID with the specified name
  {"FaceRead",          SC_FACEREAD,            'I', "SCNN",1}, // (style name, string style ref, int fg ref, int bg ref)
  {"FaceStyle",         SC_FACESTYLE,           'I', "SSIIi",1}, // (style name, mode, fg, bg, flag) create/get styleID
  {"FindPort",		SC_FINDPORT,		'I', "Si", 1},	// (portnamn, timeout)	Returns non-zero if the specified port exist. If timeout, when wait in seconds for the port!
  {"Fold",		SC_FOLD,		'S', "iii", 1},	// (startrad, slutrad, bufferid) Folda blocket.  Ret: ret-value
  {"FoldDelete",	SC_FOLD_DELETE,		'S', "ii", 1},	// (rad, bufferid) Deletea folden.  Ret: ret-value
  {"FoldHide",		SC_FOLD_HIDE,		'S', "ii", 1},	// (rad, bufferid) G�m folden.  Ret: ret-value
  {"FoldShow",		SC_FOLD_SHOW,		'S', "ii", 1},	// (rad, bufferid) Visa folden.  Ret: ret-value
  {"GetBlock",		SC_GETBLOCK,		'S', "i", 0},	// (blockid)  F� valt/markerat/default block till en str�ng.  Ret str�ngen.
  {"GetBufferID",	SC_GETBUFFERID,		'I', "sii", 0},	// (buffernamn, fileno, viewnumber)  F� buffer id f�r specad/aktiv buffer.  Kollar f�rst med hela pathen, sedan filnamnet.
  {"GetByte",		SC_GETBYTE,		'I', "ii", 0},	// (column, line)  Returnerar byte-positionen f�r motsvarande kolumn (default �r current-col, och current-line).
  {"GetChar",		SC_GETCHAR,		'I', "ii", 0},	// (bytepos, line)  H�mta tecknet fr�n column-line.
  {"GetCursor",		SC_GETCURSOR,		'I', "ii", 0},	// (bytepos, line)  Returnerar cursorpositionen f�r given str�ngposition (default �r current-bytepos, och current-line).
  {"GetDate",		SC_GETDATE,		'S', "ii", 0},	// (BufferID, type)  H�mta datumet i klartext. Returnerar datumet eller en tom str�ng.
  {"GetEntryID",	SC_GETENTRYID,		'I', "sii", 0},	// (buffernamn, fileno, viewnumber)  F� entry id f�r specad/aktiv buffer.  Kollar f�rst med hela pathen, sedan filnamnet.
  {"GetEnv",		SC_GETVAR,		'S', "S", 0},	// (namn)  Returnera en env-variabel fr�n systemet.  Returnerar str�ngen.  ErrNo s�tts till CANT_FIND_SETTING och tom str�ng returnerar om variabeln inte finns.
  {"GetErrNo",		SC_GETERRNO,		'I', NULL, 1},	// ()  H�mta senaste errornummret.
  {"GetFileList",	SC_GETFILELIST,		'I', "SB", 0},	// (pattern, resultat)  F� alla filer som matchar patternet lagrat i variabel.  Returnerar antalet matchningar.
  {"GetKey",		SC_GETKEY,		'S', "i", 0},	// (check)  V�nta p� att ett tangenttryck och returnerar str�ngen. Om check==1 s� returneras en str�ng direkt om det ligger n�got i buffern, annars tom str�ng.  Ret str�ngen.
  {"GetLine",		SC_GETLINE,		'S', "ii", 0},	// (line, BufferID)  H�mta angiven/current rad till en str�ng.  Ret str�ngen.
  {"GetList",		SC_GETLIST,		'I', "SAa>", 0},// (listnamn, resultatarray, extra)  F� en angiven lista i given str�ngarray.  Ret: antal medlemmar.
  {"GetReturnMsg",	SC_GETERRORMSG,		'S', "I", 1},	// (returnv�rde)  F� retvalue i klartext.
  {"GetWindowID",	SC_GETWINDOWID,		'I', "i", 0},	// (ID)  F� f�nster-id f�r specad/aktiv buffer.  Argumentet kan vara f�nster-id eller buffer-id, return-id'et blir is�fall bufferns tillh�rande f�nster
  {"GetWord",		SC_GETWORD,		'S', "ii", 0},	// (rad, string_pos)  Returnera ordet du st�r p�, eller ordet under positionen du anger.  Om raden �r otill�ten anv�nds current rad.  Om columnen �r otill�ten returneras en tom str�ng.  Ordet f�r inte bryta radgr�nsen.  Ret str�ngen.
  {"GotoChange",	DO_LAST_CHANGE,		'I', "i", 0},	// (nummer) Hoppa till senaste/angivna relativa �ndring. ret: vilken �ndring
  {"GotoLine",		DO_GOTO_LINE,		'I', "ii", 0},	// (rad, kolumn) Hoppa till angiven rad och kolumn(string_pos), ret: success(valid position).
  {"Hook",		SC_SETHOOK,		'I', "SSs", 1},	// (func to hook, func to call when it's hooked, setting) Hooka en funktion!
  {"HookClear",		SC_HOOKCLEAR,		'I', "sss", 1},	// (function, hook, setting) Clearar de hooks som matchar parametrarna! RET: antal borttagna hookar!
  {"HookPast",		SC_SETHOOKPAST,		'I', "SSs", 1},	// (func to hook, func to call when it's hooked, setting) Post-Hooka en funktion!
  {"Iconify",		DO_ICONIFY,		'I', "i", 0},	// (windowID) Ikonifiera FrexxEd.
  {"InsertFile",	DO_INSERT_FILE,		'I', "sss", 0},	// (filnamn, windowtitle, promptname) Inserta in en fil, tar fram requester om arg saknas eller om promptname ges. L�s in en fil fr�n nuvarande position. Ret: retval
  {"InverseLine",	SC_INVERSE_LINE,	'I', "iii", 0},	// (rad, l�ngd, kolumn) Invertera texten grafiskt.  Default �r (current rad, radl�ngd, 1)  
  {"Isclose",		SC_ISCLOSE,		'I', "Is", 1},	// (tecken) Returnerar motsvarande tecken, eller -1.
  {"Isfold",		SC_ISFOLD,		'I', "ii", 1},	// (rad, BufferID) Returnerar foldniv�n.  Negativt om den �r synlig. (current rad och buffer �r default). Om rad==0 f�s current.
  {"Islower",		SC_ISLOWER,		'I', "Is", 1},	// (tecken) Returnerar motsvarande tecken, eller -1.
  {"Isnewline",		SC_ISNEWLINE,		'I', "Is", 1},	// (tecken) Returnerar icke noll.
  {"Isopen",		SC_ISOPEN,		'I', "Is", 1},	// (tecken) Returnerar motsvarande tecken, eller -1.
  {"Isspace",		SC_ISSPACE,		'I', "Is", 1},	// (tecken) Returnerar icke noll.
  {"Issymbol",		SC_ISSYMBOL,		'I', "Is", 1},	// (tecken) Returnerar icke noll.
  {"Istab",		SC_ISTAB,		'I', "Is", 1},	// (tecken) Returnerar icke noll.
  {"Isupper",		SC_ISUPPER,		'I', "Is", 1},	// (tecken) Returnerar motsvarande tecken, eller -1.
  {"Isword",		SC_ISWORD,		'I', "Is", 1},	// (tecken) Returnerar icke noll.
  {"KeyPress",		SC_KEYPRESS,		'S', "s", 0},	// (key-str�ng)  Returnerar funktionen som finns p� en tangent.  Om ingen tangent ges, s� tas tangenten interaktivt.
  {"Kill",		DO_QUIT,		'I', "i", 0},	// (bufID) D�da, avsluta nuvarande eller angiven buffer. ret: N�sta entryid som kan anv�ndas som currentbuffer.
  {"Load",		DO_LOAD,		'I', "sss", 0},	// (filnamn, windowtitle, promptname) Ladda in en fil, tar fram requester om arg saknas eller om promptname ges. Om inget argument ges s� bes�rjer FrexxEd f�r att ta fram en requester om man laddar in en fil som redan finns. Ret: retval.  
  {"LoadString",	SC_LOADSTRING,		'S', "S", 1},	// (filnamn) Ladda en fil och returnera som str�ng. ret: str�ng
  {"LogSetting",	SC_LOG_SETTING,		'I', "SSO", 1},	// (settingnamn, typ, defaultv�rde) Anv�nds f�r att vid uppstart lagra defaultv�rden p� de settings som skapas.
  {"MacroRecord",	DO_MACRO_RECORD,	'I', "iss", 0},	// (userinput, name, tangentsekvens) Spela in ett macro.  Om userinput==0 ges en requester, annars utnyttjar han �vrig input direkt eller skapar ett eget namn och l�ter anv�ndaren ge tangentkombinationen genom tangenttryck. Ret:retval.
  {"MatchParen",	DO_MATCH_PAREN,		'I', NULL, 0},	// () Hoppa till matchande parantes ret: ret-val (0 -hoppade annars inte)
  {"MaximizeView",	DO_MAXIMIZE_BUF,	'I', "i", 0},	// (viewno) Maximera nuvarande eller angiven view. Ret: ingenting
  {"MenuAdd",		SC_MENUADD,		'I', "SSssiii", 1},	// (type, string, FPL, key) ret: success
  {"MenuBuild",		SC_MENUBUILD,		'I', NULL, 0},	// () ret: success
  {"MenuClear",		SC_MENUCLEAR,		'I', NULL, 1},	// () ret: success
  {"MenuDelete",	SC_MENUDELETE,		'I', "Iii", 1},	// (titlenum, itemnum, subitemnum) Tar bort ett menuitem (inklusive dess undermenyer). ret: success
  {"MenuRead",		SC_MENUREAD,		'I', "CCCCIii", 1},	// (type, string, FPL, key) ret: success
  {"New",		SC_NEW_BUF,		'I', NULL, 0},	// ()  �ppna en ny buffert.  Returnera BufferID'et f�r den.
  {"NextBuffer",	SC_NEXT_BUF,		'I', "ii", 0},	// (BufferID, typ)
  {"NextEntry",		SC_NEXT_ENTRY,		'I', "ii", 0},	// (BufferID, typ)
  {"NextHidden",	SC_NEXT_HIDDEN_BUF,	'I', "ii", 0},	// (BufferID, typ)
  {"NextView",		SC_NEXT_SHOW_BUF,	'I', "ii", 0},	// (BufferID, typ)
  {"NextWindow",	SC_NEXT_WINDOW,		'I', "i", 0},	// (WindowID)
  {"Output",		SC_OUTPUT,		'I', "S", 0},	// (string)  G�r en Output.  Returnerar inget.
  {"PageDown",		DO_PAGE_DOWN,		'I', "i", 0},	// (antal)	ret: number of steps
  {"PageUp",		DO_PAGE_UP,		'I', "i", 0},	// (antal)	ret: number of steps
  {"PlaceCursor",	DO_MOUSE_LEFT,		'I', "II", 0},	// (x, y) (funktionen ska finnas i Command.c pga hastighet.  Den checkar inputen ordentligt).
  {"PrevBuffer",	SC_PREV_BUF,		'I', "ii", 0},	// (BufferID, typ)
  {"PrevEntry",		SC_PREV_ENTRY,		'I', "ii", 0},	// (BufferID, typ)
  {"PrevHidden",	SC_PREV_HIDDEN_BUF,	'I', "ii", 0},	// (BufferID, typ)
  {"PrevView",		SC_PREV_SHOW_BUF,	'I', "ii", 0},	// (BufferID, typ)
  {"PrevWindow",	SC_PREV_WINDOW,		'I', "i", 0},	// (WindowID)
  {"PrintLine",		SC_PRINTLINE,		'I', "SIi", 0},	// (str�ng, viewrad, BufferID)  Skriv ut en str�ng i viewn.  Inget return.
  {"Prompt",		DO_PROMPT,		'I', NULL, 0},	// ()
  {"PromptBuffer",	SC_PROMPT_BUFFER,	'I', "si", 0},	// (f�nstertitel, typ) Ge en lista �ver alla buffrar av given typ i minnet och returnera BufferID f�r den.  Returnerar 0 om requestern blev cancellerad.
  {"PromptEntry",	SC_PROMPT_ENTRY,	'I', "si", 0},	// (f�nstertitel, typ) Ge en lista �ver alla entrys av given typ i minnet och returnera EntryID f�r den.  Returnerar 0 om requestern blev cancellerad.
  {"PromptFile",	SC_GETFILE,		'O', "ssssa", 0},// (filename, header, mask, requestertyp(ds), filelist)	Prompta efter en fil.  Ge defaultfile, header och mask.  Returnerar filen alternativt antalet filer.  
  {"PromptFont",	SC_PROMPT_FONT,		'S', "si", 0},	// (f�nstertitel, font)  Prompta fonten.  Ge f�nstertitel, font=1 ger en oprop-val, font>=2 ger ett oprop/prop-val.  Returnerar en FrexxEdFontStr�ng.
  {"PromptInfo",	SC_PROMPTINFO,		'I', "Isoos>", 0},// (BufferID, windowtitle, mask, nonmask, settings...) Ta fram settingsf�nstret med den �nskade maskningen eller settingarna.  
  {"PromptInt",		SC_GETINT,		'I', "sis", 0},	// (text, defv�rde, infotext) Prompta efter en int.  Ge header, defaultv�rde och infotext.
  {"PromptString",	SC_GETSTRING,		'S', "sss", 0},	// (defaultstr�ng, header, infotext) Prompta stringen.  Ge default, header och infotext.
  {"QuitAll",		DO_QUIT_ALL,		'I', NULL, 1},	// () Quit all!
  {"Random",		SC_RANDOM,		'I', "", 1},	// () Returnera ett slumptal.
  {"ReadInfo",		SC_READSET,		'O', "Sio", 1},	// (settingnamn, BufferID, extra input) Returnera inneh�llet i n�mnd setting
  {"RedrawScreen",	SC_REDRAWSCREEN,	'I', "i", 0},	// (typ) Rita om screenen.  'typen' finns f�r fram�tkompabilitet.  'typ'=0 ger att allt ritas om (enda m�jliga). Ret:inget  
  {"RemoveView",	DO_REMOVE_BUF,		'I', "i", 0},	// (viewno) Tar bort den aktiva viewn. OBS Ta emot viewno. Ska den finnas kvar?
  {"Rename",		SC_RENAME,		'I', "S", 0},	// (Nytt filnamn) Rename current buffer. Inget return.
  {"Replace",		DO_REPLACE,		'I', "isssi", 0},// (prompt, search, replace, flags, range) Replace:ar str�ngar. Prompt==0->prompt, Prompt==1->noprompt, Prompt==2->replace once, default �r enligt 'PromptReplace'-flaggan.  ret: Ret-value  
  {"ReplaceMatch",	SC_MATCH_SEARCH,	'S', "SSis", 0},// (search, replace, range, flags) Matcha search-str�ngen p� nuvarande position och returnera replacestr�ngen.  Om tom str�ng returneras s� kan felet l�sas med ErrNo().
  {"ReplaceSet",	SC_REPLACESET,		'I', "sss", 0},	// (flags, search, replace)  S�tter searchflaggorna.  Returnerar ret-value.  
  {"Request",		SC_CONFIRM,		'I', "Sss", 0},	// (text, header, answer) Ret: vilken knapp som valdes, enl. reqtools.
  {"RequestWindow",	SC_REQUEST_WINDOW,	'I', "Sa>", 0},	// (windowtitle, taglist) Ret: 0/1 (FALSE/TRUE).  Kasta upp en egen requester.
  {"ResizeView",	DO_RESIZE_BUF,		'I', "ii", 0},	// (storlek, bufferid) �ndra storlek p� current/angiven view. Ret: ny storlek
  {"ReturnStatus",	SC_SETRETURNMSG,	'I', "O", 0},	// (returnmsg) S�tt statusrads-meddelande n�r programmet avslutas Ret:inget
  {"Save",		DO_SAVE,		'I', "ss", 0},	// (filnamn, packmode) Spara buffer.  Ges inget packmode anv�nds bufferns default.  
  {"SaveString",	SC_SAVESTRING,		'I', "SS", 1},	// (filnamn, str�ng) Skriv FPL str�ng i fil. Ret:ret-value
  {"Screenmode",	DO_SET_SCREEN,		'I', "i", 0},	// (winid) Change screenmode of current or chosen window (-1=current, 0=default)  Om inget input s� tas current. Ret: retvalue
  {"ScrollDown",	SC_SCROLL_DOWN,		'I', "I", 0},	// (antal steg) Scrolla sk�rmen utan att flytta cursorn. Ret:relativa antalet steg 
  {"ScrollUp",		SC_SCROLL_UP,		'I', "I", 0},	// (antal steg) Scrolla sk�rmen utan att flytta cursorn. Ret:relativa antalet steg 
  {"Search",		DO_SEARCH,		'I', "ssi", 0},	// (search-str�ng, flags, range) S�k efter str�ngen. ret: Ret-value.  
  {"SearchSet",		SC_SEARCHSET,		'I', "sss", 0},	// (flags, search, replace)  S�tter searchflaggorna.  Returnerar ret-value.  
  {"SetEnv",		SC_SETVAR,		'I', "SS", 0},	// (namn, str�ng)  S�tt en env-variabel i systemet.  Enbart globala env-variabler.  Return non-zero om OK, FALSE om fel.
  {"SetInfo",		SC_SETINFO,		'I', "Io>", 1},	// (BufferID, [[settingnamn, nytt v�rde]...]) om BID==0 - default, BID==-1 - Current, BID==-2 - LocalAll else == specat buffer.    S�tt ett nytt v�rde i en setting. Om inga extra arg s� kommer motsvarande settingf�nster fram.
  {"SetSave",		DO_SET_SAVE,		'I', "si", 1},	// (filnamn, BufferID) Save settings. Ret: retvalue
  {"Sort",		SC_SORT,		'I', "Bii", 1},	// (array, antal f�lt, flags) Sortera en array.  (flags=0 normal, flags&1 case sensitive, flags&2 backward).  Returnerar ett ret-value.
  {"Status",		SC_STATUS,		'I', "isi", 0},	// (EntryID, text, del) Skriv ut text p� status raden direkt. Om inget str�ng ges, s� skrivs defaultraden ut.  Ret: ingenting  
  {"Stcgfn",		SC_STCGFN,		'S', "S", 1},	// (filename) Returnera filnamnet fr�n ett fullt filnamn   
  {"Stcgfp",		SC_STCGFP,		'S', "S", 1},	// (filename) Returnera pathen fr�n ett fullt filnamn   
  {"Stricmp",		SC_STRICMP,		'I', "SSi", 1}, // (text1, text2, len) j�mf�r text1 med text1 case insensitive med optional l�ngd 'len'
  {"StringChangeCase", 	SC_STRING_TO_UPPLOW,	'S', "Ss", 1},	// (str�ng, fact) �ndra case p� str�ngen
  {"StringToBlock",	SC_STRING2BLOCK,	'I', "Si", 1},	// (str�ng, blockid) Kopiera str�ng till block. Ret:RetValue	
  {"StringToBlockAppend",SC_STRING2BLOCKAPPEND,	'I', "Si", 1},	// (str�ng, blockid) Appendar en str�ng till block. Ret:RetValue	
  {"StringToClip",	SC_STRING2CLIP,		'I', "IS", 1},	// (unit, str�ng) Kopiera str�ng till clipboard. Ret:RetVaule   
  {"StringToLower", 	SC_STRING_TO_LOWER,	'S', "Ss", 1},	// (str�ng, fact) Konvertera str�ngen till lowercase
  {"StringToUpper", 	SC_STRING_TO_UPPER,	'S', "Ss", 1},	// (str�ng, fact) Konvertera str�ngen till uppercase
  {"Strmfp",		SC_STRMFP,		'S', "SS", 1},	// (path, filename) Sl� ihop pathen med filnamnet och returnera resultatet.
  {"SwapCase",		DO_BLOCK_TO_UPPLOW,	'I', "i", 0},	// (blockid) Byt case p� block. Ret: retvalue
  {"System",		SC_SYSTEM,		'I', "Sss", 1},	// (command, input, output) Utf�r system kommando. Ret: DOS progress
  {"TimerAdd",		SC_TIMER_START,		'I', "ISIi", 0},// (once/repeat, fpl_str�ng, sekunder, microsekunder) Exekvera FPL-str�ngen om antalet sekunder. Ret: IO-id
  {"TimerDelete",	SC_TIMER_REMOVE,	'I', "I", 0},	// (IO-id) Avbryt en timer.  Ge timer-id'et eller '-1' f�r alla. Ret: alltid OK
  {"Undo",		DO_UNDO,		'I', "i", 0},	// (antal)  Undo:a ett antal �ndringar	ret: number of steps
  {"UndoRestart",	DO_UNDO_RESTART,	'I', "i", 0},	// (antal) B�rja om undo:n, plus ett antal vanliga undos. Ret: antal steg undoade  
  {"UpCase",		DO_BLOCK_TO_UPPER,	'I', "i", 0},	// (blockid) Byt till upper case p� blocket. ret: retval
  {"Visible",		SC_VISIBLE,		'I', "i", 1},	// (off/on(0/1))  S�tter p� av Visible-flaggan.  Om inget argument ges h�nder inget.  Returnerar status p� Visible-flaggan innan funktionen tillkallades.
  {"WindowAlloc",	SC_WINDOW_ALLOC,	'I', "ii", 0},	// (buffer i f�nstret, mallf�nster)  Allokera ett nytt f�nster.  Kan ange vilken buffer som ska synas i det nya f�nstret. Ret: F�nster-ID.  
  {"WindowClose",	SC_WINDOW_CLOSE,	'I', "i", 0},	// (f�nster id) St�ng ett f�nster. Ret:Ret-Value.  
  {"WindowFree",	SC_WINDOW_FREE,		'I', "i", 0},	// (f�nster id) Fria ett f�nster. Ret:Ret-Value.  
  {"WindowOpen",	SC_WINDOW_OPEN,		'I', "i", 0},	// (f�nster id) �ppna ett f�nster. Ret:Ret-Value.  
  {"WindowToFront",	DO_WINDOWFRONT,		'I', "ii", 0},	// (mode, f�nster id)  WindowToFront. Inget return, funkar alltid.  (0=FrontActive, 1=Frontonly, 2=Activeonly).
  {"Yank",		DO_YANK,		'I', "i", 0},	// (antal)  G�r Yank.  Ret:Ret-Value.  
};

const int nofuncs=sizeof(fred)/sizeof(struct FrexxEdFunction);

char *sizetext[5];
char *slidertext[4];
char *windowtext[5];
char *windowpostext[3];
char *blockexisttext[4];
char *expandpathtext[4];
char *saveicontext[4];

#define INPUT_SYSFONT	"string f;if(strlen(f=PromptFont(\"System font\",1)))return(f);"
#define INPUT_REQFONT	"string f;if(strlen(f=PromptFont(\"Request font\",2)))return(f);"
#define INPUT_DIR	"string d=PromptFile(ReadInfo(\"\"),\"Default directory\",\"\",\"d\");if(GetErrNo()>=0)return(d);"
#define INPUT_KEYMAP	"string m=PromptFile(joinstr(\"sys:Devs/KeyMaps/\",ReadInfo(\"\")),\"KeyMap\");if(GetErrNo()>=0)return(m);"

struct Setting defaultsets[]={
  {"appicon",		SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL,-1, 1, SETG(appicon), SEUP_APPICON, NULL, NULL, MS_GLOBAL|MS_IO},						// Ska FrexxEd skapa en appicon GLOBAL/INT
  {"appwindow",		SE_NOTHING,	NULL,	ST_BOOL|ST_WINDOW,-1, 1, SETW(appwindow), SEUP_APPWINDOW, NULL, NULL, MS_LOCAL|MS_SCREEN},					// Ska FrexxEd bli ett appwindow  GLOBAL/INT
  {"arexx_port",	SE_NOTHING,	NULL,	ST_STRING|ST_GLOBAL|ST_HIDDEN|ST_READ|ST_NOSAVE,0, 0, SETG(ARexxPort), NULL, NULL, NULL, MS_GLOBAL|MS_IO|MS_HIDDEN|MS_READ},	// Vilken Arexx-port FrexxEd har  GLOBAL/STRING/READONLY/NOSAVE.
  {"auto_resize",	SE_NOTHING,	NULL,	ST_BOOL|ST_WINDOW,-1, 1, SETW(autoresize), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN},						// Flagga f�r om f�nstret automatiskt ska anpassas till j�mn fontstorlek.  (Default=TRUE) GLOBAL/BOOL
  {"autosave",		SE_NOTHING,	NULL,	ST_BOOL|ST_SHARED,-1, 1, SETS(autosave), NULL, NULL, NULL, MS_LOCAL|MS_IO},							// Autosave enabled. LOKAL/BOOL  
  {"autosave_interval",	SE_NOTHING,	NULL,	ST_NUM|ST_GLOBAL,0, 10000000, SETG(autosaveintervall), NULL, NULL, NULL, MS_GLOBAL|MS_IO},					// autosave intervall GLOBAL/INT.  
  {"autoscroll",	SE_REOPEN,	NULL,	ST_BOOL|ST_WINDOW|ST_SCREEN|ST_HIDDEN|ST_ALL_WINSCREEN,-1, 1, SETW(autoscroll), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},			// Samma som systemets AutoScroll-flagga f�r screens. (Default=TRUE). GLOBAL/BOOL
  {"block_begin_x",	SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ,0, 10000000, SETB(block_begin_x), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_SYSTEM},			// block_begin_x.  Kolumnposition.  ENTRY/INT/READ/NOSAVE
  {"block_begin_y",	SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ,0, 10000000, SETB(block_begin_y), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_SYSTEM},			// block_begin_y.  Rad.  EMTRY/INT/READ/NOSAVE
  {"block_end_x",	SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ,0, 10000000, SETB(block_end_x), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_SYSTEM},				// block_end_x.  Kolumnposition.  ENTRY/INT/READ/NOSAVE
  {"block_end_y",	SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ,0, 10000000, SETB(block_end_y), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_SYSTEM},				// block_end_y.  Rad.  ENTRY/INT/READ/NOSAVE
  {"block_exist",	SE_NOTHING,	(char **)blockexisttext, ST_CYCLE|ST_NOSAVE|ST_READ,0, 3, SETB(block_exists), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_SYSTEM},		// Talar om ifall ett block �r markerat. 0=ej, 1=Markeras efter cursorn, 2=Markerad oberoende av cursorn   ENTRY/INT/READ/NOSAVE
  {"block_id",		SE_NOTHING,	NULL,	ST_GLOBAL|ST_NUM|ST_CALCULATE|ST_NOSAVE|ST_READ,0, 0, SECALC_BLOCK_NAME, NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_SYSTEM|MS_HIDDEN},// Namn p� aktiva blockbufferten. GLOBAL/INT/READ/NOSAVE
  {"block_type",	SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ,0, 0, SETB(blocktype), NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_SYSTEM|MS_HIDDEN},			// Typ p� blockmarkeringen.  1=Normal, 2=Rektangul�rt.   ENTRY/INT/READ/NOSAVE
  {"buffers",		SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ|ST_GLOBAL,0, 0, SETG(Buffers), NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_DISPLAY},			// Antal buffrar i minnet.  GLOBAL/INT/READ/NOSAVE 
  {"byte_position",	SE_UPDATE,	NULL,	ST_NUM|ST_NOSAVE,	0, 0, SETB(string_pos), SEUP_STRINGPOS, NULL, NULL, MS_LOCAL|MS_HIDDEN|MS_DISPLAY},			// string_pos.  ENTRY/INT/NOSAVE  
  {"changes",		SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_HIDDEN,0, 10000000, SETS(changes), SEUP_CHANGES, NULL, NULL, MS_LOCAL|MS_HIDDEN|MS_SYSTEM},	// Antal �ndringar i bufferten.  BUFFERT/INT/NOSAVE
  {"colour",		SE_NOTHING,	NULL,	ST_NUM|ST_WINDOW|ST_SCREEN|ST_HIDDEN|ST_READ|ST_CALCULATE,0, 0, SECALC_COLOUR, NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN|MS_READ},	// L�s f�rg fr�n sk�rmen.  Extra argumentet anger vilket f�rgregister som ska l�sas. Resultat �r FrexxEd-RGB32-style.  -1 om det blev n�got fel (som ingen sk�rm) GLOBAL/INT
  {"colour0",		SE_RETHINK,	NULL,	ST_NUM|ST_WINDOW|ST_SCREEN|ST_HIDDEN|ST_ALL_WINSCREEN,0, 0, SETW(color0), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},			// F�rg0 GLOBAL/INT
  {"colour1",		SE_RETHINK,	NULL,	ST_NUM|ST_WINDOW|ST_SCREEN|ST_HIDDEN|ST_ALL_WINSCREEN,0, 0, SETW(color1), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},			// F�rg1 GLOBAL/INT
  {"colour2",		SE_RETHINK,	NULL,	ST_NUM|ST_WINDOW|ST_SCREEN|ST_HIDDEN|ST_ALL_WINSCREEN,0, 0, SETW(color2), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},			// F�rg2 GLOBAL/INT
  {"colour3",		SE_RETHINK,	NULL,	ST_NUM|ST_WINDOW|ST_SCREEN|ST_HIDDEN|ST_ALL_WINSCREEN,0, 0, SETW(color3), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},			// F�rg3 GLOBAL/INT
  {"column",		SE_UPDATE,	NULL,	ST_NUM|ST_CALCULATE|ST_NOSAVE|ST_HIDDEN,0, 0, SECALC_COLUMN, SEUP_CURSORMOVE, NULL, NULL, MS_LOCAL|MS_HIDDEN|MS_DISPLAY},	// Vilken kolumn man st�r i.  ENTRY/INT/NOSAVE   
  {"comment",		SE_NOTHING,	NULL,	ST_STRING|ST_SHARED,0, 80, SETS(filecomment), NULL, NULL, NULL, MS_LOCAL|MS_IO},						// Kommentar p� buffern.  BUFFER/STRING
  {"comment_begin",	SE_NOTHING,	NULL,	ST_STRING|ST_SHARED,0, 0, SETS(comment_begin), NULL, NULL, NULL, MS_LOCAL|MS_IO},						// Kommentar som anv�nds vid foldmarkering.  BUFFER/STRING
  {"comment_end",	SE_NOTHING,	NULL,	ST_STRING|ST_SHARED,0, 0, SETS(comment_end), NULL, NULL, NULL, MS_LOCAL|MS_IO},							// Kommentar som anv�nds vid foldmarkering.  BUFFER/STRING
  {"counter",		SE_NOTHING,	NULL,	ST_NUM|ST_GLOBAL|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 0, SETG(commandcount), NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_SYSTEM|MS_HIDDEN},// R�knare som r�knas upp i IDCMP.c inf�r varje anrop till Command().  GLOBAL/INT/READ/NOSAVE
  {"current_screen",	SE_NOTHING,	NULL,	ST_WINDOW|ST_STRING|ST_READ|ST_NOSAVE,	0, 0, SETW(FrexxScreenName), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_SCREEN},		// Vilken screen �r FrexxEd �ppnad p�.  GLOBAL/STRING/READ/NOSAVE.  
  {"cursor_x",		SE_UPDATE,	NULL,	ST_NUM|ST_NOSAVE|ST_HIDDEN,0, 10000000, SETB(cursor_x), SEUP_CURSOR_X, NULL, NULL, MS_LOCAL|MS_HIDDEN|MS_DISPLAY},		// cursor_x.  Var st�r cursorn relativt viewns v�nstra kant.  ENTRY/INT/NOSAVE   
  {"cursor_y",		SE_UPDATE,	NULL,	ST_NUM|ST_NOSAVE|ST_HIDDEN,0, 10000000, SETB(cursor_y), SEUP_CURSORMOVE, NULL, NULL, MS_LOCAL|MS_HIDDEN|MS_DISPLAY},		// cursor_y.  Var st�r cursorn relativt viewns �vre kant.  ENTRY/INT/NOSAVE   
  {"default_file",	SE_NOTHING,	NULL,	ST_STRING|ST_GLOBAL|ST_NOSAVE|ST_READ, 0, 0, SETG(defaultfile), NULL, NULL, NULL, MS_GLOBAL|MS_IO|MS_READ},			// defaultfile.  Bin�r defaultfile som ska laddas in vid uppstart. GLOBAL/STRING.  
  {"directory",		SE_NOTHING,	NULL,	ST_STRING|ST_GLOBAL|ST_NOSAVE, 0, 0, SETG(directory), SEUP_DIRECTORY, NULL, INPUT_DIR, MS_GLOBAL|MS_IO},					// Default directory. GLOBAL/INT
  {"disk_name",		SE_NOTHING,	NULL,	ST_STRING|ST_GLOBAL|ST_HIDDEN|ST_READ|ST_NOSAVE,0, 0, SETG(diskname), NULL, NULL, NULL, MS_GLOBAL|MS_IO|MS_HIDDEN|MS_READ},	// FrexxEd's filehandler disk name  GLOBAL/STRING/READONLY/NOSAVE.
  {"display_id",	SE_REOPEN,	NULL,	ST_NUM|ST_WINDOW|ST_SCREEN|ST_HIDDEN|ST_ALL_WINSCREEN,0, 0, SETW(DisplayID), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},			// display_id.  Likst�llt med Intuitions display_id som anv�nds n�r man �ppnar en screen.  Den finns framf�rallt f�r att den ska sparas i defaultfilen.  GLOBAL/INT
  {"ds_Days",		SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_HIDDEN, 0, 0, SETS(date)+offsetof(struct DateStamp,ds_Days), NULL, NULL, NULL, MS_LOCAL|MS_IO|MS_HIDDEN},			// Bufferdatum.  Antal dagar/sekunder/ticks sedan 1 Jan 1978.  BUFFER/INT/READ/NOSAVE
  {"ds_Minute",		SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_HIDDEN, 0, 0, SETS(date)+offsetof(struct DateStamp,ds_Minute), NULL, NULL, NULL, MS_LOCAL|MS_IO|MS_HIDDEN},			//  - "" -
  {"ds_Tick",		SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_HIDDEN, 0, 0, SETS(date)+offsetof(struct DateStamp,ds_Tick), NULL, NULL, NULL, MS_LOCAL|MS_IO|MS_HIDDEN},			//  - "" -
  {"entries",		SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ|ST_GLOBAL|ST_HIDDEN,0, 0, SETG(Entries), NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_SYSTEM|MS_HIDDEN},	// Antal entrys i minnet.  GLOBAL/INT/READ/NOSAVE 
  {"expand_path",	SE_NOTHING,	(char **)&expandpathtext,	ST_CYCLE|ST_GLOBAL,0, 2, SETG(full_path), NULL, NULL, NULL, MS_GLOBAL|MS_IO},				// Hela pathen ska tas ut n�r man laddar in en fil.  GLOBAL/BOOL
  {"face",		SE_UPDATE,	NULL,	ST_STRING|ST_SHARED,0, 0, SETS(face_name), SEUP_FACE_CHANGE, NULL, NULL, MS_LOCAL|MS_DISPLAY},					// Vilken FACT som ska anv�ndas f�r bufferten.  LOCAL/STRING/NOSAVE/SCREEN
  {"fact",		SE_UPDATE,	NULL,	ST_STRING,0, 0, SETB(fact_name), SEUP_FACT_CHANGE, NULL, NULL, MS_LOCAL|MS_DISPLAY},						// Vilken FACT som ska anv�ndas f�r bufferten.  LOCAL/STRING/NOSAVE/SCREEN
#ifdef USE_FASTSCROLL
  {"fast_scroll",	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL,0, 0, SETG(fast_scroll), NULL, NULL, NULL, MS_GLOBAL|MS_SCREEN},						// Snabbare utskrift och scrollning om man k�r p� en screen.  GLOBAL/BOOL/SCREEN
#endif
  {"file_name",		SE_NOTHING,	NULL,	ST_STRING|ST_SHARED|ST_NOSAVE|ST_READ,0, 0, SETS(filnamn), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_IO},				// filnamn.  BUFFER/STRING/READ/NOSAVE
  {"file_number",	SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_READ,0, 10000000, SETS(name_number), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_IO},			// Nummer p� buffern om det finns flera buffrar med samma filnamn.  BUFFER/INT/READ/NOSAVE
  {"file_path",		SE_NOTHING,	NULL,	ST_STRING|ST_SHARED|ST_NOSAVE|ST_READ,0, 0, SETS(path), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_IO},				// filpath.  BUFFER/STRING/READ/NOSAVE
  {"filehandler",	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL,0, 0, SETG(filehandler), SEUP_FILEHANDLER, NULL, NULL, MS_GLOBAL|MS_IO},					// filhandler.  GLOBAL/BOOL
  {"fold_save",		SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL,	0, 1, SETG(fold_save), NULL, NULL, NULL, MS_GLOBAL|MS_IO},						// Skall FrexxEd ta h�nsyn till fold vid load/save.  Default TRUE.
  {"fold_width",	SE_UPDATEALL|SE_ALL,NULL,ST_NUM|ST_GLOBAL,	0, 8, SETG(fold_margin), SEUP_LINECOUNT, NULL, NULL, MS_GLOBAL|MS_DISPLAY},					// Bredd f�r f�ltet f�r foldmarkeringar (0-8)  Default 0.
  {"fpl_debug",		SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL|ST_NOSAVE, -1, 1, SETG(FPLdebug), SEUP_FPLDEBUG, NULL, NULL, MS_GLOBAL|MS_SYSTEM},				// FPL-debug on. GLOBAL/INT
  {"fpl_path",		SE_NOTHING,	NULL,	ST_STRING|ST_GLOBAL, 0, 0, SETG(FPLdirectory), NULL, NULL, NULL, MS_GLOBAL|MS_IO},						// Default directory/directories f�r FPL filer. GLOBAL/STRING
  {"fragmentation",	SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 10000000, SETS(fragment), NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_HIDDEN|MS_SYSTEM},	// Antal fragmentering av bitar i buffern.  BUFFER/INT/READ/NOSAVE
  {"fragmentation_size",SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 10000000, SETS(fragmentlen), NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_HIDDEN|MS_SYSTEM},	// Minnesf�rlusten av ovanst�ende fragmentering.   BUFFER/INT/READ/NOSAVE
  {"full_file_name",	SE_NOTHING,	NULL,	ST_STRING|ST_CALCULATE|ST_NOSAVE,0, 0, SECALC_FULLNAME, NULL, NULL, NULL, MS_LOCAL|MS_IO},					// Hela filnamnet inklusive pathen.  BUFFER/STRING/NOSAVE.  
  {"iconify",		SE_NOTHING,	NULL,	ST_BOOL|ST_READ|ST_WINDOW|ST_HIDDEN,	-1, 1, SETW(iconify), NULL, NULL, NULL, MS_LOCAL|MS_DISPLAY|MS_HIDDEN},			// 0=normal 1=iconified.  GLOBAL/BOOL/HIDDEN
  {"insert_mode",	SE_NOTHING,	NULL,	ST_BOOL,	-1, 1, SETL(insert_mode), NULL, NULL, NULL, MS_LOCAL|MS_DISPLAY},						// Insert mode p�/av.  Default=p�.  ENTRY/BOOL
  {"keymap",		SE_NOTHING,	NULL,	ST_STRING|ST_GLOBAL,0, 0, SETG(KeyMap), SEUP_NEWKEYMAP, NULL, INPUT_KEYMAP, MS_GLOBAL|MS_IO},					// Keymap.  Tom str�ng ger systemdefaulten.  GLOBAL/STRING  
  {"language",		SE_NOTHING,	NULL,	ST_STRING|ST_READ|ST_GLOBAL, 0, 0, SETG(language), NULL, NULL, NULL, MS_GLOBAL|MS_SYSTEM|MS_READ},				// Spr�k i klartext ex: "english", "svenska" etc
  {"line",		SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 0, SETB(curr_line), NULL, NULL, NULL, MS_LOCAL|MS_HIDDEN|MS_DISPLAY|MS_READ},		// Vilken rad cursorn befinner sig p�.  ENTRY/INT/NOSAVE   
  {"line_counter",	SE_UPDATE,	NULL,	ST_BOOL,	-1, 1, SETL(l_c), SEUP_LINECOUNT, NULL, NULL, MS_LOCAL|MS_DISPLAY},						// Radnummrering p�/av.  Default=FALSE ENTRY/BOOL
  {"line_counter_width", SE_UPDATEALL|SE_ALL,NULL,ST_NUM|ST_GLOBAL,2, 8, SETG(l_c_len_store), SEUP_LINECOUNT, NULL, NULL, MS_GLOBAL|MS_DISPLAY},					// Bredd p� f�ltet f�r radnummren. Default=5 GLOBAL/INT
  {"line_length",	SE_NOTHING,	NULL,	ST_NUM|ST_CALCULATE|ST_NOSAVE|ST_READ,0, 0, SECALC_LINELEN, NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_DISPLAY},			// L�ngden i bytes p� den raden cursorn finns p�.  ENTRY/INT/READ/NOSAVE
  {"lines",		SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_READ,0, 10000000, SETS(line), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_DISPLAY},			// Antal rader i buffern.  BUFFER/INT/READ/NOSAVE
  {"macro_key",		SE_NOTHING,	NULL,	ST_STRING|ST_SHARED|ST_NOSAVE|ST_HIDDEN,0, 0, SETS(macrokey), NULL, NULL, NULL, MS_LOCAL|MS_SYSTEM|MS_HIDDEN},					// Vilken tangent som �r assignat om bufferten �r ett skapat macro.  BUFFER/STRING/READ/NOSAVE  
  {"macro_on",		SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL|ST_READ|ST_NOSAVE|ST_CALCULATE,-1, 1, SECALC_MACRO, NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_SYSTEM},		// Bools setting f�r om macroinspelning �r p�.  GLOBAL/BOOL/READ/NOSAVE  
  {"marg_left",		SE_REINIT|SE_ALL,NULL,	ST_NUM,0, 100000, SETB(leftmarg), NULL, NULL, NULL, MS_GLOBAL|MS_DISPLAY},							// Marginalerna.  Default=8.  LOCAL/INT		
  {"marg_lower",	SE_REINIT|SE_ALL,NULL,	ST_NUM,0, 100000, SETB(lowermarg), NULL, NULL, NULL, MS_GLOBAL|MS_DISPLAY},							// Marginalerna.  Default=4.  LOCAL/INT
  {"marg_right",	SE_REINIT|SE_ALL,NULL,	ST_NUM,0, 100000, SETB(rightmarg), NULL, NULL, NULL, MS_GLOBAL|MS_DISPLAY},							// Marginalerna.  Default=8.  LOCAL/INT
  {"marg_upper",	SE_REINIT|SE_ALL,NULL,	ST_NUM,0, 100000, SETB(uppermarg), NULL, NULL, NULL, MS_GLOBAL|MS_DISPLAY},							// Marginalerna.  Default=4.  LOCAL/INT
  {"mouse_x",		SE_NOTHING,	NULL,	ST_NUM|ST_CALCULATE|ST_GLOBAL|ST_READ|ST_NOSAVE,0, 0, SECALC_MOUSE_X, NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_SYSTEM},		// Var musen pekades av anv�ndaren.  Om han inte tryckte p� n�gon musenknapp �r resultatet -1.  GLOBAL/INT/READ/NOSAVE
  {"mouse_y",		SE_NOTHING,	NULL,	ST_NUM|ST_CALCULATE|ST_GLOBAL|ST_READ|ST_NOSAVE,0, 0, SECALC_MOUSE_Y, NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_SYSTEM},		// Var musen pekades av anv�ndaren.  Om han inte tryckte p� n�gon musenknapp �r resultatet -1.  GLOBAL/INT/READ/NOSAVE
  {"move_screen",	SE_REINIT,	NULL,	ST_NUM|ST_WINDOW,1, 100000, SETW(move_screen), NULL, NULL, NULL, MS_LOCAL|MS_DISPLAY},						// Hur mycket ska sk�rmen flyttas i sedled om marginalen �verskrids.  Default=8.  GLOBAL/INT
  {"overscan",     	SE_REOPEN,	NULL,	ST_NUM|ST_WINDOW|ST_SCREEN|ST_HIDDEN, 0, 0, SETW(OverscanType), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},		// Ovarscantyp enligt Intuition.  GLOBAL/INT
  {"pack_type",         SE_NOTHING,	NULL,   ST_STRING|ST_SHARED, 0, 5, SETS(pack_type), NULL, NULL, NULL, MS_LOCAL|MS_IO},                 					// Packer-typ-str�ng. Default="", men �ndras efter filen som laddats.  BUFFER/STRING
  {"page_length",	SE_UPDATEALL|SE_ALL,NULL,ST_NUM|ST_GLOBAL,0, 10000000, SETG(page_length), NULL, NULL, NULL, MS_GLOBAL|MS_DISPLAY},					// pagelength f�r statusinformation.  Default 66. 
  {"page_overhead",	SE_NOTHING,	NULL,	ST_NUM|ST_GLOBAL,0, 10000000, SETG(page_overhead), NULL, NULL, NULL, MS_GLOBAL|MS_DISPLAY},					// anger antal rader som ska sparas vid page up/down.  Default 0. 
  {"password",		SE_NOTHING,	NULL,	ST_STRING|ST_SHARED|ST_NOSAVE, 0, 80,SETS(password), NULL, NULL, NULL, MS_LOCAL|MS_IO},						// password used for encryption/decryption
  {"pen_block",		SE_UPDATEALL,	NULL,	ST_NUM|ST_WINDOW,0, 0, SETW(block_pen), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN},					// penna f�r blockmarkeringen. 
  {"pen_cursor",	SE_UPDATE,	NULL,	ST_NUM|ST_WINDOW,0, 0, SETW(cursor_pen), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN},					// penna f�r cursorn. 
  {"pen_info",		SE_UPDATEALL,	NULL,	ST_NUM|ST_WINDOW,-1, 10000000, SETW(colour_status_info), SEUP_PENS_CHANGED, NULL, NULL, MS_LOCAL|MS_SCREEN},			// penna f�r statusraden.  (-1) inneb�r att namnf�rgen anv�nds. 
  {"popup_view",	SE_NOTHING,	(char **)&sizetext,	ST_CYCLE|ST_GLOBAL,0, 3, SETG(full_size_buf), NULL, NULL, NULL, MS_GLOBAL|MS_DISPLAY},				// Hur ska en nyaktiverad buffer visas default p� sk�rmen. 0=replace, 1=half, 2=full, 3=newwindow  (Default=1)  GLOBAL/INT
  {"protection",        SE_NOTHING|SE_PROTECTION,NULL,	ST_STRING|ST_SHARED, 0, 8, SETS(protection), NULL, NULL, NULL, MS_LOCAL|MS_IO},						// Filprotectionstr�ngen.  Default="rwed"  BUFFER/STRING
  {"protectionbits",	SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_HIDDEN, 0, 0, SETS(fileprotection), SEUP_PROTECTION_BITS, NULL, NULL, MS_LOCAL|MS_IO|MS_HIDDEN},	// Filprotectionbitarna.  Default="rwed"  BUFFER/INT/NOSAVE/HIDDEN
  {"public_screen",	SE_REOPEN,	NULL,	ST_WINDOW|ST_STRING,	0, 0, SETW(PubScreen), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN},						// Vilken PubScreen som FrexxEd ska �ppnas p� samt h�mta information genom copywb-flaggan.  Default="WorkBench".  GLOBAL/STRING 
  {"real_screen_height",SE_NOTHING,	NULL,	ST_NUM|ST_WINDOW|ST_READ|ST_NOSAVE|ST_HIDDEN, 100, 100000, SETW(real_screen_height), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_READ|MS_HIDDEN},	// ScreenHeight av FrexxEds sk�rm.  GLOBAL/INT/READ  
  {"real_screen_width", SE_NOTHING,	NULL,	ST_NUM|ST_WINDOW|ST_READ|ST_NOSAVE|ST_HIDDEN, 320, 100000, SETW(real_screen_width), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_READ|MS_HIDDEN},	// ScreenWidth av FrexxEds sk�rm.  GLOBAL/INT/READ   
  {"real_window_height",SE_NOTHING,	NULL,	ST_NUM|ST_WINDOW|ST_READ|ST_NOSAVE|ST_HIDDEN, 40, 100000, SETW(real_window_height), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_READ|MS_HIDDEN},	// F�nsterh�jd.   GLOBAL/INT/READ  
  {"real_window_width", SE_NOTHING,	NULL,	ST_NUM|ST_WINDOW|ST_READ|ST_NOSAVE|ST_HIDDEN, 60, 100000, SETW(real_window_width), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_READ|MS_HIDDEN},	// F�nsterbredd.  GLOBAL/INT/READ  
  {"real_window_xpos",	SE_NOTHING,	NULL,	ST_NUM|ST_WINDOW|ST_READ|ST_NOSAVE|ST_HIDDEN, 0, 100000, SETW(real_window_xpos), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_READ|MS_HIDDEN},		// F�nsterxposition.  GLOBAL/INT/READ  
  {"real_window_ypos",	SE_NOTHING,	NULL,	ST_NUM|ST_WINDOW|ST_READ|ST_NOSAVE|ST_HIDDEN, 0, 100000, SETW(real_window_ypos), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_READ|MS_HIDDEN},		// F�nsterypostition.  GLOBAL/INT/READ  
  {"replace_buffer",	SE_NOTHING,	NULL,	ST_STRING|ST_NOSAVE|ST_READ|ST_GLOBAL|ST_CALCULATE,0, 0, SECALC_REPLACEBUFFER, NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_SYSTEM|MS_HIDDEN},	// Vad som finns i replace bufferten.  GLOBAL/STRING/READ/NOSAVE  
  {"req_ret_mode",	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL|ST_HIDDEN, -1, 1, SETG(req_ret_mode), NULL, NULL, NULL, MS_GLOBAL|MS_SYSTEM|MS_HIDDEN},					// Return ska g� mellan searchf�lten.  Default=FALSE.  GLOBAL/BOOL/HIDDEN/NOSAVE
  {"request_font",	SE_FONTCHANGED|SE_ALL,NULL,ST_STRING|ST_GLOBAL,	0, 0, SETG(RequestFont), NULL, NULL, INPUT_REQFONT, MS_GLOBAL|MS_SCREEN},				// RequestFont.  Default="Topaz 8".  GLOBAL/STRING
  {"request_pattern",	SE_NOTHING,	NULL,	ST_STRING|ST_GLOBAL|ST_NOSAVE|ST_HIDDEN,0, 0, SETG(file_pattern), NULL, NULL, NULL, MS_GLOBAL|MS_IO|MS_HIDDEN},			// FileRequester pattern.  Default="".  GLOBAL/STRING/HIDDEN/NOSAVE
  {"right_mbutton",	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL, -1, 1, SETG(RMB), NULL, NULL, NULL, MS_GLOBAL|MS_IO},							// RightMouseButton f�r anv�ndas till annat �n bara menyerna.  Default=FALSE.  GLOBAL/BOOL
  {"rwed_sensitive",	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL, -1, 1, SETG(rwed_sensitive), NULL, NULL, NULL, MS_GLOBAL|MS_IO},						// FrexxEd ska ta h�nsyn till protectionflaggorna p� filer han laddar in.  Default=TRUE.  GLOBAL/BOOL
  {"safe_save",		SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL, -1, 1, SETG(safesave), NULL, NULL, NULL, MS_GLOBAL|MS_IO},							// SafeSave==TRUE g�r att filen sparas i en tempfil innan den gamla deleteas och tempfilen renameas.  Default=TRUE.  GLOBAL/BOOL   
  {"save_icon",		SE_NOTHING,	(char **)&saveicontext,	ST_CYCLE|ST_SHARED, 0, 2, SETS(SaveInfo), NULL, NULL, NULL, MS_LOCAL|MS_IO},					// Spara en infofil till texten.  Default=TRUE.  LOCAL/BOOL
  {"screen_depth",      SE_REOPEN,	NULL,	ST_NUM|ST_WINDOW|ST_SCREEN|ST_HIDDEN|ST_ALL_WINSCREEN, 1, 65535, SETW(screen_depth), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},				// ScreenDepth av FrexxEds sk�rm. Default=0 (2 under v39). (0 ger wb) GLOBAL/INT
  {"screen_height",     SE_REOPEN,	NULL,	ST_NUM|ST_WINDOW|ST_SCREEN|ST_HIDDEN|ST_ALL_WINSCREEN, 100, 100000, SETW(screen_height), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},				// ScreenHeight av FrexxEds sk�rm. Default=256.  GLOBAL/INT
  {"screen_width",      SE_REOPEN,	NULL,	ST_NUM|ST_WINDOW|ST_SCREEN|ST_HIDDEN|ST_ALL_WINSCREEN, 320, 100000, SETW(screen_width), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},				// ScreenWidth av FrexxEds sk�rm. Default=640.  GLOBAL/INT
  {"search_block",  	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL|ST_NOSAVE|ST_HIDDEN, -1, 1, SETG(search_block), SEUP_SEARCH_FLAG, NULL, NULL, MS_GLOBAL|MS_HIDDEN|MS_SYSTEM},	// Searchflagga.  GLOBAL/BOOL  
  {"search_buffer",	SE_NOTHING,	NULL,	ST_STRING|ST_NOSAVE|ST_READ|ST_GLOBAL|ST_CALCULATE,0, 0, SECALC_SEARCHBUFFER, NULL, NULL, NULL, MS_GLOBAL|MS_READ|MS_SYSTEM},	// Vad som finns i search bufferten.  Om en tredje siffra ges som argument, s� h�mtas str�ngen fr�n motsvarande plats i search-historyn. GLOBAL/STRING/READ/NOSAVE  
  {"search_case",  	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL|ST_NOSAVE|ST_HIDDEN, -1, 1, SETG(search_case), SEUP_SEARCH_FLAG, NULL, NULL, MS_GLOBAL|MS_HIDDEN|MS_SYSTEM},	// Searchflagga.  GLOBAL/BOOL  
  {"search_flags",  	SE_NOTHING,	NULL,	ST_NUM|ST_GLOBAL|ST_NOSAVE|ST_HIDDEN, 0, 0, SETG(search_flags), SEUP_SEARCH_ALL_FLAG, NULL, NULL, MS_GLOBAL|MS_HIDDEN|MS_SYSTEM},// Alla searchflaggor.  GLOBAL/BOOL
  {"search_fold",  	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL|ST_NOSAVE|ST_HIDDEN, -1, 1, SETG(search_fold), SEUP_SEARCH_FLAG, NULL, NULL, MS_GLOBAL|MS_HIDDEN|MS_SYSTEM},// Searchflagga.  GLOBAL/BOOL  
  {"search_forward",  	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL|ST_NOSAVE|ST_HIDDEN, -1, 1, SETG(search_forward), SEUP_SEARCH_FLAG, NULL, NULL, MS_GLOBAL|MS_HIDDEN|MS_SYSTEM},// Searchflagga.  GLOBAL/BOOL  
  {"search_limit",  	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL|ST_NOSAVE|ST_HIDDEN, -1, 1, SETG(search_limit), SEUP_SEARCH_FLAG, NULL, NULL, MS_GLOBAL|MS_HIDDEN|MS_SYSTEM},	// Searchflagga.  GLOBAL/BOOL  
  {"search_prompt",  	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL|ST_NOSAVE|ST_HIDDEN, -1, 1, SETG(search_prompt), SEUP_SEARCH_FLAG, NULL, NULL, MS_GLOBAL|MS_HIDDEN|MS_SYSTEM},	// Searchflagga.  GLOBAL/BOOL  
  {"search_wildcard",  	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL|ST_NOSAVE|ST_HIDDEN, -1, 1, SETG(search_wildcard), SEUP_SEARCH_FLAG, NULL, NULL, MS_GLOBAL|MS_HIDDEN|MS_SYSTEM},// Searchflagga.  GLOBAL/BOOL  
  {"search_word",  	SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL|ST_NOSAVE|ST_HIDDEN, -1, 1, SETG(search_word), SEUP_SEARCH_FLAG, NULL, NULL, MS_GLOBAL|MS_HIDDEN|MS_SYSTEM},	// Searchflagga.  GLOBAL/BOOL  
  {"shared",		SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ|ST_SHARED,0, 0, SETS(shared), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_SYSTEM},				// Antal entrys som delar p� samma buffert.  ENTRY/INT/READ/NOSAVE  
  {"shared_number",	SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 10000000, SETB(view_number), NULL, NULL, NULL, MS_LOCAL|MS_DISPLAY|MS_READ|MS_HIDDEN},	// Vilket nummer entryt har bland de entryn som delar p� samma buffer.  ENTRY/INT/READ/NOSAVE
  {"show_path",	  	SE_UPDATE|SE_ALL,NULL,	ST_BOOL|ST_GLOBAL, -1, 1, SETG(showpath), NULL, NULL, NULL, MS_GLOBAL|MS_DISPLAY},						// Visa filpathen i statusraden.  Default=FALSE.  GLOBAL/BOOL  {"size",		SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_READ,0, 0, SETS(size), NULL, NULL, NULL, MS_LOCAL|MS_READ},					// Size p� buffern.  BUFFER/INT/READ/NOSAVE
  {"size",		SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_READ,0,0, SETS(size), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_SYSTEM},				// Size p� buffern.  BUFFER/INT/READ/NOSAVE
  {"slider",		SE_REINIT,	(char **)&slidertext,	ST_CYCLE|ST_WINDOW,0, 2, SETW(slider), SEUP_SLIDERCHANGE, NULL, NULL, MS_LOCAL|MS_SCREEN},					// Position p� slidern. 0=av 1=Right 2=Left.  Default=Right.  Kan INTE l�ggas till v�nster om FrexxEd �ppnas som ett f�nster.  GLOBAL/INT
  {"startup_file",	SE_NOTHING,	NULL,	ST_STRING|ST_GLOBAL,	0, 0, SETG(StartupFile), NULL, NULL, NULL, MS_GLOBAL|MS_IO},						// Startup FPL-fil.  Default="FrexxEd.FPL".  GLOBAL/STRING
  {"status_line",	SE_UPDATEALL|SE_ALL,NULL,ST_STRING|ST_GLOBAL,	0, 0, SETG(status_line), NULL, NULL, NULL, MS_GLOBAL|MS_DISPLAY},					// Hur status-raden ska se ut. GLOBAL/STRING
  {"system_font",	SE_FONTCHANGED|SE_ALL,NULL,ST_STRING|ST_GLOBAL,	0, 0, SETG(SystemFont), NULL, NULL, INPUT_SYSFONT, MS_GLOBAL|MS_SCREEN},				// SystemFont.  Default="Topaz 8", GLOBAL/STRING
  {"tab_size",		SE_UPDATE,	NULL,	ST_NUM,		1, 999, SETB(tabsize), NULL, NULL, NULL, MS_LOCAL|MS_DISPLAY},							// Tabsize.  Default=8.  ENTRY/INT
  {"taskpri",		SE_NOTHING,	NULL,	ST_NUM|ST_GLOBAL,-25, 25, SETG(taskpri), SEUP_PRIO, NULL, NULL, MS_GLOBAL|MS_SYSTEM},						// Taskpri p� FrexxEds task.  Default=1.  GLOBAL/INT
  {"top_offset",	SE_UPDATEALL,	NULL,	ST_NUM|ST_NOSAVE|ST_HIDDEN,0, 10000000, SETB(top_offset), SEUP_RESIZE, NULL, NULL, MS_LOCAL|MS_HIDDEN|MS_DISPLAY},		// Vilken cursor-rad som viewn b�rjar p�.  ENTRY/INT/NOSAVE   
  {"topline",		SE_UPDATE,	NULL,	ST_NUM|ST_NOSAVE|ST_HIDDEN,0, 10000000, SETB(curr_topline), SEUP_CURSORMOVE, NULL, NULL, MS_LOCAL|MS_HIDDEN|MS_DISPLAY},	// Vilken som �r den �versta raden i viewn.  ENTRY/INT/NOSAVE   
  {"type",		SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_HIDDEN,0, 10000000, SETS(type), NULL, NULL, NULL, MS_LOCAL|MS_SYSTEM|MS_HIDDEN},					// Vilken typ buffern �r. 0=fil, 1=macro.  BUFFER/INT  
  {"undo",              SE_NOTHING,	NULL,	ST_BOOL|ST_SHARED, -1, 1, SETS(Undo), NULL, NULL, NULL, MS_LOCAL|MS_SYSTEM},							// Undo p�/av. Default=p�.  BUFFER/BOOL
  {"undo_lines",	SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 10000000, SETS(Undotop), NULL, NULL, NULL, MS_LOCAL|MS_SYSTEM|MS_READ|MS_HIDDEN},// Antal rader i undobufferten. M�rk att v�rdet �r ett f�r lite.  BUFFER/INT
  {"undo_max",          SE_NOTHING,	NULL,	ST_NUM|ST_GLOBAL,0, 999999999, SETG(Undo_maxmemory), NULL, NULL, NULL, MS_GLOBAL|MS_SYSTEM},					// Maximalt minne som undobufferten f�r ta upp.  Default=100000.  GLOBAL/INT
  {"undo_memory",	SE_NOTHING,	NULL,	ST_NUM|ST_SHARED|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 10000000, SETS(Undomem), NULL, NULL, NULL, MS_LOCAL|MS_READ|MS_HIDDEN|MS_SYSTEM},// Storleken p� minnet som undobufferten tar upp.  BUFFER/INT/READ/NOSAVE
  {"undo_steps",        SE_NOTHING,	NULL,	ST_NUM|ST_GLOBAL,0, 999999, SETG(Undo_maxline), NULL, NULL, NULL, MS_GLOBAL|MS_SYSTEM},						// Maximalt antal rader i undobufferten.  Default=200.  GLOBAL/INT
  {"unpack",		SE_NOTHING,	NULL,	ST_BOOL|ST_GLOBAL, -1, 1, SETG(unpack), NULL, NULL, NULL, MS_GLOBAL|MS_IO},							// Om filen ska packas upp n�r den laddas.
  {"version",		SE_NOTHING,	NULL,	ST_NUM|ST_GLOBAL|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 10000000, SETG(version_int), NULL, NULL, NULL, MS_GLOBAL|MS_SYSTEM|MS_READ|MS_HIDDEN},	// Versionsnummer.
  {"version_id",	SE_NOTHING,	NULL,	ST_STRING|ST_GLOBAL|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 0, SETG(version_string), NULL, NULL, NULL, MS_GLOBAL|MS_SYSTEM|MS_READ|MS_HIDDEN},	// Versionsnummer.
  {"view_columns",	SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 10000000, SETB(screen_col), NULL, NULL, NULL, MS_LOCAL|MS_DISPLAY|MS_READ|MS_HIDDEN},	// Kolumnbredd i viewn.  ENTRY/INT/READ/NOSAVE
  {"view_lines",	SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 10000000, SETB(screen_lines), NULL, NULL, NULL, MS_LOCAL|MS_DISPLAY|MS_READ|MS_HIDDEN},	// Radh�jd i viewn. ENTRY/INT/READ/NOSAVE
  {"views",		SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ|ST_WINDOW|ST_HIDDEN,0, 0, SETW(Views), NULL, NULL, NULL, MS_LOCAL|MS_HIDDEN|MS_READ|MS_DISPLAY},	// Antal viewar p� sk�rmen.  GLOBAL/INT/READ/NOSAVE 
  {"visible",		SE_NOTHING,	NULL,	ST_NUM|ST_NOSAVE|ST_READ|ST_HIDDEN,0, 1, SETB(window), NULL, NULL, NULL, MS_LOCAL|MS_HIDDEN|MS_READ|MS_DISPLAY},		// Om entryt �r synlig eller ej. Returnerar 0 om det inte �r synligt, Icke 0 om synligt.  LOCAL/INT/READ/NOSAVE  
  {"window",		SE_REOPEN,	(char **)&windowtext,	ST_CYCLE|ST_WINDOW,	0, 3, SETW(window), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN},				// Om FrexxEd ska �ppnas p� ett f�nster.  0=Screen, 1=Window, 2=Backdrop, 3=WinScre. GLOBAL/INT
  {"window_height",     SE_RESIZE_WINDOW,NULL,	ST_NUM|ST_WINDOW|ST_HIDDEN, 18, 100000, SETW(window_height), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},					// F�nsterh�jd.  Default=250.  GLOBAL/INT
  {"window_pos",	SE_REOPEN,	(char **)&windowpostext,ST_CYCLE|ST_WINDOW|ST_HIDDEN, 0, 1, SETW(window_position), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN},			// F�nsterpostion.  0=Synlig, 1=Absolut.  GLOBAL/BOOL
  {"window_title",	SE_NOTHING,	NULL,	ST_STRING|ST_WINDOW, 0, 0, SETW(window_title), SEUP_TITLE_CHANGE, NULL, NULL, MS_LOCAL|MS_SCREEN},				// F�nstertittel. Fungerar bara till registrerade versioner. GLOBAL/STRING
  {"window_width",      SE_RESIZE_WINDOW,NULL,	ST_NUM|ST_WINDOW|ST_HIDDEN, 18, 100000, SETW(window_width), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},					// F�nsterbredd.  Default=640.  GLOBAL/INT
  {"window_xpos",	SE_RESIZE_WINDOW,NULL,	ST_NUM|ST_WINDOW|ST_HIDDEN, 0, 100000, SETW(window_xpos), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},						// F�nsterxposition.  Default=0.  GLOBAL/INT
  {"window_ypos",	SE_RESIZE_WINDOW,NULL,	ST_NUM|ST_WINDOW|ST_HIDDEN, 0, 100000, SETW(window_ypos), NULL, NULL, NULL, MS_LOCAL|MS_SCREEN|MS_HIDDEN},						// F�nsterypostition.  Default=0.  GLOBAL/INT
  {"windows_allocated",	SE_NOTHING,	NULL,	ST_NUM|ST_GLOBAL|ST_NOSAVE|ST_READ|ST_HIDDEN,	0, 0, SETG(windows_allocated), NULL, NULL, NULL, MS_GLOBAL|MS_SCREEN|MS_HIDDEN},// Antal allokerade f�nster.  GLOBAL/INT/HIDDEN
  {"windows_open",	SE_NOTHING,	NULL,	ST_NUM|ST_GLOBAL|ST_NOSAVE|ST_READ|ST_HIDDEN,	0, 0, SETG(windows_opened), NULL, NULL, NULL, MS_GLOBAL|MS_SCREEN|MS_HIDDEN},	// Antal �ppna f�nster.  GLOBAL/INT/HIDDEN
};

const int antaldefaultsets=sizeof(defaultsets)/sizeof(struct Setting);

int antalsets=0;
int antalallocsets=0;
struct Setting **sets=NULL;
struct SettingSaved *saved_default_setting=NULL;

/***************************** Rexx.c *******************************/
struct Library *RexxSysBase=NULL;
AREXXCONTEXT RexxHandle;

/***************************** Search.c *******************************/
HistoryStruct SearchHistory={NULL, 0, 0, 0};

BOOL searchcompiled=FALSE;
int searchcompile_flags=0;
char search_fastmap[256];

/****************************** Timer.c ********************************/
struct FrexxTimerRequest *TimerIO=NULL;
struct MsgPort *TimerMP=NULL;

/****************************** Undo.c ********************************/

/****************************** UpdtBlock.c ***************************/
struct BitMap EmptyBitMap={
  10000,
  10000,
  0,
  2,
  0,
  0,0,0,0,0,0,0,0
};

/****************************** UpdtScreen.c ***************************/
//UpdateStruct UpdtVars= { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

int UpDtNeeded=0;
int redrawneeded=0;
BOOL match_for_fastscroll=FALSE;
BOOL face_update=FALSE;

