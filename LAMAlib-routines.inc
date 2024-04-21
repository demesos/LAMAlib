;; <h2>Useful Routines</h2>

;; _ld_reg reg,arg
;; <b>_st_reg</b> <i>reg,arg</i>
;; <b>_cp_reg</b> <i>reg,arg</i>
;; macros to select register for a load, store, and compare operation.
;; For example, _ld_reg A,#12 translates into lda #12 while _st_reg $1234,Y translates into sty $1234
;; if reg is blank, A is used as a default
;; Those functions are mostly used in other macros.

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
;; Delays for arg milliseconds using a busy waiting loop. 
;; The waiting loop is calibrated to avergage available CPU cycles on a C64 with VIC and interrupts enabled.
;; Depending on interrupts or stolen cycles by VIC badlines the actual delay time may vary. 
;; When AX is given as argument it waits as many ms as the 16 bit value in AX
;; When a number 1-65536 is given as argument it waits this long
;; Registers modified: A,X,Y
.macro  delay_ms arg
.import _delay_ms_sr
.scope
       .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
            jsr _delay_ms_sr
       .else
            ldax #arg
            jsr _delay_ms_sr
       .endif
.endscope
.endmacro


;; delay_ms_abort_on_fire arg
;; Delays for arg milliseconds using a busy waiting loop. 
;; The waiting loop is calibrated to avergage available CPU cycles on a C64 with VIC and interrupts enabled.
;; Depending on interrupts or stolen cycles by VIC badlines the actual delay time may vary. 
;; When AX is given as argument it waits as many ms as the 16 bit value in AX
;; When a number 1-65536 is given as argument it waits this long
;; If the loop ran through, the carry flag is set upon return
;; If firebutton was detected, the loop is aborted and the carry flag is cleared upon return
;; Registers modified: A,X,Y
.macro  delay_ms_abort_on_fire arg
.import _delay_ms_abort_on_fire_sr
.scope
       .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
            jsr _delay_ms_abort_on_fire_sr
       .else
            ldax #arg
            jsr _delay_ms_abort_on_fire_sr
       .endif
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


;; diskstatus devicenr
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

;; div8 arg
;; div8 #arg
;; Divides the unsigned 8 bit value in A by an immediate value or the 8 bit value stored at addr
;; Quotient is returned in A, remainder in X
;; Registers modified: A,X

.macro div8 arg
.scope
	sta quotient
     	lda #0
	ldx #8
	asl quotient
L1:	rol
	cmp arg
	bcc L2
	sbc arg
L2:	rol quotient
	dex
	bne L1
	tax
quotient=*+1
	lda #00
.endscope
.endmacro

;; div16by8 arg
;; div16by8 #arg
;; Divides the unsigned 16 bit value in AX by an immediate value or the 8 bit value stored at addr
;; Resulting quotient must be 8 bit
;; Quotient is returned in A, remainder in X
;; Registers modified: A,X

.macro div16by8 arg
.scope
	sta quotient
	txa
	ldx #8
	asl quotient
L1:	rol
	bcs L2
	cmp arg
	bcc L3
L2:	sbc arg
	sec
L3:	rol quotient
	dex
	bne L1
	tax
quotient=*+1
	lda #00
.endscope
.endmacro

;; div16 arg
;; div16 #arg
;; Divides the unsigned 16 bit value in AX by an immediate value or the 16 bit value stored at addr (lo-byte) and addr+1 (hi-byte)
;; Implemented as a subroutinge, link with -lib lamalib.lib
;; When using this function in interrupt save _div16_sr, _div16_rem, _div16_arg_lo, and _div16_arg_hi before calling and restore those values afterwards.
;; Result is returned in AX
;; Registers modified: all
.import _div16_sr
.importzp _div16_arg,_div16_rem

.macro div16 arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            ldy     #<(.right (.tcount ({arg})-1, {arg}))
            sty _div16_arg_lo
            ldy     #>(.right (.tcount ({arg})-1, {arg}))
            sty _div16_arg_hi
        .else
            ; assume absolute or zero page
            ldy     arg
            sty _div16_arg_lo
            ldy     1+(arg)
            sty _div16_arg_hi
        .endif  
         jsr _div16_sr ;div16 subroutine
.endscope
.endmacro


;; mod16 arg
;; mod16 #arg
;; Divides the unsigned 16 bit value in AX by an immediate value or the 16 bit value stored at addr (lo-byte) and addr+1 (hi-byte) and returns the remainder of the division
;; Implemented as a subroutinge, link with -lib lamalib.lib
;; When using this function in interrupt save _div16_sr, _div16_rem, _div16_arg_lo, and _div16_arg_hi before calling and restore those values afterwards.
;; Result is returned in AX
;; Registers modified: all
.import _div16_sr
.importzp _div16_arg,_div16_rem

