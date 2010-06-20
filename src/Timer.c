/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */

#include <exec/execbase.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <devices/timer.h>
#include <libraries/dos.h>
#include <proto/exec.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <clib/alib_protos.h>

#include "Buf.h"
#include "Alloc.h"
#include "Setting.h"
#include "Timer.h"

extern struct FrexxTimerRequest *TimerIO;
extern struct MsgPort *TimerMP;


int __regargs InitTimer()
{
  int ret=FUNCTION_CANCEL;

  if (TimerMP=CreatePort(NULL, NULL)) {
    if (TimerIO=((FrexxTimerRequest *)CreateExtIO(TimerMP, sizeof(FrexxTimerRequest)))) {
      if (!OpenDevice("timer.device", UNIT_MICROHZ, (struct IORequest *)TimerIO, NULL))
        ret=OK;
    }
  }
  return(ret);
}

void __regargs FreeTimer()
{
  if (TimerMP) {
    if (TimerIO) {
      if (TimerIO->tr.tr_node.io_Device) {
        FreeTimeRequest((FrexxTimerRequest *)-1);
        CloseDevice((struct IORequest *)TimerIO);
      }
      DeleteExtIO((struct IORequest *)TimerIO);
    }
    DeletePort(TimerMP);
  }
}

void __regargs FreeTimeRequest(FrexxTimerRequest *timer)
{
  FrexxTimerRequest *count=TimerIO->next;
  do {
    if (timer!=(FrexxTimerRequest *)-1) {
      while (count && count!=timer)
        count=count->next;
    }
    if (count) {
      if (!CheckIO((struct IORequest *)count)) {
        AbortIO((struct IORequest *)count);
      }
      WaitIO((struct IORequest *)count);
      count->prev->next=count->next;
      if (count->next)
        count->next->prev=count->prev;
      if (count->flags&time_ACTIVE)
        count->flags=time_ACTIVE;
      else
        Dealloc(count);
    }
    count=TimerIO->next;
  } while (count && timer==(FrexxTimerRequest *)-1);
}


FrexxTimerRequest __regargs *AddTimerEvent(int flags, char *program, int seconds, int micros)
{
  FrexxTimerRequest *timer;

  timer=(FrexxTimerRequest *)Malloc(sizeof(FrexxTimerRequest)+strlen(program));
  if (timer) {
    timer->next=TimerIO->next;
    timer->prev=TimerIO;
    TimerIO->next=timer;
    timer->tr.tr_node=TimerIO->tr.tr_node;
    timer->tr.tr_node.io_Command=TR_ADDREQUEST;
    timer->tr.tr_time.tv_secs=seconds;
    if ((unsigned int)micros>999999)
      micros=0;
    timer->tr.tr_time.tv_micro=micros;
    strcpy(timer->fpl_program, program);
    timer->flags=flags;
    timer->seconds=seconds;
    timer->micros=micros;
    SendIO((struct IORequest *)timer);
  }
  return(timer);
}

FrexxTimerRequest __regargs *ReAddTimerEvent(FrexxTimerRequest *timer)
{
  timer->tr.tr_time.tv_secs=timer->seconds;
  timer->tr.tr_time.tv_micro=timer->micros;
  timer->flags&=time_REPEAT;
  SendIO((struct IORequest *)timer);
  return(timer);
}
