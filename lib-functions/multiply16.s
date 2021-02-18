.importzp _llzp_word1,_llzp_word2

multiplier:= _llzp_word1
multiplicand:=_llzp_word2

.code

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
