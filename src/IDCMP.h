/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
#define GetKey Getkey

/****************************************************************
 *
 * IDCMP.h
 *
 *********/
void __regargs IDCMP(BufStruct *);

/*******************************************
 *
 *  int GetKey(int wait)
 *
 *  Take one char from the keyboard and return it to you.
 *  The CAPS_LOCK is always stripped.
 **********/
int __regargs GetKey(BufStruct *Storage, int flags);

#define gkWAIT	1	/* Wait for the key to be pressed */
#define gkSTRIP	2	/* Strip the shift qualifier from the keystroke */
#define gkNOTEQ	4	/* Godkänn inte samma tangent som trycktes tidigare */
#define gkNOCHACHED 8	/* Godkänn inga cachade tangenter */
#define gkFULLSYNTAX 16	/* Lägg med qualifiers i strängen */
#define gkQUALIFIERS 32	/* Accepterar även att bara få en qualifier */

int __regargs ReSizeWindow(BufStruct *Storage);

typedef struct ARexxCmd {
  char *cmd;
  int func;
};
struct ReturnMsgStruct {
  int retvalue;
  int command;
  int args[2];
  char *caller;		/* Called from menus, etc */
  struct ReturnMsgStruct *next;
  BOOL sent;
  char string[1];
};
typedef struct ReturnMsgStruct ReturnMsgStruct;
#define cmd_EXECUTE 	1	// Execute string
#define cmd_STATUS  	2	// Show string in status line
#define cmd_RET     	3	// Normal ret-value
#define cmd_NEWSTORAGE	4	// Retvalue is a new Storage
#define cmd_KEY		5	// The string points to the message
#define cmd_STRINGCMD	6	// The string points to a string input
#define cmd_AUTOSAVE	7	// AutoSave is trigged.
#define cmd_WINDOWSIZE	8	// New window size.
#define cmd_MENUCLEAR   9	// Menu cleared.
#define cmd_MENUPICK    10	// Menu picked.
#define cmd_DEVICE	11	// Device handling.
#define cmd_IDCMP_MSG	12	// IDCMP message to handle.
#define cmd_ICONDROP	13	// When a file is dropped on window/icon
#define cmd_IMPORTANT	(1<<6)	// Send a CTRL-D to FrexxEd to wake him up! 
#define cmd_EGO		(1<<7)	// Delete all other messages of the same kind 


BOOL __regargs SendReturnMsg(int command, int retvalue, char *string, char *caller, int *args);
ReturnMsgStruct __regargs *GetReturnMsg(int command);

void CopyWindowPos(struct WindowStruct *win);

BOOL __regargs WindowActivated(BufStruct *Storage, struct IntuiMessage *msg);

int __regargs SmartInput(int qualifier);
int __regargs ExamineKeyPress(BufStruct *Storage);
char __regargs *WaitForKey(BufStruct *Storage);

