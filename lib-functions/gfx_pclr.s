;deletes a dot on a hires bitmap
;X-Position: X and Carry (Carry to be set for coordinates > 255)
;Y-Position: Y

.include "gfx_include.inc"
.importzp _llzp_word1

.export _gfx_pclr_sr:=unplot

unplot:
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
	and gfx_mask_andi,x
	sta (_llzp_word1),y
	rts
