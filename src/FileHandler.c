/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 *  FileHandler.c
 *
 *  Takes care of the file handler I/O
 *
 *********/

#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>
#include <exec/semaphores.h>
#include "mount.h"
#include <stdio.h>

#include "Buf.h"
#include "process.h"
#include "FH_packets.h"
#include "FileHandler.h"

#include "Alloc.h" /* for the Strdup() */

extern struct SignalSemaphore LockSemaphore;

extern struct ProcMsg *filehandlerproc;
extern struct MsgPort *filehandlerport;
extern struct Task *filehandlertask;
extern struct DeviceList *device;
extern char filehandlerstop; /* set TRUE when CTRL_C received! */

extern long fh_locks;
extern long fh_opens;
extern char buffer[];	/* temp buffer */
extern BOOL device_want_control; /* we want control! */

extern DefaultStruct Default; /* the default values of a BufStruct */

void ReportPacket(struct DosPacket *p, long out);

long FileHandler(void)
{
   struct MsgPort *replyport;
   struct Message *msg;
   struct DosPacket *pkt;
   long out=0;
   long res1, res2;
   int count;
   
   DEB(out = Open("CON:510/0/520/80/Filehandler/CLOSE", MODE_OLDFILE));

   if(filehandlerport = CreatePort(0L, 0L))
   {
     if (Default.diskname &&
         (device = Mount(Default.diskname, filehandlerport)));
     else {
       for (count=0; count<31; count++) {
         Sprintf(buffer, FILE_HANDLER_NAME "%ld", count);
         device = Mount(buffer, filehandlerport);
         if (device) {
           Dealloc(Default.diskname);
           Default.diskname=Strdup(buffer);
           break;
         }
       }
     }
     if(device)
     {
       filehandlertask = FindTask(NULL);

       DEB(FPrintf(out, "Waiting for packets. Send CTRL-C to exit.\n"));
       while(!filehandlerstop || fh_locks || fh_opens)
       {
          if(Wait( (1<<filehandlerport->mp_SigBit) | SIGBREAKF_CTRL_C) &
             SIGBREAKF_CTRL_C) {
             filehandlerstop = TRUE;
             DEB(FPrintf(out, "%ld locks left, %ld opens.\n", fh_locks, fh_opens));
             continue;
          }
          device_want_control = 1; /* please, gimme my semaphore! */
          ObtainSemaphore(&LockSemaphore);
          device_want_control = 0; /* thanks, you can have it back now! */
          while(msg = GetMsg(filehandlerport))
          {
             pkt = (struct DosPacket *)msg->mn_Node.ln_Name;
             DEB(ReportPacket(pkt, out));

             ReplyToPacket(pkt, device, &res1, &res2, out);

             /* Flush the packet back to its origin */
             pkt->dp_Res1 = res1;
             pkt->dp_Res2 = res2;
             replyport = pkt->dp_Port;
             pkt->dp_Port = filehandlerport;
             PutMsg(replyport, pkt->dp_Link);
          }
          ReleaseSemaphore(&LockSemaphore);
       }
       filehandlerstop= FALSE; /* to enable restarting! ;) */
       while(msg = GetMsg(filehandlerport)) {
         /* flush the rest of the messages! */
         pkt = (struct DosPacket *)msg->mn_Node.ln_Name;
         pkt->dp_Res1 = DOSFALSE;
         pkt->dp_Res2 = ERROR_ACTION_NOT_KNOWN;
         replyport = pkt->dp_Port;
         pkt->dp_Port = filehandlerport;
         PutMsg(replyport, pkt->dp_Link);
       }
       DisMount(device);
     }
     else
        DEB(FPrintf(out, "Can't mount volume \"%s\"\n", Default.ARexxPort));
     DeletePort(filehandlerport);
   }

   DEB(Close(out)); /* Close the debug window */

   filehandlertask = NULL;   
   return 1; /* dead! */
}

#if DEBUG_PACKETS>0
/***************************************************************************
 ** ReportPacket()
 **
 ** Outputs the received packet type to the serial port with kprintf().
 **
 ********/