.macro mod16 arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            ldy     #<(.right (.tcount ({arg})-1, {arg}))
            sty _div16_arg_lo
            ldy     #>(.right (.tcount ({arg})-1, {arg}))
            sty _div16_arg_hi
        .else
            ; assume absolute or zero page
            ldy     arg
            sty _div16_arg_lo
            ldy     1+(arg)
            sty _div16_arg_hi
        .endif  
         jsr _div16_sr ;div16 subroutine
	 ldax _div16_rem
.endscope
.endmacro


;; disable_cbm_shift
;; disable Commodore-Shift
.macro  disable_cbm_shift
.scope
	lda #8
	jsr CHROUT
.endscope
.endmacro


;; enable_cbm_shift
;; enable Commodore-Shift, this key combination toggles between the uppercase/graphics and lowercase/uppercase character set
.macro  enable_cbm_shift
.scope
	lda #9
	jsr CHROUT
.endscope
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


;; disable_chrout2window
;; Restores the original Kernal vector and disables the chrout2window mode
.macro disable_chrout2window
.import _disable_chrout2window
	jsr _disable_chrout2window
.endmacro


;; clear_window
;; Clears the window that was defined by the window parametes of chrout2window. When chrout2window is enabled, the same effect can be achieved by lda #147, jsr $FFD2
.macro clear_window
.import _clear_window
	jsr _clear_window
.endmacro


;; draw_frame
;; Draws a frame around the window defined by the window parameters
;; Note that the frame will go around the window, so it is larger than the defined window
;; When using in interrupt, save contents of _llzp_word1,_llzp_word2 before calling and restore afterwards
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


;; getkey
;; clears the key buffer, waits for a keypress and returns the ASCII value of the pressed key in A
;; Function depends on Kernal ROM and the IRQ routine regularily scanning the keyboard
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


;; include_file_as filename,identifier
;; This macro helps in linking external files to the project that should be copied into a target area later or accessed directly
;; filename should be given as a quoted string. If the filename ends with ".prg" it is assumed that the first two byter are the loading address, these bytes are skipped for the calculation of start address and length
;; Example: include_file_as "sprites.bin",sprites
;; (identifier)_filestart contains the first address where the file was included (in the example that would be sprites_filestart)s
;; (identifier)_filestart contains the last address (+1) where the file was included 
;; (identifier)_length contains the length in bytes
;; This macro is convenient for small projects, consider using a linker for projects with many different large components 
.macro include_file_as filename,identifier
        ; Check if the argument string ends with '.prg' (any case)
        .if .strlen(filename)>3
          .if (.strat (filename, .strlen(filename)-4) = '.') .and ((.strat (filename, .strlen(filename)-3) = 'p')  .or (.strat (filename, .strlen(filename)-3) = 'P')) .and ((.strat (filename, .strlen(filename)-2) = 'r')  .or (.strat (filename, .strlen(filename)-2) = 'R')) .and ((.strat (filename, .strlen(filename)-1) = 'g')  .or (.strat (filename, .strlen(filename)-1) = 'G'))
            ::.ident (.sprintf ("%s_filetype", .string(identifier))) = 1
          .else
            ::.ident (.sprintf ("%s_filetype", .string(identifier))) = 0
          .endif
        .else
          ::.ident (.sprintf ("%s_filetype", .string(identifier))) = 0
        .endif

        .if ::.ident (.sprintf ("%s_filetype", .string(identifier))) = 1
          .incbin filename,0,2  
::.ident (.sprintf ("%s_filestart", .string(identifier))):
          .incbin filename,2    
        .else
::.ident (.sprintf ("%s_filestart", .string(identifier))):
          .incbin filename
        .endif
::.ident (.sprintf ("%s_fileend", .string(identifier))):
::.ident (.sprintf ("%s_length", .string(identifier))) .set (::.ident (.sprintf ("%s_fileend", .string(identifier))) - ::.ident (.sprintf ("%s_filestart", .string(identifier)))) 
.endmacro

;; install_file identifier[,target_addr]
;; If identifier is a string, the file with that name is imported here and copied to its start address, 
;; or if a target address is given to target_addr. 
;; If the filetype is different from PRG, target_addr must be specified.
;; If identifier is not a string it refers to a previously included file. In this case, there must be a
;; command include_file_as filename,identifier placed before calling install_file.
.macro install_file identifier,target_addr
        .import _memcopy_sr
        .importzp _llzp_word1,_llzp_word2

	.if .match(identifier,"string")
	  .scope
          .if .defined(::install_foobar_counter)
	    ::install_foobar_counter .set ::install_foobar_counter + 1
	  .else
	    ::install_foobar_counter .set 1
	  .endif
	  jmp over_it
	  include_file_as identifier,.ident(.sprintf ("x%d_filetype", ::install_foobar_counter))
	over_it:
	  install_file .ident(.sprintf ("x%d_filetype", ::install_foobar_counter)),target_addr
	  .endscope
	  .exitmacro
	.endif

        .ifblank target_addr
          .if ::.ident (.sprintf ("%s_filetype", .string(identifier))) = 0
            .error "Binary files have no load address, missing parameter target_addr"
          .endif
        .endif

	.ifndef ::.ident (.sprintf ("%s_filetype", .string(identifier)))
	  .error "File object not defined, typo?"
	.endif

        .scope
        .if ::.ident (.sprintf ("%s_length", .string(identifier))) < 256
          ;code for copying up to 255 bytes
          .ifblank target_addr
            ldax ::.ident (.sprintf ("%s_filestart", .string(identifier)))-2
            stax sm_target_addr
            ;x goes from length-1 down to 0
            ldx #::.ident (.sprintf ("%s_length", .string(identifier))) - 1
  copyloop1:
            lda ::.ident (.sprintf ("%s_filestart", .string(identifier))),x
