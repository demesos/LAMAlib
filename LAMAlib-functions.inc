;; mul16 addr
;; Multiplies the unsigned 16 bit value in A/X with the 16 bit value stored at addr (lo-byte) and addr+1 (hi-byte)
;; Implemented as a subroutinge, link with -lib lamalib.lib
;; This function is not reentrant, don't use in interrupt and main program simultaneously
;; Result is returned in A/X
;; Registers modified: all
.import mul16sr
.importzp mul16arg

.macro mul16 arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            ldy     #<(.right (.tcount ({arg})-1, {arg}))
	    sty	mul16arg
            ldy     #>(.right (.tcount ({arg})-1, {arg}))
            sty mul16arg+1
        .else
            ; assume absolute or zero page
            ldy     arg
	    sty	mul16arg
            ldy     1+(arg)
            sty mul16arg+1
        .endif	
        jsr mul16sr ;mul16 subroutine
.endscope
.endmacro

.import fastmul16sr
.importzp fastmul16arg

.macro fastmul16 arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            ldy     #<(.right (.tcount ({arg})-1, {arg}))
	    sty	fastmul16arg
            ldy     #>(.right (.tcount ({arg})-1, {arg}))
            sty fastmul16arg+1
        .else
            ; assume absolute or zero page
            ldy     arg
	    sty	fastmul16arg
            ldy     1+(arg)
            sty fastmul16arg+1
        .endif	
        jsr fastmul16sr ;fastmul16 subroutine
.endscope
.endmacro

;; div16 arg
;; Multiplies the unsigned 16 bit value in A/X with an imediate value or the 16 bit value stored at addr (lo-byte) and addr+1 (hi-byte)
;; Implemented as a subroutinge, link with -lib lamalib.lib
;; This function is not reentrant, don't use in interrupt and main program simultaneously
;; Result is returned in A/X
;; Registers modified: all
.import div16sr
.importzp div16arg,div16rem

.macro div16 arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            ldy     #<(.right (.tcount ({arg})-1, {arg}))
	    sty	div16arg
            ldy     #>(.right (.tcount ({arg})-1, {arg}))
            sty div16arg+1
        .else
            ; assume absolute or zero page
            ldy     arg
	    sty	div16arg
            ldy     1+(arg)
            sty div16arg+1
        .endif	
         jsr div16sr ;div16 subroutine
.endscope
.endmacro

;; rand8 arg, rand8_setseed arg
;; based on the 8-bit pseudo-random number generator from White Flame 
;; https://codebase64.org/doku.php?id=base:small_fast_8-bit_prng
;; Simple but very fast random generator with a period of 256
;; Seed is an 8 bit value
;; Result is returned in A
;; Registers modified: A
.import rand8sr, rand8seed

.macro rand8
.scope
         jsr rand8sr
.endscope
.endmacro

.macro rand8_setseed arg
.scope
        lda #arg
	sta rand8seed
.endscope
.endmacro

;; rand16 arg, rand16_setseed arg
;; fast 16-bit pseudo-random number generator with period of 65535
;; seed must never be 32755, so setseed modifies this value it this is the case
;; Seed is a 16 bit value
;; Result is returned in A/X
;; Registers modified: A,X
.import rand16sr, rand16seed

.macro rand16
.scope
         jsr rand16sr
.endscope
.endmacro

.macro rand16_setseed arg
.scope
	ldax arg
	cmpax 32755
	bne @ok
@ok	and #FE ;make it 32754
	sta rand16seed
	stx rand16seed+1
.endscope
.endmacro