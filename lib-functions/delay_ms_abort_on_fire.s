.export _delay_ms_abort_on_fire_sr 

; busy waiting loop for AX milliseconds, not cycle exact
; if the loop ran through, the carry flag is set
; if firebutton was detected, the loop is aborted and the carry flag is cleared
.proc _delay_ms_abort_on_fire_sr
	tay
	lda $dc00
	and $dc01
	and #$10
	sta last_fire_state
	tya		;undo the tay 
loop1: 	tay
	bne declo
	dex
	bmi done 
declo:	dey
	lda $dc00
	and $dc01
	and #$10
	bne no_fire
last_fire_state=*+1
	eor #$af
	beq still_fire
	clc
	rts
no_fire:
	sta last_fire_state	
still_fire:
	tya		;move back counter low byte to A
	ldy #129	;inner loop value is adjusted for average CPU speed with badlines
inner_loop:
	dey
	beq loop1
	jmp inner_loop	;using a jmp loop to avoid different timing with bne over page border
done:	sec		;regular exit with no button abort
	rts

.endproc