/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************
*
*       Search
*
********/

#ifdef AMIGA
#include <clib/exec_protos.h>
#include <exec/tasks.h>
#include <exec/types.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <proto/FPL.h>
#endif

#include <limits.h>
#include <math.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "DoSearch.h"
#include "Edit.h"
#include "Fold.h"
#include "UpdtScreen.h"

extern srch Search;          /* search structure */
extern void *Anchor;
extern DefaultStruct Default;
extern BOOL searchcompiled;
extern int searchcompile_flags;
extern char buffer[];
extern char search_fastmap[256];
extern char GlobalEmptyString[];

extern int retval_antal;
extern int retval_params[];

/* Defined in SearchHistory.c */
extern void ClearHistory();

/*  Start Searching  */
int SearchFor(BufStruct *Storage, int argc, char **argv)
{
  SearchStruct SearchInfo;
  char *searchstring=NULL;
  int ret=OK;
  char *searchstringbuffer=NULL;

  SearchInfo.flags=Search.flags;
  SearchInfo.range=INT_MAX-10;
  if (argc>0) {
    searchstring=argv[0];
    if (argc>1) {
      SearchInfo.flags=MakeSearchFlag(argv[1], SearchInfo.flags);
      if (argc>2) {
        SearchInfo.range=(int)argv[2];
      }
    }
  }

  if (searchstring && !searchstring[0])
    searchstring=NULL;

  if (!searchstring && !Search.buffer) {
    ret=SearchAsk(Storage, FALSE);
    SearchInfo.flags=Search.flags;
  }

  if (ret==OK) {
    SearchInfo.begin_y=BUF(curr_line);
    SearchInfo.begin_x=BUF(string_pos);
    ret=SetSearchEnd(Storage, &SearchInfo);
    SearchInfo.find_direct=FALSE;
    if ((searchstring && strcmp(searchstring, Search.lastsearchstring)) ||
        (Search.lastpos!=1 && (SearchInfo.begin_y+SearchInfo.begin_x)==1)) {
      Search.lastpos=1;
      SearchInfo.find_direct=TRUE;
    }
    SearchInfo.text=SHS(text);
    SearchInfo.lines=SHS(line);
    Dealloc(Search.lastsearchstring);
    if (searchstring) {
      Search.lastsearchstring=Strdup(searchstring);
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
        SearchInfo.buflength=1;
        SearchInfo.realbuffer=searchstring;
      } else {
        searchstringbuffer=Strdup(searchstring);
        if (searchstringbuffer) {
          SearchInfo.buflength=fplConvertString(Anchor, searchstringbuffer, searchstringbuffer);
          SearchInfo.buffer=searchstringbuffer;
          searchcompiled=FALSE;
          if (SearchInfo.flags & FORWARD) {
            BMS_init_forward(SearchInfo.buffer, SearchInfo.buflength,
                             (SearchInfo.flags&CASE_SENSITIVE)?NULL:BUF(using_fact),
                             search_fastmap);
          } else {
            BMS_init_backward(SearchInfo.buffer, SearchInfo.buflength,
                              (SearchInfo.flags&CASE_SENSITIVE)?NULL:BUF(using_fact),
                              search_fastmap);
          }
        } else
          ret=OUT_OF_MEM;
      }
    } else {
      Search.lastsearchstring=GlobalEmptyString;
      SearchInfo.buffer=Search.buffer;
      SearchInfo.buflength=Search.buflen;
      if (SearchInfo.flags & WILDCARD) {
        SearchInfo.realbuffer=Search.realbuffer;
        SearchInfo.pattern_compiled=Search.pattern_compiled;
        SearchInfo.buf=Search.buf;	// Copy the pattern
      } else {
        if (!searchcompiled || searchcompile_flags!=SearchInfo.flags) {
          searchcompile_flags=SearchInfo.flags;
          searchcompiled=TRUE;
          if (SearchInfo.flags & FORWARD)
            BMS_init_forward(SearchInfo.buffer, SearchInfo.buflength, (SearchInfo.flags&CASE_SENSITIVE)?NULL:BUF(using_fact), search_fastmap);
          else
            BMS_init_backward(SearchInfo.buffer, SearchInfo.buflength, (SearchInfo.flags&CASE_SENSITIVE)?NULL:BUF(using_fact), search_fastmap);
        }
      }
    }
    if (SearchInfo.buflength && ret>=OK) {
      if (SearchInfo.flags & WILDCARD)
        SearchWild(Storage, &SearchInfo, NULL);
      else {
        if (SearchInfo.flags & FORWARD)
          DoSearch(&SearchInfo, BUF(using_fact));
        else
          DoSearchBack(&SearchInfo, BUF(using_fact));
        retval_antal=2;
        retval_params[0]=SearchInfo.last_x;
        retval_params[1]=SearchInfo.last_y;
      }
      if (SearchInfo.found_x !=-1) {
        SetScreen(Storage,SearchInfo.found_x,SearchInfo.found_y);
        if (LFLAGS(SearchInfo.found_y)&FOLD_HIDDEN)
          FoldShow(Storage, SearchInfo.found_y);
        BUF(wanted_x)=BUF(cursor_x)+BUF(screen_x);
      } else
        ret=CANT_FIND_MORE;
    } else
      ret=CANT_FIND_MORE;
  }
  if (SearchInfo.flags & WILDCARD) {
    if (searchstring) {
      Search.second_buf=SearchInfo.buf;
    } else {
      if (Search.pattern_compiled!=SearchInfo.pattern_compiled) {
        Search.pattern_compiled=SearchInfo.pattern_compiled;
        Search.buf=SearchInfo.buf;	// Copy the pattern
      }
    }
  }
  Dealloc(searchstringbuffer);
  Search.lastpos=BUF(curr_line)+BUF(string_pos)+(ret<OK);
  return(ret);
}

