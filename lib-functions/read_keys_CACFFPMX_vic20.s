; VIC20 version of read_keys_CACFFPMX
; Reads keys C=, ←, CRSR right, F7, F1, +, M, X
; Return value is in A, where the respective bit is zero if the key is pressed
; Bit mapping (matching C64 CACFFPMX output):
;   bit 0: C= (Commodore)
;   bit 1: ← (Arrow left)
;   bit 2: CRSR right
;   bit 3: F7
;   bit 4: F1
;   bit 5: +
;   bit 6: M
;   bit 7: X

.include "LAMAlib-macros16.inc"

.export _read_keys_CACFFPMX_vic20_sr

KB_COLS = $9120    ; Write to select keyboard columns
KB_ROWS = $9121    ; Read keyboard row state

_read_keys_CACFFPMX_vic20_sr:
	;=== LEFT-SHIFT GROUP (build high bits 7,6,1) ===
	
	; Read X (row bit 2, column 3 → output bit 7)
	poke KB_COLS,%11110111   ; 255-(1<<3)
	lda KB_ROWS
	and #%00000100           ; X at row bit 2
	asl                 
	asl                 
	asl                 
	sta bit_x                ; save X at bit 5
	
	; Read M (row bit 4, column 4 → output bit 6)
	poke KB_COLS,%11101111   ; 255-(1<<4)
	lda KB_ROWS
	and #%00010000           ; M at row bit 4
bit_x=*+1
	ora #$42                 ; combine with X at bit 5
	asl                   
	sta bits_xm              ; X at 7, M at 6
	
	; Read ← (row bit 0, column 1 → output bit 1)
	poke KB_COLS,%11111101   ; 255-(1<<1)
	lda KB_ROWS
	and #%00000001           ; ← at bit 0
bits_xm=*+1
	ora #$42                 ; combine with X at 7, M at 6
	asl
	sta left_result          ; save: X(7), M(6), ←(1)
	
	;=== NO-SHIFT GROUP (bits 0,5) ===
	
	; Read C= (row bit 0, column 5 → output bit 0)
	poke KB_COLS,%11011111   ; 255-(1<<5)
	lda KB_ROWS
	and #%00000001           ; C= at bit 0 (already correct!)
	sta bits_c               ; save C= at bit 0
	
	; Read + (row bit 5, column 0 → output bit 5)
	poke KB_COLS,%11111110   ; 255-(1<<0)
	lda KB_ROWS
	and #%00100000           ; + at bit 5 (already correct!)
bits_c=*+1
	ora #$42                 ; combine with C= at bit 0
	sta no_shift_result      ; C=(0), +(5)
	
	;=== RIGHT-SHIFT GROUP (build bits 2,3,4) ===
	
	; Read CRSR (row bit 7, column 2 → output bit 2)
	poke KB_COLS,%11111011   ; 255-(1<<2)
	lda KB_ROWS
	and #%10000000           ; CRSR at bit 7
	lsr               
	lsr               
	sta bits_crsr            ; CRSR at bit 2
	
	; Read F1 (row bit 7, column 4 → output bit 4)
	poke KB_COLS,%11101111   ; 255-(1<<4)
	lda KB_ROWS
	and #%10000000           ; F1 at bit 7
bits_crsr=*+1
	ora #$42                 ; combine with CRSR at bit 2
	sta bits_crsr_f1         ; CRSR(2), F1(4)
	
	; Read F7 (row bit 7, column 7 → output bit 3)
	poke KB_COLS,%01111111   ; 255-(1<<7) - read last, no restore needed!
	lda KB_ROWS
	and #%10000000           ; F7 at bit 7
	lsr                    
bits_crsr_f1=*+1
	ora #$42                 ; combine with CRSR(2), F1(4)
	lsr            
	lsr            
	lsr            
	
	;=== COMBINE ALL GROUPS ===
left_result=*+1
	ora #$42                 ; + X(7), M(6), ←(1)
no_shift_result=*+1
	ora #$42                 ; + C=(0), +(5)
	; Final: C=(0), ←(1), CRSR(2), F7(3), F1(4), +(5), M(6), X(7)
	; KB_COLS already at %01111111 (standard value)
	
	rts
