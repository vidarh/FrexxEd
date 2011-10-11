/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/*
 * ARexx interface setup routines...
 */

#include "compat.h"

#ifdef AMIGA
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include <proto/rexxsyslib.h>
#include <proto/utility.h>
#include <proto/FPL.h>
#ifndef __AROS__
#include <inline/fpl_protos.h>
#endif
#include <libraries/FPL.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <rexx/errors.h>
#include <string.h>
#include <clib/alib_protos.h>
#endif

extern char DebugOpt;

#include "Buf.h"
#include "Alloc.h"
#include "Execute.h"
#include "Rexx.h"

extern char *appiconname;

/*
 * This function returns a structure that contains the commands sent from
 * ARexx...  You will need to parse it and return the structure back
 * so that the memory can be freed...
 *
 * This returns NULL if there was no message...
 */
struct RexxMsg __regargs *GetARexxMsg(AREXXCONTEXT RexxContext)
{
  register struct RexxMsg *tmp=NULL;
#if 0
  register short flag;
#endif  

  if (RexxContext)
    if (tmp=(struct RexxMsg *)GetMsg(RexxContext->ARexxPort)) {
      if (tmp->rm_Node.mn_Node.ln_Type==NT_REPLYMSG) {
       /*
        * If we had sent a command, it would come this way...
        */
#if 0
       if (tmp->rm_Result1)
         flag=TRUE;
       else
         flag=FALSE;
#endif
       /*
        * Free the arguments and the message...
        */
       DeleteArgstring(tmp->rm_Args[0]);
       DeleteRexxMsg(tmp);
       RexxContext->Outstanding--;
       
       /*
        * Return NULL since this was a reply!
        */
       tmp=NULL;
      }
    }
  return(tmp);
}

/*
 * Use this to return a ARexx message...
 *
 */
void __regargs
ReplyARexxMsg(AREXXCONTEXT RexxContext)
{
  struct RexxMsg *rmsg;
  if (RexxContext && (rmsg=RexxContext->rmsg)) {
    if (rmsg!=REXX_RETURN_ERROR) {
      rmsg->rm_Result1= RexxContext->ResultCode;
      /*
       * we return the string
       */
      if (rmsg->rm_Action & (1L << RXFB_RESULT)) {
        if (RexxContext->Result && !rmsg->rm_Result1)
          rmsg->rm_Result2=(LONG)CreateArgstring(RexxContext->Result,
                                                 (LONG)strlen(RexxContext->Result));
        else
          rmsg->rm_Result2=0;
      }
  
      /*
       * Reply the message to ARexx...
       */
      ReplyMsg((struct Message *)rmsg);
      RexxContext->ResultCode=0;
      if (RexxContext->Result) {
        Dealloc(RexxContext->Result);
        RexxContext->Result=NULL;
      }
    }
  }
}

long __regargs
SetARexxVar(AREXXCONTEXT RexxContext,
            char *variable,
            char *string)
{
  struct RexxMsg *rmsg;
  register short OkFlag=FALSE;

  /* OBS funktionen utnyttjar amiga.lib */
  if (RexxContext &&
      (rmsg=RexxContext->rmsg) &&
      CheckRexxMsg( (struct Message *)rmsg ) ) {

    /*
     * Note that SetRexxVar() has more than just a TRUE/FALSE
     * return code, but for this "basic" case, we just care if
     * it works or not.
     */

    /* OBS funktionen utnyttjar amiga.lib */
    OkFlag = SetRexxVar((struct Message*)rmsg,
                        variable,
                        string,		
                        (long)strlen(string));
  }
  return(OkFlag);
}

long __regargs
GetARexxVar(AREXXCONTEXT RexxContext,
            char *variable,
            char **string)
{
  struct RexxMsg *rmsg;
  register short OkFlag=FALSE;

  /* OBS funktionen utnyttjar amiga.lib */
  if (RexxContext &&
      (rmsg=RexxContext->rmsg) &&
      CheckRexxMsg( (struct Message *)rmsg ) ) {

    /*
     * Note that SetRexxVar() has more than just a TRUE/FALSE
     * return code, but for this "basic" case, we just care if
     * it works or not.
     */

    /* OBS funktionen utnyttjar amiga.lib */
    OkFlag = GetRexxVar((struct Message*)rmsg,
                        variable,
                        string);
  }
  return(OkFlag);
}

