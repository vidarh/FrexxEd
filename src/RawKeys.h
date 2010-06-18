/*
 * FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
 *
 * This file is open-source software. Please refer to the file LEGAL
 * for the licensing conditions and responsibilities.
 */

/*  Raw keys explaining */

#define RAW_TILDE    	0x000
#define RAW_1    	0x001
#define RAW_2    	0x002
#define RAW_3    	0x003
#define RAW_4    	0x004
#define RAW_5    	0x005
#define RAW_6    	0x006
#define RAW_7    	0x007
#define RAW_8    	0x008
#define RAW_9    	0x009
#define RAW_0    	0x00A
#define RAW_PLUS    	0x00B
#define RAW_12    	0x00C
#define RAW_13    	0x00D
#define RAW_NUM0    	0x00F

#define RAW_Q    	0x010
#define RAW_W    	0x011
#define RAW_E    	0x012
#define RAW_R    	0x013
#define RAW_T    	0x014
#define RAW_Y    	0x015
#define RAW_U    	0x016
#define RAW_I    	0x017
#define RAW_O    	0x018
#define RAW_P    	0x019
#define RAW_AA    	0x01A
#define RAW_TAK    	0x01B
#define RAW_NUM1    	0x01D
#define RAW_NUM2    	0x01E
#define RAW_NUM3    	0x01F

#define RAW_A    	0x020
#define RAW_S    	0x021
#define RAW_D    	0x022
#define RAW_F    	0x023
#define RAW_G    	0x024
#define RAW_H    	0x025
#define RAW_J    	0x026
#define RAW_K    	0x027
#define RAW_L    	0x028
#define RAW_OE    	0x029
#define RAW_AE    	0x02A
#define RAW_MULT    	0x02B
#define RAW_NUM4    	0x02D
#define RAW_NUM5    	0x02E
#define RAW_NUM6    	0x02F

#define RAW_GAP    	0x030
#define RAW_Z    	0x031
#define RAW_X    	0x032
#define RAW_C    	0x033
#define RAW_V    	0x034
#define RAW_B    	0x035
#define RAW_N    	0x036
#define RAW_M    	0x037
#define RAW_KOMMA    	0x038
#define RAW_PUNKT    	0x039
#define RAW_MINUS    	0x03A
#define RAW_NUMPUNKT   	0x03C
#define RAW_NUM7    	0x03D
#define RAW_NUM8    	0x03E
#define RAW_NUM9    	0x03F

#define RAW_SPACE    	0x040
#define RAW_BACK    	0x041
#define RAW_TAB    	0x042
#define RAW_ENTER    	0x043
#define RAW_RETURN    	0x044
#define RAW_ESC    	0x045
#define RAW_DEL    	0x046
#define RAW_NUMMINUS	0x04A
#define RAW_UP    	0x04C
#define RAW_DOWN    	0x04D
#define RAW_RIGHT    	0x04E
#define RAW_LEFT    	0x04F

#define RAW_F1    	0x050
#define RAW_F2    	0x051
#define RAW_F3    	0x052
#define RAW_F4    	0x053
#define RAW_F5    	0x054
#define RAW_F6    	0x055
#define RAW_F7    	0x056
#define RAW_F8    	0x057
#define RAW_F9    	0x058
#define RAW_F10    	0x059
#define RAW_NUMKLAMMER1	0x05A
#define RAW_NUMKLAMMER2	0x05B
#define RAW_NUMDIV    	0x05C
#define RAW_NUMMULT    	0x05D
#define RAW_NUMPLUS    	0x05E
#define RAW_HELP    	0x05F

#define RAWC_LEFT_SHIFT   	0x060
#define RAWC_RIGHT_SHIFT    	0x061
#define RAWC_CAPS	    	0x062
#define RAWC_CTRL	    	0x063
#define RAWC_LEFT_ALT    	0x064
#define RAWC_RIGHT_ALT    	0x065
#define RAWC_LEFT_AMIGA    	0x066
#define RAWC_RIGHT_AMIGA    	0x067
#define RAWC_LEFT_MOUSE    	0x068
#define RAWC_RIGHT_MOUSE    	0x069
#define RAWC_MIDDLE_MOUSE    	0x06A

#define RAW_SHIFT 0x0003
#define RAW_CAPS  0x0004
#define RAW_CTRL  0x0008
#define RAW_ALT   0x0030
#define RAW_AMIGA 0x00c0

#define RAW_QUAL  (RAW_SHIFT|RAW_CTRL|RAW_ALT|RAW_AMIGA)

#define MOUSELEFT	1
#define MOUSEMIDDLE	2
#define MOUSERIGHT	4
#define RAWPRESSED	8
#define MOUSEDRAG	16
#define MOUSEUP		32
#define MOUSEDOUBLEPRESS 64
#define MOUSETRIPPLEPRESS 128


