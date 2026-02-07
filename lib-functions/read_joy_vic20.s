.export _read_joy_vic20_sr

; ========================================
; Read VIC-20 Joystick - Self-Modifying Code
; Returns A with bits (0=pressed):
;   bit 0: up    (JOYREG1 bit 2)
;   bit 1: down  (JOYREG1 bit 3)
;   bit 2: left  (JOYREG1 bit 4)
;   bit 3: right (JOYREG2 bit 7)
;   bit 4: fire  (JOYREG1 bit 5)
;
; Uses self-modifying code to store intermediate values
; ========================================
; VIC-20 VIA registers
JOYREG1 = $9111
JOYREG2 = $9120
DDR2    = $9122

_read_joy_vic20_sr:
    lda #$7f
    sta DDR2
    lda JOYREG1
    lsr
    pha
    ora #%11101111      ; isolate bit 4 (Joystick fire)
    bit JOYREG2		; check bit 7 (Joystick right)
    bmi skip_bit3
    and #%11110111    ; reset bit 3 
skip_bit3:
    sta joy_bits
    lda #$ff
    sta DDR2
    pla
    lsr
    ; bits for up, down, left, right are now at the right position
    ora #%11111000
joy_bits=*+1
    and #$42            ; dummy value, will be overwritten
    and #%00011111
    rts
