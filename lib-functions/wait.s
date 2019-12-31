.include "../LAMAlib-macros16.inc"

.export waitsr

waitsr:	inx
	stx counter1
loop1:
	ldx #100
	stx counter2
loop2:
	dec counter2
	bne loop2

	sec
	sbc #01
	bcs loop1

	dec counter1
	bne loop1
	rts

counter1: .byte 00
counter2: .byte 00
