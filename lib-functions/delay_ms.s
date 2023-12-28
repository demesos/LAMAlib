.export _delay_ms_sr

; busy waiting loop for AX milliseconds, not cycle exact
.proc _delay_ms_sr
loop1:  cmp #00 	;test A, sets the carry
	bne declo
	dex
	bmi done 
declo:	;sec		;carry was already set by cmp #00
	sbc #01
	ldy #133	;inner loop should run bit less than 1ms
inner_loop:
	dey
	beq loop1
	jmp inner_loop	;using a jmp loop to avoid different timing with bne over page border
done:
	rts
.endproc