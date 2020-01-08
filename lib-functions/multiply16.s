zpbase=$22        ;not used by cc65, used by BASIC for temporary ptrs and results, so do we
multiplier = zpbase	;initialized by A/X
multiplicand = zpbase+2

.include "../LAMAlib-macros16.inc"

.export _mul16_sr:=multiply16
.exportzp _mul16_arg:=multiplicand

; shortest implementation
multiply16:
	stax multiplier
	ldax #00
	ldy #16		;repeat for each bit
loop:
	lsr16 multiplier
	bcc skip
	clc
	adcax multiplicand

skip:	asl16 multiplicand
	dey
	bne loop
	rts
