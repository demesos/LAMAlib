;; <h2>Useful Routines</h2>

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

;; toggle_carry
;; Toggles the carry 
;; Registers modified: none
.macro  toggle_carry
        rol
        eor #$01
        ror
.endmacro

;; neg
;; Negate A
;; Registers modified: A
.macro  neg
	eor #$FF
	sec
	adc #0
.endmacro


;; rand8
;; Generate a random number between 0 and 255 which is returned in A.
;; Good and fast random generator for 8bit values based on the X ABC pseudo-random number generator from EternityForest, slight modification by Wil.
;; Registers modified: A
.import _rand8_sr, _rand_seed1, _rand_seed2, _rand_seed3, _rand_seed4

.macro rand8
.scope
         jsr _rand8_sr
.endscope
.endmacro


;; rand16
;; Get a random number in AX betwenn 0 and 0xFFFF
;; This function uses the same routine as rand8, therefore calling rand16 wil change the state of rand8 as well
;; Registers modified: A,X
.import _rand16_sr, _rand16_seed

.macro rand16
.scope
        jsr _rand8_sr
	ldx _rand_seed3
.endscope
.endmacro


;; <b>rand_setseed</b> [[[[<i>arg1], arg2], arg3], arg4</i>]
;; Seed is four 8 bit values. Arguments can be numbers or registers. Specifying seed is optional. When no argument is given, the default value for this seed byte is used.
;; Registers modified: A
.macro rand_setseed arg1,arg2,arg3,arg4
.scope
        .if .paramcount >= 1
           poke _rand_seed1,arg1
        .else
           poke _rand_seed1,$00
	.endif

	if .paramcount >= 2
           poke _rand_seed2,arg2
	.else
           poke _rand_seed2,$11
	.endif

	.if .paramcount >= 3
           poke _rand_seed3,arg3
	.else
           poke _rand_seed3,$37
        .endif 

	.if .paramcount = 4
           poke _rand_seed4,arg4
	.else
           poke _rand_seed4,$c2
        .endif 

        sta _rand8_seed
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

.import _fastmul16_sr
.importzp _fastmul16_arg

.macro fastmul16 arg
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
        .if (.xmatch ({arg1}, {AX}))                    ;print AX as number
           printax
       .elseif (.xmatch ({arg1}, {A}))                  ;print akku as number
           ldx #00
           printax
       .elseif (.match (.left (1, {arg1}), #))          ;immediate number
            iarg=(.right (.tcount ({arg1})-1, {arg1}))
            .if .const(iarg) .and (<iarg = >iarg)
               lda     #<iarg
               tax
            .else
               lda     #<iarg
               ldx     #>iarg
            .endif
            printax
        .elseif (.match (.left (1, {arg1}), {(})) .and (.match (.right (1, {arg1}), {)}))       ;argument in brackets
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
	.if USE_BASIC_ROM>0
                jsr STROUT
	.else
		.import _strout_sr
		jsr _strout_sr
	.endif
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

;; textcolor color
;; sets the text color
.macro  textcolor color
.scope
                poke TEXTCOLOR_ADDR,color
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


;; lowercase_mode
;; Switches charset to upper/lowercase (text mode) setting and locks the CBM+Shift switch
;; Registers modified: A
.macro  lowercase_mode
.scope
                lda #$08
                jsr CHROUT
                lda #$0E
                jsr CHROUT
.endscope
.endmacro

;; PETSCII_mode
;; Switches charset to uppercase plus graphical characters (graphics mode) setting and locks the CBM+Shift switch
;; Registers modified: A
.macro  PETSCII_mode
.scope
                lda #$08
                jsr CHROUT
                lda #$8E
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
;; Delays for arg milliseconds using a busy waiting loop. This does not account for interrupts or stolen cycles by VIC badlines.
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
;; Function works independly of IRQ routine
;; Registers modified: A,X
.import _readWASDspace_sr
.macro readwasdspace   ;version with all lowercase
        readWASDspace
.endmacro


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
       .byte $00,$00
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

;; wait_for_rasterline rasterline
;; Macro inserting code doing busy waiting until the given rasterline is reached
;; rasterline can be a value between 0 and 311 (for PAL systems) or 261 for NTSC systems, respectively
;; The routine does not turn off the IRQ, so an IRQ might make it miss the rasterline it is waiting for.
;; Registers modified: A
.macro wait_for_rasterline rasterline
.if rasterline>255 .or rasterline<56
:
.endif
        lda #<(rasterline)
:
        cmp $D012    		;wait until lobyte matches
        bne :-
        .if rasterline>255
          lda $D011		;check highbyte to be set
          bpl :--
        .endif
        .if rasterline<56
          lda $D011		;check highbyte to be cleared
          bmi :--
        .endif
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
          yinit=<($100-<length)
          pokew _source_ptr,src_addr - yinit
          pokew _target_ptr,target_addr - yinit
  
          ldx #1+>(length-1)
          ldy #yinit
	.elseif .paramcount = 1
          .if (.xmatch ({src_addr}, {AX}))          ;AX is used as length
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
        .if (.xmatch ({src_addr}, {AX}))          ;AX is used as address
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
        .if (.xmatch ({target_addr}, {AX}))          ;AX is used as address
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

;; is_in_range lower,higher
;; Tests if value in Accu is between the values <i>lower</i> and <i>higher</i>
;; If the value was inside, the Carry is set, otherwise the Carry is cleared
;; Registers modified: A
.macro is_in_range lower,higher
        clc
        adc #$ff - higher
        adc #higher - lower + 1
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
