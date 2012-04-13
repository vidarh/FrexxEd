/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************
 *
 *  FACT.h
 *
 *********/

#include "compat.h"

#ifdef AMIGA
#include <exec/types.h>
#include <string.h>
#include <libraries/FPL.h>
#include <proto/utility.h>
#endif

#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "Change.h"
#include "Command.h"
#include "FACT.h"
#include "Init.h"
#include "Limit.h"
#include "Strings.h"
#include "UpdtScreen.h"
#include "UpdtScreenC.h"

extern FACT *DefaultFact;
extern FACT *UsingFact;
extern char buffer[];
extern int Visible;
extern DefaultStruct Default;
extern char FrexxEdStarted;
extern int UpDtNeeded;
extern char GlobalEmptyString[];

static void __regargs sub_check_fact(FACT *fact);

/***************************************
 *
 *  Will init and allocate a FACT struct to default.
 *
 *  Return: Your new FACT structure or NULL.
 ********/
FACT __regargs *InitFACT(void)
{
  char *mem;
  int memlength;
  int count;

  FACT *fact;

  if (!(mem=Malloc(memlength=sizeof(FACT)+256+256*sizeof(int)+256*sizeof(char *)+
                            256+256+256)))
    return(NULL);

  memset(mem, 0, memlength);
  fact=(FACT *)mem;
  mem+=sizeof(FACT);
  fact->flags=mem;
  mem+=256;
  fact->xflags=(int *)mem;
  mem+=256*sizeof(int);
  fact->strings=(char **)mem;
  mem+=256*sizeof(char *);
  fact->length=mem;
  memset(mem, 1, 256);		/* Alla strängar är ett tecken */
  mem+=256;
  fact->delimit=mem;
  mem+=256;
  fact->cases=mem;
  mem+=256;

  for (count=0; count<256; count++)
    fact->xflags[count]=factx_CLASS_SYMBOL|factx_DEFAULT;

  for (count=65; count<91; count++) {
    fact->xflags[count]=factx_UPPERCASE|factx_CLASS_WORD|factx_DEFAULT;
    fact->cases[count]=count+32;
    fact->xflags[count+32]=factx_LOWERCASE|factx_CLASS_WORD|factx_DEFAULT;
    fact->cases[count+32]=count;
  }
  for (count=0; count<32; count++) {
    fact->xflags[count+192]=factx_UPPERCASE|factx_CLASS_WORD|factx_DEFAULT;
    fact->cases[count+192]=count+224;
    fact->xflags[count+224]=factx_LOWERCASE|factx_CLASS_WORD|factx_DEFAULT;
    fact->cases[count+224]=count+192;
  }
  for (count='0'; count<='9'; count++) {
    fact->xflags[count]=factx_CLASS_WORD|factx_DEFAULT;
  }

  fact->flags['\n']=fact_NEWLINE | fact_STRING;
  fact->strings['\n']="";
  fact->length['\n']=0;

  fact->flags['\t']=fact_TAB | fact_STRING;
  fact->strings['\t']="\x01 \0x01";
  fact->length['\t']=1;

  fact->xflags['<']=fact->xflags['(']=fact->xflags['[']=fact->xflags['{']=factx_CLASS_OPEN|factx_CLASS_SYMBOL|factx_DEFAULT;
  fact->xflags['>']=fact->xflags[')']=fact->xflags[']']=fact->xflags['}']=factx_CLASS_CLOSE|factx_CLASS_SYMBOL|factx_DEFAULT;
  fact->xflags[' ']=fact->xflags['\t']=fact->xflags['\n']=factx_CLASS_SPACE|factx_DEFAULT;

  fact->delimit['(']=')';
  fact->delimit[')']='(';
  fact->delimit['[']=']';
  fact->delimit[']']='[';
  fact->delimit['{']='}';
  fact->delimit['}']='{';
  fact->delimit['<']='>';
  fact->delimit['>']='<';

  fact->newlinechar='\n';
  fact->tabchar='\t';

  {
    register char *eof=RetString(STR_END_OF_FILE);
    fact->extra[FACT_EOF].length=strlen(eof);
    fact->extra[FACT_EOF].string=Malloc(fact->extra[FACT_EOF].length*4);
    if (fact->extra[FACT_EOF].string) {
      register int count;
      for (count=0; count<fact->extra[FACT_EOF].length; count++) {
        fact->extra[FACT_EOF].string[count<<2]=cb_REVERSE;
        fact->extra[FACT_EOF].string[(count<<2)+1]=eof[count];
        *(short *)&fact->extra[FACT_EOF].string[(count<<2)+2]=1; // colour
      }
    } else
      fact->extra[FACT_EOF].length=0;
  }

  fact->next=NULL;
  fact->name=GlobalEmptyString;

  fact->extra[FACT_NO_EOL].string=NULL;
  fact->extra[FACT_NON_CHAR].string=NULL;
  fact->extra[FACT_NO_EOL].length=0;
  fact->extra[FACT_FOLDUP_CHAR].length=0;
  fact->extra[FACT_NOFOLD_CHAR].length=0;
  fact->extra[FACT_NON_CHAR].length=0;
  fact->extra[FACT_FOLDED_CHAR].string=Malloc(4);
  if (fact->extra[FACT_FOLDED_CHAR].string) {
    fact->extra[FACT_FOLDED_CHAR].string[0]=0;
    fact->extra[FACT_FOLDED_CHAR].string[1]='»';
    *(short *)&fact->extra[FACT_FOLDED_CHAR].string[2]=1; // colour
    fact->extra[FACT_FOLDED_CHAR].length=1;
  }
  fact->extra[FACT_FOLDUP_CHAR].string=Malloc(4);
  if (fact->extra[FACT_FOLDUP_CHAR].string) {
    fact->extra[FACT_FOLDUP_CHAR].string[0]=0;
    fact->extra[FACT_FOLDUP_CHAR].string[1]='|';
    *(short *)&fact->extra[FACT_FOLDUP_CHAR].string[2]=1; // colour
    fact->extra[FACT_FOLDUP_CHAR].length=1;
  }
  fact->extra[FACT_NOFOLD_CHAR].string=NULL;

  CheckFACT(fact);

  return(fact);
}

