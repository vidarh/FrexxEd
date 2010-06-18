/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * UpdtScreenC.h
 *
 *********/

struct EndPosStruct {
  short len;
  long blockstart;
  long blocksize;
  char bitplanes;
  char eraseline; // Rensa början och slutet av raden (pga av italic)
};

struct screen_buffer {
  char *beginning; /* dummy */
  char *text;
  char *control;
  short *colors;
  int *linebuffer; /* Size of screen width */
};

int __regargs UpdtOneLineC(BufStruct *Storage, TextStruct *text, struct screen_buffer *buffer, int viewline, int radnummer);
int __regargs UpdtLinesC(BufStruct *, struct screen_buffer *, int, int, int);
void __regargs Updated_Face(SharedStruct *shared, int line);

extern BOOL face_update;

