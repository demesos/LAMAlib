;--------------------------------------------------
; test program for LAMAlib functions
;
; testing inc8, inc16, dec8, dec16 macros

.include "LAMAlib.inc"

.if .not .definedmacro(jne)
.macpack longbranch      
.endif

.proc test_no_11
INC8_test=1
INC16_test=1
DEC8_test=1
DEC16_test=1


.ifdef INC8_test
	poke $400,$56
	inc8 $400
	lda #$57
	cmp $400
	jne exit_failure

	inc8 $400,5
	lda #$5c
	cmp $400
	jne exit_failure

	inc8 $400,$f5
	lda #$51
	cmp $400
	jne exit_failure

	ldax #$400
	inc8 ax
	lda #$52
	cmp $400
	jne exit_failure

	ldax #$400
	ldy #$fe
	inc8 ax,y
	lda #$50
	cmp $400
	jne exit_failure

	ldx #$f0
	inc8 $400,x
	lda #$40
	cmp $400
	jne exit_failure

	lda #$f2
	inc8 $400,a
	lda #$32
	cmp $400
	jne exit_failure
.endif

.ifdef INC16_test
	; test inc16

	pokew $400,$1234

	inc16 $400
	ldax $400
	cmpax #$1235
	jne exit_failure

	inc16 $400,5
	ldax $400
	cmpax #$123a
	jne exit_failure
	
	lda #6
	inc16 $400,A
	ldax $400
	cmpax #$1240
	jne exit_failure

	ldax #$10
	inc16 $400,A
	ldax $400
	cmpax #$1250
	jne exit_failure

	ldax #$400
	inc16 AX
	ldax $400
	cmpax #$1251
	jne exit_failure

	ldax #$400
	inc16 AX,$101
	ldax $400
	cmpax #$1352
	jne exit_failure

	ldax #$400
	inc16 AX,$f0
	ldax $400
	cmpax #$1442
	jne exit_failure

	inc16 $400,$be
	ldax $400
	cmpax #$1500
	jne exit_failure

	inc16 $400,$ebaf
	ldax $400
	cmpax #$af
	jne exit_failure

	ldax #$400
	ldy #$ff
	inc16 ax,y
	ldax #$1ae
	cmpax $400
	jne exit_failure

	ldax #$400
	inc16 ax,$ff
	ldax #$2ad
	cmpax $400
	jne exit_failure

	ldy #$ff
	inc16 $400,y
	ldax #$3ac
	cmpax $400
	jne exit_failure

	ldx #$ff
	inc16 $400,x
	ldax #$4ab
	cmpax $400
	jne exit_failure

	lda #$ff
	inc16 $400,a
	ldax #$5aa
	cmpax $400
	jne exit_failure
.endif

.ifdef DEC8_test
	poke $400,$56
	dec8 $400
	lda #$55
	cmp $400
	jne exit_failure

	dec8 $400,5
	lda #$50
	cmp $400
	jne exit_failure

	dec8 $400,$f5
	lda #$5b
	cmp $400
	jne exit_failure

	ldax #$400
	dec8 ax
	lda #$5a
	cmp $400
	jne exit_failure

	ldax #$400
	ldy #$fe
	dec8 ax,y
	lda #$5c
	cmp $400
	jne exit_failure

	ldx #$f0
	dec8 $400,x
	lda #$6c
	cmp $400
	jne exit_failure

	lda #$f2
	dec8 $400,a
	lda #$7a
	cmp $400
	jne exit_failure
.endif

.ifdef DEC16_test
	; test dec16

	pokew $400,$1234

	dec16 $400
	ldax $400
	cmpax #$1233
	jne exit_failure

	dec16 $400,5
	ldax $400
	cmpax #$122e
	jne exit_failure
	
	lda #$0a
	dec16 $400,A
	ldax $400
	cmpax #$1224
	jne exit_failure

	ldax #$10
	dec16 $400,AX
	ldax $400
	cmpax #$1214
	jne exit_failure

	ldax #$400
	dec16 AX
	ldax $400
	cmpax #$1213
	jne exit_failure

	ldax #$400
	dec16 AX,$101
	ldax $400
	cmpax #$1112
	jne exit_failure

	ldax #$400
	dec16 AX,$f0
	ldax $400
	cmpax #$1022
	jne exit_failure

	dec16 $400,$be
	ldax $400
	cmpax #$f64
	jne exit_failure

	dec16 $400,$ebaf
	ldax $400
	cmpax #$23b5
	jne exit_failure

	ldax #$400
	ldy #$ff
	dec16 ax,y
	ldax #$22b6
	cmpax $400
	jne exit_failure

	ldax #$400
	dec16 ax,$ff
	ldax #$21b7
	cmpax $400
	jne exit_failure

	ldy #$ff
	dec16 $400,y
	ldax #$20b8
	cmpax $400
	jne exit_failure

	ldx #$ff
	dec16 $400,x
	ldax #$1fb9
	cmpax $400
	jne exit_failure

	lda #$ff
	dec16 $400,a
	ldax #$1eba
	cmpax $400
	jne exit_failure
.endif


;print "ok"
	clc
	rts

exit_failure:
	sec
	rts
.endproc
