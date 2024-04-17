;--------------------------------------------------
; test program for LAMAlib functions
;
; test A_between, div8
;

.include "LAMAlib.inc"

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
          bne exit_failure
        restore A
        next

	lda #15
	div8 #3
	cmp #5
	bne exit_failure
	txa
	bne exit_failure

	lda #255
	div8 #6
	cmp #42
	bne exit_failure
	cpx #3
	bne exit_failure

	ldax #2555
	div16by8 #12
	cmp #212
	bne exit_failure
	cpx #11
	bne exit_failure

        clc
        rts

exit_failure:
	sec
	rts
.endproc

; .proc test_no_10_extensive
; 	.repeat 21,I
; 	.repeat 21,J
; 	  .if I <= J
; 	.scope
; 	    for A,0,to,255
; 	    store A
; 	      ref_A_between I*11,J*11
; 	      rol res
;  	      A_between I*11,J*11
; 	      rol res
;               res=*+1
; 	      lda #$af
; 	      and #3
; 	      beq ok
;               cmp #3
; 	      beq ok
;               jmp exit_failure
;               ok:
; 	      inc $d020
; 	    restore A
; 	    next
; 	.endscope
; 	  .endif
; 	.endrep
; 	.endrep
; 
; 	.repeat 11,I
; 	.repeat 11,J
; 	  .if I <= J
; 	.scope
; 	    for A,0,to,255
; 	    store A
; 	      ldx#0
; 	      ref_AX_between I*5031,J*5031
; 	      rol res
;  	      AX_between I*5031,J*5031
; 	      rol res
;               res=*+1
; 	      lda #$af
; 	      and #3
; 	      beq ok
;               cmp #3
; 	      beq ok
;               jmp exit_failure
;               ok:
; 	      inc $d021
; 	    restore A
; 	    next
; 	.endscope
; 	  .endif
; 	.endrep
; 	.endrep
; 	rts
; 
; exit_failure:
; 	sec
; 	rts
; 	
; .endproc