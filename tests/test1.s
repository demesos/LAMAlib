;--------------------------------------------------
; begin of test program
;--------------------------------------------------

.include "LAMAlib.inc"

	nop
	nop
	nop

	jsr testforaddr
	ldax 253
	cmpax #5050
	if eq
	  clc
	else
	  sec
	endif
	rts

addit:
	clc
	store A
	adc 253
	sta 253
	lda 254
	adc #00
	sta 254
	restore A
	rts

;
; testing (addr) loops
;

testforaddr:
	ldax #0
	stax 2
	stax 253
	for (x1),1,to,100
.ifdef VERBOSE
	  print (x1),",",(253)," ; "
.endif
	  ldax x1
	  jsr addit
	next
.ifdef VERBOSE
	print "sum ",(253)
.endif

	rts

x1: .byte 0,0