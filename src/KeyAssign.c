/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * KeyAssign.c
 *
 *********/

#ifdef AMIGA
#include <exec/io.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <proto/dos.h>
#include <proto/exec.h>
//#include <proto/FPL.h>
#include <proto/intuition.h>
#include <proto/reqtools.h>
#include <proto/utility.h>
#endif

#include <libraries/FPL.h>

#include "compat.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Buf.h"
#include "Alloc.h"
#include "BufControl.h"
#include "BuildMenu.h"
#include "Change.h"
#include "Command.h"
#include "FACT.h"
#include "Function.h"
#include "GetFile.h"
#include "IDCMP.h"
#include "Init.h"
#include "Limit.h"
#include "Setting.h"
#include "RawKeys.h"
#include "Reqlist.h"
#include "Request.h"
#include "KeyAssign.h"
#include "UpdtScreen.h"
#include "WindowOutput.h"

extern char buffer[];
extern DefaultStruct Default;
//extern const int converttablelength;
//extern const char *converttable[][2];
//extern struct Library *FPLBase;
extern const int nofuncs;
extern struct FrexxEdFunction fred[];
extern void *Anchor;
extern int ReturnValue;		// Global return value storage.
extern char GlobalEmptyString[];
//extern char DebugOpt; /* debug on/off */
//extern struct ExecBase *SysBase;

/*** Private ***/

static const char *converttable[][2]={
  {"F1",	"\xfb\x9b""0~"},
  {"F2",	"\xfb\x9b""1~"},
  {"F3",	"\xfb\x9b""2~"},
  {"F4",	"\xfb\x9b""3~"},
  {"F5",	"\xfb\x9b""4~"},
  {"F6",	"\xfb\x9b""5~"},
  {"F7",	"\xfb\x9b""6~"},
  {"F8",	"\xfb\x9b""7~"},
  {"F9",	"\xfb\x9b""8~"},
  {"F10",	"\xfb\x9b""9~"},
  {"F11",	"\xfb\x9b""10~"},
  {"F12",	"\xfb\x9b""11~"},
  {"F13",	"\xfb\x9b""12~"},
  {"F14",	"\xfb\x9b""13~"},
  {"F15",	"\xfb\x9b""14~"},
  {"F16",	"\xfb\x9b""15~"},
  {"F17",	"\xfb\x9b""16~"},
  {"F18",	"\xfb\x9b""17~"},
  {"F19",	"\xfb\x9b""18~"},
  {"F20",	"\xfb\x9b""19~"},
  {"Del",	"\xfb\x7f"},
  {"Delete",	"\xfb\x7f"},
  {"Help",	"\xfb\x9b?~"},
  {"Up",	"\xfb\x9b""A"},
  {"Down",	"\xfb\x9b""B"},
  {"Right",	"\xfb\x9b""C"},
  {"Left",	"\xfb\x9b""D"},
  {"Esc",	"\xfb\x1b"},
  {"Escape",	"\xfb\x1b"},
  {"Enter",	"\x43"},
  {"Return",	"\x44"},
  {"Tab",	"\xfb\t"},
  {"Bspc",	"\xfb\b"},
  {"Backspace",	"\xfb\b"},
  {"Spc",	"\xfb "},
  {"Space",	"\xfb "},
  {" ",		"\xfb "},
  {"num0",	"\x0f"},
  {"num1",	"\x1d"},
  {"num2",	"\x1e"},
  {"num3",	"\x1f"},
  {"num4",	"\x2d"},
  {"num5",	"\x2e"},
  {"num6",	"\x2f"},
  {"num7",	"\x3d"},
  {"num8",	"\x3e"},
  {"num9",	"\x3f"},
  {"num[",	"\x5a"},
  {"num]",	"\x5b"},
  {"num/",	"\x5c"},
  {"num*",	"\x5d"},
  {"num-",	"\x4a"},
  {"num+",	"\x5e"},
  {"num.",	"\x3c"}
};

