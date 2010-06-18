SAS AMIGA 680x0OBJ Module Disassembler 6.55
Copyright © 1995 SAS Institute, Inc.


Amiga Object File Loader V1.00
68000 Instruction Set

EXTERNAL DEFINITIONS

_AddFaceString 0000-00    _MatchWord 01B6-00    _MatchAnyWhere 02BC-00    
_FindEnd 0350-00    _ScanBuffer 04EA-00    _InitFace 0674-00    _main 06A4-00
_PrintText 084E-00    _FaceReport 08A8-00    _type 0000-01    
_scanline 0000-02    _lastline 0004-02

SECTION 00 "text" 0000090C BYTES
;   1: /*************************************************************************
;   2:  *
;   3:  * The internal theories:
;   4:  * ~~~~~~~~~~~~~~~~~~~~~~
;   5:  * We call the areas with different style and/or colours 'faces'.
;   6:  *
;   7:  * struct FaceControl holds all local face information for a single buffer.
;   8:  *
;   9:  * struct Face holds information about a single string and which face that
;  10:  * string should bring up on screen.
;  11:  *
;  12:  * struct FaceType defines all faces. There is 256 different faces which can
;  13:  * be defined to be any combination of style, background and foreground.
;  14:  *
;  15:  * The FPL function interface:
;  16:  * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
;  17:  * FaceID( name ) will return the ID of the face-setup called 'name'. If name
;  18:  * doesn't already exist as a face-setup, it will get created first.
;  19:  * FaceAdd( ID, string, flags ) adds a string to the specified face-setup.
;  20:  * FaceType( Num, Style, Fg, Bg) sets the specified facetype as specified.
;  21:  *
;  22:  ************************************/
;  23: 
;  24: #include <stdio.h>
;  25: #include <stdlib.h>
;  26: #include <string.h>
;  27: 
;  28: #ifdef UNIX
;  29: #include <sys/types.h>
;  30: typedef unsigned short WORD;
;  31: #define __inline
;  32: #define TRUE 1
;  33: #define FALSE 0
;  34: #else
;  35: #include <exec/types.h>
;  36: #endif
;  37: 
;  38: /* Struktur för textpekare.  Storleken definieras även i UpdtScreenML.i */
;  39: typedef struct {
;  40:   char *text;
;  41:   int length;
;  42:   WORD fold_no;
;  43:   WORD flags;
;  44:   int trash;
;  45: } TextStruct;
;  46: 
;  47: #define XX(a) {a,0,0,0,0}
;  48: 
;  49: #define ERROR 1
;  50: #define OK    0
;  51: 
;  52: /**********************************************************************
;  53:  *
;  54:  * Different character defines:
;  55:  *
;  56:  **********************************************************************/
;  57: 
;  58: #define _U (1<<0)  /* upper case */
;  59: #define _L (1<<1)  /* lower case */
;  60: #define _W (1<<2)  /* also included as a valid identifier character */
;  61: #define _N (1<<3)  /* numerical digit 0-9 */
;  62: #define _S (1<<4)  /* white space */
;  63: #define _C (1<<5)  /* control character */
;  64: #define _P (1<<6)  /* punctation characters */
;  65: #define _X (1<<7)  /* hexadecimal digit */
;  66: 
;  67: /***********************************************************************
;  68:  *
;  69:  * A bunch of useful macros:
;  70:  *
;  71:  **********************************************************************/
;  72: 
;  73: #define isalpha(c)  ((type+1)[c] & (_U|_L))
;  74: #define isupper(c)  ((type+1)[c] & _U)
;  75: #define islower(c)  ((type+1)[c] & _L)
;  76: #define isdigit(c)  ((type+1)[c] & _N)
;  77: #define isxdigit(c) ((type+1)[c] & _X)
;  78: #define isalnum(c)  ((type+1)[c] & (_U|_L|_N))
;  79: #define isspace(c)  ((type+1)[c] & _S)
;  80: #define ispunct(c)  ((type+1)[c] & _P)
;  81: #define isprint(c)  ((type+1)[c] & (_U|_L|_N|_P))
;  82: #define iscntrl(c)  ((type+1)[c] & _C)
;  83: #define isascii(c)  (!((c) & ~127))
;  84: #define isgraph(c)  ((type+1)[c] & (_U|_L|_N|_P))
;  85: #define toascii(c)  ((c) & 127)
;  86: #define toupper(c)  ((type+1)[c] & _L? c-CASE_BIT: c)
;  87: #define tolower(c)  ((type+1)[c] & _U? c+CASE_BIT: c)
;  88: 
;  89: #define isodigit(c) ((c) >= CHAR_ZERO && (c) <= CHAR_SEVEN)
;  90: 
;  91: #define isident(c)  ((type+1)[c] & (_U|_L|_W))
;  92: #define isidentnum(c)  ((type+1)[c] & (_U|_L|_W|_N))
;  93: 
;  94: const char type[257] = { /* Character type codes */
;  95:    _C, /* -1 == regular ANSI C eof character */
;  96:    _C,    _C,	  _C,	 _C,    _C,    _C,    _C,    _C, /* 00		*/
;  97:    _C,    _S,	  _S,	 _C,    _C,    _S,    _C,    _C, /* 08		*/
;  98:    _C,    _C,	  _C,	 _C,    _C,    _C,    _C,    _C, /* 10		*/
;  99:    _C,    _C,	  _C,	 _C,    _C,    _C,    _C,    _C, /* 18		*/
; 100:    _S,    _P,     _P,	 _P,    _P,    _P,    _P,    _P, /* 20	!"#$%&' */
; 101:    _P,    _P,     _P,    _P,    _P,    _P,    _P,    _P, /* 28 ()*+,-./ */
; 102:  _N|_X, _N|_X, _N|_X, _N|_X, _N|_X, _N|_X, _N|_X, _N|_X, /* 30 01234567 */
; 103:  _N|_X, _N|_X,    _P,    _P,    _P,    _P,    _P,    _P, /* 38 89:;<=>? */
; 104:    _P, _U|_X,  _U|_X, _U|_X, _U|_X, _U|_X, _U|_X,    _U, /* 40 @ABCDEFG */
; 105:    _U,    _U,	  _U,	 _U,    _U,    _U,    _U,    _U, /* 48 HIJKLMNO */
; 106:    _U,    _U,	  _U,	 _U,    _U,    _U,    _U,    _U, /* 50 PQRSTUVW */
; 107:    _U,    _U,	  _U,	 _P,    _P,    _P,    _P, _P|_W, /* 58 XYZ[\]^_ */
; 108:    _P, _L|_X,  _L|_X, _L|_X, _L|_X, _L|_X, _L|_X,    _L, /* 60 `abcdefg */
; 109:    _L,    _L,	  _L,	 _L,    _L,    _L,    _L,    _L, /* 68 hijklmno */
; 110:    _L,    _L,	  _L,	 _L,    _L,    _L,    _L,    _L, /* 70 pqrstuvw */
; 111:    _L,    _L,	  _L,	 _P,    _P,    _P,    _P,   000, /* 78 xyz{|}~	*/
; 112:   000,   000,	 000,	000,   000,   000,   000,   000, /* 80 	        */
; 113:   000,   000,	 000,	000,   000,   000,   000,   000, /* 88 	        */
; 114:   000,   000,	 000,	000,   000,   000,   000,   000, /* 90 	        */
; 115:   000,   000,	 000,	000,   000,   000,   000,   000, /* 98 	        */
; 116:   000,   000,	 000,	000,   000,   000,   000,   000, /* A0 	        */
; 117:   000,   000,	 000,	000,   000,   000,   000,   000, /* A8 	        */
; 118:   000,   000,	 000,	000,   000,   000,   000,   000, /* B0 	        */
; 119:   000,   000,	 000,	000,   000,   000,   000,   000, /* B8 	        */
; 120:   000,   000,	 000,	000,   000,   000,   000,   000, /* C0 	        */
; 121:   000,   000,	 000,	000,   000,   000,   000,   000, /* C8 	        */
; 122:   000,   000,	 000,	000,   000,   000,   000,   000, /* D0 	        */
; 123:   000,   000,	 000,	000,   000,   000,   000,   000, /* D8 	        */
; 124:   000,   000,	 000,	000,   000,   000,   000,   000, /* E0 	        */
; 125:   000,   000,	 000,	000,   000,   000,   000,   000, /* E8 	        */
; 126:   000,   000,	 000,	000,   000,   000,   000,   000, /* F0 	        */
; 127:   000,   000,	 000,	000,   000,   000,   000,   000, /* F8 	        */
; 128: };
; 129: 
; 130: 
; 131: struct FaceType {
; 132:   char style;  /* BOLD / ITALICS / UNDERLINE / NORMAL */
; 133:   char fgpen;
; 134:   char bgpen;
; 135: };
; 136: 
; 137: struct Face {
; 138:   struct Face *next; /* next face-string in the list */
; 139:   char facetype; /* which style/colour type that should be used, there can
; 140:                     only be 256 different, referenced at least at the moment
; 141:                     by a mere number between 0 to 255. */
; 142:   unsigned long hash; /* for faster string checks, two strings should be
; 143:                          _very_ unlikely to have the same hash number */
; 144:   long flags;    /* extra knowledge about this */
; 145:   long strlen;
; 146:   long strlen2;
; 147:   char string[1];
; 148:   char string2[1]; /* if there is something that ends this type, only valid
; 149:                       for FACE_STARTING */
; 150: };
; 151: #define FACE_WORDONLY 1  /* classic, only this defined word should be marked
; 152:                             and considered */
; 153: #define FACE_ANYWHERE 2  /* this string can be anywhere */
; 154: #define FACE_BACKSLASH 4 /* this area can be escaped with a '\' letter which
; 155:                             will make the following letter to be ignored,
; 156:                             to fully support i.e C quoted strings */
; 157: #define FACE_1STNONSPC 8 /* this is only valid if its the first non-space
; 158:                             characters of a line */
; 159: #define FACE_OBEYIMPOR 16 /* This flag will make all strings that are marked
; 160:                              as 'important' get marked even if this first
; 161:                              string has been found and we're searching for
; 162:                              the ending one */
; 163: #define FACE_IMPORTANT 32 /* this is a more important style ;) */
; 164: 
; 165: #define MAX_STRING_LENGTH 80 /* maximum length of any face-string */
; 166: #define MAX_CONTINUATIONS 255 /* maximum amount of strings that begin a style
; 167: 				 that ends with another string */
; 168: 
; 169: #define FACETABLE_SIZE 128 /* the larger the faster, though larger than 256
; 170:                               is pointless, and larger than 128 will probably
; 171:                               never get noticed. */
; 172: 
; 173: struct FaceControl {
; 174:   /*
; 175:    * General pointer to the next face control setup.
; 176:    */
; 177:   struct FaceControl *next;
; 178: 
; 179:   /***************************************************************
; 180:    * Local array for each face-setup.
; 181:    * It contains non-zero in the entry if the character is the last character
; 182:    * of a face-control word.
; 183:    ****************************************************************/
; 184:   char exist[256];
; 185: /* FACE_WORDONLY 1 classic, only this defined word should be marked
; 186:                    and considered */
; 187: /* FACE_ANYWHERE 2 this string can be anywhere */
; 188: #define FC_SEVERAL   4 /* more than one control-string ends with this
; 189:                           character */
; 190: #define FC_IMPORTANT 8 /* this is a more prioritized string */
; 191: 
; 192:   /***************************************************************
; 193:    * Array with struct pointers.
; 194:    * The array is only FACETABLE_SIZE items big.
; 195:    * Non-zero entry means a pointer to a "struct Face" holding information
; 196:    * about that single string and its desired colour/style setup.
; 197:    ***************************************************************/
; 198:   struct Face *strings[FACETABLE_SIZE];
; 199: 
; 200:   /***************************************************************
; 201:    * The length of the shortest string added. Lines shorter than
; 202:    * this cannot change anything.
; 203:    ***************************************************************/
; 204:   long shortest;
; 205: 
; 206:   /***************************************************************
; 207:    * The total amount of control strings added.
; 208:    ***************************************************************/
; 209:   long numofstrings;
; 210: 
; 211:   /***************************************************************
; 212:    * The total amount of "continuation" strings added.
; 213:    * That is strings that begin a mode/style.
; 214:    ***************************************************************/
; 215:   long numofconts;
; 216: 
; 217:   /***************************
; 218:    * Name of the face setup to.
; 219:    **************************/
; 220:   char name[1];
; 221: };
; 222: 
; 223: static unsigned long Gethash(char *name, long len);
; 224: void PrintText(TextStruct *textinfo, int lines);
; 225: void FaceReport(struct FaceControl *facecontrol);
; 226: long ScanBuffer(struct FaceControl *facecontrol,
; 227:                 TextStruct *textinfo,
; 228:                 long lines);
; 229: 
; 230: long scanline;
; 231: long lastline;
; 232:                 
; 233: /*
; 234:  * Add a string to a facecontrol.
; 235:  */
; 236: long AddFaceString(struct FaceControl *facecontrol,
; 237:                    char facetype,
; 238:                    char *string,
; 239:                    long strlen,
; 240:                    char *string2,
; 241:                    long strlen2,
; 242:                    long flags)
       | 0000  200F                           MOVE.L      A7,D0
       | 0002  90BC 0000 0014                 SUB.L       #00000014,D0
       | 0008  B0AC  0000-XX.2                CMP.L       ___base(A4),D0
       | 000C  6500  0000-XX.1                BCS.W       __XCOVF
       | 0010  514F                           SUBQ.W      #8,A7
       | 0012  48E7 3F34                      MOVEM.L     D2-D7/A2-A3/A5,-(A7)
       | 0016  282F 0048                      MOVE.L      0048(A7),D4
       | 001A  2A2F 0044                      MOVE.L      0044(A7),D5
       | 001E  2C2F 003C                      MOVE.L      003C(A7),D6
       | 0022  1E2F 0037                      MOVE.B      0037(A7),D7
       | 0026  246F 0040                      MOVEA.L     0040(A7),A2
       | 002A  266F 0038                      MOVEA.L     0038(A7),A3
       | 002E  2A6F 0030                      MOVEA.L     0030(A7),A5
