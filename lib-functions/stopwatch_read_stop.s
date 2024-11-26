.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-systemaddresses.inc"

; to be used together with stopwatch_start_ms or stopwatch_start_ticks
; returns timer value in A/X

.export _stoptimer_sr := stoptimer
.export _readtimer_sr := readtimer

stoptimer:
	lda #%00000000
	sta CIA2_CRA
	sta CIA2_CRB
readtimer:
	lda CIA2_TIMERB
	ldx CIA2_TIMERB+1
	negax
	rts