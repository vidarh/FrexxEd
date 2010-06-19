;
; FrexxEd - Copyright (C) 1998, Daniel Stenberg and Kjell Ericson
;
; This file is open-source software. Please refer to the file LEGAL
; for the licensing conditions and responsibilities.
;


	CSECT	MultProg
	xdef	_MultML
	xdef	_BigMult

_MultML:				; Multiplicera utför D0=a*b/c.
					; Return: resultatet
	move.l	d1,a			; First input	a
	move.l	d2,b			;		b
	move.l	d3,c			;		c
	movem.l	d4-d5,-(a7)
	clr.l	res
	clr.l	res+4
	clr.l	svar

	move.w	a+2,d0
	move.w	b+2,d1
	mulu	d1,d0
	move.l	d0,res+4
	move.w	a+2,d0
	beq.s	mmla1
	move.w	b,d1
	beq.s	mmla2
	mulu	d1,d0
	add.l	d0,res+2
mmla1:	move.w	a,d0
	beq.s	mmla2
	move.w	b,d1
	beq.s	mmla2
	mulu	d1,d0
	add.l	d0,res
mmla2:

	move.l	#7,d3
	move.l	c,d1
	beq.s	mmlaend
	move.l	res,d4
	move.l	res+4,d5
	roxl.l	#1,d5
	roxl.l	#1,d4

mmlo3:	cmp.l	d4,d1
	bhi.s	mmla3
	bset	d3,svar
	sub.l	d1,d4
mmla3:	roxl.l	#1,d5
	roxl.l	#1,d4
	dbf	d3,mmlo3

	move.l	#7,d3
mmlo4:	cmp.l	d4,d1
	bhi.s	mmla4
	bset	d3,svar+1
	sub.l	d1,d4
mmla4:	roxl.l	#1,d5
	roxl.l	#1,d4
	dbf	d3,mmlo4

	move.l	#7,d3
mmlo5:	cmp.l	d4,d1
	bhi.s	mmla5
	bset	d3,svar+2
	sub.l	d1,d4
mmla5:	roxl.l	#1,d5
	roxl.l	#1,d4
	dbf	d3,mmlo5

	move.l	#7,d3
mmlo6:	cmp.l	d4,d1
	bhi.s	mmla6
	bset	d3,svar+3
	sub.l	d1,d4
mmla6:	roxl.l	#1,d5
	roxl.l	#1,d4
	dbf	d3,mmlo6



mmlaend:
	move.l	svar,d0
	movem.l	(a7)+,d4-d5
	rts

a:	dc.l	0,0
b:	dc.l	0,0
c:	dc.l	0,0
res:	dc.l	0,0
svar:	dc.l	0,0


; 103: void __regargs BigMult(char *result, char *fakt1, char *fakt2)
; 104: {
_BigMult:
        MOVEM.L     D2/D5-D7/A2-A3/A5,-(A7)
        MOVEA.L     A1,A3
        MOVEA.L     A0,A5
; 105:   unsigned long x;
; 106:   unsigned int i, k;
; 107: 
; 108:   x=0;
        MOVEQ       #$00,D7
; 109:   for (k=0; k<ANTAL_BYTES; k++) {
        MOVEQ       #$00,D5
a228:   MOVEQ       #$41,D0
        CMP.L       D0,D5
        BCC.B       a25C
; 110:     for (i=0; i<=k; i++)
        MOVEQ       #$00,D6
a230:   CMP.L       D5,D6
        BHI.B       a250
; 111:       x+=(fakt1[i]*fakt2[k-i]);
        MOVE.L      D5,D0
        SUB.L       D6,D0
        MOVEQ       #$00,D1
        MOVE.B      00(A3,D6.L),D1
        MOVEQ       #$00,D2
        MOVE.B      00(A2,D0.L),D2
        mulu	d1,d2
        add.l	d2,d7
;        MOVE.L      D2,D0
;        JSR         __CXM33(PC)
;        ADD.L       D0,D7
        ADDQ.L      #1,D6
        BRA.B       a230
; 112:     result[k]=(unsigned char)x;
a250:   MOVE.L      D7,D0
        MOVE.B      D0,00(A5,D5.L)
; 113:     x>>=8;
        LSR.L       #8,D7
; 114:   }
        ADDQ.L      #1,D5
        BRA.B       a228
; 115:   for (; k<2*ANTAL_BYTES; k++) {
a25C:   MOVEQ       #$41,D0
        ADD.L       D0,D0
        CMP.L       D0,D5
        BCC.B       a298
; 116:     for (i=k-ANTAL_BYTES+1; i<ANTAL_BYTES; i++)
        MOVE.L      D5,D6
        MOVEQ       #$40,D0
        SUB.L       D0,D6
a26A:   MOVEQ       #$41,D0
        CMP.L       D0,D6
        BCC.B       a28C
; 117:       x+=(fakt1[i]*fakt2[k-i]);
        MOVE.L      D5,D0
        SUB.L       D6,D0
        MOVEQ       #00,D1
        MOVE.B      00(A3,D6.L),D1
        MOVEQ       #00,D2
        MOVE.B      00(A2,D0.L),D2
        mulu	d1,d2
        add.l	d2,d7
;        MOVE.L      D2,D0
;        JSR         __CXM33(PC)
;        ADD.L       D0,D7
        ADDQ.L      #1,D6
        BRA.B       a26A
; 118:     result[k]=(unsigned char)x;
a28C:   MOVE.L      D7,D0
        MOVE.B      D0,00(A5,D5.L)
; 119:     x >>=8;
        LSR.L       #8,D7
; 120:   }
        ADDQ.L      #1,D5
        BRA.B       a25C
; 121: }
a298:   MOVEM.L     (A7)+,D2/D5-D7/A2-A3/A5
        RTS
 END

; 103: void __regargs BigMult(char *result, char *fakt1, char *fakt2)
;                             a0          , a1         , a2
_BigMult:
        MOVEM.L     D2/D5-D7/A2-A3/A5,-(A7)

	clr.l	d0	;x=d0
	clr.l	d1	;k=d1
	moveq	#$20,d6	;temp i d6
kloop1:
	move.l	d1,d2	;i=d2
	move.l	a1,a3	;temp fakt1
	move.l	a2,a5	;temp fakt2
	add.l	d1,a5
iloop1:	move	-(a5),d7
	mulu	(a3)+,d7
	add.l	d7,d0
	dbf	d2,iloop1
	move	d0,(a0)+
	lsr	#8,d0
	addq	#1,d1
	cmp	d1,d6
	bcs	kloop1

	add.l	d6,a1
	add.l	d6,a1
	add.l	d6,a2
	add.l	d6,a2
kloop2:
	move.l	#$20,d2	;i=d2
	addq	#1,a1
	move.l	a1,a3	;temp fakt1
	move.l	a2,a5	;temp fakt2
	add.l	d1,a5
iloop2:	move	-(a5),d7
	mulu	(a3)+,d7
	add.l	d7,d0
	dbf	d2,iloop2
	move	d0,(a0)+
	lsr	#8,d0
	addq	#1,d1
	cmp	d1,d6
	bcs	kloop2

a298:   MOVEM.L     (A7)+,D2/D5-D7/A2-A3/A5
        RTS

 END

