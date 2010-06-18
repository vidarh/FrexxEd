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
* Letar reda på rätt korresponderande tecken till det man står på. Står man
* inte på ett valid tecken letar den framåt.
* Retunerar FALSE om den inte fick napp!
*
******/
char __regargs MatchChar(BufStruct *Storage);

