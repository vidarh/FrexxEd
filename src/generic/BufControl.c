/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * BufControl.c
 *
 * Control routines for BufStruct
 *
 *********/

#ifdef AMIGA
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/utility.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Buf.h"
#include "Alloc.h"
#include "BufControl.h"
#include "Command.h"
#include "Cursor.h"
#include "Face.h"
#include "FACT.h"
#include "GetFile.h"
#include "Hook.h"
#include "Init.h"
#include "Limit.h"
#include "OpenClose.h"
#include "Reqlist.h"
#include "Request.h"
#include "Setting.h"
#include "Slider.h"
#include "Undo.h"
#include "UpdtBlock.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"
#include "WindowOutput.h"

#include <assert.h>

/********** Globals **********/

extern int errno;

#ifdef AMIGA
extern char *sys_errlist[];
#endif

extern char GlobalEmptyString[];
extern FACT *UsingFact;
extern int antalsets;
extern struct Setting **sets;
extern struct IntuitionBase *IntuitionBase;
extern struct Gadget VertSlider;
extern struct PropInfo VertSliderSInfo;
extern struct Image SliderImage;
extern int Visible;
extern BlockStruct *BlockBuffer;

extern struct TextFont *SystemFont;
extern struct TextFont *RequestFont;
extern char *statusbuffer;
extern long statusbuffer_len;

extern char buffer[];	/* temp buffer */
extern int bufferlen;
extern DefaultStruct Default; /* the default values of a BufStruct */
extern FACT *DefaultFact;

extern BOOL quitting;
extern BOOL device_has_killed_a_buffer;
extern struct SignalSemaphore LockSemaphore;

extern int semaphore_count;

/*** PRIVATE ***/

static int buffer_number=0;
static int editmax = 0;	/* how many buffers we have */

static BufStruct *DeleteEntry(BufStruct *Storage, BOOL returnwanted);
static void FreeLokalInfo(SharedStruct *shared);


/***********************************************
 *
 *  NextShowBuf( BufStruct *)
 *  PrevShowBuf( BufStruct *)
 *  NextBuf( BufStruct *)
 *  NextBuf( BufStruct *)
 *  NextHiddenBuf( BufStruct *)
 *  PrevHiddenBuf( BufStruct *)
 *
 *  Returns the pointer for the next/prev shown/not shown buffer.
 *  Will not activate it!!!
 *  Input the buffer pointer.
 *
 ***********/
static BufStruct *NextView(BufStruct *Storage)
{
  BufStruct *Storage2;

  Storage2=BUF(NextShowBuf);
  if (!Storage2) {
    if (BUF(window))
      Storage2=BUF(window->NextShowBuf);
    if (!Storage2 && FRONTWINDOW)
      Storage2=FRONTWINDOW->NextShowBuf;
  }
  return(Storage2);
}

BufStruct *PrevView(BufStruct *Storage)
{
  BufStruct *Storage2;
  Storage2=BUF(PrevShowBuf);
  if (!Storage2) {
    if (BUF(window))
      Storage2=BUF(window->NextShowBuf);
    if (!Storage2 && FRONTWINDOW)
      Storage2=FRONTWINDOW->NextShowBuf;
    while(Storage2 && Storage2->NextShowBuf)
      Storage2=Storage2->NextShowBuf;
  }
  return(Storage2);
}

static BufStruct *NextHiddenEntry(BufStruct *Storage)
{
  BufStruct *Storage2;
  Storage2=Storage;
  do {
    if (Storage2)
      Storage2=Storage2->NextBuf;
    if (!Storage2)
      Storage2=Default.BufStructDefault.NextBuf;
  } while (Storage2!=Storage && (Storage2->window));
  return(Storage2);
}
static BufStruct *PrevHiddenEntry(BufStruct *Storage)
{
  BufStruct *Storage2;
  Storage2=Storage;
  do {
    if (Storage2!=&Default.BufStructDefault)
      Storage2=Storage2->PrevBuf;
    if (Storage2==&Default.BufStructDefault)
      while(Storage2->NextBuf)
        Storage2=Storage2->NextBuf;
  } while (Storage2!=Storage && (Storage2->window));
  return(Storage2);
}

static BufStruct *NextEntry(BufStruct *Storage)
{
  BufStruct *Storage2;
  Storage2=BUF(NextBuf);
  if (!Storage2)
    Storage2=Default.BufStructDefault.NextBuf;
  return(Storage2);
}

static BufStruct *PrevEntry(BufStruct *Storage)
{
  BufStruct *Storage2;
  Storage2=Storage;
  if (Storage2!=&Default.BufStructDefault)
    Storage2=Storage2->PrevBuf;
  if (Storage2==&Default.BufStructDefault)
    while(Storage2->NextBuf)
      Storage2=Storage2->NextBuf;
  return(Storage2);
}

static BufStruct *NextBuf(BufStruct *Storage)
{
  SharedStruct *Shared2=BUF(shared);
  if (Shared2)
    Shared2=Shared2->Next;
  if (!Shared2)
    Shared2=Default.SharedDefault.Next;
  return(Shared2->Entry);
}

static BufStruct *PrevBuf(BufStruct *Storage)
{
  SharedStruct *Shared2=BUF(shared);
  if (Shared2)
    Shared2=Shared2->Prev;
  if (Shared2==&Default.SharedDefault)
    while (Shared2->Next)
      Shared2=Shared2->Next;
  return(Shared2->Entry);
}

void BufLimits(BufStruct *Storage)
{
  BUF(screen_col)=BUF(window)->window_col;
  BUF(left_offset)=BUF(window)->window_left_offset;
}

/***********************************************
 *
 *    Activate(BufStruct *, BufStruct *, int)
 *
 * Activate chosen buffer.
 * Input: current buffer, wanted buffer.
 * Return the active BufStruct pointer.
 *
 ***************/
