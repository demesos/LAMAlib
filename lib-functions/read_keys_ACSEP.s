; Reads keys @, :, ;, =, P
; Routine should not be interrupted by another keyboard scan, therefore it is
; recommended to:
; - run the function in the interrupt, or
; - have the keyboard scan in the interrupt turned off, or
; - put a sei / cli around the function call
; Return value is in A, where the respective bit is zero if the key is pressed

.include "../LAMAlib-macros16.inc"
.include "../LAMAlib-structured.inc"

.export _read_keys_ACSEP_sr
.import _twopotentials

.proc _read_keys_ACSEP_sr
	poke $dc02,0
	poke $dc03,6	           ;reverse keyboard matrix setup, PortB driving

	poke $dc01,$fd
	lda $dc00
	lsr			   ;shift down 1 bit
	ora #%11101111
	sta bits_4                 ;detecting key P (fire)

	poke $dc01,$fb
	lda $dc00
	and #%01000000
	beq unset
	lda #$ff
	.byte $2c	           ;BIT code to jump over next two bytes
unset:  lda #%11111101             ;key ; (down)

bits_4=*+1
	and #$42
	sta bits_14	

	poke $dc02,$ff
	poke $dc03,0               ;keyboard matrix setup, PortA driving, like Kernal

	poke $dc00,$df
	lda $dc01
	asl
	asl
	bpl unset2
	lda #$ff
	.byte $2c	           ;BIT code to jump over next two bytes
unset2: lda #%11111011             ;key : (left)
	if cc
	  and #%11111110
	endif
	;A contains now key : (left) and @ (up)
bits_14=*+1
	and #$42 
	sta bits_0124

	poke $dc00,$bf
	lda $dc01
	lsr
	lsr
	ora #%11110111	           ;detecting key = (right)
bits_0124=*+1
	and #$42
	;A contains now all five keys

	bit value_0c	           ;left and right ?
	if eq
	  sta bits_01234	   ;save values
	  poke $dc00,$fe
	  lda $dc01
	  and #%00000010	   ;joy1 down?
	  if eq
	    lda bits_01234
	    bit _twopotentials+4   ;P (fire) key pressed?
	    if eq
	      ora #%00000100	   ;delete left dir
              sta bits_01234
	    endif
	  endif
	  lda $dc01
	  and #%00000100	   ;joy1 left?
	  if eq
	    lda bits_01234
	    bit _twopotentials+1   ;; (down) key pressed?
	    if eq
	      ora #%00001000	   ;delete right dir
              sta bits_01234
	    endif
	  endif
bits_01234=*+1
	  lda #$42
	endif

	rts

value_0c:
	.byte $0c	
.endproc