;; <h2>Structured Programming</h2>
;; These commands allow you to use constructs like <i>if .. else .. endif</i>, <i>do...loop</i>, <i>for...next</i>, and <i>switch...case</i> in assembly language! The structures can even be nested. The implementation of these structures is basically as efficient as a a handcoded composure of branches, jumps as labels, while it is much easier to write and read.
;; All macros can be nested.

;; store reg
;; Generates self-modifying code for store/recall of a register
;; Registers can be A,X,Y, or AX (that is A and X)
;; An example application for <i>store</i>/<i>recall</i> is for saving a register to be able use it in between. An alternative to <i>store</i>/<i>recall</i> are stack push/pull, which is more compact, but 1 cycle slower.
;; Each store must match with exactly one <i>recall</i> macro using the same register, recall has to be placed later in your code
;; Registers modified: none
.macro store reg
  .scope
  .if .xmatch ({reg}, {A}) 
    stored_A=selfmodA+1
    sta stored_A
  .elseif .xmatch ({reg}, {X}) 
    stored_X=selfmodX+1
    stx stored_X
  .elseif .xmatch ({reg}, {Y}) 
    stored_Y=selfmodY+1
    sty stored_Y
  .elseif .xmatch ({reg}, {AX}) 
    stored_A=selfmodA+1
    sta stored_A
    stored_X=selfmodX+1
    stx stored_X
  .endif
.endmacro

;; recall reg
;; Generates the code for recalling a stored register
;; Registers can be A,X,Y, or AX (that is A and X)
;; Each recall must match with exactly one preceding <i>store</i> macro using the same register
;; Registers modified: the recalled register

.macro recall reg
  .if .xmatch ({reg}, {A}) 
    selfmodA: lda #00
  .elseif .xmatch ({reg}, {X}) 
    selfmodX: ldx #00
  .elseif .xmatch ({reg}, {Y}) 
    selfmodY: ldy #00
  .elseif .xmatch ({reg}, {AX}) 
    selfmodA: lda #00
    selfmodX: ldx #00
  .endif
  .endscope
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
  .scope
  .if ((.xmatch ({cond}, {eq})) || (.xmatch ({cond}, {zero})))
    bne else_or_end
  .elseif (.xmatch ({cond}, {neq}))
    beq else_or_end
  .elseif (.xmatch ({cond}, {minus}))
    bpl else_or_end
  .elseif (.xmatch ({cond}, {plus}))
    bmi else_or_end
  .elseif ((.xmatch ({cond}, {c_clr})) || (.xmatch ({cond}, {lt})))
    bcs else_or_end
  .elseif ((.xmatch ({cond}, {c_set})) || (.xmatch ({cond}, {ge})))
    bcc else_or_end
  .elseif (.xmatch ({cond}, {v_clr}))
    bvs else_or_end
  .elseif (.xmatch ({cond}, {v_set}))
    bvc else_or_end
  .else
    .error  "Condition for if undefined!"
  .endif
.endmacro

.macro else
  jmp end_of_if
  else_or_end:
.endmacro

