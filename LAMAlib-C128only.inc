.if .def(__C128__)

;; <h2>Special Macros for C128 in C128 Mode</h2>

;; shadowIRQ off|on
;; The C128 IRQ routine copies the values of the shadow registers $11D6-$11E6
;; into VIC registers $D000-$D010 that control the positions of sprites on the
;; screen. It further handles the BASIC commands SOUND, PLAY, and SPRITE. To
;; avoid this, the macro  shadowIRQ off puts a 0 into memory address $0A04, 
;; telling the Kernal that BASIC has not been initialized yet.
;; Cutting the IRQ routine provides a speed gain of about 2.5%
;; Registers modified: A
.macro  shadowirq arg  ;version with all lowercase
	shadowIRQ arg
.endmacro

.macro shadowIRQ arg
.scope
        .if .xmatch ({arg}, {off}) 
            ; deactivate BASIC part of IRQ
	    lda #00
            sta $0A04
        .else
            .if .xmatch ({arg}, {on}) 
                ; reactivate BASIC part of IRQ
    	        lda #193
                sta $0A04
            .else
                .error  "Unknown argument for shadowIRQ, must be 'off' or 'on'."
            .endif
        .endif
.endscope
.endmacro


;; set_VIC_RAMbank bank
;; Tell the MMU to feed RAM bank 0 or 1 to the VIC
;; This is a C128-only feature
;; Bank must be 0 or 1
;; I/O must be enabled for this macro to work
.macro set_VIC_RAMbank bank
	lda $d506
	.if bank=0
	  and #%10111111	;clear bit 6
	.else
	  ora #%01111111	;enable bit 6
	.endif
	sta $d506
.endmacro

.endif


