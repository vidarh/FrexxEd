
/* Generic UI Actions handling extracted from IDCMP.c */

#include "Buf.h"
#include "BuildMenu.h"
#include "IDCMP.h"

#include <assert.h>

extern char build_default_menues;
extern DefaultStruct Default;

extern char windowresized;

#define RESIZE_TIMEOUT 3 // Ticks to wait until resize of window

int RunEventHook(BufStruct * CommandStorage, char ** command, int flag) {
    int temp;
    CommandStorage->locked++;
    CommandStorage->shared->current++;
    temp=RunHook(CommandStorage, DO_EVENT, 1, command, flag);
    CommandStorage->locked--;
    CommandStorage->shared->current--;
    return temp;
}

void handle_CHANGEWINDOW(WindowStruct * win)
{
    if (win) {
        win->window_resized=RESIZE_TIMEOUT;
        windowresized++;
    }
}

void DeIconify(BufStruct * Storage) {
        BUF(locked)++;
        SHS(current)++;
        Command(Storage, DO_DEICONIFY|NO_HOOK, 0, NULL, 0);
        BUF(locked)--;
        SHS(current)--;
}

int handle_MENUCLEAR() {
    int ret = OK;
    if (build_default_menues) {
        if (SetDefaultMenu(&menu)) /* set up the entire internal menu structure list */
            ret = OUT_OF_MEM;     /*! Should we not bail early here? */
        WindowStruct *wincount;
        wincount=FRONTWINDOW;
        while (wincount) {
            if (menu_attach(wincount))
                ret=OUT_OF_MEM;
            wincount=wincount->next;
        }
        /*! Is it worth explicitly setting this to 0 here? Otherwise there is a 
          potential infinite loop */
        assert(build_default_menues == 0);
    }
    return ret;
}

void handle_WINDOWSIZE(BufStruct * Storage, BufStruct * CommandStorage,   ReturnMsgStruct *retmsg, char * Argv[3]) {
    BUF(locked)++;
    SHS(current)++;
    Default.commandcount++;
    Argv[0]=(char *)retmsg->args[0];
    Argv[1]=(char *)retmsg->args[1];
    RunHook(CommandStorage, DO_NEWWINDOWSIZE, 1, (char **)&Argv, NULL);
    BUF(locked)--;
    SHS(current)--;
}


