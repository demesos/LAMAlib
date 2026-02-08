; waits until all keys are released, VIC20 version

.export _waitkeyreleased_vic20_sr := keychk

KB_COLS = $9120    ; Write to select keyboard columns
KB_ROWS = $9121    ; Read keyboard row state

keychk:
	lda #00
	sta KB_COLS
	lda KB_ROWS
        cmp #$ff
        bne keychk 
	lda KB_COLS
	bne keychk 	;if the ISR changed the register while we did our check we have to redo
	rts