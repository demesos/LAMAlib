; waits until all keys are released

.export _waitkeyreleased_sr := keychk

keychk:
	lda #00
	sta $dc00
	lda $dc01
        cmp #$ff
        bne keychk 
	rts