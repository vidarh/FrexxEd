#include <libraries/gadtools.h>
#include <workbench/workbench.h>

static UWORD Image1Data[] = {
  0x00FF, 0xFFFF, 0xFFC0, 
  0x0080, 0x0FFC, 0x0000, 
  0x01C0, 0x07F8, 0x0000, 
  0x0360, 0x03F0, 0x0000, 
  0x06F0, 0x01E0, 0x0000, 
  0x07F0, 0x00C0, 0x0000, 
  0x07F0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x01C0, 0x00C0, 0x0000, 
  0x01C0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x0DF8, 0x00C0, 0x0000, 
  0x13FC, 0x00C0, 0x0000, 
  0x2FFE, 0x00C0, 0x0000, 
  0x2C1E, 0x00C1, 0xF000, 
  0x780F, 0x00C1, 0x0000, 
  0x7007, 0x00C1, 0x0000, 
  0x6003, 0x00C1, 0xE000, 
  0x6003, 0x00C1, 0x0000, 
  0x6003, 0x00C1, 0x1040, 
  0x2002, 0x00C1, 0x0880, 
  0x2002, 0x00C1, 0x0500, 
  0x1004, 0x00C0, 0x0200, 
  0x09C8, 0x00C0, 0x0500, 
  0x07F0, 0x00C0, 0x0880, 
  0x0470, 0x00C0, 0x1040, 
  0x0DF8, 0x00C0, 0x0000, 
  0x0FF8, 0x00C0, 0x0000, 
  0x0FF8, 0x00C0, 0x0000, 
  0x07F0, 0x80C0, 0x0000, 
  0x07F0, 0x80C0, 0x2000, 
  0x01C0, 0x40C7, 0xE000, 
  0x0000, 0x40C1, 0xE000, 
  0x0000, 0x3001, 0xE000, 
  0x0000, 0x0C06, 0x2000, 
  0x0000, 0x03F8, 0x0000, 

  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0080, 0x0000, 0x0000, 
  0x0100, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0007, 0xFFF0, 
  0x0200, 0x0007, 0xFFF0, 
  0x0C00, 0x0007, 0xFFF0, 
  0x1000, 0x0007, 0xFFF0, 
  0x1000, 0x0006, 0x0FF0, 
  0x0000, 0x0006, 0xFFF0, 
  0x0000, 0x0006, 0xFFF0, 
  0x0000, 0x0006, 0x1FF0, 
  0x0000, 0x0006, 0xFFF0, 
  0x0000, 0x0006, 0xEFB0, 
  0x0000, 0x0006, 0xF770, 
  0x0000, 0x0006, 0xFAF0, 
  0x0000, 0x0007, 0xFDF0, 
  0x0000, 0x0007, 0xFAF0, 
  0x0000, 0x0007, 0xF770, 
  0x0380, 0x0007, 0xEFB0, 
  0x0200, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
};

static struct Image AppIcon1 = {
  0, 0, 44, 38, 2, &Image1Data[0], 3, 0, NULL
};

static UWORD Image2Data[] = {
  0x00FF, 0xFFFF, 0xFFC0, 
  0x0080, 0x0FFC, 0x0000, 
  0x01C0, 0x07F8, 0x0000, 
  0x0360, 0x03F0, 0x0000, 
  0x06F0, 0x01E0, 0x0000, 
  0x07F0, 0x00C0, 0x0000, 
  0x07F0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x01C0, 0x00C0, 0x0000, 
  0x01C0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x03E0, 0x00C0, 0x0000, 
  0x07F0, 0x00C0, 0x0000, 
  0x1BFC, 0x00C0, 0x0000, 
  0x27FE, 0x00C0, 0x0000, 
  0x5FFF, 0x00C1, 0xDBC0, 
  0x580F, 0x00C0, 0x0000, 
  0xF007, 0x80C1, 0xD980, 
  0xE003, 0x80C0, 0x0000, 
  0xC001, 0x80C1, 0xBD00, 
  0xC001, 0x80C0, 0x0000, 
  0xC001, 0x80C1, 0xDE80, 
  0x4001, 0x00C0, 0x0000, 
  0x4001, 0x00C1, 0xBAC0, 
  0x2002, 0x00C0, 0x0000, 
  0x1004, 0x00C1, 0xB000, 
  0x0000, 0x00C0, 0x0000, 
  0x0000, 0x00C0, 0x0000, 
  0x00E0, 0x00C0, 0x03C0, 
  0x03F8, 0x00C0, 0x0000, 
  0x0238, 0x00C0, 0x0000, 
  0x06FC, 0x00C0, 0x0000, 
  0x07FC, 0x00C0, 0x0000, 
  0x07FC, 0x00C0, 0x0000, 
  0x03F8, 0x00C0, 0x0000, 
  0x03F8, 0x0000, 0x0000, 
  0x00E0, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 

  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0080, 0x0000, 0x0000, 
  0x0100, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0007, 0xFFF0, 
  0x0400, 0x0007, 0xFFF0, 
  0x1800, 0x0007, 0xFFF0, 
  0x2000, 0x0007, 0xFFF0, 
  0x2000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x0000, 0x0007, 0xFFF0, 
  0x01C0, 0x0000, 0x0000, 
  0x0100, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
  0x0000, 0x0000, 0x0000, 
};

static struct Image AppIcon2 = {
  0, 0, 44, 38, 2, &Image2Data[0], 3, 0, NULL
};

struct DiskObject AppIconDObj =
{
    NULL,                                        /* Magic Number */
    NULL,                                        /* Version */
    {                                            /* Embedded Gadget Structure */
        NULL,                                    /* Next Gadget Pointer */
        0, 0, 44, 39,                            /* Left,Top,Width,Height */
        GFLG_GADGHIMAGE,                              /* Flags */
        NULL,                                    /* Activation Flags */
        NULL,                                    /* Gadget Type */
        (APTR) & AppIcon1,                       /* Render Image */
        (APTR) & AppIcon2,                       /* Select Image */
        NULL,                                    /* Gadget Text */
        NULL,                                    /* Mutual Exclude */
        NULL,                                    /* Special Info */
        0,                                       /* Gadget ID */
        NULL,                                    /* User Data */
    },
    WBAPPICON,                                   /* Icon Type */
    NULL,                                        /* Default Tool */
    NULL,                                        /* Tool Type Array */
    NO_ICON_POSITION,                            /* Current X */
    NO_ICON_POSITION,                            /* Current Y */
    NULL,                                        /* Drawer Structure */
    NULL,                                        /* Tool Window */
    NULL                                         /* Stack Size */
};
