;--------------------------------------------------
; testing the include 
;--------------------------------------------------

.include "LAMAlib.inc"

.ifdef makecode
	;generating the reference values
	checksum_eor $a000,$a07e
	sta $400
	checksum_eor $a000,$a07f
	sta $401
	checksum_eor $a000,$a080
	sta $402
	checksum_eor $a000,$a0fe
	sta $403
	checksum_eor $a000,$a0ff
	sta $404
	checksum_eor $a000,$a100
	sta $405
	checksum_eor $a000,$a1fe
	sta $406
	checksum_eor $a000,$a1ff
	sta $407
	checksum_eor $a000,$a200
	sta $408
	checksum_eor $a000,$affe
	sta $409
	rts
.endif

.proc test_no_7
	lda $1
	pha
	sei
	poke 1,$34

	jmp skip

include_file_as "test7files/0x100 B.prg", block100
include_file_as "test7files/0x101 B.prg", block101
include_file_as "test7files/0x1ff B.prg", block1ff
include_file_as "test7files/0x200 B.prg", block200
include_file_as "test7files/0x201 B.prg", block201
include_file_as "test7files/0x7f B.prg", block7f
include_file_as "test7files/0x80 B.prg", block80
include_file_as "test7files/0x81 B.prg", block81
include_file_as "test7files/0xff B.prg", blockff
include_file_as "test7files/0xfff B.prg", blockfff	

include_file_as "test7files/0x100 B.bin", binblock100
include_file_as "test7files/0x101 B.bin", binblock101
include_file_as "test7files/0x1ff B.bin", binblock1ff
include_file_as "test7files/0x200 B.bin", binblock200
include_file_as "test7files/0x201 B.bin", binblock201
include_file_as "test7files/0x7f B.bin", binblock7f
include_file_as "test7files/0x80 B.bin", binblock80
include_file_as "test7files/0x81 B.bin", binblock81
include_file_as "test7files/0xff B.bin", binblockff
include_file_as "test7files/0xfff B.bin", binblockfff	

skip:
	;install_prgfile sprites

	memset $9fff,$b000,0
	install_prgfile block100
	checksum_eor $9fff,$b000

	cmp #$80
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	sta $400

	memset $9fff,$b000,0
	install_prgfile block101
	checksum_eor $9fff,$b000
	sta $401

	cmp #$c9
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $9fff,$b000,0
	install_prgfile block1ff
	checksum_eor $9fff,$b000
	sta $402

	cmp #$01
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $9fff,$b000,0
	install_prgfile block200
	checksum_eor $9fff,$b000
	sta $403

	cmp #$4c
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $9fff,$b000,0
	install_prgfile block201
	checksum_eor $9fff,$b000
	sta $404

	cmp #$05
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $9fff,$b000,0
	install_prgfile block7f
	checksum_eor $9fff,$b000
	sta $405

	cmp #$78
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $9fff,$b000,0
	install_prgfile block80
	checksum_eor $9fff,$b000
	sta $406

	cmp #$cf
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $9fff,$b000,0
	install_prgfile block81
	checksum_eor $9fff,$b000
	sta $407

	cmp #$b6
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $9fff,$b000,0
	install_prgfile blockff
	checksum_eor $9fff,$b000
	sta $408

	cmp #$d2
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $9fff,$b000,0
	install_prgfile blockfff
	checksum_eor $9fff,$b000
	sta $409

	cmp #$50
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	;second round with specified target address
	memset $a57f,$b600,0
	install_prgfile block100,$a580
	checksum_eor $a57f,$b600

	cmp #$80
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	sta $400

	memset $a57f,$b600,0
	install_prgfile block101,$a580
	checksum_eor $a57f,$b600
	sta $401

	cmp #$c9
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile block1ff,$a580
	checksum_eor $a57f,$b600
	sta $402

	cmp #$01
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile block200,$a580
	checksum_eor $a57f,$b600
	sta $403

	cmp #$4c
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile block201,$a580
	checksum_eor $a57f,$b600
	sta $404

	cmp #$05
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile block7f,$a580
	checksum_eor $a57f,$b600
	sta $405

	cmp #$78
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile block80,$a580
	checksum_eor $a57f,$b600
	sta $406

	cmp #$cf
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile block81,$a580
	checksum_eor $a57f,$b600
	sta $407

	cmp #$b6
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile blockff,$a580
	checksum_eor $a57f,$b600
	sta $408

	cmp #$d2
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile blockfff,$a580
	checksum_eor $a57f,$b600
	sta $409

	cmp #$50
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile binblock100,$a580
	checksum_eor $a57f,$b600

	cmp #$80
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	sta $400

	memset $a57f,$b600,0
	install_prgfile binblock101,$a580
	checksum_eor $a57f,$b600
	sta $401

	cmp #$c9
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile binblock1ff,$a580
	checksum_eor $a57f,$b600
	sta $402

	cmp #$01
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile binblock200,$a580
	checksum_eor $a57f,$b600
	sta $403

	cmp #$4c
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile binblock201,$a580
	checksum_eor $a57f,$b600
	sta $404

	cmp #$05
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile binblock7f,$a580
	checksum_eor $a57f,$b600
	sta $405

	cmp #$78
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif


	memset $a57f,$b600,0
	install_prgfile binblock80,$a580
	checksum_eor $a57f,$b600
	sta $406

	cmp #$cf
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile binblock81,$a580
	checksum_eor $a57f,$b600
	sta $407

	cmp #$b6
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile binblockff,$a580
	checksum_eor $a57f,$b600
	sta $408

	cmp #$d2
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

	memset $a57f,$b600,0
	install_prgfile binblockfff,$a580
	checksum_eor $a57f,$b600
	sta $409

	cmp #$50
	if eq
	  clc
	else
	  sec
	  jmp exit
	endif

exit:
	pla
	sta 1
	if cs
	  poke $d020,2
	endif
	cli
	rts
.endproc


