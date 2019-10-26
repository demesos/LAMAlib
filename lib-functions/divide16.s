zpbase=$22        ;not used by cc65, used by BASIC for temporary ptrs and results, so do we
dividend = zpbase ;initialized with values from A/X
divisor = zpbase+2	
remainder = zpbase+4
result = dividend ;given back in A/X

.include "../LAMAlib-macros16.inc"

.export div16sr:=divide16
.exportzp div16arg:=divisor, div16rem:=remainder

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