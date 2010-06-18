/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/*****************************************************
 *
 *  MultML.h
 *
 ************/

int __asm MultML(register __d1 int a, register __d2 int b, register __d3 int c);
void __asm BigMult(register __a0 char *result,
                   register __a1 char *fakt1,
                   register __a2 char *fakt2);

