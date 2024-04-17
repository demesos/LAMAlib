;; <h2>String Routines</h2>

;; strlen8 address
;; strlen8 AX
;; Returns the length of the null-terminated string at address given by the argument or by AX
;; String maximum length is 255, use strlen16 to handle longer strings
;; Return value in X
.macro strlen8 arg
.scope
	.if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))          ;AX is used as address
	  stax addr
	  ldx #0
	search0:
	addr=*+1
	  lda $affe,x
	.else
	  ldx #0
	search0:
	  lda arg,x	
	.endif
	beq found0
	inx
	bne search0
found0:	
.endscope
.endmacro

;; strlen16 address
;; strlen16 AX
;; Returns the length of the null-terminated string at address given by the argument or by AX
;; Return value in AX
.macro strlen16 arg
.scope
	.if (.xmatch ({arg}, {AX}) .or .xmatch ({arg}, {ax}))          ;AX is used as address
	  stax addr
	  ldx #0
	  stx highbyte
	search0:
	addr=*+1
	  lda $affe,x
	.else
	  ldx #0
	  stx highbyte
	search0:
	addr=*+1
	  lda arg,x	
	.endif
	beq found0
	inx
	bne search0
	inc addr+1
	inc highbyte
	bne search0
found0:
	txa
highbyte=*+1
	ldx #0	
.endscope
.endmacro