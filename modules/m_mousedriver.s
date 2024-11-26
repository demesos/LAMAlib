;***********************************************************************
;* Module: mousedriver
;* Version 0.1
;
; based on the mousedriver code from codebase64 and GEOS inputdrv code
;
; module to be configured and included with
; .scope mousedriver
; .include "m_mousedriver.s"
; .endscope
;
; usage from the main program
;
; m_init mousedriver    ;initializes the mousedriver and install the sprites
; m_run mousedriver     ;to be called once per frame
;
; Pointer coordinates are available in pointer_x (16 bit) and pointer_y (8 bit)
; Button states are in leftbutton and rightbutton, previous button states are in leftbutton_old and rightbutton_old

.include "LAMAlib.inc"

;***********************************************************************
;* parameters - can be overwritten from main
;* without a default value the constant must be set by the main program

def_const JOY_CONTROL,2 ;0..none, 1..joyport1, 2..joyport2, 4..wasdspace, combinations possible by adding the up the respective values
def_const MOUSE_CONTROL,1 ;0..no mouse control, 1..1351 mouse on port 1
def_const MIN_X,24
def_const MAX_X,343
def_const MIN_Y,50
def_const MAX_Y,249
def_const CHECK_BOUNDS,1
def_const INSTALL_SPRITES,$340  ;if 0 no sprite is initialized, otherwise sprite pointer is copied to this address
def_const PLACE_SPRITE,1
def_const OVERLAY,1     ;if 1 then the pointer is shown as two sprites overlaying each other
def_const SPR_NUM,0
;***********************************************************************
;* module implementation

.if (JOY_CONTROL & 1) .and MOUSE_CONTROL
        .error  "Joystick port 1 and the mouse cannot both be used as inputs for the mousedriver!"
.endif

init:
        pokew pointer_x,(MIN_X+MAX_X)/2
        poke pointer_y,(MIN_Y+MAX_Y)/2
.if INSTALL_SPRITES
        memcopy sprite_data,INSTALL_SPRITES,130
        setSpriteCostume SPR_NUM,INSTALL_SPRITES
        setSpriteCostume SPR_NUM+1,INSTALL_SPRITES+$40
        updateSpriteAttributes SPR_NUM
        updateSpriteAttributes SPR_NUM+1
        showSprite SPR_NUM
        showSprite SPR_NUM+1
.endif

        lda #$11
        sta CIA1_PRB    ; init mouse buttons

        rts
run:
	lda leftbutton
	sta leftbutton_old
	lda rightbutton
	sta rightbutton_old
	lda #0
	sta leftbutton
	sta rightbutton

;=======================================================================
;= joystick and keyboard part
.if JOY_CONTROL & 4 > 0
        readWASDspace
  .if JOY_CONTROL & 2 > 0
        and $dc00
  .endif
  .if JOY_CONTROL & 1 > 0
        and $dc01
  .endif
.elseif JOY_CONTROL & 2 > 0
        lda $dc00
  .if JOY_CONTROL & 1 > 0
        and $dc01
  .endif
.elseif JOY_CONTROL & 1 > 0
        lda $dc01
.endif
.if JOY_CONTROL
; maximum step size ("speed") for joystick acceleration routine, in pixels.
MAX_STEP = 5

; steps before acceleration starts, in pixels.
MAX_TIME = $04

; read control bits of joystick in Port2
        ;lda $dc00      ;input to be done before this routine
        tax             ;remember value

; check up direction
        lsr
        if cc
            tay
            sec
            lda pointer_y
            sbc joystepsize
            sta pointer_y
            tya
        endif

; check down direction
        lsr
        if cc
            tay
            ;clc    ;Carry is clear because of if cc
            lda pointer_y
            adc joystepsize
            sta pointer_y
            tya
        endif

; check left direction
        lsr
        if cc
            ;subtract current step size from x value if needed.
            tay
            sec
            lda pointer_x
            sbc joystepsize
            sta pointer_x
            if cc
                dec pointer_x+1
            endif
            tya
        endif

; check right direction
        lsr
        if cc
            tay
            ;clc    ;Carry is clear because of if cc
            lda pointer_x
            adc joystepsize
            sta pointer_x
            if cs
                inc pointer_x+1
            endif
            tya
        endif

;check button
        lsr
        if cc
            inc leftbutton
        endif


        txa             ;restore joystick direction bits
        and #$0f        ;mask for direction bits
        cmp #$0f
        if eq
            ;no joystick movement, reset speed and wait counter
            poke joystepsize,1
            poke joywaittime,MAX_TIME
        else
joystepsize=*+1
            lda #$01
            cmp #MAX_STEP
            if cc
                dec joywaittime
                if mi
                    inc joywaittime         ;set wait time back to 0 (just one wait cycle for next speedups)
                    inc joystepsize
                endif
            endif
        endif
