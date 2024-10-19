; Reads keys CBM, Arrowleft, Cursor right, F7, F1, +, M, X
; Routine should not be interrupt by another keyboard scan, therefore it is
; recommended to
; - run the function in the interrupt or
; - have the keyboard scan in the interrupt turned off or
; - put a sei / cli around the function call
; Return value is in A, where the respective bit is zero if the key is pressed

.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"

.export _read_keys_CACFFPMX_sr := _read_keys_CACFFPMX
.import _twopotentials

.proc _read_keys_CACFFPMX
        lda #0
        sta $dc03
        lda #$ff
        sta $dc02

        ;first row
        asl     ;A is now %11111110
        sta $dc00
        lda $dc01
        ora #%11100011  ;mask for CRSR lr, F7, F1
        sta bits_234

        ;next row
        lda #%11111011
        sta $dc00
        lda $dc01
        ora #%01111111
bits_234=*+1
        and #$42        ;values for bits 2,3,4
        sta bits_2347

        ;next row
        lda #%11101111
        sta $dc00
        lda $dc01
        asl
        asl
        ora #%10111111
bits_2347=*+1
        and #$42        ;values for bits 2,3,4,7
        sta bits_23467

        ;next row
        lda #%11011111
        sta $dc00
        lda $dc01
        lsr             ;bit 0 -> C
bits_23467=*+1
        lda #$42        ;values for bits 0,1,2
        if cc
            and #%11011111
        endif
        sta bits_234567

        ;next row
        lda #%01111111
        sta $dc00
        lda $dc01
        ora #%11011101
        bit _twopotentials+5
        if eq
            eor #%00100001        ;set bit 1 to 0 and bit 5 to 1
        endif
bits_234567=*+1
        and #$42        ;values for bits 2,3,4,6,7
        rts
.endproc
