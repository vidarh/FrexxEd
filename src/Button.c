/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Button.c
 *
 * Buttonpress functions.
 *
 *********/

#ifdef AMIGA
#include <clib/gadtools_protos.h>
#include <devices/console.h>
#include <exec/execbase.h>
#include <exec/io.h>
#include <exec/types.h>
#include <fpl/reference.h>
#include <graphics/gfxbase.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/FPL.h>
#include <libraries/gadtools.h>
#include <proto/exec.h>
#include <proto/FPL.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/reqtools.h>
#include <utility/tagitem.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "BufControl.h"
#include "Button.h"
#include "Command.h"
#include "Edit.h"
#include "Execute.h"
#include "Limit.h"
#include "RawKeys.h"
#include "Reqlist.h"
#include "Request.h"
#include "Setting.h"

extern struct ExecBase *SysBase;
extern struct GfxBase *GfxBase;
extern struct IntuitionBase *IntuitionBase;
extern struct ConsoleDevice *ConsoleDevice;
extern struct Library *GadToolsBase;
extern struct MsgPort *ReadPort;
extern DefaultStruct Default;
extern int BarHeight;
extern struct TextFont *RequestFont;
extern struct RastPort ButtonRastPort;	//Rastport för att kunna testa med Requestfonten.
extern struct TextFont *RequestFont;
extern int requestfontwidth;		// Kalkulerad bredd på requestfonten.
extern int visible_height;
extern int visible_width;
extern void *Anchor;
extern struct UserData userdata;
extern struct Library *FPLBase;
extern char **setting_string_pointer;
extern int *setting_int_pointer;
extern struct ReqToolsBase *ReqToolsBase;

/****************
*
* int ButtonPress(BufStruct *, ButtonStruct *)
*
* Returns -1 om det blev fel eller cancelled.
*
****/

