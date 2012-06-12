#ifndef __FREXXED_COMPAT_H
#define __FREXXED_COMPAT_H


#include <stddef.h>


#ifndef AMIGA
#define __regargs
#define __a0
#define __a1
#define __a2
#define __a3
#define __a4
#define __a5
#define __a6
#define __a7
#define __d0
#define __d1
#define __d2
#define __d3
#define __d4
#define __d5
#define __d6

#undef __asm
#define __asm

#define __stackext
#define __saveds

#define SIGBREAKF_CTRL_C 1
#define RTFI_Dir 1

#define FPrintf fprintf
#define FPuts fputs

// FIXME: This is blatantly a hack
//#define REG_A4 1

#define BADDR(addr) (&(addr))

// Amiga types. FIXME: Verify these

typedef unsigned short USHORT;
typedef unsigned short UWORD;
typedef unsigned int ULONG;
typedef signed int LONG;
typedef signed short WORD;
typedef signed short SHORT;
typedef void * APTR;
typedef unsigned char UBYTE;
typedef unsigned char * STRPTR;
typedef unsigned char BOOL;
typedef unsigned long BPTR;
typedef void * IPTR;

// FIXME: These are AmigaOS structures that need proper definitions, or to be replaced.

extern struct Library * IFFParseBase;

struct Hook {
    void * h_Data;
    void * h_SubEntry;
    void * h_Entry;
};
struct Node {
    struct Node * ln_Pred;
    struct Node * ln_Succ;
    int ln_Pri;
    char * ln_Name;
    int ln_Type;
    int lib_Version; // FIXME: Probably means something points to a node when it should point to a library
};
struct InputEvent {
    int ie_Code;
    int ie_Qualifier;
    void * ie_EventAddress;
};
struct SGWork {
    struct Gadget * Gadget;
    int NumChars;
    struct InputEvent * IEvent;
    int Code;
    int *WorkBuffer;
    int EditOp;
    int BufferPos;
    int Actions;
    struct StringInfo * StringInfo;
};
struct Library {
    struct Node LibNode;
    int lib_Version;
};

extern struct Library * SysBase;

struct StoredProperty {
    void * sp_Data;
};
struct ContextNode {
    int cn_ID;
    int cn_Type;
};
struct Preferences {
    int color0;
    int color1;
    int color2;
    int color3;    
};
struct Gadget { 
    struct Gadget * NextGadget;
    int LeftEdge;
    int TopEdge;
    int Height;
    int Width;
    int Flags;
    int Activation;
    int GadgetType;
    void * GadgetRender;
    void * GadgetSelect;
    char * Text;
    int MX;
    void * SpecialInfo;
    int GadgetID;
    void * UserData;
};
struct Catalog {
    char * cat_Language;
};
struct StringInfo {
    void * Buffer;
    int MaxChars;
};
struct RexxMsg {
    struct {
        struct Node mn_Node;
    } rm_Node;
    char * rm_Args;
    void * rm_Args1;
    void * rm_Result1;
    void * rm_Result2;
    int rm_Action;
};
struct TextExtent {
    struct {
        int MinX;
        int MaxX;
    } te_Extent;
};
struct PropInfo { 
    int HorizPot;
    int HorizBody;
    int VertPot;
    int VertBody;
    int Flags;
};

