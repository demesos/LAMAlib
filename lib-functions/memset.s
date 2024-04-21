.importzp _llzp_word1

.code

.export _memset_sr:=memsetloop

memsetloop:
	sta (_llzp_word1),y
	iny
	bne memsetloop
	inc _llzp_word1+1
	dex
	bne memsetloop
	rts