; 243: #define ADD_WORDONLY   1 /* otherwise ANYWHERE */
; 244: #define ADD_1STNONSPC  2
; 245: #define ADD_BACKSLASH  4
; 246: #define ADD_OBEYIMPO   8
; 247: #define ADD_IMPORTANT 16
; 248: 
; 249: {
; 250:   struct Face *face;
; 251:   face = malloc( sizeof(struct Face) + strlen + strlen2);
       | 0032  2006                           MOVE.L      D6,D0
       | 0034  D085                           ADD.L       D5,D0
       | 0036  7218                           MOVEQ       #18,D1
       | 0038  D081                           ADD.L       D1,D0
       | 003A  2F00                           MOVE.L      D0,-(A7)
       | 003C  6100  0000-XX.1                BSR.W       _malloc
       | 0040  584F                           ADDQ.W      #4,A7
       | 0042  2F40 0028                      MOVE.L      D0,0028(A7)
; 252:   if(face) {
       | 0046  4A80                           TST.L       D0
       | 0048  6700 015E                      BEQ.W       01A8
; 253:     char lastchar = string[strlen-1];
       | 004C  1F73 68FF 0027                 MOVE.B      FF(A3,D6.L),0027(A7)
; 254:     
; 255:     memcpy(face->string, string, strlen);
       | 0052  2040                           MOVEA.L     D0,A0
       | 0054  D0FC 0016                      ADDA.W      #0016,A0
       | 0058  224B                           MOVEA.L     A3,A1
       | 005A  2006                           MOVE.L      D6,D0
       | 005C  6002                           BRA.B       0060
       | 005E  10D9                           MOVE.B      (A1)+,(A0)+
       | 0060  5380                           SUBQ.L      #1,D0
       | 0062  64FA                           BCC.B       005E
; 256:     face->string[strlen]=0; /* zero terminate for easier access */
       | 0064  206F 0028                      MOVEA.L     0028(A7),A0
       | 0068  4230 6816                      CLR.B       16(A0,D6.L)
; 257:     face->strlen = strlen;
       | 006C  2146 000E                      MOVE.L      D6,000E(A0)
; 258:     
; 259:     facecontrol->numofstrings++; /* increase number of things added */
       | 0070  52AD 0308                      ADDQ.L      #1,0308(A5)
; 260:     if(string2 && strlen2) {
       | 0074  200A                           MOVE.L      A2,D0
       | 0076  6736                           BEQ.B       00AE
       | 0078  4A85                           TST.L       D5
       | 007A  6732                           BEQ.B       00AE
; 261:       facecontrol->numofconts++; /* increase number of continuation things */
       | 007C  52AD 030C                      ADDQ.L      #1,030C(A5)
; 262:       if(facecontrol->numofconts==MAX_CONTINUATIONS) {
       | 0080  7000                           MOVEQ       #00,D0
       | 0082  4600                           NOT.B       D0
       | 0084  B0AD 030C                      CMP.L       030C(A5),D0
       | 0088  660E                           BNE.B       0098
; 263:         /* we can only take a certain amount of these kind since it will
; 264:            be using data space in the struct stored for each line in a
; 265:            single buffer */
; 266:         free(face);
       | 008A  2F08                           MOVE.L      A0,-(A7)
       | 008C  6100  0000-XX.1                BSR.W       _free
; 267:         return ERROR;
       | 0090  584F                           ADDQ.W      #4,A7
       | 0092  7001                           MOVEQ       #01,D0
       | 0094  6000 0118                      BRA.W       01AE
; 268:       }
; 269:       memcpy(face->string2+strlen, string2, strlen2);
       | 0098  206F 0028                      MOVEA.L     0028(A7),A0
       | 009C  D1C6                           ADDA.L      D6,A0
       | 009E  43E8 0017                      LEA         0017(A0),A1
       | 00A2  204A                           MOVEA.L     A2,A0
       | 00A4  2005                           MOVE.L      D5,D0
       | 00A6  6002                           BRA.B       00AA
       | 00A8  12D8                           MOVE.B      (A0)+,(A1)+
       | 00AA  5380                           SUBQ.L      #1,D0
       | 00AC  64FA                           BCC.B       00A8
; 270:     }
; 271:     face->string2[strlen2+strlen]=0; /* zero terminate for easier access */
       | 00AE  2006                           MOVE.L      D6,D0
       | 00B0  D085                           ADD.L       D5,D0
       | 00B2  206F 0028                      MOVEA.L     0028(A7),A0
       | 00B6  4230 0817                      CLR.B       17(A0,D0.L)
; 272:     face->strlen2 = strlen2;
       | 00BA  2145 0012                      MOVE.L      D5,0012(A0)
; 273:     
; 274:     face->hash = Gethash(string, strlen);
       | 00BE  2F06                           MOVE.L      D6,-(A7)
       | 00C0  2F0B                           MOVE.L      A3,-(A7)
       | 00C2  6100 0742                      BSR.W       0806
       | 00C6  206F 0030                      MOVEA.L     0030(A7),A0
       | 00CA  2140 0006                      MOVE.L      D0,0006(A0)
       | 00CE  504F                           ADDQ.W      #8,A7
; 275:     face->flags = (flags&ADD_1STNONSPC?FACE_1STNONSPC:0)|
       | 00D0  0804 0001                      BTST        #0001,D4
       | 00D4  6704                           BEQ.B       00DA
       | 00D6  7008                           MOVEQ       #08,D0
       | 00D8  6002                           BRA.B       00DC
       | 00DA  7000                           MOVEQ       #00,D0
; 276:                     (flags&ADD_BACKSLASH?FACE_BACKSLASH:0)|
       | 00DC  0804 0002                      BTST        #0002,D4
       | 00E0  6704                           BEQ.B       00E6
       | 00E2  7204                           MOVEQ       #04,D1
       | 00E4  6002                           BRA.B       00E8
       | 00E6  7200                           MOVEQ       #00,D1
       | 00E8  8081                           OR.L        D1,D0
; 277:                       (flags&ADD_OBEYIMPO?FACE_OBEYIMPOR:0)|
       | 00EA  0804 0003                      BTST        #0003,D4
       | 00EE  6704                           BEQ.B       00F4
       | 00F0  7210                           MOVEQ       #10,D1
       | 00F2  6002                           BRA.B       00F6
       | 00F4  7200                           MOVEQ       #00,D1
       | 00F6  8081                           OR.L        D1,D0
; 278:                         (flags&ADD_WORDONLY?FACE_WORDONLY:FACE_ANYWHERE)|
       | 00F8  0804 0000                      BTST        #0000,D4
       | 00FC  57C1                           SEQ         D1
       | 00FE  7401                           MOVEQ       #01,D2
       | 0100  9401                           SUB.B       D1,D2
       | 0102  8082                           OR.L        D2,D0
; 279:                           (flags&ADD_IMPORTANT?FACE_IMPORTANT:0);
       | 0104  0804 0004                      BTST        #0004,D4
       | 0108  6704                           BEQ.B       010E
       | 010A  7220                           MOVEQ       #20,D1
       | 010C  6002                           BRA.B       0110
       | 010E  7200                           MOVEQ       #00,D1
       | 0110  8081                           OR.L        D1,D0
       | 0112  2140 000A                      MOVE.L      D0,000A(A0)
; 280:     face->facetype = facetype;
       | 0116  1147 0004                      MOVE.B      D7,0004(A0)
; 281: 
; 282:     /* fix that shortest string stuff */
; 283:     if(facecontrol->shortest>strlen)
       | 011A  202D 0304                      MOVE.L      0304(A5),D0
       | 011E  B086                           CMP.L       D6,D0
       | 0120  6F04                           BLE.B       0126
; 284:       facecontrol->shortest = strlen;
       | 0122  2B46 0304                      MOVE.L      D6,0304(A5)
; 285:     
; 286:     /* if there already is a string ending with the same character as we
; 287:        do... */
; 288:     if(facecontrol->exist[ lastchar ])
       | 0126  7000                           MOVEQ       #00,D0
       | 0128  102F 0027                      MOVE.B      0027(A7),D0
       | 012C  4A35 0004                      TST.B       04(A5,D0.W)
       | 0130  670E                           BEQ.B       0140
; 289:       facecontrol->exist[ lastchar ] |= FC_SEVERAL; /* we're not alone */
       | 0132  7200                           MOVEQ       #00,D1
       | 0134  1200                           MOVE.B      D0,D1
       | 0136  7404                           MOVEQ       #04,D2
       | 0138  8435 1004                      OR.B        04(A5,D1.W),D2
       | 013C  1B82 1004                      MOVE.B      D2,04(A5,D1.W)
; 290: 
; 291:     /* set the ->exist flag depending on the flags set for the string */
; 292:     facecontrol->exist[ lastchar ] |=
       | 0140  7200                           MOVEQ       #00,D1
       | 0142  1200                           MOVE.B      D0,D1
; 293:       (flags&ADD_WORDONLY?FACE_WORDONLY:FACE_ANYWHERE)|
       | 0144  0804 0000                      BTST        #0000,D4
       | 0148  57C2                           SEQ         D2
       | 014A  7601                           MOVEQ       #01,D3
       | 014C  9602                           SUB.B       D2,D3
; 294:         (flags&ADD_IMPORTANT?FC_IMPORTANT:0);
       | 014E  0804 0004                      BTST        #0004,D4
       | 0152  6704                           BEQ.B       0158
       | 0154  7408                           MOVEQ       #08,D2
       | 0156  6002                           BRA.B       015A
       | 0158  7400                           MOVEQ       #00,D2
       | 015A  8682                           OR.L        D2,D3
       | 015C  7400                           MOVEQ       #00,D2
       | 015E  1435 1004                      MOVE.B      04(A5,D1.W),D2
       | 0162  2002                           MOVE.L      D2,D0
       | 0164  8083                           OR.L        D3,D0
       | 0166  1B80 1004                      MOVE.B      D0,04(A5,D1.W)
; 295: 
; 296:     /* link ourself to the chain of face-strings */
; 297:     face->next = facecontrol->strings[lastchar%FACETABLE_SIZE];
       | 016A  7000                           MOVEQ       #00,D0
       | 016C  102F 0027                      MOVE.B      0027(A7),D0
       | 0170  7240                           MOVEQ       #40,D1
       | 0172  D281                           ADD.L       D1,D1
       | 0174  6100  0000-XX.1                BSR.W       __CXD33
       | 0178  2001                           MOVE.L      D1,D0
       | 017A  E580                           ASL.L       #2,D0
       | 017C  2200                           MOVE.L      D0,D1
       | 017E  0681 0000 0104                 ADDI.L      #00000104,D1
       | 0184  20B5 1800                      MOVE.L      00(A5,D1.L),(A0)
; 298:     facecontrol->strings[lastchar%FACETABLE_SIZE] = face;
       | 0188  7000                           MOVEQ       #00,D0
       | 018A  102F 0027                      MOVE.B      0027(A7),D0
       | 018E  7240                           MOVEQ       #40,D1
       | 0190  D281                           ADD.L       D1,D1
       | 0192  6100  0000-XX.1                BSR.W       __CXD33
       | 0196  2001                           MOVE.L      D1,D0
       | 0198  E580                           ASL.L       #2,D0
       | 019A  2200                           MOVE.L      D0,D1
       | 019C  0681 0000 0104                 ADDI.L      #00000104,D1
       | 01A2  2B88 1800                      MOVE.L      A0,00(A5,D1.L)
       | 01A6  6004                           BRA.B       01AC
; 299:   }
; 300:   else
; 301:     return ERROR;
       | 01A8  7001                           MOVEQ       #01,D0
       | 01AA  6002                           BRA.B       01AE
; 302:   return OK;
       | 01AC  7000                           MOVEQ       #00,D0
; 303: }
       | 01AE  4CDF 2CFC                      MOVEM.L     (A7)+,D2-D7/A2-A3/A5
       | 01B2  504F                           ADDQ.W      #8,A7
       | 01B4  4E75                           RTS
