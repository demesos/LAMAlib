.include "../LAMAlib-macros16.inc"
; a routine to delay a certain number of cycles, including the JSR and RTS command
; the number of cycles is a rather odd number but this delay is often used for stable rasterbar routines

.export _delay_53cycles
.import _delay_31cycles

_delay_53cycles:
	jsr _delay_31cycles
	pha
	pla
	bit $42
	rts

