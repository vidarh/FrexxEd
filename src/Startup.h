/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Editor.h
 *
 *********/

#define cl_QUESTION	(1 << 0)
#define cl_STICK	(1 << 1)
#define cl_COLUMN	(1 << 2)
#define cl_COPYWB	(1 << 3)
#define cl_DOUBLE	(1 << 4)
#define cl_FREXXECUTE	(1 << 5)
#define cl_LINE		(1 << 6)
#define cl_OMIT		(1 << 7)
#define cl_QUIT		(1 << 8)
#define cl_RUN		(1 << 9)
#define cl_SCREEN	(1 << 10)
#define cl_WINDOW	(1 << 11)
#define cl_FILE		(1 << 12)
#define cl_PORTNAME	(1 << 13)
#define cl_BACKDROP	(1 << 14)
#define cl_INIT		(1 << 15)
#define cl_PRIO		(1 << 16)

#ifdef DEBUGTEST
#define TEMPLATE "?/s,File/m,Backdrop/s,CopyWB/s,Directory/k,Execute/k,Iconify/s,Init/k,Omit/s,PortName/k,Prio/n,PubScreen/k,Screen/s,StartupFile/k,Window/s,Diskname/k,Debug/s"
#else
#define TEMPLATE "?/s,File/m,Backdrop/s,CopyWB/s,Directory/k,Execute/k,Iconify/s,Init/k,Omit/s,PortName/k,Prio/n,PubScreen/k,Screen/s,StartupFile/k,Window/s,Diskname/k"
#endif

enum {
  opt_ASK,
  opt_FILE,
  opt_BACKDROP,
  opt_COPYWB,
  opt_DIRECTORY,
  opt_EXECUTE,
  opt_ICONIFY,
  opt_INIT,
  opt_OMIT,
  opt_PORTNAME,
  opt_PRIO,
  opt_PUBSCREEN,
  opt_SCREEN,
  opt_STARTUP,
  opt_WINDOW,
  opt_DISKNAME,
#ifdef DEBUGTEST
  opt_DEBUG,
#endif
  opt_COUNT
};

#ifndef LIB
void ParseArg(char *string, IPTR *opts);
char *InitFrexxEd(void);
#endif
