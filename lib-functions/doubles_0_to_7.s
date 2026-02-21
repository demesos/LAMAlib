; this module provides a table with the potentials of two
; this table is often needed and therefore provided centrally here

.export _doubles_0_to_7

_doubles_0_to_7:
.repeat 8,i
    .byte 2*i
.endrep
