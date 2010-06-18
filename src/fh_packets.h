/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 *  fh_packets.h
 *
 *  Replies to all kinds of filehandler-packets.
 *
 *********/

#define DEBUG_PACKETS 0 /* 1 == enable and display packet debugging */

__regargs long ReplyToPacket(struct DosPacket *,
                             struct DeviceList *, long *, long *, long);

struct FrexxLock
{
  struct SharedStruct *Buffer;	/* The buffer address (NULL if root) */
  struct SharedStruct *Next;	/* Used by ExamineNext() */
};

struct FrexxHandle
{
  struct SharedStruct *Buffer;
  long Line;/* current line */
  long Byte;/* current byte in the line */
  long Pos; /* current file-pos (in bytes from the start) */
  long Len; /* total size */
  char write; /* set to TRUE if writing is allowed! */
};

struct FileHandler
{
  BOOL Locked;  /* TRUE when the buffer is exclusively locked */
};

#define NOFILENAME "noname"

#define EXCEPTION_TYPE_READ 1
#define EXCEPTION_TYPE_WRITE 2
#define EXCEPTION_TYPE_OPEN 3
#define EXCEPTION_TYPE_CLOSE 4
#define EXCEPTION_TYPE_CREATE 5
#define EXCEPTION_TYPE_DELETE 6

#if DEBUG_PACKETS>0
#define DEB(x) x
#else
#define DEB(x)
#endif
