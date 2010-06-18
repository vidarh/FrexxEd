/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/* Copyright (c) 1993 SAS Institute, Inc, Cary, NC, USA */
/* All Rights Reserved */

#include "process.h"

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
   putreg(REG_A4, (long)mess->global_data);
   
   
   /* Call the desired function */
   mess->return_code = (*fp)();
   
   /* Forbid so the child can finish completely, before */
   /* the parent cleans up.                             */
   Forbid();

   /* Reply so process who spawed us knows we're done */   
   ReplyMsg((struct Message *)mess);
}


struct ProcMsg *start_process(fp, priority, stacksize, procname)
long (*fp)(void);
long priority,stacksize;
char *procname;
{
   struct MsgPort *child_port;
   struct ProcMsg *start_msg;
   struct FAKE_SegList *seg_ptr;
   
   /* Allocate memory for both fake seglist and startup message */
   /* If either fail we can return, before the CreateProc()     */ 
   seg_ptr = (struct FAKE_SegList *)AllocMem(sizeof (*seg_ptr), MEMF_PUBLIC);
   if (seg_ptr == NULL) return NULL;
   start_msg = (struct ProcMsg *)AllocMem(sizeof(struct ProcMsg), 
                                          MEMF_PUBLIC|MEMF_CLEAR);
   if (start_msg == NULL)
   {
      FreeMem(seg_ptr, sizeof (*seg_ptr));
      return NULL;
   }
   
   
   /* Fill in Fake SegList */
   seg_ptr->space = 0;
   seg_ptr->length = (sizeof(*seg_ptr) + 3) & ~3;
   seg_ptr->nextseg = NULL;
   
   /* Fill in JMP to function */
   seg_ptr->jmp = 0x4EF9;  /* JMP instruction */
   seg_ptr->func = process_starter;
   
   /* If we're running under 2.0 it's possible to be on a 68040 */
   /* Therefore the cache needs to be flushed before the child  */
   /* can start executing.                                      */
   if (SysBase->LibNode.lib_Version >= 36)
      CacheClearU();
      
   /* create the child process */   
   if((child_port = CreateProc(procname,
                               priority,
                               (BPTR)((long)&seg_ptr->nextseg>>2),
                               stacksize)) == NULL)
   {
      /* error, cleanup and abort */
      FreeMem(seg_ptr, sizeof(*seg_ptr));
      FreeMem(start_msg, sizeof(*start_msg));
      return NULL;
   }
   
   /* Create the startup message */
   start_msg->msg.mn_Length = sizeof(struct ProcMsg) - sizeof(struct Message);
   start_msg->msg.mn_ReplyPort = CreatePort(0,0);
   start_msg->msg.mn_Node.ln_Type = NT_MESSAGE;
   
   /* save global data reg (A4) */
   start_msg->global_data = (void *)getreg(REG_A4);

   start_msg->seg = seg_ptr;
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
    FreeMem((void *)start_msg->seg, sizeof(struct FAKE_SegList));
    FreeMem((void *)start_msg, sizeof(*start_msg));

    return(ret);
}
