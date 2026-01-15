;***********************************************************************
;* Minimal Mouse-Controlled Sprite Demo
;* 
;* A simple demonstration of the mousedriver module.
;* The module handles everything - sprite setup, mouse reading,
;* and sprite positioning.
;***********************************************************************

.include "LAMAlib.inc"
.include "LAMAlib-sprites.inc"

; Define screen base for sprite system
SCREEN_BASE = $0400

; Jump over module code to main program
jmp over_it

;***********************************************************************
;* Include the mousedriver module
;* Explicitly set the sprite number before inclusion
;***********************************************************************
.scope mousedriver
    SPR_NUM=0               ; Will use sprites 0 and 1
    .include "modules/m_mousedriver.s"
.endscope

;***********************************************************************
;* Main program
;***********************************************************************
.code

over_it:
    clrscr
    poke $d020, 0           ; Black border
    poke $d021, 0           ; Black background
    
    ; Initialize the mouse driver
    m_init mousedriver
    
    ; Main loop - just call m_run every frame
loop:
    m_run mousedriver
    jmp loop

