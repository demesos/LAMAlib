;-----------------------------------------------------------------------
; print_wrapped
; Prints a string into a window with wrapping
;-----------------------------------------------------------------------

.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"
.include "../LAMAlib-ROMfunctions.inc"

.export _print_wrapped_x1_128       := x1
.export _print_wrapped_width_128    := width
.export _print_wrapped_endchar_128  := endchar
.export _print_wrapped_windowed_128 := print_wrapped_windowed

.if .not .definedmacro(add)
.macpack generic      
.endif

.if .not .definedmacro(jne)
.macpack longbranch      
.endif

CURR_COLUMN=$EC
SCREEN_WIDTH=40

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
        lda #2*SCREEN_WIDTH
        sub ::CURR_COLUMN
        ;if (rest > 40) rest -= 40;
        cmp #SCREEN_WIDTH+1
        if cs
          sbc #SCREEN_WIDTH	;carry is set because of if condition
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
rest=*+1
        cpy #$af
        if cc	;does the word fit into current line?
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
	else
	  lda ::CURR_COLUMN
	  beq indent
	  cmp #SCREEN_WIDTH
	  beq indent

	  lda #17
	  jsr CHROUT
indent:
	  jsr go_to_left_margin

	  ;skip space and cr after new line
	  ldy #0
	  lda (text),y
	  cmp #32
	  beq skip_text_byte
	  cmp #13
	  bne print_loop
skip_text_byte:
	  jsr adv_text_ptr
	endif
	jmp print_loop

adv_text_ptr:
	inc16 text
	rts

.proc go_to_left_margin
	lda ::CURR_COLUMN
	cmp #SCREEN_WIDTH
	lda x1
	if cs		;carry still defined from comparison
	  adc #SCREEN_WIDTH-1	;we are adding actually 40 b/c carry is set
	endif
        sta ::CURR_COLUMN
	rts
.endproc

.endproc

x1:	.byte   0
width:	.byte  SCREEN_WIDTH
endchar: .byte 0
