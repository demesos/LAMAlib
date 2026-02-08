; waits for any key or fire button to be pressed, VIC20 version

.export _waitkeyorbutton_vic20_sr := chkinput

JOYREG1 = $9111
DDR2    = $9122
KB_COLS = $9120    ; Write to select keyboard columns
KB_ROWS = $9121    ; Read keyboard row state

chkinput:
	lda #%00100000
	bit JOYREG1
	beq chkinput_done
	lda #00
	sta KB_COLS
	lda KB_ROWS
        cmp #$ff
        beq chkinput 
chkinput_done:
	lda #$7f
	sta KB_COLS	;restore KB_COLS setting

	rts
