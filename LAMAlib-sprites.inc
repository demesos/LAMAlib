;; <h2>Assembler macros to control VIC sprites (no multiplexing)</h2>
;; To use these macros, they need to be included separately with <pre>
;; .include "LAMAlib-sprites.inc"
;; </pre>

.if .not .definedmacro(showSprite)

  .ifndef VIC_BASE
VIC_BASE=$D000
  .endif

_overlay_implicit=0      ;value indicates that with this sprite macro overlay sprites need to be set and placed explicitely

;; showSprite n
;; Make sprite n visible
;; Registers modified: A
  .macro showSprite n
        lda VIC_BASE+$15
        ora #1<<(n)
        sta VIC_BASE+$15
  .endmacro

;; hideSprite n
;; Hide sprite n
;; Registers modified: A
  .macro hideSprite n
        lda VIC_BASE+$15
        and #255 - (1<<(n))
        sta VIC_BASE+$15
  .endmacro

;; enableMultiColorSprite n
;; Enable multicolor mode for sprite n
;; Registers modified: A
  .macro enableMultiColorSprite n
        lda VIC_BASE+$1c
        ora #1<<(n)
        sta VIC_BASE+$1c
  .endmacro

;; disableMultiColorSprite n
;; Disable multicolor mode for sprite n
;; Registers modified: A
  .macro disableMultiColorSprite n
        lda VIC_BASE+$1c
        and #255 - (1<<(n))
        sta VIC_BASE+$1c
  .endmacro

;; enableXexpandSprite n
;; Enable horizontal expansion for sprite n
;; Registers modified: A
  .macro enableXexpandSprite n
        lda VIC_BASE+$1d
        ora #1<<(n)
        sta VIC_BASE+$1d
  .endmacro

;; disableXexpandSprite n
;; Disable horizontal expansion for sprite n
;; Registers modified: A
  .macro disableXexpandSprite n
        lda VIC_BASE+$1d
        and #255 - (1<<(n))
        sta VIC_BASE+$1d
  .endmacro

;; enableYexpandSprite n
;; Enable vertical expansion for sprite n
;; Registers modified: A
  .macro enableYexpandSprite n
        lda VIC_BASE+$17
        ora #1<<(n)
        sta VIC_BASE+$17
  .endmacro

;; disableYexpandSprite n
;; Disable vertical expansion for sprite n
;; Registers modified: A
  .macro disableYexpandSprite n
        lda VIC_BASE+$17
        and #255 - (1<<(n))
        sta VIC_BASE+$17
  .endmacro

;; spriteBeforeBackground n
;; Make sprite n appear in front of the background
;; Registers modified: A
  .macro spriteBeforeBackground n
        lda VIC_BASE+$1b
        and #255 - (1<<(n))
        sta VIC_BASE+$1b
  .endmacro

;; spriteBehindBackground n
;; Make sprite n appear behind the background
;; Registers modified: A
  .macro spriteBehindBackground n
        lda VIC_BASE+$1b
        ora #1<<(n)
        sta VIC_BASE+$1b
  .endmacro

;; setSpriteColor n,arg
;; Set the color for sprite n
;; Registers modified: A, X, or Y (depending on input)
  .macro setSpriteColor n,arg
    .if (.xmatch ({arg}, {A}))
        sta VIC_BASE+$27+n
    .elseif (.xmatch ({arg}, {X}))
        stx VIC_BASE+$27+n
    .elseif (.xmatch ({arg}, {Y}))
        sty VIC_BASE+$27+n
    .else
        lda #arg
        sta VIC_BASE+$27+n
    .endif
  .endmacro


;; getSpriteColor n,reg
;; Get the color of sprite n into register reg
;; Registers modified: A, X, Y
  .macro getSpriteColor n,reg
    .ifblank reg
        lda VIC_BASE+$27+n
    .elseif (.xmatch ({reg}, {A}))
        lda VIC_BASE+$27+n
    .elseif (.xmatch ({reg}, {X}))
        ldx VIC_BASE+$27+n
    .elseif (.xmatch ({reg}, {Y}))
        ldy VIC_BASE+$27+n
    .else
        .error "No proper register stated, must be A, X or Y"
    .endif
  .endmacro

;; setSpriteMultiColor1 arg
;; Set the first multicolor for sprites
;; Registers modified: A, X, Y
  .macro setSpriteMultiColor1 arg
    .if (.xmatch ({arg}, {A}))
        sta VIC_BASE+$25
    .elseif (.xmatch ({arg}, {X}))
        stx VIC_BASE+$25
    .elseif (.xmatch ({arg}, {Y}))
        sty VIC_BASE+$25
    .else
        lda #arg
        sta VIC_BASE+$25
    .endif
  .endmacro

;; getSpriteMultiColor1 reg
;; Get the first multicolor of sprites into register reg
;; Registers modified: A, X, Y
  .macro getSpriteMultiColor1 reg
    .ifblank reg
        lda VIC_BASE+$25
    .elseif (.xmatch ({reg}, {A}))
        lda VIC_BASE+$25
    .elseif (.xmatch ({reg}, {X}))
        ldx VIC_BASE+$25
    .elseif (.xmatch ({reg}, {Y}))
        ldy VIC_BASE+$25
    .else
        .error "No proper register stated, must be A, X or Y"
    .endif
  .endmacro

