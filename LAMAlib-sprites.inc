;; <h2>Assembler macros to control VIC sprites (no multiplexing)</h2>
;; To use these macros, they need to be included separately with <pre>
;; .include "LAMAlib-sprites.inc"
;; </pre>
;;
;; Version 2.2
;; - Full register argument support (A, X, Y) for all sprite-number arguments
;; - Fixed getSpriteX: was checking {arg} instead of {reg} (always errored)
;; - Fixed updateSpriteAttributes: was checking {reg} instead of {n}
;; - Added A (with carry as bit 8) form to setSpriteX
;; - setSpriteY const: use lda/sta instead of ldy/sty (avoids clobbering Y)
;; - Bit-register macros (show/hide, multicolor, expand, priority):
;;   runtime bit computation for A/X/Y/all sprite number forms

.if .not .definedmacro(showSprite)

  .ifndef VIC_BASE
VIC_BASE=$D000
  .endif

_overlay_implicit=0      ;value indicates that with this sprite macro overlay sprites need to be set and placed explicitely

;***********************************************************************
;; showSprite n
;; Make sprite n visible
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A
  .macro showSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
      lda #$ff
      sta VIC_BASE+$15
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      ; compute 1<<A: use A as counter, shift bit in X
      tax
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$15
      sta VIC_BASE+$15
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$15
      sta VIC_BASE+$15
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      lda #1
:     dey
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$15
      sta VIC_BASE+$15
    .else
      lda VIC_BASE+$15
      ora #1<<(n)
      sta VIC_BASE+$15
    .endif
  .endmacro

;***********************************************************************
;; hideSprite n
;; Hide sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A
  .macro hideSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
      lda #0
      sta VIC_BASE+$15
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      tax
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$15
      sta VIC_BASE+$15
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$15
      sta VIC_BASE+$15
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      lda #1
:     dey
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$15
      sta VIC_BASE+$15
    .else
      lda VIC_BASE+$15
      and #255 - (1<<(n))
      sta VIC_BASE+$15
    .endif
  .endmacro

;***********************************************************************
;; enableMultiColorSprite n
;; Enable multicolor mode for sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A
  .macro enableMultiColorSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
      lda #$ff
      sta VIC_BASE+$1c
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      tax
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$1c
      sta VIC_BASE+$1c
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$1c
      sta VIC_BASE+$1c
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      lda #1
:     dey
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$1c
      sta VIC_BASE+$1c
    .else
      lda VIC_BASE+$1c
      ora #1<<(n)
      sta VIC_BASE+$1c
    .endif
  .endmacro

;***********************************************************************
;; disableMultiColorSprite n
;; Disable multicolor mode for sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A
  .macro disableMultiColorSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
      lda #0
      sta VIC_BASE+$1c
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      tax
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$1c
      sta VIC_BASE+$1c
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$1c
      sta VIC_BASE+$1c
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      lda #1
:     dey
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$1c
      sta VIC_BASE+$1c
    .else
      lda VIC_BASE+$1c
      and #255 - (1<<(n))
      sta VIC_BASE+$1c
    .endif
  .endmacro

;***********************************************************************
;; enableXexpandSprite n
;; Enable horizontal expansion for sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A
  .macro enableXexpandSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
      lda #$ff
      sta VIC_BASE+$1d
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      tax
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$1d
      sta VIC_BASE+$1d
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$1d
      sta VIC_BASE+$1d
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      lda #1
:     dey
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$1d
      sta VIC_BASE+$1d
    .else
      lda VIC_BASE+$1d
      ora #1<<(n)
      sta VIC_BASE+$1d
    .endif
  .endmacro

;***********************************************************************
;; disableXexpandSprite n
;; Disable horizontal expansion for sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A
  .macro disableXexpandSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
      lda #0
      sta VIC_BASE+$1d
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      tax
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$1d
      sta VIC_BASE+$1d
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$1d
      sta VIC_BASE+$1d
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      lda #1
:     dey
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$1d
      sta VIC_BASE+$1d
    .else
      lda VIC_BASE+$1d
      and #255 - (1<<(n))
      sta VIC_BASE+$1d
    .endif
  .endmacro

