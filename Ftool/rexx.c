#include "system.h"

/****************************************************************
** This one sends an ARexx string to the ARexx port. The return
** string is allocated with a strdup() call, so the application
** _MUST_ free() this when ready with it.
*****************************/

char *SendARexxMsg(char *arexxport, char *string, struct MsgPort *reply)
{
   struct MsgPort *RexxPort;
   struct RexxMsg *rmsg;
   struct RexxMsg *replymsg;
   char *answer = NULL;

   if (string)
   {
      if (reply && (rmsg = CreateRexxMsg(reply,
					 NULL,
					 NULL)))
      {

	 rmsg->rm_Action = RXCOMM | RXFF_RESULT;

	 if (rmsg->rm_Args[0] = CreateArgstring(string, (LONG) strlen(string)))
	 {
	    /*
	     */
	    Forbid();
	    if (RexxPort = FindPort(arexxport))
	    {
	       /*
	        */
	       PutMsg(RexxPort, (struct Message *)rmsg);
	    }
	    else
	    {
	       /*
	        */
	       DeleteArgstring(rmsg->rm_Args[0]);
	       DeleteRexxMsg(rmsg);
	    }
	    Permit();
	    if (RexxPort)
	    {
	       WaitPort(reply);
	       while (replymsg = (struct RexxMsg *)GetMsg(reply))
	       {
		  if (replymsg)
		  {
		     if (replymsg->rm_Result1 == RC_OK)
		     {
			if (replymsg->rm_Result2)
			{
			   answer = strdup((char *)replymsg->rm_Result2);
			   DeleteArgstring((UBYTE *)replymsg->rm_Result2);
			}
		     }
		     DeleteArgstring(replymsg->rm_Args[0]);
		     DeleteRexxMsg(replymsg);
		  }
	       }
	    }

	 }
	 else
	    DeleteRexxMsg(rmsg);

      }
   }
   return answer;
}
