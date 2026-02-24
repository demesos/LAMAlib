;; <h2>Hires Graphics Functions - CURRENTLY UNDER DEVELOPMENT</h2>

;; blank_screen
;; Waits until rasterbar is below screen area, then blanks the screen and shows only the border color
;; Registers modified: A
.macro blank_screen
.scope
  .if ::SAVE_REGS>1
  store A
  .endif
:	lda $D011
	bpl :-      ;wait until rasterbar is below screen area
	and #%11101111 ;bit 4 off = Screen blanked
	sta $D011 
  .if ::SAVE_REGS>1
  restore A
  .endif
.endscope
.endmacro

;; unblank_screen
;; Shows the screen again after it was blanked, effective with next frame
;; Registers modified: A
.macro unblank_screen
  .if ::SAVE_REGS>1
  store A
  .endif
	lda $D011   ;bit 4 on = Screen visible
	ora #%00010000
	sta $D011 
  .if ::SAVE_REGS>1
  restore A
  .endif
.endmacro


;; bitmap_on
;; Turn bitmap mode on and initialize tables in case the project uses plotting commands (plot, line, circle, ...)
;; Registers modified: A
.import gfx_inittables_sr
.macro bitmap_on
  .if ::SAVE_REGS>1
  store A
  .endif
	lda #59   ;bit 5 on = Bitmap-Modus
	sta $D011 
	lda $D018
	ora #8
	sta $D018
  .if ::SAVE_REGS>1
  restore A
  .endif
.endmacro

;; bitmap_off
;; Turn bitmap mode off
;; Registers modified: A
.macro bitmap_off
  .if ::SAVE_REGS>1
  store A
  .endif
	lda #27   ;standard text mode
	sta $D011 
	lda $D018
	and #255-8
	sta $D018 
  .if ::SAVE_REGS>1
  restore A
  .endif
.endmacro

;; gfx_clrscr bgcolor,fgcolor
;; Clear graphics screen and set background and foreground colors
;; Registers modified: A, X, Y
.import _gfx_clrscr_sr

.macro gfx_clrscr bgcolor,fgcolor
.scope
  .if ::SAVE_REGS>1
  store AX
  store Y
  .endif
	ldy #bgcolor+$10*fgcolor
	jsr _gfx_clrscr_sr
  .if ::SAVE_REGS>1
  restore AX
  restore Y
  .endif
.endscope
.endmacro

;; gfx_init [gfxtablebase]
;; Initializes the look up tables used by the gfx_plot function
;; The optional argument defines where the look up tables needed by gfx_pset and gfx_pclr are placed ($2c9 bytes). This address should be page-aligned ($xx00). Without the argument, the address $9000 is used as a default.
;; This macro needs to be called once before using gfx_plot or any function that uses gfx_plot (e.g. gfx_line)
;; Registers modified: A, X, Y
.import _gfx_init_sr
.macro gfx_init gfxtablebase
	.if .PARAMCOUNT = 1
	  .ifdef gfxtables
	    .error "gfxtables alreads defined"
	   .endif
	  gfxtables = gfxtablebase
	.else
	  .ifndef gfxtables
	    gfxtables = $9000
	  .endif
	.endif
	.export gfxtables
  .if ::SAVE_REGS>1
  store AX
  store Y
  .endif
	jsr _gfx_init_sr
  .if ::SAVE_REGS>1
  restore AX
  restore Y
  .endif
.endmacro

;; gfx_pset 
;; Sets pixel at position x=X register (Carry indicating values > 255), y=Y register
;; Registers modified: A, X, Y
.import _gfx_pset_sr
.macro gfx_pset
  .if ::SAVE_REGS>1
  store AX
  store Y
  .endif
	jsr _gfx_pset_sr
	.ifnref _gfx_init_sr 
	.warning "The function gfx_pset requires the function gfx_init to be called beforehand, are you sure you have done this?"
	.endif
  .if ::SAVE_REGS>1
  restore AX
  restore Y
  .endif
.endmacro

;; gfx_pclr 
;; Clears pixel at position x=X register (Carry indicating values > 255), y=Y register
;; Registers modified: A, X, Y
.import _gfx_pclr_sr
.macro gfx_pclr
  .if ::SAVE_REGS>1
  store AX
  store Y
  .endif
	jsr _gfx_pclr_sr
	.ifnref _gfx_init_sr 
	.warning "The function gfx_pclr requires the function gfx_init to be called beforehand, are you sure you have done this?"
	.endif
  .if ::SAVE_REGS>1
  restore AX
  restore Y
  .endif
.endmacro

;; gfx_pget 
;; Reads pixel at position x=X register (Carry indicating values > 255), y=Y register
;; Return value (0 or 1) in A
;; Registers modified: X, Y
.import _gfx_pget_sr
.macro gfx_pget
  .if ::SAVE_REGS>1
  store X
  store Y
  .endif
	jsr _gfx_pget_sr
	.ifnref _gfx_init_sr 
	.warning "The function gfx_pclr requires the function gfx_init to be called beforehand, are you sure you have done this?"
	.endif
  .if ::SAVE_REGS>1
  restore X
  restore Y
  .endif
.endmacro

;; set_VIC_charset addr
;; Sets VIC register $D018 to have the charset at the given address
;; addr must be a constant that is a multiple of $800
;; This macro does not adjust the VIC bank, see set_VIC_bank
;; Registers modified: A
.macro set_VIC_charset addr
  .if ::SAVE_REGS>1
  store A
  .endif
	lda $D018
	and #$f0
	ora #((addr & $3800)/$400)
	sta $D018
  .if ::SAVE_REGS>1
  restore A
  .endif
.endmacro

;; set_VIC_screen addr
;; Sets VIC register $D018 to have the screen RAM at the given address
;; addr must be a constant that is a multiple of $400
;; This macro does not adjust the VIC bank, see set_VIC_bank
;; Registers modified: A
.macro set_VIC_screen addr
  .if ::SAVE_REGS>1
  store A
  .endif
	lda $D018
	and #$0f
	ora #((addr & $3C00)/$40)
	sta $D018
  .if ::SAVE_REGS>1
  restore A
  .endif
.endmacro

;; set_VIC_addr screen_addr,charset_addr
;; Sets VIC register $D018 to have the screen RAM and the charset at the given addresses
;; screen_addr must be a constant that is a multiple of $400, charset_addr a multiple of $800
;; This macro does not adjust the VIC bank, see set_VIC_bank
;; Registers modified: A
.macro set_VIC_addr screen_addr,charset_addr
  .if ::SAVE_REGS>1
  store A
  .endif
	lda #((screen_addr & $3C00)/$40) | ((charset_addr & $3800)/$400)
	sta $D018
  .if ::SAVE_REGS>1
  restore A
  .endif
.endmacro

;; set_VIC_bank addr
;; Sets CIA register $DD00 to have the VIC RAM bank at the given address
;; addr must be a constant that is a multiple of $4000
;; Registers modified: A
.macro set_VIC_bank addr
  .if ::SAVE_REGS>1
  store A
  .endif
	lda $DD00
	and #$fc
	ora #(3 - addr/$4000)
	sta $DD00
  .if ::SAVE_REGS>1
  restore A
  .endif
.endmacro
