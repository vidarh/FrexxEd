/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Reqlist.h
 *
 * Header file for reqlist.c
 *
 *********/

#define GetString(g) ((( struct StringInfo * )g->SpecialInfo )->Buffer  )
#define GetNumber(g) ((( struct StringInfo * )g->SpecialInfo )->LongInt )
#define GetGadgetID(g) ((( struct Gadget * )g->IAddress)->GadgetID )

/***************************************************************************
*
* int Reqlist(int, ...);
*
* Returns the chosen array number or -1 if no one was chosen and -2 if
* anything went wrong.
*
****************************************************************************
*
* ReqList() is tag-controlled. There is a number of tags that can be specified.
* Some which demands a few (one or two) following arguments and some that are
* perfect stand aloners. REQTAG_END must *ALWAYS* be specified last in the
* parameter list!
*
* TAGS:
* =====
*
* REQTAG_BUTTON (For additional buttons)
*   ButtonStruct *
*
* REQTAG_ARRAY (For displaying selection list in requester) 
*   Array pointer
*   Number of array members
*
* REQTAG_STRING1 (For receiving the string entered in the (upper) input field.)
*   Pointer to string buffer
*   Length of buffer
*
* REQTAG_STRING2 (For receiving a second string.)
*   Pointer to string buffer
*   Length of buffer
*
* REQTAG_STRINGNAME1 (Name to be displayed above first string requester)
*   Pointer to a string
*
* REQTAG_STRINGNAME2 (Name to be displayed above second string requester)
*   Pointer to a string
*
* REQTAG_SORT (Sort items in the array when displayed, default is nonsorted.)
*
* REQTAG_WIDTH (Set the minimum width of the requester (if the largest member of
*            the array is longer, this given length is overrided!), default is
*            autosize.)
*   Width in number of characters (in the list).
*
* REQTAG_HEIGHT (Set height of requester)
*   Height in number of characters.
*
* REQTAG_TITLE (Set requester title. Default is "Select an item:".)
*   Title string pointer.
*
* REQTAG_SKIP (Skip a number of arguments, these two always plus the given
*             amount.)
*   Number of arguments to skip!
*
* REQTAG_END (End of arguments)
*
********/

enum tags {
  REQTAG_END, REQTAG_ARRAY, REQTAG_STRING1, REQTAG_STRING2, REQTAG_SORT,
  REQTAG_WIDTH, REQTAG_HEIGHT, REQTAG_TITLE, REQTAG_BUTTON, REQTAG_SKIP,
  REQTAG_STRINGNAME1, REQTAG_STRINGNAME2, REQTAG_SORT_CASE
};

int Reqlist(BufStruct *Storage, int,...);

/* return codes from Reqlist()! */
#define rl_NOMATCH -1 /* DO NOT CHANGE */
#define rl_CANCEL  -2
#define rl_ERROR   -3

/* Testar om 'wanted' med 'width'-bredden passar inom max/min värdena.
   Returnerar värdet du ska använda */

int __regargs TestPosition(int max, int min, int width, int wanted);
