/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/********************************************
 *
 *   DoSearch.h
 *
 *******/

#include "Search.h"
#include "FACT.h"

/********************************************************************
*
* void DoSearch(SearchStruct *);
*
* Hitta rätt sträng, genom innehållet i SearchStructen.
* Returnerar TRUE/FALSE.
******/
BOOL __regargs DoSearch(SearchStruct *search, FACT *fact);

/********************************************************************
*
* void DoSearchBack(SearchStruct *);
*
* Hitta rätt sträng bakåt, genom innehållet i SearchStructen.
* Returnerar TRUE/FALSE.
******/
BOOL __regargs DoSearchBack(SearchStruct *search, FACT *fact);

void __regargs BMS_init_backward(char *buffer, int len, FACT *fact, char *fastmap);
void __regargs BMS_init_forward(char *buffer, int len, FACT *fact, char *fastmap);

char __regargs *FindTextInString(String *text, String *searchstring);