const int converttablelength=sizeof(converttable)/((sizeof(char *)*2));

static const char hextab[]={ '0','1','2','3','4','5','6','7',
                      '8','9','a','b','c','d','e','f'};


static int __asm bsearch_cmp_ka(register __a0 void *name, register __d1 int count)
{
  return(strcmp(fred[count].name, (char *)name));
}

/*****************************************************************
 *  KeyAssign(BufStruct *Storage, char *key, char *function, int flag)
 *
 *  Assign the key combination to the function.
 *  flag är definierad i KeyAssign.h
 *
 *  The global ReturnValue has the allocated keystruct.
 *  Return: a ret value.
 *********/
int __regargs KeyAssign(BufStruct *Storage, char *string, char *function, char *depended, int flag)
{
  int ret=OK;
  struct Kmap *val=&Default.key;
  struct Kmap *temp;
  BOOL stop=FALSE;
  int counter=0;
  int funcset;
  char *reqstring=NULL;
  char *reqfunction;

  KeyStruct firstkey={NULL, NULL, NULL, NULL};
  KeyStruct *currkey=&firstkey;

  ReturnValue=0;
  if (depended && !depended[0])
    depended=NULL;

  if (flag&kaINPUT) {
    while (!stop) {
      Sprintf(buffer, RetString(STR_INPUT_KEY_STROKE), counter+1);
      Status(Storage, buffer, 3);
      
      if (GetKey(Storage, gkWAIT | gkSTRIP)<0 || strcmp(buffer, "\x1B")) {
        currkey->next=(KeyStruct *)Malloc(sizeof(KeyStruct));
        if (currkey->next) {
          currkey=currkey->next;
          currkey->Qual=SmartInput(((struct IntuiMessage *)(buffer+100))->Qualifier);
          currkey->string=Strdup(buffer);
          currkey->next=NULL;
          counter++;
        } else {
          stop=TRUE;
          ret=OUT_OF_MEM;
        }
      } else
        stop=TRUE;
    }
    string=NULL;
  } else if (flag&kaREQUEST) {
    int reqret;
    struct fplSymbol *sym;

    ret=fplSendTags(Anchor, FPLSEND_GETSYMBOL_FUNCTIONS, &sym, FPLSEND_DONE);
    reqstring=Malloc(2000);
    reqfunction=&reqstring[1000];
    if (ret==OK && reqstring) {
      reqstring[0]=0;
      reqfunction[0]=0;
      if (string)
        strcpy(reqstring, string);
      if (function)
        strcpy(reqfunction, function);
      reqret=Reqlist(Storage,
                     REQTAG_STRING2, reqstring, 1000-4,
                     REQTAG_STRINGNAME2, RetString(STR_KEY_SEQUENCE),
                     REQTAG_STRING1, reqfunction, 1000,
                     REQTAG_STRINGNAME1, RetString(STR_FUNCTION),
                     REQTAG_TITLE, RetString(STR_KEY_ASSIGN),
                     REQTAG_ARRAY, sym->array, sym->num,
                     REQTAG_SORT,
                     REQTAG_END);
      if (reqret==rl_CANCEL || reqstring[0]==0 || reqfunction[0]==0)
        ret=FUNCTION_CANCEL;
      else if (reqret==rl_ERROR)
        ret=-(int)RetString(STR_ERROR);
      else if (reqret>=0)
        strcat(reqfunction, "();");
  
      string=reqstring;
      function=reqfunction;
      fplSendTags(Anchor, FPLSEND_GETSYMBOL_FREE, sym, FPLSEND_DONE);
    } else
      ret=OUT_OF_MEM;
  }

  if (string && ret>=OK) {
	ret=TemplateString(string, &firstkey);
  }

  if (ret>=OK) {
    if (firstkey.next) {
      char *cmd;
      funcset=FALSE;
      currkey=firstkey.next;
      while (currkey) {
        temp=NULL;
        if (!funcset) {
          temp=GetKeyMap(val, currkey->Qual, (int)currkey->string, currkey->rawkey);
        }
        if (!temp || !temp->mul || !currkey->next) {
          if (!currkey->next) {
            cmd=function;
            counter=0;
		/* Se om det kommandot kan utbytas till vanligt DO_cmd */
		/* OBS, måste gå att göra snabbare */
            if (cmd) {
              register char *namnpek=cmd;
              register char *namnpoint=cmd;
              while (namnpek[0]) {
                switch (namnpek[0]) {
                case '(':
                  if (namnpek[1]==')' && namnpek[2]==';' && namnpek[3]==0) {
                    buffer[namnpek-cmd]=0;
                    memcpy(buffer, cmd, namnpek-cmd);
                    namnpoint=buffer;
                    break;
                  }
                case ' ':
                  counter=nofuncs;
                  goto skipcheck;
                }
                namnpek++;
              }
              counter=Bsearch(namnpoint, nofuncs, &bsearch_cmp_ka);
              if (counter<0)
                counter=nofuncs;
            }
            if (counter<nofuncs) {
              if (fred[counter].ID & FPL_CMD) {
                Sprintf(buffer, "%s();", fred[counter].name);
                cmd=buffer;
                counter=nofuncs;
              }
            }
          skipcheck:
            if (currkey->next) {
              val=AddKey(akCMD, currkey->Qual,
                         currkey->rawkey, currkey->string, DO_NOTHING, val, NULL);
            } else {
              if ((flag&kaMENU) || !temp || temp->mul ||
                  temp->Qual!=currkey->Qual || temp->Code!=currkey->rawkey ||
                  (temp->depended && !depended) ||
                  (!temp->depended && depended) ||
                  (temp->depended && depended && strcmp(temp->depended, depended)) ||
                  (counter<nofuncs && temp->flags==akCMD && temp->Command!=fred[counter].ID) ||
                  (temp->flags==akFPLALLOC && counter<nofuncs) ||
                  (temp->flags==akCMD && counter>=nofuncs) ||
                  (temp->flags==akFPLALLOC && strcmp(temp->FPLstring, cmd))) {
                if (counter<nofuncs) {
                  val=AddKey(akCMD, currkey->Qual,
                             currkey->rawkey, currkey->string, fred[counter].ID, val, depended);
                } else {
                  val=AddKey(akFPLALLOC, currkey->Qual,
                             currkey->rawkey, currkey->string, (int)cmd, val, depended);
                }
              }
            }
            if (!val) ret=OUT_OF_MEM;
            funcset=TRUE;
          } else
            val=AddKey(akCMD, currkey->Qual,
                       currkey->rawkey, currkey->string, NULL, val, NULL);
        } else {
          val=temp;
        }
        currkey=currkey->next;  
      }
      ReturnValue=(int)val;
	  // FIXME: This seems to be a bug, so taken it out, but don't understand
	  // the original rationale. VH
	  //      ret=(int)RetString(STR_FUNCTION_ASSIGNED_TO_KEY);
    } else {
      ret=FUNCTION_CANCEL;
	}
  }
  if (firstkey.next) {
    currkey=(KeyStruct *)firstkey.next;
    while (currkey) {
      Dealloc(currkey->string);
      {
        KeyStruct *temp=currkey->next;
        Dealloc(currkey);
        currkey=temp;
      }
    }
  }

  Dealloc(reqstring);
  return(ret);
}



