.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-ROMfunctions.inc"

; to be used together with stopwatch_read_stop
; returns timer value in A/X

.export _starttimer_ms_sr := starttimer

one_ms=$3FE ;for C64 PAL clock frequency

starttimer:
	lda #%00000000
	sta CRA
	sta CRB
	lda #<one_ms
	sta TIMERA
	lda #>one_ms
	sta TIMERA+1
	lda #$FF
	sta TIMERB
	sta TIMERB+1
	lda #%01000001 ;timer B counts underflows of timer A
	sta CRB
	lda #%00000001 ;start timer A 
	sta CRA
	rts