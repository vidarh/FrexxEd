/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */

#include <dos/dos.h>
#include <exec/execbase.h>
#include <exec/types.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Buf.h"
#include "Alloc.h"
#include "Command.h"
#include "Limit.h"
#include "MultML.h"
#include "RSADecrypt.h"

#define ANTAL_BYTES 65
#define ANTAL_BITAR (ANTAL_BYTES*8)

/* OBS Low byte first */


extern struct ExecBase *SysBase;
extern DefaultStruct Default;

void __regargs Modulo(unsigned char *value, unsigned char modulo[8][ANTAL_BYTES]);
void __regargs Calc(char *c, char *m, char n[8][ANTAL_BYTES]);
void __regargs RepeatN(char result[8][ANTAL_BYTES]);

void __regargs RSADecrypt(String *key)
{
  char P[ANTAL_BYTES];
  char D[ANTAL_BYTES];
  char ntab[8][ANTAL_BYTES];

  if (key->length) {
  
    {		// Snabb kryptering
      register int count=48;
      register WORD *buffer=((WORD *)key->string)-1;
      do {
        buffer++;
        *(buffer)-=buffer[1];
      } while (--count>0);
    }
		// Calculate a checksum.
    {
      register int count=key->length/2-1;
      register WORD sum=0;
      register WORD *buffer=(WORD *)key->string;
      while (count>=68/2) {
        sum+=buffer[count];
        count--;
      }
      if (sum!=buffer[32])
        return;
      buffer[32]=0;
    }
    memcpy(D, key->string+66, ANTAL_BYTES);
    D[ANTAL_BYTES-1]=0;
    memcpy(ntab[0], key->string, ANTAL_BYTES);
    ntab[0][ANTAL_BYTES-1]=0;
  
    RepeatN(ntab);
  
    Calc(P, D, ntab);
    memcpy(key->string+66, P, 64);
    {
      register int size=(key->length-66-64)/4;
      register int *bufstart=(int *)(key->string+66);
      register int *dest=bufstart+16;
      do {
        *dest-=bufstart[size&15]<<1;
        dest++;
        size--;
      } while (size>=0);
    }

    Default.BufStructDefault.reg.reg-=(*(int *)(key->string+66))-CRYPT_REVISION;
  }
}

/**********************************************
 *
 * Calc räknar ut en beräkning enligt formeln:
 *
 * svar= i1^i2 mod i3.
 *
 *
 *
 *  Input:  resultstore, Value, Exponent, Modulo
 ************/
void __regargs Calc(char *c, char *m, char n[8][ANTAL_BYTES])
{
  char string[ANTAL_BYTES*2];

  BigMult(string, m, m);	// Step 4
  Modulo(string, n);
  memcpy(c, string, ANTAL_BYTES);
  BigMult(string, c, m);
  Modulo(string, n);
  memcpy(c, string, ANTAL_BYTES);
}

void __regargs Modulo(unsigned char *value, unsigned char modulo[8][ANTAL_BYTES])
{
  int byte;
  int byteoffset;
  int bytecount;
  int bit;

  for (bytecount=ANTAL_BITAR-1; bytecount>=0; bytecount--) {
    byteoffset=bytecount/8;
    bit=bytecount&7;
    byte=ANTAL_BYTES-1;
    while (byte>=0 && value[byte+byteoffset]==modulo[bit][byte])
      byte--;
    if (byte<0 || value[byte+byteoffset]>modulo[bit][byte]) {
      register int temp;
      register WORD v=0;
      for (temp=0; temp<ANTAL_BYTES; temp++) {
        v=(unsigned short)value[temp+byteoffset]-(unsigned short)modulo[bit][temp]-v;
        value[temp+byteoffset]=v;
        v=(v<0)?1:0;
      }
    }
  }
}

void __regargs RepeatN(char ntab[8][ANTAL_BYTES])
{
  char temp[ANTAL_BYTES];
  char string[ANTAL_BYTES*2];
  register int no;

  memset(temp, 0, ANTAL_BYTES);
  temp[0]=2;

  for (no=1; no<8; no++) {
    BigMult(string, temp, &ntab[no-1][0]);
    memcpy(&ntab[no][0], string, ANTAL_BYTES);
  }
}

