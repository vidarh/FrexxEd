/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
*
* Prompt.c
*
* Command prompting
*
*******/

#include "Buf.h"

#ifdef AMIGA
#include <libraries/dos.h>
#include <clib/exec_protos.h>
#include <stdlib.h>
#include <string.h>
#include <libraries/reqtools.h>
#include <proto/reqtools.h>
#include <proto/FPL.h>
#endif

#include <libraries/FPL.h>

#include "Alloc.h"
#include "Reqlist.h"
#include "Command.h"
#include "Execute.h"
#include "Function.h"
#include "Setting.h"
#include "Limit.h"

extern DefaultStruct Default;

extern struct FrexxEdFunction fred[];
extern const int nofuncs;

extern int do_number;
extern char **Argv;
extern int Argc;
extern void *Anchor;
extern char *lastprompt;	/* Innehållet i senaste promptningen. */

extern char *fpl_executer;

#define PROMPT_LEN 1024

/**************************************************************
 *
 * int Prompt(BufStruct *);
 *
 * Command prompting. Returns (int) the exit code from the Command().
 *
 *******/

int __regargs Prompt(BufStruct *Storage)
{
  UBYTE *buffer;
  int check, ret;
  /* allocate a char pointer array big enough to hold a FEW strings! */
  struct fplSymbol *sym;

  ret=fplSendTags(Anchor, FPLSEND_GETSYMBOL_FUNCTIONS, &sym, FPLSEND_DONE);
  if(ret)
    return(OUT_OF_MEM);

  buffer=(UBYTE *)Malloc(PROMPT_LEN+10); /* max input string length! */
  if (buffer) {
    buffer[0]=0;
    if (lastprompt)
      strcpy(buffer, lastprompt);
    check=Reqlist(Storage,
                  REQTAG_ARRAY, sym->array, sym->num,
                  REQTAG_STRING1, buffer, PROMPT_LEN,
		  REQTAG_SORT, REQTAG_END);

    fplSendTags(Anchor, FPLSEND_GETSYMBOL_FREE, sym, FPLSEND_DONE);

    switch(check) {
    case rl_CANCEL:
      ret=FUNCTION_CANCEL;
      break;
    case rl_ERROR:
      ret=OUT_OF_MEM;
      break;
    default:
      Dealloc(lastprompt);
      if (check>=0)
        strcat(buffer, "();");
      lastprompt=Strdup(buffer);
      fpl_executer=EXECUTED_PROMPT;
      ret=ExecuteFPL(Storage, lastprompt, FALSE, &check, NULL);
      break;
    }
    Dealloc(buffer);
  } else {
    ret=OUT_OF_MEM;
    fplSendTags(Anchor, FPLSEND_GETSYMBOL_FREE, sym, FPLSEND_DONE);
  }
  return(ret);
}