struct Kmap __regargs *GetKeyMap(struct Kmap *firstkmap, UWORD Qualifier, int string, int rawkey)
{
  register struct Kmap *val=firstkmap->mul;
  Qualifier=SmartInput(Qualifier);

  while(val) {
    if (((val->string && string) && strcmp(val->string, (char *)string)==0) ||
        ((!val->string && !string) && val->Code==rawkey)) {
#if 0 //old
      if ((val->Qual&RAW_CTRL   && !(Qualifier&RAW_CTRL)) ||
          (val->Qual&RAW_AMIGA  && !(Qualifier&RAW_AMIGA)) ||
          (val->Qual&RAW_ALT    && !(Qualifier&RAW_ALT)) ||
          (val->Qual&RAW_SHIFT  && !(Qualifier&RAW_SHIFT)) ||
          (!(val->Qual&RAW_CTRL) && Qualifier&RAW_CTRL) ||
          (!(val->Qual&RAW_AMIGA) && Qualifier&RAW_AMIGA) ||
          (!(val->Qual&RAW_ALT)   && Qualifier&RAW_ALT) ||
          (!(val->Qual&RAW_SHIFT) && Qualifier&RAW_SHIFT));
      else
       return(val);
#endif
      if ((val->Qual&RAW_QUAL)==Qualifier)
        return(val);
    }
    val=val->next;
  }
  return(NULL);
}

