/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************************
*
*  Undo.c
*
**********/

/* Undosträngens syntax:

cursor-x, cursor-y, sträng


cursor syntax:

antal byte, värde,


sträng syntax:

antal antal-byte, antal byte, strängen

*/


#include <string.h>
#include <stdio.h>

#include "Buf.h"
#include "Alloc.h"
#include "Command.h"
#include "Cursor.h"
#include "Edit.h"
#include "GetFile.h"
#include "Hook.h"
#include "IDCMP.h"
#include "Request.h"
#include "Undo.h"
#include "UpdtBlock.h"
#include "UpdtScreen.h"
#include "WindowOutput.h"

extern DefaultStruct Default;
extern int Visible;
extern char CursorOnOff;
#define WORKMAX 1000

/**************************************************************
 *
 *  UndoAdd(BufStruct *, char *, int multiple, int len)
 *
 *  Add a string to the undo buffer of length 'int len'.
 *  Multiple input=TRUE.
 *  If multinput=undoONLYBACKSPACES then (char *) is the number of BS.
 *  Don't forget to use undoONLYTEXT if possible.
 *  Returns TRUE if everything was OK.
 ***********/
int __regargs UndoAdd(BufStruct *Storage, char *text, int multinput, int len)
{
  int length;
  int slask, count;
  char *string, *stringstart, oldchanged;

//FPrintf(Output(), "UndoAdd: %ld,%ld len-%ld\n", BUF(curr_line), BUF(string_pos), len);

  if (!(multinput&undoNEWTEXT) && (!text || (!(multinput&undoONLYBACKSPACES) && !len)))
    return(TRUE);

  if (!Default.workstring) {
    Default.worklength=0;
    Default.worklengthmax=WORKMAX;
    Default.workstring=Malloc(Default.worklengthmax);
    if (!Default.workstring) {
      FreeUndoLines(BUF(shared), SHS(Undotop));
      return(FALSE);
    }
  }

  if (SHS(changes))
    oldchanged=TRUE;
  else
    oldchanged=FALSE;

  if (!Storage->shared->Undo) {	/* Do not store undo */
    FreeUndoLines(BUF(shared), SHS(Undotop));
  } else {
  
    if (!multinput)
      return(TRUE);
  
    len--;
  
    if (multinput & undoNORMAL || Default.oldShared!=BUF(shared)) {
  /*	Dessa rader är kommenterade för att dom gav en Dealloc forgotten
          om man lekte med undo.  De två raderna under har ersatt kommenteringen.
          Jag vet inte vad den andra if'en ska göra (?).  / Kjelle
      if (worklength) {
        StoreWorkString();
        UndoAddTop(BUF(shared));
      } else if (oldShared==BUF(shared) || !oldShared || SHS(Undotop)<0)
        UndoAddTop(BUF(shared));
  */
      StoreWorkString();
      UndoAddTop(BUF(shared));
  
      Default.oldShared=BUF(shared);
      Default.workstring[0]=oldchanged;
      Default.worklength++;
    }
    if (multinput & undoONLYBACKSPACES)
      len=10;
  
    if ((len+40+Default.worklength)>=Default.worklengthmax) {
      if ((len+40+Default.worklength)>=Default.worklengthmax) {
        Default.worklengthmax+=len+40;
        if (!(multinput & undoNEWTEXT))
          Default.worklengthmax+=+WORKMAX;
      }
      Default.workstring=Realloc(Default.workstring, Default.worklengthmax);
      if (!Default.workstring) {
        FreeUndoLines(BUF(shared), SHS(Undotop));
        return(FALSE);
      }
    }
    Default.oldShared->UndoBuf[Default.oldShared->Undotop]->string=Default.workstring;
    string=Malloc(len+24);
    if (!string) {
      FreeUndoLines(BUF(shared), SHS(Undotop));
      return(FALSE);
    }
    stringstart=string;
  
    slask=BUF(string_pos);
    for (count=1; slask>0; count++) {
      *(string+count)=slask & 255;
      slask=slask >>8;
    }
    *(string)=(char)count-1;
    string+=count;
  
    slask=BUF(curr_line);
    for (count=1; slask>0; count++) {
      *(string+count)=slask & 255;
      slask=slask >>8;
    }
    *(string)=(char)count-1;
    string+=count;
  
    if (multinput & undoONLYBACKSPACES) {
      slask=(int)text;
      for (count=1; slask>0; count++) {
        *(string+count)=slask & 255;
        slask=slask >>8;
      }
      *(string)=(char)count-1;
      string+=count;
      *(string++)='b';
      slask=len+1;
    } else {	// undoONLYTEXT eller undoNEWTEXT
      slask=len+1;
      for (count=1; slask>0; count++) {
        *(string+count)=slask & 255;
        slask=slask >>8;
      }
      *(string)=(char)count-1;
      string+=count;
      if (multinput & undoNEWTEXT)
        *(string++)='N';
      else
        *(string++)='T';
      memcpy(string, text, len+1);
      string+=len+1;
    }
  
    length=string-stringstart;
    {
      register int memoryok=TRUE;
      if ((SHS(Undomem)+length)>=Default.Undo_maxmemory) {
        if (!FreeUndoMem(BUF(shared), length+SHS(Undomem)-Default.Undo_maxmemory)) {
          Default.worklength=0;
          memoryok=FALSE;
        }
      }
      if (memoryok) {
        SHS(Undomem)+=length;
      
        if (multinput & undoAPPENDBEFORE) {
          movmem(Default.workstring+1, (char *)(Default.workstring+length+1), Default.worklength-1);
          memcpy(Default.workstring+1, stringstart, length);
        } else {
          memcpy((char *)(Default.workstring+Default.worklength), stringstart, length);
        }
        Default.worklength+=length;
      }
    }
    Dealloc(stringstart);
  }

  if (multinput & undoNORMAL) {
    SHS(changes)++;
    if (SHS(autosave) && Default.autosaveintervall>0 && SHS(changes)>=Default.autosaveintervall) {
      SHS(asenabled)=1;
      SendReturnMsg(cmd_AUTOSAVE, (int)BUF(shared), NULL, NULL, NULL);
    }
  }

  return(TRUE);
}