int __regargs ButtonPress(BufStruct *Storage, ButtonStruct *Buttons)
{
  int ret=0;
  char flagi=TRUE;
  struct IntuiMessage *msg;
  struct Gadget *pressed;
  long fplret;
  char *string;

  FreezeEditor(1);
  if(OpenButton(Buttons)==-1) {
    ActivateEditor();
    return(-1);
  }
  LockBuf_release_semaphore(BUF(shared));
  while(flagi) {
    Wait(1L << BUT(winbutton->UserPort->mp_SigBit));
    while(msg=(struct IntuiMessage *)GT_GetIMsg(BUT(winbutton->UserPort))) {
      switch(msg->Class) {
      case IDCMP_RAWKEY:
	switch(msg->Code) {
	case RAW_ESC:
	  ret=-1;
	case RAW_RETURN:
	  flagi=FALSE;
          break;
	case RAW_TAB:
          if (BUT(firststring))
            ActivateGadget(BUT(firststring), BUT(winbutton), NULL);
          break;
	default:
	  break;
	}
        break;
      case IDCMP_CLOSEWINDOW:
        flagi=FALSE;
        ret=-1;
	break;
      case IDCMP_GADGETUP:
        pressed=(struct Gadget *) (msg->IAddress);
        if (pressed->GadgetID==Ok_button_ID)
          flagi=FALSE;
        else if (pressed->GadgetID==Cancel_button_ID) {
          flagi=FALSE;
          ret=-1;
        } else if (pressed->GadgetID==FPL_ADDITION_GADGET_ID) {
          char *program=Strdup(BUT(array[(int)pressed->UserData]).FPLaddition);
          BufStruct *lockbuf=userdata.buf;
          int type=ST_BOOL;
          setting_string_pointer=NULL;
          setting_int_pointer=NULL;
          if (BUT(array[(int)pressed->UserData].types))
           type=BUT(array[(int)pressed->UserData].types)&(15);
          BUF(locked)++;
          SHS(current)++;
          lockbuf->locked++;
          lockbuf->shared->current++;
          if (type==ST_STRING) {
            setting_string_pointer=&((struct StringInfo*)BUT(array[(int)pressed->UserData].gadget->SpecialInfo))->Buffer;
            string=NULL;
            {
              register APTR windowlock;
              windowlock=rtLockWindow(BUT(winbutton));
              UnLockBuf_obtain_semaphore(BUF(shared));
              fplret=fplExecuteScriptTags(Anchor, &program, 1,
                                      FPLTAG_CACHEFILE, FPLCACHE_NONE,
                                      FPLTAG_STRING_RETURN, &string,
                                      FPLTAG_PROGNAME, EXECUTED_BUTTON,
                                      FPLTAG_USERDATA, &userdata, FPLTAG_END);
              LockBuf_release_semaphore(BUF(shared));
              rtUnlockWindow(BUT(winbutton), windowlock);
            }
            if (fplret>=OK) {
              if (string) {
                GT_SetGadgetAttrs(BUT(array[(int)pressed->UserData].gadget), BUT(winbutton), NULL,
                                  GTST_String, string,
                                  TAG_END);
                fplFreeString(Anchor, string);
              }
            }
          } else {
            long holder;
            setting_int_pointer=(int *)&holder;
            switch (type) {
            case ST_BOOL:
              GT_GetGadgetAttrs(BUT(array[(int)pressed->UserData].gadget), BUT(winbutton), NULL, GTCB_Checked, &holder, TAG_END);
              break;
            case ST_STRING:
              GT_GetGadgetAttrs(BUT(array[(int)pressed->UserData].gadget), BUT(winbutton), NULL,
                                GTCY_Active, &holder,
                                TAG_END);
              break;
            case ST_NUM:
              GT_GetGadgetAttrs(BUT(array[(int)pressed->UserData].gadget), BUT(winbutton), NULL,
                                GTIN_Number, &holder,
                                TAG_END);

              break;
            case ST_CYCLE:
              GT_GetGadgetAttrs(BUT(array[(int)pressed->UserData].gadget), BUT(winbutton), NULL, GTCY_Active, &holder, TAG_END);
              break;
            }
            {
              register APTR windowlock;
              windowlock=rtLockWindow(BUT(winbutton));
              fplret=ExecuteFPL(Storage, program, FALSE, NULL, EXECUTED_BUTTON);
              rtUnlockWindow(BUT(winbutton), windowlock);
            }
            if (fplret>=OK) {
              fplSendTags(Anchor, FPLSEND_GETRETURNINT, &fplret, FPLSEND_DONE);
              if (fplret) {
                switch (type) {
                case ST_BOOL:
                  GT_SetGadgetAttrs(BUT(array[(int)pressed->UserData].gadget), BUT(winbutton), NULL, GTCB_Checked, *(long *)fplret, TAG_END);
                  break;
                case ST_STRING:
                  GT_SetGadgetAttrs(BUT(array[(int)pressed->UserData].gadget), BUT(winbutton), NULL,
                                    GTCY_Active, *(long *)fplret,
                                    TAG_END);
                  break;
                case ST_NUM:
                  GT_SetGadgetAttrs(BUT(array[(int)pressed->UserData].gadget), BUT(winbutton), NULL,
                                    GTIN_Number, *(long *)fplret,
                                    TAG_END);
  
                  break;
                case ST_CYCLE:
                  GT_SetGadgetAttrs(BUT(array[(int)pressed->UserData].gadget), BUT(winbutton), NULL, GTCY_Active, *(long *)fplret, TAG_END);
                  BUT(array[(int)pressed->UserData].flags)=*(long *)fplret;
                  break;
                }
              }
            }
          }
          userdata.buf=lockbuf;
          BUF(locked)--;
          SHS(current)--;
          lockbuf->locked--;
          lockbuf->shared->current--;
          Dealloc(program);
          setting_string_pointer=NULL;
          setting_int_pointer=NULL;
        } else if (BUT(array[pressed->GadgetID].cycletext)) {
          GT_SetGadgetAttrs(pressed, BUT(winbutton), NULL, GTCY_Active, msg->Code, TAG_END);
          BUT(array[pressed->GadgetID].flags)=msg->Code;
        }
	break;
      default:
	break;
      }
      GT_ReplyIMsg(msg);
    }
  }
  CloseButton(Buttons);
  UnLockBuf_obtain_semaphore(BUF(shared));
  ActivateEditor();
  return(ret);
}

void __regargs CloseButton(ButtonStruct *Buttons)
{
  ReadButtons(Buttons);
  CloseWindow(BUT(winbutton));
  FreeGadgets(BUT(firstgadget));
}

