; VIC20 version of read_keys_WASDspace
; checks the keyboard for keypresses of W, A ,S, D and Space
; output is a byte in A in the same format as a joystick value

.include "../LAMAlib-macros16.inc"

.export _read_keys_WASDspace_vic20_sr

KB_COLS = $9120    ; Write to select keyboard columns
KB_ROWS = $9121    ; Read keyboard row state

_read_keys_WASDspace_vic20_sr:
	poke KB_COLS,%11111101
	lda KB_ROWS
	lsr
	lsr	;key W -> Carry
	poke KB_COLS,%11111011
	lda KB_ROWS
	rol
	and #%00001101	;we have D,A, and W now
	sta ora_val1
	poke KB_COLS,%11101111
	lda KB_ROWS
	asl
	asl
	asl
	asl
	and #$10	;space key
	sta ora_val2
	poke KB_COLS,%11011111
	lda KB_ROWS
	and #%00000010	;S key
ora_val1=*+1
	ora #$42
ora_val2=*+1
	ora #$42	;accu contains now bit values for WASDSpace

	sec
	rol KB_COLS
	rol KB_COLS	;KB_COLS is now %01111111 (standard value)

	rts
