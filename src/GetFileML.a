;
; FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
;
; This file is open-source software. Please refer to the file LEGAL
; for the licensing conditions and responsibilities.
;


	CSECT	FindCharProg
	xdef	_FindChar

_FindChar:	; D1 = char
		; D2 = length
		; A0 = memory

	moveq	#0,d0
fc_loop:
	subq.l	#1,d2
	bmi.s	fc_end
	cmp.b	(a0)+,d1
	bne.s	fc_loop
	addq.l	#1,d0
	bra.s	fc_loop
fc_end:
	rts

 END

