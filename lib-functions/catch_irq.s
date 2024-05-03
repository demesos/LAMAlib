; Routine to handle the IRQ if I/O and Kernal ROM are banked out
.export _catchirq_sr

;value of memory 1 during interrupts
mem1=$36

_catchirq_sr: 
	;do what Kernal routine $FF48 would have done
	pha
	txa
	pha
	tya
	pha

	;push the data for a fake irq to the stack

	lda #>endirq
	pha
	lda #<endirq
	pha

	lda 1
	pha	;dummy for status reg
	pha	;value of peek(1) will be in A
	pha	;just a placeholder for X value
	pha	;just a placeholder for Y value

	lda #mem1
	sta 1	;now the intended memory configuration is visible

	jmp ($314)	;this goes to $ea31 or a custom routine

endirq:
	sta 1	;recover original memory configuration state
	pla
	tay
	pla
	tax
	pla
	rti	;this ends the IRQ for good
