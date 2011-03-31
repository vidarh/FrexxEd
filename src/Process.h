/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
#define _USEOLDEXEC_ 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef AMIGA
#include <exec/memory.h>
#include <libraries/dosextens.h>
#include <exec/execbase.h>
#include <proto/exec.h>
#include <proto/dos.h>

extern struct ExecBase *SysBase;
#endif

struct FAKE_SegList {
   long space;
   long length;
   BPTR nextseg;
	short jmp;
	void (*func)();
};

struct ProcMsg {                        /* startup message sent to child */
        struct Message msg;
        long (*fp)();                    /* function we're going to call */
        void *global_data;              /* global data reg (A4)         */
        long return_code;               /* return code from process     */
        struct FAKE_SegList *seg;       /* pointer to fake seglist so   */
                                        /* can it can be free'd         */
                                        /* user info can go here */
        };
extern long curdir;


/* Prototypes */
void process_starter(void);
struct ProcMsg *start_process(long (*)(void), long, long, char *);
long wait_process(struct ProcMsg *);
