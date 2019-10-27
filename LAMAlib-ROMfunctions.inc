; Addresses of used ROM routines

.if .def(__C64__)

CHROUT= $FFD2 ;outputs the character in A
STROUT= $AB1E ;outputs a string at addr A/Y
LINPRT= $BDCD ;outputs number in A/X
CLRSCR= $E544 ;clear screen

.elseif .def(__C128__)

CHROUT= $FFD2 ;outputs the character in A
STROUT= $AB1E ;outputs a string at addr A/Y
LINPRT= $8E32 ;outputs number in A/X
CLRSCR= $C142 ;clear screen

.elseif .def(__VIC20__)

CHROUT= $FFD2 ;outputs the character in A
STROUT= $CB1E ;outputs a string at addr A/Y
LINPRT= $DDCD ;outputs number in A/X
CLRSCR= $E55F ;clear screen

.endif


