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

.ifdef SAVEREGS
  ::SAVE_REGS .set 2
.endif

.if .not .definedmacro(add)
	.macpack generic
.endif

.macro checkerr testnr
	lda #testnr
	jsr checkerr
.endmacro

.FEATURE string_escapes

makesys 1984

	clrscr
	lda #$0d
	jsr CHROUT
	jsr CHROUT
	jsr CHROUT

; Main program starts here

testsuite:
	jsr test1	; testing: for (addr),start,to,end
	checkerr 1

	jsr test2	; tests for AX, store, restore with lowercase characters, do loop, if else endif
	checkerr 2
	
	jsr test3	; test the random number generator
	checkerr 3
	
	jsr test4	; test reverse subtract macros rsb, rsbax
	checkerr 4
	
	jsr test5	; test switch - case
	checkerr 5

	jsr test6	; test delay_ms
	checkerr 6

	jsr test7	; testing the include_file_as macro 
	checkerr 7

	jsr test8	; test hw timer and delay_ms function
	checkerr 8

	jsr test9	; test do_every structure
	checkerr 9

	jsr test10	; test A_between, division tests
	checkerr 10

	jsr test11	; testing inc8, inc16, dec8, dec16 macros
	checkerr 11

	jsr test12	; testing window functions
	checkerr 12

	jsr test13	; testing print_wrapped
	checkerr 13

	jsr test14	; testing strlen functions
	checkerr 14

	jsr test15	; testing strlen functions
	checkerr 15

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
test10:	.include "test10.s"
test11:	.include "test11.s"
test12:	.include "test12.s"
test13:	.include "test13.s"
test14:	.include "test14.s"
test15:	.include "test15.s"
