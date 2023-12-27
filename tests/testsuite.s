;*****************************************************************************
; Test suite for LAMAlib
;
; by Wil, September 2022
;
; calls multiple tests to ensure everything (still) works as intended
;
; each test is to be saved in a file "testn" where n indicates the test number
; for each test:  Carry: 0 = No errors, 1 = Error
;*****************************************************************************

.include "LAMAlib.inc"

;.MACPACK generic

.if .not .definedmacro(add)
	.macpack generic
.endif

.macro checkerr testnr
	lda #testnr
	jsr checkerr
.endmacro

.FEATURE string_escapes

makesys 1984

; Main program starts here

testsuite:
	jsr test1
	checkerr 1

	jsr test2
	checkerr 2
	
	jsr test3
	checkerr 3
	
	jsr test4
	checkerr 4
	
	jsr test5
	checkerr 5

	jsr test6
	checkerr 6

	jsr test7
	checkerr 7

	jsr test8
	checkerr 8

	jsr test9
	checkerr 9

	rts

.proc checkerr
	if cc 
	  print "\x1etest ",A," successful.\x0a\x9a"
	else
	  print "\x1ctest ",A," failed.\x0a\x9a"
	endif
	rts
.endproc

test1:	.include "test1.s"
test2:	.include "test2.s"
test3:	.include "test3.s"
test4:	.include "test4.s"
test5:	.include "test5.s"
test6:	.include "test6.s"
test7:	.include "test7.s"
test8:	.include "test8.s"
test9:	.include "test9.s"