/*
 * This function will send a string to ARexx...
 *
 * The default host port will be that of your task...
 *
 * If you set StringFile to TRUE, it will set that bit for the message...
 *
 * Returns TRUE if it send the message, FALSE if it did not...
 */

short __regargs
SendARexxMsg(void *Storage,
             AREXXCONTEXT RexxContext,
             void *FPLkey, /* for the fpl.library call */
             char *arexxport,
             char *string,
	     int timeout) /* in seconds */
{
  struct MsgPort *RexxPort;
  struct MsgPort *reply;
  struct RexxMsg *rmsg;
  struct RexxMsg *replymsg;
  short flag=TRUE;
  
  if (RexxContext) {
    if (string) {
      if(timeout) {
        timeout *= 5; /* to make it number of loops below! */
        if(!RexxContext->ReplyPort)
          RexxContext->ReplyPort = CreateMsgPort();
	reply = RexxContext->ReplyPort;
      }
      else {
        /* We don't care about the return! */
	reply = RexxContext->ARexxPort; /* send it to our main port! */
      }
      if (reply && (rmsg=CreateRexxMsg(reply,
                                       NULL,
                                       timeout?NULL:RexxContext->PortName))) {

        rmsg->rm_Action=RXCOMM | (timeout?RXFF_RESULT:0);

        if (rmsg->rm_Args[0]=CreateArgstring(string, (LONG)strlen(string))) {
          /*
           * We need to find the RexxPort and this needs
           * to be done in a Forbid()
           */
          Forbid();
          if (RexxPort=FindPort(arexxport)) {
            /*
             * We found the port, so put the
             * message to ARexx...
             */
            PutMsg(RexxPort,(struct Message *)rmsg);
	    if(timeout)
	      RexxContext->ReplyPortList++;
	    else
              RexxContext->Outstanding++;
            flag=FALSE;
          } else {
            /*
             * No port, so clean up...
             */
            DeleteArgstring(rmsg->rm_Args[0]);
            DeleteRexxMsg(rmsg);
          }
          Permit();
        } else
          DeleteRexxMsg(rmsg);

	if(timeout && rmsg->rm_Args[0]) {
	  do {
            replymsg = (struct RexxMsg *)GetMsg(reply);
	    if(replymsg) {
              if(replymsg == rmsg)
	        break; /* this is our message! */
              /*
               * This is an old message we timeouted, delete it!
               */
              DeleteArgstring((UBYTE *)replymsg->rm_Result2);

	      DeleteArgstring(replymsg->rm_Args[0]);
              DeleteRexxMsg(replymsg);
	      RexxContext->ReplyPortList--;
	      continue;
	    }
	    if(!--timeout)
	      break;
	    Delay(10);
	  } while(!StopCheck(Storage));
	  
	  if(replymsg && replymsg->rm_Result1 == RC_OK) {
            fplSendTags(FPLkey, FPLSEND_STRING, (char *)replymsg->rm_Result2,
                                FPLSEND_DONE);
            DeleteArgstring((UBYTE *)replymsg->rm_Result2);
	    flag=FALSE;
	  }
	  if(replymsg) {
            RexxContext->ReplyPortList--;
            DeleteArgstring(replymsg->rm_Args[0]);
            DeleteRexxMsg(replymsg);
    	    /*
	     * Eat all other messages and delete the port!
	     */
	    Forbid();
	    while(replymsg=(struct RexxMsg *)GetMsg(reply)) {
	      RexxContext->ReplyPortList--;
              ReplyMsg((struct Message *)replymsg);
	    }
	    if(0 == RexxContext->ReplyPortList)
	      DeleteMsgPort(reply);
	    Permit();
	    if(!RexxContext->ReplyPortList)
	      RexxContext->ReplyPort = NULL;
	  }
	}
      }
    }
  }
  return(flag);
}

/*
 * This function closes down the ARexx context that was opened
 * with InitARexx...
 */
