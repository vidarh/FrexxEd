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
};

struct Library {
    struct Node LibNode;
};

extern struct Library * SysBase;

struct FontPrefs {
};
struct ContextNode {
    int cn_ID;
    int cn_Type;
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
struct PropInfo { };
struct Image { };
struct Border { };
struct DateStamp {};
struct TextAttr {
    const char *ta_Name;
    int ta_YSize;
    int ta_Style;
    int ta_Flags;
};
struct ExecBase {
    struct { 
        int lib_Version;
    } LibNode; 
};

struct DosLibrary {
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
    void * IAddress;
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
};
struct Window {
    struct Screen * WScreen;
    struct Gadget * FirstGadget;
    struct RastPort * RPort;
    struct MsgPort * UserPort;
    int Width;
    int Height;
    int MinHeight;
    int BorderRight;
    int BorderLeft;
    int BorderBottom;
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

void Wait(int);
void * GetMsg(void *);
void * OpenLibrary(char *, long);
void Delay(long);

// FIXME: Arbitrary value
#define MEMF_ANY 0

#define FALSE 0
#define TRUE 1

#define IFFPARSE_SCAN 1
#define IFFERR_EOF 2
#define IFFF_WRITE 3
#define IFFSIZE_UNKNOWN 4
#define IFFERR_WRITE 5

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
#define CHECKBOX_KIND 0
#define GACT_STRINGRIGHT 0
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

#define COMPLEMENT 0
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
