.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"
.include "../LAMAlib-systemaddresses.inc"

.export _print_fillchars_ax_sr

; prints the fillchars for a right-aligned decimal number
; does not print the number itself
; AX number to print
; Y fillchar
; Function leaves AX unchanged
; code by Wil, 2024

_print_fillchars_ax_sr:
	sty fillchar_value
	ldy #0
	cmpax #10
	if cc
	  ldy #4
	else
	  cmpax #100
	  if cc
	    ldy #3
	  else
	    cmpax #1000
	    if cc
	      ldy #2
	    else
              cmpax #10000
              if cc
                ldy #1
              endif
	    endif
          endif
	endif
	pha
fillchar_value=*+1
	lda #32
	jmp loop_entry
fillchar_loop:
	jsr CHROUT
loop_entry:
	dey
	bpl fillchar_loop
	pla
	rts