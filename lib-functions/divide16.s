.importzp _llzp_word1,_llzp_word2,_llzp_word3

dividend = _llzp_word1  ;initialized with values from A/X
divisor = _llzp_word2	;to be set by calling program with the number we are dividing by
remainder = _llzp_word3
result = dividend       ;given back in A/X

.code

.include "../LAMAlib-macros16.inc"

.export _div16_sr:=divide16
.exportzp _div16_arg:=divisor, _div16_rem:=remainder

divide16:
	sta dividend
	stx dividend+1 	
	lda #0	        ;preset remainder to 0
	sta remainder
	sta remainder+1
	ldy #16	        ;repeat for each bit

divloop:
	asl16 dividend	;dividend lb & hb*2, msb -> Carry
	rol16 remainder	;remainder lb & hb * 2 + msb from carry
	lda remainder
	sec
	sbc divisor	;substract divisor to see if it fits in
	tax	        ;lb result -> X, for we may need it later
	lda remainder+1
	sbc divisor+1
	bcc :+		;if carry=0 then divisor didn't fit in yet

	sta remainder+1	;else save subtraction result as new remainder
	stx remainder	
	inc result	;increment result cause divisor fit in 1 times

:	dey
	bne divloop
	lda result
	ldx result+1	
	rts