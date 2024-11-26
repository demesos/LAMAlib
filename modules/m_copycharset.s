;***********************************************************************
;* Module: copycharset
;* Version 0.1
;
; to be configured and included with
; .scope copycharset
; CHARSET_SRC=xxxx      ;16 bit address
; CHARSET_BASE=xxxx     ;16 bit address
; CHARSET_LENGTH=xxxx   ;16 bit value
; .include "m_copycharset.s"
; .endscope
;
; usage from the main program
;
; m_init copycharset    ;copies the charset
; m_run copycharset     ;activates the charset

.include "LAMAlib.inc"

;***********************************************************************
;* parameters - can be overwritten from main
;* without a default value the constant must be set by the main program

def_const CHARSET_SRC,$D000     ;use $d800 to copy from upper/lower charset
def_const CHARSET_BASE,$3800
def_const CHARSET_LENGTH,$800
def_const EFFECT,0      ;Effects: 1 italic
                        ;         2 italic, including lower case
                        ;         3 bold
                        ;         4 bold, including lower case
                        ;         5 lower half bold
                        ;         6 lower half bold, including lower case
                        ;         7 thin
                        ;         8 thin, including lower case
def_const EFFECT_RVS,0  ;if 1 the modified chars will be placed instead of reverse chars
def_const MATCH_RVS,1   ;match rvs chars

;***********************************************************************
;* module implementation

init:
        php
        sei
        lda 1
        pha
        poke 1,51
        memcopy CHARSET_SRC,CHARSET_BASE,CHARSET_LENGTH
.if EFFECT>0
  .if EFFECT<=2
        for y,96-(EFFECT & 1)*32,downto,1
            for x,7,downto,4
src_addr= * + 1
                lda CHARSET_SRC,x
                asl
trg_addr= * + 1
                sta CHARSET_BASE,x
            next ;x
            lda src_addr
            clc
            adc #8
            sta src_addr
            sta trg_addr
            if cs
                inc src_addr+1
                inc trg_addr+1
            endif
        next ;y
  .elseif EFFECT<=4
        for y,96-(EFFECT & 1)*32,downto,1
            for x,7,downto,0
src_addr= * + 1
                lda CHARSET_SRC,x
                sta sma
                asl
sma=*+1
                ora #0
trg_addr= * + 1
                sta CHARSET_BASE,x
            next ;x
            lda src_addr
            clc
            adc #8
            sta src_addr
            sta trg_addr
            if cs
                inc src_addr+1
                inc trg_addr+1
            endif
        next ;y
  .elseif EFFECT<=6
        for y,96-(EFFECT & 1)*32,downto,1
            for x,7,downto,3
src_addr= * + 1
                lda CHARSET_SRC,x
                sta sma
                asl
sma=*+1
                ora #0
trg_addr= * + 1
                sta CHARSET_BASE,x
            next ;x
            lda src_addr
            clc
            adc #8
            sta src_addr
            sta trg_addr
            if cs
                inc src_addr+1
                inc trg_addr+1
            endif
        next ;y
  .elseif EFFECT<=8
        for y,96-(EFFECT & 1)*32,downto,1
            for x,7,downto,0
src_addr= * + 1
                lda CHARSET_SRC+$400,x
                sta sma
                asl
sma=*+1
                ora #0
                eor #$ff
trg_addr= * + 1
                sta CHARSET_BASE,x
            next ;x
            lda src_addr
            clc
            adc #8
            sta src_addr
            sta trg_addr
            if cs
                inc src_addr+1
                inc trg_addr+1
            endif
        next ;y
  .endif

  .if MATCH_RVS>0
        for y,128,downto,1
            for x,7,downto,0
src_addr2= * + 1
                lda CHARSET_BASE,x
                eor #$ff
trg_addr2= * + 1
                sta CHARSET_BASE+$400,x
            next ;x
            lda src_addr2
            clc
            adc #8
            sta src_addr2
            sta trg_addr2
            if cs
                inc src_addr2+1
                inc trg_addr2+1
            endif
        next ;y
  .endif

  .if EFFECT_RVS>0
        memcopy CHARSET_BASE,CHARSET_BASE+$400,$300-(EFFECT & 1)*$100
        memcopy CHARSET_SRC,CHARSET_BASE,$300-(EFFECT & 1)*$100
  .endif
.endif
        pla
        sta 1
        plp
        rts

run:
        poke 657,128    ;disable Commodore-Shift toggle
        set_VIC_charset CHARSET_BASE
        set_VIC_bank CHARSET_BASE & $c000
        rts
