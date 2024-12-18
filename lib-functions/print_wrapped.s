;-----------------------------------------------------------------------
; print_wrapped
; Prints a string into a window with wrapping
;-----------------------------------------------------------------------

.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"
.include "../LAMAlib-systemaddresses.inc"

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

CURR_COLUMN=211	;address where Kernal stores the column of the current cursor position
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

	cmp #13
	if eq
	  lda #17	;crsr down
	  jsr CHROUT
	  jsr go_to_left_margin		
          jsr adv_text_ptr
          jmp print_loop
	endif

        ;rest = 80 - peek (CURR_COLUMN);
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

        ;calculate length of next word including a leading space
        lda (text),y
	cmp #32
	bne nospace
	iny	;increase y to walk over space
nospace:
        dey	;instead of ldy #$ff because y = 0 from before
next_char:
        iny
        lda (text),y
        beq done	;0 ends the word (and the string)
        cmp #32
        beq done	;space ends the word
        cmp #13
        bne next_char	;cr ends the word, everything else continue
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
	  lda ::CURR_COLUMN	;are we at beginning of a line?
	  beq indent
	  cmp #SCREEN_WIDTH	;or are we at first column of extended line
	  beq indent
	else
nextline:
	  lda ::CURR_COLUMN
	  beq indent
	  cmp #SCREEN_WIDTH
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
	lda ::CURR_COLUMN
	cmp #SCREEN_WIDTH
	lda x1
	if cs		;carry still defined from comparison
	  adc #SCREEN_WIDTH-1	;we are actually adding SCREEN_WIDTH b/c carry is set
	endif
        sta ::CURR_COLUMN
	rts
.endproc

.endproc

x1:	.byte   0
width:	.byte  SCREEN_WIDTH
endchar: .byte 0
