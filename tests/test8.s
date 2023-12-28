;--------------------------------------------------
; test program for LAMAlib functions
;
; test hw timer and delay_ms function

.include "LAMAlib.inc"



.proc test_no_8

TOLERANCE=900	;in cycles, for the ms test
DELAYMS=17

	blank_screen		;to avoid jitter in delay_ms because of badlines

	stop_timerA
	ldax #40000
	sei
	set_timerA AX

	read_timerA ax
	pokew $400,AX

	start_timerA
	delay_ms DELAYMS
	stop_timerA
	read_timerA
	pokew $402,AX

	start_timerA
	delay_cycles 366 
	stop_timerA
	read_timerA
	pokew $404,AX

	set_timerA_latch $4025	;set timer latch back to 60Hz
	start_timerA
	cli
	unblank_screen

	ldax $402
	cmpax #40000-DELAYMS*983-TOLERANCE
	bcc exit_failure
	cmpax #40000-DELAYMS*983+TOLERANCE
	bcs exit_failure

	subax $404
	cmpax #376
	bne exit_failure

	clc
	rts

exit_failure:
	sec
	rts

.endproc
