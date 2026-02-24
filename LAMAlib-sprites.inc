;; <h1>Sprite Macros (Direct VIC)</h1>
;; Assembler macros to control VIC sprites directly (no multiplexing).
;; Include separately with:
;; .include "LAMAlib-sprites.inc"
;;
;; Version 2.4, February 2026

.if .not .definedmacro(showSprite)

.ifndef SAVE_REGS
  ::SAVE_REGS .set 0
.endif

;if user has a shadow VIC configuration, the alternative VIC address needs to be set before including this file
.ifndef VIC_BASE
    VIC_BASE = $D000
.endif

_overlay_implicit=0      ;value indicates that with this sprite macro overlay sprites need to be set and placed explicitely

;***********************************************************************
;; <h2>Visibility</h2>
;;
;; showSprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A or n=Y
  .macro showSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .if ::SAVE_REGS>1
    store A
    .endif
      lda #$ff
      sta VIC_BASE+$15
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    .if ::SAVE_REGS>1
    store AX
    .endif
      tax
      .import _twopotentials
      lda _twopotentials,x
      ora VIC_BASE+$15
      sta VIC_BASE+$15
    .if ::SAVE_REGS>1
    restore AX
    .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _twopotentials
      lda _twopotentials,x
      ora VIC_BASE+$15
      sta VIC_BASE+$15
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _twopotentials
      lda _twopotentials,y
      ora VIC_BASE+$15
      sta VIC_BASE+$15
    .if ::SAVE_REGS>1
    restore A
    .endif
    .else
    .if ::SAVE_REGS>1
    store A
    .endif
      lda VIC_BASE+$15
      ora #1<<(n)
      sta VIC_BASE+$15
    .if ::SAVE_REGS>1
    restore A
    .endif
    .endif
  .endmacro

;***********************************************************************
;; hideSprite n
;; Hide sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A or n=Y
  .macro hideSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .if ::SAVE_REGS>1
    store A
    .endif
      lda #0
      sta VIC_BASE+$15
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    .if ::SAVE_REGS>1
    store AX
    .endif
      tax
      .import _maskedtwopotentials
      lda _maskedtwopotentials,x
      and VIC_BASE+$15
      sta VIC_BASE+$15
    .if ::SAVE_REGS>1
    restore AX
    .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _maskedtwopotentials
      lda _maskedtwopotentials,x
      and VIC_BASE+$15
      sta VIC_BASE+$15
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _maskedtwopotentials
      lda _maskedtwopotentials,y
      and VIC_BASE+$15
      sta VIC_BASE+$15
    .if ::SAVE_REGS>1
    restore A
    .endif
    .else
    .if ::SAVE_REGS>1
    store A
    .endif
      lda VIC_BASE+$15
      and #255 - (1<<(n))
      sta VIC_BASE+$15
    .if ::SAVE_REGS>1
    restore A
    .endif
    .endif
  .endmacro

;***********************************************************************
;; <h2>Attributes</h2>
;; Sprite attribute flags: multicolor mode, size expansion, display priority.
;;
;; enableMultiColorSprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A or n=Y
  .macro enableMultiColorSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .if ::SAVE_REGS>1
    store A
    .endif
      lda #$ff
      sta VIC_BASE+$1c
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    .if ::SAVE_REGS>1
    store AX
    .endif
      tax
      .import _twopotentials
      lda _twopotentials,x
      ora VIC_BASE+$1c
      sta VIC_BASE+$1c
    .if ::SAVE_REGS>1
    restore AX
    .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _twopotentials
      lda _twopotentials,x
      ora VIC_BASE+$1c
      sta VIC_BASE+$1c
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _twopotentials
      lda _twopotentials,y
      ora VIC_BASE+$1c
      sta VIC_BASE+$1c
    .if ::SAVE_REGS>1
    restore A
    .endif
    .else
    .if ::SAVE_REGS>1
    store A
    .endif
      lda VIC_BASE+$1c
      ora #1<<(n)
      sta VIC_BASE+$1c
    .if ::SAVE_REGS>1
    restore A
    .endif
    .endif
  .endmacro

;***********************************************************************
;; disableMultiColorSprite n
;; Disable multicolor mode for sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A or n=Y
  .macro disableMultiColorSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .if ::SAVE_REGS>1
    store A
    .endif
      lda #0
      sta VIC_BASE+$1c
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    .if ::SAVE_REGS>1
    store AX
    .endif
      tax
      .import _maskedtwopotentials
      lda _maskedtwopotentials,x
      and VIC_BASE+$1c
      sta VIC_BASE+$1c
    .if ::SAVE_REGS>1
    restore AX
    .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _maskedtwopotentials
      lda _maskedtwopotentials,x
      and VIC_BASE+$1c
      sta VIC_BASE+$1c
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _maskedtwopotentials
      lda _maskedtwopotentials,y
      and VIC_BASE+$1c
      sta VIC_BASE+$1c
    .if ::SAVE_REGS>1
    restore A
    .endif
    .else
    .if ::SAVE_REGS>1
    store A
    .endif
      lda VIC_BASE+$1c
      and #255 - (1<<(n))
      sta VIC_BASE+$1c
    .if ::SAVE_REGS>1
    restore A
    .endif
    .endif
  .endmacro

