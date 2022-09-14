.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"
.include "../LAMAlib-ROMfunctions.inc"

.export _printax_sr

; prints the 16bit number in AX to the screen
; code by Wil, 2021

_printax_sr:
;tenthousands
	ldy #'1'
	sty cmp_val
	dey
	cmpax #30000
	if ge
	  ldy #'3'
	  ;sec	alreay set
	  sbcax #30000
	endif
	cmpax #20000	
	if ge
	  iny
	  iny
	  ;sec	alreay set
	  sbcax #20000
	  jmp tenthousands_done
	endif
	cmpax #10000	
	if ge
	  iny
	  ;sec	alreay set
	  sbcax #10000
	endif
tenthousands_done:
	cpy #'1'
	if ge
	  store AX
	  tya
	  jsr CHROUT
	  poke cmp_val,$30
	  restore AX
	endif
;thousands	
	ldy #'0'
	cmpax #5000
	if ge
	  ldy #'5'
	  ;sec	alreay set
	  sbcax #5000
	endif
	cmpax #3000	
	if ge
	  iny
	  iny
	  iny
	  ;sec	alreay set
	  sbcax #3000
	endif
	cmpax #2000	
	if ge
	  iny
	  iny
	  ;sec	alreay set
	  sbcax #2000
	  jmp thousands_done
	endif
	cmpax #1000	
	if ge
	  iny
	  ;sec	alreay set
	  sbcax #1000
	endif
thousands_done:
	cpy cmp_val
	if ge
	  store AX
	  tya
	  jsr CHROUT
	  poke cmp_val,$30
	  restore AX
	endif
;hundreds
	ldy #'0'
	cmpax #500
	if ge
	  ldy #'5'
	  ;sec	alreay set
	  sbcax #500
	endif
	cmpax #300	
	if ge
	  iny
	  iny
	  iny
	  ;sec	alreay set
	  sbcax #300
	endif
	cmpax #200	
	if ge
	  iny
	  iny
	  ;sec	alreay set
	  sbcax #200
	  jmp hundreds_done
	endif
	cmpax #100	
	if ge
	  iny
	  ;sec	alreay set
	  sbcax #100
	endif
hundreds_done:
	cpy cmp_val
	if ge
	  store AX
	  tya
	  jsr CHROUT
	  poke cmp_val,$30
	  restore AX
	endif
;10s
	ldy #'0'
	cmp #50
	if ge
	  ldy #'5'
	  ;sec	alreay set
	  sbc #50
	endif
	cmp #30	
	if ge
	  iny
	  iny
	  iny
	  ;sec	alreay set
	  sbc #30
	endif
	cmp #20	
	if ge
	  iny
	  iny
	  ;sec	alreay set
	  sbc #20
	  jmp tens_done
	endif
	cmp #10
	if ge
	  iny
	  ;sec	alreay set
	  sbc #10
	endif
tens_done:
	cpy cmp_val
	if ge
	  store A
	  tya
	  jsr CHROUT
	  poke cmp_val,$30
	  restore A
	endif
;unit digit
	clc
	adc #'0'
	jmp CHROUT  ;jmp b/c tail call optimized

cmp_val: .byte $00