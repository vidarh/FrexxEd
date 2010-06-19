/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**********************************************************************

MenuAdd(type, string, FPL);

**********************************************************************/

#ifdef AMIGA
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <libraries/gadtools.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#endif

#include <string.h>

#include "Buf.h"
#include "BuildMenu.h"
#include "Change.h"
#include "Command.h"
#include "Alloc.h"
#include "FACT.h"
#include "IDCMP.h"
#include "KeyAssign.h"

extern struct ExecBase *SysBase;
extern DefaultStruct Default;
extern struct MenuInfo menu;
extern struct RastPort ButtonRastPort;	//Rastport för att kunna testa med Requestfonten.
extern struct TextFont *RequestFont;
extern int requestfontwidth;		// Kalkulerad bredd på requestfonten.
extern int visible_height;
extern int visible_width;
extern char build_default_menues;
extern char FrexxEdStarted;
extern int ReturnValue;
extern char buffer[];
extern struct OwnMenu *menu_settingchain;
extern struct Setting **sets;
extern char GlobalEmptyString[];

static struct OwnMenu __regargs *menu_delete_item(struct MenuInfo *menu, struct OwnMenu *own);


/**********************************************************************
 *
 * menu_add();
 *
 * Add an entry at the end of the current list!
 *
 ****/

int __regargs menu_add(struct MenuInfo *menu,
	               struct OwnMenu *fill,
	               struct menu_position *mp)
{
  struct OwnMenu *item, *add;
  int ret=OK;

  if(ret=fill_in(&add, fill))
    return(ret);

  item=menu->ownmenu;
  if (item && mp) {
    item=find_menu(item, mp);
  }
  if (item) {
    if (!item->prev) {
      add->next=item;
      menu->ownmenu=add;
      add->prev=item->prev;
      item->prev=add;
    } else {
      add->next=item->next;
      if (item->next)
        item->next->prev=add;
      add->prev=item;
      item->next=add;
    }
  } else {
    add->prev=NULL;
    item=menu->ownmenu;
    add->next=item;
    if (item)
      item->prev=add;
    menu->ownmenu=add;
  }
  menu->array_size++;
  return(ret);
}

struct OwnMenu __regargs *find_menu(struct OwnMenu *item, struct menu_position *mp)
{
  struct OwnMenu *remember;
  int titlenum, itemnum, subitemnum;

  titlenum=mp->titlenum;
  if (!titlenum)
    return(NULL);
  itemnum=mp->itemnum;
  subitemnum=mp->subitemnum;

	// The list is reversed.
  while (item->next)
    item=item->next;

  remember=item;
  while (titlenum && item) {
    if (!(item->flags&OWNMENU_EXPUNGE) && item->nm_Type==NM_TITLE) {
      titlenum--;
      remember=item;
    }
    item=item->prev;
  }
  if (!item)
    remember=NULL;
  item=remember;
  while (itemnum && item) {
    item=item->prev;
    if (item) {
      if (!(item->flags&OWNMENU_EXPUNGE)) {
        if (item->nm_Type==NM_ITEM) {
          itemnum--;
          remember=item;
        }
        if (item->nm_Type==NM_TITLE) {
          remember=item;
          break;
        }
      }
    } else
      remember=NULL;
  }
  item=remember;
  while (subitemnum && item) {
    item=item->prev;
    if (item) {
      if (!(item->flags&OWNMENU_EXPUNGE)) {
        remember=item;
        if (item->nm_Type!=NM_SUB)
          break;
        subitemnum--;
      }
    } else
      remember=NULL;
  }
  return(remember);
}

/**********************************************************************
 *
 * fill_in();
 *
 * Allocates and fills in a OwnMennu structure.
 *
 ****/

