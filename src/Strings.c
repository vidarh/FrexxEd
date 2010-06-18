/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * String.c
 *
 * Holds the function that returns the string with the specified
 * number!
 *
 *********/

#include <exec/libraries.h>
#include <proto/exec.h>
#include <libraries/locale.h>
#include <proto/locale.h>
#include <stdarg.h>
#include <string.h>

#include "Buf.h"
#include "Request.h"
#include "Strings.h"

extern struct Library *LocaleBase;
extern struct Catalog *catalog;
extern char GlobalEmptyString[];

char __regargs *RetString(String_Num string)
{
  static char *AllStrings[]={
    "Unknown command!",
    "Out of memory!",
    "Syntax error!",
    "Nothing left to undo!",
    "No block marked!",
    "Can't delete anything!",
    "Error during open!",
    "The block buffer is empty!",
    "No more matches found!",
    "Function cancelled!",
    "No matching character found!",
    "Wrong type of block!",
    "Couldn't change desired setting",
    "FPL program not found!",
    "File not found!",
    "No block found!",
    "IFF error!",
    "Can't lock buffer!",
    "Can't find buffer!",
    "Wrong file name!",
    "Info already exist!",
    "Unregistered version!",

    "Read protected!",
    "Write protected!",
    "Delete protected!",

    "Workbench is not running!",	// New 941108

    "Failed",				// New 950810
    "",
    "",
    
    "Garbage Collection in process!",
    "Moving block...",
    "Move block:  <-  ->  Yes  Quit.",
    "OK",
    "Cancel",
    "Press the buttons!",
    "Couldn't open screen.\nTry again?",
    "Save setting:",
    "File name:",
    "Yes|No",
    "FrexxEd Request!",
    "Pick a screen mode:",
      "All coding is done by\n" "%s and\n" "%s\n\n"
      "ARexx port: %s\n" "Screen name: %s\n" "Graphic mem: %ld\n" "Other mem: %ld\n"
      "Allocated memory: %ld\n\nDisk name: %s:\nFrexxEd version: %s",
    "Sort according to field:",
    "Make your choice!",
    "Go to line:",
    "Pick file to include:",
    "Load file:",
    "Begin recording!",
    "Enter desired value:",
    "No screen to be found!",
    "Editing a new file!",
    "Loading %s...",
    "Loading %s... Done! (%ld bytes)",
    "", // OBS BORTTAGEN 941118 ......"File %s\nhas been changed after FrexxEd loaded it.\n""Save it anyway?",
    "Crunching buffer!",
    "Saving %s...",
    "XPK-Saving %s...",
    "\n\nSave it normally?",
    "xpk error:",
    "Can't crunch the buffer.\nSave it normally?",
    "Saving Trouble!  File deleted but not saved!",
    "Write error",
    "Wrote %s",
    "Open error",
    "Select font!",
    "Enter number:",
    "Enter string:",
    "\nLine %ld of \"%s\"",
    "FPL halted",
    "<none>",
    "get memory",
    " FrexxEd - Coded by Daniel Stenberg and Kjell Ericson!",
    "", //    "FrexxEd requires at least Kickstart 2.04\n",
    "Error with FPL file '%s'",
    "Can't find the startup file!",
    "Important",
    "No shit!",
    "New Font Installed!",
    "Can't install font!",
    "Couldn't open that font!",
    "That font was too big!",
    "Input key stroke #%ld! (ESC to end)",
    "Key sequence:",
    "Function:",
    "Key assign!",
    "Error",
    "Function assigned to key!",
    "Macro name:",
    "Macro assign!",
    "No key stroke found!",
    " Waiting for another keypress!",
    "No function assigned to that key!",
    "Illegal command.",
    "Yes, No, Last, Global, All, Quit.",
    "Global replace working...",
    "Break replace with space!",
    "init FPL!",
    "FrexxEd couldn't %s!",
    "FrexxEd can't continue before you\nhave closed all windows.",
    "Try again!|Ignore", //960608
    "get Public screen",
    "create write port",
    "create write request",
    "create read port",
    "create message port",
    "open screen",
    "get VisualInfo",
    "open requested window",
    "create menus",
    "open console device",
    "Search flags!",
    "Search/Replace:",
    "Search for:",
    "Line",
    "Col",
    "Select an item:",
    
    	"Project",
    "New",
    "Load",
    "Save",
    "About",
    "Kill",
    "Quit all",
    	"Block",
    "Mark",
    "Cut",
    "Copy",
    "Insert",
    	"Edit",
    "Undo",
    "",//"Undo restart",
    "",//	"Move/view",
    "",//"Goto buffer",
    "",//"Goto line",
    	"Search/Replace",
    "",//"Repeat search forward",
    "Search...",
    "",//"Repeat replace",
    "Replace...",
    	"Customizing",
    "Settings",
     "Locals",
     "All locals",
     "Globals",
     "Save",
    "ScreenMode",
    	"Special",
    "Execute FPL file",
    "Load new menus",
    "Prompt",
    
    "Replace",
    "Split",
    "Full",
    
    "None",
    "Right",
    "Left",
    
    "Screen",
    "Window",
    "Backdrop",
    
    "Visible",
    "Absolute",
    
    "Off",
    "Marking",
    "Exist",
    
    "Relative",
    "All",
    
    "@Wildcards",
    "@Case sensitive",
    "@Only words",
    "@Forward search",
    "Inside @Block",
    "@Prompt Replace",
    "@Limit Wildcard",
    
    "Password",
    "Enter password:",
    
    "This version is not registered!\n\n"\
    "Pay your register fee in order\n"\
    "to use all FrexxEd functions!",
    
    "Choose entry:",
    
    "End of file!",
    
    "Registered to",
    
    "Never",
    "Always",
    "Parent",

    "Maximize view",
    "@Forward",
    "@Case sensitive (ASCII)",

    "Save noname file:",
    "@Inside folds",
    "FrexxEd can't quit! %ld locks are\nstill held by the filehandler!",

    "WinScreen", // 950802

    "NewWindow", // 960206 (för popup_view)

    "" /* no real string */

  };
  register char *retstring=AllStrings[string];
  if(LocaleBase)
     retstring=GetCatalogStr(catalog, string, AllStrings[string]);

  if (string==STR_UNREG_VERSION)
    Ok_Cancel(NULL, RetString(STR_UNREG_VERSION_LONG), retstring, RetString(STR_OK_GADGET));

  /* no locale.library: */
  return(retstring);
}

int __regargs Sprintf(char *buffer, char *format, ...)
{
  va_list args;
  va_start(args, format);
  RawDoFmt(format, args, (void (*))"\x16\xc0\x4e\x75", buffer);
  va_end(args);
  return (int)strlen(buffer);
}

char __regargs *GetRetMsg(int num)
{
  char *string=GlobalEmptyString;
  if (num<0) {
    if (num>LAST_ERROR)
      string=RetString(-num-1);
    else
      string=(char *)-num;
  } else {
    if (num>LAST_CODE)
      string=(char *)num;
  }
  return(string);
}
