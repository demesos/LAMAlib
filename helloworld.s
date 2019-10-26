; A simple test program for LAMAlib 
;
; Assemble with:
; cl65 helloworld.s -lib LAMAlib.lib -C c64-asm.cfg -u __EXEHDR__ -o helloworld.prg
;

.include "LAMAlib.inc"

	clrscr
	lda #$0e ; white
	sta $286  ; set textcolor
	printstr hellostr
	newline

	lda #$0e ; light blue
	sta $286  ; set textcolor
	printstr didyouknow1
	newline

	rand8   ;fill A with random number
	ldx #00
	stax number1
	
	rand8   ;fill A with random number
	ldx #00
	stax number2

	mul16 number1 ;multiply A/X with number1
	stax number3

	ldax number1
	printax

	printstr didyouknow2

	ldax number2
	printax

	printstr didyouknow3

	ldax number3
	printax


	rts

hellostr:   .asciiz "hello everybody! this is a test program for the lamalib library. the library mostly contains assembler macros, but also some functions for multiplication and division and a wrapper to some rom functions."
didyouknow1: .asciiz "did you know that:"
didyouknow2: .asciiz " times "
didyouknow3: .asciiz " equals "
number1: .byte 00,00
number2: .byte 00,00
number3: .byte 00,00