BufStruct *Activate(BufStruct *Storage, BufStruct *Storage3, int flag)
{
  BufStruct *Storage2, *Storage4;
  int tempvisible=Visible;
  WindowStruct *win=FRONTWINDOW;

  if (!FRONTWINDOW) // No window available.  Return TRUE
    return Storage3;

  assert(Storage3> 1024);
  assert(Storage3->window > 1024);

  if (Storage3->window) {
    TestCursorPos(Storage3);
    FixMarg(Storage3);
    MoveSlider(Storage3);
    return(Storage3);
  }

  Storage2=Default.BufStructDefault.NextBuf;
  while (Storage2) {                       /* Finns buffen i listan */
    if (Storage2==Storage3) {
      Storage2->update=UD_NOTHING;        
      if (!BUF(window))
        flag=acFULL_SIZE;
      if (flag == acFULL_SIZE || (!Storage)) {
                                                 /* Full Size Buf */
        if (Storage && BUF(window))
          win=BUF(window);
					  /* Töm alla andra buffar */
        Storage2=win->NextShowBuf;
        if (Storage3->window)
          Storage2=Storage3->window->NextShowBuf;
        Storage3->window=Storage2->window;
        BufLimits(Storage3);
        if (Storage2) {
          Storage4=Storage2;
          while(Storage4) {
            Storage2=Storage4;
            Storage4=Storage4->NextShowBuf;
            RemoveBufGadget(Storage2);
            Storage2->NextShowBuf=0;
            Storage2->PrevShowBuf=0;
            Storage2->window=0;
          }
        }
        Storage3->window->NextShowBuf=Storage3;
        Storage3->NextShowBuf=0;
        Storage3->PrevShowBuf=0;
        Storage3->screen_lines=win->window_lines; // Ska in i fönsterstructen
        Storage3->screen_col=win->window_col;
        Storage3->top_offset=1;
        Storage3->window->Views=1;	// Set number of views
      } else if (flag == acREPLACE || (Storage->screen_lines<3)) {
activate_replace: //tillkallas om minnet är slut (för acNEW_WINDOW)
        Storage3->NextShowBuf=BUF(NextShowBuf);
        Storage3->PrevShowBuf=BUF(PrevShowBuf);
        Storage3->window=BUF(window);
        BufLimits(Storage3);
        if (BUF(NextShowBuf)) {
          BUF(NextShowBuf)->PrevShowBuf=Storage3;
        }
        if (BUF(PrevShowBuf)) {
          BUF(PrevShowBuf)->NextShowBuf=Storage3;
        } else
          Storage3->window->NextShowBuf=Storage3;

        BUF(NextShowBuf)=0;
        BUF(PrevShowBuf)=0;
        Storage3->screen_lines=BUF(screen_lines);
        Storage3->screen_col=BUF(screen_col);
        Storage3->top_offset=BUF(top_offset);
        Storage3->slide=BUF(slide);
        RemoveBufGadget(Storage);
        AddBufGadget(Storage3);
        MoveSlider(Storage3);
        Visible=VISIBLE_OFF;
        TestCursorPos(Storage3);
        Visible=tempvisible;
        UpdateScreen(Storage3);
        FixMarg(Storage3);
        Showstat(Storage);
        Storage3->namedisplayed=FALSE;
        Showstat(Storage3);
        BUF(window)=0;
        return(Storage3);
      } else if (flag==acHALF_SIZE) {/* Half Size Buf */
        Storage3->window=BUF(window);
        BufLimits(Storage3);
        Storage3->screen_lines=(BUF(screen_lines))/2;
        BUF(screen_lines)=(BUF(screen_lines)+1)/2-1;
        Storage3->top_offset=BUF(top_offset)+BUF(screen_lines)+1;
        Storage3->screen_col=BUF(screen_col);

        Storage3->PrevShowBuf=BUF(PrevShowBuf);
        BUF(PrevShowBuf)=Storage3;
        Storage3->NextShowBuf=Storage;
        Storage2=Storage3->PrevShowBuf;
        if (Storage2)
          Storage2->NextShowBuf=Storage3;
        else
          Storage3->window->NextShowBuf=Storage3;
        CenterScreen(Storage);
        Visible=VISIBLE_OFF;
        TestCursorPos(Storage);
        Visible=tempvisible;
        UpdateScreen(Storage);
        InitSlider(Storage);
        BUF(window->Views)++;	// Increase number of views
      } else if (flag==acNEW_WINDOW) {/* New Window */
        Storage3->window=MakeNewWindow(BUF(window));
        if (Storage3->window) {
          AttachBufToWindow(Storage3->window, Storage3);
          Storage3->window->iconify=FALSE;
          Storage3->screen_lines=Storage3->window->window_lines;
          Storage3->screen_col=Storage3->window->window_col;
          Storage3->top_offset=1;
          Storage3->window->Views=1;	// Set number of views
          if (OpenUpScreen(Storage3->window)) {
            BufLimits(Storage3);
          } else {
            FreeWindow(Storage3->window);
            goto activate_replace;
          }
        } else
          goto activate_replace;
      }
      AddBufGadget(Storage3);
      Visible=VISIBLE_OFF;
      TestCursorPos(Storage3);
      Visible=tempvisible;
      CenterScreen(Storage3);
      UpdateScreen(Storage3);
      FixMarg(Storage3);
      Storage3->namedisplayed=FALSE;
      BUF(namedisplayed)=FALSE;
      Showstat(Storage);
      MoveSlider(Storage3);
      Showstat(Storage3);
      return(Storage3);
    }
    Storage2=Storage2->NextBuf;
  }
  FixMarg(Storage);
  BUF(namedisplayed)=FALSE;
  return(Storage);
}

/***********************************************
 *
 *  Free( BufStruct *, BOOL, BOOL)
 *
 *  Deallocate a buffer.
 *  Input a pointer to the BufStruct to delete
 *  and if you want something in return.
 *  A LOCK_WRITE is supposed to have been done on the buffer.
 *
 *  Return a new BufStruct to use.
 *
 ***********/
