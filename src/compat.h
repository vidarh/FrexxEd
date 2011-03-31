#ifndef __FREXXED_COMPAT_H
#define __FREXXED_COMPAT_H


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

// FIXME: These are AmigaOS structures that need proper definitions, or to be replaced.

struct Gadget { 
    int GadgetID;
    void * UserData;
    void * SpecialInfo;
};
struct PropInfo { };
struct Image { };
struct Border { };
struct DateStamp {};
struct TextAttr {};
struct ExecBase {
    struct { 
        int lib_Version;
    } LibNode; 
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

struct Window {
  int Width;
  int Height;
  int MinHeight;
    void * UserPort;
};

// FIXME: Arbitrary value
#define MEMF_ANY 0
#define S_IWRITE 0

#define FALSE 0
#define TRUE 1

#define CHECKED 0
#define CHECKIT 0
#define GTMENUITEM_USERDAT(arg) 0
#define NM_BARLABEL 0
#define NM_ITEM 0
#define NM_TITLE 0
#define NM_SUB 0
#define NM_COMMANDSTRING 0
#define NM_END 0
#define GTMN_NewLookMenus 0
#define GTMN_TextAttr 0
#define TAG_END 0
#define MENUTOGGLE 0

#define WBENCHSCREEN 0
#define BUTTON_KIND 0
#define PLACETEXT_RIGHT 0
#define IDCMP_GADGETDOWN 0
#define WFLG_SMART_REFRESH 0
#define IDCMP_GADGETUP 0
#define CUSTOMSCREEN 0
#define PLACETEXT_IN 0
#define IDCMP_CLOSEWINDOW 0
#define WFLG_NOCAREREFRESH 0
#define WA_AutoAdjust 0
#define WFLG_CLOSEGADGET 0
#define WFLG_DRAGBAR 0
#define GFLG_SELECTED 0
#define CHECKBOX_KIND 0
#define GACT_STRINGRIGHT 0
#define IDCMP_RAWKEY 0
#define GTCY_Active 0
#define STRING_KIND 0
#define GTST_String 0
#define GTST_MaxChars 0
#define GTCB_Scaled 0
#define CYCLE_KIND 0
#define WFLG_DEPTHGADGET 0
#define WFLG_ACTIVATE 0
#define INTEGER_KIND 0
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
