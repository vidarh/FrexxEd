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

// FIXME: This is blatantly a hack
#define REG_A4 1

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

struct Node {
    int lib_Version;
    char * ln_Name;
};
struct InputEvent {
    int ie_Code;
    int ie_Qualifier;
    void * ie_EventAddress;
};
struct Library {
    struct Node LibNode;
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
    int GadgetID;
    void * UserData;
    void * SpecialInfo;
    int Flags;
    int LeftEdge;
    int TopEdge;
    int Height;
    int Width;
};
struct StringInfo {
    void * Buffer;
};
struct TextExtent {
    struct {
        int MinX;
        int MaxX;
    } te_Extent;
};
struct PropInfo { };
struct Image { };
struct Border { 
    int XY;
};
struct AppMessage {
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
};
struct FileHandle {
    int fh_Type;
    void * fh_Arg1;
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
struct IntuiText {
    struct IntuiText * NextText;
    const char * IText;
    void * ITextFont;
    int LeftEdge;
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
struct IntuiMessage {
    int Code;
    int Class;
    int Qualifier;
    void * IAddress;
    void * IDCMPWindow;
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

struct RastPort {
};

struct MsgPort {
    long mp_SigBit;
};

struct Font {
    int ta_YSize;
};

struct ColorMap {
};

struct Message {
    struct Node mn_Node;
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
    struct ViewPort ViewPort;
    struct Screen * NextScreen;
    struct Window * FirstWindow;
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
    int Flags;
    int MinWidth;
};
struct TextFont {
    int tf_YSize;
    int tf_XSize;
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
};

int Wait(int);
void * GetMsg(void *);
void * OpenLibrary(char *, long);
void Delay(long);

// FIXME: Arbitrary value
#define MEMF_ANY 0

#define FALSE 0
#define TRUE 1

#define WB_DISKMAGIC 123
#define WB_DISKVERSION 456

#define NO_ICON_POSITION -1

#define IFFPARSE_SCAN 1
#define IFFERR_EOF 2
#define IFFF_WRITE 3
#define IFFSIZE_UNKNOWN 4
#define IFFERR_WRITE 5

#define RTGL_Width 1
#define RTGL_Min 2
#define RTGL_Max 3
#define RTGL_ShowDefault 4
#define RTGL_BackFill 5

#define RTGS_TextFmt 5
#define RTGS_AllowEmpty 6

#define RT_TextAttr 5
#define RT_Screen 6

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
#define SYS_Input 1
#define SYS_Output 2

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
#define IDCMP_ACTIVEWINDOW 2
#define IDCMP_CHANGEWINDOW 3
#define IDCMP_INACTIVEWINDOW 4
#define IDCMP_INTUITICKS 5
#define IDCMP_MENUPICK 6
#define IDCMP_MENUVERIFY 7
#define IDCMP_MOUSEBUTTONS 8
#define IDCMP_MOUSEMOVE 9
#define IDCMP_NEWSIZE 10
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

typedef char * BSTR;

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
