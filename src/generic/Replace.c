/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************
*
*       Replace
*
********/

#ifdef AMIGA
#include <exec/types.h>
#include <proto/FPL.h>
#endif
#include <limits.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "Block.h"
#include "Command.h"
#include "Cursor.h"
#include "DoSearch.h"
#include "Edit.h"
#include "Fold.h"
#include "GetFile.h"
#include "IDCMP.h"
#include "Limit.h"
#include "Replace.h"
#include "Request.h"
#include "Search.h"
#include "Undo.h"
#include "UpdtScreen.h"
#include "WindowOutput.h"

extern int Visible;
extern srch Search;          /* search structure */
extern HistoryStruct SearchHistory;
extern char CursorOnOff;
extern int allowcleaning;
extern char buffer[];
extern void *Anchor;
extern int DebugOpt;
extern DefaultStruct Default;
extern BOOL searchcompiled;
extern int searchcompile_flags;
extern char search_fastmap[256];
extern char GlobalEmptyString[];

/********************************************
*
*   Replace(Storage, int)
*
* variant=-1 ger default (från requestern)
* variant=0 ger promptning.
* variant=1 ger continue.
* variant=2 ger engångsreplace.
*********************/
int __regargs Replace(BufStruct *Storage, int argc, char **argv)
{
  SearchStruct SearchInfo;
  char *searchfind=NULL;
  int ret=OK;
  BOOL turbo=FALSE;
  BOOL global=FALSE;
  BOOL stop=FALSE;
  BOOL next;
  BOOL undoappend=FALSE;
  BOOL undosort2=FALSE;
  char *searchstringbuffer=NULL;
  char *replacestringbuffer=NULL;
  int searchstringbufferlen=0;
  int replacestringbufferlen=0;
  turbostruct TT;
  CursorPosStruct cpos;
  int addforward=0;
  BOOL allocatedsearchstring=FALSE;
  BOOL allocatedreplacestring=FALSE;
  BOOL turbo_replaced=FALSE;
  char *searchstring=NULL;
  char *replacestring=NULL;
  int variant=-1;

  SearchInfo.flags=Search.flags;
  SearchInfo.range=INT_MAX;
  

  if (argc>0) {
    variant=(int)argv[0];
    if (argc>1) {
      searchstring=argv[1];
      if (argc>2) {
        replacestring=argv[2];
        if (argc>3) {
          SearchInfo.flags=MakeSearchFlag(argv[3], SearchInfo.flags);
          if (argc>4) {
            SearchInfo.range=(int)argv[4];
          }
        }
      }
    }
  }

  if (searchstring && !searchstring[0])
    searchstring=NULL;

  if (!searchstring && !Search.buffer) {
    ret=SearchAsk(Storage, TRUE);
    SearchInfo.flags=Search.flags;
  }

  if (ret>=OK) {
    Dealloc(Search.lastsearchstring);
    SearchInfo.find_direct=FALSE;
    if ((searchstring && strcmp(searchstring, Search.lastsearchstring)) ||
        (Search.lastpos!=1 && (BUF(curr_line)+BUF(string_pos))==1)) {
      Search.lastpos=1;
      SearchInfo.find_direct=TRUE;
    }
      
    if (searchstring) {
      Search.lastsearchstring=Strdup(searchstring);
      SearchInfo.realbuffer=searchstring;
      if (SearchInfo.flags & WILDCARD) {
        if (Search.secondbuffer && !strcmp(Search.secondbuffer, searchstring)) {
          SearchInfo.buf=Search.second_buf;
          SearchInfo.pattern_compiled=TRUE;
        } else {
          Dealloc(Search.second_buf.buffer);
          Search.second_buf.buffer=NULL;
          Dealloc(Search.secondbuffer);
          Search.secondbuffer=Strdup(searchstring);
          SearchInfo.pattern_compiled=FALSE;
          SearchInfo.buf.buffer=NULL;
          SearchInfo.buf.allocated=0;
          SearchInfo.buf.fastmap=Search.second_buf.fastmap;
        }
        SearchInfo.realbuffer=searchstring;
      } else {
        searchstringbuffer=Strdup(searchstring);
        allocatedsearchstring=TRUE;
        if (!searchstringbuffer)
          ret=OUT_OF_MEM;
        else {
          searchstringbufferlen=fplConvertString(Anchor, searchstringbuffer, searchstringbuffer);
          searchcompiled=FALSE;
          if (SearchInfo.flags & FORWARD)
            BMS_init_forward(searchstringbuffer, searchstringbufferlen, (SearchInfo.flags&CASE_SENSITIVE)?NULL:BUF(using_fact), search_fastmap);
          else
            BMS_init_backward(searchstringbuffer, searchstringbufferlen, (SearchInfo.flags&CASE_SENSITIVE)?NULL:BUF(using_fact), search_fastmap);
        }
      }
    } else {
      Search.lastsearchstring=GlobalEmptyString;
      searchstringbufferlen=Search.buflen;
      searchstringbuffer=Search.buffer;
      if (SearchInfo.flags & WILDCARD) {
        SearchInfo.realbuffer=Search.realbuffer;
        SearchInfo.pattern_compiled=Search.pattern_compiled;
        SearchInfo.buf=Search.buf;	// Copy pattern
      } else {
        if (!searchcompiled || searchcompile_flags!=SearchInfo.flags) {
          searchcompile_flags=SearchInfo.flags;
          searchcompiled=TRUE;
          if (SearchInfo.flags & FORWARD)
            BMS_init_forward(searchstringbuffer, searchstringbufferlen, (SearchInfo.flags&CASE_SENSITIVE)?NULL:BUF(using_fact), search_fastmap);
          else
            BMS_init_backward(searchstringbuffer, searchstringbufferlen, (SearchInfo.flags&CASE_SENSITIVE)?NULL:BUF(using_fact), search_fastmap);
        }
      }
    }
    if (replacestring) {
      SearchInfo.realreplacebuffer=replacestring;
      if (!(SearchInfo.flags & WILDCARD)) {
        allocatedreplacestring=TRUE;
        replacestringbuffer=Strdup(replacestring);
        if (!replacestringbuffer)
          ret=OUT_OF_MEM;
        else
          replacestringbufferlen=fplConvertString(Anchor, replacestringbuffer, replacestringbuffer);
      }
    } else {
      replacestringbufferlen=Search.repbuflen;
      replacestringbuffer=Search.repbuffer;
      SearchInfo.realreplacebuffer=Search.realrepbuffer;
    }

  }
  if (ret>=OK) {
    if (variant<0) {
      if (!Default.search_prompt)
        variant=1;
    }
    SearchInfo.buffer=searchstringbuffer;
    SearchInfo.buflength=0;
    SearchInfo.replength=0;
    SearchInfo.replacebuffer=replacestringbuffer;
    if (!(SearchInfo.flags & WILDCARD)) {
      SearchInfo.buflength=searchstringbufferlen;
      SearchInfo.replength=replacestringbufferlen;
    }
    if (variant==1) {
      if (SearchInfo.flags&FORWARD) {
        SaveCursorPos(Storage, &cpos);
        ret=BeginTurbo(Storage, &TT, &SearchInfo);
        if (ret>=OK)
          turbo=TRUE;
      } else {
        allowcleaning=FALSE;
        global=TRUE;
      }
    }
//    if (BUF(curr_topline)+BUF(cursor_y)-1==1 && BUF(string_pos)==0)
//      addforward=-1;		// Find the first place.
    while (ret>=OK && !stop) {
      next=FALSE;
      SearchInfo.text=SHS(text);
      SearchInfo.lines=SHS(line);
      SearchInfo.begin_y=BUF(curr_line);
      SearchInfo.begin_x=BUF(string_pos)+addforward;

      if (SearchInfo.begin_x<0) {
        SearchInfo.begin_y--;
        SearchInfo.begin_x=LEN(SearchInfo.begin_y)-1;
        if (SearchInfo.begin_y<=0) {
          SearchInfo.begin_y=1;
          SearchInfo.begin_x=0;
          SearchInfo.find_direct=TRUE;
        }
      }
      addforward=0;
      while (SearchInfo.begin_y!=SHS(line) &&
             (unsigned int)SearchInfo.begin_x>=(unsigned int)LEN(SearchInfo.begin_y)) {
        SearchInfo.begin_x-=LEN(SearchInfo.begin_y);
        SearchInfo.begin_y++;
//        if (SearchInfo.begin_x<0)
//          SearchInfo.begin_x=0;
      }
      if (SetSearchEnd(Storage, &SearchInfo)<OK)
        break; // can't find more

      if (SearchInfo.flags & WILDCARD) {
        searchfind=SearchWild(Storage, &SearchInfo, &SearchInfo.replength);
        if (SearchInfo.found_x ==-1) {
          ret=CANT_FIND_MORE;
          break;
        }
        SearchInfo.replacebuffer=searchfind;
      } else {
        if (SearchInfo.flags&FORWARD)
          DoSearch(&SearchInfo, BUF(using_fact));
        else
          DoSearchBack(&SearchInfo, BUF(using_fact));
        SearchInfo.find_direct=FALSE;
        if (SearchInfo.found_x ==-1) {
          ret=CANT_FIND_MORE;
          break;
        }
        SearchInfo.last_x=SearchInfo.found_x;
        SearchInfo.last_y=SearchInfo.found_y;
      }
      if (!turbo) {
        int loop=TRUE;
        SetScreen(Storage, SearchInfo.found_x, SearchInfo.found_y);
        if (LFLAGS(SearchInfo.found_y)&FOLD_HIDDEN)
          FoldShow(Storage, SearchInfo.found_y);
        CursorOnOff=TRUE;
        CursorXY(Storage, BUF(cursor_x), BUF(cursor_y));
        Showplace(Storage);
        if (variant==2 || (variant==-1 && !(SearchInfo.flags & PROMPT_REPLACE))) {
          stop=TRUE;
        } else if (global || variant==1) {
          if (GetKey(Storage, NULL)>=0) {
            if (buffer[0]!='g' && buffer[0]!='G') {
              stop=TRUE;
              next=TRUE;
            }
          }
        } else {
          Status(Storage, RetString(STR_REPLACE_OPTIONS), 0x83);
          while (loop) {
            if (GetKey(Storage, gkWAIT)<0)
              buffer[0]='q';
            if (buffer[0]==0x1B || buffer[0]=='q' || buffer[0]=='Q') {
              ret=FUNCTION_CANCEL;
              break;
            } else if (buffer[0]=='n' || buffer[0]=='N' || buffer[0]=='\r') {
              next=TRUE;
              loop=FALSE;
            } else if (buffer[0]=='l' || buffer[0]=='L') {
              stop=TRUE;
              loop=FALSE;
            } else if (buffer[0]=='a' || buffer[0]=='A') {
//              Status(Storage, RetString(STR_GLOBAL_REPLACE_WORKING), 3);
              if (SearchInfo.flags&FORWARD) {
                SaveCursorPos(Storage, &cpos);
                ret=BeginTurbo(Storage, &TT, &SearchInfo);
                if (ret<OK) {
                  stop=TRUE;
                  next=TRUE;
                  break;
                } else
                  Visible=VISIBLE_OFF;
                undoappend=TRUE;
                turbo=TRUE;
                loop=FALSE;
              } else
                buffer[0]='g';
            }
            if (buffer[0]=='g' || buffer[0]=='G') {
              Status(Storage, RetString(STR_BREAK_REPLACE_WITH_SPACE), 3);
              allowcleaning=FALSE;
              global=TRUE;
              loop=FALSE;
            } else if (buffer[0]=='y' || buffer[0]=='Y' || buffer[0]=='\n') {
              loop=FALSE;
            }
          }
          if (ret<OK)
            break; //while(!stop)
        }
      }

      if (!next) {
        if (turbo) {
          turbo_replaced=TRUE;
          ret=ReplaceTurboWord(Storage, &SearchInfo, &TT);
//          if (!(SearchInfo.flags&WILDCARD)) //960329
            addforward=-1;
      } else {
          ReplaceWord(Storage, SearchInfo.found_x,SearchInfo.found_y,
  	       		SearchInfo.last_x,SearchInfo.last_y,
            		SearchInfo.buflength, SearchInfo.replacebuffer,
            		SearchInfo.replength, undosort2);
          undosort2=undoappend;
          addforward=-1;
        }
      }
      if (searchfind) {
        Dealloc((char *)searchfind);
        searchfind=NULL;
      }
      if (Visible==VISIBLE_ON && !next && BUF(window) &&
          BUF(window)->Visible==VISIBLE_ON) {
        UpdateScreen(Storage);
        Showplace(Storage);
      }
      SearchInfo.find_direct=FALSE;
    }
    if (searchfind) {
      Dealloc(searchfind);
    }
  }
  if (turbo) {
    if (turbo_replaced && (ret>=OK || ret==CANT_FIND_MORE)) {
      TextStruct *newrad;
      int len=TT.firstpos-SHS(fileblock);
      int afterlen=SHS(fileblock)+SHS(size)-TT.currentread;
      if (len+afterlen+TT.currentwrite-TT.newtext>=TT.newsize) {
        TT.newsize=len+afterlen+TT.currentwrite-TT.newtext;
        TT.currentwrite=TT.currentwrite-(int)TT.newtext;
        TT.newtext=Realloc(TT.newtext, TT.newsize);
        TT.currentwrite=TT.currentwrite+(int)TT.newtext;
      }
      if (TT.newtext) {
        movmem(TT.newtext, TT.newtext+len, TT.newsize-len);
        memcpy(TT.newtext, SHS(fileblock), len);
        TT.currentwrite+=len;
    
        memcpy(TT.currentwrite, TT.currentread, afterlen);
        TT.currentwrite+=afterlen;
    
        ret=ExtractLines(TT.newtext, TT.currentwrite-TT.newtext, &newrad, &len, NULL);
        if (ret>=OK) {
          BUF(curr_topline)=1;
          BUF(cursor_y)=1;
          BUF(curr_line)=1;
          BUF(cursor_x)=1;
          BUF(string_pos)=0;
          UndoAdd(Storage, SHS(fileblock), undoNORMAL|undoNEWTEXT, SHS(size));
          if (len==SHS(line)) {
            register int count;
            for (count=SHS(line); count>0; count--) {
              newrad[count].fold_no=FOLD(count);
              newrad[count].flags=LFLAGS(count);
            }
          }
          Dealloc(SHS(text));
          Dealloc(SHS(fileblock));
          SHS(text)=newrad;
          SHS(line)=len;
          SHS(taket)=len;
          SHS(fileblock)=TT.newtext;
          SHS(fileblocklen)=TT.newsize;
          SHS(size)=TT.currentwrite-TT.newtext;
        } else {
          Dealloc(TT.newtext);
        }
      } else {
        ret=OUT_OF_MEM;
      }
      RestoreCursorPos(Storage, &cpos, TRUE);
      if (ret>=OK) {
        UpdateScreen(Storage);
      }
    } else
      Dealloc(TT.newtext);
  }
  if (SearchInfo.flags & WILDCARD) {
    if (searchstring)
      Search.second_buf=SearchInfo.buf;
    else {
      if (Search.pattern_compiled!=SearchInfo.pattern_compiled) {
        Search.pattern_compiled=SearchInfo.pattern_compiled;
        Search.buf=SearchInfo.buf;	// Copy pattern
      }
    }
  }
  if (next && (SearchInfo.flags&FORWARD))
    MoveCursor(Storage, -1);
  if (!allowcleaning) {
    allowcleaning=TRUE;
    DeallocLine(Storage, 0);
  }
  if (allocatedreplacestring)
    Dealloc(replacestringbuffer);
  if (allocatedsearchstring)
    Dealloc(searchstringbuffer);
  Search.lastpos=BUF(curr_line)+BUF(string_pos)+(ret<OK);
  return(ret);
}

