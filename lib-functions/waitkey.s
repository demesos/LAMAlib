; waits for any key to be pressed

.export _waitkey_sr := keychk

keychk:
	lda #00
	sta $dc00
	lda $dc01
        cmp #$ff
        beq keychk 
	rts