int __regargs fill_in(struct OwnMenu **add, struct OwnMenu *fill)
{
  (*add)=(struct OwnMenu *)Malloc(sizeof(struct OwnMenu));
  if (*add) {
    memset(*add, 0, sizeof(struct OwnMenu));

    (*add)->keypress = fill->keypress;

    if(fill->keypress) {
      register char *key;

      fill->keypress->menu = *add;

      key=KeySequence(fill->keypress, ksMENU);
      (*add)->itext.IText=key;
      if (!key) {
        Dealloc(*add);
        return(OUT_OF_MEM);
      }
    }

    if(fill->flags&OWNMENU_COPYLABEL && fill->nm_Label!=NM_BARLABEL) {
      (*add)->flags|=OWNMENU_COPYLABEL;
      (*add)->nm_Label=(STRPTR)Strdup((STRPTR)fill->nm_Label);
    } else
      (*add)->nm_Label=(STRPTR)fill->nm_Label;
  
    if(fill->nm_Type!=NM_TITLE) {
      if(fill->flags&OWNMENU_SETTING) {
        (*add)->flags|=OWNMENU_COPYDATA;
        (*add)->FPL_program=(APTR)fill->FPL_program;
        (*add)->settingname=(APTR)fill->settingname;
        (*add)->nm_Flags=MENUTOGGLE|CHECKIT;
        (*add)->nextsetting=menu_settingchain;
        menu_settingchain=*add;
      } else if(fill->flags&OWNMENU_COPYDATA) {
        (*add)->flags|=OWNMENU_COPYDATA;
        (*add)->FPL_program=(APTR)Strdup(fill->FPL_program);
      } else
        (*add)->FPL_program=(APTR)fill->FPL_program;
    } else
      (*add)->FPL_program=NULL;

    (*add)->nm_Type=fill->nm_Type;

  } else
    return(OUT_OF_MEM);
  return(OK);
}


/**********************************************************************
 *
 * menu_clear();
 *
 * Delete the current menu list.
 *
 ****/
void menu_clear()
{
  build_default_menues=1;
  SendReturnMsg(cmd_MENUCLEAR, OK, NULL, NULL, NULL);
  menu_settingchain=NULL;
  if(menu.ownmenu) {
    if (menu.con==menu.ownmenu)
      menu.con=NULL;
    menu_delete(&menu, menu.ownmenu); /* delete the previous list */
    menu.ownmenu=NULL;
    menu.array_size=0;
  }
  if(menu.con) {
    menu_delete(&menu, menu.con); /* delete the previous list */
    menu.con=NULL;
  }
}
/**********************************************************************
 *
 * menu_build();
 *
 * Call the proper intuition functions to create the proper structures
 * for us to attach the menu.
 *
 *****/

int __regargs menu_build(struct MenuInfo *menu, WindowStruct *win)
{
  /*
   * First, create a nice array of NewMenu structures from our internal linked
   * list of menu information!
   */

  struct NewMenu *menulist;

  struct Menu *oldmenu=win->menus;

  if(menu->ownmenu) {

    if (menu->flags&OWNMENU_EXPUNGE) {
      register struct OwnMenu *item=menu->ownmenu;
      while (item) {
        if (item->flags&OWNMENU_EXPUNGE) {
          item=menu_delete_item(menu, item);
          if (item && item->prev)
            item=item->prev;
          else
            item=menu->ownmenu;
        } else
          item=item->next;
      }
      menu->flags&=~OWNMENU_EXPUNGE;
    }

    if(!(menulist=menu_getarray(menu)))
      return(FAIL);
  
    /* creates internal structs (the strings are *not* copied, but must remain
       readable while the menu exists!) */
    if(!(win->menus=CreateMenus(menulist, TAG_END)))
      return(FAIL);
  
    /*
     * Forget that array again now since we've used it enough for this time!
     */
  
    Dealloc(menulist);

  }
  if (win->window_pointer) {
    if (SysBase->LibNode.lib_Version < 39)
      FrexxLayoutMenues(win->menus, TRUE);
  
    if(!(LayoutMenus(win->menus, win->visualinfo,
                     GTMN_TextAttr, &Default.RequestFontAttr,
                     GTMN_NewLookMenus, TRUE,
                     TAG_END))) /* create a nice layout! */
      return(FAIL);
  }
  if (SysBase->LibNode.lib_Version < 39) {
    register struct Menu *menucount=win->menus;
    register int bredd=visible_width;
    SetFont(&ButtonRastPort, RequestFont);
    while (menucount) {
      register int size;
      size=EnlargeMenu(menucount->FirstItem, 0);
      if (menucount->LeftEdge < bredd &&
          menucount->LeftEdge+menucount->FirstItem->LeftEdge+menucount->FirstItem->Width>bredd) {
        register struct MenuItem *itemcount=menucount->FirstItem;
        while (itemcount) {
          itemcount->LeftEdge-=size;
          itemcount=itemcount->NextItem;
        }
      }
      menucount=menucount->NextMenu;
    }
  }

  build_default_menues=0;
  if(oldmenu && menu->ownmenu) { /* there was a previous one! */
    FreeMenus(oldmenu);
  }
  return(OK);
}

