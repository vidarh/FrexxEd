/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/************************************
*
*  Search.h
*
********/

#ifndef SEARCH_H
#define SEARCH_H

#include "Regex.h"

#define LIMIT_WILDCARD 1
#define WILDCARD 2
#define CASE_SENSITIVE 4
#define ONLY_WORDS 8
#define INSIDE_BLOCK 16
#define FORWARD 32
#define PROMPT_REPLACE 64
#define INSIDE_FOLD 128

typedef struct {
  int wildno;		/* Search word no ?   ( word1|word2|word3 ...) */
  int stars;		/* Length of array */
  int *findarray;	/* Array of (int) (x,y) telling where Search found
			   the parts of the searchword.  Ending with (-1,-1).*/
} SearchFind;

typedef struct {
  char **text;
  int maxlength;
  int strings;		/* antal strängar */
  int maxalloc;		/* antal positioner allocerat */
} HistoryStruct;

typedef struct          /* Structure for Search */
{
  char flags;
  char *buffer;		/* Raw searchsträng */
  int buflen;		/* Längd av raw searchsträng */
  char *realbuffer;	/* FnuttiNize'ad searchsträng */
  char *repbuffer;	/* Raw replacesträng */
  int repbuflen;	/* Längd av raw replcaesträng */
  char *realrepbuffer;	/* FnuttiNize'ad replacesträng */
  BOOL pattern_compiled;
  struct re_pattern_buffer buf;
  char *secondbuffer;
  struct re_pattern_buffer second_buf;
  int lastpos;
  char *lastsearchstring;
} srch;

struct SearchStruct
{
  TextStruct *text;     /* pointer to all pointers */
  int lines;		/* Total lines */
  char *buffer;         /* pointer to SearchString */
  int buflength;	/* length of searchstring */
  char *realbuffer;	/* FnuttiNize'ad searchsträng */
  char *replacebuffer;  /* pointer to ReplaceString */
  int replength;	/* length of replacestring */
  char *realreplacebuffer;/* FnuttiNize'ad replacesträng */
  int begin_x;          /* start of search         */
  int begin_y;
  int end_x;            /* end of search         */
  int end_y;
  int found_x;
  int found_y;
  int last_x;		/* Only set by wildcard search (used for replace) */
  int last_y;		/* Only set by wildcard search (used for replace) */
  int range;
  char flags;
  BOOL pattern_compiled;
  struct re_pattern_buffer buf;
  BOOL find_direct;	/* Kan matcha redan första positionen */
};

typedef struct SearchStruct SearchStruct;

int __regargs SearchAsk(BufStruct *Storage, int replace);

int __regargs SearchFor(BufStruct *Storage, int argc, char **argv);

char __regargs *SearchWild(BufStruct *Storage, SearchStruct *SearchBase, int *returnwanted);

/*****************************************************
 *
 * HistoryAdd(char *string, char remove)
 *
 * Add and Dealloc a string to the SearchHistory.
 * if a copy exist, it will be removed.
 *
 ***********/
void __regargs HistoryAdd(char *string, int remove);

/***************************************************
 *
 *  SetSearch
 *
 *  Om searchstring/replacestring är lika med NULL så sätts inte den strängen om
 *
 *  flags:  "wcofb+", "w+f+", "wc+f-"
 *          Sätter bara om de flaggor som anges.  '+'/'-' efter flaggorna
 *          säger om den ska sättas på/av.
 **********/
void __regargs SetSearch(char *flags, char *searchstring, char *replacestring);

int __regargs MakeSearchFlag(char *flagstring, int currentflags);

int __regargs SearchMatch(BufStruct *Storage, char *search, char *replace, String *ret, int range, char *flags);

void __regargs ConvertSearchFlags(void);

int __regargs SetSearchEnd(BufStruct *Storage, SearchStruct *SearchInfo);

#endif // SEARCH_H
