;--------------------------------------------------
; test program for LAMAlib functions
;
; testing sprite functions

.include "LAMAlib.inc"
.include "LAMAlib-sprites.inc"

.if .not .definedmacro(jne)
        .macpack longbranch
.endif

.proc test_no_15

SCREEN_BASE=$400
SPRITE_A=5
SPRITE_B=7

        memcopy sprite_data, $340,128

        setSpriteMultiColor1 1
        setSpriteMultiColor1 4

        setSpriteCostume SPRITE_A, $340
        setSpriteCostume SPRITE_B, 14

        updateSpriteAttributes SPRITE_A
        updateSpriteAttributes SPRITE_B

        setSpriteXY SPRITE_A,100,100
        setSpriteXY SPRITE_B,260,110

        lda $d01e       ;empty sprite-sprite-collision register

        showSprite SPRITE_A
        showSprite SPRITE_B

        for AX,100,to,340
            store AX
            ldy $d01e
            bne done
            setSpriteX SPRITE_A,AX
            sync_to_rasterline256
            restore AX
        next
done:
	ldy #0
	sty 53269	;sprites off
        cmpax #239
	bne exit_failure

        clc
        rts

exit_failure:
        sec
        rts

sprite_data:
        .byte %00000000,%10101010,%00000000
        .byte %00000010,%10101010,%10000000
        .byte %00001010,%10101010,%10100000
        .byte %00101010,%10101010,%10101000
        .byte %00100101,%01101001,%01011000
        .byte %00100110,%01101001,%10011000
        .byte %10100110,%10101010,%10011010
        .byte %10101010,%10101010,%10101010
        .byte %10101010,%10101010,%10101010
        .byte %10101010,%10101010,%10101010
        .byte %10101010,%10101010,%10101010
        .byte %10001010,%10101010,%10100010
        .byte %10001110,%10101010,%10110010
        .byte %10001100,%11000011,%00110010
        .byte %10100000,%11000011,%00001010
        .byte %10100000,%00000000,%00001010
        .byte %00101011,%00110000,%11101000
        .byte %00101010,%10101010,%10101000
        .byte %00001010,%10101010,%10100000
        .byte %00001010,%10101010,%10100000
        .byte %00000010,%10101010,%10000000
        .byte $95

        .byte %00000011,%11111111,%10000000
        .byte %00001111,%11111111,%11100000
        .byte %00011111,%11111111,%11111000
        .byte %00111111,%10011111,%11111100
        .byte %01111111,%00001111,%11111100
        .byte %01111111,%10011111,%11110000
        .byte %11111111,%11111111,%11100000
        .byte %11111111,%11111111,%10000000
        .byte %11111111,%11111110,%00000000
        .byte %11111111,%11111000,%00000000
        .byte %11111111,%11100000,%00000000
        .byte %11111111,%11111000,%00000000
        .byte %11111111,%11111110,%00000000
        .byte %11111111,%11111111,%10000000
        .byte %11111111,%11111111,%11100000
        .byte %01111111,%11111111,%11110000
        .byte %01111111,%11111111,%11111100
        .byte %00111111,%11111111,%11111100
        .byte %00011111,%11111111,%11111000
        .byte %00001111,%11111111,%11100000
        .byte %00000011,%11111111,%10000000
        .byte $07

.endproc