sm_target_addr=*+1
            sta $affe,x
            dex
	    .if ::.ident (.sprintf ("%s_length", .string(identifier))) <= 128
	      bpl copyloop1
	    .else
	      cpx #$ff
              bne copyloop1
	    .endif
          .else ;ifblank target_addr
            ;x goes from length down to 1
            ldx #<::.ident (.sprintf ("%s_length", .string(identifier)))
  copyloop_cmp2:
            lda ::.ident (.sprintf ("%s_filestart", .string(identifier)))-1,x
            sta target_addr-1,x
            dex
            bne copyloop_cmp2
          .endif
          .endscope
          .exitmacro
        .endif

        .if ::.ident (.sprintf ("%s_length", .string(identifier))) = 256
          ;code copying 256 bytes
          .ifblank target_addr
            ldax ::.ident (.sprintf ("%s_filestart", .string(identifier)))-2
            stax sm_target_addr
          .endif
          ldx #0	;256 iterations
copyloop256:
          lda ::.ident (.sprintf ("%s_filestart", .string(identifier))),x
          .ifblank target_addr
sm_target_addr=*+1
            sta $affe,x
          .else
            sta target_addr,x
          .endif
          dex
          bne copyloop256
          .endscope
          .exitmacro
        .endif

        ;code for copying >256 bytes
        yinit = <($100-<::.ident (.sprintf ("%s_length", .string(identifier))) )
        ldy #yinit
        .ifblank target_addr
          ; using target address from file
          ldax ::.ident (.sprintf ("%s_filestart", .string(identifier)))-2
          .if yinit>0
            subax #yinit
          .endif
        .else
          ; target address is given as argument
          ldax #target_addr - yinit
        .endif
        stax _llzp_word1
        ldax #(::.ident (.sprintf ("%s_filestart", .string(identifier))) - yinit)
        stax _llzp_word2
        ldx #1+>(::.ident (.sprintf ("%s_length", .string(identifier))) -1)
        .endscope
        jsr _memcopy_sr
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
;; Multiplies the unsigned 16 bit value in AX with the 16 bit value stored at addr (lo-byte) and addr+1 (hi-byte)
;; Implemented as a subroutinge, link with -lib lamalib.lib
;; mul16 adds a routine of 51 byte to your program the first time you use it
;; compactmul16 adds a routine of 32 byte to your program the first time you use it
;; mul16 is about 20% faster than compactmul, we recommend using mul16 in most cases
;; When using this function in interrupt save _llzp_word1 and _llzp_word2 before calling and restore those values afterwards.
;; Result is returned in AX
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
;; if USE_BASIC_ROM .set 0 was done before, the BASIC ROM functions are replaced by onw implementations. This creates larger but also fatser code.
;; Use .FEATURE STRING_ESCAPES to enable escapes codes like "\x05" (white) or "\0x0a" (carriage return)
;; Registers modified: none
.macro print arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
	.if .defined(::nesting_level_print)
	  ::nesting_level_print .set ::nesting_level_print+1
	.else
	  ::nesting_level_print .set 1
	.endif
	sta .ident(.sprintf("print_macro_lbl%d", ::nesting_level_print))+1
	stx .ident(.sprintf("print_macro_lbl%d", ::nesting_level_print))+3
	sty .ident(.sprintf("print_macro_lbl%d", ::nesting_level_print))+5
	print_recursive1 arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
.ident(.sprintf("print_macro_lbl%d", ::nesting_level_print)):	
	lda #$af	;value will be overwritten to restore reg value
	ldx #$fe	;value will be overwritten to restore reg value
	ldy #$42	;value will be overwritten to restore reg value
.endmacro

