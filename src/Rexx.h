/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/*
 * Simple ARexx interface...
 *
 * This is a very "Simple" interface...
 */

#ifndef	SIMPLE_REXX_H
#define	SIMPLE_REXX_H

#include	<exec/types.h>
#include	<exec/nodes.h>
#include	<exec/lists.h>
#include	<exec/ports.h>

#include	<rexx/storage.h>
#include	<rexx/rxslib.h>

/*
 * This is the handle that SimpleRexx will give you
 * when you initialize an ARexx port...
 *
 * The conditional below is used to skip this if we have
 * defined it earlier...
 */

/*
 * A structure for the ARexx handler context
 * This is *VERY* *PRIVATE* and should not be touched...
 */
struct  ARexxContext
{
  struct MsgPort *ARexxPort;  /* The port messages come in at... */
  struct MsgPort *ReplyPort;  /* The port reply-messages come in at... */
  long  Outstanding;    /* The count of outstanding ARexx messages to
                           our main port... */
  long  ReplyPortList;	/* The count of outstanding ARexx messages to
                           our ARexx reply port 'ReplyPort' ... */
  char  PortName[24];   /* The port name goes here... */
  char  ErrorName[28];  /* The name of the <base>.LASTERROR... */
  char  Extension[8];   /* Default file name extension... */
  struct RexxMsg *rmsg; /* received rexxmsg or NULL! */
  char *Result;		/* allocated result string or NULL! */
  long ResultCode;	/* error number to return to ARexx */
};

#define AREXXCONTEXT  struct ARexxContext *

/*
 * The value of RexxMsg (from GetARexxMsg) if there was an error returned
 */
#define	REXX_RETURN_ERROR	((struct RexxMsg *)-1L)

/*
 * This function closes down the ARexx context that was opened
 * with InitARexx...
 */
void __regargs FreeARexx(AREXXCONTEXT);

/*
 * This routine initializes an ARexx port for your process
 * This should only be done once per process.  You must call it
 * with a valid application name and you must use the handle it
 * returns in all other calls...
 *
 */
AREXXCONTEXT __regargs InitARexx(char *);

/*
 * This function returns a structure that contains the commands sent from
 * ARexx...  You will need to parse it and return the structure back
 * so that the memory can be freed...
 *
 * This returns NULL if there was no message...
 */
struct RexxMsg __regargs *GetARexxMsg(AREXXCONTEXT);

/*
 * Use this to return a ARexx message...
 *
 * If you wish to return something, it must be in the RString.
 * If you wish to return an Error, it must be in the Error.
 */
void __regargs ReplyARexxMsg(AREXXCONTEXT);

/*
 * This function will send a string to ARexx...
 *
 * The default host port will be that of your task...
 *
 * If you set StringFile to TRUE, it will set that bit for the message...
 *
 * Returns TRUE if it send the message, FALSE if it did not...
 */
short __regargs SendARexxMsg(void *, AREXXCONTEXT, void *, char *, char *, int);

long __regargs SetARexxVar(AREXXCONTEXT, char *, char *);

long __regargs GetARexxVar(AREXXCONTEXT, char *, char **);

#define ARexxName(RexxContext) (RexxContext?RexxContext->PortName:NULL)
#define ARexxSignal(RexxContext) (RexxContext?(ULONG)1L << (RexxContext->ARexxPort->mp_SigBit):0L)

#endif	/* SIMPLE_REXX_H */
