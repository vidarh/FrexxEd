/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */

#include <stdio.h>
#include <string.h>

#include "Buf.h"
#include "FACT.h"
#include "Face.h"
#include "Fold.h"
#include "UpdtScreenC.h"
#include "UpdtScreen.h"

#ifdef USE_C_SCREEN_SETUP


extern int UpDtNeeded;

static char nummer[8];


static void __regargs FixLinesCount(BufStruct *Storage, long line)
{
  char *str=nummer;
  long rest;
  ((long *)nummer)[0]=32*(1+256+65536+16777216);
  ((long *)nummer)[1]=32*(1+256+65536+16777216);
  *str++=' ';
  do {
    rest=line%10;
    line=line/10;
    *str++=rest+'0';
  } while (line);
}

static void __regargs AddLinesCount(void)
{
  char *str=nummer;

  do {
    str++;
    switch (*str) {
    case '9':
      *str='0';
      break;
    case ' ':
      *str='1';
      return;
    default:
      *str=(*str)+1;
      return;
    }
  } while (1);
}


static int __regargs FixLine(BufStruct *Storage, unsigned char *text, int length,
                             struct screen_buffer *dest, int offset, int line,
                             int view_line, BOOL face)
{
  int retlen=0;
  int textpos=0;
  
  int startoffset=offset;
  int screen_width=BUF(screen_col);

  
  if (line<=SHS(line)) {
    if(face) {
      if (BUF(face_top_updated_line)>SHS(face_updated_line))
        BUF(face_top_updated_line)=SHS(face_updated_line);
      if (BUF(face_bottom_updated_line)>SHS(face_updated_line))
        BUF(face_bottom_updated_line)=SHS(face_updated_line);
      if (BUF(face_top_updated_line)) {
        while (BUF(face_top_updated_line)<line) {
//FPrintf(Output(), "updated %ld, line %ld (top %ld, bottom %ld)\n", SHS(face_updated_line), line, SHS(face_top_updated_line), SHS(face_bottom_updated_line));
          StringMatchInit(Storage, RAD(BUF(face_top_updated_line)), dest,
                          offset, BUF(face_top_updated_line));
          {
            int count;
            for (count=0; count<LEN(BUF(face_top_updated_line)); count++)
              StringMatch(RAD(BUF(face_top_updated_line))[count], count, -1);
          }
          StringMatchEnd(Storage, BUF(face_top_updated_line));
          BUF(face_top_updated_line)++;
        }
        if (SHS(face_updated_line)<BUF(face_top_updated_line))
          SHS(face_updated_line)=BUF(face_top_updated_line);
      }
      StringMatchInit(Storage, text, dest, offset, line);
    }
    if (BUF(fold_len)) {
      WORD count=BUF(fold_len);
      char tkn=' ';
      char style=0;
      short color=1;
      if (BUF(using_fact)->extra[FACT_NOFOLD_CHAR].length) {
        style=BUF(using_fact)->extra[FACT_NOFOLD_CHAR].string[0];
        tkn=BUF(using_fact)->extra[FACT_NOFOLD_CHAR].string[1];
        color=*(short *)&BUF(using_fact)->extra[FACT_NOFOLD_CHAR].string[2];
      }
      while (--count>=0 && screen_width--) {
        dest->text[offset]=tkn;
        dest->control[offset]=style;
        dest->colors[offset]=color;
        offset++;
        dest->linebuffer[retlen]=-1;
        retlen++;
      }
    }
    if (BUF(l_c_len)) {
      WORD count=BUF(l_c_len);
      char *str=&nummer[count];
      while (--count>=0 && screen_width--) {
        dest->text[offset]=*--str;
        dest->control[offset]=0;
        dest->colors[offset]=1;
        offset++;
        dest->linebuffer[retlen]=-1;
        retlen++;
      }
      AddLinesCount();
    }
  }
  {
    register unsigned int curr_x=-BUF(screen_x)-1;
    {
      register short tabcount=0;
      register char *flags=BUF(using_fact->flags);
      while (--length>=0) {
        long tkn;
        tkn=*text++;
        if (!flags[tkn]) {		// Normalt tecken
          if (++curr_x<screen_width) {
            dest->text[offset]=tkn;
            dest->control[offset]=0;
            dest->colors[offset]=1;
            offset++;
            dest->linebuffer[retlen]=textpos;
            retlen++;
          }
          tabcount--;
        } else {			// Specialtecken
          WORD size, len;
          long *str;
          long *start;
          switch (flags[tkn]&(fact_TAB|fact_STRING)) {
          case fact_TAB:
            size=(tabcount%(short)BUF(tabsize))+(short)BUF(tabsize);
            len=1;
            start=&tkn;
            tabcount=0;
            break;
          case fact_TAB|fact_STRING:
            len=BUF(using_fact)->length[tkn];
            start=(long *)BUF(using_fact)->strings[tkn];
            size=(tabcount%(short)BUF(tabsize))+(short)BUF(tabsize);
            tabcount=0;
            break;
          case fact_STRING:
            len=BUF(using_fact)->length[tkn];
            size=len;
            tabcount-=size;
            start=(long *)BUF(using_fact)->strings[tkn];
            break;
          }
          {
            WORD count=0;
            while (--size>=0) {
              if (--count<=0) {
                count=len;
                str=start;
              } else
                str++;
              if (++curr_x<screen_width) {
                dest->text[offset]=((char *)str)[1];
                dest->control[offset]=((char *)str)[0];
                dest->colors[offset]=((short *)str)[1];
                offset++;
                dest->linebuffer[retlen]=textpos;
                retlen++;
              }
            }
          }
        }
        if(face) {
          StringMatch(tkn, textpos, offset);
          textpos++;
        }
      }
    }
    if ((int)curr_x>=screen_width) {  // Sätt ut LineContinuation
      WORD count=BUF(using_fact)->extra[FACT_NO_EOL].length;
      long *str=(long *)BUF(using_fact)->extra[FACT_NO_EOL].string;
      offset-=count;
      while (--count>=0) {
        dest->text[offset]=((char *)str)[1];
        dest->control[offset]=((char *)str)[0];
        dest->colors[offset]=((short *)str)[1];
        offset++;
        str++;
      }
    }
    if (line<=SHS(line)) {
      if (line==SHS(line)) { // Sätt ut EndOfFile
        WORD count=BUF(using_fact)->extra[FACT_EOF].length;
        long *str=(long *)BUF(using_fact)->extra[FACT_EOF].string;
        while (--count>=0) {
          if (++curr_x<screen_width) {
            dest->text[offset]=((char *)str)[1];
            dest->control[offset]=((char *)str)[0];
            dest->colors[offset]=((short *)str)[1];
            offset++;
            retlen++;
          }
          str++;
        }
      }
      if (FOLD(line)) {
        WORD depth=FOLD(line);
        WORD rest=depth;
        char style=0;
        short color=1;
        short foldtkn=-1;
  
        if (LFLAGS(line)&FOLD_BEGIN) {
          if (BUF(using_fact)->extra[FACT_FOLDED_CHAR].length) {
            style=BUF(using_fact)->extra[FACT_FOLDED_CHAR].string[0];
            foldtkn=BUF(using_fact)->extra[FACT_FOLDED_CHAR].string[1];
            color=*(short *)&BUF(using_fact)->extra[FACT_FOLDED_CHAR].string[2];
          }
        } else {
          if (BUF(using_fact)->extra[FACT_FOLDUP_CHAR].length) {
            style=BUF(using_fact)->extra[FACT_FOLDUP_CHAR].string[0];
            foldtkn=BUF(using_fact)->extra[FACT_FOLDUP_CHAR].string[1];
            color=*(short *)&BUF(using_fact)->extra[FACT_FOLDUP_CHAR].string[2];
          }
        }
        if (foldtkn>=0) {
          if (depth>=10) {
            if (depth>=100) {
              if (depth>=1000) {
                if (depth>=10000) {
                  rest=depth/10000;
                  depth=depth%10000;
                  dest->text[startoffset]=rest+'0';
                  dest->control[startoffset]=style;
                  dest->colors[startoffset]=color;
                  startoffset++;
                }
                rest=depth/1000;
                depth=depth%1000;
                dest->text[startoffset]=rest+'0';
                dest->control[startoffset]=style;
                dest->colors[startoffset]=color;
                startoffset++;
              }
              rest=depth/100;
              depth=depth%100;
              dest->text[startoffset]=rest+'0';
              dest->control[startoffset]=style;
              dest->colors[startoffset]=color;
              startoffset++;
            }
            rest=depth/10;
            depth=depth%10;
            dest->text[startoffset]=rest+'0';
            dest->control[startoffset]=style;
            dest->colors[startoffset]=color;
            startoffset++;
          }
          dest->text[startoffset]=rest+'0';
          dest->control[startoffset]=style;
          dest->colors[startoffset]=color;
          startoffset++;
          dest->text[startoffset]=foldtkn;
          dest->control[startoffset]=style;
          dest->colors[startoffset]=color;
          startoffset++;
          if (startoffset>offset) {
            retlen+=startoffset-offset;
            offset=startoffset;
          }
        }
      }
    }
  }
  if (line<=SHS(line)) {
    if(face) {
      if (SHS(face_updated_line)<line)
        SHS(face_updated_line)=line;
      if (BUF(face_top_updated_line)<line)
        BUF(face_top_updated_line)=line;
      if (BUF(face_bottom_updated_line)<=BUF(face_top_updated_line)) {
        BUF(face_bottom_updated_line)=0;
        BUF(face_top_updated_line)=0;
      }

     /* Ändrad style och inte sista raden? */
      if (!StringMatchEnd(Storage, line) && SHS(line)>line) {
        if (BUF(face_top_updated_line)<SHS(face_updated_line)) {
//          Updated_Face(BUF(shared), line);
          Updated_Face(BUF(shared), line+1);
        }
      }
      /* Dom har mötts, nollställ */
      if (BUF(face_top_updated_line)>BUF(face_bottom_updated_line)) {
        BUF(face_top_updated_line)=0;//SHS(face_updated_line);
        BUF(face_bottom_updated_line)=0;//SHS(face_updated_line);
      }
    }
  }
  view_line--;
  {
    int templen=BUF(window)->UpdtVars.EndPos[view_line].len;
    BUF(window)->UpdtVars.EndPos[view_line].len=retlen;
    if (templen>retlen) {
      BUF(window)->UpdtVars.EndPos[view_line].eraseline=1;
/*
    while (templen>retlen) {
      *dest++=0x0000+' '; // tomt space
      retlen++;
    }
*/
    }
  }
  retlen++;
  dest->control[offset++]=cb_NEWLINE; //Newline
  dest->control[offset]=cb_EOS;  //End of string
  return retlen;
}


