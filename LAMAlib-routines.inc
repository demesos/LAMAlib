;; <h2>Useful Routines</h2>

;; _ld_reg reg,arg
;; <b>_st_reg</b> <i>reg,arg</i>
;; <b>_cp_reg</b> <i>reg,arg</i>
;; macros to select register for a load, store, and compare operation.
;; For example, _ld_reg A,#12 translates into lda #12 while _st_reg $1234,Y translates into sty $1234
;; if reg is blank, A is used as a default
;; Those funcations are mostly used in other macros.

.macro _ld_reg reg,arg
	.ifblank reg
	   lda arg
	.elseif (.xmatch ({reg}, {A}))  
	   lda arg
	.elseif (.xmatch ({reg}, {X}))  
	   ldx arg
	.elseif (.xmatch ({reg}, {Y}))  
	   ldy arg
	.else
	   .error "No proper register stated, must be A, X or Y"
	.endif
.endmacro

.macro _st_reg reg,arg
	.ifblank reg
	   sta arg
	.elseif (.xmatch ({reg}, {A}))  
	   sta arg
	.elseif (.xmatch ({reg}, {X}))  
	   stx arg
	.elseif (.xmatch ({reg}, {Y}))  
	   sty arg
	.else
	   .error "No proper register stated, must be A, X or Y"
	.endif
.endmacro

.macro _cp_reg reg,arg
	.ifblank reg
	   cmp arg
	.elseif (.xmatch ({reg}, {A}))  
	   cmp arg
	.elseif (.xmatch ({reg}, {X}))  
	   cpx arg
	.elseif (.xmatch ({reg}, {Y}))  
	   cpy arg
	.else
	   .error "No proper register stated, must be A, X or Y"
	.endif
.endmacro

;; check_C128
;; Detects if we are on a C128 in C64 mode
;; returns with carry set for C128
.macro  check_c128   ;version with all lowercase
	check_C128
.endmacro

.macro check_C128
     lda $d030
     cmp #$ff
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


;; checksum_eor startaddr,endaddr
;; Returns an 8-bit checksum calculated by EOR-conjunction over all bytes
.macro checksum_eor startaddr,endaddr
	.scope
.importzp _llzp_word1
	pokew _llzp_word1,startaddr
	ldy #0
	tya
	pha
loop:
	pla
	eor (_llzp_word1),y
	pha
	inc16 _llzp_word1
	ldax _llzp_word1
	cmpax #endaddr+1
	bne loop
	pla
        .endscope
.endmacro

;; clear_window
;; Clears the window that was defined by the window parametes above. When chrout2window is enabled, the same effect can be achieved by lda #147, jsr $FFD2
.macro clear_window
.import _clear_window
	jsr _clear_window
.endmacro


;; decimal_flag_to_N
;; Copies the decimal flag into the negative flag to detect if decimal mode is on
;; Macro always returns with a cleared Carry
;; Registers modified: A 
.macro  decimal_flag_to_n   ;version with all lowercase
	decimal_flag_to_N
.endmacro

.macro  decimal_flag_to_N
        lda #$42
        adc #$38        ;this will result in $80 in decimal mode, but not in normal mode
.endmacro


;; delay_cycles arg
;; Delays for arg cycles using a busy waiting approach. This does not account for interrupts or stolen cycles by VIC badlines.
;; arg must be a constant >=2
;; generated code does not need to be aligned, but requires around 0.3 bytes/cycle in memory
;; Registers modified: none (but flags may be messed up)
.macro  delay_cycles arg
       .if (arg)=0
            .exitmacro
       .endif
       .if (arg)=1
            .error "cannot delay a single cycle since the shortest command takes 2 cycles"
            .exitmacro
       .endif
       .if (arg)=53 .or (arg)>54
	    .import _delay_53cycles
	    jsr _delay_53cycles
            delay_cycles ((arg)-53)
            .exitmacro
       .endif
       .if (arg)=47 .or (arg)>48
	    .import _delay_47cycles
	    jsr _delay_47cycles
            delay_cycles ((arg)-47)
            .exitmacro
       .endif
       .if (arg)=43 .or (arg)>44
	    .import _delay_43cycles
	    jsr _delay_43cycles
            delay_cycles ((arg)-43)
            .exitmacro
       .endif
       .if (arg)=31 .or (arg)>32
	    .import _delay_31cycles
	    jsr _delay_31cycles
            delay_cycles ((arg)-31)
            .exitmacro
       .endif
       .if (arg)=7 .or (arg)>8
	    pha
            pla
            delay_cycles ((arg)-7)
	    .exitmacro
       .endif
       .if (arg)=6 .or (arg)=8
            cmp ($42,X)
            delay_cycles ((arg)-6)
            .exitmacro
       .endif
       .if (arg)=3 .or (arg)=5
            bit $42
            delay_cycles ((arg)-3)
            .exitmacro
       .endif
       .if (arg)=2 .or (arg)=4
            nop
            delay_cycles ((arg)-2)
            .exitmacro
       .endif
       .error "cycle-exact count did not work"
.endmacro