/******************************************************************
 * 
 *  SearchWild(BufStruct *, SearchStruct *)
 *
 *
 ***********/
char *SearchWild(BufStruct *Storage, SearchStruct *SearchBase, int *returnwanted)
{
  struct re_registers Regs;
  struct re_textdata pos;
  int i;
  char *retstring=NULL;
  

  SearchBase->buf.fact = (SearchBase->flags&CASE_SENSITIVE)?NULL:BUF(using_fact);

  if (!SearchBase->pattern_compiled) {
    re_compile_pattern (SearchBase->realbuffer, &SearchBase->buf);
    re_compile_fastmap (&SearchBase->buf);
    SearchBase->pattern_compiled=TRUE;
  }
  SearchBase->found_x=-1;

  if (PointInit(&pos,
                SearchBase,
                SearchBase->begin_y-1,		/* set start line search position, */
                SearchBase->begin_x,		/* set start column search position, */
                &SearchBase->text[1],		/* array */
                SearchBase->lines,		/* num of lines */
                SearchBase->end_y-1,		/* end line */
                SearchBase->end_x,		/* end column */
                SearchBase->flags&FORWARD)) {	/* Forward or backward */
    pos.searchflags=SearchBase->flags;
    i = re_search(&SearchBase->buf, 
                  (SearchBase->flags & FORWARD)?SearchBase->range:-SearchBase->range,
                  &Regs, &pos.pos);
    if (i>-1) {
      SearchBase->found_y=Regs.start_line[0]+1;
      SearchBase->found_x=Regs.start[0];
      SearchBase->last_y=Regs.end_line[i]+1;
      retval_params[1]=SearchBase->last_y;
      SearchBase->last_x=Regs.end[i];
      retval_params[0]=SearchBase->last_x;
      retval_antal=2;
#if 0
FPrintf(Output(), "Search start %ld-%ld, end %ld-%ld, last %ld-%ld\n",
  SearchBase->begin_y, SearchBase->begin_x,
  SearchBase->end_y, SearchBase->end_x,
  SearchBase->last_y, SearchBase->last_x);
#endif
      if (SearchBase->flags & INSIDE_BLOCK) {
        if (SearchBase->flags&FORWARD) {
          if (SearchBase->last_y==SearchBase->end_y &&
              SearchBase->last_x>SearchBase->end_x) {
            SearchBase->found_x=-1;
            goto fail;
          }
        } else {
          if (SearchBase->last_y==SearchBase->begin_y &&
              SearchBase->last_x>SearchBase->begin_x) {
            SearchBase->found_x=-1;
            goto fail;
          }
        }
      }
      if (SearchBase->last_y==SearchBase->lines-1 &&
          !SearchBase->text[SearchBase->lines].length &&
          SearchBase->last_x >= SearchBase->text[SearchBase->last_y].length) {
        SearchBase->last_x=0;
        SearchBase->last_y++;
      }
      if (returnwanted) {
        retstring=re_replace(&SearchBase->text[1], 
                             &Regs,
                             SearchBase->realreplacebuffer,
                             returnwanted);
      }
    }
  }
fail:
  return(retstring);
}

