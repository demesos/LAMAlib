;--------------------------------------------------
; test program for LAMAlib functions
;
; testing: for (addr),start,to,end


.include "LAMAlib.inc"
	jsr testforaddr1000
	ldax 253
	cmpax #$d090
	if ne
	  sec
	  rts
	endif

	jsr testforaddr100
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

;testing orax, andax, eorax, aslax, asl16, lsr16, rolax
	ldax #$a0a0
	orax #$5
	eorax #$ffff^$a0a5
	cmpax #$ffff
	if ne
	  sec
	  rts
	endif
	andax #$a55a
	lsrax
	lsrax
	aslax
	cmpax #$52ac
	if ne
	  sec
	  rts
	endif	
	sec
	rorax
	sec
	rolax
	if cc
	  sec
	  rts
	endif		
	cmpax #$52ad
	if ne
	  sec
	  rts
	endif		
	pokew 2, $a0a0
	ldax #5
	orax 2
	stax 2
	ldax #$ffff^$a0a5
	eorax 2
	stax 2
	cmpax #$ffff
	if ne
	  sec
	  rts
	endif
	ldax #$a55a
	andax 2
	stax 2
	lsr16 2
	lsr16 2
	asl16 2
	ldax #$52ac
	cmpax 2
	if ne
	  sec
	  rts
	endif	

	;test abs and absax
	lda #10
	abs
	cmp #10
	if ne 
	  sec
	  rts
	endif

	lda #0
	abs
	cmp #0
	if ne 
	  sec
	  rts
	endif

	lda #256-10
	abs
	cmp #10
	if ne 
	  sec
	  rts
	endif

	ldax #10
	absax
	cmpax #10
	if ne 
	  sec
	  rts
	endif

	ldax #0
	absax
	cmpax #0
	if ne 
	  sec
	  rts
	endif

	ldax #$10000-10
	absax
	cmpax #10
	if ne 
	  sec
	  rts
	endif

	ldax #$10000-1010
	absax
	cmpax #1010
	if ne 
	  sec
	  rts
	endif
	;---------

	clc
	rts

addit:
	store AX
	addax 253   
	stax 253    
	restore AX
	rts

;
; testing (addr) loops
;

testforaddr100:
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

testforaddr1000:
	ldax #0
	stax 2
	stax 253
	for (x1),1,to,1000,2
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