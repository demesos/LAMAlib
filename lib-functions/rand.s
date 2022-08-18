;; 16-bit "798" Xorshift
;;
;; Algorithm: George Marsaglia
;; Implementation idea: John Metcalf
;; Code: Veikko Sariola, Wil Elmenreich
;;
;; https://codebase64.org/doku.php?id=base:16bit_xorshift_random_generator
;; Routine requires 46 cycles / 25 bytes

.export _rand16_sr, _seed_low, _seed_high

_rand16_sr: 
_seed_high=*+1
	lda #$33
        lsr
_seed_low=*+1
        lda #$33
        ror
        eor _seed_high
        sta _seed_high   ; high part of x ^= x << 7 done
        ror             ; A has now x >> 9 and high bit comes from low byte
        eor _seed_low
        sta _seed_low    ; x ^= x >> 9 and the low part of x ^= x << 7 done
        eor _seed_high 
        sta _seed_high   ; x ^= x << 8 done
        rts

; set seed with value from AX
; commented out because will be implemented as a macro
;set_seed:
;	cmp #1		;set carry if value != 0
;	sbc #0
;	sta _seed_low
;	stx _seed_high
;	rts
