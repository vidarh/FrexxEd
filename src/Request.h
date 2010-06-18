/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Request.h
 *
 * Commmunicate with the user. One-way and both ways...
 *
 *********/

/**********************************************************************
 *
 * int PromptFile(BufStruct *, char *, char *)
 *
 * Returns TRUE if the user selected a file name. The entire path
 * is stored in the buffer (char *) points to, this buffer is also
 * used to set the defaultname and path.
 * If a Storage is given the path will be found there.
 * The flags shall be in reqtools filerequester style.
 *
 *****/
int __regargs PromptFile(BufStruct *Storage, char *path, char *header, char *mask, int flags, APTR *);

/********************************************************************
 *
 * int Ok_Cancel(BufStruct *Storage, char *messy, char *title, char *answer);
 *
 * Returns (int) true or false on the question (char *).
 *
 ********/
int __regargs Ok_Cancel(BufStruct *Storage, char *messy, char *title, char *answer);

/*************************************************************
 *
 *  FreezeEditor(int)
 *
 *  Will make all gadgets inactive.
 *  Clock TRUE if you want a clocksprite.
 **********/
void __regargs FreezeEditor(int);

/*************************************************************
 *
 *  ActivateEditor()
 *
 *  Will make all gadgets active.
 **********/
void ActivateEditor(void);

/********************************************************
 *
 * int ScreenModeReq(BufStruct *Storage)
 *
 * Will change the current screenmode, with help of a request.
 *
 * Return a ret-value.
 *****/
int ScreenModeReq(BufStruct *Storage);

int __regargs PaletteRequest(BufStruct *Storage);

int __regargs PromptFont(char *reqstring, int flag);