/* flaggor enligt 'ks_'-modellen */
int PrintQualifiers(char *place, int Qual, int flag)
{
  if (flag == ksKEYSEQ) {
	if (Qual & RAW_AMIGA) strcpy(place, "AMIGA ");
	if (Qual & RAW_ALT) strcat(place, "ALT ");	
	if (Qual & RAW_CTRL) strcat(place,"CTRL ");
	if (Qual & RAW_SHIFT) strcat(place,"SHIFT ");
  } else {
	if (Qual & RAW_AMIGA) strcpy(place,"A-");
	if (Qual & RAW_ALT) strcat(place, "Alt-");	
	if (Qual & RAW_CTRL) strcat(place,"C-");
	if (Qual & RAW_SHIFT) strcat(place,"S-");
  }
  return strlen(place);
}

/*****************************************************************
 *  KeySequence(struct Kmap *val, int flag)
 *
 *  Convert the Kmap to a string sequence
 *  Return: a string to be Dealloc().
 *********/
char __regargs *KeySequence(struct Kmap *val, int flag)
{
  char *retstring=GlobalEmptyString;
  int retlength=0;

  while (val!=&Default.key) {
    buffer[0]=0;
    PrintQualifiers(buffer, val->Qual, flag);

    {
      register int buflen=strlen(buffer);
      register int count;
      for (count=0; count<converttablelength; count++) {
        if ((val->string && !strcmp(&converttable[count][1][1], val->string)) ||
            val->Code==converttable[count][1][0]) {
          Sprintf(&buffer[buflen], "'%s'", converttable[count][0]);
          break;
        }
      }
      if (count==converttablelength) {
        if (val->string) {
          count=0;
          while (val->string[count]) {
            if (val->string[count]==' ')
              buffer[buflen++]='\\';
            buffer[buflen++]=val->string[count];
            count++;
          }
          buffer[buflen]=0;
        } else
          Sprintf(buffer+strlen(buffer), "'0x%02lx'", val->Code);
      }
    }

    if (retlength)
      strcat(buffer, " ");
    retstring=Realloc(retstring, retlength+strlen(buffer)+1);
    if (!retstring)
      return(NULL);
    if (retlength)
      movmem(retstring, retstring+strlen(buffer), retlength);
    retlength+=strlen(buffer);
    memcpy(retstring, buffer, strlen(buffer));

    val=val->father;
  }
  retstring[retlength]=0;
  return(retstring);
}

/*****************************************************************
 *  MacroAssign(BufStruct *Storage, BlockStruct *block, char *macroname, char *keysequence)
 *
 *  Assign the block as a macro.
 *
 *  Return: a ret value.
 *********/
