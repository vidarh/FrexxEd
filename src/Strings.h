/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
#ifndef STRING_H
#define STRING_H

/****************************************************************
 *
 * String.h
 *
 * String enums and things.
 *
 *********/

typedef enum {
  STR_UNKNOWN_COMMAND,
  STR_OUT_OF_MEMORY,
  STR_SYNTAX_ERROR,
  STR_NOTHING_LEFT_TO_UNDO,
  STR_NO_BLOCK_MARKED,
  STR_CANT_DELETE_ANYTHING,
  STR_ERROR_DURING_OPEN,
  STR_BLOCK_BUFFER_IS_EMPTY,
  STR_NO_MORE_MATCHES_FOUND,
  STR_FUNCTION_CANCELLED,
  STR_NO_MATCHING_CHARACTER_FOUND,
  STR_WRONG_TYPE_OF_BLOCK,
  STR_COULDNT_CHANGE_SETTING,
  STR_FPL_PROGRAM_NOT_FOUND,
  STR_FILE_NOT_FOUND,
  STR_NO_BLOCK_FOUND,
  STR_IFF_ERROR,
  STR_CANT_LOCK_BUFFER,
  STR_CANT_FIND_BUFFER,
  STR_WRONG_FILE_NAME,
  STR_INFO_ALREADY_EXISTS,
  STR_UNREG_VERSION,

  STR_READ_PROTECTED,
  STR_WRITE_PROTECTED,
  STR_DELETE_PROTECTED,

  STR_WORKBENCH_NOT_RUNNING,	// New 941108

  STR_DUMMY1,
  STR_DUMMY2,
  STR_DUMMY3,

  STR_GARBAGE_COLLECTION_IN_PROCESS,
  STR_MOVING_BLOCK,
  STR_MOVE_BLOCK_TEXT,

  STR_OK_GADGET,
  STR_CANCEL_GADGET,

  STR_PRESS_BUTTONS,
  STR_RETRY_OPEN_SCREEN,
  STR_SAVE_SETTING,
  STR_FILE_NAME,
  STR_YESNO,
  STR_FREXXED_REQUEST,
  STR_PICK_SCREEN_MODE,
  STR_ABOUT_TEXT,
  STR_SORT_ACCORDING_FIELD,
  STR_MAKE_YOUR_CHOICE,
  STR_GOTO_LINE,
  STR_PICK_FILE_INCLUDE,
  STR_LOAD_FILE,
  STR_BEGIN_RECORDING,
  STR_ENTER_DESIRED_VALUE,
  STR_NO_SCREEN_FOUND,
  STR_EDITING_NEW_FILE,
  STR_LOADING,
  STR_LOADING_READY,
  STR_FILE_CHANGED_SINCE_LOADED,
  STR_CRUNCHING_BUFFER,
  STR_SAVING_TEXT,
  STR_XPKSAVING_TEXT,
  STR_SAVE_IT_NORMAL_ADD,
  STR_XPK_ERROR,
  STR_CANT_CRUNCH_BUFFER,
  STR_SAVING_TROUBLE,
  STR_WRITE_ERROR,
  STR_WROTE,
  STR_OPEN_ERROR,
  STR_SELECT_FONT,
  STR_ENTER_NUMBER,
  STR_ENTER_STRING,
  STR_FPL_ERROR_ADD,
  STR_FPL_HALTED,
  STR_NONE,
  STR_GET_MEMORY,
  STR_FREXXED_INIT,
  STR_FREXXED_REQUIRES_204,
  STR_ERROR_WITH_FPL_FILE,
  STR_CANT_FIND_STARTUP_FILE,
  STR_IMPORTANT,
  STR_NO_SHIT,
  STR_NEW_FONT_INSTALLED,
  STR_CANT_INSTALL_FONT,
  STR_COULDNT_OPEN_FONT,
  STR_FONT_TOO_BIG,
  STR_INPUT_KEY_STROKE,
  STR_KEY_SEQUENCE,
  STR_FUNCTION,
  STR_KEY_ASSIGN,
  STR_ERROR,
  STR_FUNCTION_ASSIGNED_TO_KEY,
  STR_MACRO_NAME,
  STR_MACRO_ASSIGN,
  STR_NO_KEYSTROKE_FOUND,
  STR_WAITING_FOR_ANOTHER_KEY,
  STR_NO_FUNCTION_ASSIGNED_TO_KEY,
  STR_ILLEGAL_COMMAND,
  STR_REPLACE_OPTIONS,
  STR_GLOBAL_REPLACE_WORKING,
  STR_BREAK_REPLACE_WITH_SPACE,
  STR_INIT_FPL,
  STR_FREXXED_COULDNT, /* max 40 bytes! */
  STR_FREXXED_CAN_CONTINUE_BEFORE,
  STR_TRY_AGAIN,
  STR_GET_PUBLIC_SCREEN,
  STR_CREATE_WRITE_PORT,
  STR_CREATE_WRITE_REQUEST,
  STR_CREATE_READ_PORT,
  STR_CREATE_MESSAGE_PORT,
  STR_OPEN_SCREEN,
  STR_GET_VISUALINFO,
  STR_OPEN_WINDOW,
  STR_CREATE_MENUS,
  STR_OPEN_CONSOLE_DEVICE,
  STR_SEARCH_FLAGS,
  STR_SEARCH_REPLACE,
  STR_SEARCH_FOR,
  STR_LINE, /* max 5 chars */
  STR_COL, /* max 5 chars */
  STR_SELECT_ITEM,

  STR_PROJECT_MENU,
  STR_PROJECT_NEW,
  STR_PROJECT_LOAD,
  STR_PROJECT_SAVE,
  STR_PROJECT_ABOUT,
  STR_PROJECT_KILL,
  STR_PROJECT_QUIT_ALL,

  STR_BLOCK_MENU,
  STR_BLOCK_MARK,
  STR_BLOCK_CUT,
  STR_BLOCK_COPY,
  STR_BLOCK_INSERT,

  STR_EDIT_MENU,
  STR_EDIT_UNDO,
  STR_EDIT_UNDO_RESTART,

  STR_MOVEVIEW_MENU,
  STR_MOVEVIEW_GOTO_BUFFER,
  STR_MOVEVIEW_GOTO_LINE,

  STR_SEARCHREPLACE_MENU,
  STR_SEARCHREPLACE_R_SEARCH_FRW,
  STR_SEARCHREPLACE_SEARCH,
  STR_SEARCHREPLACE_REPEAT_REPLACE,
  STR_SEARCHREPLACE_REPLACE,

  STR_CUSTOMIZING_MENU,
  STR_CUSTOMIZING_SETTINGS,
  STR_CUSTOMIZING_SETTINGS_LOCALS,
  STR_CUSTOMIZING_SETTINGS_ALL_LOCALS,
  STR_CUSTOMIZING_SETTINGS_GLOBALS,
  STR_CUSTOMIZING_SETTINGS_SAVE,
  STR_CUSTOMIZING_SETTINGS_SCREENMODE,

  STR_SPECIAL_MENU,
  STR_SPECIAL_EXECUTE_FILE,
  STR_SPECIAL_LOAD_MENUS,
  STR_SPECIAL_PROMPT,

  STR_REPLACE_GADGET,
  STR_HALF_GADGET,
  STR_FULL_GADGET,

  STR_NONE_GADGET,
  STR_RIGHT_GADGET,
  STR_LEFT_GADGET,

  STR_SCREEN_GADGET,
  STR_WINDOW_GADGET,
  STR_BACKDROP_GADGET,
    /* WINDSCREEN declared later */

  STR_VISIBLE_GADGET,
  STR_ABSOLUTE_GADGET,

  STR_OFF_GADGET,
  STR_MARKING_GADGET,
  STR_EXIST_GADGET,

	/* H�r kommer nya str�ngar.... / Kjelle */

  STR_EXPAND_RELATIVE_GADGET,
  STR_EXPAND_ALL_GADGET,

  STR_WILDCARD,
  STR_CASE_SENSITIVE,
  STR_ONLY_WORDS,
  STR_FORWARD_SEARCH,
  STR_INSIDE_BLOCK,
  STR_PROMPT_REPLACE,
  STR_LIMIT_WILDCARD,

  STR_ENTER_PASSWORD_TITLE,
  STR_ENTER_PASSWORD,

  STR_UNREG_VERSION_LONG,

  STR_CHOOSE_ENTRY,
  
  STR_END_OF_FILE,

  STR_REGISTERED_TO,

  STR_ICON_NEVER,
  STR_ICON_ALWAYS,
  STR_ICON_IF_PARENT,

  STR_MOVEVIEW_MAXIVIEW,

  STR_SORT_FORWARD,
  STR_SORT_CASE_SENSITIVE,

  STR_SAVE_NONAME,

  STR_INSIDE_FOLD, //950417
  STR_CANT_QUIT,

  STR_WINSCREEN_GADGET,

  STR_NEW_WINDOW,

  STR_LAST_STRING /* this is no string, only a delimiter */
} String_Num;


char __regargs *RetString(String_Num);
int __regargs Sprintf(char *, char *, ...);
char __regargs *GetRetMsg(int num);

#endif
