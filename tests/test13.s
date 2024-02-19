;--------------------------------------------------
; test program for LAMAlib functions
;
; testing print_wrapped

.include "LAMAlib.inc"

.if .not .definedmacro(jne)
.macpack longbranch      
.endif

.proc test_no_13
SCREEN=1024

	sec
	jsr PLOT
	store X
	store Y

	poke 648,>SCREEN
	lda #19
	jsr CHROUT

.scope
	ldax #SCREEN
	stax addr
	for y,0,to,7
	  cpy #3
	  if cc
	    lda #255
	  else
	    lda #17
	  endif
	  sta cmpx

	  lda #32
	  ldx #39
xloop:
addr=*+1	
	  sta $affe,x
	  dex
cmpx=*+1
	  cpx #$af
	  bne xloop

	  inc16 addr,40
	next
.endscope

	print_wrapped "the quick brown fox jumped over the lazy dog."
	print_wrapped " what did the quick brown do? jumping over the lazy dog!"

	textcolor 3
	print_wrapped_setpars 18,22,13
	print_wrapped "the quick brown fox jumped over the lazy dog."
	print_wrapped "what did the quick brown do? jumping over the lazy dog!"

	textcolor 15
	print_wrapped_setpars 22,17,32
	set_cursor_pos 1,22
	print_wrapped "the quick brown fox jumped over the lazy dog."
	print_wrapped "what did the quick brown do? jumping over the lazy dog!"


	ldax #SCREEN
	stax addr
	poke sum,0
	for y,0,to,7
	  cpy #3
	  if cc
	    lda #255
	  else
	    lda #17
	  endif
	  sta cmpx

sum=*+1
	  lda #$af	
	 
	  ldx #39
xloop:
	  clc
addr=*+1	
	  adc $affe,x
	  dex
cmpx=*+1
	  cpx #$af
	  bne xloop

	  sta sum
	  inc16 addr,40
	next

	restore Y
	restore X
	clc 
	jsr PLOT

	lda sum
	cmp #65
	bne exit_failure
	
	clc
	rts

exit_failure:
	sec
	rts



.endproc