struct Image { };
struct Border { 
    int XY;
    int FrontPen;
    int Count;
};
struct AppMessage {
    void * am_ArgList;
    int am_NumArgs;
    void * am_UserData;
    int am_Type;
};
struct WBArg {
    char * wa_Name;
    int wa_Lock;
};
struct DateTime {
    int dat_Format;
    int dat_Flags;
    int dat_StrDay;
    int dat_StrDate;
    int dat_StrTime;
};
struct DateStamp {
    int ds_Days;
    int ds_Minute;
    int ds_Tick;
};
struct TextAttr {
    const char *ta_Name;
    int ta_YSize;
    int ta_Style;
    int ta_Flags;
};
struct FontPrefs {
    char * fp_Name;
    struct TextAttr fp_TextAttr;
};
struct ExecBase {
    struct { 
        int lib_Version;
    } LibNode; 
};
struct RDArgs {
    struct {
        char * CS_Buffer;
        int CS_Length;
    } RDA_Source;
};
struct DosPacket {
    int dp_Type;
    void * dp_Arg1;
    void * dp_Arg2;
    void * dp_Arg3;
    void * dp_Arg4;
    void * dp_Res1;
    void * dp_Res2;
    void * dp_Port;
    void * dp_Link;
};
struct DosLibrary {
};
struct DeviceList {
    const char * dl_Name;
    struct DateStamp dl_VolumeDate;
    int dl_Lock;
    void * dl_Task;
    int dl_DiskType;
};
struct FileHandle {
    int fh_Type;
    void * fh_Arg1;
};
struct BitMap {
};
struct SignalSemaphore {
};
struct DiskObject {
     UWORD              do_Magic;   /* magic number at start of file */
     UWORD              do_Version; /* so we can change structure    */
     struct Gadget      do_Gadget;  /* a copy of in core gadget      */
     UBYTE              do_Type;
     char              *do_DefaultTool;
     char             **do_ToolTypes;
     LONG               do_CurrentX;
     LONG               do_CurrentY;
     struct DrawerData *do_DrawerData;
     char              *do_ToolWindow;  /* only applies to tools */
     LONG               do_StackSize;   /* only applies to tools */
};
struct DrawInfo {
    int dri_Pens[256];
};
struct InfoData {
    int id_NumSoftErrors;
    int id_UnitNumber;
    int id_DiskState;
    int id_NumBlocks;
    int id_NumBlocksUsed;
    int id_BytesPerBlock;
    int id_DiskType;
    int id_VolumeNode;
    int id_InUse;
};
struct FileLock {
    int fl_Key;
    int fl_Access;
    void * fl_Task;
    void * fl_Volume;
    void * fl_Link;
};
struct FileInfoBlock {
    char * fib_Comment;
    struct DateStamp fib_Date;
    int fib_DirEntryType;
    int fib_DiskKey;
    int fib_EntryType;
    char * fib_FileName;
    int fib_NumBlocks;
    int fib_Protection;
    long fib_Size;
};
struct AnchorPath {
    int ap_BreakBits;
    int ap_Strlen;
    char * ap_Buf;
    struct FileInfoBlock ap_Info;
};
struct IntuiText {
    struct IntuiText * NextText;
    const char * IText;
    void * ITextFont;
    int LeftEdge;
};
struct IntuitionBase {
    struct Screen * FirstScreen;
};
struct MenuItem {
    struct MenuItem * NextItem;
    struct MenuItem * SubItem;
    struct MenuItem * NextSelect;
    int Width;
    int LeftEdge;
    int ItemFill;
    int Flags;
    const char * IText;
};
struct Menu {
    struct Menu * NextMenu;
    struct MenuItem * FirstItem;
    int Width;
    int LeftEdge;
};
struct Message {
    struct MsgPort * mn_ReplyPort;
    struct Node mn_Node;
    int mn_Length;
};
struct WBStartup {
    struct Message sm_Message;
    int sm_NumArgs;
    struct WBArg * sm_ArgList;
};

struct IntuiMessage {
    struct Message ExecMessage;
    int Code;
    int Class;
    int Qualifier;
    void * IAddress;
    struct Window * IDCMPWindow;
    int Seconds;
    int Micros;
    int MouseX;
    int MouseY;
};
struct NewMenu {
    int nm_Flags;
    int nm_Type;
    char * nm_Label;
    int nm_CommKey;
    int nm_MutualExclude;
    void * nm_UserData;
};

struct NewWindow {
    int Height;
    int Width;
    int MinWidth;
    int MaxWidth;
    int MinHeight;
    int Flags;
    int Type;
    void * Screen;
    int LeftEdge;
    int TopEdge;
    char * Title;
    struct Gadget * FirstGadget;
};

struct NewGadget {
    int ng_Flags;
    int ng_GadgetID;
    int ng_GadgetText;
    int ng_Height;
    int ng_LeftEdge;
    int ng_TextAttr;
    int ng_TopEdge;
    int ng_UserData;
    int ng_VisualInfo;
    int ng_Width;
};

struct Bitmap {
    int Depth;
};

struct RastPort {
    struct Bitmap * BitMap;
    int TxHeight;
};

struct MinList {
    struct Node * mlh_Head;
    struct Node * mlh_Tail;
    struct Node * mlh_TailPred;
};
struct List {
    struct Node * lh_Head;
};
struct KeyMapNode {
    void * kn_KeyMap;
};
struct MsgPort {
    struct List mp_MsgList;
    long mp_SigBit;
    int mp_Flags;
    int mp_SigTask;
};

