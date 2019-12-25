; Initialization of tables for fast plot algorithm

.include "gfx_include.inc"

.export gfx_init_sr := gfx_init

gfx_init:
	ldx #00
loopx:	txa
	and #$F8
	sta gfx_xtable,x
	inx
	bne loopx
	
	lda #<bitmap
	sta gfx_ytablelow
	lda #>bitmap
	sta gfx_ytablehi

	ldx #00 ;x is already 0 because of previous loop
	clc
loopy:	
	lda gfx_ytablehi,x
	tay
	lda gfx_ytablelow,x
	adc #<320
	inx
	sta gfx_ytablelow,x
	tya
	adc #>320
	sta gfx_ytablehi,x
	cpx #200
	bne loopy

	lda #01
	ldx #07
loopm:  sta gfx_masks,x
	asl
	dex
	bpl loopm

	rts	