;; println arg1 [arg2 ...]
;; Prints the arguments as strings or numbers, followed by a carriage return
;; An argument in parenthesis will print the 16bit value stored at this address
;; uses ROM functions, BASIC and KERNAL ROM need to be enabled when using this macro
;; if USE_BASIC_ROM .set 0 was done before, the BASIC ROM functions are replaced by onw implementations. This creates larger but also fatser code.
;; Use .FEATURE STRING_ESCAPES to enable escapes codes like "\x05" (white) or "\0x0a" (carriage return)
;; Registers modified: none
.macro println arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
	.if .defined(::nesting_level_print)
	  ::nesting_level_print .set ::nesting_level_print+1
	.else
	  ::nesting_level_print .set 1
	.endif
	sta .ident(.sprintf("print_macro_lbl%d", ::nesting_level_print))+1
	stx .ident(.sprintf("print_macro_lbl%d", ::nesting_level_print))+3
	sty .ident(.sprintf("print_macro_lbl%d", ::nesting_level_print))+5
	print_recursive1 arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
        lda #$0d
        jsr CHROUT
.ident(.sprintf("print_macro_lbl%d", ::nesting_level_print)):	
	lda #$af	;value will be overwritten to restore reg value
	ldx #$fe	;value will be overwritten to restore reg value
	ldy #$42	;value will be overwritten to restore reg value
.endmacro

.macro print_recursive1 arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
  .ifblank arg1
      ; First parameter is empty
      .exitmacro
  .endif
.scope
        .if (.xmatch ({arg1}, {AX}) .or .xmatch ({arg1}, {ax}))    ;print AX as number
           printax
       .elseif (.xmatch ({arg1}, {A}))                  ;print akku as number
           printa
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
           primm arg1	;immediate string
       .endif
.endscope
        .ifnblank arg2
	  ;restore A and X to prepare for next print part
	  lda .ident(.sprintf("print_macro_lbl%d", ::nesting_level_print))+1
	  ldx .ident(.sprintf("print_macro_lbl%d", ::nesting_level_print))+3
        .endif
	;recursely process remaining arguments
        print_recursive1 arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9
.endmacro

;; print_wrapped_setpars [x1],[width],[endchar]
;; set parameters for print_wrapped command
;; x1     first column of print window
;; width  width of print window in characters
;; endchar character to print after print_wrapped command, typical values are 13, 32, 0 (no extra char)
;; If not all parameter should be changed, the others can be omitted with commas, e.g. print_wrapped_setpars ,,13 sets only the endchar


.if .def(__C128__)
  .macro print_wrapped_setpars x1,width,endchar
        .import _print_wrapped_x1_128
        .import _print_wrapped_width_128
        .import _print_wrapped_endchar_128

        .if (.not .blank(x1))
          poke _print_wrapped_x1_128,x1
        .endif
        .if (.not .blank(width))
          poke _print_wrapped_width_128,width
        .endif
        .if (.not .blank(endchar))
          poke _print_wrapped_endchar_128,endchar
        .endif
  .endmacro
.elseif .def(__VIC20__)
  .macro print_wrapped_setpars x1,width,endchar
        .import _print_wrapped_x1_20
        .import _print_wrapped_width_20
        .import _print_wrapped_endchar_20

        .if (.not .blank(x1))
          poke _print_wrapped_x1_20,x1
        .endif
        .if (.not .blank(width))
          poke _print_wrapped_width_20,width
        .endif
        .if (.not .blank(endchar))
          poke _print_wrapped_endchar_20,endchar
        .endif
  .endmacro
.else
  .macro print_wrapped_setpars x1,width,endchar
        .import _print_wrapped_x1
        .import _print_wrapped_width
        .import _print_wrapped_endchar

        .if (.not .blank(x1))
          poke _print_wrapped_x1,x1
        .endif
        .if (.not .blank(width))
          poke _print_wrapped_width,width
        .endif
        .if (.not .blank(endchar))
          poke _print_wrapped_endchar,endchar
        .endif
  .endmacro
.endif


;; print_wrapped arg
;; Write arg to the screen with word wrapping. arg can be a string, an address or AX as a pointer to a null-terminated string.
;; Left margin and width of print window are defined via print_wrapped_setpars 
;; Default parameters for the printing window are 0,40, meaning full screen
;; The printing starts at the current cursor position, to support successive print_wrapped calls.
;; Using command <i>set_cursor_pos</i> for the first print is suggested.

.macro print_wrapped arg
        .if .not ((.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax})))    ;not AX
	  .if .match ({arg}, z)	;test if the argument starts with a character, then it is a label
	    ldax #arg
	  .else
            jmp :++
            : .byte arg,0
            : lda #<:--
            ldx #>:--
	  .endif
        .endif
.if .def(__C128__)
        .import _print_wrapped_windowed_128
        jsr _print_wrapped_windowed_128
.elseif .def(__VIC20__)
        .import _print_wrapped_windowed_20
        jsr _print_wrapped_windowed_20
.else
        .import _print_wrapped_windowed
        jsr _print_wrapped_windowed
.endif
.endmacro

;; printax
;; Prints the number in AX as a 16 bit unsigned decimal number
;; BASIC and KERNAL ROM need to be enabled when using this function
;; Registers modified: A,Y,X
.macro  printax
.scope
	.if ::USE_BASIC_ROM>0
          sta setx +1     ;we need to switch A and X
          txa
