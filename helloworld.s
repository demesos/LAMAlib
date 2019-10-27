; A simple test program for LAMAlib 
;
; Assemble with:
; cl65 helloworld.s -lib LAMAlib.lib -t c64 -C c64-asm.cfg -u __EXEHDR__ -o helloworld.prg
;

.include "LAMAlib.inc"
.FEATURE STRING_ESCAPES

	clrscr
	lda #$01  ; white
	sta $286  ; set textcolor

	; used PETSCII codes:
	; \x05 white
	; \x9a light blue
	; \x9f cyan
	; \x0e switch to lower/uppercase mode

	print "\x0e\x9fHello everybody! \nThis is a test program for the\n\x05LAMAlib library\x9f.\n\n"
	print "The library mostly contains assembler macros, but also some functions for multiplication and division and wrappers to some rom functions.\n\n"

	lda $d012   ;get current rasterline
	rand8_setseed

	lda #$0e  ; light blue
	sta $286  ; set textcolor
	rand8     ; fill A with random number

	ldx #00
	stax number1
	
	rand8     ; fill A with random number
	ldx #00
	stax number2

	mul16 number1 ;multiply A/X with number1
	stax number3

	print "\x9aDid you know that\n\x05",(number1),"\x9a times \x05",(number2),"\x9a equals \x05",(number3),"\x9a?\n"

	rts

number1: .byte 00,00
number2: .byte 00,00
number3: .byte 00,00