BufStruct __regargs *Free(BufStruct *Storage, BOOL returnwanted, BOOL hook)
{
  int shard=SHS(shared);
  BufStruct *Storage3;

  if (BUF(locked) || (shard==1 && SHS(locked)))
    return(Storage);

  if (hook && shard==1) {
    BUF(locked)++;
    RunHook(Storage, DO_BUFFER_KILL, 1, (char **)&Storage, 0);
    BUF(locked)--;
  }
  Storage3=DeleteEntry(Storage, returnwanted);

  return(Storage3);
}

int __regargs DeleteBuffer(SharedStruct *shared)
{
  BufStruct *Storage;
  int temp=shared->shared;
  int updt=0;
  if (!shared->changes && !shared->locked && !shared->current) {
    Storage=shared->Entry;
    while (Storage) {
      if (BUF(locked))  
        return 0;	// exit error
      updt|=(int)BUF(window);
      Storage=BUF(NextSplitBuf);
    }
    while (temp) {
      DeleteEntry(shared->Entry, TRUE);
      temp--;
    }
    device_has_killed_a_buffer=TRUE;
    if (updt)
      UpdateAll();
    return 1;	// exit OK
  }
  return 0;	// exit error
}

static BufStruct *DeleteEntry(BufStruct *Storage, BOOL returnwanted)
{
  int counter, shard=SHS(shared);
  SharedStruct *shared=BUF(shared);
  BufStruct *Storage2, *Storage3;

  if (BUF(locked) || (shard==1 && SHS(locked)))
    return(Storage);

      assert(sets != 0); // DEBUG
  if (returnwanted) {
    if (BUF(window)) {
      Storage3=NextHiddenEntry(Storage);
      if ((Storage3->shared->type&type_HIDDEN) && !(SHS(type)&type_HIDDEN)) {
        register BufStruct *first=Storage3;

        while (Storage3->shared->type&type_HIDDEN) {
          Storage3=NextHiddenEntry(Storage3);
          if (Storage3==first)
            break;
        }
        if (Storage3==first) {
          if ((Storage3->shared->type&type_HIDDEN) &&
              BUF(window)->Views==1) {
                               /* Sista buffern!  Töm den bara */
            Storage3=MakeNewBuf(Storage);
            DeleteEntry(Storage, returnwanted);
            return(Storage3);
          }
          Storage3=Storage;
        }
      }
      if (Storage3==Storage)
        Storage3=RemoveBuf(Storage);
      else
        Storage3=Activate(Storage, Storage3, acREPLACE);
    } else {
      if (!FRONTWINDOW) // No window available.
        Storage3=Default.BufStructDefault.NextBuf;  //Take first best
      else
        Storage3=FRONTWINDOW->NextShowBuf;
    }
  } else {
    RemoveBufGadget(Storage);
    Storage3=0;
    if (BUF(window)) {
      if (BUF(PrevShowBuf))
        BUF(PrevShowBuf)->NextShowBuf=BUF(NextShowBuf);
      else
        BUF(window)->NextShowBuf=BUF(NextShowBuf);
      if (BUF(NextShowBuf))
        BUF(NextShowBuf)->PrevShowBuf=BUF(PrevShowBuf);
    }
  }
  if (shard==1 && !SHS(freeable)) {
    Clear(Storage, FALSE);
    return(Storage3);
  }
  if (shard==1) {

    if (BUF(shared)==BlockBuffer)
      BlockBuffer=Default.FirstBlock;
    if (SHS(name_number)) {
    				/* Om flera buffrar har samma namn, så räknar
    				   vi ner namnnummret. */
      CountDownFileNumber(BUF(shared));
      if (returnwanted)
        ShowAllstat();
    }
    	/* Nollställ alla lock på filen */
    {
      register LockStruct *lock=SHS(Locks);
      while(lock) { /* OBS, idag kan det bara finnas EN write lock på filen */
        lock->type=0;
        lock=lock->Next;
      }
    }
#ifdef POOL_DEALLOC
    if (!quitting)
#endif
    {
      for (counter=1; counter<=SHS(line); counter++) {
        if (RAD(counter)<SHS(fileblock) || RAD(counter)>(SHS(fileblock)+SHS(fileblocklen)))
          DeallocLine(Storage, counter);
      }
      FreeUndoLines(BUF(shared), SHS(Undotop)+1);
    }
    Storage2=BUF(PrevBuf);
    Storage2->NextBuf=BUF(NextBuf);
    if ((Storage2=BUF(NextBuf)))
      Storage2->PrevBuf=BUF(PrevBuf);

    if (SHS(Next))		/* Connect the link */
      (SHS(Next))->Prev=SHS(Prev);
    (SHS(Prev))->Next=SHS(Next);

#ifdef POOL_DEALLOC
    if (!quitting)
#endif
    {
      Dealloc(SHS(UndoBuf));
      Dealloc(SHS(filnamn));  
      Dealloc(SHS(path));
      Dealloc(SHS(text));
      Dealloc(SHS(macrokey));
      Dealloc(SHS(comment_begin));
      Dealloc(SHS(comment_end));
      FreeLokalInfo(BUF(shared));
      Dealloc(SHS(fileblock));
      Dealloc(BUF(fact_name));
      Dealloc(SHS(face_name));
      Dealloc(BUF(shared));
      Dealloc(Storage);
    }
    Default.Buffers--;	// Decrease number of buffers
    Default.Entries--;	// Decrease number of entries
  } else {
	/* Buffern är splittad, så vi tar bara bort entryt. */
    register BufStruct *Storage0;
    if (SHS(Entry)==Storage) {
      SHS(Entry)=BUF(NextSplitBuf);
      if (!SHS(Entry))
        SHS(Entry)=BUF(PrevSplitBuf);
    }
    if ((Storage0=BUF(PrevSplitBuf)))
      Storage0->NextSplitBuf=BUF(NextSplitBuf);
    if ((Storage0=BUF(NextSplitBuf)))
      Storage0->PrevSplitBuf=BUF(PrevSplitBuf);
    Storage0=Storage;
    shared->shared=shard-1;
    while ((Storage0=Storage0->NextSplitBuf)) {
      Storage0->view_number--;
    }
    Storage0=BUF(PrevBuf);
    Storage0->NextBuf=BUF(NextBuf);
    if ((Storage0=BUF(NextBuf)))
      Storage0->PrevBuf=BUF(PrevBuf);
#ifdef POOL_DEALLOC
    if (!quitting)
#endif
    {
      Dealloc(BUF(fact_name));
      UpdtDupStat(Storage);
      Dealloc((char *)Storage);
    }
    Default.Entries--;	// Decrease number of entries
  }
  editmax--;
  return(Storage3);
}

