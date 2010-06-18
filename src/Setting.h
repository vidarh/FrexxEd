/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/******************************************************************************
 *
 * Setting.h
 *
 * All fula definear som hör till settingsarna.
 *
 *******/

#ifndef SETTING_H
#define SETTING_H

#define ST_WORD		1
#define ST_BOOL		2
#define ST_STRING	3
#define ST_CYCLE	4
#define ST_NUM		5
#define ST_ARRAY	6

#define ST_GLOBAL       (1 << 4)
#define ST_SHARED       (1 << 5)
#define ST_SCREEN       (1 << 6)
#define ST_NOSAVE       (1 << 7)
#define ST_READ         (1 << 8)
#define ST_CALCULATE    (1 << 9)
#define ST_USERDEFINED  (1 << 10)
#define ST_HIDDEN       (1 << 11)
#define ST_WINDOW       (1 << 12)
#define ST_ALL_WINSCREEN (1 << 13)

#define SE_NOTHING	(0)
#define SE_PROTECTION	(1 << 1)
#define SE_UPDATE	(1 << 3)
#define SE_UPDATEALL	(1 << 4)
#define SE_REINIT	(1 << 5)
#define SE_REOPEN	(1 << 6)
#define SE_RETHINK	(1 << 7)
#define SE_FONTCHANGED	(1 << 8)
#define SE_NEWPOS	(1 << 9)
#define SE_NEWSIZE	(1 << 10)
#define SE_ALL		(1 << 11)	// Alla fönster
#define SE_RESIZE_WINDOW (1 << 12)

	/* Mask in Settings */
#define MS_GLOBAL       (1 << 0)	// Globala settings
#define MS_LOCAL        (1 << 1)	// Lokala
#define MS_SCREEN       (1 << 2)	// Anknyting till intuition
#define MS_DISPLAY      (1 << 3)	// Anknyting till viewar
#define MS_IO           (1 << 4)	// IO-kommunikation
#define MS_READ         (1 << 5)	// Read only
#define MS_USERDEFINED  (1 << 6)	// Användardefinierade
#define MS_HIDDEN       (1 << 7)	// Gömda
#define MS_SYSTEM       (1 << 8)	// Parametrar

	// Lablar för värden som måste kalkuleras.
enum {
  SECALC_NOTHING,
  SECALC_FULLNAME,	/* Give path and name */
  SECALC_COLUMN,	/* Give cursor column */
  SECALC_LINE,		/* Give the current line */
  SECALC_LINELEN,	/* Give the length of the current line */
  SECALC_BLOCK_NAME,	/* Give the name of the current block buffer */
  SECALC_MOUSE_X,	/* Last mouse position */
  SECALC_MOUSE_Y,
  SECALC_SEARCHBUFFER,	/* Give the search buffer */
  SECALC_REPLACEBUFFER,	/* Give the replace buffer */
  SECALC_MACRO,		/* Macro on/off */
  SECALC_COLOUR,	/* Read colour */
};

	// Lablar för switch efter en ändring är gjord.
enum {
  SEUP_NOTHING,
  SEUP_PRIO,
  SEUP_APPICON,
  SEUP_APPWINDOW,
  SEUP_NEWKEYMAP,
  SEUP_CHANGES,
  SEUP_LINECOUNT,
  SEUP_CURSOR_X,
  SEUP_CURSORMOVE,		// Byte av cursor-positioner,curr_toppline
  SEUP_STRINGPOS,		// Ändring av string_pos.
  SEUP_PENS_CHANGED,
  SEUP_SEARCH_FLAG,
  SEUP_SEARCH_ALL_FLAG,
  SEUP_DIRECTORY,
  SEUP_PROTECTION_BITS,
  SEUP_FPLDEBUG,
  SEUP_RESIZE,
  SEUP_FACE_CHANGE,
  SEUP_FACT_CHANGE,
  SEUP_FILEHANDLER,
  SEUP_TITLE_CHANGE,
  SEUP_SLIDERCHANGE
};

typedef struct Setting {
  char *name;	/* namn på settingen		*/
  WORD update;	/* vilken handling som ska ske när variabeln ändras */
  char **cycle;/* pekare till en array strängar för cykling */
  WORD type;	/* vilken sorts setting		*/
  int min;	/* min num value		*/
  int max;	/* max num value / längden på strängen som är allokerad i strukturen */
  int offset;	/* offset till variabeln / vilken kalkuleringrutin som ska tillkallas */
  WORD setinstruct; /* Label for the type of action to be when the setting is set */
  char *FPLnotify;  /* Notify this FPL-script when setting is changed */
  char *FPLaddition; /* FPL-script to call, to get an input */
  int mask;	/* Maskvärde för settingen. */
  char *FPLsave_string;	/* The string that will be used if the setting shall be saved. */
  int menucounter; /* Number of times the setting is used in the menues. */
};

typedef struct SettingSaved {
  char *name;
  char *type;
  long value;
  char *string;
  struct SettingSaved *next;
};


int __regargs InitDefaultSetting(void);
int __regargs DeleteSetting(char *name);
int __regargs AddSetting(char *name, char *notify, char *FPLaddition, char *cyclestring, WORD type, int min, int max, int init, char format, int mask);
int __regargs ConstructSetting(char *name, char *notify, char *FPLaddition, char *typestring, char *cycle, int min, int max, int init, char format);
int __regargs GiveMaskNumber(char *type, char **newpoint);
int __regargs ExtractType(char *typestring, int *typestore, int *maskstore, int *min, int *max);
void __regargs LogSetting(char *name, char *type, long value, int format);
void __regargs FindLogSetting(char *name, char *type, long *value, char *format);
void __regargs DeleteLogSetting(void);

#endif //SETTING_H