/**********************************************************************
 *
 * menu_attach();
 *
 * Attach the menu to the window.
 *
 ****/

int __regargs menu_attach(WindowStruct *win)
{
  if(win->window_pointer) {
    if(!win->menus) { /* && !menu.on || !menu.con || menu.ownmenu */
  
      if (FrexxEdStarted<2) {
        return(OK);
      }
  
    /* we have to rebuild things! */
      if(menu_build(&menu, win))
        return(FAIL);

      if (!(SetMenuStrip(win->window_pointer, win->menus)))
        return(FAIL);
  
//    menu.on=TRUE;
      if(menu.ownmenu) {
      /* if we created a new menu */
  
        menu.con=menu.ownmenu;
        menu.con_size=menu.array_size;
      }
    }
  }
  return(OK);
}



/**********************************************************************
 *
 * menu_detach();
 *
 * Release the menu strip from the window. This must be done to enable
 * changing of it.
 *
 *****/
void __regargs menu_detach(struct MenuInfo *menu, WindowStruct *win)
{
  if(!win->menus)
    /* we're already detached! */
    return;
//  menu->on=FALSE;

  if (win->window_pointer) {
    ClearMenuStrip(win->window_pointer);
    win->menus=NULL;
  }
}

void __regargs MenuDelete(struct menu_position *mp)
{
  int type;
  struct OwnMenu *item;
  item=find_menu(menu.ownmenu, mp);
  if (item)
    type=item->nm_Type;
  while (item) {
    item->flags|=OWNMENU_EXPUNGE;
    menu.flags|=OWNMENU_EXPUNGE;
    item=item->prev;
    if (item) {
      if ((type== NM_TITLE && (item->nm_Type==NM_ITEM || item->nm_Type==NM_SUB)) ||
          (type== NM_ITEM && item->nm_Type==NM_SUB))
        continue;
      else
        break;
    } else
      break;
  }
}

static struct OwnMenu __regargs *menu_delete_item(struct MenuInfo *menu, struct OwnMenu *own)
{
  struct OwnMenu *next=own->next;
  if (own->next)
    own->next->prev=own->prev;
  if (own->prev)
    own->prev->next=own->next;
  else
    menu->ownmenu=next;

  if (own->keypress) {
    own->keypress->menu=NULL;
    DeleteKmap(own->keypress);
  }
  if(own->flags&OWNMENU_COPYLABEL)
    /* we should Dealloc() label */
    Dealloc(own->nm_Label);
  if(own->flags&OWNMENU_COPYDATA)
    /* we should Dealloc() FPL_program */
    Dealloc(own->FPL_program);
  Dealloc(own->settingname);
  Dealloc(own->itext.IText);
  Dealloc(own); /* gone! */
  menu->array_size--;
  return(next);
}
/**********************************************************************
 *
 * menu_delete();
 *
 * Free the entire linked list of menu information.
 *
 *****/

void __regargs menu_delete(struct MenuInfo *menu, struct OwnMenu *own)
{
  /* free the entire linked list of menu information! */
  while(own) {
    own=menu_delete_item(menu, own);
  }
}


/**********************************************************************
 *
 * menu_getarray();
 *
 * Create the NewMenu structure array that we need to CreateMenus().
 *
 *****/

struct NewMenu * __regargs menu_getarray(struct MenuInfo *menu)
{
  struct NewMenu *array;
  struct OwnMenu *pnt;
  int num;

  array=(struct NewMenu *)Malloc( (menu->array_size+1) * sizeof(struct NewMenu) );
  if (array) {
    pnt=menu->ownmenu;
    while (pnt->next)
      pnt=pnt->next;
  
    for(num=0; num<menu->array_size; num++) {
      array[num].nm_Type=pnt->nm_Type;
      array[num].nm_Label=pnt->nm_Label;
      array[num].nm_Flags=pnt->nm_Flags; /* no such either! */
      array[num].nm_CommKey=NULL; /* no menuitem command key equiv! */
      array[num].nm_MutualExclude=0; /* no exclude! */
      array[num].nm_UserData=pnt; /* struct OwnMenu pointer */
      if (pnt->keypress && SysBase->LibNode.lib_Version >=39) {
        array[num].nm_Flags|=NM_COMMANDSTRING;
        array[num].nm_CommKey=pnt->itext.IText;
      }
      pnt=pnt->prev; /* next OwnMenu structure */
    }
    array[0].nm_Type=NM_TITLE;
    memset(&array[num], 0, sizeof(struct NewMenu));
    array[num].nm_Type=NM_END;
  }
  return(array);
}

