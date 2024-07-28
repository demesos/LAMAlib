; waits for any key or fire button to be pressed

.export _waitkeyorbutton_sr := chkinput

chkinput:
	lda #$10
	sta $dc00
	and $dc00
	and $dc01
	beq chkinput_done
	lda #00
	sta $dc00
	lda $dc01
        cmp #$ff
        beq chkinput 
chkinput_done:
	lda #$7f
	sta $dc00	;restore dc00 setting
	rts