; 304: 
; 305: struct Face * __inline
; 306: MatchWord(TextStruct *ti,
; 307:           struct Face *face,
; 308:           long col,
; 309:           long firstnspace,
; 310:           long flags)
       | 01B6  200F                           MOVE.L      A7,D0
       | 01B8  90BC 0000 0014                 SUB.L       #00000014,D0
       | 01BE  B0AC  0000-XX.2                CMP.L       ___base(A4),D0
       | 01C2  6500  0000-XX.1                BCS.W       __XCOVF
       | 01C6  514F                           SUBQ.W      #8,A7
       | 01C8  48E7 2F14                      MOVEM.L     D2/D4-D7/A3/A5,-(A7)
       | 01CC  2A2F 0038                      MOVE.L      0038(A7),D5
       | 01D0  2C2F 0034                      MOVE.L      0034(A7),D6
       | 01D4  2E2F 0030                      MOVE.L      0030(A7),D7
       | 01D8  266F 002C                      MOVEA.L     002C(A7),A3
       | 01DC  2A6F 0028                      MOVEA.L     0028(A7),A5
; 311: {
; 312:   if(col<ti->length-1 && !isidentnum(ti->text[ col + 1 ])) {
       | 01E0  202D 0004                      MOVE.L      0004(A5),D0
       | 01E4  5380                           SUBQ.L      #1,D0
       | 01E6  BE80                           CMP.L       D0,D7
       | 01E8  6C00 00C8                      BGE.W       02B2
       | 01EC  2055                           MOVEA.L     (A5),A0
       | 01EE  D1C7                           ADDA.L      D7,A0
       | 01F0  7000                           MOVEQ       #00,D0
       | 01F2  1028 0001                      MOVE.B      0001(A0),D0
       | 01F6  720F                           MOVEQ       #0F,D1
       | 01F8  41EC  0001-01.2                LEA         01.00000001(A4),A0
       | 01FC  C230 0000                      AND.B       00(A0,D0.W),D1
       | 0200  4A01                           TST.B       D1
       | 0202  6600 00AE                      BNE.W       02B2
; 313:     /*
; 314:      * We're at the end of a word, get the entire word and its
; 315:      * checksum
; 316:      */
; 317:     long n;
; 318:     long i = col;
       | 0206  2F47 0020                      MOVE.L      D7,0020(A7)
; 319:     unsigned long hash;
; 320:     n=0;
       | 020A  7800                           MOVEQ       #00,D4
; 321:     do {
; 322:       n++;
       | 020C  5284                           ADDQ.L      #1,D4
; 323:       if(i-1 < 0 ||
       | 020E  202F 0020                      MOVE.L      0020(A7),D0
       | 0212  2200                           MOVE.L      D0,D1
       | 0214  5381                           SUBQ.L      #1,D1
       | 0216  6D1E                           BLT.B       0236
; 324:          !isidentnum(ti->text[ i-1 ]) ) {
       | 0218  2055                           MOVEA.L     (A5),A0
       | 021A  D1C0                           ADDA.L      D0,A0
       | 021C  7200                           MOVEQ       #00,D1
       | 021E  1228 FFFF                      MOVE.B      FFFF(A0),D1
       | 0222  740F                           MOVEQ       #0F,D2
       | 0224  41EC  0001-01.2                LEA         01.00000001(A4),A0
       | 0228  C430 1000                      AND.B       00(A0,D1.W),D2
       | 022C  4A02                           TST.B       D2
       | 022E  6706                           BEQ.B       0236
; 325:         break;
; 326:       }
; 327:       i--;
       | 0230  53AF 0020                      SUBQ.L      #1,0020(A7)
; 328:     } while(1);
       | 0234  60D6                           BRA.B       020C
; 329:     /* word is 'n' bytes long starting at 'i' */
; 330:     if(n<=MAX_STRING_LENGTH) {
       | 0236  7050                           MOVEQ       #50,D0
       | 0238  B880                           CMP.L       D0,D4
       | 023A  6E76                           BGT.B       02B2
; 331:       /*
; 332:        * This isn't longer than maxlength, cause if it is, we won't
; 333:        * have to bother to check the word.
; 334:        */
; 335:       hash = Gethash(&ti->text[i], n);
       | 023C  2055                           MOVEA.L     (A5),A0
       | 023E  D1EF 0020                      ADDA.L      0020(A7),A0
       | 0242  2F04                           MOVE.L      D4,-(A7)
       | 0244  2F08                           MOVE.L      A0,-(A7)
       | 0246  6100 05BE                      BSR.W       0806
       | 024A  504F                           ADDQ.W      #8,A7
       | 024C  2F40 001C                      MOVE.L      D0,001C(A7)
; 336:       do {
; 337:         if(face->flags&FACE_WORDONLY &&
       | 0250  082B 0000 000D                 BTST        #0000,000D(A3)
       | 0256  6752                           BEQ.B       02AA
; 338:            face->hash == hash &&
       | 0258  202B 0006                      MOVE.L      0006(A3),D0
       | 025C  B0AF 001C                      CMP.L       001C(A7),D0
       | 0260  6648                           BNE.B       02AA
; 339:            !memcmp(face->string, &ti->text[i], n) &&
       | 0262  41EB 0016                      LEA         0016(A3),A0
       | 0266  2255                           MOVEA.L     (A5),A1
       | 0268  D3EF 0020                      ADDA.L      0020(A7),A1
       | 026C  2004                           MOVE.L      D4,D0
       | 026E  4A80                           TST.L       D0
       | 0270  6708                           BEQ.B       027A
       | 0272  B109                           CMPM.B      (A1)+,(A0)+
       | 0274  6604                           BNE.B       027A
       | 0276  5380                           SUBQ.L      #1,D0
       | 0278  66F8                           BNE.B       0272
       | 027A  662E                           BNE.B       02AA
; 340:            face->flags & flags) {
       | 027C  2005                           MOVE.L      D5,D0
       | 027E  222B 000A                      MOVE.L      000A(A3),D1
       | 0282  C081                           AND.L       D1,D0
       | 0284  4A80                           TST.L       D0
       | 0286  6722                           BEQ.B       02AA
; 341:              
; 342:           if (face->flags&FACE_1STNONSPC &&
       | 0288  0801 0003                      BTST        #0003,D1
       | 028C  6708                           BEQ.B       0296
; 343:               i != firstnspace)
       | 028E  202F 0020                      MOVE.L      0020(A7),D0
       | 0292  B086                           CMP.L       D6,D0
       | 0294  6614                           BNE.B       02AA
; 344:             continue;
; 345:           printf("-WORDONLY--> %s\n", face->string);
       | 0296  41EB 0016                      LEA         0016(A3),A0
       | 029A  2F08                           MOVE.L      A0,-(A7)
       | 029C  486C  0102-01.2                PEA         01.00000102(A4)
       | 02A0  6100  0000-XX.1                BSR.W       __tinyprintf
; 346:           return face;
       | 02A4  504F                           ADDQ.W      #8,A7
       | 02A6  200B                           MOVE.L      A3,D0
       | 02A8  600A                           BRA.B       02B4
; 347:         }
; 348:       } while(face=face->next);
       | 02AA  204B                           MOVEA.L     A3,A0
       | 02AC  2650                           MOVEA.L     (A0),A3
       | 02AE  200B                           MOVE.L      A3,D0
       | 02B0  669E                           BNE.B       0250
; 349:     }
; 350:   }
; 351:   return NULL;
       | 02B2  7000                           MOVEQ       #00,D0
; 352: }
       | 02B4  4CDF 28F4                      MOVEM.L     (A7)+,D2/D4-D7/A3/A5
       | 02B8  504F                           ADDQ.W      #8,A7
       | 02BA  4E75                           RTS
