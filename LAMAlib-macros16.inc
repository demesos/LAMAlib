;; <h2>16-bit Emulation and Other Short Macros</h2>

;; ldax addr
;; ldax #arg
;; Loads a 16-bit value into A/X, either from an address or as immediate value
;; Note that the zero flag is not indicating 0 but indicating a value <256
;; Supports zero page addressing mode
;; Registers modified: A,X
.macro ldax arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
	    iarg=(.right (.tcount ({arg})-1, {arg}))
            .if .const(iarg) .and (<iarg = >iarg)
               lda     #<iarg
               tax
            .else
               lda     #<iarg
               ldx     #>iarg
            .endif
        .else
            ; absolute or zero page
            lda arg
            ldx     1+(arg)
        .endif
.endscope
.endmacro

;; stax addr
;; Stores the value in registers A and X to addr and addr+1
;; Supports zero page addressing mode
;; Registers modified: none
.macro stax addr
.scope
        sta addr
        stx 1+(addr)
.endscope
.endmacro

;; adcax addr
;; adcax #arg
;; Adds a 16 bit value with carry at an addr or as immediate value to 16 bit value in registers A/X
;; Supports zero page addressing mode
;; Result is returned in A/X
;; Registers modified: A,X
.macro adcax arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            adc     #<(.right (.tcount ({arg})-1, {arg}))
	    pha
	    txa
            adc     #>(.right (.tcount ({arg})-1, {arg}))
            tax
	    pla
        .else
            ; absolute or zero page
            adc arg
	    pha
	    txa
            adc     1+(arg)
            tax
	    pla
        .endif
.endscope
.endmacro

;; addax addr
;; adcax #arg
;; Adds a 16 bit value without carry at an addr or as immediate value to 16 bit value in registers A/X
;; Supports zero page addressing mode
;; Result is returned in A/X
;; Registers modified: A,X
.macro addax arg
	clc
	adcax arg
.endmacro

;; sbcax addr
;; sbcax #arg
;; Subtracts the 16 bit value with carry at an addr or as immediate value from  16 bit value in registers A/X
;; Supports zero page addressing mode
;; Result is returned in A/X
;; Registers modified: A,X
.macro sbcax arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
            sbc     #<(.right (.tcount ({arg})-1, {arg}))
	    pha
	    txa
            sbc     #>(.right (.tcount ({arg})-1, {arg}))
            tax
	    pla
        .else
            ; absolute or zero page
            sbc arg
	    pha
	    txa
            sbc     1+(arg)
            tax
	    pla
        .endif
.endscope
.endmacro

;; subax addr
;; sbcax #arg
;; Subtracts the 16 bit value with carry at an addr or as immediate value from  16 bit value in registers A/X
;; Supports zero page addressing mode
;; Result is returned in A/X
;; Registers modified: A,X
.macro subax arg
	sec
	sbcax arg
.endmacro

;; rsc arg
;; rsc #arg
;; Reverse subtraction with carry, calculate the value of arg - A - C
;; if the carry is clear before the command, this behaves like rsb
;; If there is an underflow (arg is larger than A), the carry bit will be set, otherwise carry is clear
.macro rsc arg
.scope
	sbc arg
	eor #$ff
.endscope
.endmacro

;; rsb arg
;; rsb #arg
;; Reverse subtraction, calculate the value of arg - A 
;; The result is not influenced by the carry
;; If there is an underflow (arg is larger than A), the carry bit will be set, otherwise carry is clear
.macro rsb arg
.scope
	clc
	sbc arg
	eor #$ff
.endscope
.endmacro

