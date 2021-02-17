; auxiliary defintions for long branches, basically a copy of macro package longbranch
; but with other names, since we don't know if the user will use the macro package
.macro  longbeq     Target
        .if     .match(Target, 0)
        bne     *+5
        jmp     Target
        .elseif .def(Target) .and .const((*-2)-(Target)) .and ((*+2)-(Target) <= 127)
                beq     Target
        .else
                bne     *+5
                jmp     Target
        .endif
.endmacro
.macro  longbne     Target
        .if     .match(Target, 0)
                beq     *+5
                jmp     Target
        .elseif .def(Target) .and .const((*-2)-(Target)) .and ((*+2)-(Target) <= 127)
                bne     Target
        .else
                beq     *+5
                jmp     Target
        .endif
.endmacro
.macro  longbmi     Target
        .if     .match(Target, 0)
                bpl     *+5
                jmp     Target
        .elseif .def(Target) .and .const((*-2)-(Target)) .and ((*+2)-(Target) <= 127)
                bmi     Target
        .else
                bpl     *+5
                jmp     Target
        .endif
.endmacro
.macro  longbpl     Target
        .if     .match(Target, 0)
                bmi     *+5
                jmp     Target
        .elseif .def(Target) .and .const((*-2)-(Target)) .and ((*+2)-(Target) <= 127)
                bpl     Target
        .else
                bmi     *+5
                jmp     Target
        .endif
.endmacro
.macro  longbcs     Target
        .if     .match(Target, 0)
                bcc     *+5
                jmp     Target
        .elseif .def(Target) .and .const((*-2)-(Target)) .and ((*+2)-(Target) <= 127)
                bcs     Target
        .else
                bcc     *+5
                jmp     Target
        .endif
.endmacro
.macro  longbcc     Target
        .if     .match(Target, 0)
                bcs     *+5
                jmp     Target
        .elseif .def(Target) .and .const((*-2)-(Target)) .and ((*+2)-(Target) <= 127)
                bcc     Target
        .else
                bcs     *+5
                jmp     Target
        .endif
.endmacro
.macro  longbvs     Target
        .if     .match(Target, 0)
                bvc     *+5
                jmp     Target
        .elseif .def(Target) .and .const((*-2)-(Target)) .and ((*+2)-(Target) <= 127)
                bvs     Target
        .else
                bvc     *+5
                jmp     Target
        .endif
.endmacro
.macro  longbvc     Target
        .if     .match(Target, 0)
                bvs     *+5
                jmp     Target
        .elseif .def(Target) .and .const((*-2)-(Target)) .and ((*+2)-(Target) <= 127)
                bvc     Target
        .else
                bvs     *+5
                jmp     Target
        .endif
.endmacro

; auxiliary functions for generating labels for nested function
.macro inc_nesting_level
  .if .defined(::nesting_level)
    ::nesting_level .set ::nesting_level*10
    .if ::nesting_level = 90
      ::nesting_level .set ::nesting_level*10
    .endif
  .else
    ::nesting_level .set 10
  .endif
.endmacro

.macro dec_nesting_level
  ::nesting_level .set ::nesting_level/10 + 1
.endmacro

;inc_nesting_levelA, inc_nesting_levelX and inc_nesting_levelY are used for store/restore
.macro inc_nesting_levelA
  .if .defined(::nesting_levelA)
    ::nesting_levelA .set ::nesting_levelA*10
  .else
    ::nesting_levelA .set 10
  .endif
.endmacro

.macro dec_nesting_levelA
  ::nesting_levelA .set ::nesting_levelA/10 + 1
.endmacro

.macro inc_nesting_levelX
  .if .defined(::nesting_levelX)
    ::nesting_levelX .set ::nesting_levelX*10
  .else
    ::nesting_levelX .set 10
  .endif
.endmacro

.macro dec_nesting_levelX
  ::nesting_levelX .set ::nesting_levelX/10 + 1
.endmacro

.macro inc_nesting_levelY
  .if .defined(::nesting_levelY)
    ::nesting_levelY .set ::nesting_levelY*10
  .else
    ::nesting_levelY .set 10
  .endif
.endmacro

.macro dec_nesting_levelY
  ::nesting_levelY .set ::nesting_levelY/10 + 1
.endmacro

;; <h2>Structured Programming</h2>
;; These commands allow you to use constructs like <i>if .. else .. endif</i>, <i>do...loop</i>, <i>for...next</i>, and <i>switch...case</i> in assembly language! The structures can even be nested. The implementation of these structures is basically as efficient as a a handcoded composure of branches, jumps as labels, while it is much easier to write and read.
;; All macros can be nested.