/**********************************************************************
 *
 * SetDefaultMenu();
 *
 * This function inits the default menu stript.
 *
 ******/

int __regargs SetDefaultMenu(struct MenuInfo *menu)
{
  /********************************
   *                              *
   * All default menu information *
   *                              *
   ********************************/
  static const struct StoreMenu defmenu[]={
    {NM_TITLE, STR_PROJECT_MENU, NULL, NULL},
    {NM_ITEM,  STR_PROJECT_NEW, "{int i=New();Activate(i);CurrentBuffer(i);}", "amiga n"},
    {NM_ITEM,  STR_PROJECT_LOAD, "Load();", "amiga o"},
    {NM_ITEM,  STR_PROJECT_SAVE, "Save();", "amiga s"},
    {NM_ITEM,  STR_PROJECT_ABOUT, "About();", "amiga ?"},
    {NM_ITEM,  STR_PROJECT_KILL, "Kill();", "amiga q"},
    {NM_ITEM,  STR_PROJECT_QUIT_ALL, "QuitAll();", "amiga Q"},

    {NM_TITLE, STR_BLOCK_MENU, NULL, NULL},
    {NM_ITEM,  STR_BLOCK_MARK, "BlockMark();", "amiga b"},
    {NM_ITEM,  STR_BLOCK_CUT, "BlockCut();", "amiga x"},
    {NM_ITEM,  STR_BLOCK_COPY, "BlockCopy();", "amiga c"},
    {NM_ITEM,  STR_BLOCK_INSERT, "BlockPaste();", "amiga v"},

    {NM_TITLE, STR_EDIT_MENU, NULL, NULL},
    {NM_ITEM,  STR_EDIT_UNDO, "Undo();", "amiga z"},
//    {NM_ITEM,  STR_EDIT_UNDO_RESTART, "UndoRestart();", NULL},

//    {NM_TITLE, STR_MOVEVIEW_MENU, NULL, NULL},
//    {NM_ITEM,  STR_MOVEVIEW_MAXIVIEW, "MaximizeView();", NULL},
//    {NM_ITEM,  STR_MOVEVIEW_GOTO_LINE, "GotoLine();", NULL},

    {NM_TITLE, STR_SEARCHREPLACE_MENU, NULL, NULL},
//    {NM_ITEM,  STR_SEARCHREPLACE_R_SEARCH_FRW, "SearchSet(\"f+\"); Search();", NULL},
    {NM_ITEM,  STR_SEARCHREPLACE_SEARCH, "if (SearchSet()>=0) Search();", NULL},
//    {NM_ITEM,  STR_SEARCHREPLACE_REPEAT_REPLACE, "Replace();", NULL},
    {NM_ITEM,  STR_SEARCHREPLACE_REPLACE, "if (ReplaceSet()>=0) Replace();", NULL},

    {NM_TITLE, STR_CUSTOMIZING_MENU, NULL, NULL},
    {NM_ITEM,  STR_CUSTOMIZING_SETTINGS, NULL, NULL},
    {NM_SUB,   STR_CUSTOMIZING_SETTINGS_LOCALS, "PromptInfo(-1);", NULL},
    {NM_SUB,   STR_CUSTOMIZING_SETTINGS_GLOBALS, "PromptInfo(0);", NULL},
    {NM_SUB,   STR_CUSTOMIZING_SETTINGS_SAVE, "SetSave();", NULL},
    {NM_ITEM,  STR_CUSTOMIZING_SETTINGS_SCREENMODE, "Screenmode();", NULL},

    {NM_TITLE, STR_SPECIAL_MENU, NULL, NULL},
    {NM_ITEM,  STR_SPECIAL_EXECUTE_FILE, "ExecuteFile(PromptFile());", NULL},
    {NM_ITEM,  STR_SPECIAL_LOAD_MENUS, "ExecuteFile(\"Menu.FPL\");MenuBuild();", NULL},
    {NM_ITEM,  STR_SPECIAL_PROMPT, "Prompt();", NULL},
  };

  int cnt;

  build_default_menues=0;

  menu_clear(); // clear old one (950703)

  for(cnt=0;cnt<sizeof(defmenu)/sizeof(struct StoreMenu);cnt++) {
    struct OwnMenu data;
    data.nm_Type=defmenu[cnt].nm_Type;
#if 0
    if(defmenu[cnt].num<0)
      data.nm_Label=NM_BARLABEL;
    else
#endif
      data.nm_Label=RetString(defmenu[cnt].num);
    data.FPL_program=defmenu[cnt].FPL_program;

    if(defmenu[cnt].key) {
      if(OK > KeyAssign(NULL, defmenu[cnt].key,
                        defmenu[cnt].FPL_program, 0, kaMENU))
        continue;
      data.keypress= (struct Kmap *)ReturnValue;
    } else
      data.keypress = NULL;

    data.flags=0;
    if(menu_add(menu, &data, NULL))
      return(FAIL);
  }
  return(OK);
}