int __regargs CreateFact(char *name)
{
  int ret=OK;
  FACT *fact;
  FACT *factcount;

  if (!FindFACT(name)) {
    fact=InitFACT();
    if (fact) {
      factcount=DefaultFact;
      while(factcount->next)
        factcount=factcount->next;
      factcount->next=fact;
      fact->name=Strdup(name);
      {
        register BufStruct *Storage=&Default.BufStructDefault;
        do {
          if (!strcmp(BUF(fact_name), name))
            BUF(using_fact)=fact;
          Storage=BUF(NextBuf);
        } while (Storage);
      }
    } else
      ret=OUT_OF_MEM;
  }
  return(ret);
}

/***************************************
 *
 *  Will check and set 'newlinecheck'.
 *
 *  Return:  Nothing.
 ********/
static void __regargs sub_check_fact(FACT *fact)
{
  int counter;
  int antal=0;

  for (counter=255; counter>=0; counter--) {
    if (fact->flags[counter] & fact_NEWLINE) {
      antal++;
      fact->newlinechar=counter;
    } else if (fact->flags[counter] & fact_TAB) {
      fact->tabchar=counter;
    }
  }
  if (fact->flags['\n'] & fact_NEWLINE)
    fact->newlinechar='\n';
  if (antal>255)
    antal=255;
  fact->newlinecheck=antal;
  if (FrexxEdStarted)
    TestAllCursorPos();
}

void __regargs CheckFACT(FACT *fact)
{
  if (Visible==VISIBLE_ON || !FrexxEdStarted) {
    sub_check_fact(fact);
  } else
    UpDtNeeded|=UD_CHECK_FACT;
}


/***************************************
 *
 *  Will clear and deallocate a FACT struct.
 *
 *  Return:  The next FACT structure.
 ********/
