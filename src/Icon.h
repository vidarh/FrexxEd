/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Icon.h
 *
 *********/

/********************
 *
 * MakeIcon()
 *
 * Updates/creates an icon for the specified file.
 *
 * Returns TRUE on failure.
 *
 *****/

int MakeIcon(char *);

RetCode FreeToolTypes(struct ToolInfo *tool);
RetCode SetToolTypes(char *file, char **newtools);
RetCode GetToolTypes(char *file, struct ToolInfo *tool);
char CheckIcon(char *name);

#define FREXXED_ICONDIR "FrexxEd:icons/"
#define FREXXED_NONE    ".none"
#define FREXXED_DEFAULT ".default"
#define FREXXED_APPICON ".appicon"

#define FREXXED_TOOL "(FREXXED USE)"

struct ToolInfo {
  char **Tools; /* tooltype array */
  char *File; /* to gather the information at one place */
  int TotalNum; /* total number of tooltype lines */
  int CustomNum; /* number of FrexxEd lines, including the 2 separators! */
  int FirstCustom; /* the first line of the custom ones */
};

