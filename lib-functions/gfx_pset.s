;plots a dot onto a hires bitmap
;X-Position: X and Carry (Carry to be set for coordinates > 255)
;Y-Position: Y

.include "gfx_include.inc"
.importzp _llzp_word1

.export _gfx_pset_sr:=plot

plot:
	lda gfx_ytablelo,y
	sta _llzp_word1

	lda gfx_ytablehi,y
	adc #00
	sta _llzp_word1+1

	ldy gfx_xtablelo,x

	txa
	and #7
	tax

	lda (_llzp_word1),y
	ora gfx_mask_or,x
	sta (_llzp_word1),y
	rts