setx:     ldx #00
          jsr LINPRT
	.else
	  .import _printax_sr
	  jsr _printax_sr
	.endif
.endscope
.endmacro

.macro  printax_rightaligned fillchar
.scope
	.import _print_fillchars_ax_sr
	.ifblank fillchar
	  ldy #32	;default fillchar is space
	.else
	  ldy #fillchar
	.endif
	jsr _print_fillchars_ax_sr
	printax
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

.macro  printa_rightaligned fillchar
.scope
	.import _print_fillchars_a_sr
	.ifblank fillchar
	  ldy #32	;default fillchar is space
	.else
	  ldy #fillchar
	.endif
	jsr _print_fillchars_a_sr
	printa
.endscope
.endmacro

;; printax_signed
;; Prints the number in AX as a 16 bit signed decimal number
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
.macro  printstr   arg
        .if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))
	    ;need to copy X to Y without using A
	    pha
	    txa
	    tay
	    pla
        .elseif (.xmatch ({arg}, {AY}) .or .xmatch ({arg}, {ay}))
	    ;no need to do anything
        .else
            lda #<(arg)
            ldy #>(arg)
        .endif
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
;; When using this function in interrupt save _llzp_word1 before calling and restore those values afterwards.
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
;; When using this function in interrupt save _llzp_word1 and _llzp_word2 before calling and restore those values afterwards.
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
;; Waits until a key is pressed, no meaningful return value
;; Function works independly of IRQ routine or ROM
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
       .scope
         lama_nextlineptr .set ::BASICSTART+10
          .if .not .blank(address)
            .if (address<10)
              lama_nextlineptr .set lama_nextlineptr-3
            .elseif (address<100)
              lama_nextlineptr .set lama_nextlineptr-2
            .elseif (address<1000)
              lama_nextlineptr .set lama_nextlineptr-1
            .elseif (address>9999)
              lama_nextlineptr .set lama_nextlineptr+1
            .endif
          .endif
          .ifnblank text
            lama_nextlineptr .set lama_nextlineptr + .strlen(text)
          .endif 
          .word lama_nextlineptr
          ;
          .if .paramcount > 0
               .byte <linenumber,>linenumber
          .else
               .byte <2020,>2020
          .endif
          .byte $9e   ;SYS token
          .if .paramcount < 2
               systarget=::BASICSTART+$0c
               .byte .sprintf ("%d", systarget), $00
          .elseif .paramcount = 2
               systarget=::BASICSTART+$0c+.strlen(text)
               .byte .sprintf ("%d", systarget)
               .asciiz text
          .elseif .paramcount = 3
               .byte .sprintf ("%d", address)
               .asciiz text
          .endif
       .endscope
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
;; When using this function in interrupt save _llzp_word1 and _llzp_word2 before calling and restore those values afterwards.
;; Registers modified: A,X,Y
.macro  memcopy src_addr,target_addr,length
        .scope
.import _memcopy_sr
.importzp _llzp_word1,_llzp_word2
        .if .paramcount = 3
          yinit=<($100-<(length))
          pokew _llzp_word2,src_addr - yinit
          pokew _llzp_word1,target_addr - yinit
  
          ldx #1+>(length-1)
          ldy #yinit
	.elseif .paramcount = 1
          .if (.xmatch ({src_addr}, {AX}) .or .xmatch ({src_addr}, {ax}))          ;AX is used as length
            tay
            beq skip_adjust

	    pha
	    clc
            adc _llzp_word2
	    sta _llzp_word2
            dec _llzp_word2+1
            pla
	    clc
            adc _llzp_word1
	    sta _llzp_word1
            dec _llzp_word1+1

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
              adc _llzp_word2
              sta _llzp_word2
              dec _llzp_word2+1
              lda #<length
              clc
              adc _llzp_word1
              sta _llzp_word1
              dec _llzp_word1+1
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
.importzp _llzp_word2
        .if (.xmatch ({src_addr}, {AX}) .or .xmatch ({src_addr}, {ax}))          ;AX is used as address
          sta _llzp_word2
          stx _llzp_word2+1
        .else
          lda #<src_addr
          sta _llzp_word2
          lda #>src_addr
          stx _llzp_word2+1
        .endif
        .endscope
.endmacro

.macro memcopy_to target_addr
        .scope
.importzp _llzp_word1
        .if (.xmatch ({target_addr}, {AX}) .or .xmatch ({target_addr}}))         ;AX is used as address
          sta _llzp_word1
          stx _llzp_word1+1
        .else
          lda #<target_addr
          sta _llzp_word1
          lda #>target_addr
          stx _llzp_word1+1
        .endif
        .endscope
.endmacro

;; memset start_addr,end_addr,fillvalue
;; Fills the memory area between start_addr and end_adder with the byte fillvalue
;; When using this function in interrupt save _llzp_word1 and _llzp_word2 before calling and restore those values afterwards.
;; Registers modified: A,X,Y
.macro  memset start_addr,end_addr,fillvalue
.import _memset_sr
.importzp _llzp_word1
        .scope
        length = end_addr - (start_addr) + 1
        yinit=<($100-<(length))
        pokew _llzp_word1,start_addr - yinit

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


