.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"
.include "../LAMAlib-ROMfunctions.inc"

.export _printa_sr

; prints the 8bit number in A to the screen
; code from codebase64.org

_printa_sr:
	ldy #$2f
	ldx #$3a
	sec
:	iny
	sbc #100
	bcs :-
:	dex
	adc #10
	bmi :-
	adc #$2f

	cpy #$30
	if eq
	  cpx #$31
	  if ge
	    pha
	    txa
	    jsr CHROUT
	    pla
	  endif	
	else
	  pha
	  tya
	  jsr CHROUT
	  txa
	  jsr CHROUT
	  pla
	endif
	jmp CHROUT
