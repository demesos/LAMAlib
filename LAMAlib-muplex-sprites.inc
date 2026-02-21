;***********************************************************************
;* LAMAlib-muplex-sprites.inc
;* Sprite Multiplexer Macros
;* by Wil
;* Version 2.3
;*
;* Macro interface for the m_sprmultiplexer module.
;* Drop-in replacement for LAMAlib-sprites.inc when using the multiplexer:
;* same macro names and calling conventions.

.ifndef LAMALIB_MUPLEX_SPRITES_INC
LAMALIB_MUPLEX_SPRITES_INC = 1

;if user has a shadow VIC configuration, the alternative VIC address needs to be set before including this file
.ifndef VIC_BASE
    VIC_BASE = $D000
.endif

; Import symbols from multiplexer module
; Conditional import based on whether Y array is in zero page
.ifndef SPRMUX_NO_ZP
.importzp _msprite_y_ov
.else
.import _msprite_y_ov
.endif

.import _msprite_x_ov
.import _msprite_x_hi_ov
.import _msprite_y_shadow
.import _msprite_costume_ov
.import _msprite_col_ov

;***********************************************************************
;* Basic sprite control macros

;; showSprite n
;; Make sprite n visible by copying shadow Y to active Y
;; n can be a constant, A, X, Y or the string "all" (without quotes)
;; Registers modified: A, when the argument is A or "all" also X is used
.macro showSprite n
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    lda _msprite_y_shadow,x
    sta _msprite_y_ov,x
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    lda _msprite_y_shadow,y
    sta _msprite_y_ov,y
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    tax
    lda _msprite_y_shadow,x
    sta _msprite_y_ov,x
  .elseif (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .importzp _mmaxsprites
    ldx #_mmaxsprites-1
:
    lda _msprite_y_shadow,x
    sta _msprite_y_ov,x
    dex
    bpl :-
  .else ; constant value
    lda _msprite_y_shadow+n
    sta _msprite_y_ov+n
  .endif
.endmacro

;; hideSprite n
;; Hide sprite n by setting Y coordinate to 0
;; n can be a constant, A, X, Y or the string "all" (without quotes)
;; Registers modified: A, when the argument is A or "all" also X is used
.macro hideSprite n
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    lda #0
    sta _msprite_y_ov,x
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    lda #0
    sta _msprite_y_ov,y
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    tax
    lda #0
    sta _msprite_y_ov,x
  .elseif (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .importzp _mmaxsprites
    lda #0
    ldx #_mmaxsprites-1
:
    sta _msprite_y_ov,x
    dex
    bpl :-
  .else ; constant value
    lda #0
    sta _msprite_y_ov+n
  .endif
.endmacro

;***********************************************************************
;* Sprite color macros

;; setSpriteColor n, arg
;; Set color for sprite n
;; For multicolor sprites, bit 7 is preserved (multicolor flag)
;; n can be: constant, A, X, or Y (sprite number)
;; arg can be: immediate value, A, X, or Y (color value)
;; Registers modified: A, potentially X or Y depending on arguments
.macro setSpriteColor n, arg
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    ; Sprite number in X register - use indexed addressing
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      ; Color in A, sprite in X
      sta :+ +1
      lda _msprite_col_ov,x
      and #$80
:     ora #$42
      sta _msprite_col_ov,x
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      ; Color in Y, sprite in X
      tya
      sta :+ +1
      lda _msprite_col_ov,x
      and #$80
:     ora #$42
      sta _msprite_col_ov,x
    .else
      ; Constant color, sprite in X
      lda _msprite_col_ov,x
      and #$80
      ora #arg
      sta _msprite_col_ov,x
    .endif
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    ; Sprite number in Y register - use indexed addressing
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      ; Color in A, sprite in Y
      sta :+ +1
      lda _msprite_col_ov,y
      and #$80
:     ora #$42
      sta _msprite_col_ov,y
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      ; Color in X, sprite in Y
      txa
      sta :+ +1
      lda _msprite_col_ov,y
      and #$80
:     ora #$42
      sta _msprite_col_ov,y
    .else
      ; Constant color, sprite in Y
      lda _msprite_col_ov,y
      and #$80
      ora #arg
      sta _msprite_col_ov,y
    .endif
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    ; Sprite number in A register - move to X for indexed addressing
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      .error "setSpriteColor: cannot use A for both sprite number and color"
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      ; Color in X, sprite# in A
      tay                   ; Save sprite# to Y
      txa                   ; Color to A
      sta :+ +1
      lda _msprite_col_ov,y
      and #$80
:     ora #$42
      sta _msprite_col_ov,y
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      ; Color in Y, sprite# in A
      tax                   ; Sprite# to X
      tya                   ; Color to A
      sta :+ +1
      lda _msprite_col_ov,x
      and #$80
:     ora #$42
      sta _msprite_col_ov,x
    .else
      ; Constant color, sprite# in A
      tax
      lda _msprite_col_ov,x
      and #$80
      ora #arg
      sta _msprite_col_ov,x
    .endif
  .else
    ; Sprite number is constant - use absolute addressing
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      sta :+ +1
      lda _msprite_col_ov+n
      and #$80
:     ora #$42
      sta _msprite_col_ov+n
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      txa
      sta :+ +1
      lda _msprite_col_ov+n
      and #$80
:     ora #$42
      sta _msprite_col_ov+n
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      tya
      sta :+ +1
      lda _msprite_col_ov+n
      and #$80
:     ora #$42
      sta _msprite_col_ov+n
    .else
      lda _msprite_col_ov+n
      and #$80
      ora #arg
      sta _msprite_col_ov+n
    .endif
  .endif
.endmacro

;; getSpriteColor n, reg
;; Get color of sprite n into register
;; Returns color without multicolor bit
;; n can be: constant, A, X, or Y (sprite number)
;; reg must be: A, X, or Y (destination register)
;; Registers modified: A (or specified register)
.macro getSpriteColor n, reg
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    ; Sprite number in X register
    .ifblank reg
      lda _msprite_col_ov,x
      and #$7f
    .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      lda _msprite_col_ov,x
      and #$7f
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      lda _msprite_col_ov,x
      and #$7f
      tax
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      lda _msprite_col_ov,x
      and #$7f
      tay
    .else
      .error "getSpriteColor: register must be A or Y when sprite number is in X"
    .endif
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    ; Sprite number in Y register
    .ifblank reg
      lda _msprite_col_ov,y
      and #$7f
    .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      lda _msprite_col_ov,y
      and #$7f
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      lda _msprite_col_ov,y
      and #$7f
      tax
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      lda _msprite_col_ov,y
      and #$7f
      tay
    .else
      .error "getSpriteColor: register must be A, X or Y when sprite number is in Y"
    .endif
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    ; Sprite number in A register
    .ifblank reg
      tax
      lda _msprite_col_ov,x
      and #$7f
    .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      tax
      lda _msprite_col_ov,x
      and #$7f
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      tay
      lda _msprite_col_ov,y
      and #$7f
      tax
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      tax
      lda _msprite_col_ov,x
      and #$7f
      tay
    .else
      .error "getSpriteColor: register must be A, X or Y"
    .endif
  .else
    ; Sprite number is constant
    .ifblank reg
      lda _msprite_col_ov+n
      and #$7f
    .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      lda _msprite_col_ov+n
      and #$7f
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      lda _msprite_col_ov+n
      and #$7f
      tax
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      lda _msprite_col_ov+n
      and #$7f
      tay
    .else
      .error "getSpriteColor: register must be A, X or Y"
    .endif
  .endif
.endmacro

;; enableMultiColorSprite n
;; Enable multicolor mode for sprite n
;; n can be a constant, A, X, Y or the string "all" (without quotes)
;; Registers modified: A, when the argument is A or "all" also X is used
.macro enableMultiColorSprite n
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    lda _msprite_col_ov,x
    ora #$80
    sta _msprite_col_ov,x
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    lda _msprite_col_ov,y
    ora #$80
    sta _msprite_col_ov,y
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    tax
    lda _msprite_col_ov,x
    ora #$80
    sta _msprite_col_ov,x
  .elseif (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .importzp _mmaxsprites
    ldx #_mmaxsprites-1
:
    lda _msprite_col_ov,x
    ora #$80
    sta _msprite_col_ov,x
    dex
    bpl :-
  .else ; constant value
    lda _msprite_col_ov+n
    ora #$80
    sta _msprite_col_ov+n
  .endif
.endmacro

;; disableMultiColorSprite n
;; Disable multicolor mode for sprite n
;; n can be a constant, A, X, Y or the string "all" (without quotes)
;; Registers modified: A, when the argument is A or "all" also X is used
.macro disableMultiColorSprite n
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    lda _msprite_col_ov,x
    and #$7f
    sta _msprite_col_ov,x
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    lda _msprite_col_ov,y
    and #$7f
    sta _msprite_col_ov,y
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    tax
    lda _msprite_col_ov,x
    and #$7f
    sta _msprite_col_ov,x
  .elseif (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .importzp _mmaxsprites
    ldx #_mmaxsprites-1
:
    lda _msprite_col_ov,x
    and #$7f
    sta _msprite_col_ov,x
    dex
    bpl :-
  .else ; constant value
    lda _msprite_col_ov+n
    and #$7f
    sta _msprite_col_ov+n
  .endif
.endmacro

;; setSpriteMultiColor1 arg
;; Set first shared multicolor
;; arg can be: immediate value, A, X, or Y
;; Registers modified: A
.macro setSpriteMultiColor1 arg
  .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
    sta VIC_BASE+$25
  .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
    stx VIC_BASE+$25
  .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
    sty VIC_BASE+$25
  .else
    lda #arg
    sta VIC_BASE+$25
  .endif
.endmacro

;; getSpriteMultiColor1 reg
;; Get first shared multicolor
;; reg can be: A, X, or Y (defaults to A if blank)
;; Registers modified: A (or specified register)
.macro getSpriteMultiColor1 reg
  .ifblank reg
    lda VIC_BASE+$25
  .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
    lda VIC_BASE+$25
  .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
    ldx VIC_BASE+$25
  .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
    ldy VIC_BASE+$25
  .else
    .error "getSpriteMultiColor1: register must be A, X or Y"
  .endif
.endmacro

;; setSpriteMultiColor2 arg
;; Set second shared multicolor
;; arg can be: immediate value, A, X, or Y
;; Registers modified: A
.macro setSpriteMultiColor2 arg
  .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
    sta VIC_BASE+$26
  .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
    stx VIC_BASE+$26
  .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
    sty VIC_BASE+$26
  .else
    lda #arg
    sta VIC_BASE+$26
  .endif
.endmacro

;; getSpriteMultiColor2 reg
;; Get second shared multicolor
;; reg can be: A, X, or Y (defaults to A if blank)
;; Registers modified: A (or specified register)
.macro getSpriteMultiColor2 reg
  .ifblank reg
    lda VIC_BASE+$26
  .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
    lda VIC_BASE+$26
  .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
    ldx VIC_BASE+$26
  .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
    ldy VIC_BASE+$26
  .else
    .error "getSpriteMultiColor2: register must be A, X or Y"
  .endif
.endmacro

;***********************************************************************
;* Sprite costume macros

;; setSpriteCostume n, arg
;; Set costume (graphic) for sprite n
;; n can be: constant, A, X, or Y (sprite number)
;; arg can be: immediate value, A, X, or Y (costume value)
;; Registers modified: A, potentially X or Y depending on arguments
.macro setSpriteCostume n, arg
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    ; Sprite number in X register
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      sta _msprite_costume_ov,x
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      txa
      sta _msprite_costume_ov,x
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      tya
      sta _msprite_costume_ov,x
    .else
      lda #arg
      sta _msprite_costume_ov,x
    .endif
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    ; Sprite number in Y register
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      sta _msprite_costume_ov,y
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      txa
      sta _msprite_costume_ov,y
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      tya
      sta _msprite_costume_ov,y
    .else
      lda #arg
      sta _msprite_costume_ov,y
    .endif
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    ; Sprite number in A register
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      ; sprite# == costume value: A serves both roles
      tax
      sta _msprite_costume_ov,x
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      ; Swap: sprite# in A, costume in X
      tay
      txa
      sta _msprite_costume_ov,y
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      ; sprite# in A, costume in Y
      tax
      tya
      sta _msprite_costume_ov,x
    .else
      ; sprite# in A, costume is constant
      tax
      lda #arg
      sta _msprite_costume_ov,x
    .endif
  .else
    ; Sprite number is constant
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      sta _msprite_costume_ov+n
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      stx _msprite_costume_ov+n
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      sty _msprite_costume_ov+n
    .else
      lda #arg
      sta _msprite_costume_ov+n
    .endif
  .endif
.endmacro

;; getSpriteCostume n, reg
;; Get costume of sprite n
;; n can be: constant, A, X, or Y (sprite number)
;; reg must be: A, X, or Y (destination register)
;; Registers modified: A (or specified register)
.macro getSpriteCostume n, reg
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    ; Sprite number in X register
    .ifblank reg
      lda _msprite_costume_ov,x
    .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      lda _msprite_costume_ov,x
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      lda _msprite_costume_ov,x
      tax
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      lda _msprite_costume_ov,x
      tay
    .else
      .error "getSpriteCostume: register must be A or Y when sprite number is in X"
    .endif
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    ; Sprite number in Y register
    .ifblank reg
      lda _msprite_costume_ov,y
    .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      lda _msprite_costume_ov,y
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      lda _msprite_costume_ov,y
      tax
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      lda _msprite_costume_ov,y
      tay
    .else
      .error "getSpriteCostume: register must be A or X when sprite number is in Y"
    .endif
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    ; Sprite number in A register
    .ifblank reg
      tax
      lda _msprite_costume_ov,x
    .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      tax
      lda _msprite_costume_ov,x
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      tay
      lda _msprite_costume_ov,y
      tax
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      tax
      lda _msprite_costume_ov,x
      tay
    .else
      .error "getSpriteCostume: register must be A, X or Y"
    .endif
  .else
    ; Sprite number is constant
    .ifblank reg
      lda _msprite_costume_ov+n
    .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      lda _msprite_costume_ov+n
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      ldx _msprite_costume_ov+n
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      ldy _msprite_costume_ov+n
    .else
      .error "getSpriteCostume: register must be A, X or Y"
    .endif
  .endif
.endmacro

;***********************************************************************
;* Sprite position macros

;; setSpriteX n, arg
;; Set X position (0-511) of sprite n
;; n can be: constant, X, Y (sprite number) - NOT A
;; arg can be: 16-bit immediate value, AX, or A (with carry as bit 8)
;; When arg is A: carry flag contains bit 8 of X coordinate
;; When arg is AX: A=low byte, X=high byte, carry is ignored
;; Note: n=X with arg=AX is not supported (X conflict)
;; Registers modified: A, potentially X depending on arguments
.macro setSpriteX n, arg
  .if (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    ; Sprite number in Y register
    .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
      ; AX register pair (A=low, X=high)
      sta _msprite_x_ov,y
      txa
      sta _msprite_x_hi_ov,y
    .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      ; A register with carry as bit 8
      sta _msprite_x_ov,y
      lda #0
      rol
      sta _msprite_x_hi_ov,y
    .else
      ; Constant 16-bit value
      lda #<arg
      sta _msprite_x_ov,y
      lda #>arg
      sta _msprite_x_hi_ov,y
    .endif
  .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    ; Sprite number in X register
    .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
      ; cpx #1 sets carry iff X>=1 (hi byte nonzero), X free for the store after
      cpx #1
      sta _msprite_x_ov,x
      lda #0
      rol
      sta _msprite_x_hi_ov,x
    .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      ; A register with carry as bit 8
      sta _msprite_x_ov,x
      lda #0
      rol
      sta _msprite_x_hi_ov,x
    .else
      ; Constant 16-bit value
      lda #<arg
      sta _msprite_x_ov,x
      lda #>arg
      sta _msprite_x_hi_ov,x
    .endif
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    ; Sprite number in A register - move to Y, arg in A or X is safe
    .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
      ; A=lo, X=hi: save lo, move sprite# to Y
      tay
      sta _msprite_x_ov,y
      txa 
      sta _msprite_x_hi_ov,y
    .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      ; A=lo, carry=bit8, sprite# also in A: same value, coord lo = sprite#
      tay
      sta _msprite_x_ov,y
      lda #0
      rol
      sta _msprite_x_hi_ov,y
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      ; sprite# in A, coord lo in X (carry=bit8)
      tay
      txa
      sta _msprite_x_ov,y
      lda #0
      rol
      sta _msprite_x_hi_ov,y
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      ; sprite# in A, coord lo in Y (carry=bit8)
      tax
      tya
      sta _msprite_x_ov,x
      lda #0
      rol
      sta _msprite_x_hi_ov,x
    .else
      ; Constant 16-bit value, sprite# in A
      tay
      lda #<arg
      sta _msprite_x_ov,y
      lda #>arg
      sta _msprite_x_hi_ov,y
    .endif
  .else
    ; Sprite number is constant
    .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
      ; AX register pair (A=low, X=high)
      sta _msprite_x_ov+n
      stx _msprite_x_hi_ov+n
    .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      ; A register with carry as bit 8
      sta _msprite_x_ov+n
      lda #0
      rol
      sta _msprite_x_hi_ov+n
    .else
      ; Constant 16-bit value
      lda #<arg
      sta _msprite_x_ov+n
      lda #>arg
      sta _msprite_x_hi_ov+n
    .endif
  .endif
.endmacro

;; setSpriteY n, arg
;; Set Y position (0-255) of sprite n
;; Updates both active and shadow Y
;; n can be: constant, A, X, or Y (sprite number)
;; arg can be: immediate value, A, X, or Y (Y coordinate value)
;; Registers modified: A, potentially X or Y depending on arguments
.macro setSpriteY n, arg
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    ; Sprite number in X register
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      sta _msprite_y_ov,x
      sta _msprite_y_shadow,x
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      txa
      sta _msprite_y_ov,x
      sta _msprite_y_shadow,x
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      tya
      sta _msprite_y_ov,x
      sta _msprite_y_shadow,x
    .else
      lda #arg
      sta _msprite_y_ov,x
      sta _msprite_y_shadow,x
    .endif
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    ; Sprite number in Y register
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      sta _msprite_y_ov,y
      sta _msprite_y_shadow,y
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      txa
      sta _msprite_y_ov,y
      sta _msprite_y_shadow,y
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      tya
      sta _msprite_y_ov,y
      sta _msprite_y_shadow,y
    .else
      lda #arg
      sta _msprite_y_ov,y
      sta _msprite_y_shadow,y
    .endif
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    ; Sprite number in A register
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      tax
      sta _msprite_y_ov,x
      sta _msprite_y_shadow,x
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      ; Swap: sprite# in A, Y pos in X
      tay
      txa
      sta _msprite_y_ov,y
      sta _msprite_y_shadow,y
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      ; sprite# in A, Y pos in Y
      tax
      tya
      sta _msprite_y_ov,x
      sta _msprite_y_shadow,x
    .else
      ; sprite# in A, Y pos is constant
      tax
      lda #arg
      sta _msprite_y_ov,x
      sta _msprite_y_shadow,x
    .endif
  .else
    ; Sprite number is constant
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      sta _msprite_y_ov+n
      sta _msprite_y_shadow+n
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
      stx _msprite_y_ov+n
      stx _msprite_y_shadow+n
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
      sty _msprite_y_ov+n
      sty _msprite_y_shadow+n
    .else
      lda #arg
      sta _msprite_y_ov+n
      sta _msprite_y_shadow+n
    .endif
  .endif
.endmacro

;; getSpriteX n, reg
;; Get X position of sprite n
;; n can be: constant, A, X, or Y (sprite number)
;; reg must be: AX (A=low byte, X=high byte)
;; Registers modified: A, X, Y (when X is used as sprite index)
.macro getSpriteX n, reg
  .if (.xmatch ({reg}, {AX}) .or .xmatch ({reg}, {ax}))
    .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      ; Sprite number in X - move to Y, then use n=Y path (no stack needed)
      txa
      tay
:     lda _msprite_x_hi_ov,y
      tax
      lda _msprite_x_ov,y
      cmp _msprite_x_ov,y
      bne :-
      ; A=lo, X=hi
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      ; Sprite number in Y
      ; Note: cpx abs,y is illegal on 6502, use lda+cmp for race check
:     lda _msprite_x_hi_ov,y
      tax
      lda _msprite_x_ov,y
      cmp _msprite_x_ov,y     ; re-read lo byte to detect IRQ mid-update
      bne :-
      ; A=lo, X=hi
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      ; Sprite number in A
      tay
:     lda _msprite_x_hi_ov,y
      tax
      lda _msprite_x_ov,y
      cmp _msprite_x_ov,y
      bne :-
      ; A=lo, X=hi
    .else
      ; Constant sprite number
:     ldx _msprite_x_hi_ov+n
      lda _msprite_x_ov+n
      cpx _msprite_x_hi_ov+n
      bne :-
    .endif
  .else
    .error "getSpriteX requires AX register pair"
  .endif
.endmacro

;; getSpriteY n, reg
;; Get Y position of sprite n
;; n can be: constant, A, X, or Y (sprite number)
;; reg must be: A, X, or Y (destination register)
;; Registers modified: A (or specified register)
;; Note: when n=X, reg can be A or Y only (X holds sprite index)
;;       when n=Y, all output registers supported (lda ,y + transfer)
.macro getSpriteY n, reg
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    ; Sprite number in X register
    .if (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      lda _msprite_y_shadow,x
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      lda _msprite_y_shadow,x
      tax
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      lda _msprite_y_shadow,x
      tay
    .else
      .error "getSpriteY: register must be A, X or Y when sprite number is in X"
    .endif
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    ; Sprite number in Y register
    .if (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      lda _msprite_y_shadow,y
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      lda _msprite_y_shadow,y
      tax
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      lda _msprite_y_shadow,y
      tay
    .else
      .error "getSpriteY: register must be A, X or Y when sprite number is in Y"
    .endif
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    ; Sprite number in A register
    .if (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      tax
      lda _msprite_y_shadow,x
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      tay
      lda _msprite_y_shadow,y
      tax
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      tax
      lda _msprite_y_shadow,x
      tay
    .else
      .error "getSpriteY: register must be A, X or Y"
    .endif
  .else
    ; Sprite number is constant
    .if (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
      lda _msprite_y_shadow+n
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
      ldx _msprite_y_shadow+n
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
      ldy _msprite_y_shadow+n
    .else
      .error "getSpriteY: register must be A, X or Y"
    .endif
  .endif
.endmacro

;; setSpriteXY n, xpos, ypos
;; Set both X and Y position of sprite n
;; n can be: constant, A, X, Y
;; xpos can be: 16-bit immediate value, AX, or A (with carry)
;; ypos can be: immediate value, A, X, or Y
;; Registers modified: A, potentially X or Y depending on arguments
.macro setSpriteXY n, xpos, ypos
  setSpriteY n, ypos
  setSpriteX n, xpos
.endmacro

;***********************************************************************
;* Overlay sprite macros

;; setOverlayColor n, color
;; Set overlay color for sprite n (enables overlay mode)
;; Set to 0 to disable overlay
;; n can be: constant, A, X, or Y (sprite number)
;; color can be: immediate value, A, X, or Y
;; Registers modified: A, potentially X or Y depending on arguments
.macro setOverlayColor n, color
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    ; Sprite number in X register
    .if (.xmatch ({color}, {A}) .or .xmatch ({color}, {a}))
      sta _msprite_col_overlay_ov,x
    .elseif (.xmatch ({color}, {Y}) .or .xmatch ({color}, {y}))
      tya
      sta _msprite_col_overlay_ov,x
    .else
      lda #color
      sta _msprite_col_overlay_ov,x
    .endif
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    ; Sprite number in Y register
    .if (.xmatch ({color}, {A}) .or .xmatch ({color}, {a}))
      sta _msprite_col_overlay_ov,y
    .elseif (.xmatch ({color}, {X}) .or .xmatch ({color}, {x}))
      txa
      sta _msprite_col_overlay_ov,y
    .else
      lda #color
      sta _msprite_col_overlay_ov,y
    .endif
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    ; Sprite number in A register
    .if (.xmatch ({color}, {A}) .or .xmatch ({color}, {a}))
      .error "setOverlayColor: cannot use A for both sprite number and color"
    .elseif (.xmatch ({color}, {X}) .or .xmatch ({color}, {x}))
      tay
      txa
      sta _msprite_col_overlay_ov,y
    .elseif (.xmatch ({color}, {Y}) .or .xmatch ({color}, {y}))
      tax
      tya
      sta _msprite_col_overlay_ov,x
    .else
      tax
      lda #color
      sta _msprite_col_overlay_ov,x
    .endif
  .else
    ; Sprite number is constant
    .if (.xmatch ({color}, {A}) .or .xmatch ({color}, {a}))
      sta _msprite_col_overlay_ov+n
    .elseif (.xmatch ({color}, {X}) .or .xmatch ({color}, {x}))
      stx _msprite_col_overlay_ov+n
    .elseif (.xmatch ({color}, {Y}) .or .xmatch ({color}, {y}))
      sty _msprite_col_overlay_ov+n
    .else
      lda #color
      sta _msprite_col_overlay_ov+n
    .endif
  .endif
.endmacro

;; disableOverlay n
;; Disable overlay mode for sprite n
;; n can be: constant, A, X, or Y (sprite number)
;; Registers modified: A, potentially X depending on argument
.macro disableOverlay n
  .if (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    lda #0
    sta _msprite_col_overlay_ov,x
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    lda #0
    sta _msprite_col_overlay_ov,y
  .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    tax
    lda #0
    sta _msprite_col_overlay_ov,x
  .else
    lda #0
    sta _msprite_col_overlay_ov+n
  .endif
.endmacro

;***********************************************************************
;* Grounded sprite macros

;; setGroundedSprite n
;; Mark sprite n as grounded (always below others)
;; n can be: constant, A, X, or Y
;; Registers modified: A
.macro setGroundedSprite n
  .if (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    sta _msprite_grounded_ov
  .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    stx _msprite_grounded_ov
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    sty _msprite_grounded_ov
  .else
    lda #n
    sta _msprite_grounded_ov
  .endif
.endmacro

;; unsetGroundedSprite
;; Clear grounded sprite flag
;; Registers modified: A
.macro unsetGroundedSprite
  lda #$ff
  sta _msprite_grounded_ov
.endmacro

;***********************************************************************
;* Optional feature macros

;; updateSpriteAttributes n
;; Update sprite n's attributes (requires ENABLE_UPDATE_ATTRIBUTES=1 in module)
;; n can be: constant, A, X, or Y
;; Pass sprite number in A register to the routine
;; Registers modified: A, X, Y
.macro updateSpriteAttributes n
  .import _sprmultiplexer_UpdateSpriteAttributes
  .if (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    ; Already in A
  .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    txa
  .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    tya
  .else
    lda #n
  .endif
  jsr _sprmultiplexer_UpdateSpriteAttributes
.endmacro

;***********************************************************************
;* VIC register batch macros (all hardware sprites at once)
;* These operate on real VIC hardware registers - only "all" is supported
;* since the multiplexer assigns logical->hardware sprites dynamically.

;; enableXexpandSprite all
;; Enable horizontal doubling for all hardware sprites
;; Registers modified: A
.macro enableXexpandSprite n
  .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    lda #$ff
    sta VIC_BASE+$1d
  .else
    .error "enableXexpandSprite: only 'all' is supported with the multiplexer"
  .endif
.endmacro

;; disableXexpandSprite all
;; Disable horizontal doubling for all hardware sprites
;; Registers modified: A
.macro disableXexpandSprite n
  .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    lda #0
    sta VIC_BASE+$1d
  .else
    .error "disableXexpandSprite: only 'all' is supported with the multiplexer"
  .endif
.endmacro

;; enableYexpandSprite all
;; Enable vertical doubling for all hardware sprites
;; Registers modified: A
.macro enableYexpandSprite n
  .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    lda #$ff
    sta VIC_BASE+$17
  .else
    .error "enableYexpandSprite: only 'all' is supported with the multiplexer"
  .endif
.endmacro

;; disableYexpandSprite all
;; Disable vertical doubling for all hardware sprites
;; Registers modified: A
.macro disableYexpandSprite n
  .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    lda #0
    sta VIC_BASE+$17
  .else
    .error "disableYexpandSprite: only 'all' is supported with the multiplexer"
  .endif
.endmacro

;; spriteBeforeBackground all
;; Place all hardware sprites in front of the background
;; Registers modified: A
.macro spriteBeforeBackground n
  .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    lda #0
    sta VIC_BASE+$1b
  .else
    .error "spriteBeforeBackground: only 'all' is supported with the multiplexer"
  .endif
.endmacro

;; spriteBehindBackground all
;; Place all hardware sprites behind the background
;; Registers modified: A
.macro spriteBehindBackground n
  .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    lda #$ff
    sta VIC_BASE+$1b
  .else
    .error "spriteBehindBackground: only 'all' is supported with the multiplexer"
  .endif
.endmacro

.endif  ; LAMALIB_MUPLEX_SPRITES_INC
