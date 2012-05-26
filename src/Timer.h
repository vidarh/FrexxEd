/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */

#ifdef AMIGA
#include <devices/timer.h>
#endif

struct timerequest
{
};

struct FrexxTimerRequest {
  struct timerequest tr;
  struct FrexxTimerRequest *next;
  struct FrexxTimerRequest *prev;
  int flags;
  int seconds;
  int micros;
  char fpl_program[1];
};

typedef struct FrexxTimerRequest FrexxTimerRequest;

#define time_ONCE	0
#define time_REPEAT	1
#define time_ACTIVE	2

int __regargs InitTimer(void);
void __regargs FreeTimer(void);
void __regargs FreeTimeRequest(FrexxTimerRequest *timer);
FrexxTimerRequest __regargs *AddTimerEvent(int flags, char *program, int seconds, int micros);
FrexxTimerRequest __regargs *ReAddTimerEvent(FrexxTimerRequest *timer);

