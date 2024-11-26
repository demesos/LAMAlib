.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-systemaddresses.inc"

; to be used together with stopwatch_read_stop
; returns timer value in A/X

.export _starttimer_ticks_sr := starttimer

starttimer:
	lda #%00000000
	sta CIA2_CRA
	sta CIA2_CRB
	lda CIA2_ICR
	lda #$FF
	sta CIA2_TIMERB
	sta CIA2_TIMERB+1
	lda #%00000001
	sta CIA2_CRB
	rts
