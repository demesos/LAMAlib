;--------------------------------------------------
; test program for LAMAlib functions
;
; tests for AX, store, restore with lowercase characters, do loop, if else endif


.include "LAMAlib.inc"

.proc test_no_2

	lda #4
	ldx #$ff
	store A
	store X
	lda #$fc
	clc
	adc stored_A
	adc stored_X	;A should contain now 0
	pha
	restore A
	restore X
	pla
	inx	;X should contain now 0
	stax sum
	
	for Y,0,to,100
	  store y
	  dey
	  tya
	  and #%01101100
	  if ne
	    clc
	    adc sum
	    sta sum
	  else
	    if mi
	      sec
	      sbc sum
	      sta sum
	    endif
	  endif

	  for AX,$df8,to,$e10
	    store ax
	    clc
	    adcax sum
	    stax sum
	    restore ax
	  next

	  restore y
	next

	ldax sum	;should be 16032 at this point

	cmpax #16032
	longif eq 
          if_X_in #1,#2,#<16032,#>16032
            txa
            if_X_in #1,#2,#<16032,#>16032
              clc
            else
              sec
            endif
          else
            sec
          endif
        else
	  sec
	endif
	rts

sum:
.byte 00,00

.endproc