;; sqrt16 [arg]
;; sqrt16 #arg
;; Calculates the squareroot of the argument or, if no argument is given, of AX
;; Implemented as a subroutinge, link with -lib lamalib.lib
;; When using this function in interrupt save _llzp_word1 before calling and restore those values afterwards.
;; Result is returned in A, X will contain the remainder
;; Registers modified: A,X,Y
.importzp _llzp_word1
.import _sqrt16_sr
.macro sqrt16 arg
	.if .paramcount = 1
          .if .not (.xmatch ({arg1}, {AX}) .or .xmatch ({arg1}, {ax}))    ;print AX as number
            ldax arg	;address or immediate number
          .endif
        .endif
	stax _llzp_word1
	jsr _sqrt16_sr
.endmacro


;; set_raster_irq rasterline[, isr]
;; Changes the IRQ source to VIC raster interrupt
;; Turns off the CIA IRQ source
;; If isr is given, the address is set in Kernals IRQ vector $314/315
;; IRQ routine must acknowledge the IRQ source
;; Interrupt flag will be turned off while executing the macro and reset to the previous state afterwards
.macro set_raster_irq rasterline, isr
	.ifblank rasterline
	  rline_value .set 0
	.else
	  rline_value .set rasterline
	.endif

	php		; store current state of interrupt flag
	sei

	.ifnblank isr
	  lda #<isr
	  sta $314
	  lda #>isr
	  sta $315
	.endif

        lda #$7f
        sta $dc0d       ; disable timer interrupts
        sta $dd0d

        and $d011       ; accu is still $7f
        .if rline_value>255
        ora #$80
        .endif
        sta $d011       ; Clear most significant bit in VIC's raster register

        lda $dc0d       ; acknowledge CIA interrupts

        lda #<rline_value
        sta $d012       ; Set the raster line number where interrupt should occur 

        lda #01
        sta $d01A       ; set raster interrupt

	plp
.endmacro

;; set_irq_rasterline rasterline|A|AX|X|Y
;; Sets the raster line where the IRQ should occur. This macro does not set the IRQ source, use set_raster_irq for this. 
;; Used registers: A
.macro set_irq_rasterline rasterline
	.if (.xmatch ({rasterline}, {AX}) .or .xmatch ({arg1}, {ax}))          ;AX is used as argument
	  sta $d012
	  lda $d011
	  and #$7f
	  cpx #1
	  if eq
	    ora #$80
	  endif
	  sta $d011
	.elseif (.xmatch ({rasterline}, {A}) .or .xmatch ({arg1}, {a}))          ;A is used as argument
	  sta $d012
	  lda $d011
	  and #$7f
	  sta $d011	
	.elseif (.xmatch ({rasterline}, {X}) .or .xmatch ({arg1}, {x}))          ;X is used as argument
	  stx $d012
	  lda $d011
	  and #$7f
	  sta $d011	
	.elseif (.xmatch ({rasterline}, {Y}) .or .xmatch ({arg1}, {y}))          ;Y is used as argument
	  sty $d012
	  lda $d011
	  and #$7f
	  sta $d011	
	.else          							  ;argument is a number
          poke $d012,<rasterline
          lda $d011
          .if rasterline<256
            and #$7f
          .else
            ora #$80
          .endif
          sta $d011
        .endif
.endmacro

;; set_isr_for_stabilize_raster_cycle 
;; Puts the isr address of the IRQ wedge routine of stabilize_raster_cycle into $FFFE/$FFFF
;; When using the raster stabilization from a Kernal IRQ ($314) this macro should be used once during setup
;; When using the raster stabilization with banked out Kernal, this macro should be always before the stabilize macro (or stabilize_raster_cycle_with_isr_set is used, which puts both macros together).
;; When you have multiple instances of the stabilization routine, the macros refering to each other should be put into a named scope under the same name.
;; Used registers: A
.macro set_isr_for_stabilize_raster_cycle
        ; label is set in macro stabilize_raster_cycle
        lda #<_stabilize_raster_cycle_isr
        sta $fffe
        lda #>_stabilize_raster_cycle_isr
        sta $ffff
.endmacro

;; stabilize_raster_cycle_with_isr_set
;; Combination of set_irq_for_stabilize_raster_cycle and stabilize_raster_cycle
;; This macro is useful when you have your interrupt service routine called via $FFFE/$FFFF
;; The macros are encapsuled into a common scope, so that this macro can be used multiple times.
;; Used registers: A,X
;; Minimal example showing a typical usage:
;;<tt>init:&nbsp;&nbsp;&nbsp;sei
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;set_raster_irq 48
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;pokew $fffe,isr
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;poke 1,$35
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;cli
;;:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;jmp :-
;;isr:&nbsp;&nbsp;&nbsp;&nbsp;pha
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;txa
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;pha
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;tya
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;pha
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;stabilize_raster_cycle_with_isr_set
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;; we are perfectly synchronized, do stuff
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;set_irq_rasterline 48
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;asl $d019
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;pokew $fffe,isr
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;pla
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;tay
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;pla
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;tax
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;pla
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;rti</tt>
.macro stabilize_raster_cycle_with_isr_set
.scope
        set_isr_for_stabilize_raster_cycle
        stabilize_raster_cycle
