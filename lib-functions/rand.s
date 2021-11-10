;; X ABC Algorithm Random Number Generator for 8-Bit Devices
;;
;; Algorithm from EternityForest, slight modification by Wil
;; https://www.electro-tech-online.com/threads/ultra-fast-pseudorandom-number-generator-for-8-bit.124249/
;; This version stores the seed as arguments and uses self-modifying code
;; Routine requires 50 cycles / 28 bytes

.export _rand8_sr:=rand8, _rand_seed1:=x1, _rand_seed2:=a1, _rand_seed3:=b1, _rand_seed4:=c1

rand8:	
	inc x1
	clc
x1=*+1
	lda #$00	;x1
c1=*+1
	eor #$c2	;c1
a1=*+1
	eor #$11	;a1
	sta a1
b1=*+1
	adc #$37	;b1
	sta b1
	lsr
	eor a1
	adc c1
	sta c1
	rts