
Updating the FrexxEd Palette handling
=====================================

FrexxEd is designed to work with low-color palette based screen modes. To maintain 
compatibility with older versions of AmigaOS, we wish to retain that ability.

Currently FrexxEd works directly on the screen palette. Two commands, ColorAdjust
and ColorRes, are available to manipulate the screen palette.

This works ok on private screens, but not so well on the workbench.

The new system will work like this:

 * Code will be modified to request a 24 bit RGB value, but fall back to the "standard"
   pens if needed.
 * ObtainBestPen() will be used to assign a pen when on a paletted display.
 * A palette of up to 256 entries can be saved but note that editing the palette on
   a secreen that can't show all of them will be problematic. 
 * When opening on a public screen, we will respect the palette that is there.
 * When opening on a private screen, we will set as many entries from the saved
   palette that we can. For best results, the palette should be optimized so that the
   most important spread of colors come in the first 8, then 16, then 32, then 64 
   entries, so that when opening on a low bitplane screen you still get a reasonable
   approximation.
   
 * FACE/FACT and other code will be upgraded to obtain pens by RGB value.
 
