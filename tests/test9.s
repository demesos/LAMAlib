;--------------------------------------------------
; test program for LAMAlib functions
;
; test do_every and do_once structure
;
; re-running this test fails, because the state of do_every will be changed by running it

.include "LAMAlib.inc"

.proc test_no_9
        ; Basic do_skip_every - skip every 3rd iteration
        ldx #0
        for Y, 0, to, 11  ; 12 iterations
          do_skip_every 3
            inx
          end_skip_every
        next
        cpx #8  ; Should execute 8 times (skip iterations 2, 5, 8, 11)
        if ne
          sec
          rts
        endif

        ; Nested do_skip_every with counters
        ldx #0
        for Y, 0, to, 17  ; 18 iterations
          do_skip_every 2  ; runs 1 out of 2
            inx
            do_skip_every 3  ; inner: runs 2 out of 3
              inx
            end_skip_every
          end_skip_every
        next
        cpx #15  ; outer runs 9 times, inner skips 3 of those 9, so 9 + 6 = 15
        if ne
          sec
          rts
        endif

        ; Phase offset test
        ldx #0
        for Y, 0, to, 11  ; 12 iterations
          do_skip_every 3, 0  ; skip on first iteration (phase=0)
            inx
          end_skip_every
        next
        cpx #8  ; Should still execute 8 times but different pattern
        if ne
          sec
          rts
        endif

        ; Alternating inx/dex with skip
        ldx #0
        for Y, 0, to, 8  ; 9 iterations
          do_skip_every 3
            inx
            inx
            dex  ; net +1 when not skipped
          end_skip_every
        next
        cpx #6  ; 6 times executed (skip iterations 2, 5, 8)
        if ne
          sec
          rts
        endif

        ; do_once with maxcalls
        ldx #0
        for Y,0,to,9
          do_once 3
            inx
          end_once
        next
        cpx #3
        if ne
          sec
          rts
        endif

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
