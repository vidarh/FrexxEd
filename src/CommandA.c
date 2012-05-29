
// Amiga specific code for command execution

#include "compat.h"

#ifdef AMIGA
#include <dos/datetime.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <exec/types.h>
#include <intuition/screens.h>
#include <libraries/dos.h>
#include <libraries/reqtools.h>
#include <limits.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/FPL.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/reqtools.h>
#include <proto/utility.h>
#include <proto/graphics.h>
#undef GetOutlinePen
#include <graphics/gfxmacros.h>
#include <graphics/rastport.h>

#endif

#include "Limit.h"
#include "Strings.h"
#include "Function.h"
#include "Buf.h"
#include "Init.h"

extern char buffer[];
extern struct TextFont *RequestFont;	/* Font used by requsters */
extern int requestfontwidth;		// Kalkulerad bredd på requestfonten.
extern DefaultStruct Default;
extern BufStruct *NewStorageWanted;
extern struct rtFileRequester *FileReq;

int RequestGotoLine(BufStruct * Storage) {
    int ret = 0;
    buffer[0]=0;
    if (rtGetString(buffer,MAX_CHAR_LINE, RetString(STR_GOTO_LINE), NULL, RTGL_Width, requestfontwidth*20, RT_TextAttr, &Default.RequestFontAttr, (BUF(window) && BUF(window)->screen_pointer?RT_Screen:TAG_IGNORE), BUF(window)?BUF(window)->screen_pointer:NULL, TAG_END)) {
        char *pek=buffer;
        int slask=fplStrtol(pek, 0, &pek);
        int slaskis=fplStrtol(pek, 0, &pek);
        if (slask>SHS(line))
            slask=SHS(line);
        else if (slask<=0)
            slask=1;
        SetScreen(Storage, slaskis, slask);
    } else
        ret=FUNCTION_CANCEL;
    return ret;
}

int RequestResizeBuf(BufStruct * Storage2,BOOL resize, int lines) {
    int ret = 0;
    int slask = lines;
    if (resize || rtGetLong((ULONG *)&slask, RetString(STR_ENTER_DESIRED_VALUE),
                          NULL, RTGL_Width, requestfontwidth*20, RTGL_Min, 0,
                          RTGL_Max, Storage2->window->window_lines,
                          RTGL_ShowDefault, FALSE, TAG_END)) {
        ret=NEW_STORAGE;
    } else
        ret=FUNCTION_CANCEL;
    return ret;
}

void DoWindowFront(int command, int Argc, void ** Argv, WindowStruct * win) {
    if (command==DO_WINDOWFRONT && Argc && (((int)Argv[0])==-1)) {
        if (!(win->window&FX_WINDOWBIT)) {
            if (win->screen_pointer->NextScreen) {
                ScreenToFront(win->screen_pointer);
                if (win->screen_pointer->FirstWindow)
                    ActivateWindow(win->screen_pointer->FirstWindow);
            }
        } else {
            WindowToBack(win->window_pointer);
            if (win->window_pointer->NextWindow)
                ActivateWindow(win->window_pointer->NextWindow);
        }
    } else {
        if (command==DO_ICON_CLICK || !Argc || !(((int)Argv[0])&2)) {
            ScreenToFront(win->screen_pointer);
            if (win->window!=FX_SCREEN)
                WindowToFront(win->window_pointer);
        }
        if (command==DO_ICON_CLICK || !Argc || !(((int)Argv[0])&1))
            ActivateWindow(win->window_pointer);
    }
}