int SearchMatch(BufStruct *Storage, char *search, char *replace, String *ret, int range, char *flags)
{
  char *retstring;
  int length;
  SearchStruct SearchInfo;

  memset(&SearchInfo, 0, sizeof(SearchStruct));

  SearchInfo.range=range;
  if (range<0)
    SearchInfo.range=INT_MAX-10;
  SearchInfo.flags=(Search.flags&LIMIT_WILDCARD); //|FORWARD;
  if (flags[0])
    SearchInfo.flags=MakeSearchFlag(flags, SearchInfo.flags);
  SearchInfo.flags|=WILDCARD;
  SearchInfo.flags&=~INSIDE_BLOCK;

  if (!range || (SearchInfo.flags&FORWARD)) {
    SearchInfo.end_x = LEN(SHS(line));
    SearchInfo.end_y = SHS(line);
  } else {
    SearchInfo.end_x = 0;
    SearchInfo.end_y = 1;
  }
  SearchInfo.text=SHS(text);
  SearchInfo.lines=SHS(line);
  SearchInfo.realbuffer=search;
  SearchInfo.realreplacebuffer=replace;
  SearchInfo.buf.fastmap=buffer;
  SearchInfo.begin_x = BUF(string_pos);
  SearchInfo.begin_y = BUF(curr_line);
  SearchInfo.find_direct=TRUE;

  retstring=SearchWild(Storage, &SearchInfo, &length);
  Dealloc(SearchInfo.buf.buffer);
  Search.buf.buffer=NULL;
  Search.buf.allocated=0;

  if (retstring) {
    if (range) {
      SetScreen(Storage,SearchInfo.found_x,SearchInfo.found_y);
      if (LFLAGS(SearchInfo.found_y)&FOLD_HIDDEN)
        FoldShow(Storage, SearchInfo.found_y);
      BUF(wanted_x)=BUF(cursor_x)+BUF(screen_x);
    }
    ret->string=retstring;
    ret->length=length;
    return(OK);
  }
  return(CANT_FIND_MORE);
}

/***************************************************
 *
 *  SetSearch
 *
 *  Om searchstring/replacestring är lika med NULL så sätts inte den strängen om,
 *  Om searchstring pekar på tom sträng så rörs den inte.
 *
 *  Strängarna ska vara i 'C-style'.
 *
 *  flags:  "wcofbpli+", "w+f+", "wc+f-"
 *          Sätter bara om de flaggor som anges.  '+'/'-' efter flaggorna
 *          '=' tömmer ALLA.
 *          säger om den ska sättas på/av.
 **********/
void SetSearch(char *flags, char *searchstring, char *replacestring)
{

  if (replacestring) {
    HistoryAdd(Search.repbuffer, TRUE);
    Dealloc(Search.repbuffer);
    Search.realrepbuffer=Strdup(replacestring);
    HistoryAdd(Strdup(replacestring), TRUE);
    Search.repbuflen=fplConvertString(Anchor, replacestring, replacestring);
    Search.repbuffer=Malloc(Search.repbuflen);
    if (!Search.repbuffer)
      Search.repbuflen=0;
    memcpy(Search.repbuffer, replacestring, Search.repbuflen);
  }
  if (searchstring && searchstring[0]) {
    HistoryAdd(Search.realbuffer, TRUE);
    Dealloc(Search.buffer);
    Search.pattern_compiled=FALSE;
    searchcompiled=FALSE;
    Dealloc(Search.buf.buffer);
    Search.buf.buffer=NULL;
    Search.buf.allocated=0;
    Search.realbuffer=Strdup(searchstring);
    HistoryAdd(Strdup(searchstring), TRUE);
    Search.buflen=fplConvertString(Anchor, searchstring, searchstring);
    Search.buffer=Malloc(Search.buflen);
    if (!Search.buffer)
      Search.buflen=0;
    memcpy(Search.buffer, searchstring, Search.buflen);
  }
  if (flags) {
    Search.flags=MakeSearchFlag(flags, Search.flags);
    Default.search_flags=Search.flags;
    ConvertSearchFlags();
  }

}

