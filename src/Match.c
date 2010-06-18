/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************
*
* Match.c
*
* Match parens...
*
********/

#include <exec/types.h>
#include <stdio.h>
#include "Buf.h"
#include "UpdtScreen.h"
#include "FACT.h"

/********************************************************************
*
* char MatchChar(BufStruct *);
*
* Letar reda på rätt korresponderande tecken till det man står på.
* Retunerar FALSE om den inte fick napp!
*
******/
char __regargs MatchChar(BufStruct *Storage)
{
  int line=BUF(curr_line), byte=BUF(string_pos), find;
  char *text=RAD(line)+byte, found;
  char one, two;

  if (!text)
    return(FALSE);

  one=*text;
  two=BUF(using_fact)->delimit[*text];
  find=0;
  found=FALSE;
  if (BUF(using_fact)->xflags[*text] & factx_CLASS_CLOSE) {
    while(text && line>=1) {
      if(*text==two) {
        if(!--find) {
          found=TRUE;
          break;
        }
      } else if(*text==one) {
        find++;
      }
      if(byte>0) {
        byte--;
        text--;
      } else {
        line--;
        byte=LEN(line)-1;
        text=RAD(line)+byte;
      }
    }
  } else if (BUF(using_fact)->xflags[*text] & factx_CLASS_OPEN) {
     while(text && line<=SHS(line)) {
       if(*text==two) {
         if(!--find) {
           found=TRUE;
           break;
         }
       } else if(*text==one) {
         find++;
       }
       if(byte<(LEN(line)-(line!=SHS(line)?1:0))) {
         byte++;
         text++;
       } else {
         line++;
         byte=0;
         text=RAD(line);
       }
     }
   }

  if(found)
    SetScreen(Storage, byte, line);
  return(found);
}