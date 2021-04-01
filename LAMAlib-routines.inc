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
;; <b>rand16_setseed</b> [<i>arg</i>]<br>
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
        cmpax #32755    ;this is the one seed we need to avoid
        bne ok
        and #$FE ;make it 32754
ok:     sta _rand16_seed
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
                sta setx +1     ;we need to switch A and X
                txa
setx:           ldx #00
                jsr LINPRT
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
positive:       sta setx +1     ;we need to switch A and X
                txa
setx:           ldx #00
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
       .byte $00,$00,$00
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
	.scope
	.ifblank rasterline
	  rline:=0
	.else
	  rline:=rasterline
	.endif
        lda #$7f
        sta $dc0d       ; disable timer interrupts
        sta $dd0d

        and $D011       ; accu is still $7f
        .if rline>255
        ora #$80
        .endif
        sta $D011       ; Clear most significant bit in VIC's raster register

        lda $dc0d       ; acknowledge CIA interrupts

        lda #<rline
        sta $D012       ; Set the raster line number where interrupt should occur 

        lda #01
        sta $D01A       ; set raster interrupt
	.endscope
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


