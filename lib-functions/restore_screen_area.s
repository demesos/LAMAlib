;-----------------------------------------------------------------------
; restore_screen_area
; Restores screen area from memory
;
; Apr-2024 V0.1
; Wilfried Elmenreich
; License: The Unlicense
;-----------------------------------------------------------------------

.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"

.export _restore_screen_area

.import _mul40_tbl_lo,_mul40_tbl_hi,_PTRSCRHI

_restore_screen_area:
	stax storeaddr
	jsr get_stored_byte	;y1
	sta y1_here
	tay
	jsr get_stored_byte	;x1
	sta x1_here
	clc
	adc _mul40_tbl_lo,y		
	sta scrptr_src
	sta colptr_src
	php
	lda _mul40_tbl_hi,y
	tax
	adc _PTRSCRHI
	sta scrptr_src+1
	plp	;recover carry bit
	txa
	adc #$d8
	sta colptr_src+1

	jsr get_stored_byte	;x2
	sec
x1_here=*+1
	sbc #$42	;subtract _window_x1-1 because of carry	
	tay
	sty line_width

	jsr get_stored_byte	;y2
	sta y2_here
y1_here=*+1
	ldx #$42
	dex
L2:
line_width=*+1
	ldy #$42
L1:
	jsr get_stored_byte
scrptr_src=*+1
	sta $affe,y
	jsr get_stored_byte
colptr_src=*+1
	sta $affe,y
	dey
	bpl L1

	lda scrptr_src
	clc
	adc #40
	sta scrptr_src
	sta colptr_src
	if cs
	  inc scrptr_src+1
	  inc colptr_src+1
	endif


	inx
y2_here=*+1
	cpx #$42
	bne L2
	
	rts


get_stored_byte:
storeaddr=*+1
	lda $affe
	inc16 storeaddr
	rts