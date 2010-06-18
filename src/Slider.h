/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/***********************************************
 *
 *  Slider.h
 *
 ***********/

void __regargs InitSlider(BufStruct *Storage);
void __regargs PositionSlider(BufStruct *Storage);
void __regargs InitBorder(BufStruct *Storage);

void __regargs RemoveBufGadget(BufStruct *Storage);
void __regargs AddBufGadget(BufStruct *Storage);

void __regargs ShortInitSlider(BufStruct *Storage);// only called from AddBufGadget and OpenUpScreen

void __regargs MoveSlider(BufStruct *Storage);

