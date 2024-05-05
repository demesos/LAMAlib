;--------------------------------------------------
; test program for LAMAlib functions
;
; test A_between, division tests
;

.include "LAMAlib.inc"

.if .not .definedmacro(jne)
.macpack longbranch      
.endif

.macro ref_A_between lower,higher
.scope
	cmp #lower
	bcc outside
	cmp #higher+1
	.byte $24	;BIT for skipping

outside: sec
.endscope
.endmacro

.macro ref_AX_between lower,higher
.scope
	cmpax #lower
	bcc outside
	cmpax #higher+1
	.byte $24	;BIT for skipping

outside: sec
.endscope
.endmacro

.proc test_no_10
        for A,0,to,255
        store A
          ldy #0
          sty res1
          sty res2
          ref_A_between 11,22
          rol res1
          A_between 11,22
          rol res2
          ref_A_between 75,85
          rol res1
          A_between 75,85
          rol res2
          ref_A_between 240,250
          rol res1
          A_between 240,250
          rol res2
        res1=*+1
          lda #$af
        res2=*+1
          cmp #$fe
          jne exit_failure
        restore A
        next

	lda #15
	div8 #3
	cmp #5
	jne exit_failure
	txa
	jne exit_failure

	lda #255
	div8 #6
	cmp #42
	jne exit_failure
	cpx #3
	jne exit_failure

	ldax #2555
	div16by8 #12
	cmp #212
	jne exit_failure
	cpx #11
	jne exit_failure

	ldax #389
	stax valuea
	ldax #29953
	div16 valuea
	cmpax #77
	bne exit_failure

	ldax #29995
	mod16 #389
	cmpax #42
	bne exit_failure

	pokew sum,0
	for ax,0,to,65400,100
	  store AX
	  sqrt16
	  clc
	  adc sum
	  sta sum
	  if cs
	    inc sum+1
	  endif
	  restore AX
	next
	ldax sum
	cmpax #45764
	bne exit_failure

        clc
        rts

exit_failure:
	sec
	rts

valuea:
	.res 2

sum:
	.byte 00,00
.endproc
