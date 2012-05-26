/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/**************************************************************
 *
 * ExecuteA.c
 *
 * Contains Amiga-specific utility functions for generic/Execute.c
 * that must be replaced to port
 *
 *******/

#ifdef AMIGA
#include <devices/console.h>
#include <devices/inputevent.h>
#include <dos/dostags.h>
#include <dos/var.h>
#include <exec/types.h>
#include <exec/tasks.h>
#include <graphics/gfxmacros.h>
#undef GetOutlinePen
#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/gadtools.h>
#include <libraries/reqtools.h>
#include <proto/FPL.h>
#include <proto/console.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/reqtools.h>
#include <proto/utility.h>
#include <rexx/errors.h>
#endif


/**********************************************************************
 *
 * StopCheck();
 *
 * This is the inverval function that checks if the FPL interpretation
 * should be abandoned. Whenever escape is pressed, the FPL will stop!
 *
 *****/

long StopCheck(register __a0 BufStruct *Storage)
{
  register long ret=FALSE;
{//}  if (fplabort) {
    int val;
/*
    {
      static BOOL stop_is_going_round=FALSE;
      while (!stop_is_going_round && important_message_available) {
        register ReturnMsgStruct *retmsg;
        stop_is_going_round=TRUE;
        retmsg=GetReturnMsg(cmd_DEVICE);
        if (retmsg) {
          SHS(current)++;
          BUF(locked)++;
          RunHook(Storage, DO_FILEHANDLER_EXCEPTION, 2, (char **)&retmsg->args, NULL);
          BUF(locked)--;
          SHS(current)--;
          NewStorageWanted=NULL;
        }
        important_message_available--;
        stop_is_going_round=FALSE;
      }
    }
*/
    if (device_want_control) {
      LockBuf_release_semaphore(((BufStruct *)Storage)->shared);
      UnLockBuf_obtain_semaphore(((BufStruct *)Storage)->shared);
    }
//    val=GetKey(Storage, gkNOTEQ|gkNOCHACHED);
    val=GetKey(NULL, gkNOTEQ|gkNOCHACHED); /* 960111 */
    if (val>=0) {
      if (val==RAW_ESC &&
          ((((struct IntuiMessage *)(buffer+100))->Qualifier)&RAW_QUAL)==RAW_CTRL) {
        ret=TRUE;
        while (GetReturnMsg(cmd_KEY))
          ;
      } else
        SendReturnMsg(cmd_KEY, 0, buffer+100, NULL, NULL);
    }
  }
  return(ret);
}

int ScGetVar(const char * argv, char * buffer)
{
    return GetVar((char *)argv, buffer, MAX_CHAR_LINE, GVF_GLOBAL_ONLY|GVF_LOCAL_ONLY|GVF_BINARY_VAR);
}

int ScSetVar(const char * argv, const char * buffer, int len)
{
    return SetVar((char *)argv, buffer, len, GVF_GLOBAL_ONLY);
}

int ScGetInt(const char * prompt, char * line, short show_default) {
    return rtGetLong((ULONG *)&line, prompt,
                     NULL, RTGL_ShowDefault, show_default ? TRUE:FALSE,
                     ((FRONTWINDOW && FRONTWINDOW->screen_pointer)?RT_Screen:TAG_IGNORE), FRONTWINDOW?FRONTWINDOW->screen_pointer:NULL,
                     RTGS_TextFmt, (arg->argc>2)?(char *)arg->argv[2]:NULL,
                     RTGL_BackFill, !(arg->argc>2),
                     RT_TextAttr, &Default.RequestFontAttr,
                     TAG_END);
}

long ScRandom() {
    static long lastrandom=0;
    DateStamp((struct DateStamp *)buffer);
    lastrandom=(lastrandom+1)*0xff5d;
    return lastrandom* (((struct DateStamp *)buffer)->ds_Days+1) *
        (((struct DateStamp *)buffer)->ds_Minute+1) +
        (((struct DateStamp *)buffer)->ds_Tick+1);
}


