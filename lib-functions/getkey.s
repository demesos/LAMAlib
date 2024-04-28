.export _getkey
.importzp _CHARS_IN_KEYBUF

.proc _getkey
	lda #0
	sta _CHARS_IN_KEYBUF
:
	jsr $FFE4
	beq :-
	rts
.endproc