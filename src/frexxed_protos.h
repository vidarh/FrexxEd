/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/*******************************************************
 *
 * frexxed.library functions
 *
 *********/

int __saveds __asm secondmain(register __a0 long *,
                              register __a1 char **);

char __saveds *InitFrexxEd(void);

void __saveds __asm CloseFrexxEd(register __a0 char *);

void __saveds __asm ParseArg(register __a0 char *,
                             register __a1 LONG *);
