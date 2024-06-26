;--------------------------------------------------
; test program for LAMAlib functions
;
; testing: for (addr),start,to,end


.include "LAMAlib.inc"

	nop
	nop
	nop

	jsr testforaddr
	ldax 253
	cmpax #5050
	if ne
	  sec
	  rts
	endif

	pokew 2,5050

	for [253],1,to,100
	  lda 253
	  ldx #0
	  rsbax 2
	  stax 2
	next
	ldax 2
	cmpax #0
	if ne
	  sec
	  rts
	endif
nop
lda $5880
	pokew 253,0

	for [2],100,downto,50,5
	  lda 2
	  ldx #0
	  addax 253
	  stax 253
	next
	ldax 253
	cmpax #825
	if ne
	  sec
	  rts
	endif

	pokew 253,0
	for [2],250,downto,50,5
	  lda 2
	  ldx #0
	  addax 253
	  stax 253
	next
	ldax 253
	cmpax #6150
	if ne
	  sec
	  rts
	endif

	clc
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