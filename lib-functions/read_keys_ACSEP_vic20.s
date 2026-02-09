; VIC20 version of read_keys_ACSEP
; Reads keys @, :, ;, =, P
; Return value is in A, where the respective bit is zero if the key is pressed
; Bit mapping (matching C64 ACSEP output):
;   bit 0: @ (up)
;   bit 1: ; (down)
;   bit 2: : (left)
;   bit 3: = (right)
;   bit 4: P (fire)

.include "LAMAlib-macros16.inc"

.export _read_keys_ACSEP_vic20_sr

KB_COLS = $9120    ; Write to select keyboard columns
KB_ROWS = $9121    ; Read keyboard row state

_read_keys_ACSEP_vic20_sr:
	; Read P key: bit 5 of KB_ROWS after poke KB_COLS,255-(1<<1)
	poke KB_COLS,%11111101   ; 255-(1<<1)
	lda KB_ROWS
	and #%00100000           ; mask to keep only bit 4
	sta bit_fire               ; save P key state

	poke KB_COLS,%11111011   ; 255-(1<<2)
	lda KB_ROWS
	and #%01000000           ; isolate bit 6
	sta bit_dwn
	poke KB_COLS,%10111111   ; 255-(1<<6)
	lda KB_ROWS
	and #%00100000           ; isolate bit 5
bit_dwn=*+1
	ora #$42
	lsr
	lsr
	sta bits_updwn
	poke KB_COLS,%11011111   ; 255-(1<<5)
	lda KB_ROWS
	and #%01100000           ; isolate bit 5 and 6
bits_updwn=*+1
	ora #$42
	lsr
	lsr
bit_fire=*+1
	ora #$42	
	lsr
	; Restore KB_COLS to standard value
	sec
	rol KB_COLS
	rol KB_COLS              ; KB_COLS is now %01111111 (standard value)

	rts