;***********************************************************************
;; enableYexpandSprite n
;; Enable vertical expansion for sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A
  .macro enableYexpandSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
      lda #$ff
      sta VIC_BASE+$17
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      tax
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$17
      sta VIC_BASE+$17
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$17
      sta VIC_BASE+$17
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      lda #1
:     dey
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$17
      sta VIC_BASE+$17
    .else
      lda VIC_BASE+$17
      ora #1<<(n)
      sta VIC_BASE+$17
    .endif
  .endmacro

;***********************************************************************
;; disableYexpandSprite n
;; Disable vertical expansion for sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A
  .macro disableYexpandSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
      lda #0
      sta VIC_BASE+$17
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      tax
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$17
      sta VIC_BASE+$17
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$17
      sta VIC_BASE+$17
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      lda #1
:     dey
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$17
      sta VIC_BASE+$17
    .else
      lda VIC_BASE+$17
      and #255 - (1<<(n))
      sta VIC_BASE+$17
    .endif
  .endmacro

;***********************************************************************
;; spriteBeforeBackground n
;; Make sprite n appear in front of the background
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A
  .macro spriteBeforeBackground n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
      lda #0
      sta VIC_BASE+$1b
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      tax
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$1b
      sta VIC_BASE+$1b
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$1b
      sta VIC_BASE+$1b
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      lda #1
:     dey
      bmi :+
      asl
      bne :-
:     eor #$ff
      and VIC_BASE+$1b
      sta VIC_BASE+$1b
    .else
      lda VIC_BASE+$1b
      and #255 - (1<<(n))
      sta VIC_BASE+$1b
    .endif
  .endmacro

;***********************************************************************
;; spriteBehindBackground n
;; Make sprite n appear behind the background
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A
  .macro spriteBehindBackground n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
      lda #$ff
      sta VIC_BASE+$1b
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      tax
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$1b
      sta VIC_BASE+$1b
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      lda #1
:     dex
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$1b
      sta VIC_BASE+$1b
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      lda #1
:     dey
      bmi :+
      asl
      bne :-
:     ora VIC_BASE+$1b
      sta VIC_BASE+$1b
    .else
      lda VIC_BASE+$1b
      ora #1<<(n)
      sta VIC_BASE+$1b
    .endif
  .endmacro

;***********************************************************************
;; setSpriteColor n, arg
;; Set the color for sprite n
;; n can be: constant
;; arg can be: A, X, Y, or constant
;; Registers modified: A (or specified register)
  .macro setSpriteColor n, arg
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        sta VIC_BASE+$27+n
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
        stx VIC_BASE+$27+n
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
        sty VIC_BASE+$27+n
    .else
        lda #arg
        sta VIC_BASE+$27+n
    .endif
  .endmacro

;***********************************************************************
;; getSpriteColor n, reg
;; Get the color of sprite n into register reg
;; n can be: constant
;; reg can be: A, X, Y (defaults to A)
;; Registers modified: A, X, or Y (depending on reg)
  .macro getSpriteColor n, reg
    .ifblank reg
        lda VIC_BASE+$27+n
    .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        lda VIC_BASE+$27+n
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        ldx VIC_BASE+$27+n
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        ldy VIC_BASE+$27+n
    .else
        .error "getSpriteColor: register must be A, X or Y"
    .endif
  .endmacro

;***********************************************************************
;; setSpriteMultiColor1 arg
;; Set the first multicolor for sprites
;; arg can be: A, X, Y, or constant
;; Registers modified: A (or specified register)
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

;***********************************************************************
;; getSpriteMultiColor1 reg
;; Get the first multicolor of sprites into register reg
;; reg can be: A, X, Y (defaults to A)
;; Registers modified: A, X, or Y (depending on reg)
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

;***********************************************************************
;; setSpriteMultiColor2 arg
;; Set the second multicolor for sprites
;; arg can be: A, X, Y, or constant
;; Registers modified: A (or specified register)
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

;***********************************************************************
;; getSpriteMultiColor2 reg
;; Get the second multicolor of sprites into register reg
;; reg can be: A, X, Y (defaults to A)
;; Registers modified: A, X, or Y (depending on reg)
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
;; setSpriteCostume n, arg
;; Set the costume for sprite n
;; If arg is a register or a numerical value below 256, interpreted as sprite costume block
;; If arg is a value > 255, interpreted as the absolute address of the sprite data
;;   (needs to align to a 64 byte block)
;; n can be: constant
;; arg can be: A, X, Y, or constant
;; Registers modified: A (or specified register)
  .macro setSpriteCostume n, arg
    .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        sta SCREEN_BASE+$3f8+n
    .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
        stx SCREEN_BASE+$3f8+n
    .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
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