static void FreeLokalInfo(SharedStruct *shared)
{				// Delete the LokalInfo.
  int counter;
  assert(sets != 0);
  for (counter=0; counter<antalsets; counter++) {
      if (sets[counter] && (sets[counter]->type & (15|ST_SHARED|ST_USERDEFINED))==(ST_STRING|ST_SHARED|ST_USERDEFINED))
          Dealloc((char *)shared->LokalInfo[sets[counter]->offset]);
  }
  Dealloc(shared->LokalInfo);
  shared->LokalInfo=0;
}

/***********************************************
 *
 *  MakeNewBuf(BufStruct *)
 *
 *  Allocate a new buffer in the buffer list and activates it!
 *  Input the currunt BufStruct pointer.
 *  Returns the BufStruct pointer.
 *
 ***********/
BufStruct *MakeNewBuf(BufStruct *Storage3)
{
  BufStruct *Storage, *Storage2;
  Storage=(BufStruct *)Malloc(sizeof(BufStruct));	

  if (Storage) {
    if (Init(Storage3, Storage)) { /* init all values to default */
    
      BUF(NextBuf)=Default.BufStructDefault.NextBuf;
      if ((Storage2=BUF(NextBuf))) Storage2->PrevBuf=Storage;
      BUF(PrevBuf)=&Default.BufStructDefault;
      Default.BufStructDefault.NextBuf=Storage;
      SHS(UndoBuf)=(UndoStruct **)Malloc(SHS(Undomax) * sizeof(UndoStruct));
      Default.Buffers++;	// Increase number of buffers
      Default.Entries++;	// Increase number of entries
      CheckName(Storage, FALSE, TRUE, 0);
    } else {
      Dealloc(Storage);
      Storage=0;
    }
  }
  return(Storage);
}


/***********************************************
 *
 *  Init( BufStruct *from,BufStruct *to)
 *
 *  Inits the buffer you desire using 'from-buffer' as default.
 *  Return OK/FALSE.
 ***********/
BOOL Init(BufStruct *Storage2, BufStruct *Storage)
{
  BOOL ret=FALSE;
  memcpy(Storage,&Default.BufStructDefault,sizeof(BufStruct));	/* Copy default */
  BUF(shared)=(SharedStruct *)Malloc(sizeof(SharedStruct));

  if (BUF(shared)) {
    memcpy(BUF(shared),&Default.SharedDefault,sizeof(SharedStruct));
    SHS(buffer_number)=++buffer_number;

    SHS(text)=(TextStruct *)Malloc(sizeof(TextStruct)*ALLOC_STEP);	/* Reset text pointers */
    if (SHS(text)) {
      memset(SHS(text), 0, sizeof(TextStruct)*ALLOC_STEP);
      {
        if (Default.lokalinfoalloc)
          SHS(LokalInfo)=(int *)Malloc(sizeof(int)*Default.lokalinfoalloc);
        if (!Default.lokalinfoalloc || SHS(LokalInfo)) {
          if (Default.lokalinfoalloc) {	// Copy the LokalInfo.
            register int counter;
            memcpy(SHS(LokalInfo), Default.SharedDefault.LokalInfo,
                   sizeof(int)*Default.lokalinfoantal);
            for (counter=0; counter<antalsets; counter++) {
              if ((sets[counter]->type & (15|ST_USERDEFINED|ST_GLOBAL))==(ST_STRING|ST_USERDEFINED))
                SHS(LokalInfo[sets[counter]->offset])=(int)Strdup((char *)Default.SharedDefault.LokalInfo[sets[counter]->offset]);
            }
          }
          SHS(taket)=ALLOC_STEP;
          SHS(line)=1;
          RAD(1)=0;
          LEN(1)=0;
          FOLD(1)=0;
          LFLAGS(1)=0;
          SHS(current)=0;
          SHS(text)[0].current_style=0;
          SHS(text)[0].old_style=0;
          SHS(face_updated_line)=0;
          BUF(face_top_updated_line)=0;
          BUF(face_bottom_updated_line)=0;
          SHS(filnamn)=Strdup("");
          SHS(comment_begin)=Strdup(Default.SharedDefault.comment_begin);
          SHS(comment_end)=Strdup(Default.SharedDefault.comment_end);
          DateStamp(&SHS(date));
          if (Storage2) {
            SHS(path)=Strdup(Storage2->shared->path);
            SHS(Next)=Storage2->shared->Next;	/* Make up the Shared-link */
            SHS(Prev)=Storage2->shared;
            Storage2->shared->Next=BUF(shared);
            if (SHS(Next))
              (SHS(Next))->Prev=BUF(shared);
          } else {
            SHS(path)=Strdup("");
//            strcpy((char *)&SHS(protection), Default.SharedDefault.protection);
            SHS(Next)=Default.SharedDefault.Next;/* Make up the Shared-link */
            Default.SharedDefault.Next=BUF(shared);
            SHS(Prev)=&Default.SharedDefault;
            BUF(window)=FRONTWINDOW; // Sätt fönster eller NULL
            if (SHS(Next))
              (SHS(Next))->Prev=BUF(shared);
          }
          BUF(fact_name)=Strdup(Default.BufStructDefault.fact_name);
          SHS(face_name)=Strdup(Default.SharedDefault.face_name);
          SHS(face)=FaceGet(SHS(face_name), FACEGET_CHECK);
          BUF(using_fact)=Default.BufStructDefault.using_fact;
          SHS(Entry)=Storage;
          BUF(NextShowBuf)=NULL;	/* Clear Anchor */
          BUF(PrevShowBuf)=NULL;
          BUF(NextBuf)=NULL;
          BUF(PrevBuf)=NULL;
          BUF(NextHiddenBuf)=NULL;
          BUF(PrevHiddenBuf)=NULL;
          BUF(window)=NULL;
        
          ret=TRUE;
        }
      }
    }
  }
  if (!ret) {
    if (BUF(shared)) {
      Dealloc(SHS(text));
      Dealloc(SHS(LokalInfo));
    }
    Dealloc(BUF(shared));
  }
  return(ret);
}

