.importzp _source_ptr,_target_ptr

.code

.export _memcopy_sr:=memcopyloop

memcopyloop:
	lda (_source_ptr),y
	sta (_target_ptr),y
	iny
	bne memcopyloop

	inc _source_ptr+1
	inc _target_ptr+1

	dex
	bne memcopyloop
	rts