; this module just reserves a word in the zeropage segment to be temporally used by some LAMAlib functions
; the memory is only used if it is claimed by a function

.exportzp _llzp_word1

.segment "ZEROPAGE"
_llzp_word1: .res 2
.code