struct Font {
    int ta_YSize;
};

struct ColorMap {
};
struct TagItem {
};
struct DimensionInfo {
    struct {
        int MaxX;
        int MaxY;
    } TxtOScan;
};
struct DisplayInfo {
    int PropertyFlags;
};
struct EasyStruct {
    int es_GadgetFormat;
    void * es_TextFormat;
};
struct IOStdReq {
    struct Device * io_Device;
    int io_Command;
};
struct timerequest
{
    struct IOStdReq tr_node;
    struct {
        int tv_secs;
        int tv_micro;
    } tr_time;
};


struct ViewPort {
    struct ColorMap ColorMap;
    int DxOffset;
    int DyOffset;
};

struct Screen {
    int BarHeight;
    struct Font * Font;
    int MouseX;
    int MouseY;
    int Width;
    int Height;
    int Flags;
    int WBorLeft;
    int WBorTop;
    struct ViewPort ViewPort;
    struct Screen * NextScreen;
    struct Window * FirstWindow;
    struct RastPort RastPort;
};
struct PubScreenNode {
    struct Node psn_Node;
    int psn_Flags;
    struct Screen * psn_Screen;
};
struct Window {
    struct Screen * WScreen;
    struct Gadget * FirstGadget;
    struct RastPort * RPort;
    struct MsgPort * UserPort;
    struct WIndow * NextWindow;
    int LeftEdge;
    int TopEdge;
    int Width;
    int Height;
    int MinHeight;
    int BorderRight;
    int BorderLeft;
    int BorderBottom;
    int BorderTop;
    int Flags;
    int MinWidth;
    void * UserData;
};
struct TextFont {
    int tf_YSize;
    int tf_XSize;
    int tf_Baseline;
    int tf_Flags;
    int tf_BoldSmear;
};

struct IFFHandle {
    int iff_Stream;
};

#define FREQF_NOFILES 1
#define FREQF_SAVE 2
#define FREQF_MULTISELECT 4

struct rtFileList {
    struct rtFileList * Next;
    char * Name;
};
struct rtScreenModeRequester {
    int DisplayID;
    int OverscanType;
    int DisplayWidth;
    int DisplayHeight;
    int DisplayDepth;
    int AutoScroll;
};
struct rtFontRequester {
    int Attr;
};
struct rtFileRequester {
    char * Dir;
    char * MatchPat;
};

struct Task {
    struct Node tc_Node;
};
struct CommandLineInterface {
    char * cli_CommandDir;
};
struct Process {
    struct MsgPort * pr_MsgPort;
    struct Window * pr_WindowPtr;
    struct CommandLineInterface pr_CLI;
    struct Task pr_Task;
};
struct Task * FindTask(void *);
int Wait(int);
void * GetMsg(void *);
void Delay(long);

// FIXME: Arbitrary value
#define MEMF_ANY 0

#define FALSE 0
#define TRUE 1

#define WB_DISKMAGIC 123
#define WB_DISKVERSION 456

#define NO_ICON_POSITION -1
#define IECLASS_RAWKEY 4
#define HIRES_KEY 1
#define OSCAN_TEXT 2

#define IFFPARSE_SCAN 1
#define IFFERR_EOF 2
#define IFFF_WRITE 3
#define IFFSIZE_UNKNOWN 4
#define IFFERR_WRITE 5
#define TOPAZ_EIGHTY 0
#define FPF_ROMFONT 0
#define TR_ADDREQUEST 0
#define UNIT_MICROHZ 1

#define CONST_STRPTR const char *
#define FS_NORMAL 1
#define JAM1 2
#define JAM2 3

#define RTGL_Width 1
#define RTGL_Min 2
#define RTGL_Max 3
#define RTGL_ShowDefault 4
#define RTGL_BackFill 5

#define RTGS_TextFmt 5
#define RTGS_AllowEmpty 6

#define RT_TextAttr 5
#define RT_Screen 6

#define SGH_KEY 1
#define BUTTONIDCMP 0
#define EO_INSERTCHAR 1
#define GA_Immediate 2
#define GTLV_Labels 3
#define GTLV_MakeVisible 4
#define GTLV_Selected 5
#define GTLV_ShowSelected 6
#define GTLV_Top 7
#define GTST_EditHook 8
#define HOOKFUNC void (*)

