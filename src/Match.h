/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************
*
* Match.h
*
********/

/********************************************************************
*
* char MatchChar(BufStruct *);
*
* Letar reda p� r�tt korresponderande tecken till det man st�r p�. St�r man
* inte p� ett valid tecken letar den fram�t.
* Retunerar FALSE om den inte fick napp!
*
******/
char __regargs MatchChar(BufStruct *Storage);

