.importzp _target_ptr

.code

.export _memset_sr:=memsetloop

memsetloop:
	sta (_target_ptr),y
	iny
	bne memsetloop
	inc _target_ptr+1
	dex
	bne memsetloop
	rts