/***********************************************
 *
 *  Clear( BufStruct *)
 *
 *  Clears the buffer you desire.
 *
 ***********/
void Clear(BufStruct *Storage, BOOL do_undo)
{
    int counter;
    BOOL update=BUF(block_exists);

    if (SHS(size)) {
        if (do_undo && CompactBuf(BUF(shared), NULL)>=OK) {
            UndoAdd(Storage, SHS(fileblock), undoNORMAL|undoNEWTEXT, SHS(size));
        } else {
            FreeUndoLines(BUF(shared), SHS(Undotop)+1);
            SHS(changes)=0;
            SHS(Undomem)=0;
            SHS(buffer_number)=++buffer_number;
        }
    }

    strcpy(SHS(protection), Default.SharedDefault.protection);
    SHS(fileprotection)=Default.SharedDefault.fileprotection;
    SHS(fragment)=-SHS(line);
    SHS(fragmentlen)=-SHS(size);
    for (counter=1; counter<=SHS(line); counter++)
        DeallocLine(Storage, counter);
    Dealloc((char *)SHS(fileblock));
    SHS(fragment)=0;
    SHS(fragmentlen)=0;
    SHS(fileblock)=NULL;
    SHS(fileblocklen)=0;
    SHS(taket)=ALLOC_STEP;
    
    {
        void *old_text=SHS(text);
        SHS(text)=(TextStruct *)Malloc(sizeof(TextStruct)*ALLOC_STEP);
        if (SHS(text)) Dealloc(old_text);
        else SHS(text)=old_text;
    }

    SHS(line)=1;
    RAD(1)=NULL;
    LEN(1)=0;
    FOLD(1)=0;
    LFLAGS(1)=0;
    SHS(size)=0;
    Storage->shared->text[0].current_style=0;
    Storage->shared->text[0].old_style=0;
    Storage->shared->face_updated_line=0;

    {
        BufStruct *count=SHS(Entry);
        while (count) {
            count->face_top_updated_line=0;
            count->face_bottom_updated_line=0;
            count->curr_topline=1;
            count->cursor_x=1;
            count->cursor_y=1;
            count->curr_line=1;
            count->screen_x=0;
            count->string_pos=0;
            count->wanted_x=0;
            count->block_exists=be_NONE;
            count->namedisplayed=FALSE;
            count=count->NextSplitBuf;
        }
    }

    if (update) UpdateAll();
}


BufStruct __regargs *ChooseBuf(BufStruct *Storage, char *title, int type, int entries)
{
  BufStruct *Storage2, **Storagelist;
  int count=0, counter=0, ret=OK;
  char **list;
  char *strtemp;
  char **strstore=NULL;
  int antalstrstore=0;
  int maxantalstrstore=0;

  Storage2=Default.BufStructDefault.NextBuf;
  while(Storage2) {
    if (Storage2->shared->type&type)
      count++;
    Storage2=Storage2->NextBuf;
  }
  list=(char **)Malloc(sizeof(char *)*count);
  if (list) {
    Storagelist=(BufStruct **)Malloc(sizeof(BufStruct *)*count);
    Storage2=Default.BufStructDefault.NextBuf;
    while (Storage2) {
      if ((Storage2->shared->type&(type&~type_HIDDEN)) && (Storage2->shared->shared==1 || !Storage2->view_number || entries)) {
        if (!(Storage2->shared->type&type_HIDDEN) || (type&type_HIDDEN)) {
          list[counter]=DEFAULTNAME;
          if (Storage2->shared->filnamn[0])
            list[counter]=Storage2->shared->filnamn;
          if (Storage2->shared->shared!=1 || Storage2->shared->name_number) {
            antalstrstore++;
            if (antalstrstore>maxantalstrstore) {
              register char *tempmem;
              tempmem=Realloc((char *)strstore, (maxantalstrstore+=5)*sizeof(char *));
              if (!tempmem) {
                ret=OUT_OF_MEM;
                break;
              }
              strstore=(char **)tempmem;
            }
            strtemp=Malloc(strlen(list[counter])+10);
            if (!strtemp) {
              ret=OUT_OF_MEM;
              break;
            }
            strstore[antalstrstore-1]=strtemp;
            strcpy(strtemp, list[counter]);
            if (Storage2->shared->name_number)
              Sprintf(strtemp+strlen(strtemp), " (%ld)", Storage2->shared->name_number);
            if (Storage2->shared->shared!=1 && entries)
              Sprintf(strtemp+strlen(strtemp), " #%ld", Storage2->view_number);
            list[counter]=strtemp;
          }
          Storagelist[counter]=Storage2;
          counter++;
        }
      }
      Storage2=Storage2->NextBuf;
    }

    if (ret>=OK)
      ret=Reqlist(Storage,
                  REQTAG_ARRAY, list, counter,
                  REQTAG_SORT,
                  REQTAG_TITLE, title[0]?title:RetString(STR_CHOOSE_ENTRY),
                  REQTAG_END);
  
    if (antalstrstore) {
      while (antalstrstore>0)
        Dealloc(strstore[--antalstrstore]);
      Dealloc((char *)strstore);
    }
  
    if (ret >= 0)
      Storage=Storagelist[ret];
    else {
      Storage=NULL;
    }
    Dealloc((char *)list);
    Dealloc((char *)Storagelist);
  }
  return(Storage);
}