;***********************************************************************
;; enableXexpandSprite n
;; Enable horizontal expansion for sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A or n=Y
  .macro enableXexpandSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .if ::SAVE_REGS>1
    store A
    .endif
      lda #$ff
      sta VIC_BASE+$1d
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    .if ::SAVE_REGS>1
    store AX
    .endif
      tax
      .import _twopotentials
      lda _twopotentials,x
      ora VIC_BASE+$1d
      sta VIC_BASE+$1d
    .if ::SAVE_REGS>1
    restore AX
    .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _twopotentials
      lda _twopotentials,x
      ora VIC_BASE+$1d
      sta VIC_BASE+$1d
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _twopotentials
      lda _twopotentials,y
      ora VIC_BASE+$1d
      sta VIC_BASE+$1d
    .if ::SAVE_REGS>1
    restore A
    .endif
    .else
    .if ::SAVE_REGS>1
    store A
    .endif
      lda VIC_BASE+$1d
      ora #1<<(n)
      sta VIC_BASE+$1d
    .if ::SAVE_REGS>1
    restore A
    .endif
    .endif
  .endmacro

;***********************************************************************
;; disableXexpandSprite n
;; Disable horizontal expansion for sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A or n=Y
  .macro disableXexpandSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .if ::SAVE_REGS>1
    store A
    .endif
      lda #0
      sta VIC_BASE+$1d
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    .if ::SAVE_REGS>1
    store AX
    .endif
      tax
      .import _maskedtwopotentials
      lda _maskedtwopotentials,x
      and VIC_BASE+$1d
      sta VIC_BASE+$1d
    .if ::SAVE_REGS>1
    restore AX
    .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _maskedtwopotentials
      lda _maskedtwopotentials,x
      and VIC_BASE+$1d
      sta VIC_BASE+$1d
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _maskedtwopotentials
      lda _maskedtwopotentials,y
      and VIC_BASE+$1d
      sta VIC_BASE+$1d
    .if ::SAVE_REGS>1
    restore A
    .endif
    .else
    .if ::SAVE_REGS>1
    store A
    .endif
      lda VIC_BASE+$1d
      and #255 - (1<<(n))
      sta VIC_BASE+$1d
    .if ::SAVE_REGS>1
    restore A
    .endif
    .endif
  .endmacro

;***********************************************************************
;; enableYexpandSprite n
;; Enable vertical expansion for sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A or n=Y
  .macro enableYexpandSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .if ::SAVE_REGS>1
    store A
    .endif
      lda #$ff
      sta VIC_BASE+$17
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    .if ::SAVE_REGS>1
    store AX
    .endif
      tax
      .import _twopotentials
      lda _twopotentials,x
      ora VIC_BASE+$17
      sta VIC_BASE+$17
    .if ::SAVE_REGS>1
    restore AX
    .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _twopotentials
      lda _twopotentials,x
      ora VIC_BASE+$17
      sta VIC_BASE+$17
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _twopotentials
      lda _twopotentials,y
      ora VIC_BASE+$17
      sta VIC_BASE+$17
    .if ::SAVE_REGS>1
    restore A
    .endif
    .else
    .if ::SAVE_REGS>1
    store A
    .endif
      lda VIC_BASE+$17
      ora #1<<(n)
      sta VIC_BASE+$17
    .if ::SAVE_REGS>1
    restore A
    .endif
    .endif
  .endmacro

;***********************************************************************
;; disableYexpandSprite n
;; Disable vertical expansion for sprite n
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A or n=Y
  .macro disableYexpandSprite n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .if ::SAVE_REGS>1
    store A
    .endif
      lda #0
      sta VIC_BASE+$17
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    .if ::SAVE_REGS>1
    store AX
    .endif
      tax
      .import _maskedtwopotentials
      lda _maskedtwopotentials,x
      and VIC_BASE+$17
      sta VIC_BASE+$17
    .if ::SAVE_REGS>1
    restore AX
    .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _maskedtwopotentials
      lda _maskedtwopotentials,x
      and VIC_BASE+$17
      sta VIC_BASE+$17
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _maskedtwopotentials
      lda _maskedtwopotentials,y
      and VIC_BASE+$17
      sta VIC_BASE+$17
    .if ::SAVE_REGS>1
    restore A
    .endif
    .else
    .if ::SAVE_REGS>1
    store A
    .endif
      lda VIC_BASE+$17
      and #255 - (1<<(n))
      sta VIC_BASE+$17
    .if ::SAVE_REGS>1
    restore A
    .endif
    .endif
  .endmacro

