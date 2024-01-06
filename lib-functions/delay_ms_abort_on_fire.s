.export _delay_ms_abort_on_fire_sr 

; busy waiting loop for AX milliseconds, not cycle exact
; if the loop ran through, the carry flag is set
; if firebutton was detected, the loop is aborted and the carry flag is cleared
.proc _delay_ms_abort_on_fire_sr
loop1:  cmp #00 	;test A, sets the carry
	bne declo
	dex
	bmi done 
declo:	;sec		;carry was already set by cmp #00
	sbc #01

	pha
	lda $dc00
	and $dc01
	and #$10
	bne no_fire
	pla
	clc
done:
	rts
no_fire:
	pla
	ldy #130	;inner loop value is adjusted for average CPU speed with badlines and the 
inner_loop:
	dey
	beq loop1
	jmp inner_loop	;using a jmp loop to avoid different timing with bne over page border
.endproc