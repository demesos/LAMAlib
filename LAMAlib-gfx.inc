;; <h2>Hires Graphics Functions</h2>

;; blank_screen
;; Blanks the screen and shows only the border color
.macro blank_screen
	lda $D011   ;bit 4 off = Screen blanked
	and #%11101111
	sta $D011 
.endmacro

;; unblank_screen
;; Blanks the screen and shows only the border color
.macro unblank_screen
	lda $D011   ;bit 4 on = Screen visible
	ora #%00010000
	sta $D011 
.endmacro


;; bitmap_on
;; Turn bitmap mode on and initialize tables in case the project uses plotting commands (plot, line, circle, ...)
.import gfx_inittables_sr
.macro bitmap_on
	lda #59   ;bit 5 on = Bitmap-Modus
	sta $D011 
	lda $D018
	ora #8
	sta $D018
.endmacro

;; bitmap_off
;; Turn bitmap mode off
.macro bitmap_off
	lda #27   ;standard text mode
	sta $D011 
	lda $D018
	and #255-8
	sta $D018 
.endmacro

;; gfx_clrscr bgcolor,fgcolor
;; Clear graphics screen and set background and foreground colors
.import _gfx_clrscr_sr

.macro gfx_clrscr bgcolor,fgcolor
.scope
	ldy #bgcolor+$10*fgcolor
	jsr _gfx_clrscr_sr
.endscope
.endmacro

;; gfx_init
;; Initializes the look up tables used by the gfx_plot function
;; This macro needs to be called once before using gfx_plot or any function that uses gfx_plot (e.g. gfx_line)
;; Uses all registers and overwrites memory area $9000 - $92C7
.import _gfx_init_sr
.macro gfx_init
	jsr _gfx_init_sr
.endmacro

;; gfx_plot 
;; Sets pixel at position x=AX, y=Y
;; where AX is the 16 bit value in registers A and X and Y is the Y register
.import _gfx_plot_sr
.macro gfx_plot
	cpx #01	;set carry if X>0
	jsr _gfx_plot_sr
	.ifnref _gfx_init_sr 
	.warning "The function gfx_plot requires the function gfx_init to be called beforehand, are you sure you have done this?"
	.endif
.endmacro

;; set_VIC_charset addr
;; Sets VIC register $D018 to display the charset at the given address
;; addr must be a constant that is a multiple of $800
;; This macro does not adjust the VIC bank, see set_VIC_bank
.macro set_VIC_charset addr
	lda $D018
	and #$f0
	ora #((addr & $3800)/$400)
	sta $D018
.endmacro

;; set_VIC_screen addr
;; Sets VIC register $D018 to have the screen RAM at the given address
;; addr must be a constant that is a multiple of $400
;; This macro does not adjust the VIC bank, see set_VIC_bank
.macro set_VIC_screen addr
	lda $D018
	and #$f0
	ora #((addr & $3C00)/$40)
	sta $D018
.endmacro

;; set_VIC_bank addr
;; Sets CIA register $DD00 to have the VIC RAM bank at the given address
;; addr must be a constant that is a multiple of $4000
.macro set_VIC_bank addr
	lda $DD00
	and #$fc
	ora #(3 - addr/$4000)
	sta $DD00
.endmacro
