;***********************************************************************
;* Module: bigcharout
;* Version 0.2
;* 
;* Displays characters as 8x8 pixel patterns on the C64 screen by hooking
;* the CHROUT vector. Each character is rendered using the charset data
;* referred to in the module configuration, with each pixel represented 
;* by a screen character (default: space ;* for 0, reverse space for 1).
;*
;* TECHNICAL DETAILS:
;* - Hooks KERNAL CHROUT vector ($0326/7) to intercept character output
;* - Reads 8 bytes of character definition from specified charset
;* - Each bit in the 8x8 pattern becomes a screen character (space/reverse)
;* - Automatically detects current screen base address
;* - Optional color support synchronized with character output
;* - Optional end-of-screen wraparound handling
;*
;* CONFIGURATION:
;* 
;* .scope bigcharout
;*   CHARSET_BASE=$3800        ; Address of character set data (default: $3800)
;*   SCREEN_WIDTH=40           ; Characters per screen row (default: 40)
;*   SET_PIXEL=160             ; Screencode for "on" pixels (default: 160=reverse space)
;*   EMPTY_PIXEL=32            ; Screencode for "off" pixels (default: 32=space)
;*   COLOR_SUPPORT=1           ; Enable color RAM synchronization (default: 1)
;*   END_OF_SCREEN_CHECK=1     ; Enable screen wraparound (default: 1)
;*   .include "m_bigcharout.s"
;* .endscope
;*
;* USAGE FROM MAIN PROGRAM:
;*
;* m_init bigcharout           ; Install CHROUT hook
;* m_call bigcharout,gotoxy    ; Set cursor position (X=col, Y=row in registers)
;* print "hallo"               ; Draw characters as 8x8 pattern
;* m_call bigcharout,uninstall ; Restore original CHROUT vector
;*
;* You can create multiple instances for different effects:
;*
;* .scope title_chars
;*   SET_PIXEL=160
;*   EMPTY_PIXEL=32
;*   .include "m_bigcharout.s"
;* .endscope
;*
;* .scope game_chars
;*   SET_PIXEL=81              ; Hearts for game display
;*   EMPTY_PIXEL=32
;*   .include "m_bigcharout.s"
;* .endscope
;*
;* ; Use title_chars for title screen, game_chars during gameplay
;* m_init title_chars
;* ; ... title screen code ...
;* m_call title_chars,uninstall
;*
;* m_init game_chars
;* ; ... game code ...
;* m_call game_chars,uninstall
;*
;* LIMITATIONS:
;* - Only one instance can be active (CHROUT hooked) at a time
;* - Cursor positioning is in character coordinates (0..39, 0..24)
;* - No clipping at screen edges (except wraparound if enabled)
;* - No support for proportional spacing or variable character sizes

.include "LAMAlib.inc"

;***********************************************************************
;* parameters - can be overwritten from main

def_const CHARSET_BASE,$3800
def_const SCREEN_WIDTH,40
def_const SET_PIXEL,160		;character to be used when a pixel is set, as screencode
def_const EMPTY_PIXEL,32	;character to be used when a pixel is empty, as screencode
def_const COLOR_SUPPORT,1
def_const END_OF_SCREEN_CHECK,1

;***********************************************************************
;* module implementation

init:
install:
	; Save original CHROUT vector from $0326/7
	lda $326
	sta chrout_old_lo
	lda $327
	sta chrout_old_hi
	
	; Install our run routine as new CHROUT handler
	pokew $326,run

	; Initialize screen_ptr with current screen base address
	; PTRSCRHI ($0288) contains the high byte of screen memory
	; We preserve the low 2 bits of screen_ptr+1
	; and combine with the screen base high byte
	lda screen_ptr+1
	and #3
	ora PTRSCRHI
	sta screen_ptr+1	

	rts

;=======================================================================
; Restore original CHROUT vector
; Call this before exiting program or switching to different output mode
uninstall:
chrout_old_lo=*+1
	lda #$ca		; Self-modifying: filled by init with original low byte
	sta $326
chrout_old_hi=*+1
	lda #$f1		; Self-modifying: filled by init with original high byte
	sta $327
	rts

;=======================================================================
; Main character rendering routine
; Input: A = PETSCII character to render as 8x8 pattern
; Uses: screen_ptr to track current screen position
;       colram_ptr to track current color RAM position (if COLOR_SUPPORT enabled)
; Side effects: Advances cursor 8 characters to the right
run:
	; Preserve all registers (required for CHROUT compatibility)
	store A
	store X
	store Y

	; Convert PETSCII to screencode (handles PETSCII codes $00-$FF)
	to_scrcode
	
	; Calculate address of character definition in charset
	; Each character is 8 bytes, so multiply by 8 (3 left shifts)
	ldx #0
	aslax		; AX = char * 2
	aslax		; AX = char * 4
	aslax		; AX = char * 8
	adcax #CHARSET_BASE
	stax src	; Store in self-modifying code pointer

	; Loop through 8 rows of character definition
	ldx #0
