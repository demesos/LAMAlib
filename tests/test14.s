;--------------------------------------------------
; test program for LAMAlib functions
;
; testing print_wrapped

.include "LAMAlib.inc"

.if .not .definedmacro(jne)
.macpack longbranch      
.endif

.proc test_no_14

	strlen8 zerostr
	cpx #0
	jne exit_failure

	strlen8 onestr
	cpx #1
	jne exit_failure

	strlen8 ffstr
	cpx #$ff
	jne exit_failure

	ldax #zerostr
	strlen8 AX
	cpx #0
	jne exit_failure

	ldax #onestr
	strlen8 AX
	cpx #1
	jne exit_failure

	ldax #ffstr
	strlen8 AX
	cpx #$ff
	jne exit_failure

	strlen16 zerostr
	cmpax #0
	jne exit_failure

	strlen16 onestr
	cmpax #1
	jne exit_failure

	strlen16 ffstr
	cmpax #$ff
	jne exit_failure

	ldax #zerostr
	strlen16 AX
	cmpax #0
	jne exit_failure

	ldax #onestr
	strlen16 AX
	cmpax #1
	jne exit_failure

	ldax #ffstr
	strlen16 AX
	cmpax #$ff
	jne exit_failure

	ldax #thousandstr
	strlen16 AX
	cmpax #1000
	jne exit_failure

	clc
	rts

exit_failure:
	sec
	rts

zerostr:
	.byte 0
	
onestr:
	.byte 1,0

ffstr:	; Reserve 255 bytes of memory with value 1
	.res $ff,1
	.byte 0

thousandstr:	; Reserve 1000 bytes of memory with value 1
	.res 1000,1
	.byte 0

.endproc