.endif
;= end of joystick and keyboard part
;====================================

;=======================================================================
;= mouse part

.if MOUSE_CONTROL
potx    = $d419
poty    = $d41a

        lda potx                 ; get delta values for x
opotx=*+1
        ldy #0
        jsr movchk
        sty opotx

        clc                      ; modify low byte xposition
        adc pointer_x
        sta pointer_x

        txa                      ; get high byte
        adc pointer_x+1
        if mi
            lda #0
            sta pointer_x
        endif
        sta pointer_x+1

        lda poty                 ; get delta value for y
opoty=*+1
        ldy #$0
        jsr movchk
        sty opoty

        sec                      ; modify y position ( decrease y for increase in pot )
        eor #$ff
        adc pointer_y
        sta pointer_y
        txa
        if cc
            if eq
                stx pointer_y
            endif
        else
            if ne
                stx pointer_y
            endif
        endif

        ; handling mouse buttons
        lda #$11
        sta CIA1_DDRB   ;port bits to input
        ldx #$01
        lda #$10 ; check left button
        bit CIA1_PRB
        bne checkright
        stx leftbutton ; store state
checkright: 
        txa
        bit CIA1_PRB
        bne notright
        stx rightbutton ; store state
notright: 
	; reset port to normal state
	dex
        stx CIA1_DDRB
.endif

;= end of mouse part
;====================================

;=======================================================================
;= bounds check
.if CHECK_BOUNDS
        ldax pointer_x
        cmpax #MAX_X+1
        if cs
            ldax #MAX_X
        endif
        cmpax #MIN_X
        if cc
            ldax #MIN_X
        endif
        stax pointer_x

        lda pointer_y
        cmp #MAX_Y+1
        if cs
            lda #MAX_Y
        endif
        cmp #MIN_Y
        if cc
            lda #MIN_Y
        endif
        sta pointer_y
.endif
;= end of bounds check
;====================================


;=======================================================================
;= sprite placement
.if PLACE_SPRITE
        ldax pointer_x
        setSpriteX SPR_NUM,AX
        lda  pointer_y
        setSpriteY SPR_NUM,A
  .if OVERLAY .and (::_overlay_implicit = 0)
        setSpriteY SPR_NUM+1,A
        ldax pointer_x
        setSpriteX SPR_NUM+1,AX
  .endif
.endif
;= end of sprite placement
;====================================
        rts

.if MOUSE_CONTROL
;=======================================================================
; movchk
;       entry   Y = old value of pot register
;               A = currrent value of pot register
;       exit    Y = value to use for old value
;               AX = delta value for position
movchk:
        sty oldvalue
        sta newvalue
        ldx #0

        sec
oldvalue=*+1
        sbc #0
        and #%01111111
        cmp #%01000000
        bcs bit6was1
    ;bit 6 was 0
        lsr
        beq no_move_end
newvalue=*+1
        ldy #0
        rts

    ;bit 6 was 1
bit6was1:
        ora #%11000000
        cmp #$ff
        beq no_move_end
        sec
        ror
        ldx #$ff
        ldy newvalue
        rts
no_move_end:
        lda #0
        rts
.endif
;====================================

.if JOY_CONTROL
joywaittime: .byte MAX_TIME
.endif
pointer_x:   .word (MIN_X+MAX_X)/2
pointer_y:   .byte (MIN_Y+MAX_Y)/2
rightbutton: .byte 0
leftbutton:  .byte 0
rightbutton_old: .byte 0
leftbutton_old:  .byte 0

.if INSTALL_SPRITES
sprite_data:
        .byte %00000000,%00000000,%00000000
        .byte %01000000,%00000000,%00000000
        .byte %01100000,%00000000,%00000000
        .byte %01110000,%00000000,%00000000
        .byte %01111000,%00000000,%00000000
        .byte %01111100,%00000000,%00000000
        .byte %01110000,%00000000,%00000000
        .byte %01010000,%00000000,%00000000
        .byte %00001000,%00000000,%00000000
        .byte %00001000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte $11
        .byte %11000000,%00000000,%00000000
        .byte %10100000,%00000000,%00000000
        .byte %10010000,%00000000,%00000000
        .byte %10001000,%00000000,%00000000
        .byte %10000100,%00000000,%00000000
        .byte %10000010,%00000000,%00000000
        .byte %10001110,%00000000,%00000000
        .byte %10101000,%00000000,%00000000
        .byte %11010100,%00000000,%00000000
        .byte %00010100,%00000000,%00000000
        .byte %00001100,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte %00000000,%00000000,%00000000
        .byte $00
.endif

