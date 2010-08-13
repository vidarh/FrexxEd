/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************************
*
*  Setting.c
*
**********/

#include <exec/types.h>
#include <libraries/FPL.h>
#include <proto/FPL.h>
#include <proto/utility.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "Command.h"
#include "Change.h"
#include "Edit.h"
#include "FACT.h"
#include "KeyAssign.h"
#include "Setting.h"

extern DefaultStruct Default; /* the default values of a BufStruct */
extern struct Setting defaultsets[];
extern int antaldefaultsets;
extern int antalsets;
extern int antalallocsets;
extern struct Setting **sets;
extern char DebugOpt; /* debug on/off */
extern char buffer[];
extern char **setting_string_pointer;
extern int *setting_int_pointer;
extern struct SettingSaved *saved_default_setting;

int __regargs InitDefaultSetting()
{
  int counter;

  sets=(struct Setting **)Malloc((antaldefaultsets+10)*sizeof(struct Settings *));

  if (!sets)
    return(OUT_OF_MEM);

  antalsets=antaldefaultsets;
  antalallocsets=antalsets+10;
  for (counter=0; counter<antaldefaultsets; counter++) {
    sets[counter]=&defaultsets[counter];
  }
  return(OK);
}


/* Om name==NULL, så deleteas hela strukturen, annars tas bara den 
   settingen/variabeln bort.  Defaultvärdet rörs ej. */

int __regargs DeleteSetting(char *name)
{
  int ret=OK;
  register int counter;
  if (name) {
    if (!setting_string_pointer && !setting_int_pointer) {
      register int vald=GetSettingName(name);
      if (vald>=0) {
        register int offset=sets[vald]->offset;
        if (sets[vald]->type & ST_USERDEFINED) {
          if (sets[vald]->type & ST_GLOBAL) {
            if ((sets[vald]->type & 15)==ST_STRING)
              Dealloc((char *)Default.GlobalInfo[offset]);
            memcpy(&Default.GlobalInfo[offset],
                   &Default.GlobalInfo[offset+1],
                   (Default.globalinfoantal-offset)*sizeof(int));
          } else {
            register SharedStruct *shared=&Default.SharedDefault;
            while (shared) {
              if ((sets[vald]->type & 15)==ST_STRING) {
#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "Dealloc %s=%ld\n", sets[vald]->name, shared->LokalInfo[offset]);
#endif
                Dealloc((char *)shared->LokalInfo[offset]);
              }
              memcpy(&shared->LokalInfo[offset],
                     &shared->LokalInfo[offset+1],
                     (Default.lokalinfoantal-offset)*sizeof(int));
              shared=shared->Next;
            }
          }
          if (sets[vald]->type&ST_USERDEFINED)
            Dealloc(sets[vald]->FPLsave_string);
          {
            register int bits=ST_SHARED|ST_USERDEFINED;
            if (sets[vald]->type & ST_GLOBAL)
              bits=ST_GLOBAL|ST_USERDEFINED;
            for (counter=0; counter<antalsets; counter++) {
              if ((sets[counter]->type & bits)==bits)
                if (sets[counter]->offset>offset)
                  sets[counter]->offset--;
            }
          }
          Dealloc(sets[vald]);
        }
        memcpy(&sets[vald], &sets[vald+1], (antalsets-vald)*sizeof(struct Setting *));
        antalsets--;
  
      } else
        ret=CANT_FIND_SETTING;
    }
  } else {
    register int counter;
    for (counter=0; counter<antalsets; counter++) {
      if (sets[counter]->type & ST_USERDEFINED) {
        DeleteSetting(sets[counter]->name);
        counter--;
      }
    }
    {
      register SharedStruct *shared=&Default.SharedDefault;
      while (shared) {
        Dealloc(shared->LokalInfo);
        shared->LokalInfo=NULL;
        shared=shared->Next;
      }
    }
    Dealloc(Default.GlobalInfo);
    Default.GlobalInfo=NULL;
    Dealloc(sets);
    sets=NULL;
  }
  return(ret);
}