int __regargs MacroAssign(BufStruct *Storage, BlockStruct *block,
                          char *macroname, char *keysequence, int norequest)
{
  int ret=OK;
  BufStruct *newbuf;
  char *reqname;
  char *reqkey;
  int reqret;

  if (block->line) {
    reqname=Malloc(2000);
    if (reqname) {
      reqkey=&reqname[1000];
      reqname[0]=0;
      reqkey[0]=0;
      if (macroname)
        stccpy(reqname, macroname, 999);
      if (keysequence)
        stccpy(reqkey, keysequence, 499);
      if (!norequest) {
        reqret=Reqlist(Storage,
                       REQTAG_STRING1, reqname, 1000,
                       REQTAG_STRINGNAME1, RetString(STR_MACRO_NAME),
                       REQTAG_STRING2, reqkey, 500,
                       REQTAG_STRINGNAME2, RetString(STR_KEY_SEQUENCE),
                       REQTAG_TITLE, RetString(STR_MACRO_ASSIGN),
                       REQTAG_END);
        if (reqret==rl_CANCEL)
          ret=FUNCTION_CANCEL;
        else if (reqret==rl_ERROR)
          ret=-(int)RetString(STR_ERROR);
      }
      if (ret>=OK) {
        if (newbuf=MakeNewBuf(Storage)) {
          if (norequest || reqkey[0]) {
            char *namnbuffer=reqkey+500;
            Sprintf(namnbuffer, "ExecuteBuffer(%ld);", newbuf->shared);
            ret=KeyAssign(Storage, reqkey, namnbuffer, NULL, reqkey[0]?0:kaINPUT);
            if (ret<OK)
              Free(newbuf, TRUE, FALSE);
            else
              newbuf->shared->macrokey=KeySequence((struct Kmap *)ReturnValue, ksKEYSEQ);
          }
          if (ret>=OK) {
            if (!reqname[0])
              Sprintf(reqname, "Macro %s %ld.macro", newbuf->shared->macrokey, newbuf);
            newbuf->shared->type=type_MACRO|type_HIDDEN;
            Split(newbuf->shared, reqname);
            CheckName(newbuf, FALSE, TRUE, NULL);
            Dealloc(newbuf->shared->text);
            newbuf->shared->taket=block->taket;
            newbuf->shared->text=block->text;
            newbuf->shared->line=block->line;
            {
              register int count, size=0;
              for (count=1; count<=block->line; count++)
                size+=block->text[count].length;
              newbuf->shared->size=size;
            }
            block->text=NULL;
            block->taket=0;
            block->line=0;
          }
        }
      }
      Dealloc(reqname);
    } else
      ret=OUT_OF_MEM;
  }
  return(ret);
}


/*******************************************************
 *
 * char *ConvertString(FACT *fact, String *string, String *result)
 *
 * Konverterar strängen till C-syntax med hjälp av av FACT-strukten.
 *
 * Return:  En sträng som ska Dealloc()'as läggs i result.
 *          (TRUE) om allt gick bra.
 *********/
int __regargs ConvertString(FACT *fact, String *string, String *result)
{
  char *retstring;
  int retalloc=string->length+20;
  int retlength=0;
  int counter;
  char *newstring;
  int ret=TRUE;

  if (string->length) {
    retstring=Malloc(retalloc+4);
    if (retstring) {
      for (counter=0; counter<string->length; counter++) {
        newstring=NULL;
        switch (string->string[counter]) {
        case 0:
          newstring="\\x00";
          break;
        case '\n':
          newstring="\\n";
          break;
        case '\t':
          newstring="\\t";
          break;
        case '\b':
          newstring="\\b";
          break;
        case '\\':
          newstring="\\\\";
          break;
        case '\"':
          newstring="\\\"";
          break;
        }
        if (newstring) {
          strcpy(retstring+retlength, newstring);
          retlength+=strlen(newstring);
        } else if (fact->strings[string->string[counter]]) {
          retstring[retlength++]='\\';
          retstring[retlength++]='x';
          retstring[retlength++]=hextab[(string->string[counter]>>4)&15];
          retstring[retlength++]=hextab[(string->string[counter])&15];
//          Sprintf(retstring+retlength, "\\x%lc%lc", hextab[(string->string[counter]>>8)&15], hextab[(string->string[counter])&15]);
//          retlength+=4;
        } else {
          retstring[retlength]=string->string[counter];
          retlength++;
        }
        if (retlength>retalloc) {
          retstring=Realloc(retstring, retalloc+=100);
          if (!retstring) {
            retstring=GlobalEmptyString;
            retlength=0;
            ret=FALSE;
            break;
          }
        }
      }
      retstring[retlength]=0;
    } else {
      retstring=GlobalEmptyString;
      ret=FALSE;
    }
  } else
    retstring=GlobalEmptyString;

  result->string=retstring;
  result->length=retlength;
  return(ret);
}


