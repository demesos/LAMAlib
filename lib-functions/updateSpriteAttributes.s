.include "LAMAlib.inc"
        .export _updateSpriteAttributes
        .import _twopotentials,_maskedtwopotentials

.proc _updateSpriteAttributes
        ldy #0
        sty spraddr
        tay

        lda PTRSCRHI    ;take screen base as indicator for VIC bank
        and #%11000000  ;mask VIC bank
        sta addbank

        lda PTRSCRHI    ;screen base
        ora #3          ;add 3
        sta sprpointer+1
sprpointer=*+1
        lda $7f8,y      ;get sprite costume pointer
        lsr
        ror spraddr
        lsr
        ror spraddr
addbank=*+1
        ora #00
        ; A contains now the high byte of the sprite address
        sta spraddr+1   ;hi byte
        ldx #$3f

spraddr=*+1
        lda $AFFE,x     ;get sprite attribute byte
        sta $d027,y     ;set sprite color
        if pl
            lda $d01c
            and _maskedtwopotentials,y
        else
            lda $d01c
            ora _twopotentials,y
        endif
        sta $d01c
        rts
.endproc
