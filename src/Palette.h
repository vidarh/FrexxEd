#ifndef __FREXXED_PALETTE_H
#define __FREXXED_PALETTE_H

/* For WindowStruct */
#include "Buf.h"

long FixRGB32(ULONG *cols);
void CopyColors(struct Screen *sc, WindowStruct *win);
void SetColors(struct Screen *sc, int col, int red, int green, int blue);
void SetColor(struct Screen *sc, int no, int col);
void InitColors(WindowStruct *win);
void ResetColors(int col, WindowStruct *win);

#endif