.macro endif
  .ifndef else_or_end
    else_or_end:
  .endif
  end_of_if:
  .endscope
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
  .scope
  .if .xmatch ({var}, {A})
    for_var=-1
  .elseif .xmatch ({var}, {X})
    for_var=-2
  .elseif .xmatch ({var}, {Y})
    for_var=-3
  .elseif .xmatch ({var}, {AX})
    for_var=-4
  .else
    for_var=var
  .endif

  .if .xmatch ({dir}, {to}) 
    for_dir=1
  .elseif .xmatch ({dir}, {downto}) 
    for_dir=-1
  .else
    .error  "Syntax error in for loop, 3rd argument needs to be TO or DOWNTO!"
  .endif

  .if .paramcount = 5
     for_inc=step
     .if (.match (.left (1, {step}), {(})) .and (.match (.right (1, {step}), {)}))	;argument in brackets -> means indirect addressing
       for_inc_indirect=1
     .else
       for_inc_indirect=0
     .endif
  .else
      for_inc=1
      for_inc_indirect=0
  .endif

  .if (.match (.left (1, {end}), {(})) .and (.match (.right (1, {end}), {)}))	;argument in brackets -> means indirect addressing
    for_end_indirect=1
    for_end=end
  .else
    for_end_indirect=0
    .if for_dir>0
      for_end=end+1
    .else
      for_end=end-1
    .endif
  .endif

  .if (.match (.left (1, {start}), {(})) .and (.match (.right (1, {start}), {)}))	;argument in brackets -> means indirect addressing
    for_start_positive=0
    .if for_var=-2
      ldx 0+start
    .elseif for_var=-3
      ldy 0+start
    .elseif for_var=-1
      lda 0+start
    .elseif for_var=-4
      ldax 0+start
    .else
      lda 0+start
      sta for_var
      lda 0+start+1
      sta for_var+1
    .endif
  .else
    .if for_var=-2
      ldx #start
      .if start<128
        for_start_positive=1
      .else
        for_start_positive=0
      .endif
    .elseif for_var=-3
      ldy #start
      .if start<128
        for_start_positive=1
      .else
        for_start_positive=0
      .endif
    .elseif for_var=-1
      lda #start
      .if start<128
        for_start_positive=1
      .else
        for_start_positive=0
      .endif
    .elseif for_var=-4
      ldax #start
      .if start<32768
        for_start_positive=1
      .else
        for_start_positive=0
      .endif
    .else
      .if start<32768
        for_start_positive=1
      .else
        for_start_positive=0
      .endif
      lda #<start
      sta for_var
      lda #>start
      sta for_var+1
    .endif
  .endif
begin_loop:
.endmacro