/*********************************************************
 *
 * TemplateString(char *string, KeyStruct *firstkey)
 *
 * Convert a plain key stroke text to a chain och KeyStructs.
 * Input:  The string.
 *         KeyStruct pointer to attach the first key.
 * Result: A ret value.
 *         A chain to the 'firstkey' to be Deallocated.
 *******/
int __regargs TemplateString(char *string, KeyStruct *firstkey)
{
  int ret=OK, val;
  KeyStruct *currkey=firstkey;
  char *result;

  if (string) {
    int len;
    KeyStruct newkey={NULL, NULL, NULL, NULL};

    while (ret>=OK && string[0]) {
      val=-1;
      while (isspace(string[0])) ++string;
      {
		char *p=strchr(string, ' ');
        while (p && (*(p-1)=='\\'))
          p=strchr(p+1, ' ');
        if (p)
          len=p-string;
        else
          len=strlen(string);
      }
      if (len==5 && !strnicmp(string, "AMIGA", 5)) {
        newkey.Qual|=RAW_AMIGA;
	  } else if (len==5 && !Strnicmp(string, "SHIFT", 5))
        newkey.Qual|=RAW_SHIFT;
      else if ((len==4 && !Strnicmp(string, "CTRL", 4)) || (len==7 && !Strnicmp(string, "CONTROL", 7)))
        newkey.Qual|=RAW_CTRL;
      else if (len==3 && !Strnicmp(string, "ALT", 3))
        newkey.Qual|=RAW_ALT;
      else {
        int truelen=0;
        int count=0;
        while (truelen<len) {
          if (string[truelen]!='\\') {
            if (string[truelen]=='\'') {
                          /* Sträng sekvens att matchas från converttable*/
              register int counter;
              register int stop=(int)strchr(&string[++truelen], '\'');
              register BOOL found=FALSE;
              if (!stop)
                ret=SYNTAX_ERROR;
              else {
                stop=stop-(int)&string[truelen];
                for (counter=0; counter<converttablelength; counter++) {
                  if (strlen(converttable[counter][0])==stop &&
                      !Strnicmp((char *)(converttable[counter][0]), &string[truelen], stop)) {
                    if (converttable[counter][1][0]!=0xfb)
                      val=converttable[counter][1][0];
                    else
                      strcpy(&buffer[count], &converttable[counter][1][1]);
                    found=TRUE;
                    count+=strlen(&converttable[counter][1][1]);
                    truelen+=stop;
                    break;
                  }
                }
                if (!found) {
                  
                  val=(fplStrtol(&string[truelen], 0, &result))&255;
                  if (result-string)
                    truelen=result-string;
                  else
                    ret=SYNTAX_ERROR;
                }
              }
            } else
              buffer[count]=string[truelen];		/* Vanligt tecken */
          } else /* if (string[++truelen]!='x')*/ {
            if (string[truelen+1]==0)
              ret=SYNTAX_ERROR;
            else {
              count+=fplConvertString(Anchor, string, &buffer[count]);
              truelen=len;
            }
          }
          truelen++;
          count++;
        }
        if (ret>=OK) {
          buffer[count]=0;
          if (truelen>len)
            len=truelen;
          currkey->next=(KeyStruct *)Malloc(sizeof(KeyStruct));
          if (currkey->next) {
            currkey=currkey->next;
            currkey->Qual=newkey.Qual;
            currkey->string=NULL;
            currkey->rawkey=val;
            if (val<0)
              currkey->string=Strdup(buffer);
            currkey->next=NULL;
            newkey.Qual=NULL;
          } else {
            ret=OUT_OF_MEM;
          }
        } else {
          currkey=firstkey->next;
          while (currkey) {
            register KeyStruct *next=currkey->next;
            Dealloc(currkey->string);
            Dealloc(currkey);
            currkey=next;
          }
          firstkey->next=NULL;
        }
      }
      string+=len;
    }
  }
  return(ret);
}

