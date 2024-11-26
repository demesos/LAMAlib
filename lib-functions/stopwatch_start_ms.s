.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-systemaddresses.inc"

; to be used together with stopwatch_read_stop
; returns timer value in A/X

.export _starttimer_ms_sr := starttimer

one_ms=$3FE ;for C64 PAL clock frequency

starttimer:
	lda #%00000000
	sta CIA2_CRA
	sta CIA2_CRB
	lda #<one_ms
	sta CIA2_TIMERA
	lda #>one_ms
	sta CIA2_TIMERA+1
	lda #$FF
	sta CIA2_TIMERB
	sta CIA2_TIMERB+1
	lda #%01000001 ;timer B counts underflows of timer A
	sta CIA2_CRB
	lda #%00000001 ;start timer A 
	sta CIA2_CRA
	rts