FACT __regargs *ClearFACT(FACT *fact)
{
  int count;
  FACT *next=fact->next;

  for (count=0; count<256; count++) {
    if ((fact->xflags[count] & factx_ALLOCSTRING) && fact->strings[count])
      Dealloc(fact->strings[count]);
  }
  Dealloc(fact->name);
  for (count=0; count<FACT_EXTRA_STRINGS; count++)
    Dealloc(fact->extra[count].string);
  Dealloc(fact);
  return(next);
}

/********************************************************
 *
 * SetFACT(FACT *fact, int Argc, char **Argv)
 *
 * Replaces selected FACT with the content from the tags.
 *
 * First in the tags is the character that is affected
 * (-1==eofstring, -2==neolstring, -3-5=FOLDS).
 * Tags:
 * 'E' + flagga	= Erase flag.  flag=='-' => erase all.
 * '(' + tecken = OpenChar + delimit.
 * ')' + tecken = CloseChar + delimit.
 * 'W'          = ClassWord.
 * ' '		= ClassSpace.
 * '!'		= ClassSymbol.
 * 'N'		= NewLineChar.
 * 'T'		= TabChar.
 * 'U' + opposit= UpperCase + opposite.
 * 'L' + opposit= LowerCase + opposite.
 * 'S' + string = string to be inputted.
 *
 * All tag-characters are case insensitive.
 *
 *  Important: It is not possible to remove/add characters that
 *  creates a newline if any buffer or block buffer is using
 *  memory
 *
 * Return: A ret value.  (ie.  OK/SYNTAX_ERROR)
 *******/
