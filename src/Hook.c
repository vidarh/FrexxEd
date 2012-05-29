/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
*
* Hook.c
*
* Hook handling routines.
*
*******/

#ifdef AMIGA
#include <exec/tasks.h>
#include <clib/exec_protos.h>
#include <proto/FPL.h>
#include <proto/utility.h>
#endif

#include <libraries/FPL.h>

#include <stdio.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "BufControl.h"
#include "Command.h"
#include "Function.h"  /* for the frexxed function structure */
#include "FACT.h"      /* the include below wants this! */
#include "KeyAssign.h" /* for the convertstring shit */
#include "Execute.h"
#include "Setting.h"
#include "Hook.h"
#include "Change.h"

extern DefaultStruct Default;
extern const int nofuncs;
extern FACT *DefaultFact;
extern void *Anchor;
extern BufStruct *NewStorageWanted;
extern int Visible;
extern struct Setting **sets;
extern struct FrexxEdFunction fred[];
extern BOOL hook_enabled;
extern BOOL clear_all_currents;
extern int undoantal;
extern int ErrNo;		// Global Error value storage.

static struct FrexxHook __regargs *IfHook(BufStruct *, struct FrexxHook **);

static int __regargs Clear_hook(BufStruct *, char *, char *, char *, struct FrexxHook **);
static int __regargs GoHook(BufStruct *, struct FrexxHook *, int, char **, int);


/*************************************************************
 *
 *  Om NewStorageWanted är satt efter att den här funktionen är
 * tillkallad är det upp till tillkallaren att ställa om Storagevärdet.
 *
 ****/

int __regargs RunHook(BufStruct *Storage,
                      int command,
                      int Argc,
                      char **Argv,
                      int flags)
{
  register int ret=FALSE;
  struct FrexxHook *hook=(flags&HOOK_POSTHOOK)?
                         Default.posthook[command&~LOCKINFO_BITS]:
                         Default.hook[command&~LOCKINFO_BITS];
  if (hook_enabled && hook) {
    if (IfHook(Storage, &hook)) { /* a first check with the dependenses */
      SHS(current)++;
      ret=GoHook(Storage, hook, Argc, Argv, flags);  /* go baby, go...! */
      if (NewStorageWanted) {
        clear_all_currents=TRUE;
      } else
        SHS(current)--;
    }
  }
  return ret;
}

static int __regargs
GoHook(BufStruct *Storage,
       struct FrexxHook *hook,
       int Argc,
       char **Argv,
       int flags)
{
  long len;
  int i;
  long fplret=FALSE;
  int ret;
  String newstring;
  String string;
  char *FPL;
  char *tempprg=NULL;
  int tempprglen=1000;
  int oldret;
  BOOL fpl_arg_string_made=FALSE;

  if (undoantal)
    Command(Storage, DO_NOTHING|NO_HOOK, 0, NULL, NULL);

  {
    do {
      if(hook->flags&HOOK_ABS)
        FPL = hook->name;
      else {
        if (!fpl_arg_string_made) {
          /* this is not an "absolute" hook! */
          register char *tempprgend;

          fpl_arg_string_made=TRUE;
          tempprg=Malloc(tempprglen);
          if (!tempprg)
            return(fplret);
          
          tempprg[0]='\0';
      
          len=hook->func->format? strlen(hook->func->format) : 0;
        
          Sprintf(tempprg, "%64s(", hook->func->name); /* for debugging only*/
          tempprgend=tempprg+65;
    
          for(i=0; i<len; i++) {
        
            if(hook->func->format[i]==FPL_STRARG ||
               hook->func->format[i]==FPL_STRARG_OPT) {
              tempprgend=strcpy(tempprgend, "\"")+1;
              if(i<Argc) {
                string.string=(char *)Argv[i];
                string.length=strlen((char *)Argv[i]); /* NOT the proper way!!! */
                if (ConvertString(DefaultFact, &string, &newstring)) {
                  if (newstring.length+tempprgend-tempprg>tempprglen-100) {
                    tempprglen=newstring.length+tempprgend-tempprg+1000;
                    tempprgend-=(int)tempprg;
                    tempprg=Realloc(tempprg, tempprglen);
                    if (!tempprg)
                      return(fplret);
                    tempprgend+=(int)tempprg;
                  }
                  strcpy(tempprgend, newstring.string);
                  tempprgend+=newstring.length;
                  Dealloc(newstring.string);
                } else {
                  Dealloc(tempprg);
                  return(fplret);
                }
              }
              tempprgend=strcpy(tempprgend, "\"")+1;
            } else {
              if(i<Argc)
                tempprgend+=Sprintf(tempprgend, "%ld", (long)Argv[i]);
              else
                tempprgend=strcpy(tempprgend, "0")+1;
            }
            if(i< (len-1) ) {
              strcpy(tempprgend, ",");
              tempprgend++;
            }
          }
        
          tempprgend=strcpy(tempprgend, ");")+1;
        }
        len=strlen(hook->name);
        memcpy(tempprg, hook->name, len);
        memset(&tempprg[len], ' ', 64-len);
        FPL = tempprg;
      }

      oldret = ErrNo;
      ret=ExecuteFPL(Storage, FPL, FALSE, &i, NULL);
      ErrNo=oldret;
  
      if(ret>=OK) {
        if (ret==NEW_STORAGE) {	// New Storage value.
          Storage=NewStorageWanted;
          clear_all_currents=TRUE;
          Visible=VISIBLE_OFF;
        }
        hook=hook->next;
        fplSendTags(Anchor, FPLSEND_GETRETURNCODE, &fplret, FPLSEND_DONE);
      } /* else
        the FPL function failed, but we go on to the next one as if nothing
        ever happened! */
  
      if(hook)
        IfHook(Storage, &hook); /* sets 'hook' to NULL if no more is to be
                                 run! */
    } while(!fplret && hook);
  }

  Dealloc(tempprg);
  return(fplret);
}