; 353: 
; 354: struct Face * __inline
; 355: MatchAnyWhere(TextStruct *ti,
; 356:               struct Face *face,
; 357:               long col,
; 358:               long firstnspace,
; 359:               long flags)
       | 02BC  200F                           MOVE.L      A7,D0
       | 02BE  90BC 0000 000C                 SUB.L       #0000000C,D0
       | 02C4  B0AC  0000-XX.2                CMP.L       ___base(A4),D0
       | 02C8  6500  0000-XX.1                BCS.W       __XCOVF
       | 02CC  48E7 0734                      MOVEM.L     D5-D7/A2-A3/A5,-(A7)
       | 02D0  2A2F 002C                      MOVE.L      002C(A7),D5
       | 02D4  2C2F 0028                      MOVE.L      0028(A7),D6
       | 02D8  2E2F 0024                      MOVE.L      0024(A7),D7
       | 02DC  266F 0020                      MOVEA.L     0020(A7),A3
       | 02E0  2A6F 001C                      MOVEA.L     001C(A7),A5
; 360: {
; 361:   do {
; 362:     if(face->flags&FACE_ANYWHERE &&
       | 02E4  082B 0001 000D                 BTST        #0001,000D(A3)
       | 02EA  6754                           BEQ.B       0340
; 363:        col + 1 >= face->strlen &&
       | 02EC  2007                           MOVE.L      D7,D0
       | 02EE  5280                           ADDQ.L      #1,D0
       | 02F0  222B 000E                      MOVE.L      000E(A3),D1
       | 02F4  B081                           CMP.L       D1,D0
       | 02F6  6D48                           BLT.B       0340
; 364:        !memcmp(face->string,
; 365:                &ti->text[ col - face->strlen + 1],
; 366:                face->strlen) &&
       | 02F8  41EB 0016                      LEA         0016(A3),A0
       | 02FC  2007                           MOVE.L      D7,D0
       | 02FE  9081                           SUB.L       D1,D0
       | 0300  2255                           MOVEA.L     (A5),A1
       | 0302  D3C0                           ADDA.L      D0,A1
       | 0304  45E9 0001                      LEA         0001(A1),A2
       | 0308  4A81                           TST.L       D1
       | 030A  6708                           BEQ.B       0314
       | 030C  B10A                           CMPM.B      (A2)+,(A0)+
       | 030E  6604                           BNE.B       0314
       | 0310  5381                           SUBQ.L      #1,D1
       | 0312  66F8                           BNE.B       030C
       | 0314  662A                           BNE.B       0340
; 367:        face->flags&flags) {
       | 0316  2005                           MOVE.L      D5,D0
       | 0318  222B 000A                      MOVE.L      000A(A3),D1
       | 031C  C081                           AND.L       D1,D0
       | 031E  4A80                           TST.L       D0
       | 0320  671E                           BEQ.B       0340
; 368:       if (face->flags&FACE_1STNONSPC &&
       | 0322  0801 0003                      BTST        #0003,D1
       | 0326  6704                           BEQ.B       032C
; 369:           col != firstnspace)
       | 0328  BE86                           CMP.L       D6,D7
       | 032A  6614                           BNE.B       0340
; 370:         continue;
; 371:       printf("-ANYWHERE--> %s\n", face->string);
       | 032C  41EB 0016                      LEA         0016(A3),A0
       | 0330  2F08                           MOVE.L      A0,-(A7)
       | 0332  486C  0114-01.2                PEA         01.00000114(A4)
       | 0336  6100  0000-XX.1                BSR.W       __tinyprintf
; 372:       return face; /* found it! */
       | 033A  504F                           ADDQ.W      #8,A7
       | 033C  200B                           MOVE.L      A3,D0
       | 033E  600A                           BRA.B       034A
; 373:     }
; 374:   } while(face=face->next);
       | 0340  204B                           MOVEA.L     A3,A0
       | 0342  2650                           MOVEA.L     (A0),A3
       | 0344  200B                           MOVE.L      A3,D0
       | 0346  669C                           BNE.B       02E4
; 375:   return NULL; /* none found */
       | 0348  7000                           MOVEQ       #00,D0
; 376: }
       | 034A  4CDF 2CE0                      MOVEM.L     (A7)+,D5-D7/A2-A3/A5
       | 034E  4E75                           RTS
; 377: 
; 378: /*
; 379:  * Search for a specific string in the line from the specific position
; 380:  */
; 381: long __inline FindEnd(TextStruct **ti,
; 382:                       struct FaceControl *fc,
; 383:                       long *count,
; 384:                       char *string2,
; 385:                       long strlen2,
; 386:                       long flags)
       | 0350  200F                           MOVE.L      A7,D0
       | 0352  90BC 0000 001C                 SUB.L       #0000001C,D0
       | 0358  B0AC  0000-XX.2                CMP.L       ___base(A4),D0
       | 035C  6500  0000-XX.1                BCS.W       __XCOVF
       | 0360  594F                           SUBQ.W      #4,A7
       | 0362  48E7 2736                      MOVEM.L     D2/D5-D7/A2-A3/A5-A6,-(A7)
       | 0366  2C2F 003C                      MOVE.L      003C(A7),D6
       | 036A  2E2F 0038                      MOVE.L      0038(A7),D7
       | 036E  266F 002C                      MOVEA.L     002C(A7),A3
       | 0372  2A6F 0028                      MOVEA.L     0028(A7),A5