/************************************************
*
*  UndoAddTop(BufStruct *)
*
*  Increase the top of the undo buffer and realloc if necessary.
*
**********/
void __regargs UndoAddTop(SharedStruct *shared)
{

  shared->Undotop++;
  if (shared->Undotop >= shared->Undomax) {
    if ((shared->Undomax) < Default.Undo_maxline) {
      if ((shared->Undomax+10) < Default.Undo_maxline)
        shared->Undomax+=10;
      else
        shared->Undomax=Default.Undo_maxline;
      shared->UndoBuf=(UndoStruct **)Realloc((char *)shared->UndoBuf,
                     sizeof(UndoStruct *)*(shared->Undomax));
      if (!shared->UndoBuf) {
        FreeUndoLines(shared, shared->Undotop);
        return;
      }
    } else {
      int dif;

      dif=shared->Undotop-Default.Undo_maxline+(shared->Undomax>>2);
      FreeUndoLines(shared, dif);
    }
  }
  shared->UndoBuf[shared->Undotop]=UndoAlloc(shared);
  if (shared->UndoBuf[shared->Undotop])
    shared->Undopos=shared->Undotop+1;
}

/************************************************
*
*  FreeUndoMem(BufStruct *, int)
*
*  Free (int) bytes from the wanted undo buffer.
*  Return:  TRUE=all OK, FALSE=can't free mem.
*
**********/
int __regargs FreeUndoMem(SharedStruct *shared, int mem)
{
  register int counter=0;
  register int memcount=0;
  int ret=TRUE;


  while (memcount<mem && counter<=shared->Undotop) {
    memcount+=shared->UndoBuf[counter]->len;
    counter++;
  }

  if (mem>shared->Undomem)
    ret=FALSE;
/*
  if (counter==SHS(Undotop)) {
    if (!Ok_Cancel("This action will erase\nthe undo buffer.\nContinue?"))
      return(FALSE);
  }
*/

  FreeUndoLines(shared, counter);
  return(ret);
}

