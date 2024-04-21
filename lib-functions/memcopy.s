.importzp _llzp_word1,_llzp_word2

.code

.export _memcopy_sr:=memcopyloop

memcopyloop:
	lda (_llzp_word2),y
	sta (_llzp_word1),y
	iny
	bne memcopyloop

	inc _llzp_word2+1
	inc _llzp_word1+1

	dex
	bne memcopyloop
	rts