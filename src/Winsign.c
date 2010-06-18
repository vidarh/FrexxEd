/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * WinSign.c
 *
 * Handle all IDCMP signals...
 *
 *********/

#include <exec/types.h>
#include <exec/io.h>
#include <intuition/intuition.h>
#include <libraries/dos.h>
#include <libraries/gadtools.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <clib/macros.h>

#include "Buf.h"
#include "BufControl.h"
#include "Command.h"
#include "Execute.h"
#include "Winsign.h"
#include "IDCMP.h"
#include "BuildMenu.h"

extern struct Gadget VertSlider;
extern struct IOStdReq *WriteReq, *ReadReq;
extern struct MsgPort *ReadPort;
extern BufStruct *NewStorageWanted;
extern DefaultStruct Default;

BufStruct __regargs *GadgetAddress(BufStruct *Storage, struct IntuiMessage *msg)
{
  if (FRONTWINDOW) {
    BufStruct *Storage2=FRONTWINDOW->NextShowBuf;
    struct Gadget *gadget= (struct Gadget *) msg->IAddress;
    while (Storage2 && &Storage2->slide.Slider != gadget)
      Storage2=Storage2->NextShowBuf;
    if (!Storage2)
      return(NULL);
    else if (Storage2 != Storage)
      return(Storage2);
    return(Storage);
  }
}
