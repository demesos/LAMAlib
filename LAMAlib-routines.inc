;; <h2>Useful Routines</h2>

;; rand8
;; <b>rand8_setseed</b> [<i>arg</i>]<br>
;; based on the 8-bit pseudo-random number generator from White Flame 
;; https://codebase64.org/doku.php?id=base:small_fast_8-bit_prng
;; Simple but very fast random generator with a period of 256
;; Seed is an 8 bit value, setting seed is optional. When no argument is given, the current value in the Accumulator is used as the seed
;; Result is returned in A
;; Registers modified: A
.import _rand8_sr, _rand8_seed

.macro rand8
.scope
         jsr _rand8_sr
.endscope
.endmacro

.macro rand8_setseed arg
.scope
	.if .paramcount = 1
	   lda #arg
	.endif 
	sta _rand8_seed
.endscope
.endmacro

;; rand16
;; <b>rand16_setseed</b> <i>arg</i><br>
;; Fast 16-bit pseudo-random number generator with period of 65535
;; seed must never be 32755, so setseed modifies this value it this is the case
;; Seed is a 16 bit value, setting seed is optional. When no argument is given, the current value in A/X is used as the seed
;; Result is returned in A/X
;; Registers modified: A,X
.import _rand16_sr, _rand16_seed

.macro rand16
.scope
         jsr _rand16_sr
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
@ok:	sta _rand16_seed
	stx _rand16_seed+1
.endscope
.endmacro

;; mul16 addr
;; Multiplies the unsigned 16 bit value in A/X with the 16 bit value stored at addr (lo-byte) and addr+1 (hi-byte)
;; Implemented as a subroutinge, link with -lib lamalib.lib
;; This function is not reentrant, don't use in interrupt and main program simultaneously
;; Result is returned in A/X
;; Registers modified: A,X,Y
.import _mul16_sr
.importzp _mul16_arg

.macro mul16 arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            ldy     #<(.right (.tcount ({arg})-1, {arg}))
	    sty	_mul16_arg
            ldy     #>(.right (.tcount ({arg})-1, {arg}))
            sty _mul16_arg+1
        .else
            ; assume absolute or zero page
            ldy     arg
	    sty	_mul16_arg
            ldy     1+(arg)
            sty _mul16_arg+1
        .endif	
        jsr _mul16_sr ;mul16 subroutine
.endscope
.endmacro

.import _fastmul16_sr
.importzp _fastmul16_arg

.macro fastmul16 arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            ldy     #<(.right (.tcount ({arg})-1, {arg}))
	    sty	_fastmul16_arg
            ldy     #>(.right (.tcount ({arg})-1, {arg}))
            sty _fastmul16_arg+1
        .else
            ; assume absolute or zero page
            ldy     arg
	    sty	_fastmul16_arg
            ldy     1+(arg)
            sty _fastmul16_arg+1
        .endif	
        jsr _fastmul16_sr ;fastmul16 subroutine
.endscope
.endmacro

;; div16 arg
;; Multiplies the unsigned 16 bit value in A/X with an imediate value or the 16 bit value stored at addr (lo-byte) and addr+1 (hi-byte)
;; Implemented as a subroutinge, link with -lib lamalib.lib
;; This function is not reentrant, don't use in interrupt and main program simultaneously
;; Result is returned in A/X
;; Registers modified: all
.import _div16_sr
.importzp _div16_arg,_div16_rem

