/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * WinSign.h
 *
 *********/

void __regargs DragSlider(BufStruct *Storage);

BufStruct __regargs *GadgetAddress(BufStruct *Storage, struct IntuiMessage *msg);

/***********************************************************
 *
 * SliderFix(BufStruct *);
 *
 * Call this to update the screen according to the proper value from the
 * slider.
 *
 ****/
void __regargs SliderFix(BufStruct *Storage);

/*****************************************
*
* int Winsignal (BufStruct *, struct IntuiMessage *);
*
* This function handles all menus', actions.
* Returns the command to run.
*
*****/
int __regargs Winsignal(BufStruct *Storage, struct IntuiMessage *msg);

