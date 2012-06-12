#include "Palette.h"

#ifdef AMIGA
#include <exec/execbase.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
extern struct ExecBase *SysBase;

#endif

#include <stdio.h>

long FixRGB32(ULONG *cols)
{
  long color;
// rgbRGB
  color =((cols[0]&0xf)<<20)+((cols[0]&0xf0)<<4);
  color|=((cols[1]&0xf)<<16)+((cols[1]&0xf0));
  color|=((cols[2]&0xf)<<12)+((cols[2]&0xf0)>>4);
  return color;
}


void __regargs CopyColors(struct Screen *sc, WindowStruct *win)
{
  if (win) {
#ifdef AMIGA
    if (SysBase->LibNode.lib_Version < 39) {
      win->color0=GetRGB4(sc->ViewPort.ColorMap, 0);
      win->color1=GetRGB4(sc->ViewPort.ColorMap, 1);
      win->color2=GetRGB4(sc->ViewPort.ColorMap, 2);
      win->color3=GetRGB4(sc->ViewPort.ColorMap, 3);
    } else 
#endif
        {
            ULONG cols[3*12];
            GetRGB32(sc->ViewPort.ColorMap, 0, 12, cols);
            win->color0=FixRGB32(&cols[0]);
            win->color1=FixRGB32(&cols[3]);
            win->color2=FixRGB32(&cols[6]);
            win->color3=FixRGB32(&cols[9]);
        }
  }
}

void SaveColors(struct Screen *sc, const char * fname)
{
  int i;
  FILE * f = fopen(fname,"w");
  if (!f) return;
  
  if (SysBase->LibNode.lib_Version < 39) {
#if 0
	for(i=0; i<256; ++i) {
	  ULONG col = GetRGB4(sc->ViewPort.ColorMap, i);
	}
#endif
  } else {
	ULONG colors[768];
	GetRGB32(sc->ViewPort.ColorMap,0,256,colors);
	for(i=0; i<256; ++i) {
	  /* FIXME: This needs to do the reverse transformation of SetColors */
	  fprintf(f,"ColorAdjust(%d,%u,%u,%u);\n",i,colors[i*3]>>24,colors[i*3+i]>>24,colors[i*3+2]>>24);
	}
  }
  fclose(f);
}


void SetColors(struct Screen *sc, int col, int red, int green, int blue)
{
  if (SysBase->LibNode.lib_Version < 39) {
    SetRGB4(&sc->ViewPort, col, red, green, blue);
  } else {
    SetRGB32(&sc->ViewPort, col,
             (((red&0xf)<<4)|((red&0xf0)>>4))<<24,
             (((green&0xf)<<4)|((green&0xf0)>>4))<<24,
             (((blue&0xf)<<4)|((blue&0xf0)>>4))<<24);
  }
}

void SetColor(struct Screen *sc, int no, int col)
{
  if (SysBase->LibNode.lib_Version < 39) {
    SetRGB4(&sc->ViewPort, no, (col/256)&15, (col/16)&15, col&15);
  } else {
    SetRGB32(&sc->ViewPort, no,
             (((col>>20)&0xf)|((col>>4)&0xf0))<<24,
             (((col>>16)&0xf)|((col)&0xf0))<<24,
             (((col>>12)&0xf)|((col<<4)&0xf0))<<24);
  }
}

void InitColors(WindowStruct *win)
{
  if (win && win->window_pointer && (win->window==FX_SCREEN || win->window==FX_WINSCREEN) && (win->color0 | win->color1 | win->color2 | win->color3)) {
    SetColor(win->screen_pointer, 0, win->color0);
    SetColor(win->screen_pointer, 1, win->color1);
    SetColor(win->screen_pointer, 2, win->color2);
    SetColor(win->screen_pointer, 3, win->color3);
  }
}

/*************************************************************
*
* ResetColors(int col);
*
* Reset all colors to the Workbench default colors.
*
******/
void ResetColors(int col, WindowStruct *win)
{
  struct Preferences preffan;
  GetPrefs(&preffan, sizeof(preffan));
  if (col&1) {
    register int col = preffan.color0;
    SetRGB4(&win->screen_pointer->ViewPort, 0, (col>>8)&15, (col>>4)&15, (col)&15);
  }
  if (col&2) {
    register int col = preffan.color1;
    SetRGB4(&win->screen_pointer->ViewPort, 1, (col>>8)&15, (col>>4)&15, (col)&15);
  }
  if (col&4) {
    register int col = preffan.color2;
    SetRGB4(&win->screen_pointer->ViewPort, 2, (col>>8)&15, (col>>4)&15, (col)&15);
  }
  if (col&8) {
    register int col = preffan.color3;
    SetRGB4(&win->screen_pointer->ViewPort, 3, (col>>8)&15, (col>>4)&15, (col)&15);
  }
}