nextrow:
	; Loop through 8 pixels (bits) in current row
	ldy #7
src=*+1
	lda $1234,x	; Self-modifying: get character row from charset
	sta charrow	; Buffer the row byte for bit testing
	
printrow:
	; Test rightmost bit and select appropriate screencode
	lsr charrow	; Shift right, bit 0 -> carry
	if cc		; If carry clear (bit was 0)
	  lda #EMPTY_PIXEL
	else		; If carry set (bit was 1)
	  lda #SET_PIXEL
	endif
	
screen_ptr=*+1
	sta $400,y	; Self-modifying: write to screen memory

.if COLOR_SUPPORT
	; Update corresponding color RAM byte
	lda TEXTCOLOR_ADDR	; Current text color from KERNAL variable
colram_ptr=*+1
	sta $d800,y		; Self-modifying: write to color RAM
.endif

	dey
	bpl printrow		; Continue for all 8 pixels in row

	; Move to next screen row (advance by SCREEN_WIDTH)
	lda screen_ptr
	clc
	adc #SCREEN_WIDTH
	sta screen_ptr
.if COLOR_SUPPORT
	sta colram_ptr		; Keep color pointer synchronized
.endif
	if cs			; If addition carried
	  inc screen_ptr+1
.if COLOR_SUPPORT
	  inc colram_ptr+1
.endif
	endif

	inx
	cpx #8
	bcc nextrow		; Continue for all 8 rows

	; Move cursor to position for next character
	; We're currently at bottom-left of character (+8 rows from start)
	; Move back up 8 rows and right 8 columns: subtract (8*SCREEN_WIDTH-8)
	lda screen_ptr
	sec
	sbc #<(8*SCREEN_WIDTH-8)
	sta screen_ptr
.if COLOR_SUPPORT
	sta colram_ptr
.endif
	lda screen_ptr+1
	sbc #>(8*SCREEN_WIDTH-8)
	sta screen_ptr+1
.if COLOR_SUPPORT
	; Restore color RAM high byte (always $D8xx)
	and #3			; Keep only low 2 bits (sub-page offset)
	ora #$d8		; Combine with color RAM base
	sta colram_ptr+1
.endif

.if END_OF_SCREEN_CHECK
	; Check if we've gone past end of screen and wrap if needed
	; Maximum valid address is (screen_base + 1000 - (8*SCREEN_WIDTH-8))
	max_addr=1000 - (7*SCREEN_WIDTH+8)+1

  .if COLOR_SUPPORT
	; If we have colram_ptr, extract screen high byte from it
	eor #$d8		; Remove color RAM base, leaving screen offset
  .else
	; Otherwise compare directly with screen base
	eor PTRSCRHI
  .endif
	cmp #>max_addr
	bcc addr_ok		; If high byte is less, we're definitely OK
	if eq			; If high byte matches, check low byte
	  lda screen_ptr
	  cmp #<max_addr
	  bcc addr_ok		; If low byte is less, we're OK
	endif
	
	; We've exceeded screen bounds - wrap to screen start
	lda #0
	sta screen_ptr
.if COLOR_SUPPORT
	sta colram_ptr
	lda #$d8
	sta colram_ptr+1
.endif
	lda PTRSCRHI
	sta screen_ptr+1
	  
addr_ok:
.endif

	; Restore registers and return with carry clear (success)
	clc
	restore A
	restore X
	restore Y
	rts

charrow: .byte 0	; Temporary storage for current character row

;=======================================================================
; Set cursor position for next character output
; Input: X = column (0-39 for standard screen)
;        Y = row (0-24 for standard screen)
; Note: Positions are in character coordinates, not pixels
;       Character will be drawn in an 8x8 area starting at this position
gotoxy:
.import _mul40_tbl_lo,_mul40_tbl_hi	; LAMAlib lookup tables for row offsets
	; Calculate screen address = screen_base + (Y * 40) + X
	txa
	clc
	adc _mul40_tbl_lo,y	; Add column to row offset (low byte)
	sta screen_ptr
	lda PTRSCRHI		; Screen base high byte
	adc _mul40_tbl_hi,y	; Add row offset high byte
	sta screen_ptr+1

.if COLOR_SUPPORT
	; Set up color RAM pointer to match screen position
	; Color RAM is always at $D800-$DBFF
	lda screen_ptr
	sta colram_ptr
	lda screen_ptr+1
	and #3			; Keep low 2 bits of offset
	ora #$d8		; Combine with color RAM base
	sta colram_ptr+1
.endif
	rts
	
