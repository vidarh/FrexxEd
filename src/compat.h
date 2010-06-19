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

struct Gadget { };
struct PropInfo { };
struct Image { };
struct Border { };
struct DateStamp {};
struct TextAttr {};
struct ExecBase {};
struct IntuiText {
  struct IntuiText * NextText;
};
struct MenuItem {
  struct MenuItem * NextItem;
  struct MenuItem * SubItem;x
  int Width;
};
struct Menu {
  struct Menu * NextMenu;
  struct MenuItem * FirstItem
};
struct IntuiMessage {};
struct NewMenu {};

struct Window {
  int Width;
  int Height;
  int MinHeight;
};

// FIXME: Arbitrary value
#define MEMF_ANY 0
#define S_IWRITE 0

#define FALSE 0
#define TRUE 1

#endif




#ifdef __VBCC__
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

#endif

#endif
