; A simple test program for LAMAlib 
;
; How to assemble:
; for the C64:
; cl65 helloworld.s -lib LAMAlib.lib -t c64 -C c64-asm.cfg -u __EXEHDR__ -o helloworld.prg
;
; for the VIC20:
; ca65 helloworld.s -t vic20
; cl65 helloworld.o -lib LAMAlib.lib -t vic20 -u __EXEHDR__ -o helloworld.prg
;
; for the C128:
; ca65 helloworld.s -t c128
; cl65 helloworld.o -lib LAMAlib.lib -t c128 -u __EXEHDR__ -o helloworld.prg

.include "LAMAlib.inc"
.FEATURE STRING_ESCAPES

	clrscr
	lda #$01  ; white
	sta $286  ; set textcolor

	; used PETSCII codes:
	; \x1e green
	; \x9e yellow
	; \x9f cyan
	; \x0e switch to lower/uppercase mode

	print "\x0e\x9fHello everybody! \nThis is a test program for the\n\x9eLAMAlib library\x9f.\n\n"
	print "The library mostly contains assembler macros, but also some functions for multiplication and division and wrappers to some ROM functions.\n\n"

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

	print "\x9fDid you know that\n\x1e",(number1),"\x9f times \x1e",(number2),"\x9f equals \x1e",(number3),"\x9f?\n"

	rts

number1: .byte 00,00
number2: .byte 00,00
number3: .byte 00,00