void __regargs LogSetting(char *name, char *type, long value, int format)
{
  struct SettingSaved *ptr;

  ptr=(struct SettingSaved *)Malloc(sizeof(struct SettingSaved));
  if (ptr) {
    ptr->string=NULL;
    if (format==FPL_STRARG)
      ptr->string=Strdup((char *)value);
    else
      ptr->value=value;
    ptr->name=Strdup(name);
    ptr->type=Strdup(type);
    ptr->next=saved_default_setting;
    saved_default_setting=ptr;
  }
}
void __regargs FindLogSetting(char *name, char *type, long *value, char *format)
{
  struct SettingSaved *ptr=saved_default_setting;
  while (ptr) {
    if (!strcmp(ptr->name, name)) {
      if (!strcmp(ptr->type, type)) {
        if (ptr->string) {
          *value=(long)ptr->string;
          *format=FPL_STRARG;
        } else {
          *value=ptr->value;
          *format=FPL_INTARG;
        }
      }
      break;
    }
    ptr=ptr->next;
  }
}

#ifndef POOL_DEALLOC
void __regargs DeleteLogSetting()
{
  struct SettingSaved *ptr=saved_default_setting;
  while (ptr) {
    Dealloc(ptr->name);
    Dealloc(ptr->type);
    Dealloc(ptr->string);
    {
      struct SettingSaved *next=ptr->next;
      Dealloc(ptr);
      ptr=next;
    }
  }
}
#endif

/**********************************************************************
 *
 *
 * The typestring is case insensitive.
 * The function skips an unknown character.
 * If min==0 and max==0, they will be ignored.
 * The cykle string is ignored if a 'C'-keyword is specified.
 *
 *
 * 'G' - Global info.
 * 'L' - Lokal info.
 * 'B' - Bool (0-1)
 * 'I' - Integer (-int -> +int)
 * 'C' - Cykle (0-x)
 * 'S' - String
 * 'W' - Write this info with the default file.
 * 'H' - Hidden.
 * 'R' - Read only.
 * 'T'+num - Type.
 *
 ********/
