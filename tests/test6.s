;--------------------------------------------------
; test program for LAMAlib functions
;
; test delay_ms

.include "LAMAlib.inc"


	sei
	wait_for_rasterline 25
	delay_ms 8		;8ms are 8/20*312 = 124.8 rasterlines
	lda $d012
	cmp #145		;lower value
	bcc exit_failure
	cmp #155
	bcs exit_failure

	wait_for_rasterline 4
	ldax #10
	delay_ms AX		;10ms are 10/20*312 = 156 rasterlines
	lda $d012
	cmp #155		;lower value
	bcc exit_failure
	cmp #165
	bcs exit_failure

	do
	  wait_for_rasterline 25
	  delay_ms_abort_on_fire 8		;8ms are 8/20*312 = 124.8 rasterlines
	loop while cc
	lda $d012
	cmp #145		;lower value
	bcc exit_failure
	cmp #155
	bcs exit_failure

	do
	  wait_for_rasterline 4
	  ldax #10
	  delay_ms_abort_on_fire AX		;10ms are 10/20*312 = 156 rasterlines
	loop while cc
	lda $d012
	cmp #155		;lower value
	bcc exit_failure
	cmp #165
	bcs exit_failure




	clc
	rts

exit_failure:
	sec
	rts

