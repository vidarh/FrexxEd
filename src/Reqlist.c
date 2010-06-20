/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**********************************************************************
 *
 * Reqlist.c
 *
 * Reqlist requester source code. Part of the FrexxEd project.
 *
 ******/

#include <devices/console.h>
#include <devices/inputevent.h>
#include <exec/execbase.h>
#include <exec/types.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxbase.h>
#include <intuition/classes.h>
#include <intuition/classusr.h>
#include <intuition/gadgetclass.h>
#include <intuition/imageclass.h>
#include <intuition/intuition.h>
#include <intuition/sghooks.h>
#include <libraries/gadtools.h>
#include <proto/console.h>
#include <proto/exec.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <stdarg.h>
#include <string.h>
#include <utility/hooks.h>

#include "Buf.h"
#include "Alloc.h"
#include "BufControl.h"
#include "Button.h"
#include "Edit.h"
#include "Limit.h"
#include "Rawkeys.h"
#include "Reqlist.h"
#include "Request.h"
#include "Sort.h"
#include "UpdtScreen.h"

extern struct GfxBase *GfxBase;
extern struct Library *GadToolsBase;
extern struct IntuitionBase *IntuitionBase;
extern DefaultStruct Default;
extern struct TextFont *RequestFont;	/* RequestFont */
extern struct ReqToolsBase *ReqToolsBase;
extern int requestfontwidth;		// Kalkulerad bredd på requestfonten.
extern BlockStruct *BlockBuffer;
extern struct InputEvent ievent;        /* used for RawKeyConvert() */
extern char buffer[];
extern struct KeyMap *internalkeymap;
extern int visible_height;
extern int visible_width;
extern char DebugOpt;

typedef struct {
  char **array;			// Sträng array.
  int number;			// Antal medlemmar.
  struct Gadget *gadget1;
  int listpos1;
  struct Gadget *gadget2;
  int listpos2;
  struct Gadget *listview;
  int listviewlines;
  ButtonStruct *buts;
  BlockStruct *block;
  struct Window *window;
  long a4;
  char findflag;
} ListStruct;

#define FIND_DOWN	1
#define FIND_SORT	2
#define FIND_SORTCASE	4
#define FIND_NEAR	8


static int __regargs _strcmp_(int flag, char *str1, char *str2)
{
  if (flag&FIND_SORTCASE)
    return(strcmp(str1, str2));
  else
    return(Stricmp(str1, str2));
}
static int __regargs FindListName(ListStruct *list,
                                  char *name,
                                  int curxpos,
                                  int curypos,
                                  char flag)
{
  int n;
  n=curypos;

  if (flag&(FIND_SORT|FIND_SORTCASE)) {
    if (n<0)
      flag=flag|FIND_DOWN;
    else if (n>=list->number)
      flag=(flag&~FIND_DOWN);
    else {
      register int check;
      check=_strcmp_(flag, name, list->array[n]);
      if (check>0)
        flag=flag|FIND_DOWN;
      else if (check<0)
        flag=(flag&~FIND_DOWN);
    }
  } else {
    if (n<0) {
      for (n=0; n<list->number; n++) {
        if (!strcmp(name, list->array[n])) {
          n--;
          break;
        }
      }
      if (n>=list->number) {
        for (n=0; n<list->number-1; n++) {
          if (strcmp(name, list->array[n])<0) {
            n--;
            break;
          }
        }
      }
    }
  }

  if(flag&FIND_DOWN) {
    if(n<list->number) {
      n++;
      if (flag&(FIND_SORT|FIND_SORTCASE)) {
        for(; n<list->number; n++) {
          if (_strcmp_(flag, name, list->array[n])<=0)
            break;
        }
//        if (flag&(FIND_SORT|FIND_SORTCASE)) {
//          if (n<0)
//            n=FindListName(list, name, curxpos, curypos, flag&~FIND_DOWN);
//        }
      }
      if (n>list->number-1)
        n=list->number-1;
    }
  } else {
    if(n>=0) {
      n--;
      if (flag&(FIND_SORT|FIND_SORTCASE)) {
        for(; n>=0; n--) {
          if (_strcmp_(flag, name, list->array[n])>=0)
            break;
        }
      }
      if (n<0)
        n=0;
    }
  }
  if (n>=list->number)
    n=-1;
  return n;
}

