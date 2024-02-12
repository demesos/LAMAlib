;--------------------------------------------------
; test program for LAMAlib functions
;
; testing window functions

.include "LAMAlib.inc"

.if .not .definedmacro(jne)
.macpack longbranch      
.endif

.import _window_x1,_window_y1,_window_x2,_window_y2

.proc test_no_12
	sec
	jsr PLOT
	store X
	store Y

	poke _window_x1,21
	poke _window_y1,1
	poke _window_x2,38
	poke _window_y2,10
	enable_chrout2window
	draw_frame
	clear_window
	
	for x,0,to,200
	  print "hallo"
	  lda #157
	  jsr CHROUT
	next

	disable_chrout2window

	ldax #$400
	stax addr
	poke sum,0
	for y,0,to,11
sum=*+1
	  lda #$af	
	 
	  for x,20,to,39
	    clc
addr=*+1	
	    adc $affe,x
	  next
	  sta sum
	  inc16 addr,40
	next

	restore Y
	restore X
	clc 
	jsr PLOT

	lda sum
	cmp #94
	bne exit_failure
	
	clc
	rts

exit_failure:
	sec
	rts
.endproc
