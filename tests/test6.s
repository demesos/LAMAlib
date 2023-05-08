;--------------------------------------------------
; begin of test program
;--------------------------------------------------

.include "LAMAlib.inc"


	sei
	wait_for_rasterline 25
	delay_ms 8
	lda $d012
	cmp #160		;lower value
	bcc exit_failure
	cmp #172
	bcs exit_failure

	wait_for_rasterline 4
	ldax #10
	delay_ms AX
	lda $d012
	cmp #168		;lower value
	bcc exit_failure
	cmp #180
	bcs exit_failure

	clc
	rts

exit_failure:
	sec
	rts