;; store reg
;; Generates self-modifying code for store/restore of a register
;; Registers can be A,X,Y, or AX (that is A and X)
;; An example application for <i>store</i>/<i>restore</i> is for saving a register to be able use it in between. An alternative to <i>store</i>/<i>restore</i> are stack push/pull, which is more compact, but 1 cycle slower.
;; Each store must match with exactly one <i>restore</i> macro using the same register, restore has to be placed later in your code
;; Registers modified: none
.macro store reg
  .if .xmatch ({reg}, {A}) 
    inc_nesting_levelA
    sta .ident(.sprintf("selfmod_A%d", ::nesting_levelA/10))+1
  .elseif .xmatch ({reg}, {X}) 
    inc_nesting_levelX
    stx .ident(.sprintf("selfmod_X%d", ::nesting_levelX/10))+1
  .elseif .xmatch ({reg}, {Y}) 
    inc_nesting_levelY
    sty .ident(.sprintf("selfmod_Y%d", ::nesting_levelY/10))+1
  .elseif .xmatch ({reg}, {AX}) 
    store A
    store X
  .endif
.endmacro

;; restore reg
;; Generates the code for restoreing a stored register
;; Registers can be A,X,Y, or AX (that is A and X)
;; Each restore must match with exactly one preceding <i>store</i> macro using the same register
;; Registers modified: the restoreed register

.macro restore reg
  .if .xmatch ({reg}, {A}) 
.ident(.sprintf("selfmod_A%d", ::nesting_levelA/10)):
    lda #00
    dec_nesting_levelA
  .elseif .xmatch ({reg}, {X}) 
.ident(.sprintf("selfmod_X%d", ::nesting_levelX/10)):
    ldx #00
    dec_nesting_levelX
  .elseif .xmatch ({reg}, {Y}) 
.ident(.sprintf("selfmod_Y%d", ::nesting_levelY/10)):
    ldy #00
    dec_nesting_levelY
  .elseif .xmatch ({reg}, {AX}) 
    restore A
    restore X
  .endif
.endmacro

;; if cond
;;   ...
;; [<b>else</b>]
;;   ...
;; <b>endif</b>
;; Structure for conditionals.
;; <i>cond</i> can be <i>eq</i>, <i>neq</i>, <i>minus</i>, <i>plus</i>, <i>c_clr</i>, <i>c_set</i>, <i>v_clr</i>, <i>v_set</i>, <i>lt</i>, <i>ge</i>
;; This corresponds to assembler commands BEQ, BNE, BMI, BPL, BCC, BCS, BVC, BVS
;; <i>lt</i> (less than) is equal to <i>c_clr</i> (BCC), <i>ge</i> (greater or equal) is equal to <i>c_set</i> (BCS)
;; using else is optional
;; Registers modified: none

