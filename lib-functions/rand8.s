;; based on the 8-bit pseudo-random number generator from White Flame 
;; https://codebase64.org/doku.php?id=base:small_fast_8-bit_prng
;; Simple but very fast random generator with a period of 256

.export _rand8_sr:=rand8, _rand8_seed:=sm+1

;possible values with full period are 
;$1d, $2b, $2d, $4d, $5f, $63, $65, $69, $71, $87, $8d, $a9, $c3, $cf, $e7, $f5
magic=$63

rand8:	
sm:	lda #00
	beq doEor
        asl
	beq noEor
        bcc noEor
doEor:	eor #magic
noEor:  sta sm+1
	rts