/************************************************
*
*  FreeUndoLines(BufStruct *, int)
*
*  Free (int) line from the wanted undo buffer.
*  Return:  TRUE=all OK, FALSE=can't free mem.
*
**********/
int __regargs FreeUndoLines(SharedStruct *shared, int rader)
{
  register int counter;
  int ret=TRUE;

  if (rader && shared->Undotop>=0) {
//    if (Visible==VISIBLE_ON)
//      Status(Storage, "Undo is cleaning up!", 1);
    if (rader>shared->Undotop) {
      rader=shared->Undotop+1;
      if (shared==Default.oldShared) {
        Default.worklength=0;
//        rader--;
      }
    }
    for (counter=0; counter<rader; counter++)
      UndoFree(shared, shared->UndoBuf[counter]);
    shared->Undotop-=rader;
    shared->Undopos-=rader;
    if (shared->Undotop>=0) {
      for (counter=0; counter<=shared->Undotop; counter++)
        shared->UndoBuf[counter]=shared->UndoBuf[counter+rader];
    } else {
      if (Default.oldShared==shared)
        Default.oldShared=NULL;
    }
  }
  return(ret);
}

/************************************************
*
*  Undo(BufStruct *, int)
*
*  Perform a real undo. Say if you want it to continue down in the
*  undo list (TRUE) or go to the end (FALSE).
*  Returns a std-error massage.
*
**********/
int __regargs Undo(BufStruct *Storage, int cont)
{
  int ret=OK;
  int slask;
  UBYTE sign;
  int count;
  int xpos, ypos, antalchar;
  int multiundo=undoNORMAL;
  int tempundopos, temptop;
  char *string;
  char *stringcopy;
  char *retstring;

  StoreWorkString();
  if (!cont) {
    SHS(Undopos)=SHS(Undotop)+1;
    SHS(Undostrpos)=1;
  }

  SHS(Undopos)--;
  if (SHS(Undopos)<=-1) {
    ret=NOTHING_TO_UNDO;
    SHS(Undopos)=-1;
    return(ret);
  }

  string=(stringcopy=SHS(UndoBuf[SHS(Undopos)])->string)+1;

  if (CursorOnOff) {
    CursorXY(Storage, -1, -1);
  }

  do {
    sign=*(string++);
    xpos=0;
    for (count=sign-1; count!=-1; count--)
      xpos=xpos*256+((UBYTE)(*(string+count)));

    string+=sign;
    sign=*(string++);
    ypos=0;
    for (count=sign-1; count!=-1; count--)
      ypos=ypos*256+((UBYTE)(*(string+count)));

    string+=sign;
    sign=*(string++);
    antalchar=0;
    for (count=sign-1; count!=-1; count--)
      antalchar=antalchar*256+((UBYTE)(*(string+count)));
    string+=sign;

    if (Visible==VISIBLE_ON && (antalchar>1 || xpos==0))
      Visible=VISIBLE_OFF;

    SetScreen(Storage, xpos, ypos);
//FPrintf(Output(), "Undo   : %ld,%ld\n", BUF(curr_line), BUF(string_pos));

    sign=*(string++);
    tempundopos=SHS(Undopos);
    temptop=SHS(Undotop);
    if (sign=='b') {
      retstring=(char *)Malloc(antalchar+1);
      if (retstring) {
        ret=BackspaceNr(Storage, retstring, antalchar);
        if (ret>=OK) {
          UndoAdd(Storage, retstring, multiundo | undoONLYTEXT, antalchar);
          Dealloc(retstring);
        } else
          FreeUndoLines(BUF(shared), SHS(Undotop)+1);
      } else
        ret=OUT_OF_MEM;
    } else if (sign=='T') {
      slask=OutputText(Storage, string, antalchar, TRUE, NULL);
      UndoAdd(Storage, (char *)slask, multiundo | undoONLYBACKSPACES, 0);
      string+=antalchar;
    } else if (sign=='N') {
      int newline;
      char *newtext;
      TextStruct *newrad;
      newtext=Malloc(antalchar);
      if (newtext || !antalchar) {
        memcpy(newtext, string, antalchar);
        ret=ExtractLines(newtext, antalchar, &newrad, &newline, NULL);
        if (ret>=OK) {
          ret=CompactBuf(BUF(shared), NULL);
          if (ret>=OK) {
            UndoAdd(Storage, SHS(fileblock), undoNORMAL|undoNEWTEXT, SHS(size));
            Dealloc(SHS(fileblock));
            Dealloc(SHS(text));
            SHS(fileblock)=newtext;
            SHS(fileblocklen)=antalchar;
            SHS(size)=antalchar;
            SHS(text)=newrad;
            SHS(line)=newline;
          }
        } else
          Dealloc(newtext);
      } else
        ret=OUT_OF_MEM;
      if (ret<OK)
        FreeUndoLines(BUF(shared), SHS(Undotop));
      else {
        BUF(curr_topline)=1;
        BUF(cursor_y)=1;
        BUF(curr_line)=1;
        BUF(cursor_x)=1;
        BUF(string_pos)=0;
        UpdateDupScreen(Storage);
      }
      string+=antalchar;
    }
    if (ret>=OK) {
      multiundo=undoAPPENDBEFORE;
      SHS(Undostrpos)=string-stringcopy;
      SHS(Undopos)=tempundopos+((SHS(Undotop)!=temptop)?SHS(Undotop)-1-temptop:0);
    }
  } while (ret>=OK && SHS(Undopos)>=0 && SHS(Undostrpos)<SHS(UndoBuf[SHS(Undopos)]->len));

  SetScreen(Storage, BUF(string_pos), BUF(curr_line));
  if (BUF(block_exists)) {
    SetBlockMark(Storage, FALSE);
  }
  BUF(wanted_x)=BUF(cursor_x)+BUF(screen_x);

  if (SHS(Undopos)>=0 && !stringcopy[0]) {
    SHS(changes)=0;
    UpdtDupStat(Storage);
  }
  return(ret);
}