int __regargs MenuAdd(int argc, char **strings)
{
  int ret=OK;
  struct OwnMenu data;
  int len=strlen(strings[0]);
  struct menu_position mp;

  if (build_default_menues==2)
    SetDefaultMenu(&menu);

  data.flags=OWNMENU_COPYLABEL|OWNMENU_COPYDATA;
  switch (strings[0][0]) {
  case 'i':
  case 'I':
    if (!Strnicmp(strings[0], MENU_ITEM, len)) {
      data.nm_Type=NM_ITEM;
      break;
    }
    return(SYNTAX_ERROR);
  case 's':
  case 'S':
    if (!Strnicmp(strings[0], MENU_SUBITEM, len)) {
      data.nm_Type=NM_SUB;
      break;
    } else if (!Strnicmp(strings[0], MENU_SETTING, len)) {
      data.nm_Type=NM_ITEM;
      data.flags|=OWNMENU_SETTING;
      break;
    } else if (!Strnicmp(strings[0], MENU_SUBSETTING, len)) {
      data.nm_Type=NM_SUB;
      data.flags|=OWNMENU_SETTING;
      break;
    }
    return(SYNTAX_ERROR);
  case 't':
  case 'T':
    if (!Strnicmp(strings[0], MENU_TITLE, len)) {
      data.nm_Type=NM_TITLE;
      break;
    }
    return(SYNTAX_ERROR);
  default:
    return(SYNTAX_ERROR);
  }

  if (strings[1][0]!='-' || strncmp(strings[1], "--------", strlen(strings[1])))
    data.nm_Label=strings[1];
  else
    data.nm_Label=NM_BARLABEL;

  data.FPL_program=NULL;
  data.settingname=NULL;
  if (argc>2 && strings[2][0]) {
    if (data.flags&OWNMENU_SETTING) {
      data.settingname=Strdup(strings[2]);
      Sprintf(buffer, "SetInfo(-1,\"%s\",%ld);", strings[2], -1);
      data.FPL_program=Strdup(buffer);
    } else
      data.FPL_program=strings[2];
  }
  if (argc>3 && strings[3][0]) {
    ret=KeyAssign(NULL, strings[3], data.FPL_program, 0, kaMENU);
    if (ret<OK)
      return(ret);
    data.keypress= (struct Kmap *)ReturnValue;
  } else
    data.keypress= NULL;
  if (argc>4) {	// Menu position is given
    mp.titlenum=(int)strings[4];
    mp.itemnum=0;
    mp.subitemnum=0;
    if (argc>5) {
      mp.itemnum=(int)strings[5];
      if (argc>6)
        mp.subitemnum=(int)strings[6];
    }
    ret=menu_add(&menu, &data, &mp);
  } else
    ret=menu_add(&menu, &data, NULL);
  return(ret);
}


void __regargs FrexxLayoutMenues(struct Menu *menucount, int add)
{
  struct MenuItem *itemcount;
  struct MenuItem *subitemcount;

  while (menucount) {
    menucount->Width+=30;
    itemcount=menucount->FirstItem;
    while (itemcount) {
      subitemcount=itemcount->SubItem;
      if (subitemcount) {
        while (subitemcount) {
          FixIntuiText(subitemcount, add);
          subitemcount=subitemcount->NextItem;
        }
      } else
        FixIntuiText(itemcount, add);
      itemcount=itemcount->NextItem;
    }
    menucount=menucount->NextMenu;
  }
}