#define LISTVIEWIDCMP 10
#define LISTVIEW_KIND 11
#define PLACETEXT_ABOVE 12
#define SGA_END 13
#define SGA_NEXTACTIVE 14
#define SGA_REDISPLAY 15
#define SGA_REUSE 16
#define SGA_USE 17
#define STRINGIDCMP 18
#define WA_CustomScreen 19
#define WA_Flags 20
#define WA_Gadgets 21
#define WA_IDCMP 22

#define CHECKED 0
#define CHECKIT 0
#define GTMENUITEM_USERDAT(arg) 0
#define NM_BARLABEL 0
#define NM_ITEM 2
#define NM_TITLE 0
#define NM_SUB 1
#define NM_COMMANDSTRING 0
#define NM_END 0
#define GTMN_NewLookMenus 0
#define GTMN_TextAttr 0
#define TAG_END 0
#define TAG_IGNORE 1
#define TAG_DONE 2
#define MENUTOGGLE 0

#define WBENCHSCREEN 0
#define BUTTON_KIND 0
#define PLACETEXT_RIGHT 0

#define IDCMP_GADGETDOWN 1
#define IDCMP_CLOSEWINDOW 2
#define IDCMP_RAWKEY 0
#define IDCMP_GADGETUP 3
#define IDCMP_ACTIVEWINDOW 4
#define IDCMP_CHANGEWINDOW 5
#define IDCMP_INACTIVEWINDOW 6
#define IDCMP_INTUITICKS 7
#define IDCMP_MENUPICK 8
#define IDCMP_MENUVERIFY 9
#define IDCMP_MOUSEBUTTONS 10
#define IDCMP_MOUSEMOVE 11
#define IDCMP_NEWSIZE 12

#define AFF_DISK 0
#define FREQF_FIXEDWIDTH 1
#define FREQF_PATGAD 2
#define FREQF_SCALE 3
#define GTYP_PROPGADGET 4
#define RTFI_Flags 5
#define RTFI_MatchPat 6
#define RTFO_Flags 7
#define RTFO_FontHeight 8
#define RTFO_FontName 9
#define RTFO_MaxHeight 10
#define RT_FONTREQ 11
#define RTSC_AutoScroll 12
#define RTSC_DisplayDepth 13
#define RTSC_DisplayHeight 14
#define RTSC_DisplayID 15
#define RTSC_DisplayWidth 16
#define RTSC_Flags 17
#define RTSC_MaxDepth 18
#define RTSC_MinDepth 19
#define RTSC_OverscanType 20
#define RT_SCREENMODEREQ 21
#define RT_WaitPointer 22
#define RT_Window 23
#define SCREQF_AUTOSCROLLGAD 24
#define SCREQF_DEPTHGAD 25
#define SCREQF_OVERSCANGAD 26
#define SCREQF_SIZEGADS 27

#define WFLG_SMART_REFRESH 0
#define CUSTOMSCREEN 0
#define PLACETEXT_IN 0
#define WFLG_NOCAREREFRESH 0
#define WA_AutoAdjust 0
#define WFLG_CLOSEGADGET 0
#define WFLG_DRAGBAR 0
#define GFLG_SELECTED 0
#define GFLG_GADGIMAGE 1
#define GFLG_GADHIMAGE 2
#define GFLG_GADGHIMAGE 3
#define GACT_IMMEDIATE 5
#define GACT_RELVERIFY 4
#define CHECKBOX_KIND 0
#define GACT_STRINGRIGHT 0
#define GTYP_BOOLGADGET 1
#define WBPROJECT 2
#define GTCY_Active 0
#define STRING_KIND 0
#define GTST_String 0
#define GTST_MaxChars 0
#define GTCB_Scaled 0
#define CYCLE_KIND 0
#define WFLG_DEPTHGADGET 0
#define WFLG_ACTIVATE 0
#define INTEGER_KIND 0
#define MODE_NEWFILE 0
#define MODE_OLDFILE 0
#define ACCESS_READ 0
#define SHARED_LOCK 0
#define STRINGA_Justification 0
#define GA_TabCycle 0
#define GTCY_Labels 0
#define GT_Underscore 0
#define GA_Disabled 0
#define GTIN_Number 0
#define GTCB_Checked 0

