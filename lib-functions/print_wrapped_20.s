;-----------------------------------------------------------------------
; print_wrapped
; Prints a string into a window with wrapping
;-----------------------------------------------------------------------

.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"
.include "../LAMAlib-ROMfunctions.inc"

.export _print_wrapped_x1_20       := x1
.export _print_wrapped_width_20    := width
.export _print_wrapped_endchar_20  := endchar
.export _print_wrapped_windowed_20 := print_wrapped_windowed

.if .not .definedmacro(add)
.macpack generic      
.endif

.if .not .definedmacro(jne)
.macpack longbranch      
.endif

CURR_COLUMN=211
SCREEN_WIDTH=22

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
        lda #SCREEN_WIDTH
        sub x1
        sbc width
        sta right_margin

print_loop:
	ldy #0
	lda (text),y
	if eq
	  lda endchar
	  if ne
	    jsr CHROUT
	  endif

          ;restore contents of _llzp_word1
          pla
          sta text+1
          pla
          lda text
          rts
	endif

        ;rest = 80 - peek (211);
        lda #4*SCREEN_WIDTH
        sub ::CURR_COLUMN
        ;if (rest > 40) rest -= 40;
re_check:
        cmp #SCREEN_WIDTH+1
        if cs
          sbc #SCREEN_WIDTH	;carry is set because of if condition
	  bcs re_check		;unconditional jump because result is never negative
        endif 
        ;rest -= right_margin;
	sec
right_margin=*+1
	sbc #$af
	sta rest

	cmp width
	if cs
	  jsr go_to_left_margin
	  lda width
	endif

        ;calculate length of next word
        dey	;instead of ldy #$ff because y = 0 from before
next_char:
        iny
        lda (text),y
        beq done
        cmp #32
        beq done
        cmp #13
        bne next_char
done:	;y now contains the length of the next word
	dey
	if mi
	  iny
	endif
	cpy width
	if cs		;is the word too long to even fit the line?
	  ldy rest
	  beq nextline
	  ldx #0
printword1:
	  lda (text,x)
	  jsr CHROUT
	  jsr adv_text_ptr
	  dey
	  bne printword1
	  jmp nextline
	endif
rest=*+1
        cpy #$af
	if cc		;does the word fit into current line?
	  ldx #0
printword:
	  lda (text,x)
	  jsr CHROUT
	  jsr adv_text_ptr
	  dey
	  bpl printword
	  lda ::CURR_COLUMN
	  beq indent
	  cmp #SCREEN_WIDTH
	  beq indent
	  cmp #2*SCREEN_WIDTH
	  beq indent
	  cmp #3*SCREEN_WIDTH
	  beq indent
	else
nextline:
	  lda ::CURR_COLUMN
	  beq indent
	  cmp #SCREEN_WIDTH
	  beq indent
	  cmp #2*SCREEN_WIDTH
	  beq indent
	  cmp #3*SCREEN_WIDTH
	  beq indent

	  lda #17	;crsr down
	  jsr CHROUT
indent:
	  jsr go_to_left_margin

	  ;skip space and cr after new line
	  ldy #0
	  lda (text),y
	  cmp #32
	  beq skip_text_byte
	  cmp #13
	  bne goto_print_loop
skip_text_byte:
	  jsr adv_text_ptr
	endif
goto_print_loop:
	jmp print_loop

.proc adv_text_ptr
	inc16 ::text
	rts
.endproc

.proc go_to_left_margin
	lda #SCREEN_WIDTH-1
findlinestart:
	cmp ::CURR_COLUMN
	bcs foundlinestart
	;clc	;not necessary because of the not taken bcs
	adc #SCREEN_WIDTH
	bcc findlinestart
foundlinestart:
	;sec	;not necessary because of the taken bcs
	sbc #SCREEN_WIDTH
	sec
	adc x1
        sta ::CURR_COLUMN
	rts
.endproc

.endproc

x1:	.byte   0
width:	.byte  SCREEN_WIDTH
endchar: .byte 0
