;--------------------------------------------------
; begin of test program
;--------------------------------------------------

.include "LAMAlib.inc"
	ldax #40000
	sei
	set_timerA_latch AX

	read_timerA ax
	stax timer_before

	delay_ms 20

	read_timerA

	subax timer_before
	cli

	printax

	clc
	rts

exit_failure:
	sec
	rts

timer_before:
.byte 00,00