; 387: {
; 388:   register char *pnt;
; 389:   while(*count+strlen2 <= (*ti)->length ) {
       | 0376  2007                           MOVE.L      D7,D0
       | 0378  206F 0030                      MOVEA.L     0030(A7),A0
       | 037C  2210                           MOVE.L      (A0),D1
       | 037E  D081                           ADD.L       D1,D0
       | 0380  2055                           MOVEA.L     (A5),A0
       | 0382  B0A8 0004                      CMP.L       0004(A0),D0
       | 0386  6E00 0158                      BGT.W       04E0
; 390:     pnt = &(*ti)->text[*count];
       | 038A  2250                           MOVEA.L     (A0),A1
       | 038C  D3C1                           ADDA.L      D1,A1
       | 038E  2449                           MOVEA.L     A1,A2
; 391:     if(flags&FACE_BACKSLASH && '\\' == *pnt) {
       | 0390  0806 0002                      BTST        #0002,D6
       | 0394  6710                           BEQ.B       03A6
       | 0396  1012                           MOVE.B      (A2),D0
       | 0398  745C                           MOVEQ       #5C,D2
       | 039A  B002                           CMP.B       D2,D0
       | 039C  6608                           BNE.B       03A6
; 392:       (*count)+=2;
       | 039E  226F 0030                      MOVEA.L     0030(A7),A1
       | 03A2  5491                           ADDQ.L      #2,(A1)
; 393:       continue;
       | 03A4  60D0                           BRA.B       0376
; 394:     }
; 395: 
; 396:     if(flags&FACE_OBEYIMPOR &&
       | 03A6  0806 0004                      BTST        #0004,D6
       | 03AA  6700 0102                      BEQ.W       04AE
; 397:        fc->exist[ *pnt ] & FC_IMPORTANT) {
       | 03AE  7000                           MOVEQ       #00,D0
       | 03B0  1012                           MOVE.B      (A2),D0
       | 03B2  0833 0003 0004                 BTST        #0003,04(A3,D0.W)
       | 03B8  6700 00F4                      BEQ.W       04AE
; 398:       char hit=FALSE;
       | 03BC  7A00                           MOVEQ       #00,D5
; 399:       struct Face *face = fc->strings[ *pnt % FACETABLE_SIZE];
       | 03BE  7200                           MOVEQ       #00,D1
       | 03C0  1200                           MOVE.B      D0,D1
       | 03C2  2001                           MOVE.L      D1,D0
       | 03C4  7240                           MOVEQ       #40,D1
       | 03C6  D281                           ADD.L       D1,D1
       | 03C8  6100  0000-XX.1                BSR.W       __CXD33
       | 03CC  2001                           MOVE.L      D1,D0
       | 03CE  E580                           ASL.L       #2,D0
       | 03D0  2200                           MOVE.L      D0,D1
       | 03D2  0681 0000 0104                 ADDI.L      #00000104,D1
       | 03D8  2073 1800                      MOVEA.L     00(A3,D1.L),A0
; 400: 
; 401:       if(fc->exist[ *pnt ] & FACE_WORDONLY) {
       | 03DC  7000                           MOVEQ       #00,D0
       | 03DE  1012                           MOVE.B      (A2),D0
       | 03E0  2F48 0020                      MOVE.L      A0,0020(A7)
       | 03E4  0833 0000 0004                 BTST        #0000,04(A3,D0.W)
       | 03EA  6722                           BEQ.B       040E
; 402:         if(face = MatchWord((*ti), face, *count, -1, FACE_IMPORTANT))
       | 03EC  4878 0020                      PEA         0020
       | 03F0  4878 FFFF                      PEA         FFFF
       | 03F4  226F 0038                      MOVEA.L     0038(A7),A1
       | 03F8  2F11                           MOVE.L      (A1),-(A7)
       | 03FA  2F08                           MOVE.L      A0,-(A7)
       | 03FC  2F15                           MOVE.L      (A5),-(A7)
       | 03FE  6100 FDB6                      BSR.W       01B6
       | 0402  4FEF 0014                      LEA         0014(A7),A7
       | 0406  2F40 0020                      MOVE.L      D0,0020(A7)
       | 040A  6702                           BEQ.B       040E
; 403:           hit=TRUE;
       | 040C  7A01                           MOVEQ       #01,D5
; 404:       }
; 405:       if(!hit &&
       | 040E  4A05                           TST.B       D5
       | 0410  6630                           BNE.B       0442
; 406:          fc->exist[ *pnt ] & FACE_ANYWHERE) {
       | 0412  7000                           MOVEQ       #00,D0
       | 0414  1012                           MOVE.B      (A2),D0
       | 0416  0833 0001 0004                 BTST        #0001,04(A3,D0.W)
       | 041C  6724                           BEQ.B       0442
; 407:         /*
; 408:          * We may have matched some word right *now*.
; 409:          * Check all FACE_ANYWHERE strings on the text in the buffer.
; 410:          */
; 411:         if(face = MatchAnyWhere((*ti), face, *count, -1, FACE_IMPORTANT))
       | 041E  4878 0020                      PEA         0020
       | 0422  4878 FFFF                      PEA         FFFF
       | 0426  206F 0038                      MOVEA.L     0038(A7),A0
       | 042A  2F10                           MOVE.L      (A0),-(A7)
       | 042C  2F2F 002C                      MOVE.L      002C(A7),-(A7)
       | 0430  2F15                           MOVE.L      (A5),-(A7)
       | 0432  6100 FE88                      BSR.W       02BC
       | 0436  4FEF 0014                      LEA         0014(A7),A7
       | 043A  2F40 0020                      MOVE.L      D0,0020(A7)
       | 043E  6702                           BEQ.B       0442
; 412:           hit=TRUE;
       | 0440  7A01                           MOVEQ       #01,D5
; 413:       }
; 414:       if(hit && face->strlen2) {
       | 0442  4A05                           TST.B       D5
       | 0444  6768                           BEQ.B       04AE
       | 0446  206F 0020                      MOVEA.L     0020(A7),A0
       | 044A  4AA8 0012                      TST.L       0012(A0)
       | 044E  675E                           BEQ.B       04AE
; 415:         /*
; 416:          * If we found a match and this turns out to be a "starting"
; 417:          * string, find the end of it!
; 418:          */
; 419:         (*count)++; /* pass the ending letter of the init string */
       | 0450  206F 0030                      MOVEA.L     0030(A7),A0
       | 0454  5290                           ADDQ.L      #1,(A0)
; 420:         while(scanline<lastline) {
       | 0456  202C  0000-02.2                MOVE.L      02.00000000(A4),D0
       | 045A  B0AC  0004-02.2                CMP.L       02.00000004(A4),D0
       | 045E  6C4E                           BGE.B       04AE
; 421:           if(FindEnd(ti, fc, count,
       | 0460  206F 0020                      MOVEA.L     0020(A7),A0
       | 0464  226F 0020                      MOVEA.L     0020(A7),A1
       | 0468  D1E9 000E                      ADDA.L      000E(A1),A0
       | 046C  4DE8 0017                      LEA         0017(A0),A6
; 422:                      face->string2+face->strlen, face->strlen2,
; 423:                      face->flags)) {
       | 0470  2F29 000A                      MOVE.L      000A(A1),-(A7)
       | 0474  2F29 0012                      MOVE.L      0012(A1),-(A7)
       | 0478  2F0E                           MOVE.L      A6,-(A7)
       | 047A  2F2F 003C                      MOVE.L      003C(A7),-(A7)
       | 047E  2F0B                           MOVE.L      A3,-(A7)
       | 0480  2F0D                           MOVE.L      A5,-(A7)
       | 0482  6100 FECC                      BSR.W       0350
       | 0486  4FEF 0018                      LEA         0018(A7),A7
       | 048A  4A80                           TST.L       D0
       | 048C  6710                           BEQ.B       049E
; 424:             *count+=face->strlen2;
       | 048E  206F 0020                      MOVEA.L     0020(A7),A0
       | 0492  2028 0012                      MOVE.L      0012(A0),D0
       | 0496  206F 0030                      MOVEA.L     0030(A7),A0
       | 049A  D190                           ADD.L       D0,(A0)
; 425:             break;
       | 049C  6010                           BRA.B       04AE
; 426:           }
; 427:           scanline++;
       | 049E  52AC  0000-02.2                ADDQ.L      #1,02.00000000(A4)
; 428:           (*ti)++;
       | 04A2  7010                           MOVEQ       #10,D0
       | 04A4  D195                           ADD.L       D0,(A5)
; 429:           (*count)=0; /* restart at column 0 */
       | 04A6  206F 0030                      MOVEA.L     0030(A7),A0
       | 04AA  4290                           CLR.L       (A0)
; 430:         }
       | 04AC  60A8                           BRA.B       0456
; 431: 
; 432:       }
; 433:     }
; 434: 
; 435:     if(!memcmp(string2, pnt, strlen2)) {
       | 04AE  2007                           MOVE.L      D7,D0
       | 04B0  204A                           MOVEA.L     A2,A0
       | 04B2  226F 0034                      MOVEA.L     0034(A7),A1
       | 04B6  4A80                           TST.L       D0
       | 04B8  6708                           BEQ.B       04C2
       | 04BA  B308                           CMPM.B      (A0)+,(A1)+
       | 04BC  6604                           BNE.B       04C2
       | 04BE  5380                           SUBQ.L      #1,D0
       | 04C0  66F8                           BNE.B       04BA
       | 04C2  6612                           BNE.B       04D6
; 436:       printf("-FOUNDEND--> %s\n", string2);
       | 04C4  2F2F 0034                      MOVE.L      0034(A7),-(A7)
       | 04C8  486C  0126-01.2                PEA         01.00000126(A4)
       | 04CC  6100  0000-XX.1                BSR.W       __tinyprintf
; 437:       return 1;
       | 04D0  504F                           ADDQ.W      #8,A7
       | 04D2  7001                           MOVEQ       #01,D0
       | 04D4  600C                           BRA.B       04E2
; 438:     }
; 439:     (*count)++;
       | 04D6  206F 0030                      MOVEA.L     0030(A7),A0
       | 04DA  5290                           ADDQ.L      #1,(A0)
; 440:   }
       | 04DC  6000 FE98                      BRA.W       0376
; 441:   return 0; /* next line please */
       | 04E0  7000                           MOVEQ       #00,D0
; 442: }
       | 04E2  4CDF 6CE4                      MOVEM.L     (A7)+,D2/D5-D7/A2-A3/A5-A6
       | 04E6  584F                           ADDQ.W      #4,A7
       | 04E8  4E75                           RTS
; 443: 
; 444: long ScanBuffer(struct FaceControl *fc,
; 445:                 TextStruct *ti,
; 446:                 long lines)
       | 04EA  200F                           MOVE.L      A7,D0
       | 04EC  90BC 0000 0024                 SUB.L       #00000024,D0
       | 04F2  B0AC  0000-XX.2                CMP.L       ___base(A4),D0
       | 04F6  6500  0000-XX.1                BCS.W       __XCOVF
       | 04FA  9EFC 000C                      SUBA.W      #000C,A7
       | 04FE  48E7 2F14                      MOVEM.L     D2/D4-D7/A3/A5,-(A7)
       | 0502  2E2F 0034                      MOVE.L      0034(A7),D7
       | 0506  2A6F 002C                      MOVEA.L     002C(A7),A5