void __regargs FixIntuiText(struct MenuItem *item, int add)
{
  register struct OwnMenu *own;
  own=((struct OwnMenu *)GTMENUITEM_USERDATA(item));
  if (own && own->keypress) {
    if (add) {
      own->itextpatch=(struct IntuiText *)item->ItemFill;
      own->itextpatch->NextText=&own->itext;
      own->itext.ITextFont=&Default.RequestFontAttr;
    } else
      own->itextpatch->NextText=NULL;
  }
}



/* Skicka in första itemet */
int __regargs EnlargeMenu(struct MenuItem *firstitem, int moveitem)
{
  struct MenuItem *itemcount=firstitem;
  int size=0, orgsize=firstitem->Width;
  int orgleft;

  if (firstitem)
    orgleft=firstitem->LeftEdge;
  while (itemcount) {
    if (((struct IntuiText *)itemcount->ItemFill)->NextText) {
      register int tempsize;
      tempsize=TextLength(&ButtonRastPort, 
                          ((struct IntuiText *)itemcount->ItemFill)->NextText->IText,
                          strlen(((struct IntuiText *)itemcount->ItemFill)->NextText->IText));
      if (tempsize>size)
        size=tempsize;
    }
    itemcount=itemcount->NextItem;
    if (itemcount && orgleft!=itemcount->LeftEdge)
      break;
  }    
  if (size) {
    size+=requestfontwidth;
    orgsize+=requestfontwidth;
    itemcount=firstitem;
  
    while (itemcount) {
      itemcount->Width+=size;
      itemcount->LeftEdge+=moveitem;
      if (itemcount->SubItem)
        EnlargeMenu(itemcount->SubItem, moveitem+size);
      else {
        if (((struct IntuiText *)itemcount->ItemFill)->NextText) {
          {
            register char *temp=((struct IntuiText *)itemcount->ItemFill)->NextText->IText;
            memcpy(((struct IntuiText *)itemcount->ItemFill)->NextText, ((struct IntuiText *)itemcount->ItemFill), sizeof(struct IntuiText));
            ((struct IntuiText *)itemcount->ItemFill)->NextText->IText=temp;
          }
          ((struct IntuiText *)itemcount->ItemFill)->NextText->LeftEdge=orgsize;
          ((struct IntuiText *)itemcount->ItemFill)->NextText->NextText=NULL;
        }
      }
      itemcount=itemcount->NextItem;
      if (itemcount && orgleft!=itemcount->LeftEdge) {
        moveitem+=size+requestfontwidth;
        EnlargeMenu(itemcount, moveitem);
        itemcount=NULL;
      }
    }    
  }
  return(size);
}


BOOL __regargs SetItem(BufStruct *Storage, struct MenuItem *item)
{
  struct OwnMenu *count=menu_settingchain;
  int vald;
  char *text=((struct IntuiText *)(item->ItemFill))->IText;
  char *text2=NULL;
  BOOL changed=FALSE;


  if (((struct IntuiText *)(item->ItemFill))->NextText)
    text2=((struct IntuiText *)(item->ItemFill))->NextText->IText;

  while (count) {
    if (text==count->itext.IText || text2==count->itext.IText) {
      vald=GetSettingName(count->settingname);
      if (vald>=0) {
        vald=ChangeAsk(Storage, vald, NULL);
        if ((sets[vald]->type&15)==ST_BOOL &&
            vald && !(item->Flags&CHECKED) ||
            !vald && (item->Flags&CHECKED)) {
          item->Flags=(item->Flags&~CHECKED)|(vald?CHECKED:0);
          changed=TRUE;
        }
        break;
      }
    }
    count=count->nextsetting;
  }
  return(changed);
}

BOOL __regargs InitializeMenu(BufStruct *Storage)
{
  BOOL changed=FALSE;
  struct Menu *menucount=BUF(window)->menus;
  struct MenuItem *itemcount;
  struct MenuItem *subitemcount;

  while (menucount) {
    itemcount=menucount->FirstItem;
    while (itemcount) {
      subitemcount=itemcount->SubItem;
      if (subitemcount) {
        do {
          if (subitemcount->Flags&CHECKIT) {
            changed|=SetItem(Storage, subitemcount);
          }
          subitemcount=subitemcount->NextItem;
        } while (subitemcount);
      } else {
        if (itemcount->Flags&CHECKIT) {
          changed|=SetItem(Storage, itemcount);
        }
      }
      itemcount=itemcount->NextItem;
    }
    menucount=menucount->NextMenu;
  }
  return(changed);
}

