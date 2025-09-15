;--------------------------------------------------
; test program for LAMAlib functions
;
; test do_every and do_once structure
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
	    tax
            for Y,0,to,255
                do_once
                inx
                end_once
                do_once 2
                do_once 3
                inx
                inx
                inx
                inx
                inx
                end_once
                dex
	        dex
                dex
	        dex
                dex
	        dex
                end_once
            next
        endif
        cpx #28+1-2*6+2*5
        if eq
            clc
        else
            sec
        endif
        rts

.endproc
