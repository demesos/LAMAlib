.export _getkey
.importzp CHARS_IN_KEYBUF

.proc _getkey
	lda #0
	;sta _chars_in_keybuf
	sta CHARS_IN_KEYBUF
:
	jsr $FFE4
	beq :-
	rts
.endproc