const char * ScGetDate(int argc, const char ** arg, char * buffer) {
    struct DateTime datetime;
    register int format=0;

    if (SHS(date.ds_Days)==-1) return 0;

    memcpy(&datetime, &Storage2->shared->date, sizeof(struct DateStamp));
    if (argc>1) {
        format=(int)argv[1];
        if ((int)argv[0]<0)
            DateStamp((struct DateStamp *)&datetime);
    }
    datetime.dat_Format=(format>>4)&7;//FORMAT_DOS
    datetime.dat_Flags=(format>>8)&7;//DTF_SUBST;
    datetime.dat_StrDay=NULL;
    datetime.dat_StrDate=buffer+100;
    datetime.dat_StrTime=buffer+120;
    DateToStr(&datetime);
    buffer[0]=0;
    if ((format&3)!=2)
        strcpy(buffer, buffer+100);
    if ((format&3)!=1) {
        strcat(buffer, " ");
        strcat(buffer, buffer+120);
    }
    return buffer;
}

int ScGetString(char * buffer, const char * msg, const char * textfmt, int backfill)
{
      return rtGetString(buffer, MAX_CHAR_LINE, msg, NULL,
                          ((FRONTWINDOW && FRONTWINDOW->screen_pointer)?RT_Screen:TAG_IGNORE), 
                          FRONTWINDOW?FRONTWINDOW->screen_pointer:NULL,
                          RTGS_TextFmt, textfmt,
                          RTGS_AllowEmpty, TRUE,
                          RTGL_BackFill, backfill,
                          RT_TextAttr, &Default.RequestFontAttr,
                          TAG_END);
}

int ScPrompFile() {
          if(PromptFile(Storage, fullname, arg->argc>1?(char *)arg->argv[1]:NULL, arg->argc>2?(char *)arg->argv[2]:NULL, flags, (APTR *)&filelist)) {
            if (flags&FREQF_MULTISELECT) {
              struct rtFileList *filetemp=filelist;
              if (filetemp) {
                struct fplRef ref;
                tempint=0;
                while (filetemp) {
                  tempint++;
                  filetemp=filetemp->Next;
                }
                ref.Dimensions=1;
                ref.ArraySize=(long *)&tempint;
                fplReferenceTags(Anchor, (void *)(arg->argv[4]),
                                 FPLREF_ARRAY_RESIZE, &ref,
                                 FPLREF_END);
                filetemp=filelist;
                tempint=0;
                do {
                  dims[0]=tempint;
                  strmfp(buffer, fullname, filetemp->Name);
                  fplReferenceTags(Anchor, (void *)(arg->argv[4]),
                                   FPLREF_ARRAY_ITEM, &dims[0],
                                   FPLREF_SET_MY_STRING, buffer,
                                   FPLREF_END);
                  tempint++;
                  filetemp=filetemp->Next;
                } while (filetemp);
              }
              ReturnInt=&tempint;	// Return number of entries.
              rtFreeFileList(filelist);
            } else
              fplSendTags(arg->key, FPLSEND_STRING, fullname, FPLSEND_DONE);
          } else
            ret=FUNCTION_CANCEL;
          Dealloc(fullname);
}


int ScArexxResult()
{
    /*
     * Result string to send to ARexx if we were invoke from there!
     */
    if(RexxHandle) {
        RexxHandle->ResultCode = (int)arg->argv[0]?RC_ERROR:0;
        if(arg->argc>1) {
            if(RexxHandle->Result)
                Dealloc(RexxHandle->Result);
            if(!RexxHandle->ResultCode)
                /*
                 * Only set the return string if no error is reported!
                 */
                RexxHandle->Result=Strdup(arg->argv[1]);
            else
                RexxHandle->Result=NULL;
        }
    }
}

int ScSystem()
{
    BPTR fhin=NULL;
    BPTR fhut=NULL;
    BPTR wb_path=NULL;
    LockBuf_release_semaphore(BUF(shared));
    if (arg->argc>1) {
        if (((char *)arg->argv[1])[0]) 
            fhin = Open(arg->argv[1], MODE_NEWFILE);
        if (arg->argc>2 && ((char *)arg->argv[2])[0])
            fhut = Open(arg->argv[2], MODE_NEWFILE);
    }
    if (FromWorkbench)
        wb_path = CloneWorkbenchPath((struct WBStartup *)FromWorkbench);
    tempint=SystemTags((char *)arg->argv[0],
                       wb_path?NP_Path:TAG_IGNORE, wb_path,
                       fhin?SYS_Input:TAG_IGNORE, fhin,
                       fhut?SYS_Output:TAG_IGNORE, fhut,
                       TAG_DONE);
    if (tempint==-1 && wb_path)
        FreeWorkbenchPath(wb_path);
    if (fhin)
        Close(fhin);
    if (fhut)
        Close(fhut);
    UnLockBuf_obtain_semaphore(BUF(shared));
}
