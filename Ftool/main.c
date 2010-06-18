#include "system.h"

#include "/GadLib/FrexxLayout.h"
#include "/GadLib/FrexxLayoutProtos.h"

char *version = "$VER: FrexxTool 0.1 (13.8.94)";

#define DEFAULT_COLUMNS 3	/* Max number of buttons per row */

/****************************************************************
** For argument parsing
************************************/
struct
{
   char *port;
   char *setup;
   LONG *columns;
   BOOL help;
} Args = {NULL, NULL, NULL, FALSE};

struct RDArgs *rdargs = NULL;

/****************************************************************
** Handle for FrexxLayout.lib functions
************************************/
struct FrexxLayoutInfo *GadInfo;

/****************************************************************
** Tags for window opening
************************************/
struct TagItem WindowTags[] =
{
	WA_Title, (ULONG)"FrexxTool",
	WA_Left, 64,
	WA_Top, 32,
	WA_DragBar, TRUE,
	WA_Activate, TRUE,
	WA_CloseGadget, TRUE,
	WA_SizeGadget, FALSE,
	WA_DepthGadget, TRUE,
	WA_IDCMP, IDCMP_CLOSEWINDOW | LISTVIEWIDCMP,
	TAG_END
};

extern struct LayoutGadgets Gads[];	/* Setup struct for CreateGads() */
extern char *Commands[];	/* Contains all our ARexx commands */

/* This function is in rexx.c */
char *SendARexxMsg(char *arexxport, char *string, struct MsgPort *reply);

/* This function is in setup.c */
int GetSetup(char *filename, int columns);

int main(int argc, char **argv)
{
   struct Screen *screen;	/* Pointer to our public screen */
   struct Window *window;	/* Pointer to our window */
   struct IntuiMessage *msg;	/* Holds messages from intuition */
   int end = FALSE;		/* TRUE when we want to exit */
   char *PubScreenName;		/* The name of our public screen */
   char *answer = NULL;		/* Answer string from ARexx. Must be freed

				   with free() after each ARexx call */
   BOOL port;			/* FALSE if FindPort() failed */
   struct MsgPort *reply;	/* ARexx reply port */
   UWORD GadgetID;		/* Current pressed gadget */
   extern struct ExecBase *SysBase;	/* Exec base structure pointer */

   /* Check if we have a too old system version */
   if (SysBase->LibNode.lib_Version < 37)
   {
      printf("You need at least 2.04!\n");
      return (0);
   }

   if (!(rdargs = ReadArgs("P=PORT/K,S=SETUP/K,C=COLUMNS/N/K,H=HELP/S/K", (LONG *) & Args, NULL)))
   {
      printf("Missing argument\n");
      exit(RETURN_FAIL);
   }

   if (Args.help)
   {
      printf("The Frexx add-on toolbox!\n");
      return (0);
   }

   /* Read the setup file and construct all buttons and their commands */
   if (GetSetup(Args.setup, Args.columns ? *Args.columns : DEFAULT_COLUMNS))
   {
      exit(1);
   }

   /* If the PORT parameter was not specified, then FREXXED.1 is default */
   if (!Args.port)
   {
      Args.port = "FREXXED.1";
   }

   /* Try to find the ARexx port */
   Forbid();
   port = FindPort(Args.port) ? TRUE : FALSE;
   Permit();

   if (!port)
   {
      printf("Could not find ARexx port: %s\n", Args.port);
      exit(1);
   }

   reply = CreateMsgPort();	/* ARexx reply port */
   if (reply)
   {
      /* Start with a call to get the screen name */
      PubScreenName = SendARexxMsg(Args.port,
			    "ARexxResult(0, ReadInfo(\"current_screen\"));",
				   reply);

      if (screen = LockPubScreen(PubScreenName))
      {
	 if (GadInfo = CreateGads(Gads, screen, TAG_END))
	 {
	    if (!GadInfo->gi_Error)	/* Check for layout error */
	    {
	       if (window = OpenWindowTags(NULL,
					   WA_Gadgets, GadInfo->gi_GList,
				    WA_InnerWidth, GadInfo->gi_ExtremeRight,
			      WA_InnerHeight, GadInfo->gi_ExtremeBottom + 1,
					   TAG_MORE, WindowTags))
	       {
		  GT_RefreshWindow(window, NULL);

		  while (!end)
		  {
		     WaitPort(window->UserPort);
		     while (msg = GT_GetIMsg(window->UserPort))
		     {
			switch (msg->Class)
			{
			   case IDCMP_CLOSEWINDOW:
			      end = TRUE;
			      break;
			   case IDCMP_GADGETUP:
			      GadgetID = ((struct Gadget *)msg->IAddress)->GadgetID;
			      answer = SendARexxMsg(Args.port,
						    Commands[GadgetID],
						    reply);
			      if (answer)
			      {
				 free(answer);	/* Important!!! */
			      }
			      break;
			}
			GT_ReplyIMsg(msg);
		     }
		  }
		  CloseWindow(window);
	       }
	       else
	       {
		  printf("Could not resolve gadgets!!!\n");
	       }
	    }
	    DeleteGads(GadInfo);
	 }
	 UnlockPubScreen(0, screen);
      }
      DeleteMsgPort(reply);
   }
   return (0);
}
