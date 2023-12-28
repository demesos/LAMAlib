;--------------------------------------------------
; test program for LAMAlib functions
;
; test do_every structure
;
; re-running this test fails, because the state of do_every will be changed by running it

.include "LAMAlib.inc"

.proc test_no_9
        for Y,0,to,79
          lda #'.'
          tya
          and #7
          sta $400,Y
          do_every 10
            lda #'1'+128
            sta $400,Y
            do_every 2
              lda #'2'+128
              sta $400,Y
            end_every
            do_every 3,1
              lda #'-'+128
              sta $400,Y
            end_every
          end_every
        next

	checksum_eor $0400,$044f
	cmp #28
	if eq
	  clc
	else
	  sec
	endif
        rts

.endproc