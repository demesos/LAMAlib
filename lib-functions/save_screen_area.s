;-----------------------------------------------------------------------
; save_screen_area
; Saves a screen area to memory
;
; Apr-2024 V0.11
; Wilfried Elmenreich
; License: The Unlicense
;-----------------------------------------------------------------------

.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"
.include "../LAMAlib-ROMfunctions.inc"

.export _save_screen_window_x1,_save_screen_window_y1,_save_screen_window_x2,_save_screen_window_y2
.export _save_screen_area,_save_screen_addr

.import _mul40_tbl_lo,_mul40_tbl_hi

_save_screen_area:
	stax _save_screen_addr
_save_screen_window_y1=*+1
	lda #$42
	jsr store_byte
_save_screen_window_x1=*+1
	lda #$42
	jsr store_byte
_save_screen_window_x2=*+1
	lda #$42
	jsr store_byte
	lda _save_screen_window_y2
	jsr store_byte

	ldy _save_screen_window_y1
	ldx _save_screen_window_x1
	txa
	clc
	adc _mul40_tbl_lo,y
	sta scrptr_src
	sta colptr_src
	php
	lda _mul40_tbl_hi,y
	tax
	adc PTRSCRHI
	sta scrptr_src+1
	plp	;recover carry bit
	txa
	adc #$d8
	sta colptr_src+1

	lda _save_screen_window_x2
	sec
	sbc _save_screen_window_x1	;subtract _save_screen_window_x1-1 because of carry	
	tay
	sty line_width

	ldx _save_screen_window_y1
	dex
L2:
line_width=*+1
	ldy #$42
L1:
scrptr_src=*+1
	lda $affe,y
	jsr store_byte
colptr_src=*+1
	lda $affe,y
	jsr store_byte
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
_save_screen_window_y2=*+1
	cpx #$42
	bne L2
	
	rts

store_byte:
_save_screen_addr=*+1
	sta $affe
	inc16 _save_screen_addr
	rts