BufStruct __regargs *ReSizeBuf(BufStruct *CurrentStorage, BufStruct *Storage, BufStruct *NextHdnStorage, int lines)
{
  BufStruct *Storage2, *Storage3, *Storage4=CurrentStorage;
  int linedif;

  CursorXY(NULL, -1, -1);
  if (BUF(window) && (BUF(NextBuf) || BUF(screen_lines)!=BUF(window)->window_lines ||
      BUF(PrevBuf)!=&Default.BufStructDefault)) {
    if ((BUF(top_offset)+lines) >= BUF(window)->window_lines)
      lines=BUF(window)->window_lines-BUF(top_offset)+1;

    if (lines < 1) {
      if (!NextHdnStorage)
        NextHdnStorage=GetNewBuf(Storage, Storage, SC_PREV_HIDDEN_BUF, SHS(type));
      if (lines)
        lines++;
      Storage2=BUF(NextShowBuf);
      if (Storage2)
        lines+=Storage2->screen_lines;
      Storage4=RemoveBuf(Storage);
      if (Storage4 && Storage2) {
        Storage3=ReSizeBuf(CurrentStorage, Storage2, NextHdnStorage, lines);
        if (Storage3!=CurrentStorage)
          Storage4=Storage3;
      }
    } else {
      linedif=lines-BUF(screen_lines);
      BUF(screen_lines)=lines;

      Storage2=BUF(PrevShowBuf);
      if (Storage2) {
        Storage2->top_offset+=linedif;
        Storage2->screen_lines-=linedif;
        while (Storage2) {
          Storage=Storage2->NextShowBuf;
          if ((linedif=Storage2->top_offset-
			   BUF(top_offset)-BUF(screen_lines)-1)) {
            Storage2->screen_lines+=linedif;
            Storage2->top_offset-=linedif;
          }
          Storage3=Storage2->PrevShowBuf;
          if (Storage2->screen_lines < 1) {
            if (Storage2->NextShowBuf)
              Storage2->NextShowBuf->PrevShowBuf=Storage2->PrevShowBuf;
            if (Storage3)
              Storage3->NextShowBuf=Storage2->NextShowBuf;
            else
              Storage2->window->NextShowBuf=Storage2->NextShowBuf;
            RemoveBufGadget(Storage2);
            if (Storage2==CurrentStorage)
              Storage4=Storage2->NextShowBuf;
            Storage2->PrevShowBuf=NULL;
            Storage2->NextShowBuf=NULL;
            Storage2->window->Views--;
            Storage2->window=NULL;
          }
          Storage2=Storage3;
        }
      }
      Storage2=BUF(window)->NextShowBuf;  // Fönster ska finnas
      if ((Storage2->top_offset + Storage2->screen_lines) <
          Storage2->window->window_lines) {
        if (!NextHdnStorage) {
          NextHdnStorage=GetNewBuf(Storage2, Storage2, SC_PREV_HIDDEN_BUF, Storage2->shared->type);
        }
        Storage3=NextHdnStorage;
        Storage4=Storage3;
        if (Storage3==Storage2)
          Storage2->screen_lines=Storage2->window->window_lines-
                                Storage2->top_offset +1;
        else {
          Storage3->top_offset=Storage2->top_offset+Storage2->screen_lines+1;
          Storage3->screen_lines=Storage2->window->window_lines-
                                Storage3->top_offset +1;
          Storage3->screen_col=Storage2->window->window_col;

          Storage3->NextShowBuf=Storage2;
          Storage2->PrevShowBuf=Storage3;
          Storage3->PrevShowBuf=NULL;
          Storage3->window=BUF(window);
          Storage3->window->NextShowBuf=Storage3;
	  AddBufGadget(Storage3);
          UpdateScreen(Storage3);
          FixMarg(Storage3);
          Storage3->window->Views++;
        }
      }
    }
    if (Storage4) {
      FixMarg(Storage4);
      TestCursorPos(Storage4);
    }
  }
  if (CurrentStorage->window)
    Storage4=CurrentStorage;
  return(Storage4);
}


BufStruct * RemoveBuf(BufStruct *Storage)
{
    BufStruct * Storage2;
    BufStruct * Storage3 = Storage;

  if (BUF(window)) {
      if (BUF(window)->window_pointer) RemoveBufGadget(Storage);

      BUF(namedisplayed)=FALSE;
      Storage3=BUF(NextShowBuf);
      
      if ((Storage3==NULL) && !BUF(PrevShowBuf)) {
          /* Enda buffen */
          Storage3=GetNewBuf(Storage, Storage, SC_PREV_HIDDEN_BUF, SHS(type));
          if (Storage3) {
              Storage3->screen_lines=BUF(screen_lines);
              Storage3->screen_col=BUF(screen_col);
              Storage3->top_offset=BUF(top_offset);
              Storage3->left_offset=BUF(left_offset);
              Storage3->window=BUF(window);
              Storage3->window->NextShowBuf=Storage3;
              Storage3->PrevShowBuf=NULL;
              Storage3->NextShowBuf=NULL;
              AddBufGadget(Storage3);
          }
          Storage3->window->NextShowBuf=Storage3;
      } else {
          if (!Storage3) {
              Storage3=BUF(PrevShowBuf);        /* Översta buffen */
              Storage3->top_offset=Storage3->top_offset-BUF(screen_lines)-1;
              Storage3->screen_lines=BUF(screen_lines)+Storage3->screen_lines+1;
          }
          else                                /* Ej översta buffen */
              Storage3->screen_lines=BUF(screen_lines)+Storage3->screen_lines+1;
          BUF(window)->Views--;	// Decrease number of views
          if (BUF(PrevShowBuf)) {
              Storage2=BUF(PrevShowBuf);
              Storage2->NextShowBuf=BUF(NextShowBuf);
          } else {
              BUF(window)->NextShowBuf=BUF(NextShowBuf);
          }
          if ((Storage2=BUF(NextShowBuf)))
              Storage2->PrevShowBuf=BUF(PrevShowBuf);
      }


      if (Storage3) {
          assert(Storage3 > 1024);

          if (Storage3!=Storage /*&& BUF(PrevShowBuf)*/) {
              
              if (Storage3->block_exists)
                  SetBlockMark(Storage3, FALSE);
              
              if (BUF(window)->ActiveBuffer==Storage) {
                  BUF(window)->ActiveBuffer=Storage3;
              }
              
              BUF(PrevShowBuf)=NULL;
              BUF(NextShowBuf)=NULL;
              BUF(window)=NULL;
              TestCursorPos(Storage3);
          }
      }
  }
  return(Storage3);
}

