; waits for any key to be pressed, VIC20 version

.export _waitkey_vic20_sr := keychk

KB_COLS = $9120    ; Write to select keyboard columns
KB_ROWS = $9121    ; Read keyboard row state

keychk:
	lda #00
	sta KB_COLS
	lda KB_ROWS
        cmp #$ff
        beq keychk 
	rts
