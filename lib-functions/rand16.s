;; 16-bit pseudo-random number generator with period of 65535
;; seed must never be 32755

.include "../LAMAlib-macros16.inc"

.export rand16sr, rand16seed:=seed

;one out of 2048 possible magic EOR numbers that give a 16-bit PRNG with full period

magic=$8015

rand16sr:	
	asl seed
	rol seed+1
	bcs :+
	lda #$80
	eor seed+1
	sta seed+1
	tax
	lda #$15
	eor seed
	sta seed
	rts

:	lda seed
	ldx seed+1
	rts

seed:   .byte <56278, >56278