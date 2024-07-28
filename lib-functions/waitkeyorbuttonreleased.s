; waits until all keys and fire buttons are released


.export _waitkeyorbuttonreleased_sr := chkinput

chkinput:
	lda #$10
	sta $dc00
	and $dc00
	and $dc01
	beq chkinput
keychk:
	lda #00
	sta $dc00
	lda $dc01
        cmp #$ff
        bne keychk 
	lda $dc00
	bne keychk 	;if the ISR changed the register while we did our check we have to redo
chkinput_done:
	lda #$7f
	sta $dc00	;restore dc00 setting
	rts

