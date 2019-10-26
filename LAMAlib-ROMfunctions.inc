;; <h2>ROM library calls</h2>

CHROUT= $FFD2 ;outputs the character in A
STROUT= $AB1E ;outputs a string at addr A/Y
LINPRT= $BDCD ;outputs number in A/X
CLRSCR= $E544 ;clear screen

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
