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

;; rand8 arg; rand8_setseed [arg]
;; based on the 8-bit pseudo-random number generator from White Flame 
;; https://codebase64.org/doku.php?id=base:small_fast_8-bit_prng
;; Simple but very fast random generator with a period of 256
;; Seed is an 8 bit value, setting seed is optional. When no argument is given, the current value in the Accumulator is used as the seed
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
	.if .paramcount = 1
	   lda #arg
	.endif 
	sta rand8seed
.endscope
.endmacro

;; rand16 arg; rand16_setseed arg
;; fast 16-bit pseudo-random number generator with period of 65535
;; seed must never be 32755, so setseed modifies this value it this is the case
;; Seed is a 16 bit value, setting seed is optional. When no argument is given, the current value in A/X is used as the seed
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
	.if .paramcount = 1
	   ldax #arg
	.endif 
	cmpax #32755	;this is the one seed we need to avoid
	bne @ok
	and #$FE ;make it 32754
@ok:	sta rand16seed
	stx rand16seed+1
.endscope
.endmacro

;; print arg1 [arg2 ...]
;; Prints the arguments as strings or numbers
;; An argument in parenthesis will print the 16bit value stored at this address
;; uses ROM functions, BASIC and KERNAL ROM need to be enabled when using this macro
;; Registers modified: A,Y,X
.macro print arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
  .ifblank arg1
      ; First parameter is empty
      .exitmacro
  .endif
.scope
       .if (.match (.left (1, {arg1}), {(}))
	    ldax .mid (1, .tcount ({arg1})-2, {arg1})
	    printax
       .else
            primm arg1
       .endif
.endscope
.if .paramcount > 1
	;macro calls itself recursely to process remaining arguments
        print arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
.endif
.endmacro

;; primm str
;; Prints the given string, string is inlined in program code
;; uses ROM functions, BASIC and KERNAL ROM need to be enabled when using this macro
;; Registers modified: A,Y,X
.macro  primm str
.import primmsr
.scope
       jsr primmsr
       .asciiz str
.endscope
.endmacro

;; printax
;; Prints the number in A/X as a 16 bit unsigned decimal number
;; BASIC and KERNAL ROM need to be enabled when using this function
;; Registers modified: A,Y,X
.macro  printax
.scope
		sta @setx +1	;we need to switch A and X
		txa
@setx:		ldx #00
                jsr LINPRT
.endscope
.endmacro

;; printax_signed
;; Prints the number in A/X as a 16 bit signed decimal number
;; BASIC and KERNAL ROM need to be enabled when using this function
;; Registers modified: A,Y,X
.macro  printax_signed
.scope
		cpx #$80	;is it a negative number?
		bcc @positive
		negax		;negate number
		pha
		lda #'-'	;write out minus sign
		jsr CHROUT
		pla
@positive:	sta @setx +1	;we need to switch A and X
		txa
@setx:		ldx #00
                jsr LINPRT
.endscope
.endmacro

;; printstr addr
;; Prints the null-terminated string at addr using the STROUT function
;; BASIC and KERNAL ROM need to be enabled when using this function
;; Registers modified: A,Y,X
.macro  printstr   addr
.scope
                lda #<addr
                ldy #>addr
                jsr STROUT
.endscope
.endmacro

;; newline
;; Prints a newline character
;; KERNAL ROM needs to be enabled when using this function
;; Registers modified: A,Y,X
.macro  newline
.scope
		lda #$0d
                jsr CHROUT
.endscope
.endmacro

;; clrscr
;; Clears the screen
;; KERNAL ROM needs to be enabled when using this function
;; Registers modified: A,Y,X
.macro  clrscr
.scope
                jsr CLRSCR
.endscope
.endmacro
