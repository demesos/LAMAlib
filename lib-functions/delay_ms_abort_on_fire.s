.include "../LAMAlib-structured.inc"

.export _delay_ms_abort_on_fire_sr 

DEBOUNCE_TIME=16

; busy waiting loop for AX milliseconds, not cycle exact
; if the loop ran through, the carry flag is set
; if firebutton was detected, the loop is aborted and the carry flag is cleared
.proc _delay_ms_abort_on_fire_sr
	ldy #DEBOUNCE_TIME
	sty debounce_ctr
loop1: 	tay
	bne declo
	dex
	bmi done 
declo:	dey
	lda $dc00
	and $dc01
	and #$10
	bne no_fire
debounce_ctr=*+1
	lda #$af
	bpl fire_was_already_pressed
	clc
	rts
no_fire:
	bit debounce_ctr
	if pl
	  dec debounce_ctr
	endif
fire_was_already_pressed:
	tya		;move back counter low byte to A
	ldy #129	;inner loop value is adjusted for average CPU speed with badlines
inner_loop:
	dey
	beq loop1
	jmp inner_loop	;using a jmp loop to avoid different timing with bne over page border
done:	sec		;regular exit with no button abort
	rts

.endproc