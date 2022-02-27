.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"

;calculate A mod Y and return result in A

.importzp _llzp_byte1
.export _a_mod_y_sr

_a_mod_y_sr:
	ldx #0
	store A
	tya 
	bmi done_shifting
shloop:	inx
	asl
	bpl shloop
done_shifting:
	sta _llzp_byte1
	restore A
modloop:
	cmp _llzp_byte1
	if cs
	  sbc _llzp_byte1
	endif
	lsr _llzp_byte1
	dex
	bpl modloop
	rts