.macro if cond
  inc_nesting_level
  .if ((.xmatch ({cond}, {eq})) || (.xmatch ({cond}, {zero})))
    bne .ident(.sprintf("else_or_end%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {neq}))
    beq .ident(.sprintf("else_or_end%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {minus}))
    bpl .ident(.sprintf("else_or_end%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {plus}))
    bmi .ident(.sprintf("else_or_end%d", ::nesting_level/10))
  .elseif ((.xmatch ({cond}, {c_clr})) || (.xmatch ({cond}, {lt})))
    bcs .ident(.sprintf("else_or_end%d", ::nesting_level/10))
  .elseif ((.xmatch ({cond}, {c_set})) || (.xmatch ({cond}, {ge})))
    bcc .ident(.sprintf("else_or_end%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {v_clr}))
    bvs .ident(.sprintf("else_or_end%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {v_set}))
    bvc .ident(.sprintf("else_or_end%d", ::nesting_level/10))
  .else
    .error  "Condition for if undefined!"
  .endif
.endmacro

; future extension: add a version iflong with long branches

.macro else
  jmp .ident(.sprintf("end_of_if%d", ::nesting_level/10))
  .ident(.sprintf("else_or_end%d", ::nesting_level/10)):
.endmacro

.macro endif
  .ifndef .ident(.sprintf("else_or_end%d", ::nesting_level/10))
    .ident(.sprintf("else_or_end%d", ::nesting_level/10)):
  .endif
  .ident(.sprintf("end_of_if%d", ::nesting_level/10)):
  dec_nesting_level
.endmacro

;; for X|Y|A|AX|addr,start,to|downto,end,step
;;   ...
;; <b>next</b>
;; For loop structure
;; Counting variable can be an 8-bit register (A,X,Y), a 16 bit register (AX) or a 16 bit memory address (in parentheses)
;; <i>start</i> can be a constant or a memory address pointing to the value
;; <i>end</i> can be a constant or a memory address pointing to the value. The loop is continued including an iteration reaching the <i>end</i> value
;; <i>start</i> can be a constant or a memory address pointing to the value
;; <b>to</b> indicates a loop that counts up, <b>downto</b> indicates a loop that counts down
;; <i>end</i> can be a constant or a memory address pointing to the value
;; <b>step</b> is optional and defines the increment/decrement (default=1). This value is alway positive, counting down is indicated with <b>downto</b>
;; 
;; When the loop variable is an 8 bit register, <i>start</i>, <i>end</i>, and <i>step</i> are a single byte (or a memory reference to a single byte)
;; When the loop variable is a 16 bit register (AX) or a 16 bit memory address, <i>start</i>, <i>end</i>, and <i>step</i> are a 16 bit value (or a memory reference to a 16 bit value)
;; If memory references can also go to zero page the zero page addressing mode is used which speeds up the code a bit
;; When <i>step</i> is larger than 1 or no constant, loops that count down can are limited to a 7 bit (for single register loops) or a 15 bit range
;; It is possible to nest multiple for loops but each <b>for</b> must be followed by exactly one corresponding <i>next</i> later in the code.
;;
;; Within a for loop, the macros <i>break</i> and <i>continue</i> can be used to exit the loop or go to next iteration.
;;
;; Registers modified: the loop register, or the accu otherwise
;;
;; Code example that outputs '9876543210':
;; <code>
;; for A,$39,downto,$30
;; &nbsp;  jsr $ffd2
;; next
;; </code>

.macro for var,start,dir,end,step
  inc_nesting_level
  .if .xmatch ({var}, {A})
    .ident(.sprintf("for_var%d", ::nesting_level/10))=-1
  .elseif .xmatch ({var}, {X})
    .ident(.sprintf("for_var%d", ::nesting_level/10))=-2
  .elseif .xmatch ({var}, {Y})
    .ident(.sprintf("for_var%d", ::nesting_level/10))=-3
  .elseif .xmatch ({var}, {AX})
    .ident(.sprintf("for_var%d", ::nesting_level/10))=-4
  .else
    .ident(.sprintf("for_var%d", ::nesting_level/10))=var
  .endif

  .if .xmatch ({dir}, {to}) || .xmatch ({dir}, {TO})
    .ident(.sprintf("for_dir%d", ::nesting_level/10))=1
  .elseif .xmatch ({dir}, {downto}) || .xmatch ({dir}, {DOWNTO})
    .ident(.sprintf("for_dir%d", ::nesting_level/10))=-1
  .else
    .error  "Syntax error in for loop, 3rd argument needs to be TO or DOWNTO!"
  .endif

  .if .paramcount = 5
     .ident(.sprintf("for_inc%d", ::nesting_level/10))=step
     .if (.match (.left (1, {step}), {(})) .and (.match (.right (1, {step}), {)}))	;argument in brackets -> means indirect addressing
       .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=1
     .else
       .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=0
     .endif
  .else
      .ident(.sprintf("for_inc%d", ::nesting_level/10)) .set 1
      .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=0
  .endif

  .if (.match (.left (1, {end}), {(})) .and (.match (.right (1, {end}), {)}))	;argument in brackets -> means indirect addressing
    .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
    .ident(.sprintf("for_end%d", ::nesting_level/10))=end
  .else
    .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=0
    .if .ident(.sprintf("for_dir%d", ::nesting_level/10))>0
      .ident(.sprintf("for_end%d", ::nesting_level/10))=end+1
    .else
      .ident(.sprintf("for_end%d", ::nesting_level/10))=end-1
    .endif
  .endif

  .if (.match (.left (1, {start}), {(})) .and (.match (.right (1, {start}), {)}))	;argument in brackets -> means indirect addressing
    .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=0
    .if .ident(.sprintf("for_var%d", ::nesting_level/10))=-2
      ldx 0+start
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-3
      ldy 0+start
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-1
      lda 0+start
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-4
      ldax 0+start
    .else
      lda 0+start
      sta .ident(.sprintf("for_var%d", ::nesting_level/10))
      lda 0+start+1
      sta .ident(.sprintf("for_var%d", ::nesting_level/10))+1
    .endif
  .else
    .if .ident(.sprintf("for_var%d", ::nesting_level/10))=-2
      ldx #start
      .if start<128
        .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=1
      .else
        .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=0
      .endif
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-3
      ldy #start
      .if start<128
        .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=1
      .else
        .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=0
      .endif
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-1
      lda #start
      .if start<128
        .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=1
      .else
        .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=0
      .endif
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-4
      ldax #start
      .if start<32768
        .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=1
      .else
        .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=0
      .endif
    .else
      .if start<32768
        .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=1
      .else
        .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=0
      .endif
      lda #<start
      sta .ident(.sprintf("for_var%d", ::nesting_level/10))
      lda #>start
      sta .ident(.sprintf("for_var%d", ::nesting_level/10))+1
    .endif
  .endif
.ident(.sprintf("begin_loop%d", ::nesting_level/10)):
.endmacro

.macro next
.ident(.sprintf("continue_point%d", ::nesting_level/10)):
  .if .ident(.sprintf("for_dir%d", ::nesting_level/10))>0   ;--- incrementing loops ---
    .if .ident(.sprintf("for_var%d", ::nesting_level/10))=-2   ;--- next X ---
      .if .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=1
        txa
        clc
        adc .ident(.sprintf("for_inc%d", ::nesting_level/10))
        tax
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cpx .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          longbeq .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          cpx #.ident(.sprintf("for_end%d", ::nesting_level/10))
          longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .endif
      .else
        .repeat .ident(.sprintf("for_inc%d", ::nesting_level/10))
        inx
        .endrep
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cpx .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          longbeq .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          .if .ident(.sprintf("for_inc%d", ::nesting_level/10))=1
            .if .not .ident(.sprintf("for_end%d", ::nesting_level/10))=0
              cpx #.ident(.sprintf("for_end%d", ::nesting_level/10))
            .endif
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .else
            cpx #.ident(.sprintf("for_end%d", ::nesting_level/10))
            longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .endif
        .endif
      .endif
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-3   ;--- next Y ---
      .if .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=1
        tya
        clc
        adc .ident(.sprintf("for_inc%d", ::nesting_level/10))
        tay
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cpy .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          longbeq .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          cpy #.ident(.sprintf("for_end%d", ::nesting_level/10))
          longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .endif
      .else
        .repeat .ident(.sprintf("for_inc%d", ::nesting_level/10))
        iny
        .endrep
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cpy .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          longbeq .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          .if .ident(.sprintf("for_inc%d", ::nesting_level/10))=1
            .if .not .ident(.sprintf("for_end%d", ::nesting_level/10))=0
              cpy #.ident(.sprintf("for_end%d", ::nesting_level/10))
            .endif
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .else
            cpy #.ident(.sprintf("for_end%d", ::nesting_level/10))
            longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .endif
        .endif
      .endif
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-1   ;--- next A ---
      .if .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=1
        clc
        adc .ident(.sprintf("for_inc%d", ::nesting_level/10))
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cmp .ident(.sprintf("for_end%d", ::nesting_level/10))
          .if ((*+4)-(.ident(.sprintf("begin_loop%d", ::nesting_level/10))) <= 127)
            bcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            beq .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .else
  	  bcs *+5
            jmp .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .endif
        .else
          cmp #.ident(.sprintf("for_end%d", ::nesting_level/10))
          longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .endif
      .else
        clc
        adc #.ident(.sprintf("for_inc%d", ::nesting_level/10))
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cmp .ident(.sprintf("for_end%d", ::nesting_level/10))
          .if ((*+4)-(.ident(.sprintf("begin_loop%d", ::nesting_level/10))) <= 127)
            bcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            beq .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .else
  	  bcs *+5
            jmp .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .endif
        .else
          .if .ident(.sprintf("for_inc%d", ::nesting_level/10))=1
            .if .not .ident(.sprintf("for_end%d", ::nesting_level/10))=0
              cmp #.ident(.sprintf("for_end%d", ::nesting_level/10))
            .endif
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .else
            cmp #.ident(.sprintf("for_end%d", ::nesting_level/10))
            longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .endif
        .endif
      .endif
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-4   ;--- next AX ---
      .if .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=1
        clc
        adcax .ident(.sprintf("for_inc%d", ::nesting_level/10))
      .else
        .if .ident(.sprintf("for_inc%d", ::nesting_level/10))=1
          incax
        .else
          clc
          adcax #.ident(.sprintf("for_inc%d", ::nesting_level/10))
        .endif
      .endif
      .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
        cpx .ident(.sprintf("for_end%d", ::nesting_level/10))+1
        longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        cmp .ident(.sprintf("for_end%d", ::nesting_level/10))
        .if ((*+4)-(.ident(.sprintf("begin_loop%d", ::nesting_level/10))) <= 127)
          bcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          beq .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
	  bcs *+5
          jmp .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .endif
      .else
        cpx #>.ident(.sprintf("for_end%d", ::nesting_level/10))
        longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        cmp #<.ident(.sprintf("for_end%d", ::nesting_level/10))     
        longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
      .endif
    .else   ;--- next (addr) ---
      .if .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=1
        clc
        lda .ident(.sprintf("for_var%d", ::nesting_level/10))
        adc .ident(.sprintf("for_inc%d", ::nesting_level/10))
        sta .ident(.sprintf("for_var%d", ::nesting_level/10))
        lda .ident(.sprintf("for_var%d", ::nesting_level/10))+1
        adc .ident(.sprintf("for_inc%d", ::nesting_level/10))+1
        sta .ident(.sprintf("for_var%d", ::nesting_level/10))+1
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cmp .ident(.sprintf("for_end%d", ::nesting_level/10))+1
          longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          bne .ident(.sprintf("end_loop%d", ::nesting_level/10))
          lda .ident(.sprintf("for_end%d", ::nesting_level/10))
          cmp .ident(.sprintf("for_var%d", ::nesting_level/10))
          longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          cmp #>.ident(.sprintf("for_end%d", ::nesting_level/10))
          longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          bne .ident(.sprintf("end_loop%d", ::nesting_level/10))
          lda .ident(.sprintf("for_var%d", ::nesting_level/10))
          cmp #<.ident(.sprintf("for_end%d", ::nesting_level/10))     
          longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .endif
      .else
        .if .ident(.sprintf("for_inc%d", ::nesting_level/10))=1
          inc16 .ident(.sprintf("for_var%d", ::nesting_level/10))
          .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
            lda .ident(.sprintf("for_var%d", ::nesting_level/10))+1
            cmp .ident(.sprintf("for_end%d", ::nesting_level/10))+1
            longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            lda .ident(.sprintf("for_end%d", ::nesting_level/10))
            cmp .ident(.sprintf("for_var%d", ::nesting_level/10))
            longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .else
            lda .ident(.sprintf("for_var%d", ::nesting_level/10))
            .if (<.ident(.sprintf("for_end%d", ::nesting_level/10)))>0
              cmp #<.ident(.sprintf("for_end%d", ::nesting_level/10))
            .endif 
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            lda .ident(.sprintf("for_var%d", ::nesting_level/10))+1
            .if (>.ident(.sprintf("for_end%d", ::nesting_level/10)))>0
              cmp #>.ident(.sprintf("for_end%d", ::nesting_level/10))
            .endif
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .endif
        .else
          clc
          lda .ident(.sprintf("for_var%d", ::nesting_level/10))
          adc #<.ident(.sprintf("for_inc%d", ::nesting_level/10))
          sta .ident(.sprintf("for_var%d", ::nesting_level/10))
          lda .ident(.sprintf("for_var%d", ::nesting_level/10))+1
          adc #>.ident(.sprintf("for_inc%d", ::nesting_level/10))
          sta .ident(.sprintf("for_var%d", ::nesting_level/10))+1
          .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
            cmp .ident(.sprintf("for_end%d", ::nesting_level/10))+1
            longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            bne .ident(.sprintf("end_loop%d", ::nesting_level/10))
            lda .ident(.sprintf("for_end%d", ::nesting_level/10))
            cmp .ident(.sprintf("for_var%d", ::nesting_level/10))
            longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .else
            cmp #>.ident(.sprintf("for_end%d", ::nesting_level/10))
            longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            bne .ident(.sprintf("end_loop%d", ::nesting_level/10))
            lda .ident(.sprintf("for_var%d", ::nesting_level/10))
            cmp #<.ident(.sprintf("for_end%d", ::nesting_level/10))     
            longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .endif
        .endif
      .endif
    .endif
  .else              ;--- decrementing loops ---
    .if .ident(.sprintf("for_var%d", ::nesting_level/10))=-2   ;--- next X ---
      .if .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=1
        txa
        sec
        sbc .ident(.sprintf("for_inc%d", ::nesting_level/10))
        tax
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cpx .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          .if .ident(.sprintf("for_end%d", ::nesting_level/10)) >= 0
            cpx #.ident(.sprintf("for_end%d", ::nesting_level/10))+1
          .endif
          longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .endif
      .else
        .repeat .ident(.sprintf("for_inc%d", ::nesting_level/10))
        dex
        .endrep
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cpx .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          .if .ident(.sprintf("for_inc%d", ::nesting_level/10))=1
            .if .ident(.sprintf("for_end%d", ::nesting_level/10)) = -1
              .if .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=1
                longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
              .else
                cpx #.ident(.sprintf("for_end%d", ::nesting_level/10))+$100
                longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
              .endif
            .else
              .if .ident(.sprintf("for_end%d", ::nesting_level/10)) > 0
                cpx #.ident(.sprintf("for_end%d", ::nesting_level/10))
              .endif
              longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .endif
          .else
            .if .ident(.sprintf("for_end%d", ::nesting_level/10))-.ident(.sprintf("for_inc%d", ::nesting_level/10)) < 0
              .if .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=1
                longbmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
              .else
                cpx #.ident(.sprintf("for_end%d", ::nesting_level/10))-.ident(.sprintf("for_inc%d", ::nesting_level/10))+$100
                longbcs .ident(.sprintf("end_loop%d", ::nesting_level/10))
              .endif
            .endif
            .if .ident(.sprintf("for_end%d", ::nesting_level/10)) < 0
              jmp .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .elseif .ident(.sprintf("for_end%d", ::nesting_level/10)) > 0
              cpx #.ident(.sprintf("for_end%d", ::nesting_level/10))+1
              longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    	    .else
              longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .endif
          .endif
        .endif
      .endif
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-3   ;--- next Y ---
      .if .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=1
        tya
        sec
        sbc .ident(.sprintf("for_inc%d", ::nesting_level/10))
        tay
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cpy .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          .if .ident(.sprintf("for_end%d", ::nesting_level/10)) >= 0
            cpy #.ident(.sprintf("for_end%d", ::nesting_level/10))+1
          .endif
          longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .endif
      .else
        .repeat .ident(.sprintf("for_inc%d", ::nesting_level/10))
        dey
        .endrep
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cpy .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          .if .ident(.sprintf("for_inc%d", ::nesting_level/10))=1
            .if .ident(.sprintf("for_end%d", ::nesting_level/10)) = -1
              .if .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=1
                longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
              .else
                cpy #.ident(.sprintf("for_end%d", ::nesting_level/10))+$100
                longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
              .endif
            .else
              .if .ident(.sprintf("for_end%d", ::nesting_level/10)) > 0
                cpy #.ident(.sprintf("for_end%d", ::nesting_level/10))
              .endif
              longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .endif
          .else
            .if .ident(.sprintf("for_end%d", ::nesting_level/10))-.ident(.sprintf("for_inc%d", ::nesting_level/10)) < 0
              .if .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=1
                bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
              .else
                cpy #.ident(.sprintf("for_end%d", ::nesting_level/10))-.ident(.sprintf("for_inc%d", ::nesting_level/10))+$100
                bcs .ident(.sprintf("end_loop%d", ::nesting_level/10))
              .endif
            .endif
            .if .ident(.sprintf("for_end%d", ::nesting_level/10)) < 0
              jmp .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .elseif .ident(.sprintf("for_end%d", ::nesting_level/10)) > 0
              cpy #.ident(.sprintf("for_end%d", ::nesting_level/10))+1
              longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    	    .else
              longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .endif
          .endif
        .endif
      .endif
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-1   ;--- next A ---
      .if .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=1
        sec
        sbc .ident(.sprintf("for_inc%d", ::nesting_level/10))
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cmp .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          .if .ident(.sprintf("for_end%d", ::nesting_level/10)) >= 0
            cmp #.ident(.sprintf("for_end%d", ::nesting_level/10))+1
          .endif
          longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .endif
      .else
        sec
        sbc #.ident(.sprintf("for_inc%d", ::nesting_level/10))
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cmp .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          .if .ident(.sprintf("for_inc%d", ::nesting_level/10))=1
            .if .ident(.sprintf("for_end%d", ::nesting_level/10)) = -1
              .if .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=1
                longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
              .else
                cmp #.ident(.sprintf("for_end%d", ::nesting_level/10))+$100
                longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
              .endif
            .else
              .if .ident(.sprintf("for_end%d", ::nesting_level/10)) > 0
                cmp #.ident(.sprintf("for_end%d", ::nesting_level/10))
              .endif
              longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .endif
          .else
            .if .ident(.sprintf("for_end%d", ::nesting_level/10))-.ident(.sprintf("for_inc%d", ::nesting_level/10)) < 0
              .if .ident(.sprintf("for_start_positive%d", ::nesting_level/10))=1
                bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
              .else
                cmp #.ident(.sprintf("for_end%d", ::nesting_level/10))-.ident(.sprintf("for_inc%d", ::nesting_level/10))+$100
                bcs .ident(.sprintf("end_loop%d", ::nesting_level/10))
              .endif
            .endif
            .if .ident(.sprintf("for_end%d", ::nesting_level/10)) < 0
              jmp .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .elseif .ident(.sprintf("for_end%d", ::nesting_level/10)) > 0
              cmp #.ident(.sprintf("for_end%d", ::nesting_level/10))+1
              longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    	    .else
              longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .endif
          .endif
        .endif
      .endif
    .elseif .ident(.sprintf("for_var%d", ::nesting_level/10))=-4   ;--- next AX ---
      .if .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=1
        sec
        sbcax .ident(.sprintf("for_inc%d", ::nesting_level/10))
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cpx .ident(.sprintf("for_end%d", ::nesting_level/10))+1
          bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
          longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          cmp .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          cpx #>(.ident(.sprintf("for_end%d", ::nesting_level/10))+1)
          bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
          longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          cmp #<(.ident(.sprintf("for_end%d", ::nesting_level/10))+1)
          longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .endif
      .else
        .if .ident(.sprintf("for_inc%d", ::nesting_level/10))=1
          decax
          .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
            cpx .ident(.sprintf("for_end%d", ::nesting_level/10))+1
            bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            cmp .ident(.sprintf("for_end%d", ::nesting_level/10))
            longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .else
            cpx #>.ident(.sprintf("for_end%d", ::nesting_level/10))
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            cmp #<.ident(.sprintf("for_end%d", ::nesting_level/10))     
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .endif
        .else
          sec
          sbcax #.ident(.sprintf("for_inc%d", ::nesting_level/10))
          .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
            cpx .ident(.sprintf("for_end%d", ::nesting_level/10))+1
            bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            cmp .ident(.sprintf("for_end%d", ::nesting_level/10))
            longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .else
            .if .ident(.sprintf("for_end%d", ::nesting_level/10))-.ident(.sprintf("for_inc%d", ::nesting_level/10))>0
              cpx #>.ident(.sprintf("for_end%d", ::nesting_level/10))
              longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
              cmp #<.ident(.sprintf("for_end%d", ::nesting_level/10))     
              longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .else
              cpx #>(.ident(.sprintf("for_end%d", ::nesting_level/10))+1)
              bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
              longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
              cmp #<(.ident(.sprintf("for_end%d", ::nesting_level/10))+1)
              longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .endif
          .endif
        .endif
      .endif
    .else   ;--- next (addr) ---
      .if .ident(.sprintf("for_inc%d_indirect", ::nesting_level/10))=1
        sec
        lda .ident(.sprintf("for_var%d", ::nesting_level/10))
        sbc .ident(.sprintf("for_inc%d", ::nesting_level/10))
        sta .ident(.sprintf("for_var%d", ::nesting_level/10))
        lda .ident(.sprintf("for_var%d", ::nesting_level/10))+1
        sbc .ident(.sprintf("for_inc%d", ::nesting_level/10))+1
        sta .ident(.sprintf("for_var%d", ::nesting_level/10))+1
        .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
          cmp .ident(.sprintf("for_end%d", ::nesting_level/10))+1
          bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
          longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          lda .ident(.sprintf("for_var%d", ::nesting_level/10))
          cmp .ident(.sprintf("for_end%d", ::nesting_level/10))
          longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .else
          cmp #>(.ident(.sprintf("for_end%d", ::nesting_level/10))+1)
          bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
          longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          lda .ident(.sprintf("for_var%d", ::nesting_level/10))
          cmp #<(.ident(.sprintf("for_end%d", ::nesting_level/10))+1)
          longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
        .endif
      .else
        .if .ident(.sprintf("for_inc%d", ::nesting_level/10))=1
          dec16 .ident(.sprintf("for_var%d", ::nesting_level/10))
          .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
            lda .ident(.sprintf("for_var%d", ::nesting_level/10))+1
            cmp .ident(.sprintf("for_end%d", ::nesting_level/10))+1
            bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            lda .ident(.sprintf("for_var%d", ::nesting_level/10))
            cmp .ident(.sprintf("for_end%d", ::nesting_level/10))
            longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .else
            lda .ident(.sprintf("for_var%d", ::nesting_level/10))
            .if (<.ident(.sprintf("for_end%d", ::nesting_level/10)))>0
              cmp #<.ident(.sprintf("for_end%d", ::nesting_level/10))
            .endif 
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            lda .ident(.sprintf("for_var%d", ::nesting_level/10))+1
            .if (>.ident(.sprintf("for_end%d", ::nesting_level/10)))>0
              cmp #>.ident(.sprintf("for_end%d", ::nesting_level/10))
            .endif
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .endif
        .else
          sec
          lda .ident(.sprintf("for_var%d", ::nesting_level/10))
          sbc #<.ident(.sprintf("for_inc%d", ::nesting_level/10))
          sta .ident(.sprintf("for_var%d", ::nesting_level/10))
          lda .ident(.sprintf("for_var%d", ::nesting_level/10))+1
          sbc #>.ident(.sprintf("for_inc%d", ::nesting_level/10))
          sta .ident(.sprintf("for_var%d", ::nesting_level/10))+1
          .if .ident(.sprintf("for_end%d_indirect", ::nesting_level/10))=1
            lda .ident(.sprintf("for_var%d", ::nesting_level/10))+1
            cmp .ident(.sprintf("for_end%d", ::nesting_level/10))+1
            bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
            longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            lda .ident(.sprintf("for_var%d", ::nesting_level/10))
            cmp .ident(.sprintf("for_end%d", ::nesting_level/10))
            longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
          .else
            .if .ident(.sprintf("for_end%d", ::nesting_level/10))-.ident(.sprintf("for_inc%d", ::nesting_level/10))>0
              lda .ident(.sprintf("for_var%d", ::nesting_level/10))+1
              cmp #>.ident(.sprintf("for_end%d", ::nesting_level/10))
              longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
              lda .ident(.sprintf("for_var%d", ::nesting_level/10))
              cmp #<.ident(.sprintf("for_end%d", ::nesting_level/10))     
              longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .else
              lda .ident(.sprintf("for_var%d", ::nesting_level/10))+1
              cmp #>(.ident(.sprintf("for_end%d", ::nesting_level/10))+1)
              bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
              longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
              lda .ident(.sprintf("for_var%d", ::nesting_level/10))
              cmp #<(.ident(.sprintf("for_end%d", ::nesting_level/10))+1)
              longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
            .endif
          .endif
        .endif
      .endif
    .endif
  .endif
.ident(.sprintf("end_loop%d", ::nesting_level/10)):
  dec_nesting_level
.endmacro 

;; do
;;   ...
;; <b>until</b>|<b>while</b> <i>cond</i>
;;   ...
;; <b>loop</b> [<b>until</b>|<b>while</b> <i>cond</i>]
;; Defines a loop that is exit based on a while or until condition
;; <i>cond</i> can be <i>eq</i>, <i>neq</i>, <i>minus</i>, <i>plus</i>, <i>c_clr</i>, <i>c_set</i>, <i>v_clr</i>, <i>v_set</i>, <i>lt</i>, <i>ge</i>
;; This corresponds to assembler commands BEQ, BNE, BMI, BPL, BCC, BCS, BVC, BVS
;; <i>lt</i> (less than) is equal to <i>c_clr</i> (BCC), <i>ge</i> (greater or equal) is equal to <i>c_set</i> (BCS)
;; <b>until</b> and <b>while</b> can be used on a seperate line or after <b>loop</b>
;; There can be any number of <b>until</b> or <b>while</b> conditions, also none, which defines an endless loop
;; Within a do...loop, the macros <i>break</i> and <i>continue</i> can be used to exit the loop or go to next iteration.
;;
;; Registers modified: none
;;
;; Code example that waits for joystick 2 button pressed:
;; <code>
;; lda #$10
;; do
;; &nbsp;  and $dc00
;; loop until eq
;; </code>

.macro do
  inc_nesting_level
  .ident(.sprintf("begin_loop%d", ::nesting_level/10)):
  .ident(.sprintf("continue_point%d", ::nesting_level/10)):
.endmacro

.macro until cond
  .if ((.xmatch ({cond}, {eq})) || (.xmatch ({cond}, {zero})))
    beq .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {neq}))
    bne .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {minus}))
    bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {plus}))
    bpl .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif ((.xmatch ({cond}, {c_clr})) || (.xmatch ({cond}, {lt})))
    bcc .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif ((.xmatch ({cond}, {c_set})) || (.xmatch ({cond}, {ge})))
    bcs .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {v_clr}))
    bvc .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {v_set}))
    bvs .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .else
    .error  "Syntax error in until condition"
  .endif