;; delay_ms arg
;; Delays for arg milliseconds using a busy waiting loop. This does not account for interrupts or stolen cycles by VIC badlines.
;; When AX is given as argument it waits as many ms as the 16 bit value in A/X
;; When a number (1-65536) is given as argument it waits this long
;; Registers modified: A,X
.macro  delay_ms arg
.import _delay_ms_sr
.scope
       .if (.xmatch ({arg1}, {AX}) .or .xmatch ({arg1}, {ax}))
            jsr _delay_ms_sr
       .else
            ldax #arg
            jsr _delay_ms_sr
       .endif
.endscope
.endmacro


;; disable_chrout2window
;; Restores the original Kernal vector and disables the chrout2window mode
.macro disable_chrout2window
.import _disable_chrout2window
	jsr _disable_chrout2window
.endmacro


;; disable_NMI
;; Executes a short routine to disable the NMI
;; the trick is to cause an NMI but don't ackowledge it
;; Uses CIA2 Timer A, but the timer can be used afterwards (without IRQ function)
;; Registers modified: A
.macro  disable_nmi   ;version with all lowercase
	disable_NMI
.endmacro

.macro  disable_NMI
.import _disable_NMI_sr
       jsr _disable_NMI_sr
.endmacro


; diskstatus devicenr
;; Returns the disk status error message number in A
;; This code only reads the number without further text
;; Error codes (listed in decimal) are:
;;  0   OK, no error exists
;;  1   Files scratched response. Not an error condition
;;  20  Block header not found on disk
;;  21  Sync character not found
;;  22  Data block not present
;;  23  Checksum error in data
;;  24  Byte decoding error
;;  25  Write-verify error
;;  26  Attempt to write with write protect on
;;  27  Checksum error in header
;;  28  Data extends into next block
;;  29  Disk id mismatch
;;  30  General syntax error
;;  31  Invalid command
;;  32  Long line
;;  33  Invalid filename
;;  34  No file given
;;  39  Command file not found
;;  50  Record not present
;;  51  Overflow in record
;;  52  File too large
;;  60  File open for write
;;  61  File not open
;;  62  File not found
;;  63  File exists
;;  64  File type mismatch
;;  65  No block
;;  66  Illegal track or sector
;;  67  Illegal system track or sector
;;  70  No channels available
;;  71  Directory error
;;  72  Disk full or directory full
;;  73  Power up message, or write attempt with DOS Mismatch
;;  74  Drive not ready
.macro diskstatus devicenr
;ToDo - this should be a library routine with a short macro stub instead of a long macro
	.scope
        lda #0
	jsr SETNAM	;no parameter
	lda #$0f
	tay		;open 15,x,15
	.if devicenr = 0
	  ldx $BA
	  bne :+
	  ldx #$08	;use address 8 as default
:	  
	.else
	  ldx #devicenr
	.endif
	jsr SETLFS
	jsr OPEN
	ldx #$0f
	jsr CHKIN
	jsr CHRIN	;read tens digit
	sec
	sbc #$30
	asl
	sta sm_tens
	asl
	asl
sm_tens=*+1
	adc #00
	sta sm_to_add
	jsr CHRIN	;read ones digit
	sec
	sbc #$30
	clc
sm_to_add=*+1
	adc #00	
	pha
	jsr CLRCHN
	lda #$0f
	jsr CLOSE
	pla
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
            sty _div16_arg
            ldy     #>(.right (.tcount ({arg})-1, {arg}))
            sty _div16_arg+1
        .else
            ; assume absolute or zero page
            ldy     arg
            sty _div16_arg
            ldy     1+(arg)
            sty _div16_arg+1
        .endif  
         jsr _div16_sr ;div16 subroutine
.endscope
.endmacro


;; draw_frame
;; Draws a frame around the window defined by the window parameters
;; Note that the frame will go around the window, so it is larger than the defined window
;; Window parameters:
;; <tt>.import _window_x1,_window_y1,_window_x2,_window_y2</tt>
;; Further configuration parameters (default is a white frame using PETSCII characters):
;; <tt> .import _frame_upper_left
;;  .import _frame_upper_right
;;  .import _frame_lower_left
;;  .import _frame_lower_right
;;  .import _frame_vertical
;;  .import _frame_horizontal
;;  .import _frame_color</tt>
;; For example to change the color of the frame to green, write
;; <tt>  .import _frame_color
;;   poke _frame_color,5</tt>

.macro draw_frame
.import _draw_frame_sr
	jsr _draw_frame_sr
.endmacro


;; enable_chrout2window
;; Switches the Kernal chrout vector to a routine that prints within a window
;; The page of the textscreen (stored in $288 / 648) is used to determine the output screen, but if you change the screen page, <b>enable_chrout2window</b> needs to be called again.
;; Limitations: no backspace, no insert
;; Note that control character keypresses in direct mode are not handled via $FFD2, therefore pressing for example CLR/HOME will leave the window in direct mode.
;; Window parameters:
;; <tt>.import _window_x1,_window_y1,_window_x2,_window_y2</tt>
;; For example to set a window starting on column 5, write 
;; <tt>  .import _window_x1
;;   poke _window_x1,5</tt>
.macro enable_chrout2window
.import _enable_chrout2window
	jsr _enable_chrout2window
.endmacro

;; getkey
;; clears the key buffer, waits for a keypress and returns the ASCII value of the pressed key in A
;; relies on the interrupt routine regularily scanning the keyboard
.macro getkey
.import _getkey
.exportzp CHARS_IN_KEYBUF
	jsr _getkey
