; locates the descriptor for a variable specified at $46-$46
; before calling, the variable name and its flags need to be set in $45/$46

;.include "../LAMAlib-ROMfunctions.inc"

.export _getArrayElementAddr_sr

.import FINDARRAYELEMENT
.importzp VAR_FLAGS, CURR_VAR_NAME

.proc _getArrayElementAddr_sr
	;push array index
	pha
	txa
	pha

	ldy #00
	sty VAR_FLAGS+1		;we don't have the DIM case
	lda CURR_VAR_NAME
	and #$80
	sta VAR_FLAGS+3		;$80 for integer vars, 0 otherwise
	bmi skip
	bit CURR_VAR_NAME+1
	bpl skip
	dey
skip:
	sty VAR_FLAGS+2		;0=numeric variable, ff=string
	lda #1
	sta VAR_FLAGS		;1 dimension
	
	jmp FINDARRAYELEMENT	;this ROM function searches for the variable and creates it if necessary
.endproc