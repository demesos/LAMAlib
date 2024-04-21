.importzp _llzp_word1
.export _sqrt16_sr

.proc _sqrt16_sr
numl=_llzp_word1
numh=_llzp_word1+1
	lda #0
	sta root
	sta rem
	ldx #8
L1:	sec
	lda numh
	sbc #$40
	tay
	lda rem
	sbc root
	bcc L2
	sty numh
	sta rem
L2:	rol root
	asl numl
	rol numh
	rol rem
	asl numl
	rol numh
	rol rem
	dex
	bne L1
root=*+1
	lda #$42
rem=*+1
	ldx #$42
	rts
.endproc