int __regargs OpenButton(ButtonStruct *Buttons)
{
  struct NewWindow buttonwin = {
    0, 0, 0, 0, 0, 1,
    IDCMP_GADGETDOWN | IDCMP_GADGETUP | IDCMP_CLOSEWINDOW | IDCMP_RAWKEY,
    WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_NOCAREREFRESH | WFLG_SMART_REFRESH | WFLG_ACTIVATE,
    NULL, NULL,
    NULL, NULL, NULL,
    0, 0, 0, 0, WBENCHSCREEN
  };

  BUT(yoffset)=BarHeight+6;
  if (FRONTWINDOW)
    BUT(yoffset)=(BarHeight?BarHeight:(FRONTWINDOW->screen_pointer->BarHeight?FRONTWINDOW->screen_pointer->BarHeight:FRONTWINDOW->screen_pointer->Font->ta_YSize))+6;
  BUT(xoffset)=14;
  BUT(OkCancel)=TRUE;

  if (!SetButton(Buttons))
    return(-1);

  buttonwin.Height=BUT(height)+BUT(yoffset);
  buttonwin.Width=BUT(bredd)+38;
  if (FRONTWINDOW) {
    buttonwin.Type=CUSTOMSCREEN;
    buttonwin.Screen=FRONTWINDOW->screen_pointer;
    buttonwin.LeftEdge=TestPosition(visible_width-FRONTWINDOW->screen_pointer->ViewPort.DxOffset,
                          -FRONTWINDOW->screen_pointer->ViewPort.DxOffset,
                          buttonwin.Width, FRONTWINDOW->screen_pointer->MouseX-(buttonwin.Width>>1));
    buttonwin.TopEdge=TestPosition(visible_height-FRONTWINDOW->screen_pointer->ViewPort.DyOffset,
                          -FRONTWINDOW->screen_pointer->ViewPort.DyOffset,
                          buttonwin.Height, FRONTWINDOW->screen_pointer->MouseY-(buttonwin.Height>>1));
  }
  buttonwin.Title=(UBYTE *)BUT(toptext);

  buttonwin.FirstGadget=BUT(firstgadget);
  if(!(BUT(winbutton)=OpenWindowTags(&buttonwin, WA_AutoAdjust, TRUE, TAG_END)))
    return(-1);
  return(0);
}