; 447: {
; 448:   struct Face *face;
; 449:   char hit;
; 450:   long firstnspace;
; 451:   char spacestat;
; 452:   register char byte;
; 453:   lastline = lines;
       | 050A  2947  0004-02.2                MOVE.L      D7,02.00000004(A4)
; 454:   for(scanline=0; scanline<lastline; scanline++, ti++) {
       | 050E  42AC  0000-02.2                CLR.L       02.00000000(A4)
       | 0512  202C  0000-02.2                MOVE.L      02.00000000(A4),D0
       | 0516  B0AC  0004-02.2                CMP.L       02.00000004(A4),D0
       | 051A  6C00 014C                      BGE.W       0668
; 455:     spacestat = FALSE;
       | 051E  422F 0024                      CLR.B       0024(A7)
; 456:     firstnspace = 0;
       | 0522  7A00                           MOVEQ       #00,D5
; 457:     if(fc->shortest <= ti->length) {
       | 0524  202D 0304                      MOVE.L      0304(A5),D0
       | 0528  206F 0030                      MOVEA.L     0030(A7),A0
       | 052C  B0A8 0004                      CMP.L       0004(A0),D0
       | 0530  6E00 0128                      BGT.W       065A
; 458:       long count;
; 459:       for(count=fc->shortest-1;
       | 0534  5380                           SUBQ.L      #1,D0
       | 0536  2F40 0020                      MOVE.L      D0,0020(A7)
; 460:           count < ti->length;
       | 053A  202F 0020                      MOVE.L      0020(A7),D0
       | 053E  206F 0030                      MOVEA.L     0030(A7),A0
       | 0542  B0A8 0004                      CMP.L       0004(A0),D0
       | 0546  6C00 0112                      BGE.W       065A
; 461:           count++) {
; 462:         byte = ti->text[ count ];
       | 054A  2250                           MOVEA.L     (A0),A1
       | 054C  D3C0                           ADDA.L      D0,A1
       | 054E  1811                           MOVE.B      (A1),D4
; 463:         if(!spacestat && !isspace(byte)) {
       | 0550  4A2F 0024                      TST.B       0024(A7)
       | 0554  6618                           BNE.B       056E
       | 0556  7200                           MOVEQ       #00,D1
       | 0558  1204                           MOVE.B      D4,D1
       | 055A  43EC  0001-01.2                LEA         01.00000001(A4),A1
       | 055E  0831 0004 1000                 BTST        #0004,00(A1,D1.W)
       | 0564  6608                           BNE.B       056E
; 464:           spacestat = TRUE;
       | 0566  1F7C 0001 0024                 MOVE.B      #01,0024(A7)
; 465:           firstnspace = count;
       | 056C  2A00                           MOVE.L      D0,D5
; 466:         }
; 467:         if( fc->exist[byte] ) {
       | 056E  7200                           MOVEQ       #00,D1
       | 0570  1204                           MOVE.B      D4,D1
       | 0572  4A35 1004                      TST.B       04(A5,D1.W)
       | 0576  6700 00DA                      BEQ.W       0652
; 468:           char code = fc->exist[byte];
       | 057A  7200                           MOVEQ       #00,D1
       | 057C  1204                           MOVE.B      D4,D1
       | 057E  1435 1004                      MOVE.B      04(A5,D1.W),D2
; 469:           face=fc->strings[byte%FACETABLE_SIZE];
       | 0582  7200                           MOVEQ       #00,D1
       | 0584  1204                           MOVE.B      D4,D1
       | 0586  2001                           MOVE.L      D1,D0
       | 0588  7240                           MOVEQ       #40,D1
       | 058A  D281                           ADD.L       D1,D1
       | 058C  6100  0000-XX.1                BSR.W       __CXD33
       | 0590  2001                           MOVE.L      D1,D0
       | 0592  E580                           ASL.L       #2,D0
       | 0594  2200                           MOVE.L      D0,D1
       | 0596  0681 0000 0104                 ADDI.L      #00000104,D1
       | 059C  2675 1800                      MOVEA.L     00(A5,D1.L),A3
; 470:           hit = FALSE;
       | 05A0  7C00                           MOVEQ       #00,D6
; 471:           if(code & FACE_WORDONLY) {
       | 05A2  1F42 001F                      MOVE.B      D2,001F(A7)
       | 05A6  0802 0000                      BTST        #0000,D2
       | 05AA  671E                           BEQ.B       05CA
; 472:             if(face = MatchWord(ti, face, count, firstnspace, ~0))
       | 05AC  4878 FFFF                      PEA         FFFF
       | 05B0  2F05                           MOVE.L      D5,-(A7)
       | 05B2  2F2F 0028                      MOVE.L      0028(A7),-(A7)
       | 05B6  2F0B                           MOVE.L      A3,-(A7)
       | 05B8  2F08                           MOVE.L      A0,-(A7)
       | 05BA  6100 FBFA                      BSR.W       01B6
       | 05BE  2640                           MOVEA.L     D0,A3
       | 05C0  4FEF 0014                      LEA         0014(A7),A7
       | 05C4  200B                           MOVE.L      A3,D0
       | 05C6  6702                           BEQ.B       05CA
; 473:               hit=TRUE;
       | 05C8  7C01                           MOVEQ       #01,D6
; 474:           }
; 475:           if(!hit &&
       | 05CA  4A06                           TST.B       D6
       | 05CC  6626                           BNE.B       05F4
; 476:              code & FACE_ANYWHERE) {
       | 05CE  0802 0001                      BTST        #0001,D2
       | 05D2  6720                           BEQ.B       05F4
; 477:             /*
; 478:              * We may have matched some word right *now*.
; 479:              * Check all FACE_ANYWHERE strings on the text in the buffer.
; 480:              */
; 481:             if(face = MatchAnyWhere(ti, face, count, firstnspace, ~0))
       | 05D4  4878 FFFF                      PEA         FFFF
       | 05D8  2F05                           MOVE.L      D5,-(A7)
       | 05DA  2F2F 0028                      MOVE.L      0028(A7),-(A7)
       | 05DE  2F0B                           MOVE.L      A3,-(A7)
       | 05E0  2F2F 0040                      MOVE.L      0040(A7),-(A7)
       | 05E4  6100 FCD6                      BSR.W       02BC
       | 05E8  2640                           MOVEA.L     D0,A3
       | 05EA  4FEF 0014                      LEA         0014(A7),A7
       | 05EE  200B                           MOVE.L      A3,D0
       | 05F0  6702                           BEQ.B       05F4
; 482:               hit=TRUE;
       | 05F2  7C01                           MOVEQ       #01,D6
; 483:           }
; 484:           if(hit && face->strlen2) {
       | 05F4  4A06                           TST.B       D6
       | 05F6  675A                           BEQ.B       0652
       | 05F8  4AAB 0012                      TST.L       0012(A3)
       | 05FC  6754                           BEQ.B       0652
; 485:             /*
; 486:              * If we found a match and this turns out to be a "starting"
; 487:              * string, find the end of it!
; 488:              */
; 489:             count++; /* pass the ending letter of the init string */
       | 05FE  52AF 0020                      ADDQ.L      #1,0020(A7)
; 490:             while(scanline<lastline) {
       | 0602  202C  0000-02.2                MOVE.L      02.00000000(A4),D0
       | 0606  B0AC  0004-02.2                CMP.L       02.00000004(A4),D0
       | 060A  6C46                           BGE.B       0652
; 491:               if(FindEnd(&ti, fc, &count,
       | 060C  204B                           MOVEA.L     A3,A0
       | 060E  D1EB 000E                      ADDA.L      000E(A3),A0
       | 0612  43E8 0017                      LEA         0017(A0),A1
; 492:                          face->string2+face->strlen, face->strlen2,
; 493:                          face->flags)) {
       | 0616  2F2B 000A                      MOVE.L      000A(A3),-(A7)
       | 061A  2F2B 0012                      MOVE.L      0012(A3),-(A7)
       | 061E  2F09                           MOVE.L      A1,-(A7)
       | 0620  486F 002C                      PEA         002C(A7)
       | 0624  2F0D                           MOVE.L      A5,-(A7)
       | 0626  486F 0044                      PEA         0044(A7)
       | 062A  6100 FD24                      BSR.W       0350
       | 062E  4FEF 0018                      LEA         0018(A7),A7
       | 0632  4A80                           TST.L       D0
       | 0634  670C                           BEQ.B       0642
; 494:                 count+=face->strlen2-1;
       | 0636  202B 0012                      MOVE.L      0012(A3),D0
       | 063A  5380                           SUBQ.L      #1,D0
       | 063C  D1AF 0020                      ADD.L       D0,0020(A7)
; 495:                 break;
       | 0640  6010                           BRA.B       0652
; 496:               }
; 497:               scanline++;
       | 0642  52AC  0000-02.2                ADDQ.L      #1,02.00000000(A4)
; 498:               ti++;
       | 0646  7010                           MOVEQ       #10,D0
       | 0648  D1AF 0030                      ADD.L       D0,0030(A7)
; 499:               count=0; /* restart at column 0 */
       | 064C  42AF 0020                      CLR.L       0020(A7)
; 500:             }
       | 0650  60B0                           BRA.B       0602
; 501: 
; 502:           }
; 503:         }
; 504:       }
       | 0652  52AF 0020                      ADDQ.L      #1,0020(A7)
       | 0656  6000 FEE2                      BRA.W       053A
; 505:     }
; 506:   }
       | 065A  52AC  0000-02.2                ADDQ.L      #1,02.00000000(A4)
       | 065E  7010                           MOVEQ       #10,D0
       | 0660  D1AF 0030                      ADD.L       D0,0030(A7)
       | 0664  6000 FEAC                      BRA.W       0512
; 507:   return 0;
       | 0668  7000                           MOVEQ       #00,D0
; 508: }
       | 066A  4CDF 28F4                      MOVEM.L     (A7)+,D2/D4-D7/A3/A5
       | 066E  DEFC 000C                      ADDA.W      #000C,A7
       | 0672  4E75                           RTS
; 509: 
; 510: void InitFace(struct FaceControl *facecontrol)
       | 0674  200F                           MOVE.L      A7,D0
       | 0676  90BC 0000 000C                 SUB.L       #0000000C,D0
       | 067C  B0AC  0000-XX.2                CMP.L       ___base(A4),D0
       | 0680  6500  0000-XX.1                BCS.W       __XCOVF
       | 0684  2F0D                           MOVE.L      A5,-(A7)
       | 0686  2A6F 0008                      MOVEA.L     0008(A7),A5
; 511: {
; 512:   memset(facecontrol, 0, sizeof(struct FaceControl));
       | 068A  7000                           MOVEQ       #00,D0
       | 068C  204D                           MOVEA.L     A5,A0
       | 068E  323C 0311                      MOVE.W      #0311,D1
       | 0692  10C0                           MOVE.B      D0,(A0)+
       | 0694  51C9 FFFC                      DBF         D1,0692
; 513:   facecontrol->shortest=9999;
       | 0698  2B7C 0000 270F 0304            MOVE.L      #0000270F,0304(A5)
; 514: }
       | 06A0  2A5F                           MOVEA.L     (A7)+,A5
       | 06A2  4E75                           RTS
; 515: 
; 516: int main(int argc, char **argv)
       | 06A4  200F                           MOVE.L      A7,D0
       | 06A6  90BC 0000 0760                 SUB.L       #00000760,D0
       | 06AC  B0AC  0000-XX.2                CMP.L       ___base(A4),D0
       | 06B0  6500  0000-XX.1                BCS.W       __XCOVF
       | 06B4  9EFC 0744                      SUBA.W      #0744,A7