UndoStruct __regargs *UndoAlloc(SharedStruct *shared)
{
  register UndoStruct *ret;

  ret=(UndoStruct *)Malloc(sizeof(UndoStruct));
  if (ret) {
    ret->string=NULL;
    ret->len=0;
  }
  return(ret);
}

void __regargs UndoFree(SharedStruct *shared, UndoStruct *undo)
{
  if (undo) {
    if (undo->string) {
      if (undo->string!=Default.workstring) {
        Dealloc(undo->string);
        if (shared)
          shared->Undomem-=undo->len;
      } else
        Default.worklength=0;
    }
    Dealloc((char *)undo);
  }
}

void __regargs UndoNoChanged(SharedStruct *shared)
{
  register int counter;

  for (counter=0; counter<=shared->Undotop; counter++)
    shared->UndoBuf[counter]->string[0]=TRUE;
}

void StoreWorkString(void)
{
  if (Default.worklength && Default.oldShared) {
    if ((Default.worklength+Default.oldShared->Undomem)>Default.Undo_maxmemory)
      FreeUndoMem(Default.oldShared, (Default.worklength+Default.oldShared->Undomem)-Default.Undo_maxmemory);
    if (Default.oldShared && Default.worklength<Default.Undo_maxmemory) {
#ifdef DEBUGTEST
      if (Default.oldShared && Default.oldShared->Undotop<0)
FPrintf(Output(), "Säj till Kjelle att här ska man inte vara, :-)  (Undo.c)\n");
#endif
      Default.oldShared->UndoBuf[Default.oldShared->Undotop]->len=Default.worklength;
      Default.oldShared->UndoBuf[Default.oldShared->Undotop]->string=Malloc(Default.worklength);
      if (Default.oldShared->UndoBuf[Default.oldShared->Undotop]->string) {
        memcpy(Default.oldShared->UndoBuf[Default.oldShared->Undotop]->string, Default.workstring, Default.worklength);
        if (Default.worklengthmax>(WORKMAX << 2)) {
          Dealloc(Default.workstring);
          Default.worklengthmax=WORKMAX;
          Default.workstring=Malloc(WORKMAX);
          if (!Default.workstring)
            Default.worklengthmax=0;
        }
      } else {
        FreeUndoLines(Default.oldShared, Default.oldShared->Undotop);
      }
    }
    Default.worklength=0;
  }
}