int __regargs SetFACT(FACT *fact, int Argc, char **Argv, char *format)
{
  int counter;
  int ret=TRUE;
  char flag;
  int xflags;
  char *string=NULL;
  int stringlen=-1;
  char delimit;
  char cases;
  int tecken=(int)Argv[0]; /* The character we'll work on is first in the tags */

  if (format[0]==FPL_INTARG) {
    flag=fact->flags[tecken];
    xflags=fact->xflags[tecken]&(~factx_DEFAULT);
    delimit=fact->delimit[tecken];
    cases=fact->cases[tecken];
    for (counter=1; counter<Argc; counter++) {
      if (format[counter]!=FPL_INTARG) {
        ret=SYNTAX_ERROR;
        break;	// Break the for() loop
      }
      switch((int)ToUpper((int)Argv[counter])) {
      case 'E':		/* Erase flag + flag */
        ++counter;
        {
          register int val=(int)ToUpper((int)Argv[counter]);
          if (format[counter]!=FPL_INTARG)
            ret=SYNTAX_ERROR;
          if (val=='(')
            xflags=xflags&(~factx_CLASS_CLOSE);
          else if (val==')')
            xflags=xflags&(~factx_CLASS_OPEN);
          else if (val=='W')
            xflags=xflags&(~factx_CLASS_WORD);
          else if (val==' ')
            xflags=xflags&(~factx_CLASS_SPACE);
          else if (val=='!')
            xflags=xflags&(~factx_CLASS_SYMBOL);
          else if (val=='N') {
            flag=flag&(~fact_NEWLINE);
          } else if (val=='T')
            flag=flag&(~fact_TAB);
          else if (val=='U')
            xflags=xflags&(~factx_UPPERCASE);
          else if (val=='L')
            xflags=xflags&(~factx_LOWERCASE);
          else if (val=='S') {
            string=NULL;
            stringlen=0;
          } else if (val=='-') {
            string=NULL;
            stringlen=0;
            xflags=0;
            flag=0;
          } else
            ret=SYNTAX_ERROR;
        }
        break;
      case '(':		/* CLASS_OPEN + delimit */
        xflags|=factx_CLASS_OPEN;
        counter++;
        if (format[counter]!=FPL_INTARG)
          ret=SYNTAX_ERROR;
        delimit=(char)Argv[counter];
        break;
      case ')':		/* CLASS_CLOSE + delimit */
        xflags|=factx_CLASS_CLOSE;
        counter++;
        if (format[counter]!=FPL_INTARG)
          ret=SYNTAX_ERROR;
        delimit=(char)Argv[counter];
        break;
      case 'W':		/* CLASS_WORD */
        xflags|=factx_CLASS_WORD;
        break;
      case ' ':		/* CLASS_SPACE */
        xflags|=factx_CLASS_SPACE;
        break;
      case '!':		/* CLASS_SYMBOL */
        xflags|=factx_CLASS_SYMBOL;
        break;
      case 'N':		/* NEWLINE */
        flag|=fact_NEWLINE;
        break;
      case 'T':		/* TAB */
        flag|=fact_TAB;
        break;
      case 'U':		/* UPPER_CASE + opposite case*/
        xflags|=factx_UPPERCASE;
        counter++;
        if (format[counter]!=FPL_INTARG)
          ret=SYNTAX_ERROR;
        cases=(char)Argv[counter];
        break;
      case 'L':		/* LOWER_CASE + opposite case */
        xflags|=factx_LOWERCASE;
        counter++;
        if (format[counter]!=FPL_INTARG)
          ret=SYNTAX_ERROR;
        cases=(char)Argv[counter];
        break;
      case 'S':		/* String */
        counter++;
        if (format[counter]!=FPL_STRARG || strlen(Argv[counter])>MAX_CHAR_LINE)
          ret=SYNTAX_ERROR;
        else {
          register char *inputstring=Argv[counter];
          register int inputcount=0;
          register int destcount=0;
          register char type=cb_NORMAL;
          register char charac;
          short color=1;

          while (inputcount<(MAX_CHAR_LINE>>4) && inputstring[inputcount]) {
            charac=inputstring[inputcount];
            if (charac=='#') {
              inputcount++;
              charac=inputstring[inputcount];
              switch (charac) {
              case '#':
                break;
              case 0:
                inputcount--;
                charac='#';
                break;
              case '?':
                charac=' ';  //If it is a special string
                if (tecken>=0)
                  charac=tecken;  // annars ett tecken.
                break;
              default:
                switch(charac) {
                case 'N':
                case 'n':
                  type=cb_NORMAL;
                  break;
                case 'R':
                  type|=cb_REVERSE;
                  break;
                case 'B':
                  type|=cb_BOLD;
                  break;
                case 'U':
                  type|=cb_UNDERLINE;
                  break;
                case 'I':
                  type|=cb_ITALIC;
                  break;
                case 'r':
                  type&=~cb_REVERSE;
                  break;
                case 'b':
                  type&=~cb_BOLD;
                  break;
                case 'u':
                  type&=~cb_UNDERLINE;
                  break;
                case 'i':
                  type&=~cb_ITALIC;
                  break;
                case 'C': //Color
                  {
                    register int count;
                    char temp;
                    color=0;
                    for (count=0; count<4; count++) {
                      inputcount++;
                      if (temp=inputstring[inputcount]) {
                        if (temp<='9')
                          temp-='0';
                        else if (temp<='F')
                          temp-='F';
                        else
                          temp-='f';
                        color=temp+(color<<4);
                      } else {
                        inputcount-=2;
                        break;
                      }
                    }
                  }
                  break;
                }
                inputcount++;
                continue; // continue loop
              }  //end switch
            }  //endif
            if (!charac)
              break;	/* end of string */
            buffer[destcount++]=type&(cb_NORMAL|cb_REVERSE|cb_BOLD|cb_ITALIC|cb_UNDERLINE);	// Ta ut de flaggor som man får använda.
            buffer[destcount++]=charac;
            *(short *)&buffer[destcount]=color;
            destcount+=2;
            inputcount++;
          }
          stringlen=destcount>>1;
          buffer[destcount]=0;
          string=buffer;
        }
        break;
      default:
        ret=SYNTAX_ERROR;
        counter=Argc;
        break;
      }
    }
    if (ret==OK) {
      if (tecken<0) {
        if (stringlen>=0) {
          if (!string)
            stringlen=0;
          tecken=-tecken-1;
          if (tecken<FACT_EXTRA_STRINGS) {
            Dealloc(fact->extra[tecken].string);
            fact->extra[tecken].length=0;
            fact->extra[tecken].string=Malloc(stringlen*2);
            if (fact->extra[tecken].string) {
              fact->extra[tecken].length=stringlen>>1;
              memcpy(fact->extra[tecken].string, string, stringlen*2);
            }
          }
          CopyLineNumberWidth();
        }
      } else {
        int oldnewline=DefaultFact->flags[tecken]&fact_NEWLINE;
  
        if (stringlen>=0) {
          if ((fact->xflags[tecken] & factx_ALLOCSTRING))
            Dealloc(fact->strings[tecken]);
          fact->length[tecken]=stringlen>>1;
          if (!string) {
            fact->strings[tecken]=NULL;
            fact->length[tecken]=1;
          } else {
            fact->strings[tecken]=Malloc(stringlen*2);
            if (fact->strings[tecken]) {
              memcpy(fact->strings[tecken], string, stringlen*2);
              flag|=fact_STRING;
              xflags|=factx_ALLOCSTRING;
            } else
              fact->length[tecken]=0;
          }
        }
        fact->delimit[tecken]=delimit;
        fact->cases[tecken]=cases;
        fact->xflags[tecken]=xflags;
        if (oldnewline!=(flag&fact_NEWLINE)) {
          if (!CheckNotStarted())
             flag^=fact_NEWLINE;
        }
        fact->flags[tecken]=flag;
        CheckFACT(fact);
      }
      if (FrexxEdStarted) {
        {
          WindowStruct *win=FRONTWINDOW;
          while (win) {
            PrepareRedrawOfWindow(win);
            win=win->next;
          }
        }
        UpdateAll();
      }
    }
  } else
    ret=SYNTAX_ERROR;
  return(ret);
}