static int __asm
          HookControl(register __a0 struct Hook *hookptr,
                      register __a2 struct SGWork *object,
                      register __a1 ULONG *msg)
{
  int retcode=NULL;
  register ListStruct *list=(ListStruct *)hookptr->h_Data;
  int newstring=-1;
  int *pos=NULL, oldtop=-1;
  char movecursor=TRUE;
//  long olda4=getreg(REG_A4);
#ifdef REG_A4  
  putreg(REG_A4, list->a4);
#endif

  if (list->number) {
    if (object->Gadget==list->gadget1)
      pos=&list->listpos1;
    else if (object->Gadget==list->gadget2)
      pos=&list->listpos2;
    if (pos && object->NumChars)
      oldtop=*pos;
  }
  if (*msg==SGH_KEY) {
    if (list->number>0 && pos) {
      if (object->IEvent->ie_Code==RAW_UP) {
        if (*pos<0 || (object->IEvent->ie_Qualifier&RAW_SHIFT) || (object->IEvent->ie_Qualifier&RAW_CTRL)) {
          *pos=0;
        } else {
          register int newpos;
          newpos=FindListName(list, object->WorkBuffer, (list->findflag&(FIND_SORT|FIND_SORTCASE))?object->NumChars:object->BufferPos, *pos, list->findflag);
          if (newpos>=0)
            *pos=newpos;
        }
        newstring=*pos;
        {
          register int len=strlen(list->array[newstring]);
          if (len<object->BufferPos)
            object->BufferPos=len;
        }
        movecursor=FALSE;
      } else if (object->IEvent->ie_Code==RAW_DOWN) {
        if ((object->IEvent->ie_Qualifier&RAW_SHIFT) || (object->IEvent->ie_Qualifier&RAW_CTRL)) {
          *pos=list->number-1;
        } else {
          register int newpos;
          newpos=FindListName(list, object->WorkBuffer, (list->findflag&(FIND_SORT|FIND_SORTCASE))?object->NumChars:object->BufferPos, *pos, FIND_DOWN|list->findflag);
          if (newpos>=0)
            *pos=newpos;
        }
        if (*pos<0)
          *pos=0;
        newstring=*pos;
        {
          register int len=strlen(list->array[newstring]);
          if (len<object->BufferPos)
            object->BufferPos=len;
        }
        movecursor=FALSE;
      } else if ((object->IEvent->ie_Code<RAWC_LEFT_SHIFT || 
                  object->IEvent->ie_Code>RAWC_MIDDLE_MOUSE) &&
                 (object->IEvent->ie_Code<RAW_UP ||
                  object->IEvent->ie_Code>RAW_LEFT)) {
        if (SysBase->LibNode.lib_Version >= 39) {
          GT_SetGadgetAttrs(list->listview, list->window, NULL,
                            GTLV_Selected, ~0,
                            TAG_END);
        }
        *pos=-1;
        pos=NULL;
      }
    }
    if (object->IEvent->ie_Code==RAW_ESC) {
      object->Actions|=SGA_REUSE|SGA_END;
      retcode=-1;
    } else if (Default.req_ret_mode && object->IEvent->ie_Code==RAW_RETURN) {
      if (object->Gadget==list->gadget2)
        object->Actions|=SGA_REUSE|SGA_END;
      else
        object->Actions|=SGA_NEXTACTIVE;
      retcode=-1;
    } else if (object->IEvent->ie_Qualifier&RAW_AMIGA) {

      if (TOLOWER(object->Code)==0x7f) {	// Del
        object->NumChars=object->BufferPos;
        object->Actions|=SGA_USE|SGA_REDISPLAY;
        retcode=-1;
        if (pos)
          *pos=-1;
      } else if (TOLOWER(object->Code)=='\b') {	// Backspace
        object->NumChars-=object->BufferPos;
        memcpy(object->WorkBuffer,
               object->WorkBuffer+object->BufferPos,
               object->NumChars);
        object->BufferPos=0;
        object->Actions|=SGA_USE|SGA_REDISPLAY;
        retcode=-1;
        if (pos)
          *pos=-1;
      } else if (TOLOWER(object->Code)=='v') {	// Block paste
        register int count=1;
        register int len=0;
        register BlockStruct *block=list->block;
        while (count<=block->line)
          len+=block->text[count++].length;
        if (len+object->NumChars<object->StringInfo->MaxChars) {
          if (object->EditOp==EO_INSERTCHAR)
            len--;
          count=1;
          MoveUp(object->WorkBuffer+object->BufferPos+len,
                 object->WorkBuffer+object->BufferPos,
                 object->NumChars-object->BufferPos);
          len=0;
          if (object->EditOp==EO_INSERTCHAR)
            len--;
          while (count<=block->line) {
            memcpy(object->WorkBuffer+object->BufferPos+len,
                   block->text[count].text, block->text[count].length);
            len+=block->text[count].length;
            count++;
          }
          object->BufferPos+=len;
          object->NumChars+=len;
          object->Actions|=SGA_USE|SGA_REDISPLAY;
          retcode=-1;
          if (pos)
            *pos=-1;
        }
      } else {
        if (list->buts) {
          struct Gadget *gadget=CheckHotKey(list->buts, object->Code);
          if (gadget) {
            object->Actions&=~SGA_USE;
            retcode=-1;
            GT_SetGadgetAttrs(gadget,
                              list->buts->winbutton,
                              NULL,
                              GTCB_Checked, (gadget->Flags & SELECTED)?FALSE:TRUE,
                              TAG_END);
          }
        }
      }
      object->WorkBuffer[object->NumChars]=0;
    }

    if (newstring>=0) {
      object->Actions|=SGA_USE|SGA_REDISPLAY;
      strcpy(object->WorkBuffer, list->array[newstring]);
      object->NumChars=strlen(list->array[newstring]);
      retcode=-1;
      if (movecursor)
        object->BufferPos=0;
    }
    if (pos && *pos>=0 && *pos!=oldtop && !strcmp(object->WorkBuffer, list->array[*pos])) {
      register int top=GTLV_MakeVisible;
      if (SysBase->LibNode.lib_Version < 39)
        top=GTLV_Top;
      GT_SetGadgetAttrs(list->listview, list->window, NULL,
			top, *pos,
			GTLV_Selected, *pos,
			TAG_END);
    }
  }

//  putreg(REG_A4, olda4);
  return(retcode);
}