void __regargs FreeARexx(AREXXCONTEXT RexxContext)
{
  register struct RexxMsg *rmsg;
  if (RexxContext) {
    /*
     * Clear port name so it can't be found...
     */
    RexxContext->PortName[0]='\0';
      
    /*
     * Clean out any outstanding messages we had sent out...
     */
    while (RexxContext->Outstanding) {
      WaitPort(RexxContext->ARexxPort);
      while (rmsg=GetARexxMsg(RexxContext)) {
    	if (rmsg!=REXX_RETURN_ERROR) {
    	  ReplyARexxMsg(RexxContext);
    	}
      }
    }

    /*
     * Clean out any outstanding messages we had sent out...
     */
    while (RexxContext->ReplyPortList) {
      WaitPort(RexxContext->ReplyPort);
      while (rmsg=(struct RexxMsg *)GetMsg(RexxContext->ReplyPort)) {
        ReplyMsg((struct Message *) rmsg);
	RexxContext->ReplyPortList--;
      }
    }
    
    /*
     * Clean up the port and delete it...
     */
    if (RexxContext->ARexxPort) {
      while (rmsg=GetARexxMsg(RexxContext)) {
	ReplyARexxMsg(RexxContext);
      }
      DeletePort(RexxContext->ARexxPort);
    }
      
    /*
     * Make sure we close the library...
     */
    if (RexxSysBase)
      CloseLibrary((struct Library *)RexxSysBase);
      
    /*
     * Free the memory of the RexxContext
     */
    FreeMem(RexxContext,sizeof(struct ARexxContext));
  }
}

/*
 * This routine initializes an ARexx port for your process
 * This should only be done once per process.  You must call it
 * with a valid application name and you must use the handle it
 * returns in all other calls...
 *
 */
AREXXCONTEXT __regargs InitARexx(char *AppName) 
{
  register  AREXXCONTEXT  RexxContext;
  register  short   loop;
  register  short   count;
  register  char    *tmp;
  BOOL stop=FALSE;

#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "Alloc ARexx      \r");
#endif
  RexxContext=AllocMem(sizeof(struct ARexxContext), MEMF_PUBLIC|MEMF_CLEAR);
  if ( RexxContext ) {

    if (AppName && !AppName[0])
      AppName=NULL;

#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "Open ARexx      \r");
#endif
    RexxSysBase=OpenLibrary("rexxsyslib.library", NULL);
    if ( RexxSysBase ) {
      /*
       * Set up the extension...
       */
      strcpy(RexxContext->Extension, "rexx");

      /*
       * Set up a port name...
       */

      while (!stop && !RexxContext->ARexxPort) {
#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "Find name      \r");
#endif
        tmp=RexxContext->PortName;
        if (!AppName) {
          strcpy(RexxContext->PortName, "FREXXED.");
          tmp+=8;
        } else {
          for (loop=0; (loop<16)&&(AppName[loop]); loop++)
            *tmp++ = ToUpper(AppName[loop]);
          *tmp='\0';
        }
        /*
         * Set up the last error RVI name...
         *
         * This is <appname>.LASTERROR
         */
#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "Setup error         \r");
#endif
        strcpy(RexxContext->ErrorName,RexxContext->PortName);
        strcat(RexxContext->ErrorName,".LASTERROR");
  
        /* We need to make a unique port name... */
#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "Make port      \r");
#endif
        Forbid();
        if (AppName) {
          RexxContext->ARexxPort=FindPort(RexxContext->PortName);
          if (RexxContext->ARexxPort) {
            RexxContext->ARexxPort=NULL;
          } else {
            RexxContext->ARexxPort=CreatePort( RexxContext->PortName,NULL );
            Dealloc(appiconname);
            appiconname=Strdup(AppName);
          }
          AppName=NULL;
        } else {
          RexxContext->ARexxPort=(VOID *)1;
          for (count=1;RexxContext->ARexxPort;count++) {
            Sprintf(tmp, "%ld", count);		//941214- used to be 'stci_d(tmp,count)'
            RexxContext->ARexxPort=FindPort(RexxContext->PortName);
          }
#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "%s            \n", RexxContext->PortName);
#endif
          if (count!=2) {
            Dealloc(appiconname);
            appiconname=Strdup(RexxContext->PortName);
          }
          RexxContext->ARexxPort=CreatePort( RexxContext->PortName,NULL );
          stop=TRUE;
        }
        Permit();
      }
    }
#ifdef DEBUGTEST
  if(DebugOpt)
    FPrintf(Output(), "Finished ARexx      \r");
#endif
    if (  (!(RexxSysBase)) || (!(RexxContext->ARexxPort)) ) {
      FreeARexx(RexxContext);
      RexxContext=NULL;
    }
  }
  return(RexxContext);
}
