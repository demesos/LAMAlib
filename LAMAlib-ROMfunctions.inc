; Addresses of used ROM routines

.if .def(__C64__)

CHROUT= $FFD2 ;outputs the character in A
STROUT= $AB1E ;outputs a string at addr A/Y
LINPRT= $BDCD ;outputs number in A/X
CLRSCR= $E544 ;clear screen
INITVIC = $E5A0
INITVEC = $FD15   
BASICSTART=$0801
PTRSCRHI=$288 ;high byte of pointer to screen memory for screen input/output
CURSORX=$D3
CURSORY=$D6

.elseif .def(__C128__)

CHROUT= $FFD2 ;outputs the character in A
STROUT= $AB1E ;outputs a string at addr A/Y
LINPRT= $8E32 ;outputs number in A/X
CLRSCR= $C142 ;clear screen
INITVIC = $E170
INITVEC = $E056 
BASICSTART=$1C01
PTRSCRHI=$a3b ;high byte of pointer to screen memory for screen input/output
CURSORX=$EC
CURSORY=$EB


.elseif .def(__VIC20__)

CHROUT= $FFD2 ;outputs the character in A
STROUT= $CB1E ;outputs a string at addr A/Y
LINPRT= $DDCD ;outputs number in A/X
CLRSCR= $E55F ;clear screen
BASICSTART=$1001

.endif


