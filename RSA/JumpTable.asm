
	CSECT	JumpTables

	xref	_String2Clip

REVISION equ 2
start:	dc.l	$11223344
public_modulo:
	dc.b	$2d,$9f,$d3,$bd,$04,$b3,$e1,$d1
	dc.b	$c8,$5b,$52,$a8,$12,$7a,$46,$9c
	dc.b	$2f,$e2,$18,$cc,$09,$7e,$75,$df
	dc.b	$3f,$ad,$0c,$5e,$3a,$27,$f9,$61
	dc.b	$77,$57,$fe,$0f,$69,$fe,$30,$48
	dc.b	$2f,$ca,$f4,$cb,$98,$72,$de,$80
	dc.b	$d8,$98,$45,$03,$94,$3d,$ca,$74
	dc.b	$8e,$d4,$52,$0f,$ff,$ff,$ff,$ff
	dc.w	0

check:	dc.l	$55AA1234+REVISION
Start:	jmp	_String2Clip(PC)

userid:	dc.l	$61f0ff1c
username:
	dc.b	"Jesper Svennevid",0

align
namecopy:	dc.b	$54,$7a,$93,$9b,$9b,$b3,$6c,$aa,$d8,$d2,$e6,$f1,$f3,$0f,$0d,0

 END