int __regargs SetButton(ButtonStruct *Buttons)
{
  int count;
  int antalstring;
  int spalter=1;
  int radcount=0, spaltcount=0;
  int bredd, breddcount=0, stringbredd;
  int gadgetcount=0;
  BOOL nostrings=TRUE;
  struct NewGadget newgad;
  struct Screen *frontscreen=NULL;

  SetFont(&ButtonRastPort, RequestFont);

  if (FRONTWINDOW)
    newgad.ng_VisualInfo=FRONTWINDOW->visualinfo;
  else {
    frontscreen=LockPubScreen(NULL);
    newgad.ng_VisualInfo=GetVisualInfo(frontscreen, TAG_END);
  }
  newgad.ng_TextAttr=&Default.RequestFontAttr;

  if (!BUT(firstgadget))
    BUT(gad)=CreateContext(&BUT(firstgadget));

  antalstring=BUT(antal);

  /* Checka om det bara är knappar i listan, isåfall ska de högerjusteras */
  {
    register int counter=0;
    while (counter<BUT(antal)) {
      if (BUT(array[counter].types) && 
          BUT(array[counter].types)!=ST_BOOL) {
        nostrings=FALSE;
        if (BUT(array[counter].types)==ST_CYCLE) {
          register int count2=0;
          while (BUT(array[counter].cycletext[count2])) {
            register int len;
            len=TextLength(&ButtonRastPort, BUT(array[counter].cycletext[count2]),
                           strlen(BUT(array[counter].cycletext[count2])));
            len=(len+requestfontwidth-1)/requestfontwidth+3;
            if (len>BUT(string_width))
              BUT(string_width)=len;
            count2++;
          }
        }
      }
      counter++;
    }
  }

  while ((antalstring*(RequestFont->tf_YSize+(RequestFont->tf_YSize>>1)))>BUT(maxheight)) {
    spalter++;
    antalstring=(BUT(antal)+spalter-1)/spalter;
  }
  BUT(antalrader)=antalstring;

  bredd=0;
  count=0;
  while(count<BUT(antal)) {
    {
      register char *pos=strchr(BUT(array[count].name), '@');
      BUT(array[count].shortcut)=0;
      if (pos)
        BUT(array[count].shortcut)=pos[1];
    }

    newgad.ng_TopEdge=(WORD)((RequestFont->tf_YSize+(max(4,RequestFont->tf_YSize>>1)))*radcount+BUT(yoffset));
    newgad.ng_UserData=NULL;
    newgad.ng_GadgetText=BUT(array[count].name);
    stringbredd=TextLength(&ButtonRastPort, BUT(array[count].name),
                           strlen(BUT(array[count].name)));
    newgad.ng_Height=RequestFont->tf_YSize+4;

    if (!(BUT(array[count].typesfull&ST_READ)) && BUT(array[count].FPLaddition) && BUT(array[count].FPLaddition[0])) {
      stringbredd+=6;
      newgad.ng_UserData=(void *)count;
      newgad.ng_Width=stringbredd;
      newgad.ng_LeftEdge=(WORD)(breddcount+spaltcount*8+BUT(xoffset)+requestfontwidth*BUT(string_width))+8;
      stringbredd+=8;
      newgad.ng_GadgetID=(UWORD)FPL_ADDITION_GADGET_ID;
      newgad.ng_Flags=PLACETEXT_IN;
      BUT(gad)=CreateGadget(BUTTON_KIND, BUT(gad), &newgad, TAG_END);
      BUT(array[gadgetcount].additiongadget)=BUT(gad);
      newgad.ng_GadgetText="";
    }
    newgad.ng_Width=BUT(string_width)*requestfontwidth;
    stringbredd+=newgad.ng_Width;
    newgad.ng_LeftEdge=(WORD)(breddcount+spaltcount*8+BUT(xoffset));
    newgad.ng_GadgetID=(UWORD)count;
    newgad.ng_Flags=PLACETEXT_RIGHT;

    if (stringbredd>bredd) {
      bredd=stringbredd;
    }
    if (BUT(array[count].types)) {
      if (BUT(array[count].types)==ST_STRING) {
        BUT(gad)=CreateGadget(STRING_KIND, BUT(gad), &newgad,
                              GTST_String, (char *)BUT(array[count].flags),
                              GTST_MaxChars, 256,
                              STRINGA_Justification, GACT_STRINGRIGHT,
                              GA_TabCycle, TRUE,
                              GT_Underscore, '@',
                              GA_Disabled, (BUT(array[count].typesfull&ST_READ)),
                              TAG_END);
        if (!BUT(firststring))
          BUT(firststring)=BUT(gad);
      } else if (BUT(array[count].types)==ST_CYCLE) {
        BUT(gad)=CreateGadget(CYCLE_KIND, BUT(gad), &newgad,
                              GTCY_Labels, BUT(array[count].cycletext),
                              GTCY_Active, BUT(array[count].flags),
                              GT_Underscore, '@',
                              GA_Disabled, (BUT(array[count].typesfull&ST_READ)),
                              TAG_END);
      } else if (BUT(array[count].types)==ST_NUM) {
        BUT(gad)=CreateGadget(INTEGER_KIND, BUT(gad), &newgad,
                              GTIN_Number, BUT(array[count].flags),
                              STRINGA_Justification, GACT_STRINGRIGHT,
                              GA_TabCycle, TRUE,
                              GT_Underscore, '@',
                              GA_Disabled, (BUT(array[count].typesfull&ST_READ)),
                              TAG_END);
        if (!BUT(firststring))
          BUT(firststring)=BUT(gad);
      }
    }
    if (!BUT(array[count].types) || BUT(array[count].types)==ST_BOOL) {
      newgad.ng_Height--;
      if (SysBase->LibNode.lib_Version >= 39)
        newgad.ng_Width=requestfontwidth*3;//2+4;
      else
        newgad.ng_Width=26;
      if (!nostrings)
        newgad.ng_LeftEdge+=requestfontwidth*BUT(string_width)-newgad.ng_Width;
      BUT(gad)=CreateGadget(CHECKBOX_KIND, BUT(gad), &newgad,
                            GTCB_Checked, BUT(array[count].flags),
                            GTCB_Scaled, TRUE,
                            GA_Disabled, (BUT(array[count].typesfull&ST_READ)),
                            GT_Underscore, '@', TAG_END);
    }
    BUT(array[gadgetcount].gadget)=BUT(gad);
    gadgetcount++;
    count++;
    radcount++;
    if (radcount==BUT(antalrader)) {
      if (nostrings)
        bredd-=4*requestfontwidth;
      radcount=0;
      spaltcount++;
      breddcount+=bredd+requestfontwidth;
      bredd=0;
    }
  }
  if (radcount!=BUT(antalrader))
    breddcount+=bredd+(nostrings?-6:2);

  BUT(bredd)=breddcount;
  BUT(height)=BUT(antalrader)*(RequestFont->tf_YSize+(max(4,RequestFont->tf_YSize>>1)));

  if (BUT(OkCancel)) {
    newgad.ng_TopEdge=(WORD)(BUT(height)+BUT(yoffset));
    newgad.ng_LeftEdge=(WORD)BUT(xoffset);
    newgad.ng_Width=requestfontwidth*9+2;
    newgad.ng_Height=RequestFont->tf_YSize+6;
    newgad.ng_GadgetID=(UWORD)Ok_button_ID;
    newgad.ng_GadgetText=RetString(STR_OK_GADGET);
    newgad.ng_Flags=PLACETEXT_IN;
    newgad.ng_UserData=NULL;
    BUT(height)+=newgad.ng_Height;
    BUT(gad)=CreateGadget(BUTTON_KIND, BUT(gad), &newgad, TAG_END);

    if (BUT(bredd)<20*(requestfontwidth)+BUT(xoffset))
      BUT(bredd)=20*(requestfontwidth)+BUT(xoffset);

    if (BUT(width))
      newgad.ng_LeftEdge=BUT(width)-20;
    else
      newgad.ng_LeftEdge=BUT(bredd);

    newgad.ng_LeftEdge+=-9*(requestfontwidth)+BUT(xoffset);
    newgad.ng_GadgetID=(UWORD)Cancel_button_ID;
    newgad.ng_GadgetText=RetString(STR_CANCEL_GADGET);
    BUT(gad)=CreateGadget(BUTTON_KIND, BUT(gad), &newgad, TAG_END);
    BUT(antalrader)++;
  }
  BUT(height)+=4;
  if (frontscreen) {
    FreeVisualInfo(newgad.ng_VisualInfo);
    UnlockPubScreen(NULL, frontscreen);
  }

  return(TRUE);
}