int __regargs UpdtOneLineC(BufStruct *Storage, TextStruct *text,
                           struct screen_buffer *dest, int viewline, int radnummer)
{
  int antal;
  if (BUF(l_c_len))
    FixLinesCount(Storage, radnummer);

  antal=FixLine(Storage, text->text, text->length, dest, 0,
                radnummer, viewline+BUF(top_offset)-1, FALSE);

//FPrintf(Output(), "Result OneLine updated %ld, (top %ld, bottom %ld) %ld\n", SHS(face_updated_line), SHS(face_top_updated_line), SHS(face_bottom_updated_line), face_update);
  return antal;
}

int __regargs UpdtLinesC(BufStruct *Storage, struct screen_buffer *dest,
                         int topline, int lines, int topoffset)
{
  unsigned char *rad;
  int length;
  int totallength=0;

  topoffset+=BUF(top_offset);

  if (BUF(l_c_len))
    FixLinesCount(Storage, topline);

  while (lines) {
    if (topline>SHS(line)) {
      rad=NULL;
      length=0;
    } else {
      rad=RAD(topline);
      length=LEN(topline);
    }

 /* Är raden utanför så skapas en tom rad.  Är den foldad så ska den hoppas över. */
    if (topline>SHS(line) || !(LFLAGS(topline)&FOLD_HIDDEN)) {
      length=FixLine(Storage, rad, length, dest, totallength, topline, topoffset,
                     SHS(face)?TRUE:FALSE);
      totallength+=length;
      topoffset++;
      lines--;
    } else {
      if (BUF(l_c_len))
        AddLinesCount();
    }
    topline++;
  }
//FPrintf(Output(), "Result More Lines updated %ld, (top %ld, bottom %ld) %ld\n", SHS(face_updated_line), SHS(face_top_updated_line), SHS(face_bottom_updated_line), face_update);
  return totallength;
}

void __regargs Updated_Face(SharedStruct *shared, int line)
{
  if (shared->face) {
    BufStruct *Storage=shared->Entry;
    while (Storage) {
      if (!BUF(face_top_updated_line) ||
          BUF(face_top_updated_line)>=line) {
        BUF(face_top_updated_line)=line;
        face_update=TRUE;
      }
      if (shared->line>line &&
          BUF(face_bottom_updated_line)<=line) {
        BUF(face_bottom_updated_line)=line;
        face_update=TRUE;
      }
      Storage=BUF(NextSplitBuf);
    }
  }
//FPrintf(Output(), "Update_Face line %ld, (top %ld, bottom %ld) %ld\n", line, shared->face_top_updated_line, shared->face_bottom_updated_line, face_update);
}

#endif // USE_C_SCREEN_SETUP

