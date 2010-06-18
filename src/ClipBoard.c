/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
*
* ClipBoard.c
*
* ClipBoard functions!
*
*******/

#include <proto/exec.h>
#include <proto/iffparse.h>
#include <libraries/iffparse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/commifmt.h>

#include "Buf.h"
#include "Alloc.h"
#include "Limit.h"

extern char buffer[];

#define ID_FTXT   MAKE_ID('F','T','X','T')
#define ID_CHRS   MAKE_ID('C','H','R','S')


/*************************************************************
*
*  int Clip2String(BufStruct *Storage, char **retstring, int *retlen, int clipno)
*
*  Copy the given ClipBoard and place the result in retstring.
*  Return a ret value.
************/
int __regargs Clip2String(BufStruct *Storage, char **retstring, int *retlen, int clipno)
{
  struct IFFHandle *iff;
  long error;
  int ret=IFF_ERROR;
  char *store=NULL;
  int storelen=0;
  int rlen;
  struct ContextNode *cn;

  if (IFFParseBase=OpenLibrary("iffparse.library", 0L)) {
    if (iff=AllocIFF()) {
      if (iff->iff_Stream=(ULONG)OpenClipboard(clipno)) {
        InitIFFasClip(iff);
        if (!(error=OpenIFF(iff, IFFF_READ))) {
          if (!(error=StopChunk(iff, ID_FTXT, ID_CHRS))) {
            while (1) {
              error=ParseIFF(iff, IFFPARSE_SCAN);
              if (error==IFFERR_EOC)
                continue;
              else if (error)
                break;

              cn=CurrentChunk(iff);
              
              if((cn) && (cn->cn_Type==ID_FTXT) && (cn->cn_ID==ID_CHRS)) {
                ret=OK;
                while ((rlen=ReadChunkBytes(iff, buffer, MAX_CHAR_LINE))>0) {
                  store=Realloc(store, storelen+rlen);
                  if (!store) {
                    ret=OUT_OF_MEM;
                    break;
                  }
                  memcpy(store+storelen, buffer, rlen);
                  storelen+=rlen;
                }
                if (rlen<0) {
                  Dealloc(store);
                  storelen=0;
                  store=NULL;
                  error=rlen;
                }
              }
            }
            if (error && error!=IFFERR_EOF)
              ret=IFF_ERROR;
          }
          CloseIFF(iff);
        }
        CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
      }
      FreeIFF(iff);
    }
    CloseLibrary(IFFParseBase);
  }
  
  *retstring=store;
  *retlen=storelen;

  return(ret);
}

#if 0  // This part is supplied in the key file

/*************************************************************
*
*  int String2Clip(BufStruct *Storage, char *sendstring, int sendlen, int clipno)
*
*  Copy the string to given ClipBoard.
*  Return TRUE/FALSE.
************/
int __asm String2Clip(register __a0 char *string, register __d0 int stringlen, register __d1 int clipno)
{
  struct Library *IFFParseBase ;
  struct IFFHandle *iff=NULL;
  long error=0;
  int ret;

  ret=FALSE;	// error
  if (IFFParseBase=OpenLibrary("iffparse.library", 0L)) {
    if (iff=AllocIFF()) {
      if (iff->iff_Stream=(ULONG)OpenClipboard(clipno)) {
        InitIFFasClip(iff);
        if (!(error=OpenIFF(iff, IFFF_WRITE))) {
          if (!(error=PushChunk(iff, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN))) {
            if (!(error=PushChunk(iff, 0, ID_CHRS, IFFSIZE_UNKNOWN))) {
              if (WriteChunkBytes(iff, string, stringlen)==stringlen) {
                ret=TRUE;	// OK
              } else
                error=IFFERR_WRITE;

              if (!error)
                error=PopChunk(iff);
            }
            if (!error)
              error=PopChunk(iff);
          }
          CloseIFF(iff);
        }
        CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
      }
      FreeIFF(iff);
    }
    CloseLibrary(IFFParseBase);
  }
  return(ret);
}          
#endif
