;turns screen back on

.export _screen_on

.proc _screen_on
	lda $D011
	and #%01111111	;mask out raster high bit to avoid setting an unreachable raster interrupt
	ora #%00010000  ;set screen on bit
	sta $D011	;turn screen on
	rts
.endproc