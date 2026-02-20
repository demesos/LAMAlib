;=============================================================================
; Lissajous Sprite Demo - Sprite Multiplexer with Phase-Offset Animation
;
; NUM_SPRITES sprites trail a Lissajous curve.
; Each sprite has a phase offset of N_POINTS/NUM_SPRITES steps apart.
; Every other frame all phases advance by 1 (do_every 2), animating the swarm.
;
; POST_ROUTINE trick: must be a pure constant for .if evaluation in module.
; Trampoline JMP planted at $0110 (low stack - never reached in practice).
;
; Build: make
;=============================================================================

.include "LAMAlib.inc"
.include "LAMAlib-muplex-sprites.inc"

SPRITE_BASE     = $3f00
SCREEN_BASE     = $0400

NUM_SPRITES     = 24
FIRST_COSTUME   = <(SPRITE_BASE / 64)
NUM_COSTUMES    = 4

TRAMPOLINE      = $0110     ; low stack page - safe, stack never reaches this low

jmp start

;-----------------------------------------------------------------------------
; Pattern tables
;-----------------------------------------------------------------------------

    .include "lissajous_inf.inc"

;-----------------------------------------------------------------------------
; Post routine - reached via trampoline at $0110
;-----------------------------------------------------------------------------
do_lissajous:
    do_every 2
    jmp $ea81
    end_every
    poke spr_idx, NUM_SPRITES-1

place_sprites_and_update_phase:
    ldx spr_idx
    ldy phaseoffset,x

    lda xpoints_hi,y        ; 0 or 1 (X max=320, hi byte is 0 or 1)
    lsr                     ; bit0 -> carry = 9th X bit
    lda xpoints_lo,y        ; lo byte, carry preserved

    ldy spr_idx             ; setSpriteX requires sprite# in Y (not X)
    setSpriteX Y, A         ; A=lo byte, carry=hi bit

    ldx spr_idx
    ldy phaseoffset,x
    lda ypoints,y
    setSpriteY X, A

    iny                     ; advance phase (wraps mod 256 = N_POINTS)
    ldx spr_idx
    tya                     ; sty abs,x illegal on 6502 - use tya/sta
    sta phaseoffset,x

    dec spr_idx
    bpl place_sprites_and_update_phase
    jmp $ea31               ; KERNAL IRQ exit - RTI to restore interrupted code

;-----------------------------------------------------------------------------
; Sprite multiplexer module
;-----------------------------------------------------------------------------
.scope sprmux
    MAXSPRITES               = ::NUM_SPRITES
    ENABLE_OVERLAY           = 0
    ENABLE_YPRIORITY         = 0
    ENABLE_GROUNDED          = 0
    ENABLE_UPDATE_ATTRIBUTES = 1
    DEBUG_RASTER_TIME        = 1
    PRE_ROUTINE              = 0
    POST_ROUTINE             = ::TRAMPOLINE
    .include "modules/m_sprmultiplexer.s"
.endscope

;-----------------------------------------------------------------------------
; Variables
;-----------------------------------------------------------------------------
spr_idx:     .res 1
phaseoffset: .res NUM_SPRITES

;-----------------------------------------------------------------------------
; Main program
;-----------------------------------------------------------------------------
.code
start:
    clrscr
    println "lissajous: infinity"
    lda #NUM_SPRITES
    println "num sprites: ", A

    install_file "testsprites.prg", SPRITE_BASE

    ; Calculate phase offsets with 8.8 bit fixed point arithmetic
    ldax #0
    ldy #NUM_SPRITES
phase_init_loop:
    pha
    txa
    sta phaseoffset,y
    pla
    addax #(N_POINTS*$100/NUM_SPRITES)
    dey
    bpl phase_init_loop

    ; Install trampoline at $0110: JMP do_lissajous
    poke  TRAMPOLINE,   $4C
    pokew TRAMPOLINE+1, do_lissajous

    m_init sprmux

    ;-------------------------------------------------------------------------
    ; Initialise sprites: set costume, colour, show
    ;-------------------------------------------------------------------------
    ldx #0
init_loop:
    txa
    and #(NUM_COSTUMES-1)
    clc
    adc #FIRST_COSTUME
    setSpriteCostume X, A

    updateSpriteAttributes X
    showSprite X

    inx
    cpx #NUM_SPRITES
    bne init_loop

    println "\nrunning..."
    println "press key to exit"

    getkey
    clrscr
    println "done."
    rts

