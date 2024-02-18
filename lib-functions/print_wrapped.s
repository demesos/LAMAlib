;-----------------------------------------------------------------------
; print_wrapped
; Prints a string into a window with wrapping
;-----------------------------------------------------------------------

.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"
.include "../LAMAlib-ROMfunctions.inc"

.export _print_wrapped_x1       := x1
.export _print_wrapped_width    := width
.export _print_wrapped_endchar  := endchar
.export _print_wrapped_windowed := print_wrapped_windowed

.if .not .definedmacro(add)
.macpack generic      
.endif

.if .not .definedmacro(jne)
.macpack longbranch      
.endif

.importzp _llzp_word1

text=_llzp_word1

.proc print_wrapped_windowed

        ;save contents of _llzp_word1
        tay
        lda text
        pha
        lda text+1
        pha

        ;put pointer that was passed in AX into ZP regs
        sty text	;y because we did tay
        stx text+1

        ;calc right margin
        lda #40
        sub x1
        sbc width
        sta right_margin

print_loop:
	ldy #0
	lda (text),y
	jeq done_print_loop

	jsr calc_rest	;result in A and rest

        ;calculate length of next word
        ldy #$ff
next_char:
        iny
        lda (text),y
        beq done
        cmp #32
        beq done
        cmp #13
        bne next_char
done:	;y now contains the length of the next word
        cpy rest
        if cs	;is the word too long to fit into current line?
          beq just_fit	;the next word fits exactly
          lda #13
          jsr CHROUT
          lda x1
          sta 211
        endif
just_fit:	
        cpy #0
        if ne ;do we have a word with length>0 to print?
          ldy #0
print_word_loop:
          lda (text),y
          beq done_print_word_loop
          cmp #32
          beq done_print_word_loop
          cmp #13    
          beq done_print_word_loop
          jsr CHROUT
          inc16 text
          jmp print_word_loop
done_print_word_loop:
	  jsr calc_rest
	  if eq	;rest==0 ?
	    lda #13
            jsr CHROUT
            lda x1
            sta 211
	    lda width
	    sta rest
	  endif
	  cmp width
          bne print_loop
	  beq entry		;unconditional jump
skip_space_after_new_line:
	  inc16 text
entry:
          lda (text),y   
	  cmp #32
          beq skip_space_after_new_line
        endif  ;if ne ;do we have...

	;ldy #0		;not necessary, y=0 at that point
	lda (text),y  
	cmp #32
	bne check_cr
	dec rest
	beq cr_out
	jsr CHROUT
	jmp adv_text_ptr
check_cr:
	cmp #13
	if eq
cr_out:
	  lda #13
	  jsr CHROUT
	  lda x1
	  sta 211
adv_text_ptr:
	  inc16 text
	endif
        jmp print_loop
done_print_loop:
        ;restore contents of _llzp_word1
        pla
        sta text+1
        pla
        lda text
        rts


calc_rest:
        ;rest = 80 - peek (211);
        lda #80
        sub 211
        ;if (rest > 40) rest -= 40;
        cmp #41
        if cs
          sbc #40	;carry is set because of if condition
        endif 
        ;rest -= right_margin;
	sec
right_margin=*+1
	sbc #$af
	sta rest
	rts

rest:          .byte 0

.endproc

x1:	.byte   0
width:	.byte  40
endchar: .byte 0