.endmacro


;; identify_SID [baseaddress]
;; Detects the SID soundchip model
;; SID detection routine from codebase64 by SounDemon and a tip from Dag Lem
;; If no base address is given, the standard base address $d400 is used
;; Overwrites register A
;; Result is returned in carry
;; Carry flag is set for 6581, and clear for 8580
.macro  identify_sid address   ;version with all lowercase
	identify_SID address
.endmacro

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


;; is_alpha
;; is_alpha tests if value in Accu is between the values 'a' and 'z' (in lowercase mode). 
;; Return value: Carry set if value is in range, carry cleared otherwise
;; Registers modified: none
.macro is_alpha
	is_in_range $41,$5a
.endmacro


;; is_not_alpha
;; is_alpha tests if value in Accu is <b>not</b> between the values 'a' and 'z' (in lowercase mode). 
;; Return value: Carry set if value is outside range, carry cleared otherwise
;; Registers modified: none
.macro is_not_alpha
	is_not_in_range $41,$5a
.endmacro


;; is_digit
;; is_digit tests if value in Accu is between the values '0' and '9'. 
;; Return value: Carry set if value is a digit, carry cleared otherwise
;; Registers modified: none
.macro is_digit
	eor #$cf
        cmp #$f5
	eor #$cf
.endmacro


;; is_not_digit
;; is_not_digit tests if value in Accu is <b>not</b> between the values '0' and '9'. 
;; Return value: Carry clear if value is a digit, carry set otherwise
;; Registers modified: none
.macro is_not_digit
	eor #$30
        cmp #$0a
	eor #$30
.endmacro


;; is_in_range lower,higher
;; Tests if value in Accu is between the values <i>lower</i> and <i>higher</i>
;; Return value: Carry set if value is in range, carry cleared otherwise
;; Registers modified: none
.macro is_in_range lower,higher
	cmp #lower
	bcc :+
        eor #$ff
	cmp #$ff-higher
        eor #$ff
:	
.endmacro

;; is_not_in_range lower,higher
;; Tests if value in Accu is outside the values <i>lower</i> and <i>higher</i>
;; Return value: Carry set if value is outside range, carry cleared otherwise
;; Registers modified: none
.macro is_not_in_range lower,higher
	cmp #higher+1
	bcs :+
        eor #$ff
	cmp #$ff-(lower-1)
        eor #$ff
:	
.endmacro


;; is_in_range_trash_A lower,higher
;; Tests if value in Accu is between the values <i>lower</i> and <i>higher</i>
;; If the value was inside, the Carry is set, otherwise the Carry is cleared
;; Registers modified: A
.macro is_in_range_trash_A lower,higher
        clc
        adc #$ff - higher
        adc #higher - lower + 1
.endmacro


;; load_prg filename[,devicenr[,loadaddr]]
;; Wrapper around ROM load function, prg means that the file is assumed to have a two-byte load address at its start
;; if device number is 0 or not stated, the last used device numner stored in address $BA is used. In case $BA contains 0, 8 is used as default
;; if loadaddr is omitted, the load address is defined by the first two bytes of the file
;; Return value in carry, if carry is set, an error has happened and error code is returned in A:
;;  A = $05 (DEVICE NOT PRESENT)
;;  A = $04 (FILE NOT FOUND)
;;  A = $1D (LOAD ERROR)
;;  A = $00 (BREAK, RUN/STOP has been pressed during loading)
;; Registers modified: A,X,Y
.macro load_prg filename,devicenr,loadaddr
        .scope
        lda #endfilename-beginfilename
	ldx #<beginfilename
	ldy #>beginfilename
	jsr SETNAM
	
	.ifblank devicenr
	  devno:=0
	.else
	  devno:=devicenr
	.endif

	.if devno = 0
	  ldx $BA
	  bne :+
	  ldx #$08	;use address 8 as default
:	  
	.else
	  ldx #devno
	.endif
	lda #$01
	.if .paramcount < 3
	  ldy #$01	;load file to address defined in file
	.else
	  ldy #$00	;load file to specified address
	.endif
	jsr SETLFS

	lda #$00	;load (instead of verify)
	.if .paramcount = 3
	  ldx #<loadaddr
	  ldy #>loadaddr
	.endif
	jsr LOAD

	jmp endfilename	;hop over filename data
beginfilename:
	.byte filename
endfilename:
        .endscope
.endmacro


;; lowercase_mode
;; Switches charset to upper/lowercase (text) mode setting and locks the CBM+Shift switch
;; To switch back, use the macro PETSCII_mode
;; Registers modified: A
.macro  lowercase_mode
.scope
                lda #$08
                jsr CHROUT
                lda #$0E
                jsr CHROUT
.endscope
.endmacro


;; mul16 addr
;; compactmul16 addr
;; Multiplies the unsigned 16 bit value in A/X with the 16 bit value stored at addr (lo-byte) and addr+1 (hi-byte)
;; Implemented as a subroutinge, link with -lib lamalib.lib
;; mul16 adds a routine of 51 byte to your program the first time you use it
;; compactmul16 adds a routine of 32 byte to your program the first time you use it
;; mul16 is about 20% faster than compactmul, we recommend using mul16 in most cases
;; This function is not reentrant, don't use in interrupt and main program simultaneously
;; Result is returned in A/X
;; Registers modified: A,X,Y