static struct FrexxHook __regargs *IfHook(BufStruct *Storage, struct FrexxHook **hook)
{
  while (*hook && (*hook)->settings[0] && !CheckSetting(Storage, (*hook)->settings))
    (*hook) = (*hook)->next;
  return (*hook);
}

int __regargs AddHook(BufStruct *Storage,
                      char *onfunc,
                      char *callfunc,
                      char *setting,
                      char flags)
{
  int function;
  struct FrexxHook **hook;
  int setlen;

  setlen = setting?strlen(setting):0;

  if(!strlen(callfunc))
     return(1);

  for (function=0; function<nofuncs; function++) {
    if (!strcmp(fred[function].name, onfunc))
      break;
  }
  if(function==nofuncs)
    return(1);

  if(flags & HOOK_POSTHOOK)
    hook=&Default.posthook[fred[function].ID&~LOCKINFO_BITS];
  else
    hook=&Default.hook[fred[function].ID&~LOCKINFO_BITS];

  {
    struct FrexxHook *checkhook=*hook;
    while(checkhook) {
      if(!strcmp(callfunc, checkhook->name) &&
         (!setlen || !strcmp(setting, checkhook->settings)))
	return 1; /* exactly this hook already exist! */
      checkhook = checkhook->next;
    }
  }

  while(*hook)
    hook=&(*hook)->next;

  (*hook)=(struct FrexxHook *)Malloc(sizeof(struct FrexxHook) + setlen);
  if ((*hook)) {
    (*hook)->name=Strdup(callfunc);
    (*hook)->next=NULL;
    (*hook)->func = &fred[function];
    (*hook)->flags=flags;
    if(!setlen)
      (*hook)->settings[0]=0;
    else
      strcpy((*hook)->settings, setting);

    setlen = strlen((*hook)->name);
    if(!setlen) {
      Dealloc((*hook)->name);
      Dealloc(*hook);
      return 1;
    }
    setlen = ToUpper((*hook)->name[setlen-1]);

    if( (setlen<'A' || setlen>'Z') &&
        (setlen<'0' || setlen>'9') )
      /* an exact program has been entered */
      (*hook)->flags |= HOOK_ABS;

  } else
    return(1);
  return (0);
}

int __regargs
HookClear(BufStruct *Storage,
          char *function,
          char *hook,
          char *setting)
{
  int ret;
  ret = Clear_hook(Storage, function, hook, setting, Default.posthook);
  ret += Clear_hook(Storage, function, hook, setting, Default.hook);
  return ret;
}

static int __regargs
Clear_hook(BufStruct *Storage,
          char *function,
          char *hookname,
          char *setting,
          struct FrexxHook **hook)
{
  int funcnum=0;
  struct FrexxHook *hk;
  struct FrexxHook *next;
  struct FrexxHook *prev;
  char match;
  int deletes=0;

  do {
    hk =hook[funcnum];
    if(!hk)
      continue;
    if(function && function[0]) {
      if(strcmp(function, hook[funcnum]->func->name))
	continue;
    }
    prev = NULL;
    while(hk) {
      if(hookname && hookname[0]) {
        match= !strcmp(hk->name, hookname);
      } else
        match = TRUE;

      if(match && setting && setting[0]) {
        match = !strcmp(hk->settings+(hk->settings[0]=='!'), setting);
      }
      next=hk->next;
      if(match) {
        /* this hook is subject of removal! */
        Dealloc(hk->name);
        Dealloc(hk);
        if(prev)
          prev->next=next; /* set previous '->next' pointer */
        else
          hook[funcnum] = next; /* set the origin pointer to 'next' */
        deletes++; /* one more removed */
        hk = prev; /* to make the 'prev' get right below */
      }
      prev = hk; /* this is the previous now */
      hk = next;
    }
  } while(++funcnum<MAX_COMMANDS);

  return deletes;
}
