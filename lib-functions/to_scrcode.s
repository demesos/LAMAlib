.export _to_scrcode_sr

.proc _to_scrcode_sr
	cmp #$ff
	bne L0
	lda #126     ;pi character
L0:
        cmp #$60
        bcc L1+1
	cmp #$80
	bcs L2
	and #$df     ;delete bit $20 to handle uppercase chars
L2:
        ora #$40
        and #$7f
L1:
        bit $3f29    ;contains command AND #$3f 
	rts
.endproc

