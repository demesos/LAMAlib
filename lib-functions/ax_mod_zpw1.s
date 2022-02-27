.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"

;calculate AX mod the 16 bit value in _llzp_word1 and return result in AX

.importzp _llzp_word1
.export _ax_mod_zpw1_sr

_ax_mod_zpw1_sr:
	ldy #00
	bit _llzp_word1+1
	bmi done_shifting2
shloop2: iny
	asl _llzp_word1
	rol _llzp_word1+1
	bpl shloop2
done_shifting2:
modloop2:
	cmpax _llzp_word1
	if cs
	  sbcax _llzp_word1
	endif
	lsr _llzp_word1+1
	ror _llzp_word1
	dey
	bpl modloop2

	rts