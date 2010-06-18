/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */
/****************************************************************
 *
 * Button.h
 *
 * Buttonpress function.
 *
 *********/

#define Cancel_button_ID	65534
#define Ok_button_ID		65535

#define FPL_ADDITION_GADGET_ID	65533

#define BUT(x) (Buttons->x)

typedef struct {
  char *name;			/* namnet */
  int flags;			/* default- och resultatvärden av knapparna */
  char types;			/* type of flags */
  int typesfull;		/* full type of flags */
  char **cycletext;		/* pointer to cycle text */
  struct Gadget *gadget;	/* Corresponding gadget */
  char *FPLaddition;		/* FPL-script to get input from */
  struct Gadget *additiongadget;/* Corresponding addition gadget */
  char shortcut;		/* Shortcut of the button */
  int settingnumber;		/* Setting number (for Global/Lokal Info) */
  char *depended;		/* Vilken setting som ghostar gadgeten om den är satt */
} ButtonArrayStruct;

typedef struct {
  int antal;			/* storlek på arrayen */
  struct Window *winbutton;	/* window tjafs */
  char *toptext;		/* TopText */
  struct Gadget *firstgadget;	/* firstgadget */
  struct Gadget *gad;		/* last gadget in list */
  struct Gadget *firststring;	/* first string gadget (for tab) */
  int OkCancel;			/* set a ok/cancel gadget */
  int maxheight;		/* tillåten höjd på gadgetarean */
  int xoffset;			/* xoffset för gadgetarna */
  int yoffset;			/* yoffset för gadgetarna */
  int antalrader;		/* antal rader med gadgetar (resultat) */
  int bredd;			/* antal tecken i bredd för gadgetar (resultat) */
  int width;			/* bredd på blivande fönstret (för Cancel-knappen (0 == relativt bredden)) */
  int height;			/* antalet pixelrader som tas upp av gadgetarna (resultat) */
  ButtonArrayStruct *array;	/* Array of ButtonStructArray */
  int string_width;		/* Bredd i tecken på strängar/cycles/numeriskt gadgets */
  struct Screen *screen;
  void *visualinfo;
} ButtonStruct;

/****************
*
* char ButtonPress(char **, int, int *, int *)
*
* Returns -1 om det blev fel eller cancelled.
*
****/

int __regargs ButtonPress(BufStruct *Storage, ButtonStruct *Buttons);

void __regargs CloseButton(ButtonStruct *Buttons);

int __regargs OpenButton(ButtonStruct *Buttons);

int __regargs SetButton(ButtonStruct *Buttons);

void __regargs InitButtonStruct(ButtonStruct *Buttons);

void __regargs ReadButtons(ButtonStruct *Buttons);

struct Gadget __regargs *CheckHotKey(ButtonStruct *buts, char code);