.macro next
continue_point:
  .if for_dir>0   ;--- incrementing loops ---
    .if for_var=-2   ;--- next X ---
      .if for_inc_indirect=1
        txa
        clc
        adc for_inc
        tax
        .if for_end_indirect=1
          cpx for_end
          bcc begin_loop
          beq begin_loop
        .else
          cpx #for_end
          bcc begin_loop
        .endif
      .else
        .repeat for_inc
        inx
        .endrep
        .if for_end_indirect=1
          cpx for_end
          bcc begin_loop
          beq begin_loop
        .else
          .if for_inc=1
            .if .not for_end=0
              cpx #for_end
            .endif
            bne begin_loop
          .else
            cpx #for_end
            bcc begin_loop
          .endif
        .endif
      .endif
    .elseif for_var=-3   ;--- next Y ---
      .if for_inc_indirect=1
        tya
        clc
        adc for_inc
        tay
        .if for_end_indirect=1
          cpy for_end
          bcc begin_loop
          beq begin_loop
        .else
          cpy #for_end
          bcc begin_loop
        .endif
      .else
        .repeat for_inc
        iny
        .endrep
        .if for_end_indirect=1
          cpy for_end
          bcc begin_loop
          beq begin_loop
        .else
          .if for_inc=1
            .if .not for_end=0
              cpy #for_end
            .endif
            bne begin_loop
          .else
            cpy #for_end
            bcc begin_loop
          .endif
        .endif
      .endif
    .elseif for_var=-1   ;--- next A ---
      .if for_inc_indirect=1
        clc
        adc for_inc
        .if for_end_indirect=1
          cmp for_end
          bcc begin_loop
          beq begin_loop
        .else
          cmp #for_end
          bcc begin_loop
        .endif
      .else
        clc
        adc #for_inc
        .if for_end_indirect=1
          cmp for_end
          bcc begin_loop
          beq begin_loop
        .else
          .if for_inc=1
            .if .not for_end=0
              cmp #for_end
            .endif
            bne begin_loop
          .else
            cmp #for_end
            bcc begin_loop
          .endif
        .endif
      .endif
    .elseif for_var=-4   ;--- next AX ---
      .if for_inc_indirect=1
        clc
        adcax for_inc
      .else
        .if for_inc=1
          incax
        .else
          clc
          adcax #for_inc
        .endif
      .endif
      .if for_end_indirect=1
        cpx for_end+1
        bcc begin_loop
        cmp for_end
        bcc begin_loop
        beq begin_loop
      .else
        cpx #>for_end
        bcc begin_loop
        cmp #<for_end     
        bcc begin_loop
      .endif
    .else   ;--- next (addr) ---
      .if for_inc_indirect=1
        clc
        lda for_var
        adc for_inc
        sta for_var
        lda for_var+1
        adc for_inc+1
        sta for_var+1
        .if for_end_indirect=1
          cmp for_end+1
          bcc begin_loop
          bne end_loop
          lda for_end
          cmp for_var
          bcs begin_loop
        .else
          cmp #>for_end
          bcc begin_loop
          bne end_loop
          lda for_var
          cmp #<for_end     
          bcc begin_loop
        .endif
      .else
        .if for_inc=1
          inc16 for_var
          .if for_end_indirect=1
            lda for_var+1
            cmp for_end+1
            bcc begin_loop
            lda for_end
            cmp for_var
            bcs begin_loop
          .else
            lda for_var
            .if (<for_end)>0
              cmp #<for_end
            .endif 
            bne begin_loop
            lda for_var+1
            .if (>for_end)>0
              cmp #>for_end
            .endif
            bne begin_loop
          .endif
        .else
          clc
          lda for_var
          adc #<for_inc
          sta for_var
          lda for_var+1
          adc #>for_inc
          sta for_var+1
          .if for_end_indirect=1
            cmp for_end+1
            bcc begin_loop
            bne end_loop
            lda for_end
            cmp for_var
            bcs begin_loop
          .else
            cmp #>for_end
            bcc begin_loop
            bne end_loop
            lda for_var
            cmp #<for_end     
            bcc begin_loop
          .endif
        .endif
      .endif
    .endif
  .else              ;--- decrementing loops ---
    .if for_var=-2   ;--- next X ---
      .if for_inc_indirect=1
        txa
        sec
        sbc for_inc
        tax
        .if for_end_indirect=1
          cpx for_end
          bpl begin_loop
        .else
          .if for_end >= 0
            cpx #for_end+1
          .endif
          bpl begin_loop
        .endif
      .else
        .repeat for_inc
        dex
        .endrep
        .if for_end_indirect=1
          cpx for_end
          bpl begin_loop
        .else
          .if for_inc=1
            .if for_end = -1
              .if for_start_positive=1
                bpl begin_loop
              .else
                cpx #for_end+$100
                bne begin_loop
              .endif
            .else
              .if for_end > 0
                cpx #for_end
              .endif
              bne begin_loop
            .endif
          .else
            .if for_end-for_inc < 0
              .if for_start_positive=1
                bmi end_loop
              .else
                cpx #for_end-for_inc+$100
                bcs end_loop
              .endif
            .endif
            .if for_end < 0
              jmp begin_loop
            .elseif for_end > 0
              cpx #for_end+1
              bcs begin_loop
    	    .else
              bne begin_loop
            .endif
          .endif
        .endif
      .endif
    .elseif for_var=-3   ;--- next Y ---
      .if for_inc_indirect=1
        tya
        sec
        sbc for_inc
        tay
        .if for_end_indirect=1
          cpy for_end
          bpl begin_loop
        .else
          .if for_end >= 0
            cpy #for_end+1
          .endif
          bpl begin_loop
        .endif
      .else
        .repeat for_inc
        dey
        .endrep
        .if for_end_indirect=1
          cpy for_end
          bpl begin_loop
        .else
          .if for_inc=1
            .if for_end = -1
              .if for_start_positive=1
                bpl begin_loop
              .else
                cpy #for_end+$100
                bne begin_loop
              .endif
            .else
              .if for_end > 0
                cpy #for_end
              .endif
              bne begin_loop
            .endif
          .else
            .if for_end-for_inc < 0
              .if for_start_positive=1
                bmi end_loop
              .else
                cpy #for_end-for_inc+$100
                bcs end_loop
              .endif
            .endif
            .if for_end < 0
              jmp begin_loop
            .elseif for_end > 0
              cpy #for_end+1
              bcs begin_loop
    	    .else
              bne begin_loop
            .endif
          .endif
        .endif
      .endif
    .elseif for_var=-1   ;--- next A ---
      .if for_inc_indirect=1
        sec
        sbc for_inc
        .if for_end_indirect=1
          cmp for_end
          bpl begin_loop
        .else
          .if for_end >= 0
            cmp #for_end+1
          .endif
          bpl begin_loop
        .endif
      .else
        sec
        sbc #for_inc
        .if for_end_indirect=1
          cmp for_end
          bpl begin_loop
        .else
          .if for_inc=1
            .if for_end = -1
              .if for_start_positive=1
                bpl begin_loop
              .else
                cmp #for_end+$100
                bne begin_loop
              .endif
            .else
              .if for_end > 0
                cmp #for_end
              .endif
              bne begin_loop
            .endif
          .else
            .if for_end-for_inc < 0
              .if for_start_positive=1
                bmi end_loop
              .else
                cmp #for_end-for_inc+$100
                bcs end_loop
              .endif
            .endif
            .if for_end < 0
              jmp begin_loop
            .elseif for_end > 0
              cmp #for_end+1
              bcs begin_loop
    	    .else
              bne begin_loop
            .endif
          .endif
        .endif
      .endif
    .elseif for_var=-4   ;--- next AX ---
      .if for_inc_indirect=1
        sec
        sbcax for_inc
        .if for_end_indirect=1
          cpx for_end+1
          bmi end_loop
          bne begin_loop
          cmp for_end
          bcs begin_loop
        .else
          cpx #>(for_end+1)
          bmi end_loop
          bne begin_loop
          cmp #<(for_end+1)
          bcs begin_loop
        .endif
      .else
        .if for_inc=1
          decax
          .if for_end_indirect=1
            cpx for_end+1
            bmi end_loop
            bne begin_loop
            cmp for_end
            bcs begin_loop
          .else
            cpx #>for_end
            bne begin_loop
            cmp #<for_end     
            bne begin_loop
          .endif
        .else
          sec
          sbcax #for_inc
          .if for_end_indirect=1
            cpx for_end+1
            bmi end_loop
            bne begin_loop
            cmp for_end
            bcs begin_loop
          .else
            .if for_end-for_inc>0
              cpx #>for_end
              bcc begin_loop
              cmp #<for_end     
              bcc begin_loop
            .else
              cpx #>(for_end+1)
              bmi end_loop
              bne begin_loop
              cmp #<(for_end+1)
              bcs begin_loop
            .endif
          .endif
        .endif
      .endif
    .else   ;--- next (addr) ---
      .if for_inc_indirect=1
        sec
        lda for_var
        sbc for_inc
        sta for_var
        lda for_var+1
        sbc for_inc+1
        sta for_var+1
        .if for_end_indirect=1
          cmp for_end+1
          bmi end_loop
          bne begin_loop
          lda for_var
          cmp for_end
          bcs begin_loop
        .else
          cmp #>(for_end+1)
          bmi end_loop
          bne begin_loop
          lda for_var
          cmp #<(for_end+1)
          bcs begin_loop
        .endif
      .else
        .if for_inc=1
          dec16 for_var
          .if for_end_indirect=1
            lda for_var+1
            cmp for_end+1
            bmi end_loop
            bne begin_loop
            lda for_var
            cmp for_end
            bcs begin_loop
          .else
            lda for_var
            .if (<for_end)>0
              cmp #<for_end
            .endif 
            bne begin_loop
            lda for_var+1
            .if (>for_end)>0
              cmp #>for_end
            .endif
            bne begin_loop
          .endif
        .else
          sec
          lda for_var
          sbc #<for_inc
          sta for_var
          lda for_var+1
          sbc #>for_inc
          sta for_var+1
          .if for_end_indirect=1
            lda for_var+1
            cmp for_end+1
            bmi end_loop
            bne begin_loop
            lda for_var
            cmp for_end
            bcs begin_loop
          .else
            .if for_end-for_inc>0
              lda for_var+1
              cmp #>for_end
              bcc begin_loop
              lda for_var
              cmp #<for_end     
              bcc begin_loop
            .else
              lda for_var+1
              cmp #>(for_end+1)
              bmi end_loop
              bne begin_loop
              lda for_var
              cmp #<(for_end+1)
              bcs begin_loop
            .endif
          .endif
        .endif
      .endif
    .endif
  .endif
