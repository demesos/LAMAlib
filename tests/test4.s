; test rsb and rsbax

.include "LAMAlib.inc"

.proc test_no_4

	pokew sum,0

	for a,0,to,150,5
	  store a
	  rsb #99
	  clc
	  ldx #00
	  adcax sum
	  stax sum
	  restore a
	next

	ldax sum
	cmpax #3560
	if ne
	  sec
	  rts
	endif
	pokew sum,0

	for ax,0,to,2000,51
	  store ax
	  rsbax #999
	  addax sum
	  stax sum
	  restore ax
	next

	;sum is now 180
	ldax sum
	for Y,1,to,18
	  subax #10
	next
	;ax is now 0
	for Y,1,to,10
	  addax #18
	next
	;ax is now 180
	cmpax #180
	if eq
	  clc
	else
	  sec
	endif
	rts


sum:
.byte 00,00

.endproc



; 1 forax=0to50000step51:r=999-ax:c=0
; 2 ifr<0thenr=r+65536:c=1
; 3 s=s+r+c:next
; 4 ifs>65536thens=s-65536:goto4
; 5 prints



; 1 forax=0to2000step51:r=999-ax:c=0
; 2 ifr<0thenr=r+65536:c=1
; 3 s=s+r+c
; 4 ifs>65536thens=s-65536
; 5 printr;s;",";:next
; 7 prints


