; 517: {
; 518:   TextStruct textinfo[]={
; 519:     XX("  #define killerninjainthewoods --- /*\n"),
; 520: 	XX("    */ for int ---- \n"),
; 521:     XX("int oldret;\n"),
; 522:     XX("for(;undoantal;)\n"),
; 523:     XX("  Command(Storage, DO_NOTHING|NO_HOOK, 0, NULL, NULL);\n"),
; 524:     XX("tempprg=Malloc(tempprglen);\n"),
; 525:     XX("else {\n"),
; 526:     XX("  ninja++;\n"),
; 527:     XX("}\n"),
; 528:     XX("\n"),
; 529:     XX("if (tempprg) {\n"),
; 530:     XX("  tempprg[0]='\0';\n"),
; 531:     XX("while(!(hook->flags&HOOK_ABS)) {\n"),
; 532:     XX("  /* this is not an \"absolute\" hook! */\n"),
; 533:     XX("  register char *tempprgend;\n"),
; 534:     XX("  len=hook->func->format? strlen(hook->func->format) : 0;\n"),
; 535:     XX("}\n"),
; 536:     XX("Sprintf(tempprg, \"%64\\\"s/* */ (\", name); /*for only*/\n"),
; 537:     XX("tempprgend=tempprg+65;\n")
; 538:   };
       | 06B8  41EC  0324-01.2                LEA         01.00000324(A4),A0
       | 06BC  43EF 0614                      LEA         0614(A7),A1
       | 06C0  704C                           MOVEQ       #4C,D0
       | 06C2  E588                           LSL.L       #2,D0
       | 06C4  6100  0000-XX.1                BSR.W       __CXAMEMCPY
; 539:   struct FaceControl facecontrol;
; 540:   struct FaceType facetype[256];
; 541: 
; 542:   InitFace(&facecontrol);
       | 06C8  486F 0002                      PEA         0002(A7)
       | 06CC  61A6                           BSR.B       0674
; 543: 
; 544:   PrintText((TextStruct *)&textinfo[0], sizeof(textinfo)/sizeof(textinfo[0]));
       | 06CE  4878 0013                      PEA         0013
       | 06D2  486F 061C                      PEA         061C(A7)
       | 06D6  6100 0176                      BSR.W       084E
; 545: 
; 546:   AddFaceString(&facecontrol, 0, "while", 5, NULL, 0, ADD_WORDONLY);
       | 06DA  4878 0001                      PEA         0001
       | 06DE  7000                           MOVEQ       #00,D0
       | 06E0  2F00                           MOVE.L      D0,-(A7)
       | 06E2  42A7                           CLR.L       -(A7)
       | 06E4  4878 0005                      PEA         0005
       | 06E8  486C  0454-01.2                PEA         01.00000454(A4)
       | 06EC  2F00                           MOVE.L      D0,-(A7)
       | 06EE  486F 0026                      PEA         0026(A7)
       | 06F2  6100 F90C                      BSR.W       0000
; 547:   AddFaceString(&facecontrol, 0, "if",    2, NULL, 0, ADD_WORDONLY);
       | 06F6  4878 0001                      PEA         0001
       | 06FA  7000                           MOVEQ       #00,D0
       | 06FC  2F00                           MOVE.L      D0,-(A7)
       | 06FE  42A7                           CLR.L       -(A7)
       | 0700  4878 0002                      PEA         0002
       | 0704  486C  045A-01.2                PEA         01.0000045A(A4)
       | 0708  2F00                           MOVE.L      D0,-(A7)
       | 070A  486F 0042                      PEA         0042(A7)
       | 070E  6100 F8F0                      BSR.W       0000
       | 0712  4FEF 0044                      LEA         0044(A7),A7
; 548:   AddFaceString(&facecontrol, 0, "int",   3, NULL, 0, ADD_WORDONLY);
       | 0716  4878 0001                      PEA         0001
       | 071A  7000                           MOVEQ       #00,D0
       | 071C  2F00                           MOVE.L      D0,-(A7)
       | 071E  42A7                           CLR.L       -(A7)
       | 0720  4878 0003                      PEA         0003
       | 0724  486C  045E-01.2                PEA         01.0000045E(A4)
       | 0728  2F00                           MOVE.L      D0,-(A7)
       | 072A  486F 001A                      PEA         001A(A7)
       | 072E  6100 F8D0                      BSR.W       0000
; 549:   AddFaceString(&facecontrol, 0, "for",   3, NULL, 0, ADD_WORDONLY);
       | 0732  4878 0001                      PEA         0001
       | 0736  7000                           MOVEQ       #00,D0
       | 0738  2F00                           MOVE.L      D0,-(A7)
       | 073A  42A7                           CLR.L       -(A7)
       | 073C  4878 0003                      PEA         0003
       | 0740  486C  0462-01.2                PEA         01.00000462(A4)
       | 0744  2F00                           MOVE.L      D0,-(A7)
       | 0746  486F 0036                      PEA         0036(A7)
       | 074A  6100 F8B4                      BSR.W       0000
; 550:   AddFaceString(&facecontrol, 0, "do",    2, NULL, 0, ADD_WORDONLY);
       | 074E  4878 0001                      PEA         0001
       | 0752  7000                           MOVEQ       #00,D0
       | 0754  2F00                           MOVE.L      D0,-(A7)
       | 0756  42A7                           CLR.L       -(A7)
       | 0758  4878 0002                      PEA         0002
       | 075C  486C  0466-01.2                PEA         01.00000466(A4)
       | 0760  2F00                           MOVE.L      D0,-(A7)
       | 0762  486F 0052                      PEA         0052(A7)
       | 0766  6100 F898                      BSR.W       0000
       | 076A  4FEF 0054                      LEA         0054(A7),A7
; 551:   AddFaceString(&facecontrol, 0, "else",  4, NULL, 0, ADD_WORDONLY);
       | 076E  4878 0001                      PEA         0001
       | 0772  7000                           MOVEQ       #00,D0
       | 0774  2F00                           MOVE.L      D0,-(A7)
       | 0776  42A7                           CLR.L       -(A7)
       | 0778  4878 0004                      PEA         0004
       | 077C  486C  046A-01.2                PEA         01.0000046A(A4)
       | 0780  2F00                           MOVE.L      D0,-(A7)
       | 0782  486F 001A                      PEA         001A(A7)
       | 0786  6100 F878                      BSR.W       0000
; 552:   AddFaceString(&facecontrol, 0, "/*",    2, "*/", 2, ADD_IMPORTANT);
       | 078A  4878 0010                      PEA         0010
       | 078E  7002                           MOVEQ       #02,D0
       | 0790  2F00                           MOVE.L      D0,-(A7)
       | 0792  486C  0474-01.2                PEA         01.00000474(A4)
       | 0796  2F00                           MOVE.L      D0,-(A7)
       | 0798  486C  0470-01.2                PEA         01.00000470(A4)
       | 079C  42A7                           CLR.L       -(A7)
       | 079E  486F 0036                      PEA         0036(A7)
       | 07A2  6100 F85C                      BSR.W       0000
; 553:   AddFaceString(&facecontrol, 0, "\"",    1, "\"", 1, ADD_BACKSLASH);
       | 07A6  4878 0004                      PEA         0004
       | 07AA  7001                           MOVEQ       #01,D0
       | 07AC  2F00                           MOVE.L      D0,-(A7)
       | 07AE  486C  047A-01.2                PEA         01.0000047A(A4)
       | 07B2  2F00                           MOVE.L      D0,-(A7)
       | 07B4  486C  0478-01.2                PEA         01.00000478(A4)
       | 07B8  42A7                           CLR.L       -(A7)
       | 07BA  486F 0052                      PEA         0052(A7)
       | 07BE  6100 F840                      BSR.W       0000
       | 07C2  4FEF 0054                      LEA         0054(A7),A7
; 554:   AddFaceString(&facecontrol, 0, "#",     1, "\n", 1, ADD_BACKSLASH|
; 555:                                                       ADD_1STNONSPC|
; 556:                                                       ADD_OBEYIMPO);
       | 07C6  4878 000E                      PEA         000E
       | 07CA  7001                           MOVEQ       #01,D0
       | 07CC  2F00                           MOVE.L      D0,-(A7)
       | 07CE  486C  047E-01.2                PEA         01.0000047E(A4)
       | 07D2  2F00                           MOVE.L      D0,-(A7)
       | 07D4  486C  047C-01.2                PEA         01.0000047C(A4)
       | 07D8  42A7                           CLR.L       -(A7)
       | 07DA  486F 001A                      PEA         001A(A7)
       | 07DE  6100 F820                      BSR.W       0000
; 557: 
; 558:   FaceReport(&facecontrol);
       | 07E2  486F 001E                      PEA         001E(A7)
       | 07E6  6100 00C0                      BSR.W       08A8
; 559:   
; 560:   ScanBuffer(&facecontrol,
; 561:              &textinfo[0],
; 562:              sizeof(textinfo)/sizeof(textinfo[0]));
       | 07EA  4878 0013                      PEA         0013
       | 07EE  486F 0638                      PEA         0638(A7)
       | 07F2  486F 002A                      PEA         002A(A7)
       | 07F6  6100 FCF2                      BSR.W       04EA
; 563: 
; 564:   return 0;
       | 07FA  4FEF 002C                      LEA         002C(A7),A7
       | 07FE  7000                           MOVEQ       #00,D0
; 565: }
       | 0800  DEFC 0744                      ADDA.W      #0744,A7
       | 0804  4E75                           RTS
; 566: 
; 567: /**********************************************************************
; 568:  *
; 569:  * int Gethash();
; 570:  *
; 571:  * Return the hash number for the name received as argument.
; 572:  *
; 573:  *****/
; 574: 
; 575: static unsigned long Gethash(char *name, long len)
       | 0806  BFEC  0000-XX.2                CMPA.L      ___base(A4),A7
       | 080A  6500  0000-XX.1                BCS.W       __XCOVF
       | 080E  48E7 0304                      MOVEM.L     D6-D7/A5,-(A7)
       | 0812  2E2F 0014                      MOVE.L      0014(A7),D7
       | 0816  2A6F 0010                      MOVEA.L     0010(A7),A5
; 576: {
; 577:   unsigned long hash=0;
       | 081A  7C00                           MOVEQ       #00,D6
; 578:   while(len--)
       | 081C  2007                           MOVE.L      D7,D0
       | 081E  5387                           SUBQ.L      #1,D7
       | 0820  4A80                           TST.L       D0
       | 0822  6722                           BEQ.B       0846
; 579:     hash=(hash<<1)+(*name++ +1)+(hash&(1<<31)?-2000000000:0);
       | 0824  2006                           MOVE.L      D6,D0
       | 0826  D080                           ADD.L       D0,D0
       | 0828  7200                           MOVEQ       #00,D1
       | 082A  121D                           MOVE.B      (A5)+,D1
       | 082C  D081                           ADD.L       D1,D0
       | 082E  0806 001F                      BTST        #001F,D6
       | 0832  6708                           BEQ.B       083C
       | 0834  223C 88CA 6C00                 MOVE.L      #88CA6C00,D1
       | 083A  6002                           BRA.B       083E
       | 083C  7200                           MOVEQ       #00,D1
       | 083E  D081                           ADD.L       D1,D0
       | 0840  2C00                           MOVE.L      D0,D6
       | 0842  5286                           ADDQ.L      #1,D6
       | 0844  60D6                           BRA.B       081C
; 580:   return hash;
       | 0846  2006                           MOVE.L      D6,D0
; 581: }
       | 0848  4CDF 20C0                      MOVEM.L     (A7)+,D6-D7/A5
       | 084C  4E75                           RTS
; 582: 
; 583: 
; 584: void PrintText(TextStruct *textinfo, int lines)
       | 084E  200F                           MOVE.L      A7,D0
       | 0850  90BC 0000 000C                 SUB.L       #0000000C,D0
       | 0856  B0AC  0000-XX.2                CMP.L       ___base(A4),D0
       | 085A  6500  0000-XX.1                BCS.W       __XCOVF
       | 085E  48E7 0304                      MOVEM.L     D6-D7/A5,-(A7)
       | 0862  2E2F 0014                      MOVE.L      0014(A7),D7
       | 0866  2A6F 0010                      MOVEA.L     0010(A7),A5
; 585: {
; 586:   int i;
; 587:   printf("TEXT:\n");
       | 086A  486C  0480-01.2                PEA         01.00000480(A4)
       | 086E  6100  0000-XX.1                BSR.W       __writes
       | 0872  584F                           ADDQ.W      #4,A7
; 588:   for(i=0; i<lines; i++, textinfo++) {
       | 0874  7C00                           MOVEQ       #00,D6
       | 0876  BC87                           CMP.L       D7,D6
       | 0878  6C28                           BGE.B       08A2
; 589:     printf("%2d %s", i, textinfo->text);
       | 087A  2F15                           MOVE.L      (A5),-(A7)
       | 087C  2F06                           MOVE.L      D6,-(A7)
       | 087E  486C  0488-01.2                PEA         01.00000488(A4)
       | 0882  6100  0000-XX.1                BSR.W       _printf
       | 0886  4FEF 000C                      LEA         000C(A7),A7
; 590:     textinfo->length = strlen(textinfo->text);
       | 088A  2055                           MOVEA.L     (A5),A0
       | 088C  2008                           MOVE.L      A0,D0
       | 088E  4A18                           TST.B       (A0)+
       | 0890  66FC                           BNE.B       088E
       | 0892  5388                           SUBQ.L      #1,A0
       | 0894  91C0                           SUBA.L      D0,A0
       | 0896  2B48 0004                      MOVE.L      A0,0004(A5)
; 591:   }
       | 089A  5286                           ADDQ.L      #1,D6
       | 089C  DAFC 0010                      ADDA.W      #0010,A5
       | 08A0  60D4                           BRA.B       0876
; 592: }
       | 08A2  4CDF 20C0                      MOVEM.L     (A7)+,D6-D7/A5
       | 08A6  4E75                           RTS
