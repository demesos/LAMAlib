; turns off cursor visibility, checks the cursor phase and restires character under cursor if necessary
; code works for C64 and VIC20

.export _turn_off_cursor_sr:=turn_off_cursor

; shortest implementation
turn_off_cursor:
	lda #1
	sta 204		;cursor visibility switch for C64, VIC20
	lda 207		;cursor phase
	beq :+
	lda #0		
	sta 207		;cursor phase
	ldy 211		;cursor X position
	lda 206		;char under cursor
	sta (209),y	;pointer to current line in screen memory
	lda 647
	sta (243),y	;pointer to current line in screen memory
:
	rts
