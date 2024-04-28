; restores VIC and I/O vectors

.include "../LAMAlib-ROMfunctions.inc"

.export _restore_sr:=restore
.import _PTRSCRHI

restore:
	sei
	; bank in ROM and I/O (default value)
	lda #$37
	sta 1

	; set VIC bank $0000
	lda $dd00
	ora #%00000011
	sta $dd00

	; tell the KERNAL where the screen is
	lda #04
	sta _PTRSCRHI

	;initialise vectors
	jsr INITVEC	

	;initialise vic chip
	jsr INITVIC

	;set timer interrupt
	lda #$81
	sta $DC0D
	lda #$01
	sta $DC0E

	cli
	rts