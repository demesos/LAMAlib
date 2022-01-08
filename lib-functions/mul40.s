.export _mul40_tbl_lo,_mul40_tbl_hi

_mul40_tbl_lo:
	.repeat 25,I
	.byte <(I*40)
	.endrep

_mul40_tbl_hi:
	.repeat 25,I
	.byte >(I*40)
	.endrep