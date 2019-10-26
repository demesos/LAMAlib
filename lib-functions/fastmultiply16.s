zpbase=$22        ;not used by cc65, used by BASIC for temporary ptrs and results, so do we
multiplier = zpbase	;initialized by A/X
multiplicand = zpbase+2

.include "../LAMAlib-macros16.inc"

.export fastmul16sr:=multiply16
.exportzp fastmul16arg:=multiplicand

multiply16:
	eor #$ff
	sta  multiplier
	txa
	eor #$ff
	sta  multiplier+1

	ldax #00
	ldy #8		;repeat for one byte
loop1:
	lsr multiplier
	bcs skip1
	;clc		;carry is already cleared by previous comparison
	adcax multiplicand

skip1:	asl16 multiplicand
	dey
	bne loop1
	pha	;low byte of result is already fixed
	txa

	ldy #8		;repeat for one byte
loop2:
	lsr multiplier+1
	bcs skip2
	;clc
	adc multiplicand+1

skip2:	asl multiplicand+1
	dey
	bne loop2
	tax	;high byte
	pla	;get low byte
	rts
