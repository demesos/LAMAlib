.export _screen_off

.proc _screen_off
	lda $D011
	and #%01101111	;mask out raster high bit and screen on bit
	sta $D011	;turn screen off
	rts
.endproc