end_loop:
  .endscope
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
  .scope
  begin_loop:
  continue_point:
.endmacro

.macro until cond
  .if ((.xmatch ({cond}, {eq})) || (.xmatch ({cond}, {zero})))
    beq end_loop
  .elseif (.xmatch ({cond}, {neq}))
    bne end_loop
  .elseif (.xmatch ({cond}, {minus}))
    bmi end_loop
  .elseif (.xmatch ({cond}, {plus}))
    bpl end_loop
  .elseif ((.xmatch ({cond}, {c_clr})) || (.xmatch ({cond}, {lt})))
    bcc end_loop
  .elseif ((.xmatch ({cond}, {c_set})) || (.xmatch ({cond}, {ge})))
    bcs end_loop
  .elseif (.xmatch ({cond}, {v_clr}))
    bvc end_loop
  .elseif (.xmatch ({cond}, {v_set}))
    bvs end_loop
  .else
    .error  "Syntax error in until condition"
  .endif
.endmacro

.macro while cond
  .if ((.xmatch ({cond}, {eq})) || (.xmatch ({cond}, {zero})))
    bne end_loop
  .elseif (.xmatch ({cond}, {neq}))
    beq end_loop
  .elseif (.xmatch ({cond}, {minus}))
    bpl end_loop
  .elseif (.xmatch ({cond}, {plus}))
    bmi end_loop
  .elseif ((.xmatch ({cond}, {c_clr})) || (.xmatch ({cond}, {lt})))
    bcs end_loop
  .elseif ((.xmatch ({cond}, {c_set})) || (.xmatch ({cond}, {ge})))
    bcc end_loop
  .elseif (.xmatch ({cond}, {v_clr}))
    bvs end_loop
  .elseif (.xmatch ({cond}, {v_set}))
    bvc end_loop
  .else
    .error  "Syntax error in while condition"
  .endif
