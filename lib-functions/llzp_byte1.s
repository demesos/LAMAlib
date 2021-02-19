; this module just reserves a byte in the zeropage segment to be temporally used by some LAMAlib functions
; the memory is only used if it is claimed by a function

.exportzp _llzp_byte1

.segment "ZEROPAGE"
_llzp_byte1: .res 1
.code
