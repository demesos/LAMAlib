;returns if a dot on a hires bitmap at the given position is set
;X-Position: X and Carry (Carry to be set for coordinates > 255)
;Y-Position: Y
;return value in A (either 0 or 1)

.include "gfx_include.inc"
.importzp _llzp_word1

.export _gfx_pget_sr

_gfx_pget_sr:
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
	and gfx_mask_or,x
	bne not_empty
	  lda #00
	  rts
not_empty:
	lda #1
	rts
