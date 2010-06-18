/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************************
*
*  ShellSort(char ***, int antal_f�lt, int antal_sort_falt, int antal_poster, char flags)
*
*  Sorterar enligt Shell-Metznermetoden.
*  Input:  Arraypekare till en stringarray,
*          totala antalet f�lt (multipliceras med tv� av sorten),
*          antalet f�lt som ska ing� i sorteringen (multipliceras med tv� av sorten),
*          antalet poster.
*          SORT_CASE | SORT_BACK
*  Returnerar ingenting.
*
*  Varannan array i stringarrayen skall peka p� l�ngderna f�r motsvarande
*  str�ng.  Om str�ngen �r nollavslutad, s�tt pekaren till NULL.
*********/
void ShellSort(char ***StringArrayArray, int no_falt, int sort_no_falt, int no_post, char flags);

#define SORT_NORMAL 0
#define SORT_INCASE 1	// Sortera case sensitive
#define SORT_BACK 2