.macro div16 arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            ldy     #<(.right (.tcount ({arg})-1, {arg}))
	    sty	_div16_arg
            ldy     #>(.right (.tcount ({arg})-1, {arg}))
            sty _div16_arg+1
        .else
            ; assume absolute or zero page
            ldy     arg
	    sty	_div16_arg
            ldy     1+(arg)
            sty _div16_arg+1
        .endif	
         jsr _div16_sr ;div16 subroutine
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
	.if (.xmatch ({arg1}, {AX}))			;print AX as number
           printax
       .elseif (.xmatch ({arg1}, {A}))			;print akku as number
	   ldx #00
           printax
       .elseif (.match (.left (1, {arg1}), #))		;immediate number
	    iarg=(.right (.tcount ({arg1})-1, {arg1}))
            .if .const(iarg) .and (<iarg = >iarg)
               lda     #<iarg
               tax
            .else
               lda     #<iarg
               ldx     #>iarg
            .endif
	    printax
	.elseif (.match (.left (1, {arg1}), {(})) .and (.match (.right (1, {arg1}), {)}))	;argument in brackets
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
.import _primm_sr
.scope
       jsr _primm_sr
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

;; delay_ms arg
;; Waits for arg milliseconds using a busy waiting loop
;; When AX is given as argument it waits as many ms as the 16 bit value in A/X
;; When a number (1-65536) is given as argument it waits this long
;; Registers modified: A,X
.macro  delay_ms arg
.import _delay_ms_sr
.scope
       .if (.xmatch ({arg1}, AX))
	    jsr _delay_ms_sr
       .else
	    ldax #arg
            jsr _delay_ms_sr
       .endif
.endscope
.endmacro

;; waitkey
;; Waits until a key is pressed
;; Function works independly of IRQ
;; Registers modified: A
.import _waitkey_sr
.macro waitkey
	jsr _waitkey_sr
.endmacro


;; waitkeyreleased
;; Waits until all keys are released
;; Function works independly of IRQ
;; Registers modified: A
.import _waitkeyreleased_sr
.macro waitkeyreleased
	jsr _waitkeyreleased_sr
.endmacro


;; readWASDspace
;; Checks the keyboard for keypresses of W, A ,S, D and Space
;; Output is a byte in A in the same format as a joystick value
;; Function works independly of IRQ or ROM
;; Registers modified: A,X
.import _readWASDspace_sr
.macro readWASDspace
	jsr _readWASDspace_sr
.endmacro


;; makesys [linenumber[,text]]
;; Generates the code for a BASIC line with a SYS command and an optional text behind the sys command
;; This is similar to the command line option  -u __EXEHDR__
;; Difference is that with this function the code segment starts at $801, so .align is off only by 1
;; Default line number is 2020
.macro  makesys linenumber,text
       .byte $0b,$08
       .if .paramcount > 0
	    .byte <linenumber,>linenumber
       .else
	    .byte <2020,>2020
       .endif
       .byte $9e   ;SYS token
       ;.byte $30+2 ;systarget/1000
       ;.byte $30+systarget .mod 1000/100
       ;.byte $30+systarget .mod 100/10
       ;.byte $30+(systarget .mod 10)
       .if .paramcount = 2
            systarget=BASICSTART+$0c+.strlen(text)
            .byte .sprintf ("%d", systarget)
	    .asciiz text
       .else
            systarget=BASICSTART+$0c
            .asciiz .sprintf ("%d", systarget)
       .endif
       .byte $00,$00,$00
.endmacro

;; check_C128
;; Detects if we are on a C128 in C64 mode
;; returns with carry set for C128
.macro check_C128
     lda $d030
     cmp #$ff
.endmacro


;; identify_SID [baseaddress]
;; Detects the SID soundchip model
;; SID detection routine from codebase64 by SounDemon and a tip from Dag Lem
;; If no base address is given, the standard base address $d400 is used
;; Overwrites register A
;; Result is returned in carry
;; Carry flag is set for 6581, and clear for 8580
.macro identify_SID address
.scope
       .if .paramcount > 0
	    SIDbaseaddress=address
       .else
	    SIDbaseaddress=$d400
       .endif
       sei
       lda #$ff
waitraster:
       cmp $D012    ;wait until screen is in border
       bne waitraster

       sta SIDbaseaddress+$12  ;control register voice 3 
       sta SIDbaseaddress+$0e
       sta SIDbaseaddress+$0f  ;set frequency in voice 3 to $ffff
       lda #$20                ;sawtooth wave and gatebit off to start oscillator again
       sta SIDbaseaddress+$12
       lda SIDbaseaddress+$1b
       lsr
.endscope
.endmacro

;; disable_NMI
;; Executes a short routine to disable the NMI
;; the trick is to cause an NMI but don't ackowledge it
;; Uses CIA2 Timer A, but the timer can be used afterwards (without IRQ function)
;; Registers modified: A
.macro  disable_NMI
.import _disable_NMI_sr
       jsr _disable_NMI_sr
.endmacro

;; set_raster_irq
;; Changes the IRQ source to VIC raster interrupt
;; Turns off the CIA IRQ source
;; IRQ routine must acknowledge the IRQ source
.macro set_raster_irq rasterline
	lda #$7f
	sta $dc0d	; disable timer interrupts
	sta $dd0d

	and $D011	; accu is still $7f
	.if rasterline>255
	ora #$80
	.endif
	sta $D011 	; Clear most significant bit in VIC's raster register

	lda $dc0d	; acknowledge CIA interrupts

	lda #<rasterline
	sta $D012 	; Set the raster line number where interrupt should occur 

	lda #01
	sta $D01A 	; set raster interrupt
.endmacro

;; memset start_addr end_adder fillvalue
;; Fills the memory area between start_addr and end_adder with the byte fillvalue
;; Registers modified: A
.macro  memset start_addr,end_adder,fillvalue
.import _memset_sr
       jsr _memset_sr
.endmacro

;; decimal_flag_to_N
;; Copies the decimal flag into the negative flag to detect if decimal mode is on
;; Macro alsways returns with a cleared Carry
;; Registers modified: A 
.macro  decimal_flag_to_N
	lda #$42
	adc #$38	;this will result in $80 in decimal mode, but not in normal mode
.endmacro