/***********************************************************
 *
 * int ReplaceWord(...)
 *
 * Replacear ett ord med ett annat.  sstringlen adderas till slutpositionen.
 * Returnar antalet relativt deletade tecken.
 ******/
int __regargs ReplaceWord(BufStruct *Storage, int xpos, int rad, int xstop, int ystop,
                int sstringlen, char *rstring, int rstringlen, int undoadd)
{
  String retstring;
  int undoinput;
  int rettis=0;
  int tempvisible=Visible;

//FPrintf(Output(), "xpos %ld, rad %ld, xstop %ld, ystop %ld, sstringlen %ld, rstring '%s', rstringlen %ld\n",
//        xpos, rad, xstop, ystop, sstringlen, rstring, rstringlen);

  Visible=VISIBLE_OFF;
  SetScreen(Storage,xstop,ystop);
  if (sstringlen)
    MoveCursor(Storage, sstringlen);

  if (sstringlen || xpos!=xstop || rad!=ystop) {
    if (BackspaceUntil(Storage, xpos, rad, &retstring.string, &retstring.length)<OK)
      return(0);   // Error
    rettis=retstring.length;

    if (undoadd)
      undoinput=undoAPPENDBEFORE;
    else
      undoinput=undoNORMAL;
    UndoAdd(Storage, retstring.string, undoinput, retstring.length);
    Dealloc(retstring.string);
  }
  if (rstringlen) {
    retstring.length=OutputText(Storage, rstring, rstringlen, TRUE, NULL);
    UndoAdd(Storage, (char *)retstring.length, undoAPPENDBEFORE|undoONLYBACKSPACES, 0);
  } else
    retstring.length=0;

  BUF(wanted_x)=0;
  Visible=tempvisible;
  return(rettis-retstring.length);
}

