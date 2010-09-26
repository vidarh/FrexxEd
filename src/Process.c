/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/* Copyright (c) 1993 SAS Institute, Inc, Cary, NC, USA */
/* All Rights Reserved */

#include "Process.h"
#include <clib/alib_protos.h>
#include <exec/memory.h>

void process_starter(void)
{
   struct Process *proc;
   struct ProcMsg *mess;
   long (*fp)(void);
         
   proc = (struct Process *)FindTask((char *)NULL);

   /* get the startup message */
   WaitPort(&proc->pr_MsgPort);
   mess = (struct ProcMsg *)GetMsg(&proc->pr_MsgPort);

   /* gather necessary info from message */
   fp = mess->fp;
#ifdef REG_A4
   putreg(REG_A4, (long)mess->global_data);
#endif   
   
   /* Call the desired function */
   mess->return_code = (*fp)();
   
   /* Forbid so the child can finish completely, before */
   /* the parent cleans up.                             */
   Forbid();

   /* Reply so process who spawed us knows we're done */   
   ReplyMsg((struct Message *)mess);
}

struct ProcMsg *start_process(long (* fp)(void), long priority, long stacksize, char * procname)
{
   struct MsgPort *child_port;
   struct ProcMsg *start_msg;
   struct Process *proc;

   struct TagItem tags[] = {
     {NP_Entry, process_starter},
     {NP_Name, procname},
     {NP_StackSize, stacksize},
     {TAG_END,0}
   };
   start_msg = (struct ProcMsg *)AllocMem(sizeof(struct ProcMsg),MEMF_PUBLIC|MEMF_CLEAR);
   if (start_msg == NULL) {
     FPrintf(Output(),"ERROR: Out of memory. Unable to start process '%s'\n",procname);
     return NULL;
   }
   if((proc = CreateNewProcTagList( tags)) == NULL) {
     FPrintf(Output(),"ERROR: Unable to start process '%s'\n",procname);
     return NULL;
   }
   child_port = &proc->pr_MsgPort;

   /* Create the startup message */
   start_msg->msg.mn_Length = sizeof(struct ProcMsg) - sizeof(struct Message);
   start_msg->msg.mn_ReplyPort = CreatePort(0,0);
   start_msg->msg.mn_Node.ln_Type = NT_MESSAGE;
   
#ifdef REG_A4
   /* save global data reg (A4) */
   start_msg->global_data = (void *)getreg(REG_A4);
#else
   start_msg->global_data = 0;
#endif

   start_msg->seg = 0;
   start_msg->fp = fp;                 /* Fill in function pointer */
   
   /* send startup message to child */
   PutMsg(child_port, (struct Message *)start_msg);
   
   return start_msg;
}

long wait_process(start_msg)
struct ProcMsg *start_msg;
{
    struct ProcMsg *msg;
    long ret;
           
    /* Wait for child to reply, signifying that it is finished */
    while ((msg = (struct ProcMsg *)
	    WaitPort(start_msg->msg.mn_ReplyPort)) != start_msg) 
      ReplyMsg((struct Message *)msg);

    /* get return code */
    ret = msg->return_code;

    /* Free up remaining resources */
    DeletePort(start_msg->msg.mn_ReplyPort);
    FreeMem((void *)start_msg, sizeof(*start_msg));

    return(ret);
}