;***********************************************************************
;; spriteBeforeBackground n
;; Make sprite n appear in front of the background
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A or n=Y
  .macro spriteBeforeBackground n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .if ::SAVE_REGS>1
    store A
    .endif
      lda #0
      sta VIC_BASE+$1b
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    .if ::SAVE_REGS>1
    store AX
    .endif
      tax
      .import _maskedtwopotentials
      lda _maskedtwopotentials,x
      and VIC_BASE+$1b
      sta VIC_BASE+$1b
    .if ::SAVE_REGS>1
    restore AX
    .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _maskedtwopotentials
      lda _maskedtwopotentials,x
      and VIC_BASE+$1b
      sta VIC_BASE+$1b
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _maskedtwopotentials
      lda _maskedtwopotentials,y
      and VIC_BASE+$1b
      sta VIC_BASE+$1b
    .if ::SAVE_REGS>1
    restore A
    .endif
    .else
    .if ::SAVE_REGS>1
    store A
    .endif
      lda VIC_BASE+$1b
      and #255 - (1<<(n))
      sta VIC_BASE+$1b
    .if ::SAVE_REGS>1
    restore A
    .endif
    .endif
  .endmacro

;***********************************************************************
;; spriteBehindBackground n
;; Make sprite n appear behind the background
;; n can be: constant, A, X, Y, or "all"
;; Registers modified: A (always); X when n=A or n=Y
  .macro spriteBehindBackground n
    .if (.xmatch ({n}, {ALL}) .or .xmatch ({n}, {all}))
    .if ::SAVE_REGS>1
    store A
    .endif
      lda #$ff
      sta VIC_BASE+$1b
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
    .if ::SAVE_REGS>1
    store AX
    .endif
      tax
      .import _twopotentials
      lda _twopotentials,x
      ora VIC_BASE+$1b
      sta VIC_BASE+$1b
    .if ::SAVE_REGS>1
    restore AX
    .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _twopotentials
      lda _twopotentials,x
      ora VIC_BASE+$1b
      sta VIC_BASE+$1b
    .if ::SAVE_REGS>1
    restore A
    .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
    .if ::SAVE_REGS>1
    store A
    .endif
      .import _twopotentials
      lda _twopotentials,y
      ora VIC_BASE+$1b
      sta VIC_BASE+$1b
    .if ::SAVE_REGS>1
    restore A
    .endif
    .else
    .if ::SAVE_REGS>1
    store A
    .endif
      lda VIC_BASE+$1b
      ora #1<<(n)
      sta VIC_BASE+$1b
    .if ::SAVE_REGS>1
    restore A
    .endif
    .endif
  .endmacro