.import _fastmul16_sr
.importzp _fastmul16_arg

.macro mul16 arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            ldy     #<(.right (.tcount ({arg})-1, {arg}))
            sty _fastmul16_arg
            ldy     #>(.right (.tcount ({arg})-1, {arg}))
            sty _fastmul16_arg+1
        .else
            ; assume absolute or zero page
            ldy     arg
            sty _fastmul16_arg
            ldy     1+(arg)
            sty _fastmul16_arg+1
        .endif  
        jsr _fastmul16_sr ;fastmul16 subroutine
.endscope
.endmacro

.import _mul16_sr
.importzp _mul16_arg

.macro compactmul16 arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            ldy     #<(.right (.tcount ({arg})-1, {arg}))
            sty _mul16_arg
            ldy     #>(.right (.tcount ({arg})-1, {arg}))
            sty _mul16_arg+1
        .else
            ; assume absolute or zero page
            ldy     arg
            sty _mul16_arg
            ldy     1+(arg)
            sty _mul16_arg+1
        .endif  
        jsr _mul16_sr ;mul16 subroutine
.endscope
.endmacro


;; neg
;; Negate A
;; Registers modified: A
.macro  neg
	eor #$FF
	sec
	adc #0
.endmacro


;; newline
;; Prints a newline character
;; KERNAL ROM needs to be enabled when using this function
;; Registers modified: A
.macro  newline
.scope
                lda #$0d
                jsr CHROUT
.endscope
.endmacro


;; PETSCII_mode
;; Switches charset to uppercase plus graphical characters (graphics mode) setting and locks the CBM+Shift switch
;; To switch to upper/lowercase (text) mode, use the macro lowercase_mode
;; Registers modified: A
.macro  PETSCII_mode
.scope
                lda #$08
                jsr CHROUT
                lda #$8E
                jsr CHROUT
.endscope
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


;; print arg1 [arg2 ...]
;; Prints the arguments as strings or numbers, without a carriage return
;; An argument in parenthesis will print the 16bit value stored at this address
;; uses ROM functions, BASIC and KERNAL ROM need to be enabled when using this macro
;; Use .FEATURE STRING_ESCAPES to enable escapes codes like "\x05" (white) or "\0x0a" (carriage return)
;; Registers modified: none
.macro print arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
	store Y
	print_recursive1 arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
	restore Y
.endmacro

.macro print_recursive1 arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
  .ifblank arg1
      ; First parameter is empty
      .exitmacro
  .endif
