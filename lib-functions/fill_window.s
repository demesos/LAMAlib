; Fills a defined window with the character stored in A

.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"

.import _window_x1,_window_y1,_window_x2,_window_y2
.import _TEXTCOLOR_ADDR,_PTRSCRHI
.import _mul40_tbl_lo,_mul40_tbl_hi
mul40_lo:=_mul40_tbl_lo
mul40_hi:=_mul40_tbl_hi

.export _fill_window

_fill_window:
	sta fill_char
	ldy _window_y1
	lda _window_x1
	clc
	adc mul40_lo,y
	sta scr_clr_ptr2
	sta col_clr_ptr2
	php
	lda mul40_hi,y
	adc _PTRSCRHI
	sta scr_clr_ptr2+1
	plp	;recover carry bit
	lda mul40_hi,y
	adc #$d8
	sta col_clr_ptr2+1

	lda _window_x2
	sec
	sbc _window_x1
	sta line_width2

	dey	;because we need to clear one line more

clear_lines:
line_width2=*+1
	ldx #00

clear_line:
fill_char=*+1
	lda #$20
scr_clr_ptr2=*+1
	sta $400,x
	lda _TEXTCOLOR_ADDR
col_clr_ptr2=*+1	
	sta $d800,x

	dex
	bpl clear_line

	clc
	lda scr_clr_ptr2
	adc #40
	if cs
	  inc scr_clr_ptr2+1
	  inc col_clr_ptr2+1
	endif
	sta scr_clr_ptr2
	sta col_clr_ptr2

	iny
	cpy _window_y2
	bcc clear_lines
	rts