/********************************************************
 *
 * DeleteKey(char *string)
 *
 * Delete the first key with given key stroke.
 * If no string is given, an input from the keyboard is taken.
 *****/
int __regargs DeleteKey(BufStruct *Storage, char *string)
{
  int ret=OK;
  BOOL stop=FALSE;

  struct Kmap *val=&Default.key;
  KeyStruct firstkey={NULL, NULL, NULL, NULL};
  KeyStruct *currkey=&firstkey;

  if (!string || !string[0]) {
    int counter=0;
    ret=FUNCTION_CANCEL;// Om inget tangenttryck kommer är detta resultatet.
    while (!stop) {
      Sprintf(buffer, RetString(STR_INPUT_KEY_STROKE), counter+1);
      Status(Storage, buffer, 3);
      
      if (GetKey(Storage, gkWAIT | gkSTRIP)<0 || strcmp(buffer, "\x1B")) {
        ret=OK;
        currkey->next=(KeyStruct *)Malloc(sizeof(KeyStruct));
        if (currkey->next) {
          currkey=currkey->next;
          currkey->Qual=((struct IntuiMessage *)(buffer+100))->Qualifier;
          currkey->string=Strdup(buffer);
          currkey->next=NULL;
          counter++;
        } else {
          ret=OUT_OF_MEM;
          stop=TRUE;
        }
      } else
        stop=TRUE;
    }
  } else
    ret=TemplateString(string, &firstkey);

  if (ret>=OK) {
    if (!firstkey.next)
      ret=FUNCTION_CANCEL;
    else {
      currkey=(KeyStruct *)firstkey.next;
      while (currkey) {
        register struct Kmap *temp;
        temp=GetKeyMap(val, currkey->Qual, (int)currkey->string, currkey->rawkey);
        if (!temp) {
          ret=-(int)RetString(STR_NO_KEYSTROKE_FOUND);
          break;	// Exit from while(currkey);
        } else {
          val=temp;
        }
        currkey=currkey->next;  
      }
      if (ret>=OK && val!=&Default.key)
        DeleteKmap(val);
    }
  }
  if (firstkey.next) {
    currkey=(KeyStruct *)firstkey.next;
    while (currkey) {
      Dealloc(currkey->string);
      {
        register KeyStruct *temp=currkey->next;
        Dealloc(currkey);
        currkey=temp;
      }
    }
  }

  return(ret);
}

/********************************************************
 *
 * FindKey(char *string)
 *
 * Find the first key with given key stroke.
 * Returns the FPL-program.
 *****/
char __regargs *FindKey(BufStruct *Storage, char *string)
{
  char *result=NULL;

  struct Kmap *val=&Default.key;
  KeyStruct firstkey={NULL, NULL, NULL, NULL};
  KeyStruct *currkey;

  if (TemplateString(string, &firstkey)>=OK) {
    if (firstkey.next) {
      currkey=(KeyStruct *)firstkey.next;
      while (val && currkey) {
        val=GetKeyMap(val, currkey->Qual, (int)currkey->string, currkey->rawkey);
        currkey=currkey->next;  
      }
      if (val) {
        if (val->FPLstring)
          result=val->FPLstring;
        else {
          MakeCall(val->Command, 0, NULL);
          result=buffer;
        }
      }
    }
  }
  if (firstkey.next) {
    currkey=(KeyStruct *)firstkey.next;
    while (currkey) {
      Dealloc(currkey->string);
      {
        register KeyStruct *temp=currkey->next;
        Dealloc(currkey);
        currkey=temp;
      }
    }
  }
  return(result);
}