;***********************************************************************
;; <h2>Color</h2>
;; Per-sprite color and shared multicolor registers.
;;
;; setSpriteColor n, arg
;; n can be: constant, A, X, Y
;; arg can be: A, X, Y, or constant
;; Registers modified: depends on n and arg (see cases below)
  .macro setSpriteColor n, arg
    .if (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      ; n in A: need a free index reg for the sta ,index form
      .if (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
        ; clobbers A, Y
        .if ::SAVE_REGS>1
        store A
        store Y
        .endif
        tay
        txa
        sta VIC_BASE+$27,y
        .if ::SAVE_REGS>1
        restore A
        restore Y
        .endif
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
        ; clobbers A, X
        .if ::SAVE_REGS>1
        store AX
        .endif
        tax
        tya
        sta VIC_BASE+$27,x
        .if ::SAVE_REGS>1
        restore AX
        .endif
      .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        ; clobbers A, X
        .if ::SAVE_REGS>1
        store AX
        .endif
        tax
        sta VIC_BASE+$27,x
        .if ::SAVE_REGS>1
        restore AX
        .endif
      .else
        ; constant arg — clobbers A, X
        .if ::SAVE_REGS>1
        store AX
        .endif
        tax
        lda #arg
        sta VIC_BASE+$27,x
        .if ::SAVE_REGS>1
        restore AX
        .endif
      .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        .if ::SAVE_REGS>1
        store A
        .endif
        sta VIC_BASE+$27,x
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
        .if ::SAVE_REGS>1
        store A
        .endif
        txa
        sta VIC_BASE+$27,x
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
        .if ::SAVE_REGS>1
        store A
        .endif
        tya
        sta VIC_BASE+$27,x
        .if ::SAVE_REGS>1
        restore A
        .endif
      .else
        .if ::SAVE_REGS>1
        store A
        .endif
        lda #arg
        sta VIC_BASE+$27,x
        .if ::SAVE_REGS>1
        restore A
        .endif
      .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        .if ::SAVE_REGS>1
        store A
        .endif
        sta VIC_BASE+$27,y
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
        .if ::SAVE_REGS>1
        store A
        .endif
        txa
        sta VIC_BASE+$27,y
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
        .if ::SAVE_REGS>1
        store A
        .endif
        tya
        sta VIC_BASE+$27,y
        .if ::SAVE_REGS>1
        restore A
        .endif
      .else
        .if ::SAVE_REGS>1
        store A
        .endif
        lda #arg
        sta VIC_BASE+$27,y
        .if ::SAVE_REGS>1
        restore A
        .endif
      .endif
    .else
      ; n is a constant
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        sta VIC_BASE+$27+n
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
        stx VIC_BASE+$27+n
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
        sty VIC_BASE+$27+n
      .else
        .if ::SAVE_REGS>1
        store A
        .endif
        lda #arg
        sta VIC_BASE+$27+n
        .if ::SAVE_REGS>1
        restore A
        .endif
      .endif
    .endif
  .endmacro

;***********************************************************************
;; getSpriteColor n, reg
;; Get the color of sprite n into register reg
;; n can be: constant, A, X, Y
;; reg can be: A, X, Y (defaults to A)
;; Registers modified: reg (and clobbers an index reg when n is A/X/Y)
  .macro getSpriteColor n, reg
    .if (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      ; n=A: tax clobbers X (original X gone)
      .ifblank reg
        .if ::SAVE_REGS>1
        store X
        .endif
        tax
        lda VIC_BASE+$27,x
        .if ::SAVE_REGS>1
        restore X
        .endif
      .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        .if ::SAVE_REGS>1
        store X
        .endif
        tax
        lda VIC_BASE+$27,x
        .if ::SAVE_REGS>1
        restore X
        .endif
      .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        ; X is output — original A is clobbered via tax
        .if ::SAVE_REGS>1
        store A
        .endif
        tax
        lda VIC_BASE+$27,x
        tax
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        ; Y is output — original X clobbered via tax
        .if ::SAVE_REGS>1
        store X
        .endif
        tax
        ldy VIC_BASE+$27,x
        .if ::SAVE_REGS>1
        restore X
        .endif
      .else
        .error "getSpriteColor: register must be A, X or Y"
      .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      ; n=X: indexed by X, no extra clobber for A/Y output; A clobbered for X output
      .ifblank reg
        lda VIC_BASE+$27,x
      .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        lda VIC_BASE+$27,x
      .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        .if ::SAVE_REGS>1
        store A
        .endif
        lda VIC_BASE+$27,x
        tax
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        ldy VIC_BASE+$27,x
      .else
        .error "getSpriteColor: register must be A, X or Y"
      .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      ; n=Y: indexed by Y, no extra clobber for A/X output; A clobbered for Y output
      .ifblank reg
        lda VIC_BASE+$27,y
      .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        lda VIC_BASE+$27,y
      .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        ldx VIC_BASE+$27,y
      .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        .if ::SAVE_REGS>1
        store A
        .endif
        lda VIC_BASE+$27,y
        tay
        .if ::SAVE_REGS>1
        restore A
        .endif
      .else
        .error "getSpriteColor: register must be A, X or Y"
      .endif
    .else
      ; n is a constant — no extra clobbers
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
        .if ::SAVE_REGS>1
        store A
        .endif
        lda #arg
        sta VIC_BASE+$25
        .if ::SAVE_REGS>1
        restore A
        .endif
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
        .if ::SAVE_REGS>1
        store A
        .endif
        lda #arg
        sta VIC_BASE+$26
        .if ::SAVE_REGS>1
        restore A
        .endif
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
;; <h2>Costume</h2>
;; Sprite costume (shape) selection.
;;
;; setSpriteCostume n, arg
;; If arg is a register or a numerical value below 256, interpreted as sprite costume block
;; If arg is a value > 255, interpreted as the absolute address of the sprite data
;;   (needs to align to a 64 byte block)
;; n can be: constant, A, X, Y
;; arg can be: A, X, Y, or constant
;; Registers modified: depends on n and arg (see cases below)
  .macro setSpriteCostume n, arg
    .if (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      .if (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
        ; clobbers A, Y
        .if ::SAVE_REGS>1
        store A
        store Y
        .endif
        tay
        txa
        sta SCREEN_BASE+$3f8,y
        .if ::SAVE_REGS>1
        restore A
        restore Y
        .endif
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
        ; clobbers A, X
        .if ::SAVE_REGS>1
        store AX
        .endif
        tax
        tya
        sta SCREEN_BASE+$3f8,x
        .if ::SAVE_REGS>1
        restore AX
        .endif
      .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        ; clobbers X
        .if ::SAVE_REGS>1
        store X
        .endif
        tax
        sta SCREEN_BASE+$3f8,x
        .if ::SAVE_REGS>1
        restore X
        .endif
      .else
        ; clobbers A, X
        .if ::SAVE_REGS>1
        store AX
        .endif
        tax
        lda #arg
        sta SCREEN_BASE+$3f8,x
        .if ::SAVE_REGS>1
        restore AX
        .endif
      .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        .if ::SAVE_REGS>1
        store A
        .endif
        sta SCREEN_BASE+$3f8,x
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
        .if ::SAVE_REGS>1
        store A
        .endif
        txa
        sta SCREEN_BASE+$3f8,x
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
        .if ::SAVE_REGS>1
        store A
        .endif
        tya
        sta SCREEN_BASE+$3f8,x
        .if ::SAVE_REGS>1
        restore A
        .endif
      .else
        .if ::SAVE_REGS>1
        store A
        .endif
        lda #arg
        sta SCREEN_BASE+$3f8,x
        .if ::SAVE_REGS>1
        restore A
        .endif
      .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        .if ::SAVE_REGS>1
        store A
        .endif
        sta SCREEN_BASE+$3f8,y
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
        .if ::SAVE_REGS>1
        store A
        .endif
        txa
        sta SCREEN_BASE+$3f8,y
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
        .if ::SAVE_REGS>1
        store A
        .endif
        tya
        sta SCREEN_BASE+$3f8,y
        .if ::SAVE_REGS>1
        restore A
        .endif
      .else
        .if ::SAVE_REGS>1
        store A
        .endif
        lda #arg
        sta SCREEN_BASE+$3f8,y
        .if ::SAVE_REGS>1
        restore A
        .endif
      .endif
    .else
      ; n is a constant
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        sta SCREEN_BASE+$3f8+n
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
        stx SCREEN_BASE+$3f8+n
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
        sty SCREEN_BASE+$3f8+n
      .else
        .if ::SAVE_REGS>1
        store A
        .endif
        .if .const(arg) && arg>255
          lda #((arg) & $3fc0) >> 6   ;values > 255 are interpreted as sprite memory address
        .else
          lda #arg
        .endif
        sta SCREEN_BASE+$3f8+n
        .if ::SAVE_REGS>1
        restore A
        .endif
      .endif
    .endif
  .endmacro

;***********************************************************************
;; getSpriteCostume n, reg
;; Get the costume of sprite n into register reg
;; n can be: constant, A, X, Y
;; reg can be: A, X, Y (defaults to A)
;; Registers modified: reg (and clobbers an index reg when n is A/X/Y)
  .macro getSpriteCostume n, reg
    .if (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      ; n=A: tax always clobbers original X
      .ifblank reg
        .if ::SAVE_REGS>1
        store X
        .endif
        tax
        lda SCREEN_BASE+$3f8,x
        .if ::SAVE_REGS>1
        restore X
        .endif
      .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        .if ::SAVE_REGS>1
        store X
        .endif
        tax
        lda SCREEN_BASE+$3f8,x
        .if ::SAVE_REGS>1
        restore X
        .endif
      .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        ; X is output — original A clobbered via tax
        .if ::SAVE_REGS>1
        store A
        .endif
        tax
        lda SCREEN_BASE+$3f8,x
        tax
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        ; Y is output — original X clobbered via tax
        .if ::SAVE_REGS>1
        store X
        .endif
        tax
        ldy SCREEN_BASE+$3f8,x
        .if ::SAVE_REGS>1
        restore X
        .endif
      .else
        .error "getSpriteCostume: register must be A, X or Y"
      .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      ; n=X: no extra clobbers for A/Y output; A clobbered for X output
      .ifblank reg
        lda SCREEN_BASE+$3f8,x
      .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        lda SCREEN_BASE+$3f8,x
      .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        .if ::SAVE_REGS>1
        store A
        .endif
        lda SCREEN_BASE+$3f8,x
        tax
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        ldy SCREEN_BASE+$3f8,x
      .else
        .error "getSpriteCostume: register must be A, X or Y"
      .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      ; n=Y: no extra clobbers for A/X output; A clobbered for Y output
      .ifblank reg
        lda SCREEN_BASE+$3f8,y
      .elseif (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        lda SCREEN_BASE+$3f8,y
      .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        ldx SCREEN_BASE+$3f8,y
      .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        .if ::SAVE_REGS>1
        store A
        .endif
        lda SCREEN_BASE+$3f8,y
        tay
        .if ::SAVE_REGS>1
        restore A
        .endif
      .else
        .error "getSpriteCostume: register must be A, X or Y"
      .endif
    .else
      ; n is a constant — no extra clobbers
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
    .endif
  .endmacro

;***********************************************************************
;; <h2>Position</h2>
;; Sprite position read/write macros.
;;
;; setSpriteX n, arg
;; n can be: constant, A, X, Y
;; arg can be: AX (A=low byte, X=high byte),
;;             A  (carry flag = bit 8 of X coordinate),
;;             or 16-bit constant
;; Registers modified: A if n is constant, otherwise clobbers the other index register
  .macro setSpriteX n, arg

    .if (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      ; n in A: move to Y, then use n=Y path — clobbers A, X, Y
      .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
      .if ::SAVE_REGS>1
      store AX
      store Y
      .endif
	cpx #1  ;set carry if X>=1
        tay
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
      .if ::SAVE_REGS>1
      restore AX
      restore Y
      .endif
      .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        ; A=lo, carry=bit8 — clobbers A, X, Y
      .if ::SAVE_REGS>1
      store AX
      store Y
      .endif
        tay
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
      .if ::SAVE_REGS>1
      restore AX
      restore Y
      .endif
      .else
        ; constant arg — clobbers A, X, Y
      .if ::SAVE_REGS>1
      store AX
      store Y
      .endif
        tay
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
      .if ::SAVE_REGS>1
      restore AX
      restore Y
      .endif
      .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      ; --- n in Y --- clobbers A, X always
      .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
      .if ::SAVE_REGS>1
      store AX
      .endif
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
      .if ::SAVE_REGS>1
      restore AX
      .endif
      .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
        ; A=lo, carry=bit8
      .if ::SAVE_REGS>1
      store AX
      .endif
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
      .if ::SAVE_REGS>1
      restore AX
      .endif
      .else
        ; constant arg
      .if ::SAVE_REGS>1
      store AX
      .endif
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
      .if ::SAVE_REGS>1
      restore AX
      .endif
      .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      ; n in X — clobbers A, Y always
      .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
      .if ::SAVE_REGS>1
      store A
      store Y
      .endif
        cpx #1
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
      .if ::SAVE_REGS>1
      restore A
      restore Y
      .endif
      .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      .if ::SAVE_REGS>1
      store A
      store Y
      .endif
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
      .if ::SAVE_REGS>1
      restore A
      restore Y
      .endif
      .else
      .if ::SAVE_REGS>1
      store A
      store Y
      .endif
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
      .if ::SAVE_REGS>1
      restore A
      restore Y
      .endif
      .endif
    .else
      ; --- n is a constant --- clobbers A always
      .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
      .if ::SAVE_REGS>1
      store A
      .endif
        sta VIC_BASE+n+n
        lda VIC_BASE+$10
        cpx #0
        if ne
            ora #1<<(n)
        else
            and #255 - (1<<(n))
        endif
        sta VIC_BASE+$10
      .if ::SAVE_REGS>1
      restore A
      .endif
      .elseif (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
      .if ::SAVE_REGS>1
      store A
      .endif
        sta VIC_BASE+n+n
        lda VIC_BASE+$10
        if cs
            ora #1<<(n)
        else
            and #255 - (1<<(n))
        endif
        sta VIC_BASE+$10
      .if ::SAVE_REGS>1
      restore A
      .endif
      .else
      .if ::SAVE_REGS>1
      store A
      .endif
        lda #<(arg)
        sta VIC_BASE+n+n
        lda VIC_BASE+$10
        .if arg>255
          ora #1<<(n)
        .else
          and #255 - (1<<(n))
        .endif
        sta VIC_BASE+$10
      .if ::SAVE_REGS>1
      restore A
      .endif
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
          ; clobbers X
          .if ::SAVE_REGS>1
          store X
          .endif
          .import _doubles_0_to_7
          ldx _doubles_0_to_7,y	    ;x=2*y
          sta VIC_BASE+1,x
          .if ::SAVE_REGS>1
          restore X
          .endif
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
          ; clobbers A, X
          .if ::SAVE_REGS>1
          store AX
          .endif
          txa
          .import _doubles_0_to_7
          ldx _doubles_0_to_7,y	    ;x=2*y
          sta VIC_BASE+1,x
          .if ::SAVE_REGS>1
          restore AX
          .endif
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
          ; Y = sprite number AND value — clobbers A, X
          .if ::SAVE_REGS>1
          store AX
          .endif
          tya                         ; A = value (=sprite number, same thing)
          .import _doubles_0_to_7
          ldx _doubles_0_to_7,y	    ;x=2*y  (Y still holds sprite number)
          sta VIC_BASE+1,x
          .if ::SAVE_REGS>1
          restore AX
          .endif
      .else
          ; clobbers A, X
          .if ::SAVE_REGS>1
          store AX
          .endif
          .import _doubles_0_to_7
          ldx _doubles_0_to_7,y	    ;x=2*y
          lda #arg
          sta VIC_BASE+1,x
          .if ::SAVE_REGS>1
          restore AX
          .endif
      .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      ; --- n in X --- 
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
          ; clobbers Y
          .if ::SAVE_REGS>1
          store Y
          .endif
          .import _doubles_0_to_7
          ldy _doubles_0_to_7,x	    ;y=2*x
          sta VIC_BASE+1,y
          .if ::SAVE_REGS>1
          restore Y
          .endif
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
          ; X = sprite number AND value — clobbers A, Y
          .if ::SAVE_REGS>1
          store A
          store Y
          .endif
          txa                         ; A = value (=sprite number, same thing)
          .import _doubles_0_to_7
          ldy _doubles_0_to_7,x      ;y=2*x  (X still holds sprite number)
          sta VIC_BASE+1,y
          .if ::SAVE_REGS>1
          restore A
          restore Y
          .endif
      .else
          ; clobbers A, Y
          .if ::SAVE_REGS>1
          store A
          store Y
          .endif
          .import _doubles_0_to_7
          ldy _doubles_0_to_7,x	    ;y=2*x
          lda #arg
          sta VIC_BASE+1,y
          .if ::SAVE_REGS>1
          restore A
          restore Y
          .endif
      .endif
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      ; --- n in A --- 
      .if (.xmatch ({arg}, {A}) .or .xmatch ({arg}, {a}))
          ; A = sprite number AND value — clobbers X, Y
          .if ::SAVE_REGS>1
          store X
          store Y
          .endif
          tay                         ; Y = sprite number (for lookup)
          .import _doubles_0_to_7
          ldx _doubles_0_to_7,y      ;x=2*y
          sta VIC_BASE+1,x
          .if ::SAVE_REGS>1
          restore X
          restore Y
          .endif
      .elseif (.xmatch ({arg}, {X}) .or .xmatch ({arg}, {x}))
          ; clobbers A, X, Y
          .if ::SAVE_REGS>1
          store AX
          store Y
          .endif
          tay
	  txa
          .import _doubles_0_to_7
          ldx _doubles_0_to_7,y	    ;x=2*y
          sta VIC_BASE+1,x
          .if ::SAVE_REGS>1
          restore AX
          restore Y
          .endif
      .elseif (.xmatch ({arg}, {Y}) .or .xmatch ({arg}, {y}))
          ; clobbers A, X, Y
          .if ::SAVE_REGS>1
          store AX
          store Y
          .endif
          tax
	  tya
          .import _doubles_0_to_7
          ldy _doubles_0_to_7,x     ;y=2*x
          sta VIC_BASE+1,y
          .if ::SAVE_REGS>1
          restore AX
          restore Y
          .endif
      .else
          ; constant — clobbers A, X, Y
          .if ::SAVE_REGS>1
          store AX
          store Y
          .endif
          .import _doubles_0_to_7
	  tay
          ldx _doubles_0_to_7,y	    ;x=2*y
          lda #arg
          sta VIC_BASE+1,x
          .if ::SAVE_REGS>1
          restore AX
          restore Y
          .endif
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
          .if ::SAVE_REGS>1
          store A
          .endif
          lda #arg
          sta VIC_BASE+n+n+1
          .if ::SAVE_REGS>1
          restore A
          .endif
      .endif
    .endif
  .endmacro

;***********************************************************************
;; getSpriteX n, reg
;; Get the X position of sprite n
;; n can be: constant, X, Y, A
;; reg must be: AX (A=low byte, X=high byte / bit 8)
;; Registers modified: A, X; Y when n=Y
  .macro getSpriteX n, reg
    .if (.not (.xmatch ({reg}, {AX}) .or .xmatch ({reg}, {ax})))
        .error "getSpriteX: register must be AX"
    .elseif (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      ; n in A: tay clobbers Y
      .if ::SAVE_REGS>1
      store Y
      .endif
      tay
      lda VIC_BASE+$10
      .import _twopotentials
      and _twopotentials,y
      beq :+
      ldx #1
      .byte $0c           ; skip next 2 bytes (ldx #0)
:     ldx #0
      .import _doubles_0_to_7
      lda _doubles_0_to_7,y
      tay
      lda VIC_BASE+0,y
      .if ::SAVE_REGS>1
      restore Y
      .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      ; n in X: ldy clobbers Y
      .if ::SAVE_REGS>1
      store Y
      .endif
      .import _doubles_0_to_7
      ldy _doubles_0_to_7,x       ; Y = 2*n, X still = sprite number
      .import _twopotentials
      lda VIC_BASE+$10
      and _twopotentials,x        ; X still = sprite number
      beq :+
      ldx #1
      .byte $0c           ; skip next 2 bytes (ldx #0)
:     ldx #0
      lda VIC_BASE+0,y
      .if ::SAVE_REGS>1
      restore Y
      .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      ; n in Y: original Y consumed by lda _doubles,y / tay
      .if ::SAVE_REGS>1
      store Y
      .endif
      lda VIC_BASE+$10
      .import _twopotentials
      and _twopotentials,y
      beq :+
      ldx #1
      .byte $0c           ; skip next 2 bytes (ldx #0)
:     ldx #0
      .import _doubles_0_to_7
      lda _doubles_0_to_7,y
      tay
      lda VIC_BASE+0,y
      .if ::SAVE_REGS>1
      restore Y
      .endif
    .else
      ; n is a constant — no extra clobbers
      ldx #0
      lda VIC_BASE+$10
      and #1<<(n)
      if ne
          inx
      endif
      lda VIC_BASE+n+n
    .endif
  .endmacro

;***********************************************************************
;; getSpriteY n, reg
;; Get the Y position of sprite n into register reg
;; n can be: constant, A, X, Y
;; reg can be: A, X, Y
;; Registers modified: see cases; always clobbers at least one index reg when n is runtime
  .macro getSpriteY n, reg
    .if (.xmatch ({n}, {A}) .or .xmatch ({n}, {a}))
      ; n in A
      .if (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        ; clobbers X, Y
        .if ::SAVE_REGS>1
        store X
        store Y
        .endif
        tay
        .import _doubles_0_to_7
        ldx _doubles_0_to_7,y       ; x=2*n
        lda VIC_BASE+1,x
        .if ::SAVE_REGS>1
        restore X
        restore Y
        .endif
      .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        ; X is output — clobbers A, Y
        .if ::SAVE_REGS>1
        store A
        store Y
        .endif
        tay
        .import _doubles_0_to_7
        ldx _doubles_0_to_7,y       ; x=2*n
        lda VIC_BASE+1,x
        tax
        .if ::SAVE_REGS>1
        restore A
        restore Y
        .endif
      .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        ; Y is output — clobbers A, X
        .if ::SAVE_REGS>1
        store AX
        .endif
        tay
        .import _doubles_0_to_7
        ldx _doubles_0_to_7,y       ; x=2*n
        lda VIC_BASE+1,x
        tay
        .if ::SAVE_REGS>1
        restore AX
        .endif
      .else
        .error "getSpriteY: register must be A, X or Y"
      .endif
    .elseif (.xmatch ({n}, {X}) .or .xmatch ({n}, {x}))
      ; n in X
      .if (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        ; clobbers Y
        .if ::SAVE_REGS>1
        store Y
        .endif
        .import _doubles_0_to_7
        ldy _doubles_0_to_7,x       ; y=2*n
        lda VIC_BASE+1,y
        .if ::SAVE_REGS>1
        restore Y
        .endif
      .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        ; X is output — clobbers A, Y
        .if ::SAVE_REGS>1
        store A
        store Y
        .endif
        .import _doubles_0_to_7
        ldy _doubles_0_to_7,x       ; y=2*n (X still = sprite n)
        lda VIC_BASE+1,y
        tax
        .if ::SAVE_REGS>1
        restore A
        restore Y
        .endif
      .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        ; Y is output — clobbers A (original Y gone but that's output)
        .if ::SAVE_REGS>1
        store A
        .endif
        .import _doubles_0_to_7
        ldy _doubles_0_to_7,x       ; y=2*n (X consumed, now Y=offset)
        lda VIC_BASE+1,y
        tay
        .if ::SAVE_REGS>1
        restore A
        .endif
      .else
        .error "getSpriteY: register must be A, X or Y"
      .endif
    .elseif (.xmatch ({n}, {Y}) .or .xmatch ({n}, {y}))
      ; n in Y
      .if (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
        ; clobbers X
        .if ::SAVE_REGS>1
        store X
        .endif
        .import _doubles_0_to_7
        ldx _doubles_0_to_7,y       ; x=2*n
        lda VIC_BASE+1,x
        .if ::SAVE_REGS>1
        restore X
        .endif
      .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
        ; X is output — clobbers A (original X gone, that's output)
        .if ::SAVE_REGS>1
        store A
        .endif
        .import _doubles_0_to_7
        ldx _doubles_0_to_7,y       ; x=2*n (Y consumed, X=offset)
        lda VIC_BASE+1,x
        tax
        .if ::SAVE_REGS>1
        restore A
        .endif
      .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
        ; Y is output — clobbers A, X
        .if ::SAVE_REGS>1
        store AX
        .endif
        .import _doubles_0_to_7
        ldx _doubles_0_to_7,y       ; x=2*n
        lda VIC_BASE+1,x
        tay
        .if ::SAVE_REGS>1
        restore AX
        .endif
      .else
        .error "getSpriteY: register must be A, X or Y"
      .endif
    .else
      ; n is a constant — no extra clobbers
      .if (.xmatch ({reg}, {A}) .or .xmatch ({reg}, {a}))
          lda VIC_BASE+n+n+1
      .elseif (.xmatch ({reg}, {X}) .or .xmatch ({reg}, {x}))
          ldx VIC_BASE+n+n+1
      .elseif (.xmatch ({reg}, {Y}) .or .xmatch ({reg}, {y}))
          ldy VIC_BASE+n+n+1
      .else
          .error "getSpriteY: register must be A, X or Y"
      .endif
    .endif
  .endmacro

;***********************************************************************
;; setSpriteXY n, xpos, ypos
;; Set the X and Y positions of sprite n
;; n can be: constant, A, X, Y (delegates to setSpriteX / setSpriteY)
;; xpos: see setSpriteX
;; ypos: see setSpriteY
;; Registers modified: A, X, Y
  .macro setSpriteXY n, xpos, ypos
  BACKUP_SAVE_REGS .set ::SAVE_REGS
  ::SAVE_REGS .set 2
  setSpriteY n, ypos
  ::SAVE_REGS .set BACKUP_SAVE_REGS
  setSpriteX n, xpos
  .endmacro

;***********************************************************************
;; updateSpriteAttributes n
;; Update sprite n's attributes
;; n can be: constant, A, X, or Y
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
  .import _updateSpriteAttributes
  jsr _updateSpriteAttributes
.endmacro

.endif


