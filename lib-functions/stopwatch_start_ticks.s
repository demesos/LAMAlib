.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-ROMfunctions.inc"

; to be used together with stopwatch_read_stop
; returns timer value in A/X

.export _starttimer_ticks_sr := starttimer

starttimer:
	lda #%00000000
	sta CRA
	sta CRB
	lda ICR
	lda #$FF
	sta TIMERB
	sta TIMERB+1
	lda #%00000001
	sta CRB
	rts
