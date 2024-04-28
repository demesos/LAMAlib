;; <h2>Interacting with BASIC</h2>

;; setBasicVarnam "varnam"
;; This macro puts name and type of current variable into $45/46 (on C128 $47/48) in preparation for using ROM routines
;; The values of $45 and $46 are also put into registers A and X
;; The varname can be of 1 or 2 characters length plus an optional $ or % to indicate string or integer variables
;; Example usage: 
;; setBasicVarnam "ab%" sets the integer variable ab%
;; setBasicVarnam "s$" sets the string variable s$
;; setBasicVarnam "x1" sets the floating point variable x1
.macro setBasicVarnam varnam
	.scope
	vartype=.strat (varnam, .strlen(varnam)-1)

	.if (vartype = '%')
	  varflag1=128
	  varflag2=128
	  xtralen=1
	.elseif (vartype = '$')
	  varflag1=0
	  varflag2=128
	  xtralen=1
	.else
	  varflag1=0
	  varflag2=0
	  xtralen=0
	.endif

	lda #(.strat(varnam,0)+varflag1)
	sta CURR_VAR_NAME

	.if .strlen(varnam) > 1+xtralen
          ldx #(.strat(varnam,1)+varflag2)
	.else
	  ldx #varflag2
	.endif
	stx CURR_VAR_NAME+1
	.endscope
.endmacro

;; getAddrOfBasicArryVar varnam,arrayidx
;; The arrayidx must be not larger than decimal 10 for arrays to be created
;; Example usage: 
;; getAddrOfBasicArryVar "ab%",3 returns the address of AB%(3)
;; The return value is the variables address in AX
;; uses zero page addresses $0b, $0c, $0d, $0e, $45, $46
;; on C128 it uses zero page addresses $0d, $0e, $0f, $10, $47, $48
;; On the C128, the bank will be set to 1 after return, so this code needs to be in common RAM or in bank 1.
.macro getAddrOfBasicArryVar varnam,arrayidx
.import _getArrayElementAddr_sr
	setBasicVarnam varnam
	.if .not .xmatch({arrayidx},{AX})
	  lda #>(arrayidx)
	  ldx #<(arrayidx)	;lo/hi values are switched here!!
	.else
	  .scope
	    sta load_x
	    txa			;hi value into A
load_x=*+1
	    ldx #$af		;lo value into X
	  .endscope
	.endif
	jsr _getArrayElementAddr_sr
	.scope
	sty load_x
load_x=*+1
	ldx #$af		;ho value from y into X
	.endscope
.endmacro