/****************************************************
 *
 * DeleteKmap(struct Kmap *)
 *
 * Delete a KeyMap with its multiples
 *
 *********/
void __regargs DeleteKmap(struct Kmap *Kmap)
{
  while (Kmap->mul)
    DeleteKmap(Kmap->mul);

  if ((Kmap->father)->mul==Kmap)
    (Kmap->father)->mul=Kmap->next;
  else {
    register struct Kmap *temp=Kmap->father->mul;
    while (temp->next!=Kmap)
      temp=temp->next;
    temp->next=Kmap->next;
  }

  if (Kmap->menu) {
    WindowStruct *wincount;
    wincount=FRONTWINDOW;
    while (wincount) {
      menu_detach(&menu, wincount);
      wincount=wincount->next;
    }
#ifndef V39PLUS
    wincount=FRONTWINDOW;
    while (wincount) {
      if (SysBase->LibNode.lib_Version < 39)
        FrexxLayoutMenues(wincount->menus, FALSE);
      wincount=wincount->next;
    }
#endif
    Kmap->menu->keypress=NULL;
    wincount=FRONTWINDOW;
    while (wincount) {
      menu_attach(wincount);
      wincount=wincount->next;
    }
  }

  if (Kmap->flags & akFPLALLOC)
    Dealloc(Kmap->FPLstring);
  Dealloc(Kmap->depended);
  Dealloc(Kmap->string);
  Dealloc(Kmap);
}

/* Add another one to the list */
struct Kmap __regargs *AddKey(int flags, int qual,UWORD code,
 char *string, int cmd, struct Kmap *fatherkmap, char *depended)
{
  struct Kmap *val=(struct Kmap *)Malloc(sizeof(struct Kmap));
  struct Kmap *remap;
  struct Kmap *kmap=fatherkmap->mul;

#ifdef DEBUGTEST
  if (DebugOpt)
    FPrintf(Output(), "AssignKey\n");
#endif

  if (val) {
    memset(val, 0, sizeof(struct Kmap));
    if (string && kmap) {
      remap=kmap;
      if ((qual & ~RAW_AMIGA)) {
        while (kmap) {			/* Hitta första sträng keyn */
          if (kmap->string)
            break;
          remap=kmap;
          kmap=kmap->next;
        }
      } else {
        while (kmap) {			/* Hitta första string med qual keyn */
          if (kmap->string && !(kmap->Qual & ~RAW_AMIGA))
            break;
          remap=kmap;
          kmap=kmap->next;
        }
      }
      if (kmap && remap==fatherkmap->mul) {
        fatherkmap->mul=val;
        val->next=remap;
      } else {
        val->next=remap->next;
        remap->next=val;
      }
    } else {
      val->next=fatherkmap->mul;
      fatherkmap->mul=val;
    }
    val->Qual=qual&RAW_QUAL;
    val->Code=code;
    val->father=fatherkmap;
    val->flags=flags;
    if (flags & akCMD) {
      val->Command=(int)cmd;
    } else if (flags & akFPL) {
      val->FPLstring=(char *)cmd;
    } else if (flags & akFPLALLOC) {
      val->FPLstring=(char *)Strdup((char *)cmd);
    }

    val->menu=NULL;
    val->depended=NULL;
    if (depended && depended[0])
      val->depended=Strdup(depended);
  
    if (string) {
      val->string=(char *)Strdup(string);
    } else {
      val->string=NULL;
    }
    val->mul=NULL;
  }

  return(val);
}