void FixMarg(BufStruct *Storage) {
    BufStruct *Storage2;
    int summa;
    int col;
    int rightmarg;

  Storage2=&Default.BufStructDefault;
  BUF(move_screen)=BUF(window)?BUF(window)->move_screen:Default.WindowDefault.move_screen;

  rightmarg=((Storage2->rightmarg<BUF(using_fact)->extra[FACT_NO_EOL].length)?BUF(using_fact)->extra[FACT_NO_EOL].length:Storage2->rightmarg);
  col=Storage->screen_col-BUF(using_fact)->extra[FACT_NO_EOL].length;

  summa=Storage2->uppermarg+Storage2->lowermarg;

  if (summa > BUF(screen_lines)) {
    BUF(uppermarg)=BUF(screen_lines)/2;
    BUF(lowermarg)=BUF(screen_lines)/2;
    if (BUF(uppermarg) > Storage2->uppermarg) {
      BUF(uppermarg)=Storage2->uppermarg;
      BUF(lowermarg)=BUF(screen_lines)-BUF(uppermarg);
    }
    if (BUF(lowermarg) > Storage2->lowermarg) {
      BUF(lowermarg)=Storage2->lowermarg;
      BUF(uppermarg)=BUF(screen_lines)-BUF(lowermarg);
    }
  } else {
    BUF(uppermarg)=Storage2->uppermarg;
    BUF(lowermarg)=Storage2->lowermarg;
  }

  summa=Storage2->leftmarg+rightmarg;
  BUF(rightmarg)=rightmarg;
  BUF(leftmarg)=Storage2->leftmarg;

  if (summa > col-BUF(l_c_len)) {
    if (BUF(leftmarg) > col/2) {
      BUF(leftmarg)=col/2;
    }
    if (BUF(rightmarg) > col/2) {
      BUF(rightmarg)=col/2;
    }
    if (BUF(rightmarg) < rightmarg) {
      BUF(rightmarg)=rightmarg;
      BUF(leftmarg)=BUF(screen_col)-BUF(rightmarg)-BUF(l_c_len);
      if (BUF(leftmarg)<=0) {
        BUF(leftmarg)=1;
        BUF(move_screen)=1;
      }
    }
  }
  if (Storage->screen_col-BUF(rightmarg)-BUF(leftmarg)-BUF(l_c_len)<BUF(move_screen)) {
    BUF(move_screen)=Storage->screen_col-BUF(rightmarg)-BUF(leftmarg)-BUF(l_c_len);
    if (BUF(move_screen)<=0)
      BUF(move_screen)=1;
  }
  BUF(rightmarg)+=BUF(l_c_len);
}

/*************************************************
*
*  BufStruct *DuplicateBuf(BufStruct *Storage)
*
*  Duplicerar önskad buffert men aktiverar den inte.
**********/
BufStruct __regargs *DuplicateBuf(BufStruct *Storage2)
{
  BufStruct *Storage, *Storage3;

  Storage=(BufStruct *)Malloc(sizeof(BufStruct));	
  if (!Storage)
    return(NULL);

  memcpy(Storage, Storage2, sizeof(BufStruct));
  BUF(NextShowBuf)=NULL;
  BUF(PrevShowBuf)=NULL;
  BUF(NextHiddenBuf)=NULL;
  BUF(PrevHiddenBuf)=NULL;
  BUF(window)=NULL;
  BUF(namedisplayed)=FALSE;
  BUF(slider_on)=FALSE;

  BUF(NextBuf)=Default.BufStructDefault.NextBuf;
  if ((Storage3=BUF(NextBuf)))
    Storage3->PrevBuf=Storage;
  BUF(PrevBuf)=&Default.BufStructDefault;
  Default.BufStructDefault.NextBuf=Storage;

  BUF(locked)=0;

  Storage3=Storage2;
  while(Storage3->NextSplitBuf)
    Storage3=Storage3->NextSplitBuf;
  SHS(shared)++;
  BUF(view_number)=Storage3->view_number+1;

  BUF(fact_name)=Strdup(Storage2->fact_name);

  BUF(PrevSplitBuf)=Storage3;
  BUF(NextSplitBuf)=NULL;
  Storage3->NextSplitBuf=Storage;
  Default.Entries++;	// Increase number of entries
  return(Storage);
}

/************************************
 *
 * FindBuffer(int Argc, char **Argv)
 *
 * Där Argv innehåller (char *buffer, int fileno, int viewnumber)
 * 
 * Försöker först hitta PATH+NAMN CASE SENSITIVE, därefter
 * PATH+NAMN CASE INSENSITIVE, sen NAMN SENSITIVE och sist NAMN INSENSITIVE.
 ******/
BufStruct __regargs *FindBuffer(int Argc, char **Argv)
{
  register BufStruct *Storage2=NULL;
  register int format=0;
  register int result;
  char temp[FILESIZE];
  char *buf=temp;

  if (Argc) {
    while (format<4) {
      Storage2=Default.BufStructDefault.NextBuf;
      while (Storage2) {
        if (format<=1)
          strmfp(buf, Storage2->shared->path, Storage2->shared->filnamn);
        else
          buf=Storage2->shared->filnamn;
        if (format&1)
          result=stricmp(buf, Argv[0]);
        else
          result=strcmp(buf, Argv[0]);
        if (!result) {
          if (Argc>1) {
            if (Storage2->shared->name_number==(int)Argv[1] || (!(int)Argv[1] && Storage2->shared->name_number==1)) {
              if (Argc==2 || (Argc>2 && Storage2->view_number==(int)Argv[2]))
                return(Storage2);
            }
          } else
            return(Storage2);
        }
        Storage2=Storage2->NextBuf;
      }
      format++;
    }
  }
  return(Storage2);
}


