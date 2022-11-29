.include "../LAMAlib-macros16.inc"

.export _delay_ms_sr := delay_ms

; busy waiting loop for AX milliseconds, not cycle exact

delay_ms:
	inx		;increase X because we test for 0 after decrementation
	stx counter1
loop1:
	ldx #100
	stx counter2	;reset inner loop
loop2:
	dec counter2
	bne loop2

	sec
	sbc #01
	bcs loop1	;we have decremented A and compared with $FF

	dec counter1
	bne loop1
	rts

counter1: .byte 00
counter2: .byte 00