/***************************************************************
 *
 *  FACTConvertString(char *string, int len, String *result)
 *
 * Convert a string according to the FACT
 *
 * Give the string + its length
 * Resultatet ges i den String-struktur som ges.
 * flag=TRUE gives a string to print on screen
 * Return a ret-value.
 *******/
int __regargs FACTConvertString(BufStruct *Storage, char *string, int len, String *result, int flag)
{
  int counter;
  int mem=len+(BUF(tabsize)<<1);
  int rescount=0;
  int ret=OK;
  char temp[2]="";
  char *stringpoint;
  int cursorpos=0;
  int repeat;
  int downcount=-1;

  if (flag) {
    mem=mem<<1;
    downcount=BUF(screen_col);
  }
  result->length=0;
  result->string=Malloc(mem+10);
  if (!result->string)
    return(OUT_OF_MEM);

  counter=0;
  while (counter<len) {
    if (mem<=BUF(using_fact)->length[string[counter]]+rescount+BUF(tabsize)) {
      mem+=(len-counter+(BUF(tabsize)<<1))<<(flag?1:0);
      result->string=Realloc(result->string, mem+20);
      if (!result->string)
        return(OUT_OF_MEM);
    }

    if (BUF(using_fact)->strings[string[counter]])
      stringpoint=BUF(using_fact)->strings[string[counter]];
    else {
      stringpoint=temp;
      temp[1]=string[counter];
    }
    repeat=BUF(using_fact)->length[string[counter]];
    if (BUF(using_fact)->flags[string[counter]] & fact_TAB)
      repeat=BUF(tabsize)-(cursorpos%BUF(tabsize));
    if (repeat) {
      register int count=0;
      while (repeat-- && downcount) {
        downcount--;
        if (flag) {
          result->string[rescount]=stringpoint[(count<<2)];
          rescount++;
        }
        result->string[rescount]=stringpoint[(count<<2)+1];
        rescount++;
        cursorpos++;
        count++;
        if (count==BUF(using_fact)->length[string[counter]])
          count=0;
      }      
    }
    if (BUF(using_fact)->flags[string[counter]] & fact_NEWLINE) {
      if (!flag) {
        result->string[rescount]=BUF(using_fact)->newlinechar;
        rescount++;
      } else {
        result->string[rescount++]=cb_NEWLINE;
        result->string[rescount++]=' ';
        downcount=BUF(screen_col);
      }
      cursorpos=0;
    }
    counter++;
  }
  result->length=rescount;
  return(ret);
}

