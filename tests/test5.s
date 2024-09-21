;--------------------------------------------------
; test program for LAMAlib functions
;
; test switch - case and on_A_jmp, on_A_jsr

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
	if ne
	  sec
	  rts
	endif
	jmp test_on_a_jxx

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

;---------------------------------------------
test_on_a_jxx:
checksum=2

pokew checksum,0
for A,0,to,42
store A
jsr test_on_A_jmp
restore A
next

ldax checksum
cmpax #903
	if ne
	  sec
	  rts
	endif



pokew checksum,0
for A,0,to,42
store A
jsr test_on_A_jmp0
restore A
next

ldax checksum
cmpax #903
	if ne
	  sec
	  rts
	endif


pokew checksum,0
for A,0,to,42
store A
jsr test_on_A_jmp0_nocheck
restore A
next

ldax checksum
cmpax #903
	if ne
	  sec
	  rts
	endif


pokew checksum,0
for A,0,to,42
store A
jsr test_on_A_jmp_mul3
restore A
next

ldax checksum
cmpax #903
	if ne
	  sec
	  rts
	endif


pokew checksum,0
for A,0,to,31
store A
jsr test_on_A_jmp_mul4
restore A
next

ldax checksum
cmpax #496
	if ne
	  sec
	  rts
	endif


pokew checksum,0
for A,0,to,43
store A
on_A_jsr  r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16,r17,r18,r19,r20,r21,r22,r23,r24,r25,r26,r27,r28,r29,r30,r31,r32,r33,r34,r35,r36,r37,r38,r39,r40,r41,r42
restore A
next

ldax checksum
cmpax #903
	if ne
	  sec
	  rts
	endif


pokew checksum,0
for A,0,to,42
store A
on_A_jsr0  r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16,r17,r18,r19,r20,r21,r22,r23,r24,r25,r26,r27,r28,r29,r30,r31,r32,r33,r34,r35,r36,r37,r38,r39,r40,r41,r42
restore A
next

ldax checksum
cmpax #903
	if ne
	  sec
	  rts
	endif

	clc
	rts



test_on_A_jmp:
on_A_jmp  r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16,r17,r18,r19,r20,r21,r22,r23,r24,r25,r26,r27,r28,r29,r30,r31,r32,r33,r34,r35,r36,r37,r38,r39,r40,r41,r42
rts

test_on_A_jmp0:
on_A_jmp0  r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16,r17,r18,r19,r20,r21,r22,r23,r24,r25,r26,r27,r28,r29,r30,r31,r32,r33,r34,r35,r36,r37,r38,r39,r40,r41,r42
rts

test_on_A_jmp0_nocheck:
on_A_jmp0_nocheck  r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16,r17,r18,r19,r20,r21,r22,r23,r24,r25,r26,r27,r28,r29,r30,r31,r32,r33,r34,r35,r36,r37,r38,r39,r40,r41,r42
rts

test_on_A_jmp_mul3:
on_A_jmp_mul3 r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16,r17,r18,r19,r20,r21,r22,r23,r24,r25,r26,r27,r28,r29,r30,r31,r32,r33,r34,r35,r36,r37,r38,r39,r40,r41,r42
rts

test_on_A_jmp_mul4:
on_A_jmp_mul4  r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15,r16,r17,r18,r19,r20,r21,r22,r23,r24,r25,r26,r27,r28,r29,r30,r31
rts

r0:
    inc16 checksum,0
    rts

r1:
    inc16 checksum,1
    rts

r2:
    inc16 checksum,2
    rts

r3:
    inc16 checksum,3
    rts

r4:
    inc16 checksum,4
    rts

r5:
    inc16 checksum,5
    rts

r6:
    inc16 checksum,6
    rts

r7:
    inc16 checksum,7
    rts

r8:
    inc16 checksum,8
    rts

r9:
    inc16 checksum,9
    rts

r10:
    inc16 checksum,10
    rts

r11:
    inc16 checksum,11
    rts

r12:
    inc16 checksum,12
    rts

r13:
    inc16 checksum,13
    rts

r14:
    inc16 checksum,14
    rts

r15:
    inc16 checksum,15
    rts

r16:
    inc16 checksum,16
    rts

r17:
    inc16 checksum,17
    rts

r18:
    inc16 checksum,18
    rts

r19:
    inc16 checksum,19
    rts

r20:
    inc16 checksum,20
    rts

r21:
    inc16 checksum,21
    rts

r22:
    inc16 checksum,22
    rts

r23:
    inc16 checksum,23
    rts

r24:
    inc16 checksum,24
    rts

r25:
    inc16 checksum,25
    rts

r26:
    inc16 checksum,26
    rts

r27:
    inc16 checksum,27
    rts

r28:
    inc16 checksum,28
    rts

r29:
    inc16 checksum,29
    rts

r30:
    inc16 checksum,30
    rts

r31:
    inc16 checksum,31
    rts

r32:
    inc16 checksum,32
    rts

r33:
    inc16 checksum,33
    rts

r34:
    inc16 checksum,34
    rts

r35:
    inc16 checksum,35
    rts

r36:
    inc16 checksum,36
    rts

r37:
    inc16 checksum,37
    rts

r38:
    inc16 checksum,38
    rts

r39:
    inc16 checksum,39
    rts

r40:
    inc16 checksum,40
    rts

r41:
    inc16 checksum,41
    rts

r42:
    inc16 checksum,42
    rts

.endproc