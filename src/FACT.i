;
; FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
;
; This file is open-source software. Please refer to the file LEGAL
; for the licensing conditions and responsibilities.
;
	* FACT.i

	IFND	EXEC_TYPES_I
	INCLUDE	"exec/types.i"
	ENDC


 STRUCTURE FACT,0
  ULONG fc_flags	; The eight most important flags	(char)
  ULONG fc_xflags	; Extended flags			(int)
  ULONG fc_strings	; String to output			(char *)
  ULONG fc_length	; Length of string (max 255)		(char)
  ULONG fc_delimit	; Delimiter char			(char)
  ULONG fc_cases	; Opposit case				(char)
  BYTE	fc_newlinechar	; Charachter to use when to make a newline
  BYTE	fc_tabchar	; Charachter to use when to make a tab
  BYTE  fc_newlinecheck ; Number of newline characters that exists (256=255)
  BYTE  fc_lowestnewline; Lowest newline character
  ULONG fc_name		; Name of FACT
  ULONG fc_next		; Next structure
  ULONG fc_extra	; extra strings.
  LABEL FACT_SIZEOF

FACT_EOF_LENGTH		EQU	0+fc_extra
FACT_EOF_STRING		EQU	4+fc_extra
FACT_NO_EOL_LENGTH	EQU	8+fc_extra
FACT_NO_EOL_STRING	EQU	12+fc_extra
FACT_FOLDED_CHAR_LENGTH	EQU	16+fc_extra
FACT_FOLDED_CHAR_STRING	EQU	20+fc_extra
FACT_FOLDUP_CHAR_LENGTH	EQU	24+fc_extra
FACT_FOLDUP_CHAR_STRING	EQU	28+fc_extra
FACT_NOFOLD_CHAR_LENGTH	EQU	32+fc_extra
FACT_NOFOLD_CHAR_STRING	EQU	36+fc_extra
FACT_EXTRA_STRINGS	EQU	5


			* Only bits
fact_NEWLINE EQU 0	* A new line char
fact_TAB EQU	 1	* A tab char
fact_STRING EQU	 2	* Ouput a string