int __regargs MakeSearchFlag(char *flagstring, int currentflags)
{
  BOOL clear_history=FALSE;
  if (flagstring) {
    int flag=0;
    while (flagstring[0]) {
      switch(flagstring[0]) {
      case 'w':
        flag|=WILDCARD;
        break;
      case 'c':
        flag|=CASE_SENSITIVE;
        break;
      case 'o':
        flag|=ONLY_WORDS;
        break;
      case 'f':
        flag|=FORWARD;
        break;
      case 'b':
        flag|=INSIDE_BLOCK;
        break;
      case 'p':
        flag|=PROMPT_REPLACE;
        break;
      case 'l':
        flag|=LIMIT_WILDCARD;
        break;
      case 'i':
        flag|=INSIDE_FOLD;
        break;
      case '=':
        flag=0;
        currentflags=0;
        break;
      case '-':
        currentflags&=~flag;
        if (clear_history) {
            ClearHistory();
        }
        flag=0;
        clear_history=FALSE;
        break;
      case '+':
        currentflags|=flag;
        flag=0;
        clear_history=FALSE;
        break;
      case 'H':
        clear_history=TRUE;
        break;
      }
      flagstring++;
    }
  }
  return(currentflags);
}

void ConvertSearchFlags()
{
  Default.search_limit=(Search.flags&LIMIT_WILDCARD)?1:0;
  Default.search_wildcard=(Search.flags&WILDCARD)?1:0;
  Default.search_word=(Search.flags&ONLY_WORDS)?1:0;
  Default.search_block=(Search.flags&INSIDE_BLOCK)?1:0;
  Default.search_forward=(Search.flags&FORWARD)?1:0;
  Default.search_prompt=(Search.flags&PROMPT_REPLACE)?1:0;
  Default.search_fold=(Search.flags&INSIDE_FOLD)?1:0;
}


int SetSearchEnd(BufStruct *Storage, SearchStruct *SearchInfo)
{
  int ret=OK;
  BOOL backone=FALSE;
  if (BUF(block_exists) && (SearchInfo->flags & INSIDE_BLOCK)) {
    if (SearchInfo->flags&FORWARD) {
      SearchInfo->end_y=BUF(block_end_y);
      SearchInfo->end_x=Col2Byte(Storage, BUF(block_end_x), RAD(BUF(block_end_y)), LEN(BUF(block_end_y))+(SearchInfo->end_y==SHS(line)));//+((SearchInfo->flags & WILDCARD)?0:1);
      if ((BUF(curr_line)<BUF(block_begin_y) || 
           (BUF(curr_line)==BUF(block_begin_y) && BUF(cursor_x)+BUF(screen_x)<BUF(block_begin_x)))) {
        SearchInfo->begin_y = BUF(block_begin_y);
        SearchInfo->begin_x = Col2Byte(Storage, BUF(block_begin_x), RAD(BUF(block_begin_y)), LEN(BUF(block_begin_y)));
        backone=TRUE;
      }
    } else {
      SearchInfo->end_y=BUF(block_begin_y);
      SearchInfo->end_x=Col2Byte(Storage, BUF(block_begin_x), RAD(BUF(block_begin_y)), LEN(BUF(block_begin_y)));
      if ((BUF(curr_line)>BUF(block_end_y) || 
           (BUF(curr_line)==BUF(block_end_y) && BUF(cursor_x)+BUF(screen_x)>BUF(block_end_x)))) {
        SearchInfo->begin_y = BUF(block_end_y);
        SearchInfo->begin_x = Col2Byte(Storage, BUF(block_end_x), RAD(BUF(block_end_y)), LEN(BUF(block_end_y)));
      }
//      if (SearchInfo->flags&WILDCARD)
//        backone=TRUE;
    }
  } else {
    if (SearchInfo->flags&FORWARD) {
      SearchInfo->end_x=LEN(SHS(line));
      SearchInfo->end_y=SHS(line);
    } else {
      SearchInfo->end_x=0;
      SearchInfo->end_y=1;
//      if (SearchInfo->flags&WILDCARD)
//        backone=TRUE;
    }
  }
  if (backone) {	// Flytta bak början ett steg.
    SearchInfo->begin_x--;
    if (SearchInfo->begin_x<0) {
      if (SearchInfo->begin_y>1) {
        SearchInfo->begin_y--;
        SearchInfo->begin_x=LEN(SearchInfo->begin_y);
      } else
        SearchInfo->begin_x++;
    }
  }
  if (SearchInfo->flags&FORWARD) {
    if (SearchInfo->begin_y>SearchInfo->end_y ||
        (SearchInfo->begin_y==SearchInfo->end_y && SearchInfo->begin_x>SearchInfo->end_x))
      ret=CANT_FIND_MORE;
  } else {
    if (SearchInfo->begin_y<SearchInfo->end_y ||
        (SearchInfo->begin_y==SearchInfo->end_y && SearchInfo->begin_x<SearchInfo->end_x))
      ret=CANT_FIND_MORE;
  }
  return ret;
}

