.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-ROMfunctions.inc"

; print immediate implementation by Wil
; works with strings up to 254 chars
; based on an idea from http://www.6502.org/source/io/primm.htm

.export primmsr := primm

primm:	pla		;get the low byte from return address
	sta @loopstart+1 ;self-modification
	pla
	sta @loopstart+2
	ldy #01		;string starts at return addr + 1
	bne @loopstart

@loop:  jsr CHROUT
	iny
@loopstart:
        lda $ffff,Y
	bne @loop

	tya
	clc
	adc @loopstart+1
	tay
	lda @loopstart+2
	adc #00
	pha	;push high address onto stack
	tya
	pha	;push low address onto stack
        rts