.scope
        .if (.xmatch ({arg1}, {AX}) .or .xmatch ({arg1}, {ax}))    ;print AX as number
	   store AX
           printax
	   restore AX
       .elseif (.xmatch ({arg1}, {A}))                  ;print akku as number
	   store AX
           printa
	   restore AX
       .elseif (.match (.left (1, {arg1}), #))          ;immediate number
	   store AX
           iarg=(.right (.tcount ({arg1})-1, {arg1}))
           .if .const(iarg) .and (<iarg = >iarg)
              lda     #<iarg
              tax
           .else
              lda     #<iarg
              ldx     #>iarg
           .endif
           printax
	   restore AX
        .elseif (.match (.left (1, {arg1}), {(})) .and (.match (.right (1, {arg1}), {)}))       ;argument in brackets
	   store AX
           ldax .mid (1, .tcount ({arg1})-2, {arg1})
           printax
	   restore AX
        .else
	   store AX
           primm arg1
	   restore AX
       .endif
.endscope
        .if .paramcount > 1
                ;macro calls itself recursely to process remaining arguments
                print_recursive1 arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
        .endif
.endmacro


;; println arg1 [arg2 ...]
;; Prints the arguments as strings or numbers, followed by a carriage return
;; An argument in parenthesis will print the 16bit value stored at this address
;; uses ROM functions, BASIC and KERNAL ROM need to be enabled when using this macro
;; Use .FEATURE STRING_ESCAPES to enable escapes codes like "\x05" (white) or "\0x0a" (carriage return)
;; Registers modified: none
.macro println arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
	store Y
	print_recursive1 arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
	restore Y
        lda #$0d
        jsr CHROUT
.endmacro


;; printax
;; Prints the number in A/X as a 16 bit unsigned decimal number
;; BASIC and KERNAL ROM need to be enabled when using this function
;; Registers modified: A,Y,X
.macro  printax
.scope
	.if ::USE_BASIC_ROM>0
                sta setx +1     ;we need to switch A and X
                txa
setx:           ldx #00
                jsr LINPRT
	.else
		.import _printax_sr
		jsr _printax_sr
	.endif
.endscope
.endmacro


;; printa
;; Prints the number in A as a 8 bit unsigned decimal number
;; BASIC and KERNAL ROM need to be enabled when using this function
;; Registers modified: A,X
.macro  printa
.scope
	.if ::USE_BASIC_ROM>0
	        tax
	        lda #00
                jsr LINPRT
	.else
		.import _printa_sr
		jsr _printa_sr
	.endif
.endscope
.endmacro


;; printax_signed
;; Prints the number in A/X as a 16 bit signed decimal number
;; BASIC and KERNAL ROM need to be enabled when using this function
;; Registers modified: A,Y,X
.macro  printax_signed
.scope
                cpx #$80        ;is it a negative number?
                bcc positive
                negax           ;negate number
                pha
                lda #'-'        ;write out minus sign
                jsr CHROUT
                pla
positive:       

	.if ::USE_BASIC_ROM>0
                sta setx +1     ;we need to switch A and X
                txa
setx:           ldx #00
                jsr LINPRT
	.else
		.import _printax_sr
		jsr _printax_sr
	.endif
.endscope
.endmacro

;; printstr addr
;; Prints the null-terminated string at addr using the STROUT function
;; BASIC and KERNAL ROM need to be enabled when using this function
;; Registers modified: A,Y,X
.macro  printstr   addr
                lda #<addr
                ldy #>addr
	.if ::USE_BASIC_ROM>0
                jsr STROUT
	.else
		.import _strout_sr
		jsr _strout_sr
	.endif
.endmacro


;; rand8 [maxvalue-1]
;; Generate a random number between 0 and 255 which is returned in A.
;; The argument is optional and can be a number or the Y register
;; If an argument is given, the value is caclulated between 0 and given number-1
;; Good and fast random generator for 8bit values based on the X ABC pseudo-random number generator from EternityForest, slight modification by Wil.
;; Registers modified: A,Y (Y is only used when an argument is used)
.import _rand16_sr, _seed_low, _seed_high, _a_mod_y_sr

.macro rand8 max
.scope
        .if .paramcount = 1
	  .if (.xmatch ({max}, {A}))                  ;calc mod A
	    tay
	  .elseif (.xmatch ({max}, {X}))              ;calc mod X
	    txa
	    tay
	  .endif
	.endif
        jsr _rand16_sr
        .if .paramcount = 1
	  .if (.xmatch ({max}, {A}))                  ;calc mod A, value already in Y
	    jsr a_mod_y_sr
	  .elseif (.xmatch ({max}, {X}))              ;calc mod X, value already in Y
	    jsr a_mod_y_sr
	  .elseif (.xmatch ({max}, {Y}))              ;calc mod Y         
	    jsr a_mod_y_sr
	  .elseif max <= 1
	    and #0
	  .elseif max = 2
	    and #1
	  .elseif max = 4
	    and #3
	  .elseif max = 8
	    and #7
	  .elseif max = 16
	    and #15
	  .elseif max = 32
	    and #31
	  .elseif max = 64
	    and #63
	  .elseif max = 128
	    and #127
	  .else
	    ldy #max
	    jsr _a_mod_y_sr
	  .endif
	.endif
.endscope
.endmacro


;; rand16 [maxvalue-1]
;; Get a random number in AX betwenn 0 and 0xFFFF
;; The argument is optional and can be a number or AX
;; If an argument is given, the value is caclulated between 0 and given number-1
;; This function uses the same routine as rand8, therefore calling rand16 will change the state of rand8 as well
;; Registers modified: A,X,Y (Y is only used when an argument is used)
.import _rand16_sr, _seed_low, _seed_high, _ax_mod_zpw1_sr
.importzp _llzp_word1
.macro rand16 max
.scope
        .if .paramcount = 1
	  .if (.xmatch ({max}, {AX}) .or .xmatch ({max}, {ax}))                 ;calc mod AX
	    stax _llzp_word1
	  .endif
	.endif
        jsr _rand16_sr
	ldx _seed_low
        .if .paramcount = 1
	  .if (.xmatch ({max}, {AX}) .or .xmatch ({max}, {ax}))                 ;calc mod AX
	     jsr _ax_mod_zpw1_sr
	  .elseif max <= 1
	    ldx #0
	    and #0
	  .elseif max = 2
	    ldx #0
	    and #1
	  .elseif max = 4
	    ldx #0
	    and #3
	  .elseif max = 8
	    ldx #0
	    and #7
	  .elseif max = 16
	    ldx #0
	    and #15
	  .elseif max = 32
	    ldx #0
	    and #31
	  .elseif max = 64
	    ldx #0
	    and #63
	  .elseif max = 128
	    ldx #0
	    and #127
	  .elseif max = 256
	    ldx #0
	  .elseif max = 512
	    pha
	    txa
	    and #1
	    tax
	    pla
	  .elseif max = 1024
	    pha
	    txa
	    and #3
	    tax
	    pla
	  .elseif max = 2048
	    pha
	    txa
	    and #7
	    tax
	    pla
	  .elseif max = 4096
	    pha
	    txa
	    and #15
	    tax
	    pla
	  .elseif max = 8192
	    pha
	    txa
	    and #31
	    tax
	    pla
	  .elseif max = 16384
	    pha
	    txa
	    and #63
	    tax
	    pla
	  .elseif max = 32768
	    pha
	    txa
	    and #127
	    tax
	    pla
	  .else
	    ldy #<(max)
	    sty _llzp_word1
	    ldy #>(max)
	    sty _llzp_word1+1
	    jsr _ax_mod_zpw1_sr
	  .endif
	.endif
.endscope
.endmacro


;; <b>rand_setseed</b> AX | [[<i>arg1</i>|A|X|Y], <i>arg2</i>|A|X|Y]
;; Seed is two 8 bit values. Arguments can be numbers or registers. Specifying seed is optional. When no argument is given, the default value for this seed byte is used.
;; When setting with AX, there is a check to avoid the seed 0 which would cause the PRNG to lock up. The seed affects both, rand8 and rand16 functions.
;; Registers modified: normally none, except for the case where lo(AX)==0, then A is set to $FF
.import _seed_low, _seed_high
.macro rand_setseed arg1,arg2
.scope
        .if .paramcount = 0
	  lda #$33
	  sta _seed_low
	  sta _seed_high
	  .endscope
          .exitmacro
	.endif	

        .if .paramcount = 1 .and (.xmatch ({arg1}, {AX}) .or .xmatch ({arg1}, {ax}))
	    cmp #1		;set carry if value != 0
	    sbc #0
	    sta _seed_low
	    stx _seed_high
	    
	.else
          .if .paramcount >= 1
	    .if .xmatch ({arg1}, {A})
	      sta _seed_low
	    .elseif .xmatch ({arg1}, {X})
	      stx _seed_low 
	    .elseif .xmatch ({arg1}, {Y})
	      sty _seed_low 
	    .else 
	      lda #arg1
	      sta _seed_low
	      .if .paramcount = 1
	        sta _seed_high
	      .endif
	    .endif
	  .endif

          .if .paramcount = 2
	    .if .xmatch ({arg2}, {A})
	      sta _seed_high
	    .elseif .xmatch ({arg2}, {X})
	      stx _seed_high 
	    .elseif .xmatch ({arg2}, {Y})
	      sty _seed_high 
	    .else 
	      lda #arg2
	      sta _seed_high
	    .endif
	  .endif

	.endif	

.endscope
.endmacro



;; set_cursor_pos line,column
;; place the cursor at screen position line,column (counted in characters)
;; 0,0 is upper left corner, 24,39 the lower right corner
;; arguments can be constants or registers
.macro  set_cursor_pos   line,column
.scope
	.if ( .not .xmatch ({line}, {X}) )
	  .if .xmatch ({line}, {Y})
            sty to_x
            xvalue:=0
          .elseif .xmatch ({line}, {A})
	    sta to_x
            xvalue:=0
          .else
            xvalue:=line
          .endif
        .endif 

	.if ( .not .xmatch ({column}, {Y}) )
	  .if .xmatch ({column}, {X})
            stx to_y
            yvalue:=0
          .elseif .xmatch ({column}, {A})
	    sty to_y
            yvalue:=0
          .else
            yvalue:=column
          .endif
          to_y=*+1
          ldy #yvalue
        .endif 

	.if ( .not .xmatch ({line}, {X}) )
	  to_x=*+1
          ldx #xvalue
        .endif

	clc
	jsr PLOT
.endscope
.endmacro


;; textcolor color
;; sets the text color
.macro  textcolor color
.scope
                poke TEXTCOLOR_ADDR,color
.endscope
.endmacro


;; toggle_carry
;; Toggles the carry 
;; Registers modified: none
.macro  toggle_carry
        rol
        eor #$01
        ror
.endmacro


;; turn_off_cursor
;; Turns off the blinking cursor and restores the character and color under cursor if necessary.
;; Requires the KERNAL IRQ routines to be active
;; If this function is used on the C128, a bank with visible ROM must be active.
;; Use this before output of a char, otherwise you get inverse character artifacts when blink phase is on
.macro turn_off_cursor
.if .def(__C128__)
	jsr $CD9F	;bank 15 needs to be enabled
.else
.import _turn_off_cursor_sr
	jsr _turn_off_cursor_sr
.endif
.endmacro	


;; turn_on_cursor
;; Shows the blinking cursor during program execution
;; Requires the KERNAL IRQ routines to be active
;; if this function is used on the C128, a bank with visible ROM must be active.
;; Turn off cursor briefly before output of a char, otherwise you get inverse character artifacts when blink phase is on
.macro turn_on_cursor
.if .def(__C128__)
	jsr $CD6F	;Bank 15 needs to be enabled
.else
	lda #0
	sta 204		;cursor visibility switch for C64, VIC20
.endif
.endmacro


;; waitkey
;; Waits until a key is pressed
;; Function works independly of IRQ routine
;; Registers modified: A
.import _waitkey_sr
.macro waitkey
        jsr _waitkey_sr
.endmacro


;; waitkeyreleased
;; Waits until all keys are released
;; Function works independly of IRQ routine
;; Registers modified: A
.import _waitkeyreleased_sr
.macro waitkeyreleased
        jsr _waitkeyreleased_sr
.endmacro


;; readWASDspace
;; Checks the keyboard for keypresses of W, A ,S, D and Space
;; Output is a byte in A in the same format as a joystick value
;; Since movement of joystick 1 disturbs the value, no keys are returen if joystick 1 is moved to any direction
;; Function does not require the Kernal keyboard routine that is in the IRQ
;; Registers modified: A,X
.import _readWASDspace_sr
.macro readwasdspace   ;version with all lowercase
        readWASDspace
.endmacro


.macro readWASDspace
        jsr _readWASDspace_sr
.endmacro


;; makesys [linenumber[,text[, address]]]
;; Generates the code for a BASIC line with a SYS command, an optional text behind the sys command, and an optional specified entry address
;; This is similar to the command line option  -u __EXEHDR__
;; Difference is that with this function the code segment starts at $801, so .align is off only by 1
;; If no SYS address is given, the target address is calculated to be the address right after the BASIC stub
;; Default line number is 2020
.macro  makesys linenumber,text,address
       .byte $0b,$08
       .if .paramcount > 0
            .byte <linenumber,>linenumber
       .else
            .byte <2020,>2020
       .endif
       .byte $9e   ;SYS token
       ;.byte $30+systarget/1000
       ;.byte $30+systarget .mod 1000/100
       ;.byte $30+systarget .mod 100/10
       ;.byte $30+(systarget .mod 10)
       .if .paramcount < 2
            systarget=BASICSTART+$0c
	    .byte .sprintf ("%d", systarget), $00
       .elseif .paramcount = 2
            systarget=BASICSTART+$0c+.strlen(text)
	    .byte .sprintf ("%d", systarget)
            .asciiz text
       .elseif .paramcount = 3
	    .byte .sprintf ("%d", address)
            .asciiz text
       .endif
       .byte $00,$00
.endmacro


;; memcopy src_addr,target_addr,length
;;
;; memcopy_from [AX|addr]
;; memcopy_to   [AX|addr]
;; memcopy [AX|length]
;; Copies the memory area src_addr to src_addr+length over target_addr
;; If the areas are overlapping, then target_addr must be < src_addr
;; The three parameter version takes three constant numbers
;; Alternatively, the function can be configured parameter by parameter, either with AX or a constant
;; Registers modified: A,X,Y
.macro  memcopy src_addr,target_addr,length
        .scope
.import _memcopy_sr
.importzp _source_ptr,_target_ptr
        .if .paramcount = 3
          yinit=<($100-<(length))
          pokew _source_ptr,src_addr - yinit
          pokew _target_ptr,target_addr - yinit
  
          ldx #1+>(length-1)
          ldy #yinit
	.elseif .paramcount = 1
          .if (.xmatch ({src_addr}, {AX}) .or .xmatch ({src_addr}, {ax}))          ;AX is used as length
            tay
            beq skip_adjust

	    pha
	    clc
            adc _source_ptr
	    sta _source_ptr
            dec _source_ptr+1
            pla
	    clc
            adc _target_ptr
	    sta _target_ptr
            dec _target_ptr+1

	    neg
            tay
            inx
skipadjust:
          .else                                     ;length is given as constant parameter
            yinit=<($100-<length)
            ldx #1+>(length-1)
            ldy #yinit
	    .if yinit>0
              lda #<length
              clc
              adc _source_ptr
              sta _source_ptr
              dec _source_ptr+1
              lda #<length
              clc
              adc _target_ptr
              sta _target_ptr
              dec _target_ptr+1
            .endif
          .endif
	.else
	  .error "Wrong number of parameters for memcopy!"
	.endif
        jsr _memcopy_sr
        .endscope
.endmacro

.macro memcopy_from src_addr
        .scope
.importzp _source_ptr
        .if (.xmatch ({src_addr}, {AX}) .or .xmatch ({src_addr}, {ax}))          ;AX is used as address
          sta _source_ptr
          stx _source_ptr+1
        .else
          lda #<src_addr
          sta _source_ptr
          lda #>src_addr
          stx _source_ptr+1
        .endif
        .endscope
.endmacro

.macro memcopy_to target_addr
        .scope
.importzp _target_ptr
        .if (.xmatch ({target_addr}, {AX}) .or .xmatch ({target_addr}}))         ;AX is used as address
          sta _target_ptr
          stx _target_ptr+1
        .else
          lda #<target_addr
          sta _target_ptr
          lda #>target_addr
          stx _target_ptr+1
        .endif
        .endscope
.endmacro

;; memset start_addr,end_addr,fillvalue
;; Fills the memory area between start_addr and end_adder with the byte fillvalue
;; Registers modified: A,X,Y
.macro  memset start_addr,end_addr,fillvalue
.import _memset_sr
.importzp _target_ptr
        .scope
        length = end_addr - start_addr + 1
        yinit=<($100-<length)
        pokew _target_ptr,start_addr - yinit

        ldx #1+>(length-1)
        ldy #yinit
        lda #fillvalue
        jsr _memset_sr
        .endscope
.endmacro


;; save_prg filename,devicenr,startaddr,endaddr
;; Saves the memory from startaddr to endaddr (including the endaddr) using the CBM ROM save function.
;; The file will have a two-byte load address at its start
;; if device number is 0, the last used device numner stored in address $BA is used. In case $BA contains 0, 8 is used as default
;; Return value in carry, if carry is set, an error has happened and error code is returned in A, for example
;;  A = $05 (DEVICE NOT PRESENT)
;;  A = $00 (BREAK, RUN/STOP has been pressed during saving)
;; Registers modified: A,X,Y
.macro save_prg filename,devicenr,startaddr,endaddr
        .scope
        lda #endfilename-beginfilename
	ldx #<beginfilename
	ldy #>beginfilename
	jsr SETNAM

	.if devicenr = 0
	  ldx $BA
	  bne :+
	  ldx #$08	;use address 8 as default
:	  
	.else
	  ldx #devicenr
	.endif
.importzp _llzp_word1
	pokew _llzp_word1,startaddr
	lda #_llzp_word1
	ldx #<(endaddr+1)
	ldy #>(endaddr+1)
	jsr SAVE

	jmp endfilename	;hop over filename data
beginfilename:
	.byte filename
endfilename:
        .endscope
.endmacro



;; scramble startaddr,endaddr
;; Scrambles/unscrambles a memory area. To unscramble, call scramble with the same parameters again
.macro scramble startaddr,endaddr
	.scope
.importzp _llzp_word1,_llzp_byte1,_llzp_byte2
	checksum_eor startaddr,endaddr
	sta _llzp_byte1

	;now do the scrambling
	pokew _llzp_word1,startaddr
	ldy #0
	sty _llzp_byte2
loop:
	;generate next byte from a pseudo random sequence
sm:	lda _llzp_byte1
	beq doEor
        asl
	beq noEor
        bcc noEor
doEor:	eor #$e7
noEor:  sta _llzp_byte1
	pha
	eor (_llzp_word1),y
	sta (_llzp_word1),y
	pla

	eor _llzp_byte2
	sta _llzp_byte2

	inc16 _llzp_word1
	ldax _llzp_word1
	cmpax #endaddr

	bne loop

	;the last eor byte will be chosen to match the same eor checksum

	lda _llzp_byte2
	eor (_llzp_word1),y
	sta (_llzp_word1),y

        .endscope
.endmacro


;; scratch_file filename[,devicenr]
;; Deletes a file on disk. If device number is not stated or 0, the last used device number stored in address $BA is used. In case $BA contains 0, 8 is used as default.
;; Registers modified: A,X,Y
.macro scratch_file filename,devicenr
	.if .paramcount = 1
          scratch_file filename,0
          .exitmacro
        .endif

	.scope
	lda #$0f
	tay		;open 15,x,15
	.if devicenr = 0
	  ldx $BA
	  bne :+
	  ldx #$08	;use address 8 as default
:	  
	.else
	  ldx #devicenr
	.endif
	jsr SETLFS

        lda #endfilename-beginfilename
	ldx #<beginfilename
	ldy #>beginfilename
	jsr SETNAM

	jsr OPEN
	lda #$0f
	jsr CLOSE
	jmp endfilename	;hop over filename data

beginfilename:
	.byte "s:"
	.byte filename
endfilename:
	.endscope
.endmacro


;; set_raster_irq rasterline
;; Changes the IRQ source to VIC raster interrupt
;; Turns off the CIA IRQ source
;; IRQ routine must acknowledge the IRQ source
.macro set_raster_irq rasterline
	.ifblank rasterline
	  rline_value .set 0
	.else
	  rline_value .set rasterline
	.endif
        lda #$7f
        sta $dc0d       ; disable timer interrupts
        sta $dd0d

        and $D011       ; accu is still $7f
        .if rline_value>255
        ora #$80
        .endif
        sta $D011       ; Clear most significant bit in VIC's raster register

        lda $dc0d       ; acknowledge CIA interrupts

        lda #<rline_value
        sta $D012       ; Set the raster line number where interrupt should occur 

        lda #01
        sta $D01A       ; set raster interrupt
.endmacro


;; to_screencode
;; If the PETSCII value in A belongs to a printable character, it is converted to the corresponding screencode and returned in A
;; Control codes will lead to an arbitrary byte.
;; Registers modified: A
.macro to_screencode
        .scope
        cmp #$60
        bcc L1+1
        ora #$40
        and #$7f
L1:
        bit $3f29    ;contains command AND #$3f 
        .endscope
.endmacro


;; wait_for_rasterline rasterline[,reg]
;; Macro inserting code doing busy waiting until the given rasterline is reached
;; rasterline can be a value between 0 and 311 (for PAL systems) or 261 for NTSC systems, respectively
;; The routine does not turn off the IRQ, so an IRQ might make it miss the rasterline it is waiting for.
;; Registers modified: A or the register (A,X, or Y) given as second argument

.macro wait_for_rasterline rasterline,reg
.scope
.if rasterline>255 .or rasterline<56
:	;unnamed label is only set if necessary to avoid a warning
.endif
        _ld_reg reg,#<(rasterline)
:
        _cp_reg reg,$D012    		;wait until lobyte matches
        bne :-
        .if rasterline>255
          _ld_reg reg,$D011		;check highbyte to be set
          bpl :--
        .endif
        .if rasterline<56
          _ld_reg reg,$D011		;check highbyte to be cleared
          bmi :--
        .endif
.endscope
.endmacro


;; screen_off
;; turns off the screen via $d011
;; The function always sets a 0 as high bit to avoid an unreachable raster irq line.

.macro screen_off
.import _screen_off
	jsr _screen_off
.endmacro

;; wait_screen_off
;; Waits until raster>249 and turns screen off. Since it is necessary to write to $d011 to turn the screen off, we cannot avoid setting the high bit.
;; The function always sets a 0 as high bit to avoid an unreachable raster irq line.

.macro wait_screen_off
.import _wait_screen_off
	jsr _wait_screen_off
.endmacro

;; wait_screen_on
;;turns the screen back on. Since it is necessary to write to $d011, the high bit of the next raster IRQ will be written (with a 0)

.macro screen_on
.import _screen_on
	jsr _screen_on
.endmacro