int __regargs ConstructSetting(char *name, char *notify, char *FPLaddition, char *typestring, char *cycle, int min, int max, int init, char format)
{
  int type=ST_NOSAVE|ST_GLOBAL|ST_BOOL;
  int mask=0;
//  int ret=OK;

  if (setting_string_pointer || setting_int_pointer)
    return(FUNCTION_CANCEL);

  if (!name || !typestring || !name[0] || !typestring[0])
    return(SYNTAX_ERROR);

  ExtractType(typestring, &type, &mask, &min, &max);
  if (!(type&15) || (type&15)==ST_ARRAY)
    return(SYNTAX_ERROR);

  type|=ST_USERDEFINED;
  mask|=MS_USERDEFINED;
  {
    struct Setting **temp;
    struct Setting *tempset;
    int counter;
    int cyclelength=0;
    int notifylength=strlen(notify);
    int additionlength=strlen(FPLaddition);
    char *top;
  
    if (GetSettingName(name)>=0)
      return(CANT_ALLOC_SETTING);
  
    FindLogSetting(name, typestring, (long *)&init, &format);

    {
      register char *temp;
      if (type & ST_GLOBAL) {
        if (Default.globalinfoalloc<=Default.globalinfoantal+2) {
          temp=Remalloc((char *)Default.GlobalInfo, (Default.globalinfoalloc+10)*sizeof(int));
          if (!temp)
            return(OUT_OF_MEM);
          Default.GlobalInfo=(int *)temp;
          memset(&Default.GlobalInfo[Default.globalinfoalloc], 0, sizeof(int)*10);
          Default.globalinfoalloc+=10;
        }
      } else {
        if (Default.lokalinfoalloc<=Default.lokalinfoantal+2) {
          register SharedStruct *shared=&Default.SharedDefault;
          while (shared) {
            temp=Remalloc((char *)shared->LokalInfo, (Default.lokalinfoalloc+10)*sizeof(int));
            if (!temp)
              return(OUT_OF_MEM);
            shared->LokalInfo=(int *)temp;
            memset(&shared->LokalInfo[Default.lokalinfoalloc], 0, sizeof(int)*10);
            shared=shared->Next;
          }
          Default.lokalinfoalloc+=10;
        }
      }
    }
  
    if ((type&15)==ST_CYCLE) {
      counter=0;
      cyclelength++;
      while(cycle[counter]) {
        if (cycle[counter]=='|')
          cyclelength++;
        counter++;
      }
      min=0;
      max=cyclelength;
    }
  
    tempset=(struct Setting *)Malloc(sizeof(struct Setting)+strlen(name)+
                                     strlen(cycle)+notifylength+
                                     additionlength+
                                     (cyclelength+2)*sizeof(char *)+8);
    if (!tempset)
      return(OUT_OF_MEM);
  
    if (antalsets>=antalallocsets-2) {
      temp=(struct Setting **)Remalloc((char *)sets, (antalallocsets+10)*sizeof(struct Settings *));
      if (!temp) {
        Dealloc(tempset);
        return(OUT_OF_MEM);
      }
      antalallocsets+=10;
      sets=temp;
    }
  
    for (counter=0; counter<antalsets; counter++) {
      if (strcmp(sets[counter]->name, name)>0)
        break;
    }
  
    MoveUp4(&sets[counter+1], &sets[counter], (antalsets-counter)*sizeof(struct Setting *)/4);
    sets[counter]=tempset;
    tempset->mask=mask;
    tempset->update=SE_NOTHING;
    tempset->name=((char *)tempset)+sizeof(struct Setting);
    strcpy(tempset->name, name);
    top=tempset->name+strlen(name)+1;
  
    tempset->FPLnotify=top;
    strcpy(top, notify);
    top+=notifylength+1;
  
    tempset->FPLaddition=top;
    strcpy(top, FPLaddition);
    top+=additionlength+1;
  
    tempset->min=0;
    tempset->max=0;
  
    if (cyclelength) {
      char *off=top;
      char **array=(char **)(top+strlen(cycle)+6);
      array=(char **)((int)array&~1);
      strcpy(off, cycle);
      tempset->cycle=array;
      array[0]=off;
      array=&array[1];
      while(off[0]) {
        if (off[0]=='|') {
          off[0]=0;
          off++;
          array[0]=off;
          array=&array[1];
        } else
          off++;
      }
      array[0]=NULL;
      tempset->max=cyclelength-1;
    } else {
      tempset->cycle=NULL;
      if ((type & 15)!=ST_STRING) {
        if ((type & 15)==ST_BOOL) {
          tempset->min=-1;
          tempset->max=1;
        } else {
          tempset->min=min;
          tempset->max=max;
        }
      }
    }
    tempset->type=type;
  
    if (format) {
      if ((type & 15)==ST_STRING) {
        if (format!=FPL_STRARG)
          init=(int)"";
      } else {
        if (format!=FPL_INTARG)
          init=tempset->min;
      }
    } else {
      init=tempset->min;
      if ((type & 15)==ST_STRING)
        init=(int)"";
      else if ((type & 15)==ST_BOOL)
        init=0;
    }

    if (!(type&ST_NOSAVE)) {
      register int pos;

      pos=Sprintf(buffer, "\"%s\", \"%s\"", name, typestring);

      tempset->FPLsave_string=Strdup(buffer);
    } else
      tempset->FPLsave_string=NULL;

    if (format==FPL_INTARG && ((type&15)==ST_CYCLE || (type&15)==ST_BOOL))
      init=init<<24;
    if (type & ST_GLOBAL) {
      tempset->offset=Default.globalinfoantal;
      if ((type & 15)==ST_STRING)
        Default.GlobalInfo[Default.globalinfoantal]=(int)Strdup((char *)init);
      else
        Default.GlobalInfo[Default.globalinfoantal]=(int)init;
      Default.globalinfoantal++;
    } else {
      register SharedStruct *shared=&Default.SharedDefault;
      tempset->offset=Default.lokalinfoantal;
      while (shared) {
        if ((type & 15)==ST_STRING) {
          shared->LokalInfo[Default.lokalinfoantal]=(int)Strdup((char *)init);
#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "String %s=%s (%ld)\n", tempset->name, init, shared->LokalInfo[Default.lokalinfoantal]);
#endif
        } else
          shared->LokalInfo[Default.lokalinfoantal]=(int)init;
        shared=shared->Next;
      }
      Default.lokalinfoantal++;
    }
    antalsets++;
  }
  return(OK);
} 