void __regargs ReadButtons(ButtonStruct *Buttons)
{
  int count;
  char *slask;
  struct Gadget *gadget;
  for (count=0; count<BUT(antal); count++) {
    gadget=BUT(array[count].gadget);
    if (!BUT(array[count].types) ||
        BUT(array[count].types)==ST_BOOL) {
      if (gadget->Flags & GFLG_SELECTED)
        BUT(array[count].flags)=1;
      else
        BUT(array[count].flags)=0;
    } else if (BUT(array[count].types)==ST_STRING) {
      if (strcmp((char *)BUT(array[count].flags), ((struct StringInfo*)gadget->SpecialInfo)->Buffer)) {
        Dealloc((char *)BUT(array[count].flags));
        BUT(array[count].flags)=(int)Strdup(((struct StringInfo*)gadget->SpecialInfo)->Buffer);
      }
    } else if (BUT(array[count].types)==ST_NUM) {
      BUT(array[count].flags=fplStrtol(((struct StringInfo*)gadget->SpecialInfo)->Buffer, 10, &slask));
    }
  }
}

void __regargs InitButtonStruct(ButtonStruct *Buttons)
{
  memset(Buttons, 0, sizeof(ButtonStruct));
  BUT(maxheight)=visible_height-30;
  BUT(toptext)=RetString(STR_PRESS_BUTTONS);
  BUT(string_width)=12;
}

struct Gadget __regargs *CheckHotKey(ButtonStruct *buts, char code)
{
  register int counter;
  code=TOLOWER(code);
  for (counter=0; counter<buts->antal; counter++) {
    if (code==TOLOWER(buts->array[counter].shortcut)) {
      return(buts->array[counter].gadget);
    }
  }
  return(NULL);
}

