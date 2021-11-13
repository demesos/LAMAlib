; Initialization of tables for fast plot algorithm

.include "gfx_include.inc"

.export _gfx_init_sr := gfx_init

gfx_init:  
	ldx #00
loopx:	txa
	and #$F8
	sta gfx_xtablelo,x
	inx
	bne loopx
	
	lda #<(bitmap)
	sta gfx_ytablelo
	ldy #>(bitmap)
	sty gfx_ytablehi

	;ldx #00 ;x is already 0 because of previous loop
	clc

loopy:	
	txa 
	and #07
	eor #07
	beq next_charline
	lda gfx_ytablelo,x
	clc
	adc #01
	inx
	sta gfx_ytablelo,x
	tya
	sta gfx_ytablehi,x
	bne loopy	;unconditional jump

next_charline:
	lda gfx_ytablelo,x
	adc #<313
	inx
	sta gfx_ytablelo,x
	tya
	adc #>313
	sta gfx_ytablehi,x
	tay
	cpx #200
	bne loopy

; initialize gfx_masks
	lda #01
	ldx #07
loopm:  sta gfx_mask_or,x
	eor #$ff
	sta gfx_mask_andi,x
	eor #$ff
	asl
	dex
	bpl loopm

	rts	