; 593: 
; 594: void FaceReport(struct FaceControl *facecontrol)
       | 08A8  200F                           MOVE.L      A7,D0
       | 08AA  90BC 0000 000C                 SUB.L       #0000000C,D0
       | 08B0  B0AC  0000-XX.2                CMP.L       ___base(A4),D0
       | 08B4  6500  0000-XX.1                BCS.W       __XCOVF
       | 08B8  48E7 0314                      MOVEM.L     D6-D7/A3/A5,-(A7)
       | 08BC  2A6F 0014                      MOVEA.L     0014(A7),A5
; 595: {
; 596:   int i;
; 597:   int count=0;
       | 08C0  7C00                           MOVEQ       #00,D6
; 598:   struct Face *face;
; 599:   for(i=0; i<256; i++) {
       | 08C2  7E00                           MOVEQ       #00,D7
       | 08C4  0C87 0000 0100                 CMPI.L      #00000100,D7
       | 08CA  6C38                           BGE.B       0904
; 600:     if(facecontrol->exist[i]) {
       | 08CC  4A35 7804                      TST.B       04(A5,D7.L)
       | 08D0  672E                           BEQ.B       0900
; 601:       face = facecontrol->strings[i];
       | 08D2  2007                           MOVE.L      D7,D0
       | 08D4  E580                           ASL.L       #2,D0
       | 08D6  2200                           MOVE.L      D0,D1
       | 08D8  0681 0000 0104                 ADDI.L      #00000104,D1
       | 08DE  2675 1800                      MOVEA.L     00(A5,D1.L),A3
; 602:       count = 0;
       | 08E2  7C00                           MOVEQ       #00,D6
; 603:       while(face) {
       | 08E4  200B                           MOVE.L      A3,D0
       | 08E6  6708                           BEQ.B       08F0
; 604:         ++count;
       | 08E8  5286                           ADDQ.L      #1,D6
; 605:         face=face->next;
       | 08EA  204B                           MOVEA.L     A3,A0
       | 08EC  2650                           MOVEA.L     (A0),A3
; 606:       }
       | 08EE  60F4                           BRA.B       08E4
; 607:       printf("Strings end with '%c' %d times\n", i, count);
       | 08F0  2F06                           MOVE.L      D6,-(A7)
       | 08F2  2F07                           MOVE.L      D7,-(A7)
       | 08F4  486C  0490-01.2                PEA         01.00000490(A4)
       | 08F8  6100  0000-XX.1                BSR.W       _printf
       | 08FC  4FEF 000C                      LEA         000C(A7),A7
; 608:     }
; 609:   }
       | 0900  5287                           ADDQ.L      #1,D7
       | 0902  60C0                           BRA.B       08C4
; 610: }
       | 0904  4CDF 28C0                      MOVEM.L     (A7)+,D6-D7/A3/A5
       | 0908  4E75                           RTS
       | 090A  4E71                           NOP

SECTION 01 "__MERGED" 000004B0 BYTES
0000 20 20 20 20 20 20 20 20 20 20 10 10 20 20 10 20           ..  . 
0010 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20                 
0020 20 10 40 40 40 40 40 40 40 40 40 40 40 40 40 40  .@@@@@@@@@@@@@@
0030 40 88 88 88 88 88 88 88 88 88 88 40 40 40 40 40 @..........@@@@@
0040 40 40 81 81 81 81 81 81 01 01 01 01 01 01 01 01 @@..............
0050 01 01 01 01 01 01 01 01 01 01 01 01 40 40 40 40 ............@@@@
0060 44 40 82 82 82 82 82 82 02 02 02 02 02 02 02 02 D@..............
0070 02 02 02 02 02 02 02 02 02 02 02 02 40 40 40 40 ............@@@@
OFFSETS 0080 THROUGH 00FF CONTAIN ZERO
0100 00 00 2D 57 4F 52 44 4F 4E 4C 59 2D 2D 3E 20 25 ..-WORDONLY--> %
0110 73 0A 00 00 2D 41 4E 59 57 48 45 52 45 2D 2D 3E s...-ANYWHERE-->
0120 20 25 73 0A 00 00 2D 46 4F 55 4E 44 45 4E 44 2D  %s...-FOUNDEND-
0130 2D 3E 20 25 73 0A 00 00 20 20 23 64 65 66 69 6E -> %s...  #defin
0140 65 20 6B 69 6C 6C 65 72 6E 69 6E 6A 61 69 6E 74 e killerninjaint
0150 68 65 77 6F 6F 64 73 20 2D 2D 2D 20 2F 2A 0A 00 hewoods --- /*..
0160 20 20 20 20 2A 2F 20 66 6F 72 20 69 6E 74 20 2D     */ for int -
0170 2D 2D 2D 20 0A 00 69 6E 74 20 6F 6C 64 72 65 74 --- ..int oldret
0180 3B 0A 00 00 66 6F 72 28 3B 75 6E 64 6F 61 6E 74 ;...for(;undoant
0190 61 6C 3B 29 0A 00 20 20 43 6F 6D 6D 61 6E 64 28 al;)..  Command(
01A0 53 74 6F 72 61 67 65 2C 20 44 4F 5F 4E 4F 54 48 Storage, DO_NOTH
01B0 49 4E 47 7C 4E 4F 5F 48 4F 4F 4B 2C 20 30 2C 20 ING|NO_HOOK, 0, 
01C0 4E 55 4C 4C 2C 20 4E 55 4C 4C 29 3B 0A 00 74 65 NULL, NULL);..te
01D0 6D 70 70 72 67 3D 4D 61 6C 6C 6F 63 28 74 65 6D mpprg=Malloc(tem
01E0 70 70 72 67 6C 65 6E 29 3B 0A 00 00 65 6C 73 65 pprglen);...else
01F0 20 7B 0A 00 20 20 6E 69 6E 6A 61 2B 2B 3B 0A 00  {..  ninja++;..
0200 7D 0A 00 00 0A 00 69 66 20 28 74 65 6D 70 70 72 }.....if (temppr
0210 67 29 20 7B 0A 00 20 20 74 65 6D 70 70 72 67 5B g) {..  tempprg[
0220 30 5D 3D 27 00 27 3B 0A 00 00 77 68 69 6C 65 28 0]='.';...while(
0230 21 28 68 6F 6F 6B 2D 3E 66 6C 61 67 73 26 48 4F !(hook->flags&HO
0240 4F 4B 5F 41 42 53 29 29 20 7B 0A 00 20 20 2F 2A OK_ABS)) {..  /*
0250 20 74 68 69 73 20 69 73 20 6E 6F 74 20 61 6E 20  this is not an 
0260 22 61 62 73 6F 6C 75 74 65 22 20 68 6F 6F 6B 21 "absolute" hook!
0270 20 2A 2F 0A 00 00 20 20 72 65 67 69 73 74 65 72  */...  register
0280 20 63 68 61 72 20 2A 74 65 6D 70 70 72 67 65 6E  char *tempprgen
0290 64 3B 0A 00 20 20 6C 65 6E 3D 68 6F 6F 6B 2D 3E d;..  len=hook->
02A0 66 75 6E 63 2D 3E 66 6F 72 6D 61 74 3F 20 73 74 func->format? st
02B0 72 6C 65 6E 28 68 6F 6F 6B 2D 3E 66 75 6E 63 2D rlen(hook->func-
02C0 3E 66 6F 72 6D 61 74 29 20 3A 20 30 3B 0A 00 00 >format) : 0;...
02D0 7D 0A 00 00 53 70 72 69 6E 74 66 28 74 65 6D 70 }...Sprintf(temp
02E0 70 72 67 2C 20 22 25 36 34 5C 22 73 2F 2A 20 2A prg, "%64\"s/* *
02F0 2F 20 28 22 2C 20 6E 61 6D 65 29 3B 20 2F 2A 66 / (", name); /*f
0300 6F 72 20 6F 6E 6C 79 2A 2F 0A 00 00 74 65 6D 70 or only*/...temp
0310 70 72 67 65 6E 64 3D 74 65 6D 70 70 72 67 2B 36 prgend=tempprg+6
0320 35 3B 0A 00 5;..
0324 00000138-01 01.00000138
OFFSETS 0328 THROUGH 0333 CONTAIN ZERO
0334 00000160-01 01.00000160
OFFSETS 0338 THROUGH 0343 CONTAIN ZERO
0344 00000176-01 01.00000176
OFFSETS 0348 THROUGH 0353 CONTAIN ZERO
0354 00000184-01 01.00000184
OFFSETS 0358 THROUGH 0363 CONTAIN ZERO
0364 00000196-01 01.00000196
OFFSETS 0368 THROUGH 0373 CONTAIN ZERO
0374 000001CE-01 01.000001CE
OFFSETS 0378 THROUGH 0383 CONTAIN ZERO
0384 000001EC-01 01.000001EC
OFFSETS 0388 THROUGH 0393 CONTAIN ZERO
0394 000001F4-01 01.000001F4
OFFSETS 0398 THROUGH 03A3 CONTAIN ZERO
03A4 00000200-01 01.00000200
OFFSETS 03A8 THROUGH 03B3 CONTAIN ZERO
03B4 00000204-01 01.00000204
OFFSETS 03B8 THROUGH 03C3 CONTAIN ZERO
03C4 00000206-01 01.00000206
OFFSETS 03C8 THROUGH 03D3 CONTAIN ZERO
03D4 00000216-01 01.00000216
OFFSETS 03D8 THROUGH 03E3 CONTAIN ZERO
03E4 0000022A-01 01.0000022A
OFFSETS 03E8 THROUGH 03F3 CONTAIN ZERO
03F4 0000024C-01 01.0000024C
OFFSETS 03F8 THROUGH 0403 CONTAIN ZERO
0404 00000276-01 01.00000276
OFFSETS 0408 THROUGH 0413 CONTAIN ZERO
0414 00000294-01 01.00000294
OFFSETS 0418 THROUGH 0423 CONTAIN ZERO
0424 000002D0-01 01.000002D0
OFFSETS 0428 THROUGH 0433 CONTAIN ZERO
0434 000002D4-01 01.000002D4
OFFSETS 0438 THROUGH 0443 CONTAIN ZERO
0444 0000030C-01 01.0000030C
OFFSETS 0448 THROUGH 044F CONTAIN ZERO
0450 00 00 00 00 77 68 69 6C 65 00 69 66 00 00 69 6E ....while.if..in
0460 74 00 66 6F 72 00 64 6F 00 00 65 6C 73 65 00 00 t.for.do..else..
0470 2F 2A 00 00 2A 2F 00 00 22 00 22 00 23 00 0A 00 /*..*/..".".#...
0480 54 45 58 54 3A 0A 00 00 25 32 64 20 25 73 00 00 TEXT:...%2d %s..
0490 53 74 72 69 6E 67 73 20 65 6E 64 20 77 69 74 68 Strings end with
04A0 20 27 25 63 27 20 25 64 20 74 69 6D 65 73 0A 00  '%c' %d times..

SECTION 02 "__MERGED" 00000008 BYTES
