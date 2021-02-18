.include "../LAMAlib.inc"

.FEATURE string_escapes

;cl65 testmacros.s -lib ../LAMAlib/LAMAlib.lib -C c64-asm.cfg -o testmacros.prg

	clrscr
	printstr hellostr
	printstr lamastr
	newline
	printstr didyouknow
	rand8            ;get random number into A      
	ldx #0           ;high byte is 0                
	stax valuea      ;store value for later use     
	printax	         ;print 16 bit number           
	lda #'*'                                        
	jsr CHROUT       ;output asterisk               
	rand8            ;get random number into A      
	ldx #0           ;high byte is 0                
	pushax           ;save value for later use      
	printax	         ;print 16 bit number           
	pullax	         ;get back value                
	mul16 valuea     ;16 bit multiplication of AX and valuea
	pushax           ;save value for later use   
	printstr eqals   ;output value
	pullax           ;get back value  
	pushax
	printax	         ;print 16 bit number  
	lda #'?'
	jsr CHROUT	 ;output question mark
	newline
	
	primm "Let's divide "
	pullax
	pushax
	printax
 	primm " by "
	rand8
	ldx #00
	stax valuea
	printax
	newline
	pullax
        div16 valuea
	pushax
	primm "Result is "
	pullax
	print AX," remainder is ", (_div16_rem)
	rts

hellostr:    .BYTE $0e,$9a,"Hello from ",0
lamastr:     .asciiz "\x05LAMA\x96lib!"
didyouknow:  .asciiz "\x9aDid you know that "
eqals:       .asciiz " equals "
valuea:	     .byte 00,00
