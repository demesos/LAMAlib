; test switch - case

.include "LAMAlib.inc"

testscreen=$c000

.proc test_no_5

	lda 211
	pha
	lda 214
	pha


	poke 648,>testscreen   ;print to screen at testcreen

	;clear parts of the screen without affecting teh color RAM
	memset testscreen,testscreen+40,32
	lda #19
	jsr CHROUT

	for A,0,to,5
	  store A
	  jsr num_to_text
	  restore A
	next

	poke 648,4	      ;switch screen back to 1024

	pla
	sta 214
	pla
	sta 211
	jsr $E56C	      ;set cursor pos to cursor row, column in 214, 211

	checksum_eor testscreen, testscreen+39

	cmp #57
	if eq
	  clc
	else
	  sec
	endif
	rts

num_to_text:
	switch A
	case 1
	   print "one"
	   break
	case 2
	   print "two"
	   break
	case 3
	   print "three"
	   break
	default
	   print "?"
	endswitch

	print " "
	rts
.endproc






