;; setSpriteMultiColor2 arg
;; Set the second multicolor for sprites
;; Registers modified: A, X, Y
  .macro setSpriteMultiColor2 arg
    .if (.xmatch ({arg}, {A}))
        sta VIC_BASE+$26
    .elseif (.xmatch ({arg}, {X}))
        stx VIC_BASE+$26
    .elseif (.xmatch ({arg}, {Y}))
        sty VIC_BASE+$26
    .else
        lda #arg
        sta VIC_BASE+$26
    .endif
  .endmacro

;; getSpriteMultiColor2 reg
;; Get the second multicolor of sprites into register reg
;; Registers modified: A, X, Y
  .macro getSpriteMultiColor2 reg
    .ifblank reg
        lda VIC_BASE+$26
    .elseif (.xmatch ({reg}, {A}))
        lda VIC_BASE+$26
    .elseif (.xmatch ({reg}, {X}))
        ldx VIC_BASE+$26
    .elseif (.xmatch ({reg}, {Y}))
        ldy VIC_BASE+$26
    .else
        .error "No proper register stated, must be A, X or Y"
    .endif
  .endmacro

;; setSpriteCostume n,arg
;; Set the costume for sprite n
;; If arg is an register or a numerical value below 256, it is interpreted as the sprite costume block 
;; If arg is a value > 255, it is interpreted as the absolute address of the sprite data (needs to align to a 64 byte block)
;; Registers modified: A, X, Y
  .macro setSpriteCostume n,arg
    .if (.xmatch ({arg}, {A}))
        sta SCREEN_BASE+$3f8+n
    .elseif (.xmatch ({arg}, {X}))
        stx SCREEN_BASE+$3f8+n
    .elseif (.xmatch ({arg}, {Y}))
        sty SCREEN_BASE+$3f8+n
    .else
      .if .const(arg) && arg>255
        lda #((arg) & $3fc0) >> 6   ;values > 255 are interpreted as sprite memory address
      .else
        lda #arg
      .endif
        sta SCREEN_BASE+$3f8+n
    .endif
  .endmacro

;; getSpriteCostume n,reg
;; Get the costume of sprite n into register reg
;; Registers modified: A, X, Y
  .macro getSpriteCostume n,reg
    .ifblank reg
        lda SCREEN_BASE+$3f8+n
    .elseif (.xmatch ({reg}, {A}))
        lda SCREEN_BASE+$3f8+n
    .elseif (.xmatch ({reg}, {X}))
        ldx SCREEN_BASE+$3f8+n
    .elseif (.xmatch ({reg}, {Y}))
        ldy SCREEN_BASE+$3f8+n
    .else
        .error "No proper register stated, must be A, X or Y"
    .endif
  .endmacro

;; setSpriteX n,arg
;; Set the X position of sprite n
;; arg can be a number or AX
;; Registers modified: A, X
  .macro setSpriteX n,arg
    .if (.xmatch ({arg}, {AX}))
        sta VIC_BASE+n+n
        cpx #0
        if ne
            lda VIC_BASE+$10
            ora #1<<(n)
            sta VIC_BASE+$10
        else
            lda VIC_BASE+$10
            and #255 - (1<<(n))
            sta VIC_BASE+$10
        endif
    .else
        lda #<(arg)
        sta VIC_BASE+n+n
      .if arg>255
        lda VIC_BASE+$10
        ora #1<<(n)
        sta VIC_BASE+$10
      .else
        lda VIC_BASE+$10
        and #255 - (1<<(n))
        sta VIC_BASE+$10
      .endif
    .endif
  .endmacro

;; setSpriteY n,arg
;; Set the Y position of sprite n
;; arg can be a number or any register
;; Registers modified: A, X, Y
  .macro setSpriteY n,arg
    .if (.xmatch ({arg}, {A}))
        sta VIC_BASE+n+n+1
    .elseif (.xmatch ({arg}, {X}))
        stx VIC_BASE+n+n+1
    .elseif (.xmatch ({arg}, {Y}))
        sty VIC_BASE+n+n+1
    .else
        ldy #arg
        sty VIC_BASE+n+n+1
    .endif
  .endmacro

;; getSpriteX n,reg
;; Get the X position of sprite n into register reg
;; Registers modified: X
  .macro getSpriteX n,reg
    .if (.xmatch ({arg}, {AX}))
        ldx #00
        lda VIC_BASE+$10
        and #1<<(n)
        if ne
            inx
        endif
        lda VIC_BASE+n+n
    .else
        .error "No proper register stated, must be AX"
    .endif
  .endmacro

;; getSpriteY n,reg
;; Get the Y position of sprite n into register reg
;; Registers modified: A, X, Y
  .macro getSpriteY n,reg
    .if (.xmatch ({reg}, {A}))
        lda VIC_BASE+n+n+1
    .elseif (.xmatch ({reg}, {X}))
        ldx VIC_BASE+n+n+1
    .elseif (.xmatch ({reg}, {Y}))
        ldy VIC_BASE+n+n+1
    .else
        .error "No proper register stated, must be A, X or Y"
    .endif
  .endmacro

;; setSpriteXY n,arg1,arg2
;; Set the X and Y positions of sprite n
;; Registers modified: A, X, Y
  .macro setSpriteXY n,arg1,arg2
        setSpriteY n,arg2
        setSpriteX n,arg1
  .endmacro

;; updateSpriteAttributes n
;; Update attributes for sprite n
;; Registers modified: A, X, Y
  .macro updateSpriteAttributes n
    .ifblank (.xmatch ({reg}, {A}))
     ;
    .elseif (.xmatch ({reg}, {X}))
        txa
    .elseif (.xmatch ({reg}, {Y}))
        tya
    .else
        lda #n
    .endif
.import _updateSpriteAttributes
        jsr _updateSpriteAttributes
  .endmacro

.endif


