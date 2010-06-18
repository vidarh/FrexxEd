/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/***************************************************
 *
 *  GetFont(BufStruct *, char *font, int flag)
 *
 *  Will make (char *) to the system/request font.
 *  Font shall be written like "topaz 8"
 *  Set 'flag' to '1' if you want to change the systemfont, 
 *  '2' for the requestfont and '3' for both.
 *
 *  Return a ret value.
 ******/
int __regargs GetFont(BufStruct *Storage, char *font, int flag);

void __regargs SetSystemFont(void);
