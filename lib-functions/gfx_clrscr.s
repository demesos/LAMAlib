; code for C64, hires-bitmap at $2000
;
; clear the hires screen quick and dirty
; Y register is expected to hold bg/fg color

.export _gfx_clrscr_sr:=gfx_clrscr

.include "gfx_include.inc"

gfx_clrscr: 
	;bg/fg color in y
	ldx #00
loop:	
	lda #00
	.repeat 32, I
	sta bitmap+I*$100,x
	.endrepeat
	tya
	sta screen,x
	sta screen+$100,x
	sta screen+$200,x
	sta screen+$300,x	;this overwrites sprite pointers, sry
	inx
	bne loop
	rts
