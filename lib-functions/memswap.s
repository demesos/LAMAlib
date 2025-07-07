.importzp _llzp_word1,_llzp_word2

.code

.export _memswap_sr:=memswaploop

memswaploop:
	lda (_llzp_word2),y
	sta sm_A
	lda (_llzp_word1),y
	sta (_llzp_word2),y
sm_A=*+1
	lda #$42
	sta (_llzp_word1),y
	iny
	bne memswaploop

	inc _llzp_word2+1
	inc _llzp_word1+1

	dex
	bne memswaploop
	rts