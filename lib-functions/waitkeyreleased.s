; waits until all keys are released

.export _waitkeyreleased_sr := keychk

keychk:
	lda #00
	sta $dc00
	lda $dc01
        cmp #$ff
        bne keychk 
	lda $dc00
	bne keychk 	;if the ISR changed the register while we did our check we have to redo
	rts