int __regargs FactInfo(FACT *fact, int type, short tkn)
{
  int retint=-1;
  short color=1;

  switch(type) {
  case SC_ISUPPER:
    if (fact->xflags[(char)tkn] & factx_UPPERCASE)
      retint=fact->cases[(char)tkn];
    break;
  case SC_ISLOWER:
    if (fact->xflags[(char)tkn] & factx_LOWERCASE)
      retint=fact->cases[(char)tkn];
    break;
  case SC_ISSPACE:
    retint=fact->xflags[(char)tkn] & factx_CLASS_SPACE;
    break;
  case SC_ISWORD:
    retint=fact->xflags[(char)tkn] & factx_CLASS_WORD;
    break;
  case SC_ISSYMBOL:
    retint=fact->xflags[(char)tkn] & factx_CLASS_SYMBOL;
    break;
  case SC_ISOPEN:
    if (fact->xflags[(char)tkn] & factx_CLASS_OPEN)
      retint=fact->delimit[(char)tkn];
    break;
  case SC_ISCLOSE:
    if (fact->xflags[(char)tkn] & factx_CLASS_CLOSE)
      retint=fact->delimit[(char)tkn];
    break;
  case SC_ISTAB:
    retint=fact->flags[(char)tkn] & fact_TAB;
    break;
  case SC_ISNEWLINE:
    retint=fact->flags[(char)tkn] & fact_NEWLINE;
    break;
  case SC_ISSTRING:
    {
      register int counter=0;
      register int len=0;
      register char *string=NULL;
      if (tkn>=0) {
        if (fact->flags[(char)tkn] & fact_STRING) {
          string=fact->strings[(char)tkn];
          len=fact->length[(char)tkn];
        } else {
          buffer[0]=(char)tkn;
          retint=257;
          break;
        }
      } else if (tkn<0 && tkn>=-FACT_EXTRA_STRINGS) {
        string=fact->extra[-tkn-1].string;
        len=fact->extra[-tkn-1].length;
      }
      if (string) {
        register int bufcnt=0;
        register int type=0;
        while (counter<(len<<2)) {
          while (type!=(string[counter] & (cb_REVERSE|cb_BOLD|cb_ITALIC|cb_UNDERLINE))) {
            buffer[bufcnt++]='#';
            if (!string[counter]) {
              buffer[bufcnt++]='N';
              type=0;
            } else if ((type & cb_REVERSE)!=(string[counter]&cb_REVERSE)) {
              if (type & cb_REVERSE)
                buffer[bufcnt++]='r';
              else
                buffer[bufcnt++]='R';
              type^=cb_REVERSE;
            } else if ((type & cb_BOLD)!=(string[counter]&cb_BOLD)) {
              if (type & cb_BOLD)
                buffer[bufcnt++]='b';
              else
                buffer[bufcnt++]='B';
              type^=cb_BOLD;
            } else if ((type & cb_ITALIC)!=(string[counter]&cb_ITALIC)) {
              if (type & cb_ITALIC)
                buffer[bufcnt++]='i';
              else
                buffer[bufcnt++]='I';
              type^=cb_ITALIC;
            } else if ((type & cb_UNDERLINE)!=(string[counter]&cb_UNDERLINE)) {
              if (type & cb_UNDERLINE)
                buffer[bufcnt++]='u';
              else
                buffer[bufcnt++]='U';
              type^=cb_UNDERLINE;
            }
          }
          if (color!=(*(short *)(&string[counter+2]))) {
            color=(*(short *)(&string[counter+2]));
            buffer[bufcnt++]='C';
            Sprintf(&buffer[bufcnt], "%04lx", color);
            bufcnt+=4;
          }
          counter++;
          buffer[bufcnt++]=string[counter];
          counter++;
          counter+=2; // jump over color
        }
        retint=bufcnt+256;
      } else {
        retint=256;
      }
    }
    break;
  }
  return(retint);
}

