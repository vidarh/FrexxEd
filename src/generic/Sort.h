/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************************
*
*  ShellSort(char ***, int antal_fält, int antal_sort_falt, int antal_poster, char flags)
*
*  Sorterar enligt Shell-Metznermetoden.
*  Input:  Arraypekare till en stringarray,
*          totala antalet fält (multipliceras med två av sorten),
*          antalet fält som ska ingå i sorteringen (multipliceras med två av sorten),
*          antalet poster.
*          SORT_CASE | SORT_BACK
*  Returnerar ingenting.
*
*  Varannan array i stringarrayen skall peka på längderna för motsvarande
*  sträng.  Om strängen är nollavslutad, sätt pekaren till NULL.
*********/
void ShellSort(char ***StringArrayArray, int no_falt, int sort_no_falt, int no_post, char flags);

#define SORT_NORMAL 0
#define SORT_INCASE 1	// Sortera case sensitive
#define SORT_BACK 2