.endmacro

.macro while cond
  .if ((.xmatch ({cond}, {eq})) || (.xmatch ({cond}, {zero})))
    bne .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {neq}))
    beq .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {minus}))
    bpl .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {plus}))
    bmi .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif ((.xmatch ({cond}, {c_clr})) || (.xmatch ({cond}, {lt})))
    bcs .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif ((.xmatch ({cond}, {c_set})) || (.xmatch ({cond}, {ge})))
    bcc .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {v_clr}))
    bvs .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .elseif (.xmatch ({cond}, {v_set}))
    bvc .ident(.sprintf("end_loop%d", ::nesting_level/10))
  .else
    .error  "Syntax error in while condition"
  .endif
.endmacro

.macro break
  jmp .ident(.sprintf("end_loop%d", ::nesting_level/10))
.endmacro

.macro continue
  jmp .ident(.sprintf("continue_point%d", ::nesting_level/10))
.endmacro

.macro loop cond
  .if .paramcount = 0
    jmp .ident(.sprintf("begin_loop%d", ::nesting_level/10))
  .elseif .paramcount = 1
    .if ((.xmatch ({cond}, {until eq})) || (.xmatch ({cond}, {until zero})))
      longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif (.xmatch ({cond}, {until neq}))
      longbeq .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif (.xmatch ({cond}, {until minus}))
      longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif (.xmatch ({cond}, {until plus}))
      longbmi .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif ((.xmatch ({cond}, {until c_clr})) || (.xmatch ({cond}, {lt})))
      longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif ((.xmatch ({cond}, {until c_set})) || (.xmatch ({cond}, {ge})))
      longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif (.xmatch ({cond}, {until v_clr}))
      longbvs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif (.xmatch ({cond}, {until v_set}))
      longbvc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif ((.xmatch ({cond}, {while eq})) || (.xmatch ({cond}, {while zero})))
      longbeq .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif (.xmatch ({cond}, {while neq}))
      longbne .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif (.xmatch ({cond}, {while minus}))
      longbmi .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif (.xmatch ({cond}, {while plus}))
      longbpl .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif ((.xmatch ({cond}, {while c_clr})) || (.xmatch ({cond}, {while lt})))
      longbcc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif ((.xmatch ({cond}, {while c_set})) || (.xmatch ({cond}, {while ge})))
      longbcs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif (.xmatch ({cond}, {while v_clr}))
      longbvc .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .elseif (.xmatch ({cond}, {while v_set}))
      longbvs .ident(.sprintf("begin_loop%d", ::nesting_level/10))
    .else
      .error "Syntax error in loop condition"
    .endif
  .elseif 
    .error  "Too many parameters in loop condition"
  .endif
.ident(.sprintf("end_loop%d", ::nesting_level/10)):
  dec_nesting_level
.endmacro

;; switch A|X|Y|AX
;; case|case_eq|case_lt|case_ge num1
;;   ...
;; [break]
;; case num2
;;   ...
;; [break]
;; case num3
;;   ...
;; [break]
;; default
;; endswitch
;;   ...
;; Defines a selection mechanism to allow the value of a register to change the control flow
;; <b>case</b> and <b>case_eq</b> are the same function, they trigger if the register value and the comparison value are identical
;; <b>case_lt</b> ("less than") triggers if the register value is smaller than the comparison value 
;; <b>case_ge</b> ("greater or equal") triggers if the register value is greater or equal than the comparison value 
;; <b>break</b> jumps out of the structure. If break is omitted, the next comparison is done
;; <b>default</b> performs no comparison but triggers always
;;
;; Registers modified: none
;;

.macro switch reg

.endmacro