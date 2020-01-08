;plots a dot onto a hires bitmap
;X-Position: A and Carry (Carry to be set for coordinates > 255)
;Y-Position: Y

.include "gfx_include.inc"

.export _gfx_plot_sr:=plot

plot:
	lda #<bitmap
	sta POINT
	lda #>bitmap
	adc #0
	sta POINT+1
	rts
