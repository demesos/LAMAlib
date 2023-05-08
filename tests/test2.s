; tests for AX, store, restore with lowercase characters, do loop, if else endif

.include "LAMAlib.inc"

.proc test_no_2
	pokew sum,0
	
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

	ldax sum

	cmpax #16032
	if eq
	  clc
	else
	  sec
	endif
	rts

sum:
.byte 00,00

.endproc