#ifndef __FREXXED_SWAP_H__
#define __FREXXED_SWAP_H__

/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */

/*******************************************
*
*  SWAP4(a,b)
*
*  Will swap the 4byte value in 'a' with 'b'.
*******/
#define SWAP4(a,b) {  register int temp, *c=(int *)&a, *d=(int *)&b;  temp=*c;  *c=*d;  *d=temp; }
#define SWAP1(a,b) {  register char temp, *c=(char *)&a, *d=(char *)&b;  temp=*c;  *c=*d;  *d=temp; }

#define TOLOWER(x) ((((x)>='A')&&((x)<='Z'))?((x)-'A'+'a'):(x))
#define TOUPPER(x) ((((x)>='a')&&((x)<='z'))?((x)-'a'+'A'):(x))

#endif