#define MEMF_CLEAR 0
#define MEMF_PUBLIC 0
#define MEMF_CHIP 0
#define MEMF_FAST 0

#define FSF_UNDERLINED 0
#define FPF_PROPORTIONAL 0
#define FSF_BOLD 0
#define FSF_ITALIC 0
#define FPF_DESIGNED 0

#define ID_PREF 0
#define ID_PRHD 0
#define ID_FONT 0
#define ID_FORM 0
#define IFFF_READ 0
#define IFFPARSE_STEP 0
#define IFFERR_EOC 0

#define GVF_GLOBAL_ONLY 1
#define GVF_LOCAL_ONLY 2
#define GVF_BINARY_VAR 3

#define COMPLEMENT 0

#define RC_ERROR 0

#define NP_Path 0
#define NP_Entry 1
#define NP_Name 2
#define NP_StackSize 3

#define NT_MESSAGE 4
#define NT_PROCESS 0
#define PA_SIGNAL 1
#define PF_ACTION 2
#define WBAPPICON 0

#define NT_REPLYMSG 0
#define RC_OK 1
#define RXCOMM 2
#define RXFB_RESULT 3
#define RXFF_RESULT 4

#define AUTOKNOB 0
#define FREEVERT 1
#define GACT_RIGHTBORDER 2
#define GFLG_GADGHBOX 3
#define GFLG_RELRIGHT 4
#define MAXBODY 5
#define PROPBORDERLESS 6
#define PROPNEWLOOK 7

#define SYS_Input 1
#define SYS_Output 2

#define LDF_VOLUMES 1
#define LDF_WRITE 2
#define DLT_VOLUME 3

#define ERROR_ACTION_NOT_KNOWN 0
#define ERROR_OBJECT_NOT_FOUND -1
#define ERROR_NO_MORE_ENTRIES -2
#define ERROR_DISK_FULL -3
#define ERROR_SEEK_ERROR -4
#define ERROR_WRITE_PROTECTED -5
#define ERROR_OBJECT_IN_USE -6
#define ERROR_OBJECT_EXISTS -7
#define LOCK_SAME_OBJECT 1
#define LOCK_SAME_VOLUME 2
#define LOCK_SAME 3
#define LOCK_DIFFERENT 4
#define OFFSET_BEGINNING 5
#define OFFSET_CURRENT 6
#define OFFSET_END 7
#define ERROR_NO_FREE_STORE 8
#define ST_USERDIR 1
#define ST_FILE 2
#define ID_VALIDATED 3
#define ID_FFS_DISK 4
#define ID_DOS_DISK 8
#define DOSTRUE 1
#define DOSFALSE 0

#define ACTION_IS_FILESYSTEM 1
#define ACTION_LOCATE_OBJECT 2
#define ACTION_FREE_LOCK 3
#define ACTION_EXAMINE_OBJECT 4
#define ACTION_FINDINPUT 5
#define ACTION_FINDUPDATE 6
#define ACTION_DISK_INFO 7
#define ACTION_INFO 8
#define ACTION_EXAMINE_NEXT 9
#define ACTION_COPY_DIR 10
#define ACTION_SAME_LOCK 11
#define ACTION_READ 12
#define ACTION_SEEK 13
#define ACTION_WRITE 14
#define ACTION_SET_PROTECT 15
#define ACTION_SET_COMMENT 16
#define ACTION_SET_DATE 17
#define ACTION_FINDOUTPUT 18
#define ACTION_END 19
#define ACTION_RENAME_OBJECT 20
#define ACTION_DELETE_OBJECT 21
#define ACTION_PARENT 22
#define ACTION_ADD_NOTIFY 23
#define ACTION_CHANGE_MODE 24
#define ACTION_CHANGE_SIGNAL 25
#define ACTION_COPY_DIR_FH 26
#define ACTION_CREATE_DIR 27
#define ACTION_CURRENT_VOLUME 28
#define ACTION_DIE 29
#define ACTION_DISK_CHANGE 30
#define ACTION_DISK_TYPE 31
#define ACTION_EVENT 32
#define ACTION_EXAMINE_ALL 33
#define ACTION_EXAMINE_FH 34
#define ACTION_FH_FROM_LOCK 35
#define ACTION_FLUSH 36
#define ACTION_FORMAT 37
#define ACTION_FREE_RECORD 38
#define ACTION_GET_BLOCK 39
#define ACTION_INHIBIT 40
#define ACTION_LOCK_RECORD 41
#define ACTION_MAKE_LINK 42
#define ACTION_MORE_CACHE 43
#define ACTION_PARENT_FH 44
#define ACTION_READ_LINK 45
#define ACTION_READ_RETURN 46
#define ACTION_REMOVE_NOTIFY 47
#define ACTION_RENAME_DISK 48
#define ACTION_SCREEN_MODE 49
#define ACTION_SERIALIZE_DISK 50
#define ACTION_SET_FILE_SIZE 51
#define ACTION_SET_MAP 52
#define ACTION_STARTUP 53
#define ACTION_TIMER 54
#define ACTION_WAIT_CHAR 55
#define ACTION_WRITE_PROTECT 56
#define ACTION_WRITE_RETURN 57

