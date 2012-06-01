
// Amiga specific declarations

#ifdef AMIGA
#include <devices/console.h>
#include <devices/keymap.h>
#include <dos/rdargs.h>
#include <exec/types.h>
#include <exec/semaphores.h>
#include <graphics/rastport.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <libraries/dos.h>
#include <libraries/gadtools.h>
#include <libraries/reqtools.h>
#include <libraries/locale.h>
#include <workbench/workbench.h>
#include <utility/tagitem.h>
#include <exec/types.h>
#else
#include "compat.h"
#endif

#include "Buf.h"

/***************************** IDCMP.c ********************************/

struct InputEvent ievent = {	/* used for RawKeyConvert() */
  NULL,
  IECLASS_RAWKEY,
  0,0,0
};


/***************************** Init.c **********************************/

struct PropInfo VertSliderSInfo = {			/* Default */
  AUTOKNOB|FREEVERT|PROPNEWLOOK|PROPBORDERLESS,       /* PropInfo flags */
  65535,65535,             /* horizontal and vertical pot values */
  65535,65535,             /* horizontal and vertical body values */
   0,0,0,0,0,0
};


struct Image SliderImage = {			/* Default */
  0,0,7,1,0, NULL,0x0000,0x0000, NULL
};

struct Gadget VertSlider = {			/* Default */
  NULL, -20, 20, 16, 20,
  NULL,
  GACT_RELVERIFY|GACT_IMMEDIATE,
  GTYP_PROPGADGET,                   /* Proportional Gadget */
  NULL, 			  /* Slider Imagry */
  NULL,NULL,NULL,
  NULL,			  /* SpecialInfo structure */
  1, /* SLIDER,*/
  NULL
};

FrexxBorder borderdef = {
  {
    NULL, 0, 0, 8, 0,
    GFLG_GADGHBOX,
    NULL, //RELVERIFY,
    GTYP_BOOLGADGET,
    NULL,
    NULL,NULL,NULL,
    NULL,
    0,
    NULL
  },
  {
    -1, -1,
    2, 0,
//    JAM1, 5,
    JAM1, 2,
    NULL, NULL
  },
  {
    0,0,0,0,0,0,0,0,0,0
  },
};


/***************************** OpenClose.c *****************************/

struct rtFileRequester *FileReq=NULL; /* Buffrad Filerequester. Bra o ha! */

struct SignalSemaphore LockSemaphore;

struct Message msgsend = {
    NULL,
    NULL,
    NT_MESSAGE,
    0,
    NULL,
  NULL,
  sizeof(struct MsgPort)
};

struct TextAttr topazfont = {
  "topaz.font",
  TOPAZ_EIGHTY,
  FS_NORMAL,
  FPF_ROMFONT
};

/****************************** UpdtBlock.c ***************************/
struct BitMap EmptyBitMap={
  10000,
  10000,
  0,
  2,
  0,
  0,0,0,0,0,0,0,0
};

