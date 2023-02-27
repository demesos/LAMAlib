;waits until raster>249 and turns screen off

.export _wait_screen_off

.proc _wait_screen_off
	lda $D011
	and #%01101111	;mask out raster high bit and screen on bit
	ldx #249

	;wait for raster > 249  
wtlp00: bit $D011
	bmi after_loop	;if this bit is set we have a rasterline>=256
	cpx $D012	;check lower 8 bits of rasterline
	bcs wtlp00	;loop if value<=249
after_loop:
	sta $D011	;turn screen off
	rts
.endproc