#define AMTYPE_APPICON 0
#define AMTYPE_APPWINDOW 1
#define IEQUALIFIER_LEFTBUTTON 11
#define IEQUALIFIER_MIDBUTTON 12
#define IEQUALIFIER_RBUTTON 13
#define MENUCANCEL 14
#define MENUHOT 15
#define MENUNULL 16
#define MENUUP 17
#define MIDDLEDOWN 18
#define MIDDLEUP 19
#define SELECTDOWN 20
#define SELECTUP 21
#define SIGBREAKF_CTRL_D 22
#define SIGBREAKF_CTRL_E 23
#define WFLG_WINDOWACTIVE 24
#define WFLG_ZOOMED 25

#define DIPF_IS_FOREIGN 0
#define DTAG_DIMS 1
#define DTAG_DISP 2
#define FILLPEN 3
#define FILLTEXTPEN 4
#define PSNF_PRIVATE 6
#define PUBLICSCREEN 7
#define REQTOOLSNAME 8
#define REQTOOLSVERSION 9
#define RTEZ_ReqTitle 10
#define RT_FILEREQ 11
#define SA_AutoScroll 12
#define SA_BlockPen 13
#define SA_Depth 14
#define SA_DetailPen 15
#define SA_DisplayID 16
#define SA_Font 17
#define SA_Height 18
#define SA_Interleaved 19
#define SA_LikeWorkbench 20
#define SA_Overscan 21
#define SA_Pens 22
#define SA_PubName 23
#define SA_Type 24
#define SA_Width 25
#define SHINEPEN 26
#define WA_BlockPen 28
#define WA_DetailPen 29
#define WA_Height 30
#define WA_Left 31
#define WA_NewLookMenus 32
#define WA_NoCareRefresh 33
#define WA_PubScreen 34
#define WA_ScreenTitle 35
#define WA_SizeBBottom 36
#define WA_SizeBRight 37
#define WA_SizeGadget 38
#define WA_Title 39
#define WA_Top 40
#define WA_Width 41
#define WA_Zoom 42
#define WFLG_BACKDROP 43
#define WFLG_BORDERLESS 44
#define WFLG_NEWLOOKMENUS 45
#define WFLG_REPORTMOUSE 46
#define XPKNAME 48

typedef char * BSTR;


// --------- Prototypes for compatibility layer functions
void * AllocMem(long size, int flags);
void * OpenLibrary(const STRPTR, long);


#else

/* Amiga / AROS */

#ifndef __reg
#define __reg(x)
#endif

#define __regargs

#define __a0 __reg("a0")
#define __a1 __reg("a1")
#define __a2 __reg("a2")
#define __a3 __reg("a3")
#define __a4 __reg("a4")
#define __a5 __reg("a5")
#define __a6 __reg("a6")
#define __a7 __reg("a7")

#define __d0 __reg("d0")
#define __d1 __reg("d1")
#define __d2 __reg("d2")
#define __d3 __reg("d3")
#define __d4 __reg("d4")
#define __d5 __reg("d5")
#define __d6 __reg("d6")

#undef __asm
#define __asm

#define __stackext
#define __aligned

#undef NULL
#define NULL (0L)

#ifdef __inline
#undef __inline
#endif
// We want to rely on the compiler to get this right except in exceptional cases
#define __inline

#endif /* Amiga / AROS */



#define FESIZE 32

#include "stat.h"
#include "util.h"

#endif