void ReportPacket(struct DosPacket *p, long out)
{
   switch (p->dp_Type)
   {
      case ACTION_STARTUP:
	 FPrintf(out, "Got packet ACTION_STARTUP.\n");
	 break;
      case ACTION_GET_BLOCK:
	 FPrintf(out, "Got packet ACTION_GET_BLOCK.\n");
	 break;
      case ACTION_SET_MAP:
	 FPrintf(out, "Got packet ACTION_SET_MAP.\n");
	 break;
      case ACTION_DIE:
	 FPrintf(out, "Got packet ACTION_DIE.\n");
	 break;
      case ACTION_EVENT:
	 FPrintf(out, "Got packet ACTION_EVENT.\n");
	 break;
      case ACTION_CURRENT_VOLUME:
	 FPrintf(out, "Got packet ACTION_CURRENT_VOLUME.\n");
	 break;
      case ACTION_LOCATE_OBJECT:
	 FPrintf(out, "Got packet ACTION_LOCATE_OBJECT.\n");
	 break;
      case ACTION_RENAME_DISK:
	 FPrintf(out, "Got packet ACTION_RENAME_DISK.\n");
	 break;
      case ACTION_WRITE:
	 FPrintf(out, "Got packet ACTION_WRITE.\n");
	 break;
      case ACTION_READ:
	 FPrintf(out, "Got packet ACTION_READ.\n");
	 break;
      case ACTION_FREE_LOCK:
	 FPrintf(out, "Got packet ACTION_FREE_LOCK.\n");
	 break;
      case ACTION_DELETE_OBJECT:
	 FPrintf(out, "Got packet ACTION_DELETE_OBJECT.\n");
	 break;
      case ACTION_RENAME_OBJECT:
	 FPrintf(out, "Got packet ACTION_RENAME_OBJECT.\n");
	 break;
      case ACTION_MORE_CACHE:
	 FPrintf(out, "Got packet ACTION_MORE_CACHE.\n");
	 break;
      case ACTION_COPY_DIR:
	 FPrintf(out, "Got packet ACTION_COPY_DIR.\n");
	 break;
      case ACTION_WAIT_CHAR:
	 FPrintf(out, "Got packet ACTION_WAIT_CHAR.\n");
	 break;
      case ACTION_SET_PROTECT:
	 FPrintf(out, "Got packet ACTION_SET_PROTECT.\n");
	 break;
      case ACTION_CREATE_DIR:
	 FPrintf(out, "Got packet ACTION_CREATE_DIR.\n");
	 break;
      case ACTION_EXAMINE_OBJECT:
	 FPrintf(out, "Got packet ACTION_EXAMINE_OBJECT.\n");
	 break;
      case ACTION_EXAMINE_NEXT:
	 FPrintf(out, "Got packet ACTION_EXAMINE_NEXT.\n");
	 break;
      case ACTION_DISK_INFO:
	 FPrintf(out, "Got packet ACTION_DISK_INFO.\n");
	 break;
      case ACTION_INFO:
	 FPrintf(out, "Got packet ACTION_INFO.\n");
	 break;
      case ACTION_FLUSH:
	 FPrintf(out, "Got packet ACTION_FLUSH.\n");
	 break;
      case ACTION_SET_COMMENT:
	 FPrintf(out, "Got packet ACTION_SET_COMMENT.\n");
	 break;
      case ACTION_PARENT:
	 FPrintf(out, "Got packet ACTION_PARENT.\n");
	 break;
      case ACTION_TIMER:
	 FPrintf(out, "Got packet ACTION_TIMER.\n");
	 break;
      case ACTION_INHIBIT:
	 FPrintf(out, "Got packet ACTION_INHIBIT.\n");
	 break;
      case ACTION_DISK_TYPE:
	 FPrintf(out, "Got packet ACTION_DISK_TYPE.\n");
	 break;
      case ACTION_DISK_CHANGE:
	 FPrintf(out, "Got packet ACTION_DISK_CHANGE.\n");
	 break;
      case ACTION_SET_DATE:
	 FPrintf(out, "Got packet ACTION_SET_DATE.\n");
	 break;
      case ACTION_SCREEN_MODE:
	 FPrintf(out, "Got packet ACTION_SCREEN_MODE.\n");
	 break;
      case ACTION_READ_RETURN:
	 FPrintf(out, "Got packet ACTION_READ_RETURN.\n");
	 break;
      case ACTION_WRITE_RETURN:
	 FPrintf(out, "Got packet ACTION_WRITE_RETURN.\n");
	 break;
      case ACTION_SEEK:
	 FPrintf(out, "Got packet ACTION_SEEK.\n");
	 break;
      case ACTION_FINDUPDATE:
	 FPrintf(out, "Got packet ACTION_FINDUPDATE.\n");
	 break;
      case ACTION_FINDINPUT:
	 FPrintf(out, "Got packet ACTION_FINDINPUT.\n");
	 break;
      case ACTION_FINDOUTPUT:
	 FPrintf(out, "Got packet ACTION_FINDOUTPUT.\n");
	 break;
      case ACTION_END:
	 FPrintf(out, "Got packet ACTION_END.\n");
	 break;
      case ACTION_SET_FILE_SIZE:
	 FPrintf(out, "Got packet ACTION_SET_FILE_SIZE.\n");
	 break;
      case ACTION_WRITE_PROTECT:
	 FPrintf(out, "Got packet ACTION_WRITE_PROTECT.\n");
	 break;
      case ACTION_SAME_LOCK:
	 FPrintf(out, "Got packet ACTION_SAME_LOCK.\n");
	 break;
      case ACTION_CHANGE_SIGNAL:
	 FPrintf(out, "Got packet ACTION_CHANGE_SIGNAL.\n");
	 break;
      case ACTION_FORMAT:
	 FPrintf(out, "Got packet ACTION_FORMAT.\n");
	 break;
      case ACTION_MAKE_LINK:
	 FPrintf(out, "Got packet ACTION_MAKE_LINK.\n");
	 break;
      case ACTION_READ_LINK:
	 FPrintf(out, "Got packet ACTION_READ_LINK.\n");
	 break;
      case ACTION_FH_FROM_LOCK:
	 FPrintf(out, "Got packet ACTION_FH_FROM_LOCK.\n");
	 break;
      case ACTION_IS_FILESYSTEM:
	 FPrintf(out, "Got packet ACTION_IS_FILESYSTEM.\n");
	 break;
      case ACTION_CHANGE_MODE:
	 FPrintf(out, "Got packet ACTION_CHANGE_MODE.\n");
	 break;
      case ACTION_COPY_DIR_FH:
	 FPrintf(out, "Got packet ACTION_COPY_DIR_FH.\n");
	 break;
      case ACTION_PARENT_FH:
	 FPrintf(out, "Got packet ACTION_PARENT_FH.\n");
	 break;
      case ACTION_EXAMINE_ALL:
	 FPrintf(out, "Got packet ACTION_EXAMINE_ALL.\n");
	 break;
      case ACTION_EXAMINE_FH:
	 FPrintf(out, "Got packet ACTION_EXAMINE_FH.\n");
	 break;
      case ACTION_LOCK_RECORD:
	 FPrintf(out, "Got packet ACTION_LOCK_RECORD.\n");
	 break;
      case ACTION_FREE_RECORD:
	 FPrintf(out, "Got packet ACTION_FREE_RECORD.\n");
	 break;
      case ACTION_ADD_NOTIFY:
	 FPrintf(out, "Got packet ACTION_ADD_NOTIFY.\n");
	 break;
      case ACTION_REMOVE_NOTIFY:
	 FPrintf(out, "Got packet ACTION_REMOVE_NOTIFY.\n");
	 break;
      case ACTION_SERIALIZE_DISK:
	 FPrintf(out, "Got packet ACTION_SERIALIZE_DISK.\n");
	 break;
      default:
	 FPrintf(out, "Got unknown packet: %ld\n", p->dp_Type);
	 break;
   }
}
#endif