/***************************************************************************
*
* int Reqlist(int, ...);
*
* Returns the chosen array number or if anything went wrong or the string
* enterred didn't match any entry in the list, a negative number.
*
****************************************************************************

#define MAX_STR 128	/* if no string is given, this will be the default
			   maximum length of the input dest string */
#define REQLIST_WIDTH 24 /* Standard width in characters */
#define REQLIST_HEIGHT 12 /* Standard height in character lines */

#define REQLIST_STRING1 65000
#define REQLIST_STRING2 65001
#define REQLIST_LISTVIEW 65002

#define REQLIST_LISTVIEW_MINHEIGHT 32

#define MAX_CHARS 256

#define reql_SORT    1
#define reql_REPLACE 2
#define reql_REPORT  4
#define reql_SORTCASE 8

int Reqlist(BufStruct *Storage, int data, ...)
{
  struct IntuiMessage *msg;
  struct Window *Project0Wnd;
  struct Gadget *Project0GList = NULL;
  int gadget_margin=RequestFont->tf_YSize>>2;

  ListStruct list={NULL, NULL, NULL, NULL, NULL};
  struct Hook hook;

  struct NewGadget ng;
  struct Gadget	*g, *input1, *input2;
  UWORD offx=10, offy, leftedge=0, topedge=0;
  struct Node *nodes;
  struct MinList GadgetList;
  struct Node *last;
  WindowStruct *win;
  char *remember=NULL;

  int lpress=-1;
  ULONG lsecs=0;
  ULONG lmics=0;

  char **strings;
  int i, top;
  int Height, Width;
  int LineH, LineW;
  int *integers;
  int width=0;	/* desired width */
  int height=0;			/* percentage of view */
  char *title=RetString(STR_SELECT_ITEM);  /* desired title */
  char **array;	     /* store array here */
  int number=0;      /* number of items in the array */
  char *string=NULL;
  char *replace=NULL;
  char *stringname1=NULL, *stringname2=NULL;
  int replen=MAX_CHARS, stringlen=MAX_CHARS;
  char cancel=FALSE; /* set if Reqlist is cancelled in any way */
  ButtonStruct *Buttons=NULL;
  ButtonStruct but;
  char flagi=TRUE;
  char stringno=1;	/* Vilken sträng-requester som ska vara aktiv */

  va_list arglist; /* to get the tags */
  char ctrl=0;	     /* flag variable */

  struct Screen *frontscreen=NULL; /* Om inte FRONTWINDOW finns */

  win=BUF(window->window_pointer)?BUF(window):FRONTWINDOW;

  hook.h_Entry=(HOOKFUNC)HookControl;
  hook.h_SubEntry=NULL;
  hook.h_Data=(APTR)&list;

  list.findflag=FIND_NEAR;

#ifdef DEBUGTEST
  if (DebugOpt)
    FPrintf(Output(), "Reqlist is entered\n");
#endif
  /* check all tags */
  va_start(arglist, data);
  while(data!=REQTAG_END) {
    switch(data) {
    case REQTAG_BUTTON:
      Buttons=(ButtonStruct *)va_arg(arglist, char *);
      break;
    case REQTAG_ARRAY:
      array=(char **)va_arg(arglist, char *);
      number=(int)va_arg(arglist, char *);
      break;
    case REQTAG_STRINGNAME1:
      stringname1=(char *)va_arg(arglist, char *);
      if (stringname1 && !*stringname1)
        stringname1=NULL;
      break;
    case REQTAG_STRINGNAME2:
      stringname2=(char *)va_arg(arglist, char *);
      if (stringname2 && !*stringname2)
        stringname2=NULL;
      break;
    case REQTAG_STRING1:
      string=(char *)va_arg(arglist, char *);
      stringlen=(int)va_arg(arglist, char *);
      break;
    case REQTAG_STRING2:
      replace=(char *)va_arg(arglist, char *);
      replen=(int)va_arg(arglist, char *);
      if (replace)
        ctrl|=reql_REPLACE;
      break;
    case REQTAG_SORT:
      ctrl|=reql_SORT;
      list.findflag=FIND_SORT;
      break;
    case REQTAG_SORT_CASE:
      ctrl|=reql_SORTCASE;
      list.findflag=FIND_SORTCASE;
      break;
    case REQTAG_WIDTH:
      width=(int)va_arg(arglist, char *);
      if(width>0) {
        if (width<REQLIST_WIDTH)
          width=REQLIST_WIDTH;
      }
      break;
    case REQTAG_HEIGHT:
      height=(int)va_arg(arglist, char *);
      break;
    case REQTAG_TITLE:
      title=(char *)va_arg(arglist, char *);
      break;
    case REQTAG_SKIP:
      i=(int)va_arg(arglist, char *);
      while(i-->0) /* eat specified number of arguments */
	va_arg(arglist, char *);
      break;
    }
    data=(int)va_arg(arglist, char *);
  }
  va_end(arglist);

  list.array=array;
  list.number=number;
  list.buts=Buttons;
  list.block=BlockBuffer;
  list.listpos1=-1;
  list.listpos2=-1;
#ifdef REG_A4
  list.a4=getreg(REG_A4);
#endif

  /* set all member of the ReqLStruct: */
  LineH=RequestFont->tf_YSize;
  LineW=requestfontwidth;

  if (win && win->screen_pointer) {
    offx = win->screen_pointer->WBorLeft+5; /* left margin */
    offy = win->screen_pointer->WBorTop + win->screen_pointer->RastPort.TxHeight + 6; /* upper margin */
  }

  /*
   * I adjust the height and width with the given width and height of the
   * font we're currently using. That is not 100% cause the font may be
   * proportional.
   */

  /* Adjust height */


  if(number) {
    Height=LineH*number;
    if(Height>visible_height/2)
      Height=visible_height/2;
    if (Height<REQLIST_LISTVIEW_MINHEIGHT)
      Height=REQLIST_LISTVIEW_MINHEIGHT;
    list.listviewlines=Height/LineH;
    Height+=LineH>>1;
  } else {
    Height=0;
    list.listviewlines=0;
  }

#ifdef DEBUGTEST
  if (DebugOpt)
    FPrintf(Output(), "Reqlist check 2\n");
#endif
  if(number) {
    char **stringarray[4];
    strings=(char **)OwnAllocRemember(&remember, number * sizeof(char *));
    integers=(int *)OwnAllocRemember(&remember, number*sizeof(int));
    nodes=(struct Node *)OwnAllocRemember(&remember, number * sizeof(struct Node));

    if (!strings || !integers || !nodes) {
#ifdef DEBUGTEST
  if (DebugOpt)
    FPrintf(Output(), "Reqlist faild %ld %ld %ld\n", strings, integers, nodes);
#endif
      OwnFreeRemember(&remember);
      return(rl_ERROR);
    }
#ifdef DEBUGTEST
  if (DebugOpt)
    FPrintf(Output(), "Reqlist allocated mem\n");
#endif
    stringarray[0]=strings;
    stringarray[1]=NULL;
    stringarray[2]=(char **)integers;
    stringarray[3]=NULL;

    memcpy(strings, array, number * sizeof(char *));
    list.array=strings;
    for (i=0; i<number; i++)
      integers[i]=i;
    if(ctrl&(reql_SORT|reql_SORTCASE))
      ShellSort(stringarray, 2, 1, number, ctrl&reql_SORTCASE?SORT_NORMAL:SORT_INCASE);

    last=( struct Node * )&GadgetList.mlh_Head;

    /* build a node list */
    {
      register maxwidth=0;
      for(i=0; i<number; i++) {
        nodes[i].ln_Succ=(struct Node *)&nodes[i+1];
        nodes[i].ln_Pred=(struct Node *)last;
        nodes[i].ln_Type=0;
        nodes[i].ln_Pri=0;
        nodes[i].ln_Name=strings[i]; /* already sorted if requested! */
        if (!width) {
          register len=strlen(nodes[i].ln_Name);
          if (maxwidth<len)
            maxwidth=len;
        }
        last=&nodes[i];
      }
      if (!width)
        width=maxwidth+3;
    }
    nodes[i-1].ln_Succ=(struct Node *)&GadgetList.mlh_Tail;
    
    GadgetList.mlh_Head=( struct MinNode * )&nodes[0];
    GadgetList.mlh_Tail=NULL;
    GadgetList.mlh_TailPred=( struct MinNode * )&nodes[number-1];
  } else
    strings=array;

  /* Adjust width */
  if (width<REQLIST_WIDTH)
    width=REQLIST_WIDTH;

  Width=LineW*width;
  if(Width>visible_width-30)
    Width=visible_width-30;

  if (Buttons && BUT(antal)) {
#ifdef DEBUGTEST
  if (DebugOpt)
    FPrintf(Output(), "Reqlist SetBut\n");
#endif
    SetButton(Buttons);
    if (Width<BUT(bredd))
      Width=BUT(bredd);
    FreeGadgets(Buttons->firstgadget);
    Buttons->firstgadget=NULL;
  }
  Width+=offx*2;

  if ( ! ( g = CreateContext( &Project0GList ))) {
#ifdef DEBUGTEST
  if (DebugOpt)
    FPrintf(Output(), "Reqlist faild CreateContext\n");
#endif
    OwnFreeRemember(&remember);
    return( rl_ERROR );
  }

  /* STRING_KIND Gadget: */

  ng.ng_LeftEdge   = offx;
  ng.ng_TopEdge    = Height+offy+(number?gadget_margin:0)+(stringname1?RequestFont->tf_YSize<<1:0);
  ng.ng_Width      = Width-offx*2;
  ng.ng_Height     = LineH+6;
  ng.ng_GadgetText = stringname1;
  ng.ng_TextAttr   = &Default.RequestFontAttr;
  ng.ng_GadgetID   = REQLIST_STRING1;
  ng.ng_Flags      = PLACETEXT_ABOVE;
  if (!win) {
    frontscreen=LockPubScreen(NULL);
    ng.ng_VisualInfo=GetVisualInfo(frontscreen, TAG_END);
  } else
    ng.ng_VisualInfo = win->visualinfo;
  ng.ng_UserData   = NULL; /* whatever we want! */
  
  g=input1=CreateGadget(STRING_KIND, g, &ng,
			GTST_String, string,
                        GA_Immediate, TRUE,
                        GTST_EditHook, (int)&hook,
			GTST_MaxChars, stringlen, TAG_DONE);
#ifdef DEBUGTEST
  if (DebugOpt)
    FPrintf(Output(), "Reqlist Creategadget1 (%ld)\n", g);
#endif
  if (!input1) {
    OwnFreeRemember(&remember);
    return(rl_ERROR);
  }
  list.gadget1=g;
  if (GadToolsBase->lib_Version==37)
    g->Activation|=GACT_IMMEDIATE;

  top = ng.ng_TopEdge + ng.ng_Height + gadget_margin;

  if(number) {

    /* LISTVIEW_KIND Gadget: */

    ng.ng_TopEdge    = offy;
    ng.ng_Height     = Height;
    ng.ng_GadgetID   = REQLIST_LISTVIEW;
    ng.ng_GadgetText = NULL;
    
    g=CreateGadget(LISTVIEW_KIND, g, &ng,
		   GTLV_Labels, (ULONG)&GadgetList,
		   GTLV_Selected, (ULONG)~0, /* none is selected! */
		   SysBase->LibNode.lib_Version<39?TAG_DONE:
		   GTLV_ShowSelected, NULL,
		   TAG_DONE);
#ifdef DEBUGTEST
    if (DebugOpt)
      FPrintf(Output(), "Reqlist Creategadget2 (%ld)\n", g);
#endif
    if (!g) {
      OwnFreeRemember(&remember);
      return(rl_ERROR);
    }
    list.listview=g;
  }
  if(ctrl&reql_REPLACE) {
    /* STRING_KIND Gadget: */

    ng.ng_TopEdge    = top+=(stringname1?RequestFont->tf_YSize<<1:0);
    ng.ng_Height     = LineH+6;
    ng.ng_GadgetID   = REQLIST_STRING2;
    ng.ng_GadgetText = stringname2;
  
    g=input2=CreateGadget(STRING_KIND, g, &ng,
			  GTST_String, replace,
                          GA_Immediate, TRUE,
                          GTST_EditHook, (int)&hook,
			  GTST_MaxChars, replen, TAG_DONE);
#ifdef DEBUGTEST
    if (DebugOpt)
      FPrintf(Output(), "Reqlist Creategadget3 (%ld)\n", g);
#endif
    if (!input2) {
      OwnFreeRemember(&remember);
      return(rl_ERROR);
    }
    list.gadget2=g;
    if (GadToolsBase->lib_Version==37)
      g->Activation|=GACT_IMMEDIATE;

    top += ng.ng_Height + gadget_margin;

  }

  if (!Buttons) {
    Buttons=&but;
    InitButtonStruct(Buttons);
  }
  BUT(gad)=g;
  BUT(firstgadget)= Project0GList;
  BUT(width)=Width;
  BUT(xoffset)=offx;
  BUT(yoffset)=top;
  BUT(OkCancel)=TRUE;
  SetButton(Buttons);

//  if (Width<BUT(bredd))
//    Width=BUT(bredd);
  Height=top+BUT(height);

  if (win && win->screen_pointer) {
    leftedge=TestPosition(visible_width-win->screen_pointer->ViewPort.DxOffset,
                          -win->screen_pointer->ViewPort.DxOffset,
                          Width, win->screen_pointer->MouseX-(Width>>1));
    topedge=TestPosition(visible_height-win->screen_pointer->ViewPort.DyOffset,
                          -win->screen_pointer->ViewPort.DyOffset,
                          Height, win->screen_pointer->MouseY-(Height>>1));
  }

  Project0Wnd = OpenWindowTags(NULL,
			       WA_Left,		leftedge,
			       WA_Top,		topedge,
			       WA_Width,	Width,
			       WA_Height,	Height,
			       WA_IDCMP,	LISTVIEWIDCMP|STRINGIDCMP|IDCMP_ACTIVEWINDOW|
				 		BUTTONIDCMP|IDCMP_CLOSEWINDOW|IDCMP_GADGETDOWN|
			       			IDCMP_RAWKEY|IDCMP_CHANGEWINDOW,
			       WA_Flags,	WFLG_DRAGBAR|WFLG_DEPTHGADGET|
				 		WFLG_CLOSEGADGET|WFLG_ACTIVATE|
				 		WFLG_SMART_REFRESH,
			       WA_Gadgets,	Project0GList,
			       WA_Title,	title,
			       WA_AutoAdjust,	TRUE,
	      (!win || !win->screen_pointer?TAG_IGNORE:(win->window!=FX_SCREEN?WA_PubScreen:WA_CustomScreen)), win?win->screen_pointer:NULL,
			       TAG_DONE );
#ifdef DEBUGTEST
  if (DebugOpt)
    FPrintf(Output(), "Reqlist OpenWindow (%ld)\n", Project0Wnd);
#endif
  if(!Project0Wnd) {
    OwnFreeRemember(&remember);
    return(rl_ERROR);
  }
  if (Buttons)
    BUT(winbutton)=Project0Wnd;
  list.window=Project0Wnd;

  GT_RefreshWindow( Project0Wnd, NULL );

  FreezeEditor(1);
  LockBuf_release_semaphore(BUF(shared));

  while(flagi) {
    int setcur;
    Wait(1L << Project0Wnd->UserPort->mp_SigBit);
    while(msg=(struct IntuiMessage *)GT_GetIMsg(Project0Wnd->UserPort)) {
      int id;
      setcur=TRUE;
      switch(msg->Class) {
      case GADGETUP:
	switch( id=GetGadgetID(msg) ) {
	case Ok_button_ID:  /* OKAY button */
	  flagi = FALSE;
	  break;
	case Cancel_button_ID:  /* CANCEL button */
	  flagi = FALSE;
	  cancel= TRUE;
	  break;
	case REQLIST_STRING1: /* upper string gadget */
	  if (msg->Code==9 && (ctrl&reql_REPLACE)) {
	    stringno=2;
          } else
            stringno=0;
	  break;
	case REQLIST_STRING2: /* lower string gadget */
	  if (msg->Code==9) {
	    stringno=1;
          } else
            stringno=0;
	  break;
	case REQLIST_LISTVIEW: /* listview click! */
	  if(msg->Code==lpress &&
	     DoubleClick(lsecs, lmics, msg->Seconds, msg->Micros))
	    flagi=FALSE;
	  else { //if (stringno) 941211
	    lsecs=msg->Seconds;
	    lmics=msg->Micros;
	    lpress=msg->Code;
	    GT_SetGadgetAttrs((stringno==2)?input2:input1,
			      Project0Wnd,
			      NULL,
			      GTST_String, strings[msg->Code],
			      TAG_DONE);
	  }
	  break;
	default:
          if (BUT(array[id].cycletext)) {
            GT_SetGadgetAttrs(msg->IAddress, BUT(winbutton), NULL, GTCY_Active, msg->Code, TAG_END);
            BUT(array[id].flags)=msg->Code;
          }
          break;
	}
	break;
      case GADGETDOWN:
	switch (GetGadgetID(msg)) {
	case REQLIST_STRING1: /* upper string gadget */
          stringno=1;
	  break;
	case REQLIST_STRING2: /* lower string gadget */
          stringno=2;
	  break;
	}
	break;
      case RAWKEY:
        setcur=FALSE;
        ievent.ie_Code=msg->Code;
        ievent.ie_Qualifier=msg->Qualifier;
        ievent.ie_EventAddress= *((APTR *)msg->IAddress);
        if (RawKeyConvert(&ievent, buffer, 10, internalkeymap)==1) {
          if (msg->Qualifier & RAW_AMIGA) {
            register struct Gadget *gadget=CheckHotKey(Buttons, buffer[0]);
            if (gadget) {
              GT_SetGadgetAttrs(gadget,
                                Buttons->winbutton,
                                NULL,
                                GTCB_Checked, (gadget->Flags & SELECTED)?FALSE:TRUE,
                                TAG_END);
            }
          } else {
            switch(buffer[0]) {
            case '\x1b':
              flagi=FALSE;
              cancel=TRUE;
              break;
            case '\r':
            case '\n':
              flagi=FALSE;
              break;
            case '\t':
              setcur=TRUE;
              stringno=1;
              break;
            }
          }
        }
        break;
      case CLOSEWINDOW:
	flagi=FALSE;
	cancel=TRUE;
	break;
      }
      if (setcur && flagi && stringno) {
        ActivateGadget((stringno==1)?input1:input2, Project0Wnd, NULL);
      }
      GT_ReplyIMsg(msg);
    }
  }
  UnLockBuf_obtain_semaphore(BUF(shared));

  if(cancel) {
    /* if it was cancelled, no string should be returned */
    if(string)
      string[0]='\0';
    if(replace)
      replace[0]='\0';
  } else {
    Width=rl_NOMATCH;
    if(string)
      stccpy(string, GetString(input1), stringlen);
    if(replace)
      stccpy(replace, GetString(input2), replen);

    {
      register char *stringpoint=GetString(input1);
      for(i=0; i<number; i++) {
        if(!strcmp(array[i], stringpoint)) {  /* search in the un-sorted list */
	  Width=i;
	  break;
        }
      }
    }
  }
  
  if(number) {
    OwnFreeRemember(&remember);
  }
  ActivateEditor();
  ReadButtons(Buttons);
  CloseWindow(Project0Wnd);
  FreeGadgets(Project0GList);

#ifdef DEBUGTEST
  if (DebugOpt)
    FPrintf(Output(), "Reqlist exit\n");
#endif

  if (frontscreen) {
    FreeVisualInfo(ng.ng_VisualInfo);
    UnlockPubScreen(NULL, frontscreen);
  }

  return(cancel?rl_CANCEL:Width /* entry*/ );
}


/* Testar om 'wanted' med 'width'-bredden passar inom max/min värdena.
   Returnerar värdet du ska använda */

int __regargs TestPosition(int max, int min, int width, int wanted)
{
  if (wanted<min)
    wanted=min;
  else if (wanted+width>max)
    wanted=max-width;

  return(wanted);
}  