/**********************************************************
 *
 *  LockBuf_release_semaphore(SharedStruct *shared)
 *
 *  LockBuf and release semaphore
 ******/
void __regargs LockBuf_release_semaphore(SharedStruct *shared)
{
  if (semaphore_count==1) {
    if (shared)
      shared->current++;
    ReleaseSemaphore(&LockSemaphore);
  }
  semaphore_count--;
}

void __regargs UnLockBuf_obtain_semaphore(SharedStruct *shared)
{
  if (!semaphore_count) {
    if (shared)
      shared->current--;
    ObtainSemaphore(&LockSemaphore);
  }
  semaphore_count++;
}

BufStruct * __regargs CheckBufID(BufStruct *BufferID)
{
  BufStruct *Storage=&Default.BufStructDefault;
  {
    register WindowStruct *win=&Default.WindowDefault;
  
    while (win) {
      win=win->next;
      if (win && win==(WindowStruct *)BufferID)
        return win->NextShowBuf;
    }
  }
  {
  
    while (Storage) {
      Storage=BUF(NextBuf);
      if (Storage==BufferID)
        break;
    }
    if (!Storage) {
      register SharedStruct *shared=&Default.SharedDefault;
      while ((shared=shared->Next)) {
        if (shared==(SharedStruct *)BufferID) {
          Storage=shared->Entry;
          break;
        }
      }
    }
  }
  return(Storage);
}

WindowStruct *__regargs CheckWinID(WindowStruct *winID)
{
  register WindowStruct *win=NULL;
  register BufStruct *Storage;
  Storage=CheckBufID((BufStruct *)winID);
  if (Storage)
    win=BUF(window);
  return(win);
}

/* Flytta ett fönster längst fram i listan */
void __regargs HeadWindow(WindowStruct *win)
{
  WindowStruct *to_last;
  WindowStruct *last=win;
  if (win && win->prev) {
    while (last->next)
      last=last->next;
    to_last=FRONTWINDOW;
    win->prev->next=NULL;
    win->prev=NULL;
    last->next=to_last;
    to_last->prev=last;
    Default.WindowDefault.next=win;
  }
}

WindowStruct *__regargs FindWindow(struct Window *window)
{
  register WindowStruct *win=&Default.WindowDefault;

  while (win) {
    win=win->next;
    if (win && win->window_pointer==window)
      break;
  }
  return(win);
}

int __regargs CountDownFileNumber(SharedStruct *Shared)
{
  register int number=Shared->name_number;
  register int found=0;
  register SharedStruct *Shared2=Default.SharedDefault.Next;
  register SharedStruct *Sharedtemp=NULL;

  while (Shared2) {
    if (Shared2!=Shared) {
      if (Shared2->name_number &&
          !Stricmp(Shared->filnamn, Shared2->filnamn)) {
        if (Shared2->name_number>number)
          Shared2->name_number--;
        found++;
        Sharedtemp=Shared2;
      }
    }
    Shared2=Shared2->Next;
  }
  if (found==1) {
    Sharedtemp->name_number=0;
  }
  Shared->name_number=0;
  return(found);
}


BufStruct __regargs *GetNewBuf(BufStruct *Storage, BufStruct *Storage2, int argID, int type)
{
  register BufStruct *firststor=NULL;
  int tempint=-1;
  do {
    if (!tempint)
      firststor=Storage2;
    tempint++;
    switch (argID) {
    case SC_NEXT_BUF:
      Storage2=NextBuf(Storage2);
      break;
    case SC_PREV_BUF:
      Storage2=PrevBuf(Storage2);
      break;
    case SC_NEXT_ENTRY:
      Storage2=NextEntry(Storage2);
      break;
    case SC_PREV_ENTRY:
      Storage2=PrevEntry(Storage2);
      break;
    case SC_NEXT_SHOW_BUF:
      Storage2=NextView(Storage2);
      break;
    case SC_PREV_SHOW_BUF:
      Storage2=PrevView(Storage2);
      break;
    case SC_NEXT_HIDDEN_BUF:
      Storage2=PrevHiddenEntry(Storage2);
      break;
    case SC_PREV_HIDDEN_BUF:
      Storage2=NextHiddenEntry(Storage2);
      break;
    }
    if (type) {
      if (Storage2==firststor ||
          (((Storage2->shared->type&type)&~type_HIDDEN) && (Storage2->shared->type&type_HIDDEN)<=(type&type_HIDDEN))) {
        break;
      }
    }
  } while (type);
  if (tempint>0 && firststor==Storage2)
    Storage2=Storage;
  if (argID==SC_NEXT_BUF || argID==SC_PREV_BUF)
      Storage2=(BufStruct *)Storage2->shared;// Returnera SharedStructen

  return(Storage2);
}


void AttachBufToWindow(WindowStruct *win, BufStruct *Storage)
{
    assert(Storage);
    win->NextShowBuf=Storage;
    BUF(window)=win;
    BUF(window->Views)++;

    BufLimits(Storage);
    if (win->window_pointer && win->window_pointer->Height>win->window_minheight &&
        win->window_pointer->Height>win->window_pointer->MinHeight) {
        Status(Storage, RetString(STR_FREXXED_INIT), 0x80);
    }
    assert(Storage > 1024);
    win->ActiveBuffer=Storage;
    BUF(screen_lines)=win->window_lines;
    BUF(screen_col)=win->window_col;
    BUF(top_offset)=1;
    FixMarg(Storage);
    AddBufGadget(Storage);
    TestCursorPos(Storage);
}