;***********************************************************************
;; getSpriteCostume n, reg
;; Get the costume of sprite n into register reg
;; n can be: constant
;; reg can be: A, X, Y (defaults to A)
;; Registers modified: A, X, or Y (depending on reg)
  .macro getSpriteCostume n, reg
    .ifblank reg
        lda SCREEN_BASE+$3f8+n
    .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        lda SCREEN_BASE+$3f8+n
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        ldx SCREEN_BASE+$3f8+n
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        ldy SCREEN_BASE+$3f8+n
    .else
        .error "getSpriteCostume: register must be A, X or Y"
    .endif
  .endmacro

;***********************************************************************
;; setSpriteX n, arg
;; Set the X position of sprite n
;; n can be: constant, X, Y
;; arg can be: AX (A=low byte, X=high byte),
;;             A  (carry flag = bit 8 of X coordinate),
;;             or 16-bit constant
;; Registers modified: A if n is constant, otherwise clobbers the other index register
  .macro setSpriteX n, arg

    .if (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      ; --- n in Y ---
      .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
	cpx #1  ;set carry if X>=1
        .import _doubles_0_to_7
        ldx _doubles_0_to_7,y        ;x=2*y        
        sta VIC_BASE+0,x
        lda VIC_BASE+$10
        if cs
            .import _twopotentials
            ora _twopotentials,y
        else
            .import _maskedtwopotentials
            and _maskedtwopotentials,y
        endif
        sta VIC_BASE+$10
      .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        ; A=lo, carry=bit8
        .import _doubles_0_to_7
        ldx _doubles_0_to_7,y        ;x=2*y        
        sta VIC_BASE+0,x
        lda VIC_BASE+$10
        if cs
            .import _twopotentials
            ora _twopotentials,y
        else
            .import _maskedtwopotentials
            and _maskedtwopotentials,y
        endif
        sta VIC_BASE+$10
      .else
        ; constant arg — sprite n in Y, arg known at assemble time
        .import _doubles_0_to_7
        ldx _doubles_0_to_7,y        ;x=2*y        
	lda #<arg
        sta VIC_BASE+0,x
        lda VIC_BASE+$10
        .if arg>255
            .import _twopotentials
            ora _twopotentials,y
        .else
            .import _maskedtwopotentials
            and _maskedtwopotentials,y
        .endif
        sta VIC_BASE+$10
      .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      ; n in X: sprite X lo at VIC_BASE + n*2 (X-indexed after doubling)
      .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
        .error "setSpriteX: n=X with arg=AX conflicts (X used for both)"
      .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        ; A=lo, carry=bit8. X=sprite n.
        .import _doubles_0_to_7
        ldy _doubles_0_to_7,x        ;y=2*x        
        sta VIC_BASE+0,y
        lda VIC_BASE+$10
        if cs
            .import _twopotentials
            ora _twopotentials,x
        else
            .import _maskedtwopotentials
            and _maskedtwopotentials,x
        endif
        sta VIC_BASE+$10
      .else
        ; constant arg — sprite n in x, arg known at assemble time
        .import _doubles_0_to_7
        ldy _doubles_0_to_7,x        ;y=2*x        
	lda #<arg
        sta VIC_BASE+0,y
        lda VIC_BASE+$10
        .if arg>255
            .import _twopotentials
            ora _twopotentials,x
        .else
            .import _maskedtwopotentials
            and _maskedtwopotentials,x
        .endif
        sta VIC_BASE+$10
      .endif
    .else
      ; --- n is a constant ---
      .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
        sta VIC_BASE+n+n
        lda VIC_BASE+$10
        cpx #0
        if ne
            ora #1<<(n)
        else
            and #255 - (1<<(n))
        endif
        sta VIC_BASE+$10
      .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        sta VIC_BASE+n+n
        lda VIC_BASE+$10
        if cs
            ora #1<<(n)
        else
            and #255 - (1<<(n))
        endif
        sta VIC_BASE+$10
      .else
        lda #<(arg)
        sta VIC_BASE+n+n
        lda VIC_BASE+$10
        .if arg>255
          ora #1<<(n)
        .else
          and #255 - (1<<(n))
        .endif
        sta VIC_BASE+$10
      .endif
    .endif
  .endmacro

;***********************************************************************
;; setSpriteY n, arg
;; Set the Y position of sprite n
;; n can be: constant
;; arg can be: A, X, Y, or constant
;; Registers modified: none if n is constant, otherwise clobbers the other index register
  .macro setSpriteY n, arg
    .if (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      ; --- n in Y --- 
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
          .import _doubles_0_to_7
          ldx _doubles_0_to_7,y	    ;x=2*y
          sta VIC_BASE+1,x
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
          txa
          .import _doubles_0_to_7
          ldx _doubles_0_to_7,y	    ;x=2*y
          sta VIC_BASE+1,x
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
          .error "setSpriteY: n=Y with arg=Y conflicts (Y used for both)"
      .else
          .import _doubles_0_to_7
          ldx _doubles_0_to_7,y	    ;x=2*y
          lda #arg
          sta VIC_BASE+1,x
      .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      ; --- n in X --- 
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
          .import _doubles_0_to_7
          ldy _doubles_0_to_7,x	    ;y=2*x
          sta VIC_BASE+1,y
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
          .error "setSpriteY: n=X with arg=X conflicts (X used for both)"
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
          tya
          .import _doubles_0_to_7
          ldy _doubles_0_to_7,x	    ;y=2*x
          sta VIC_BASE+1,y
      .else
          .import _doubles_0_to_7
          ldy _doubles_0_to_7,x	    ;y=2*x
          lda #arg
          sta VIC_BASE+1,y
      .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      ; --- n in A --- 
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
          .error "setSpriteY: n=A with arg=A conflicts (A used for both)"
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
          tay
	  txa
          .import _doubles_0_to_7
          ldx _doubles_0_to_7,y	    ;x=2*y
          sta VIC_BASE+1,x
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
          tax
	  tya
          .import _doubles_0_to_7
          ldy _doubles_0_to_7,x     ;y=2*x
          sta VIC_BASE+1,y
      .else
          .import _doubles_0_to_7
	  tay
          ldx _doubles_0_to_7,y	    ;x=2*y
          lda #arg
          sta VIC_BASE+1,x
      .endif
    .else
      ; --- n is a constant ---
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
          sta VIC_BASE+n+n+1
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
          stx VIC_BASE+n+n+1
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
          sty VIC_BASE+n+n+1
      .else
          lda #arg
          sta VIC_BASE+n+n+1
      .endif
    .endif
  .endmacro

;***********************************************************************
;; getSpriteX n, reg
;; Get the X position of sprite n
;; n can be: constant
;; reg must be: AX (A=low byte, X=high byte / bit 8)
;; Registers modified: A, X
  .macro getSpriteX n, reg
    .if (.xmatch ({reg}, {AX}) .or .xmatch ({reg}, {ax}))
        ldx #0
        lda VIC_BASE+$10
        and #1<<(n)
        if ne
            inx
        endif
        lda VIC_BASE+n+n
    .else
        .error "getSpriteX: register must be AX"
    .endif
  .endmacro

;***********************************************************************
;; getSpriteY n, reg
;; Get the Y position of sprite n into register reg
;; n can be: constant
;; reg can be: A, X, Y
;; Registers modified: A, X, or Y (depending on reg)
  .macro getSpriteY n, reg
    .if (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        lda VIC_BASE+n+n+1
    .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        ldx VIC_BASE+n+n+1
    .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        ldy VIC_BASE+n+n+1
    .else
        .error "getSpriteY: register must be A, X or Y"
    .endif
  .endmacro

;***********************************************************************
;; setSpriteXY n, xpos, ypos
;; Set the X and Y positions of sprite n
;; n can be: constant
;; xpos: see setSpriteX
;; ypos: see setSpriteY
;; Registers modified: A, X, Y
  .macro setSpriteXY n, xpos, ypos
        setSpriteY n, ypos
        setSpriteX n, xpos
  .endmacro

;***********************************************************************
;; updateSpriteAttributes n
;; No-op for hardware sprites - attributes are written directly to VIC
;; registers. Provided for source compatibility with the multiplexer version.
;; n can be: constant, A, X, Y
;; Registers modified: none
  .macro updateSpriteAttributes n
      ; no-op: hardware sprites update VIC registers directly
  .endmacro

.endif