.endmacro

.macro break
  jmp end_loop
.endmacro

.macro continue
  jmp continue_point
.endmacro

.macro loop cond
  .if .paramcount = 0
    jmp begin_loop
  .elseif .paramcount = 1
    .if ((.xmatch ({cond}, {until eq})) || (.xmatch ({cond}, {until zero})))
      bne begin_loop
    .elseif (.xmatch ({cond}, {until neq}))
      beq begin_loop
    .elseif (.xmatch ({cond}, {until minus}))
      bpl begin_loop
    .elseif (.xmatch ({cond}, {until plus}))
      bmi begin_loop
    .elseif ((.xmatch ({cond}, {until c_clr})) || (.xmatch ({cond}, {lt})))
      bcs begin_loop
    .elseif ((.xmatch ({cond}, {until c_set})) || (.xmatch ({cond}, {ge})))
      bcc begin_loop
    .elseif (.xmatch ({cond}, {until v_clr}))
      bvs begin_loop
    .elseif (.xmatch ({cond}, {until v_set}))
      bvc begin_loop
    .elseif ((.xmatch ({cond}, {while eq})) || (.xmatch ({cond}, {while zero})))
      beq begin_loop
    .elseif (.xmatch ({cond}, {while neq}))
      bne begin_loop
    .elseif (.xmatch ({cond}, {while minus}))
      bmi begin_loop
    .elseif (.xmatch ({cond}, {while plus}))
      bpl begin_loop
    .elseif ((.xmatch ({cond}, {while c_clr})) || (.xmatch ({cond}, {while lt})))
      bcc begin_loop
    .elseif ((.xmatch ({cond}, {while c_set})) || (.xmatch ({cond}, {while ge})))
      bcs begin_loop
    .elseif (.xmatch ({cond}, {while v_clr}))
      bvc begin_loop
    .elseif (.xmatch ({cond}, {while v_set}))
      bvs begin_loop
    .else
      .error "Syntax error in loop condition"
    .endif
  .elseif 
    .error  "Too many parameters in loop condition"
  .endif
end_loop:  
  .endscope
.endmacro