.endscope
.endmacro

;; stabilize_raster_cycle [extranops]
;; Syncs to raster cycle 3 at current raster line+2 (cycle perfect)
;; The current and the next two rasterlines must not be badlines
;; Uses the double IRQ method with a NOP slide and a conditional jump to eliminate jitter
;; Assembling may fail if the conditional branch goes of a page boundary, enable the option extranops in this case. 
;; Used registers: A,X
;; Minimal example showing a typical usage:
;;<tt>init:&nbsp;&nbsp;&nbsp;set_isr_for_stabilize_raster_cycle
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;set_raster_irq 48,isr
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;rts
;;isr:&nbsp;&nbsp;&nbsp;&nbsp;poke 1,$35      
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;stabilize_raster_cycle
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;; we are perfectly synchronized, do stuff
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;poke 1,$37
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;set_irq_rasterline 48
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;asl $d019
;;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;jmp $ea31</tt>
.macro stabilize_raster_cycle extranops
        ; **************************************************
        ; Raster Stabilizing Code
        ; based on: https://codebase64.org/doku.php?id=base:stable_raster_routine
	; by TWW / Creators
        ; **************************************************
        ; A Raster Compare IRQ is triggered on cycle 0 on the current $d012 line
        ; The CPU needs to finish its current OP code before starting the Interrupt Handler,
        ; meaning a 0 -> 7 cycles delay depending on OP code.
        ; Then a 7 cycle delay is spent invoking the Interrupt Handler (Push SR/PC to stack++)
        ; Then 13 cycles for storing registers (pha, txa, pha, tya, pha)
        ; CYCLECOUNT: [20 -> 27] cycles after Raster IRQ occurred.
        ; Acknowledge current Raster IRQ
        asl $d019
        ; Store current Stack Pointer (will be messed up when the next IRQ occurs)
        tsx
        ; Set the Raster IRQ to trigger on the next Raster line
	inc $d012
        ; Allow IRQ to happen (Remember, the Interrupt flag is set by the Interrupt Handler).
        cli
        ; Execute nops until the raster line changes and the Raster IRQ triggers
:       nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
        nop
	jmp :-	;if this jmp is reached it can spoil the timing, but avoid a crash
	.if .xmatch ({extranops}, {extranops})
	  ; add two extra nops, to be used in case of the beq goes over a page limit
	  nop
	  nop
	.endif
        ; Add one extra nop for 65 cycles NTSC machines
        ; CYCLECOUNT: [64 -> 71]
_stabilize_raster_cycle_isr:
        ; At this point, the next Raster Compare IRQ has triggered and the jitter is max 1 cycle.
        ; CYCLECOUNT: [7 -> 8] (7 cycles for the interrupt handler + [0 -> 1] cycle Jitter for the nop)
        ; Restore previous Stack Pointer (undo the last stack manipulation by the IRQ)
        txs
	delay_cycles 44	;44 for PAL, 45 for NTSC-64, 46 for NTSC-65  
        ; Check if $d012 is incremented and rectify with an additional cycle if necessary
        lda $d012
        cmp $d012  ; <- critical instruction (ZERO-Flag will indicate if Jitter = 0 or 1)
        ; CYCLECOUNT: [61 -> 62] <- Will not work if this timing is wrong
        ; cmp $d012 is originally a 5 cycle instruction but due to pipelining tech. the
        ; 5th cycle responsible for calculating the result is executed simultaneously
        ; with the next OP fetch cycle (first cycle of beq *+2).
        ; Add one cycle if $d012 wasn't incremented (Jitter / ZERO-Flag = 0)
	.assert >* = >(*+2), error, "Time critical branch goes over page limit, please use *stabilize_raster_cycle extranops* in your code"
        beq *+2 ;remove remaining jitter of 1 cycle
        ; we are now stable
.endmacro


;; set_timerA_latch value|AX[,CIA base address]
;; Sets the timer A value of the Complex Interface Adapter (CIA)
;; If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be passed as second argument.
;; Note that the value specifies the reload value of the timer, not the current counter value
;; Used registers: A
.macro set_timerA_latch arg1, base_addr
	.scope
	.ifblank base_addr
	  base_addr_value .set $DC00
	.else
	  base_addr_value .set base_addr
	.endif
	.if (.xmatch ({arg1}, {AX}) .or .xmatch ({arg1}, {ax}))          ;AX is used as argument
	  sta base_addr_value+4
	  stx base_addr_value+5	
	.else
	  lda #<arg1
	  sta base_addr_value+4
	  lda #>arg1
	  sta base_addr_value+5
	.endif
	.endscope
