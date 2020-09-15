; Addresses of used ROM routines

.if .def(__C64__)

STROUT= $AB1E ;outputs a string at addr A/Y
LINPRT= $BDCD ;outputs number in A/X

CURSORX=$D3
CURSORY=$D6

CHROUT= $FFD2 ;outputs the character in A
GETIN = $FFE4
CLRSCR= $E544 ;clear screen
INITVIC = $E5A0
INITVEC = $FD15   
BASICSTART=$0801
PTRSCRHI=$288 ;high byte of pointer to screen memory for screen input/output

CIABASE=$DD00

TIMERA= CIABASE+4
TIMERB= CIABASE+6

ICR= CIABASE+$D ;Interrupt control reg

CRA= CIABASE+$E ;Control Timer A
CRB= CIABASE+$F ;Control Timer B

.elseif .def(__C128__)

STROUT= $AB1E ;outputs a string at addr A/Y
LINPRT= $8E32 ;outputs number in A/X

CHROUT= $FFD2 ;outputs the character in A
GETIN = $FFE4
CLRSCR= $C142 ;clear screen
INITVIC = $E170
INITVEC = $E056 
BASICSTART=$1C01
PTRSCRHI=$a3b ;high byte of pointer to screen memory for screen input/output
CURSORX=$EC
CURSORY=$EB

CIABASE=$DD00

TIMERA= CIABASE+4
TIMERB= CIABASE+6

ICR= CIABASE+$D ;Interrupt control reg

CRA= CIABASE+$E ;Control Timer A
CRB= CIABASE+$F ;Control Timer B

.elseif .def(__VIC20__)

STROUT= $CB1E ;outputs a string at addr A/Y
LINPRT= $DDCD ;outputs number in A/X

CHROUT= $FFD2 ;outputs the character in A
GETIN = $FFE4
CLRSCR= $E55F ;clear screen
BASICSTART=$1001

VIABASE=$9110

TIMERAlatch= VIABASE+4
TIMERAcounter = VIABASE+6
TIMERBlatch= VIABASE+8

ICR= CIABASE+$D ;Interrupt control reg

CRA= CIABASE+$E ;Control Timer A
CRB= CIABASE+$F ;Control Timer B

.endif