int __regargs ReplaceTurboWord(BufStruct *Storage,
                               SearchStruct *SearchInfo,
                               turbostruct *TT)
{
  int ret=OK;
  int xpos=SearchInfo->found_x, rad=SearchInfo->found_y;
  char *rstring=SearchInfo->replacebuffer;
  int rstringlen=SearchInfo->replength;
  
  if (TT->newtext) {
    {
      register char *currentpos;
      register int currentlen;
      currentpos=RAD(rad)+xpos;
      currentlen=currentpos-TT->currentread;
      if (TT->currentwrite-TT->newtext+currentlen+rstringlen >= TT->newsize) {
        char *nytext;
        TT->newsize=rstringlen*10+TT->currentwrite-TT->newtext+currentlen+
                    SHS(fileblock)+SHS(fileblocklen)-TT->currentread;
        nytext=Remalloc(TT->newtext, TT->newsize);
        if (!nytext) {
          TT->newtext=NULL;
          return(OUT_OF_MEM);
        }
        TT->currentwrite=TT->currentwrite-TT->newtext+nytext;
        TT->newtext=nytext;
      }
      memcpy(TT->currentwrite, TT->currentread, currentlen);
      TT->currentwrite+=currentlen;
      if (SearchInfo->flags&WILDCARD)
        TT->currentread=RAD(SearchInfo->last_y)+SearchInfo->last_x;
      else
        TT->currentread+=currentlen+SearchInfo->buflength;
    }
    memcpy(TT->currentwrite, rstring, rstringlen);
    TT->currentwrite+=rstringlen;
    if (SearchInfo->flags&WILDCARD) {
      BUF(string_pos)=SearchInfo->last_x;
      BUF(curr_line)=SearchInfo->last_y;
      BUF(cursor_y)=SearchInfo->last_y-BUF(curr_topline)+1;
    } else {
      BUF(string_pos)=xpos+SearchInfo->buflength;
      BUF(curr_line)=rad;
      BUF(cursor_y)=rad-BUF(curr_topline)+1;
      while (BUF(string_pos)>LEN(rad)) {
        BUF(string_pos)-=LEN(rad);
        BUF(cursor_y)++;
        BUF(curr_line)++;
        rad++;
      }
    }
  } else
    ret=OUT_OF_MEM;
  return(ret);
}


int __regargs BeginTurbo(BufStruct *Storage, turbostruct *TT, SearchStruct *SearchInfo)
{
  int ret;
  ret=CompactBuf(BUF(shared), NULL);
  if (ret>=OK) {
    TT->firstpos=RAD(BUF(curr_line))+BUF(string_pos);
    TT->newsize=SHS(size)+(SearchInfo->replength-SearchInfo->buflength>0?(SearchInfo->replength-SearchInfo->buflength)*8:0);
    TT->newtext=Malloc(TT->newsize);
    TT->currentread=TT->firstpos;
    TT->currentwrite=TT->newtext;
    if (!TT->newtext)
      ret=OUT_OF_MEM;
  }
  return(ret);
}

