/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
#include "Setting.h"

struct OwnMenu {
  UBYTE nm_Type;		/*  NM_xxx defines in <libraries/gadtools.h> */
  STRPTR nm_Label;		/*  Menu's label */
  short nm_Flags;			/*  Menu's flags */
  char *FPL_program;		/*  FPL program */
  char *settingname;		/*  setting name */
  struct Kmap *keypress;	/*  Key assign */
  struct IntuiText itext;
  struct IntuiText *itextpatch;
  struct OwnMenu *next;  /* dual direction linked list */
  struct OwnMenu *prev;
  struct OwnMenu *nextsetting;
  UBYTE flags;			/* see below */
};

struct StoreMenu {
  /*
   * Internal struct only to store default menu data.
   */
  UBYTE nm_Type;		/*  NM_xxx defines in <libraries/gadtools.h> */
  int  num;		        /*  Menu's label string number */
  char *FPL_program;		/*  FPL program */
  char *key;
};

struct menu_position {
  int titlenum;
  int itemnum;
  int subitemnum;
};

#define MENU_TITLE "title"
#define MENU_ITEM "item"
#define MENU_SUBITEM "subitem"
#define MENU_SETTING "setting"
#define MENU_SUBSETTING "subsetting"
#define MENU_BAR "--------"


#define OWNMENU_COPYLABEL 1 /* this struct contains a copied and allocated
			       label member. When this struct is to be
			       freed, the label must be freed! */
#define OWNMENU_COPYDATA 2  /* this struct contains a copied and allocated
			       userdata member. When this struct is to be
			       freed, the userdata must be freed! */
#define OWNMENU_SETTING 4
#define OWNMENU_EXPUNGE 8
#define FAIL 1

struct MenuInfo {
//  struct Menu *menus;
//  char on; /* on/off ? */
  struct OwnMenu *ownmenu; /* current menu list  */
  struct OwnMenu *con;     /* constructed menu list */
  int array_size;	   /* current array size */
  int con_size;		   /* constructed array size */
  int flags;		   /* different flags (same as OWNMENU_... */
};

extern struct MenuInfo menu;

int  menu_attach(WindowStruct *);
void menu_clear(void);
void menu_detach(struct MenuInfo *, WindowStruct *);
void menu_delete(struct MenuInfo *, struct OwnMenu *);

int  SetDefaultMenu(struct MenuInfo *);
void FrexxLayoutMenues(struct Menu *menucount, int add);

BOOL InitializeMenu(BufStruct *Storage);
BOOL SetItem(BufStruct *Storage, struct MenuItem *item);

struct OwnMenu *find_menu(struct OwnMenu *item, struct menu_position *mp);

int MenuAdd(int, char **);
void MenuDelete(struct menu_position *mp);

