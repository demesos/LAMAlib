.export _strout_sr

_strout_sr:
	sta sm_addr
	sty sm_addr+1
	ldx #00
	beq entry
printloop:
	jsr $FFD2
	inx
entry:
sm_addr=*+1
	lda $AFFE,x
	bne printloop
	rts