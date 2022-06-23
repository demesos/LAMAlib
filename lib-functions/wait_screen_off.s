;waits until raster>255 and turns screen off

.export _wait_screen_off

.proc _wait_screen_off
	lda $D011
	and #%01101111	;mask out raster high bit and screen on bit

	;wait for raster > 255  
wtlp00: bit $D011
	bpl wtlp00

	sta $D011	;turn screen off
	rts
.endproc