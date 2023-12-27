;--------------------------------------------------
; test program for LAMAlib functions
;
; test the random number generator
; this is a probabilistic test if among 1000 rolls all numbers between 0 and 28 are hit
; there is a chance of 3.45e-15 that this will fail naturally, but if this test fails
; a problem with the random number generator is much more likely

.include "LAMAlib.inc"

.proc test_no_3
.define fieldsize 29

	memset testmem,testmem+fieldsize-1,$00

	for ax,0,to,1000
	  store ax
	  rand8 fieldsize
	  tay
	  sta testmem,y
	  restore ax
	next

	checksum_eor testmem,testmem+fieldsize-1

	cmp #28
	if ne
	  sec
	  rts
	endif

	memset testmem,testmem+fieldsize-1,$00
	for ax,0,to,1000
	  store ax
	  rand16 16*fieldsize
	  lsrax
	  lsrax
	  lsrax
	  lsrax
	  tay
	  sta testmem,y
	  restore ax
	next

	checksum_eor testmem,testmem+fieldsize-1

	cmp #28
	if eq
	  clc
	else
	  sec
	endif
	rts


testmem:
.res fieldsize, $aa

.endproc

; https://math.stackexchange.com/questions/123117/how-do-you-calculate-probability-of-rolling-all-faces-of-a-die-after-n-number-of

; BASIC program to calculate probability
; 
; 10 rem rolling an s-sided dice nn times
; 20 s=29:nn=1000:su=0:v=1
; 30 forj=1tos-1:n=6:k=j:gosub200
; 40 su=su+v*nk*((s-j)/s)^nn
; 50 v=-v
; 60 next
; 70 print"p(failure)="su:end
; 99 rem factorial
; 100 a=1:fori=2tox:a=a*x:next:return
; 199 rem n over k
; 200 nk=1:if2*k<nthenk=n-k
; 210 fori=1ton-k
; 220 nk=nk*(i+k)/i
; 230 next
; 240 a=nk:return