.endmacro

;; set_timerA value|AX[,CIA base address]
;; Sets the timer A latch value of the Complex Interface Adapter (CIA) and makes it load this value
;; If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be passed as second argument.
;; Used registers: A
.macro set_timerA arg1, base_addr
	set_timerA_latch arg1, base_addr
	force_load_timerA base_addr
.endmacro

;; force_load_timerA [CIA base address]
;; Makes timer A of the Complex Interface Adapter (CIA) load the latch value
;; If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be passed as second argument.
;; Used registers: A
.macro force_load_timerA base_addr
	.scope
	.ifblank base_addr
	  base_addr_value .set $DC00
	.else
	  base_addr_value .set base_addr
	.endif
	lda base_addr_value+$e		;force load timer A
	ora #$10
	sta base_addr_value+$e
	.endscope
.endmacro

;; stop_timerA [CIA base address]
;; Stops timer A of the Complex Interface Adapter (CIA)
;; If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be passed as second argument.
;; Used registers: A
.macro stop_timerA base_addr
	.scope
	.ifblank base_addr
	  base_addr_value .set $DC00
	.else
	  base_addr_value .set base_addr
	.endif
	lda base_addr_value+$e		;force load timer A
	and #$fe
	sta base_addr_value+$e
	.endscope
.endmacro

;; start_timerA [CIA base address]
;; Starts timer A of the Complex Interface Adapter (CIA)
;; If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be passed as second argument.
;; Used registers: A
.macro start_timerA base_addr
	.scope
	.ifblank base_addr
	  base_addr_value .set $DC00
	.else
	  base_addr_value .set base_addr
	.endif
	lda base_addr_value+$e		;force load timer A
	ora #$01
	sta base_addr_value+$e
	.endscope
.endmacro

;; read_timerA [AX[,cia base address]]
;; Reads the current timer value and puts it into AX
;; If the high byte changes during the reading, the reading is done again
;; Used registers: A,X
.macro read_timerA regs, base_addr
	.scope
	.ifblank base_addr
	  base_addr_value .set $DC00
	.else
	  base_addr_value .set base_addr
	.endif
	.if .blank(regs) .or (.xmatch ({regs}, {AX}) .or .xmatch ({regs}, {ax}))          ;AX is used as argument
:	  ldx base_addr_value+5
	  lda base_addr_value+4
	  cpx base_addr_value+5
	  bne :-
	.else
	  .error "Wrong argument for read_timerA"
	.endif
	.endscope
.endmacro


;; set_timerB_latch value|AX[,cia base address]
;; Sets the timer B value of the Complex Interface Adapter (CIA)
;; If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be specified.
;; Note that the value specifies the reload value of the timer, not the current counter value
;; Used registers: A
.macro set_timerB_latch arg1, base_addr
	.scope
	.ifblank base_addr
	  base_addr_value .set $DC00
	.else
	  base_addr_value .set base_addr
	.endif
	.if (.xmatch ({arg1}, {AX}) .or .xmatch ({arg1}, {ax}))          ;AX is used as argument
	  sta base_addr_value+6
	  stx base_addr_value+7	
	.else
	  lda #<arg1
	  sta base_addr_value+6
	  lda #>arg1
	  sta base_addr_value+7
	.endif
	.endscope
.endmacro

;; sync_to_rasterline256
;; A sturdy version of waiting for the rasterline transition from 255 to 256. 
;; Does busy waiting and comes with some jitter, so setting an IRQ might be the better option.
;; Might be delayed if an IRQ occurs, but will still trigger.
;; see also wait_for_rasterline
;; Registers modified: A
.macro sync_to_rasterline256 
.scope
        ;wait for a rasterline under 255
        waitsync1:
          lda $d011
          bmi waitsync1
        ;wait for a rasterline over 255
        waitsync2:
          lda $d011
          bpl waitsync2
.endscope
.endmacro

;; to_scrcode
;; If the PETSCII value in A belongs to a printable character, it is converted to the corresponding screencode and returned in A
;; Control codes will lead to an arbitrary byte.
;; Used registers: A
.macro to_scrcode
	.import _to_scrcode_sr
	jsr _to_scrcode_sr
.endmacro


;; wait_for_rasterline rasterline[,reg]
;; Macro inserting code doing busy waiting until the given rasterline is reached
;; rasterline can be a value between 0 and 311 (for PAL systems) or 261 for NTSC systems, respectively
;; The routine does not turn off the IRQ, so an IRQ might make it miss the rasterline it is waiting for.
;; see also sync_to_rasterline256
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

;; screen_on
;;turns the screen back on. Since it is necessary to write to $d011, the high bit of the next raster IRQ will be reset to 0 as a side effect

.macro screen_on
.import _screen_on
	jsr _screen_on
.endmacro