int __regargs GiveMaskNumber(char *type, char **newpoint)
{
  int result=0;
  int negresult=0;
  int mask;
  BOOL neg;

  while (type[0]=='(') {
    type++;
    neg=FALSE;
    if (*type=='!') {
      type++;
      neg=TRUE;
    }
    if (!Strnicmp(type, "global", sizeof("global")-1))
      mask=MS_GLOBAL;
    else if (!Strnicmp(type, "local", sizeof("local")-1))
      mask=MS_LOCAL;
    else if (!Strnicmp(type, "screen", sizeof("screen")-1))
      mask=MS_SCREEN;
    else if (!Strnicmp(type, "display", sizeof("display")-1))
      mask=MS_DISPLAY;
    else if (!Strnicmp(type, "io", sizeof("io")-1))
      mask=MS_IO;
    else if (!Strnicmp(type, "read", sizeof("read")-1))
      mask=MS_READ;
    else if (!Strnicmp(type, "user", sizeof("user")-1))
      mask=MS_USERDEFINED;
    else if (!Strnicmp(type, "hidden", sizeof("hidden")-1))
      mask=MS_HIDDEN;
    else if (!Strnicmp(type, "system", sizeof("system")-1))
      mask=MS_SYSTEM;
    while (*(type++)!=')');
    if (neg)
      negresult|=mask;
    else
      result|=mask;
  }
  if (negresult)
    result|=~negresult;
  if (newpoint)
    *newpoint=type;
  return(result);
}


int __regargs ExtractType(char *typestring, int *typestore, int *maskstore, int *min, int *max)
{
  int type=*typestore;
  int mask=*maskstore;
  {
    char *typecount=typestring;
    while (*typecount) {
      switch(*typecount) {
      case 'G':	// Global
      case 'g':
        type=(type & ~(ST_GLOBAL|ST_SHARED))|ST_GLOBAL|ST_USERDEFINED;
        mask|=MS_GLOBAL;
        break;
      case 'L':	// Lokal
      case 'l':
        type=(type & ~(ST_GLOBAL|ST_SHARED))|ST_SHARED|ST_USERDEFINED;
        mask|=MS_LOCAL;
        break;
      case 'B':	// Bool
      case 'b':
        type=(type & ~15)|ST_BOOL|ST_USERDEFINED;
        *min=0;
        *max=1;
        break;
      case 'I':	// Integer
      case 'i':
        type=(type & ~15)|ST_NUM|ST_USERDEFINED;
        break;
      case 'C':	// Cykle
      case 'c':
        type=(type & ~15)|ST_CYCLE|ST_USERDEFINED;
        break;
      case 'A':	// Array (only for RequestWindow)
      case 'a':
        type=(type & ~15)|ST_ARRAY|ST_USERDEFINED;
        break;
      case 'S':	// String
      case 's':
        type=(type & ~15)|ST_STRING|ST_USERDEFINED;
        break;
      case 'W':	// Write it with the default file.
      case 'w':
        type&=~ST_NOSAVE;
        break;
      case 'R':	// Read only.
      case 'r':
        type|=ST_READ;
        mask|=MS_READ;
        break;
      case 'T':	// Type.
      case 't':
        mask=fplStrtol(typecount+1, 0, &typecount)|MS_USERDEFINED;
        typecount--;
        break;
      case '(':	// Type.
        mask=(GiveMaskNumber(typecount, &typecount)&(~((MS_GLOBAL|MS_LOCAL|MS_HIDDEN|MS_READ))))|(mask&(MS_GLOBAL|MS_LOCAL|MS_HIDDEN|MS_READ))|MS_USERDEFINED;
        typecount--;
        break;
      case 'H':	// Hidden.
      case 'h':
        type|=ST_HIDDEN;
        mask|=MS_HIDDEN;
        break;
      }
      typecount++;
    }
  }
  *typestore=type;
  *maskstore=mask;
  return(OK);
}
