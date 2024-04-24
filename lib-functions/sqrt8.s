.importzp _llzp_word1
.export _sqrt8_sr

.proc _sqrt8_sr
num=_llzp_word1
rem=_llzp_word1+1
	sta num
	
	lda #0
	sta root
	sta rem
	ldx #4
L1:	sec
	lda num
	sbc #$40
	tay
	lda rem
	sbc root
	bcc L2
	sty num
	sta rem
L2:	rol root
	asl num
	rol rem
	asl num
	rol rem
	dex
	bne L1
root=*+1
	lda #$42	
	ldx rem
	rts
.endproc