;; rscax arg
;; rscax #arg
;; 16 bit reverse subtraction, calculate the value of arg - AX - C
;; if the carry is clear before the command, this behaves like rsbax
;; If there is an underflow (arg is larger than AX), the carry bit will be set, otherwise carry is clear
.macro rscax arg
.scope
        .if (.match (.left (1, {arg}), #))
           ; immediate mode
           sbc #<(.right (.tcount ({arg})-1, {arg}))
           eor #$ff
           pha
           txa
           sbc #>(.right (.tcount ({arg})-1, {arg}))
           eor #$ff
	   tax
	   pla
	.else 
           ; absolute or zero page
           sbc arg
           eor #$ff
           pha
           txa
           sbc 1+(arg)
           eor #$ff
	   tax
	   pla	   
	.endif
.endscope
.endmacro

;; rsbax arg
;; rsbax #arg
;; 16 bit reverse subtraction, calculate the value of arg - AX 
;; The result is not influenced by the carry
;; If there is an underflow (arg is larger than AX), the carry bit will be set, otherwise carry is clear
.macro rsbax arg
.scope
        .if (.match (.left (1, {arg}), #))
           ; immediate mode
           clc
           sbc #<(.right (.tcount ({arg})-1, {arg}))
           eor #$ff
           pha
           txa
           sbc #>(.right (.tcount ({arg})-1, {arg}))
           eor #$ff
	   tax
	   pla
	.else 
           ; absolute or zero page
           clc
           sbc arg
           eor #$ff
           pha
           txa
           sbc 1+(arg)
           eor #$ff
	   tax
	   pla	   
	.endif
.endscope
.endmacro

;; A_between lower,higher
;; Checks if the value in A is between the lower and higher value (including the values themselves)
;; Arguments are constant immediate values between 0 and 255, A is changed in the progress
;; If lower <= A <= higher, the carry bit is cleared, otherwise carry is set
;; Registers modified: A
.macro A_between lower,higher
.scope
	.if lower>0
	  sec
	  sbc #lower
	.endif
	cmp #(higher - lower + 1)
.endscope
.endmacro

;; AX_between lower,higher
;; Checks if the value in A is between the lower and higher value (including the values themselves)
;; Arguments are constant immediate values between 0 and 6535, AX is changed in the progress
;; If lower <= AX <= higher, the carry bit is cleared, otherwise carry is set
;; Registers modified: A,X
.macro AX_between lower,higher
.scope
	.if lower>0
	  subax #lower
	.endif
	cmpax #(higher - lower + 1)
.endscope
.endmacro

;; cmpax addr
;; cmpax #arg
;; Compares the value in A/X with the 16 bit value in addr or the immediate value
;; Supports zero page addressing mode
;; Result is returned in A/X
;; Registers modified: none
.macro cmpax arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
	    cpx     #>(.right (.tcount ({arg})-1, {arg}))
	    bne end
            cmp     #<(.right (.tcount ({arg})-1, {arg}))        
        .else
            ; absolute or zero page
            cpx     1+(arg)
	    bne end
            cmp arg
        .endif
end:
.endscope
.endmacro

;; orax addr
;; orax #arg
;; Calculates the bitwise OR operation between A/X and a 16 bit value at an addr or as immediate value
;; Supports zero page addressing mode
;; Result is returned in A/X
;; Registers modified: A,X
.macro orax arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
	    iarg=(.right (.tcount ({arg})-1, {arg}))
            .if (<iarg > 0)
                ora     #<iarg
            .endif
            .if (>iarg > 0)
                pha
                txa
                ora     #>iarg
                tax
                pla
            .endif
        .else
            ; absolute or zero page
            ora arg
	    pha
	    txa
            ora     (arg)+1
            tax
	    pla
        .endif
.endscope
.endmacro

;; andax addr
;; andax #arg
;; Calculates the bitwise AND operation between A/X and a 16 bit value at an addr or as immediate value
;; Supports zero page addressing mode
;; Result is returned in A/X
;; Registers modified: A,X
.macro andax arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
	    iarg=(.right (.tcount ({arg})-1, {arg}))
            .if (<iarg < 255)
                and     #<iarg
            .endif
            .if (>iarg < 255)
                pha
                txa
                and     #>iarg
                tax
                pla
            .endif
        .else
            ; absolute or zero page
            and arg
	    pha
	    txa
            and     (arg)+1
            tax
	    pla
        .endif
.endscope
.endmacro

;; eorax addr
;; eorax #arg
;; Calculates the bitwise exclusive-or operation between A/X and a 16 bit value at addr or as immediate value
;; Supports zero page addressing mode
;; Result is returned in A/X
;; Registers modified: A,X
.macro eorax arg
.scope
        .if (.match (.left (1, {arg}), #))
            ; immediate mode
	    iarg=(.right (.tcount ({arg})-1, {arg}))
            .if (<iarg > 0)
                eor     #<iarg
            .endif
            .if (>iarg > 0)
                pha
                txa
                eor     #>iarg
                tax
                pla
            .endif
        .else
            ; absolute or zero page
            eor arg
	    pha
	    txa
            eor     (arg)+1
            tax
	    pla
        .endif
.endscope
.endmacro

;; aslax
;; Performs an arithmetic shift left of A/X (essentially a multiplication with 2, MSB goes into carry)
;; Result is returned in A/X
;; Registers modified: A,X
.macro aslax
.scope
	asl
	pha
	txa
	rol
	tax
	pla
.endscope
.endmacro

;; asl16 addr
;; Performs an arithmetic shift left of a 16 bit number at addr
;; Result at addr, addr+1
;; Registers modified: none
.macro asl16 addr
.scope
	asl addr
	rol addr+1
.endscope
.endmacro

;; lsrax
;; Performs a logic shift right of A/X (essentially a division by 2, LSB goes into carry)
;; Result is returned in A/X
;; Registers modified: A,X
.macro lsrax
.scope
	pha	
	txa
	lsr
	tax
	pla
	ror
.endscope
.endmacro

;; lsr16 addr
;; Performs a logic shift right of a 16 bit number at addr
;; Result at addr, addr+1
;; Registers modified: none
.macro lsr16 addr
.scope	
	lsr addr+1
	ror addr
.endscope
.endmacro

;; rolax
;; Performs a rotate left of A/X (essentially a multiplication with 2, carry goes into LSB, MSB goes into carry)
;; Result is returned in A/X
;; Registers modified: A,X
.macro rolax
.scope
	rol
	pha
	txa
	rol
	tax
	pla
.endscope
.endmacro

;; rol16 addr
;; Performs a rotate left of a 16 bit number at addr
;; Result at addr, addr+1
;; Registers modified: none
.macro  rol16 addr
.scope
	rol addr
	rol addr+1
.endscope
.endmacro

;; rorax
;; Performs a rotate right of A/X (essentially a division by 2, carry goes into MSB, LSB goes into carry)
;; Result is returned in A/X
;; Registers modified: A,X
.macro  rorax
.scope
	; variant 1: 15 cyc, 6 byte
	pha	
	txa
	ror
	tax
	pla
	ror

;	; variant 2: 14 cyc, 9 byte, self-modifying, not reentrant
;	stx sm+1
;	ror sm+1
;	ror
;sm	ldx #00

.endscope
.endmacro

;; ror16 addr
;; Performs a rotate right of a 16 bit number at addr
;; Result at addr, addr+1
;; Registers modified: none
.macro  ror16 addr
.scope
	ror addr+1
	ror addr
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

;; negax
;; Negates the value in A/X
;; Result is returned in A/X
;; Registers modified: A,X
.macro  negax
.scope
	clc
	eor #$ff
	adc #$01
	pha
	txa
	eor #$ff
	adc #$00
	tax
	pla
.endscope
.endmacro

;; abs
;; Converts the signed 8-bit value in A to an absolute value
;; Result is returned in A
;; Registers modified: A
.macro  abs
.scope
	cmp #$80
	bcc end
	eor #$ff
	adc #0	;the carry is know to be set because of the cmp / bcc instruction
end:
.endscope
.endmacro

;; absax
;; Converts the signed 16-bit value in A/X to an absolute value
;; Result is returned in A/X
;; Registers modified: A,X
.macro  absax
.scope
	cpx #$80
	bne end
	negax
end:
.endscope
.endmacro

;; incax
;; Increments 16 bit value in registers A/X
;; Registers modified: A,X
.macro  incax
.scope
        clc
        adc #01
	bne end
        inx
end:
.endscope
.endmacro


;; inc8 addr[,increment]
;; Increments the byte stored at addr by increment
;; addr or increment can also be specified by AX
;; if addr is specified by AX, the increment can also be specified by Y
;; if increment is not specified, an increment of 1 is used, with a given addr this is identical to an INC command
;; Registers modified: none in increment = 1, A otherwise
.macro  inc8 addr,increment
        .if .paramcount = 1
          .if (.xmatch ({addr}, {AX}) .or .xmatch ({addr}, {ax}))          ;AX is used as address
            sta :+ + 1
            stx :+ + 2
	    :
            inc $affe
          .else ;we assume that the address is a number or a label that evaluates to a number
            inc addr
          .endif
        .else	;.paramcount > 1
          .if (.xmatch ({addr}, {AX}) .or .xmatch ({addr}, {ax}))          ;AX is used as address
            sta :+ + 1
            sta :+ + 4  
            stx :+ + 2
            stx :+ + 5
            .if (.xmatch ({increment}, {Y}) .or .xmatch ({increment}, {y}))            ;Y is used as increment
              tya
	    .else
	      lda #increment
            .endif
            clc
	    :
            adc $affe
            sta $affe
          .else ;we assume that the address is a number or a label that evaluates to a number
            .if (.xmatch ({increment}, {A}) .or .xmatch ({increment}, {a}))            ;A is used as increment
              clc
              adc addr
              sta addr
	    .elseif (.xmatch ({increment}, {X}) .or .xmatch ({increment}, {x}))            ;X is used as increment
              txa
              clc
              adc addr
              sta addr
	    .elseif (.xmatch ({increment}, {Y}) .or .xmatch ({increment}, {y}))            ;Y is used as increment	
              tya
              clc
              adc addr
              sta addr
            .else
              lda addr
              clc
              adc #increment
              sta addr
	    .endif
          .endif
        .endif
.endmacro

;; inc16 addr[,increment]
;; Increments the value stored at addr (lo-byte) and addr+1 (hi-byte) as a 16 bit value
;; addr can also be specified by AX
;; increment can also be specified by Y and, if AX is not used as addr, by AX,A, or X
;; if increment is not specified, an increment of 1 is used
;; Registers modified: none in increment = 1, AX otherwise
.macro  inc16 addr,increment
        .if .paramcount = 1
          .if (.xmatch ({addr}, {AX}) .or .xmatch ({addr}, {ax}))          ;AX is used as address
            sta :+ + 1
            stx :+ + 2
            ldx #0
    	    :
            inc $affe,x
            bne :+	;done
            inx
            cpx #2
            bne :-	;inc again
            :
          .else ;we assume that the address is a number or a label that evaluates to a number
            inc addr
            bne :+
            inc addr+1
	    :
          .endif
        .else	;.paramcount > 1
          .if (.xmatch ({addr}, {AX}) .or .xmatch ({addr}, {ax}))          ;AX is used as address
            sta :+ + 2
            sta :+ + 7  
            stx :+ + 3
            stx :+ + 8
            .if (.xmatch ({increment}, {Y}) .or .xmatch ({increment}, {y}))            ;Y is used as increment
              sty :+ +5
              ldx #$ff
              clc
              :
              inx
              lda $affe,x
              adc #$af
              sta $affe,x    
              bcc :+
              stx :- +5 ;x is 0
              txa         ;test x for 0
              beq :-
              :
            .else
	      lda #<increment
	      sta :+ +5
              ldx #$ff
  	      clc
  	      :
  	      inx
              lda $affe,x
  	      adc #$af
              sta $affe,x	   
  	      .if .const(increment) .and increment<256
  	        bcc :+
  	      .endif
	      lda #>increment
	      sta :- +5
              txa         ;test x for 0
              beq :-
	      .if .const(increment) .and increment<256	;only set the label if used to avoid warning
	        :
	      .endif
            .endif
          .else ;we assume that the address is a number or a label that evaluates to a number
            .if (.xmatch ({increment}, {A}) .or .xmatch ({increment}, {a}))            ;A is used as increment
              clc
              adc addr
              sta addr
              bcc :+
              inc addr+1
              :
	    .elseif (.xmatch ({increment}, {X}) .or .xmatch ({increment}, {x}))            ;X is used as increment
              txa
              clc
              adc addr
              sta addr
              bcc :+
              inc addr+1
              :
	    .elseif (.xmatch ({increment}, {Y}) .or .xmatch ({increment}, {y}))            ;Y is used as increment
              tya
              clc
              adc addr
              sta addr
              bcc :+
              inc addr+1
              :
            .elseif (.xmatch ({increment}, {AX}) .or .xmatch ({increment}, {ax}))          ;AX is used as increment
              clc
              adc addr
              sta addr
              txa
              adc addr+1
              sta addr+1        
            .elseif .const(increment) .and increment<256
              lda addr
              clc
              adc #increment
              sta addr
              bcc :+
              inc addr+1
              :
            .else
              lda addr
              clc
              adc #<increment
              sta addr
              lda addr+1
              adc #>increment
              sta addr+1
            .endif
          .endif
        .endif
.endmacro


;; decax
;; Decrements 16 bit value in registers A/X
;; Registers modified: A,X
.macro  decax
.scope
                cmp #00 ;test A, sets the carry
		bne declo
		dex
declo:		sbc #01
.endscope
.endmacro


;; dec8 addr[,decrement]
;; Decrements the byte stored at addr by decrement
;; addr or decrement can also be specified by AX
;; if addr is specified by AX, the decrement can also be specified by Y
;; if decrement is not specified, an decrement of 1 is used, with a given addr this is identical to an INC command
;; Registers modified: none in decrement = 1, A otherwise
.macro  dec8 addr,decrement
        .if .paramcount = 1
          .if (.xmatch ({addr}, {AX}) .or .xmatch ({addr}, {ax}))          ;AX is used as address
            sta :+ + 1
            stx :+ + 2
	    :
            dec $affe
          .else ;we assume that the address is a number or a label that evaluates to a number
            dec addr
          .endif
        .else	;.paramcount > 1
          .if (.xmatch ({addr}, {AX}) .or .xmatch ({addr}, {ax}))          ;AX is used as address
            sta :+ + 1
            sta :++ + 1
            stx :+ + 2
            stx :++ + 2
            .if (.xmatch ({decrement}, {Y}) .or .xmatch ({decrement}, {y}))            ;Y is used as decrement
              tya
              clc
              :
              sbc $affe
	      eor #$ff	;reverse subtraction
	    .else
	      :
	      lda $affe
	      sec
	      sbc #decrement
            .endif
	    :
	    sta $affe
          .else ;we assume that the address is a number or a label that evaluates to a number
            .if (.xmatch ({decrement}, {A}) .or .xmatch ({decrement}, {a}))            ;A is used as decrement
              clc               ;begin reverse subtraction
              sbc addr
	      eor #$ff		;end reverse subtraction
              sta addr
	    .elseif (.xmatch ({decrement}, {X}) .or .xmatch ({decrement}, {x}))            ;X is used as decrement
              txa
              clc               ;begin reverse subtraction
              sbc addr
	      eor #$ff		;end reverse subtraction
              sta addr
	    .elseif (.xmatch ({decrement}, {Y}) .or .xmatch ({decrement}, {y}))            ;Y is used as decrement	
              tya
              clc               ;begin reverse subtraction
              sbc addr
	      eor #$ff		;end reverse subtraction
              sta addr
            .else
              lda addr
              sec
              sbc #decrement
	      sta addr
	    .endif
          .endif
        .endif
.endmacro


;; dec16 addr[,decrement]
;; Decrements the value stored at addr (lo-byte) and addr+1 (hi-byte) as a 16 bit value
;; addr can also be specified by AX
;; decrement can also be specified by Y and, if AX is not used as addr, by AX,A, or X
;; if decrement is not specified, an decrement of 1 is used
;; Registers modified: none with decrement = 1, AX otherwise
.macro  dec16 addr,decrement
        .if .paramcount = 1
          .if (.xmatch ({addr}, {AX}) .or .xmatch ({addr}, {ax}))          ;AX is used as address
            sta :+ + 1
            stx :+ + 2
            sta :++ + 1
            stx :++ + 2
            ldx #0
	    :
	    lda $affe
	    bne :+
             ldx #1
            :
            dec $affe,x
            dex
            bpl :-
          .else ;we assume that the address is a number or a label that evaluates to a number
            lda addr	;test addr for zero
            bne :+
            dec addr+1
            :
            dec addr
          .endif
        .else   ;.paramcount > 1
          .if (.xmatch ({addr}, {AX}) .or .xmatch ({addr}, {ax}))          ;AX is used as address
            sta :+ + 2
            sta :+ + 7  
            stx :+ + 3
            stx :+ + 8
            .if (.xmatch ({decrement}, {Y}) .or .xmatch ({decrement}, {y}))            ;Y is used as decrement
              sty :+ +5
              ldx #$ff
              sec
              :
              inx
              lda $affe,x
              sbc #$af
              sta $affe,x    
              bcs :+
              stx :- +5 ;x is 0
              txa         ;test x for 0
              beq :-
              :
            .else
              lda #<decrement
              sta :+ +5
              ldx #$ff
              sec
              :
              inx
              lda $affe,x
              sbc #$af
              sta $affe,x          
              .if .const(decrement) .and decrement<256
                bcs :+
              .endif
              lda #>decrement
              sta :- +5
              txa         ;test x for 0
              beq :-
              .if .const(decrement) .and decrement<256  ;only set the label if used to avoid warning
                :
              .endif
            .endif
          .else ;we assume that the address is a number or a label that evaluates to a number
            .if (.xmatch ({decrement}, {A}) .or .xmatch ({decrement}, {a}))            ;A is used as decrement
              clc               ;begin reverse subtraction
              sbc addr
	      eor #$ff		;end reverse subtraction
              sta addr
              bcc :+		;underflow
              dec addr+1
              :
            .elseif (.xmatch ({decrement}, {X}) .or .xmatch ({decrement}, {x}))            ;X is used as decrement
              txa
              clc               ;begin reverse subtraction
              sbc addr
	      eor #$ff		;end reverse subtraction
              sta addr
              bcc :+		;underflow
              dec addr+1
              :
            .elseif (.xmatch ({decrement}, {Y}) .or .xmatch ({decrement}, {y}))            ;Y is used as decrement
              tya
              clc               ;begin reverse subtraction
              sbc addr
	      eor #$ff		;end reverse subtraction
              sta addr
              bcc :+		;underflow
              dec addr+1
              :
            .elseif (.xmatch ({decrement}, {AX}) .or .xmatch ({decrement}, {ax}))          ;AX is used as decrement
              clc               ;begin reverse subtraction
              sbc addr
	      eor #$ff		;end reverse subtraction
              sta addr  
	      txa
              sbc addr+1	;begin reverse subtraction with carry
	      eor #$ff		;end reverse subtraction
              sta addr+1  
            .elseif .const(decrement) .and decrement<256
              lda addr
              sec
              sbc #decrement
              sta addr
              bcs :+
              dec addr+1
              :
            .else
              lda addr
              sec
              sbc #<decrement
              sta addr
              lda addr+1
              sbc #>decrement
              sta addr+1
            .endif
          .endif
        .endif
.endmacro

;; incx n
;; Increments the X register by a value of n
;; Registers modified: X
;; Flags affected: N,Z,C
.macro  incx n
.scope
	.if .const(n) .and n<8
            .repeat n
              inx
            .endrep
        .else
          sta recva+1
          clc
          txa
          adc #n
          tax
  recva: lda #00
	.endif
.endscope
.endmacro

;; decx n
;; Decrements the X register by a value of n
;; Registers modified: X
;; Flags affected: N,Z,C
.macro  decx n
.scope
	.if n<8
            .repeat n
              dex
            .endrep
        .else
.warning "not const"
          sta recva+1
          sec
          txa
          sbc #n
          tax
  recva: lda #00
	.endif

.endscope
.endmacro

;; incy n
;; Increments the Y register by a value of n
;; Registers modified: Y
;; Flags affected: N,Z,C
.macro  incy n
.scope
	.if .const(n) .and n<8
            .repeat n
              iny
            .endrep
        .else
          sta recva+1
          clc
          tya
          adc #n
          tay
  recva: lda #00
	.endif
.endscope
.endmacro

;; decy n
;; Decrements the Y register by a value of n
;; Registers modified: Y
;; Flags affected: N,Z,C
.macro  decy n
.scope
	.if .const(n) .and n<8
            .repeat n
              dey
            .endrep
        .else
          sta recva+1
          sec
          tya
          sbc #n
          tay
  recva: lda #00
	.endif

.endscope
.endmacro

;; pushax 
;; Pushes AX to the stack and preserves AX
.macro  pushax
.scope
                pha
		sta savea+1
		txa
                pha
savea:		lda #00
.endscope
.endmacro

;; pullax 
;; Pulls AX from the stack
.macro  pullax
.scope
                pla
		tax
                pla
.endscope
.endmacro

;; poke arg1,arg2 
;; Copies arg2 into the address of arg1
;; arg1 can be a constant or AX
;; arg2 can be a constant or A, X or Y
.macro  poke arg1,arg2
.scope
        .if (.xmatch ({arg1}, {AX}) .or .xmatch ({arg1}, {ax}))          ;AX is used as address
          sta targetsm+1
          stx targetsm+2
          .if (.xmatch ({arg2}, {Y}))         ;value in Y register is to be poked
            targetsm: sty $ffff
          .else                              ;value to be poked is a given constant
                      lda #arg2
            targetsm: sta $ffff
          .endif
        .else                                ;address is a given constant
          .if (.xmatch ({arg2}, {A}))         ;value in Accu is to be poked
            sta arg1
          .elseif (.xmatch ({arg2}, {X}))      ;value in X register is to be poked
            stx arg1
          .elseif (.xmatch ({arg2}, {Y}))      ;value in y register is to be poked
            sty arg1
          .else                              ;value to be poked is a given constant
            lda #arg2
            sta arg1
          .endif
        .endif
.endscope
.endmacro


;; pokew arg1,arg2
;; poke word: copies 16 bit value arg2 into the address of arg1 and arg1+1
;; arg1 will be filled with low byte of arg2
;; arg2 will be filled with high byte of arg2
;; arg1, arg2 can be both constants or one can be AX and the other a constant
;; Regs changed: A, Y (in case AX is used as address)

.macro  pokew arg1,arg2
.scope
        .if (.xmatch ({arg1}, {AX}) .or .xmatch ({arg1}, {ax}))         ;AX is used as address
.importzp _zp_ptr1
          sta _zp_ptr1
          stx _zp_ptr1+1
          ldy #00
          lda #<(arg2)
          sta (_zp_ptr1),y
          lda #>(arg2)
          iny
          sta (_zp_ptr1),y
        .else                                ;address is a given constant
          .if (.xmatch ({arg2}, {AX}) .or .xmatch ({arg2}, {ax}))       ;value to be poked in AX
            sta arg1
            stx arg1+1
          .else                              ;value to be poked is a given constant
            lda #<(arg2)
            sta arg1
            lda #>(arg2)
            sta arg1+1
          .endif
        .endif
.endscope
.endmacro

;; jsr_ind addr
;; Executes a subroutine by a vector stored in addr and returns to the command after jsr_ind upon completion with an rts
;; arg1 will be a 16 bit address containing the vector to jump to
;; Regs changed: none 

.macro  jsr_ind addr
.scope
	store A
	lda #>ret
	pha
	lda #<ret
	pha
	restore A
ret=*+2		;the jmp () has three bytes, but ret must be decreased by 1
.endscope
	jmp (addr)
.endmacro