BOOL __regargs CheckNotStarted()
{
  register BOOL ret=TRUE;
  if (DefaultFact->next)
    ret=FALSE;
  else {
    register BufStruct *Storage2=Default.BufStructDefault.NextBuf;
    while(ret && Storage2) {
      if (Storage2->shared->size || Storage2->shared->Undomem) {
        ret=FALSE;
      }
      Storage2=Storage2->NextBuf;
    }
  }
  return(ret);
}


int __regargs ResetFACT(FACT *fact, int tecken)
{
  FACT *newfact;
  int ret=OK;

  newfact=InitFACT();
  if (newfact) {
    if (!CheckNotStarted()) {
      register int counter=0;
      if (tecken<256) {
        counter=tecken;
      }
      for (; counter<256; counter++) {
        if (DefaultFact->flags[counter]&fact_NEWLINE)
          newfact->flags[counter]|=fact_NEWLINE;
        else {
          if (newfact->flags[counter]&fact_NEWLINE)
            newfact->flags[counter]&=~fact_NEWLINE;
        }
        if (tecken<256)
          break;
      }
    }
    if (tecken>255) {
      if (fact==DefaultFact)
        UsingFact=DefaultFact=newfact;
      else {
        FACT *factcount=DefaultFact;
        while (factcount->next!=fact)
          factcount=factcount->next;
        factcount->next=newfact;
        newfact->name=fact->name;	// Copy the name
        fact->name=NULL;
      }
      newfact->next=fact->next;
      NewFACT(newfact, fact);
      ClearFACT(fact);
//      UsingFact=newfact;
//      DefaultFact=newfact;
      sub_check_fact(newfact);
    } else {
      register char *temp=NULL;
      if (tecken<0) {
        tecken=-tecken-1;
        if (tecken<FACT_EXTRA_STRINGS) {
          temp=Malloc(newfact->extra[tecken].length<<1);
          if (temp) {
            memcpy(temp, newfact->extra[tecken].string, newfact->extra[tecken].length<<1);
            fact->extra[tecken].length=newfact->extra[tecken].length;
            Dealloc(fact->extra[tecken].string);
            fact->extra[tecken].string=temp;
          } else
            ret=OUT_OF_MEM;
        }
      } else {
        if (newfact->strings[tecken] && newfact->length[tecken]<<1) {
          temp=Malloc(newfact->length[tecken]<<1);
          if (!temp)
            ret=OUT_OF_MEM;
        }
        if (ret>=OK) {
          if (temp)
            memcpy(temp, newfact->strings[tecken], newfact->length[tecken]<<1);
          if (fact->xflags[tecken] & factx_ALLOCSTRING)
            Dealloc(fact->strings[tecken]);
          fact->strings[tecken]=temp;
          fact->flags[tecken]=newfact->flags[tecken];
          fact->xflags[tecken]=(newfact->xflags[tecken]&(~factx_DEFAULT))|factx_ALLOCSTRING;
          fact->delimit[tecken]=newfact->delimit[tecken];
          fact->cases[tecken]=newfact->cases[tecken];
          fact->length[tecken]=newfact->length[tecken];
        }
      }
      ClearFACT(newfact);
      sub_check_fact(newfact);
    }
    if (FrexxEdStarted)
      UpdateAll();
  } else
    ret=OUT_OF_MEM;
  return(ret);
}


FACT __regargs *FindFACT(char *name)
{
  register FACT *factcount=DefaultFact;
  while (factcount && strcmp(factcount->name, name))
    factcount=factcount->next;
  return(factcount);
}

int __regargs FACTDelete(char *name)
{
  FACT *factcount=DefaultFact;
  FACT *fact=FindFACT(name);
  if (fact && name[0]) {
    while (factcount->next!=fact)
      factcount=factcount->next;
    factcount->next=fact->next;
    ClearFACT(fact);
  }
  return(OK);
}

void __regargs NewFACT(FACT *newfact, FACT *oldfact)
{
  BufStruct *Storage=&Default.BufStructDefault;
  do {
    if (BUF(using_fact)==oldfact)
      BUF(using_fact)=newfact;
    Storage=BUF(NextBuf);
  } while (Storage);
}
