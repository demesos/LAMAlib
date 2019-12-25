;; <h2>Hires Graphics Functions</h2>

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
.import gfx_clrscr_sr

.macro gfx_clrscr bgcolor,fgcolor
.scope
	ldy #bgcolor+$10*fgcolor
	jsr gfx_clrscr_sr
.endscope
.endmacro

;; gfx_init
;; Initializes the look up tables used by the gfx_plot function
;; This macro needs to be called once before using gfx_plot or any function that uses gfx_plot (e.g. gfx_line)
;; Uses all registers and overwrites memory area $9000 - $92C7
.import gfx_init_sr
.macro gfx_init
	jsr gfx_init_sr
.endmacro

;; gfx_plot 
;; Sets pixel at position x=AX, y=Y
;; where AX is the 16 bit value in registers A and X and Y is the Y register
.import gfx_plot_sr
.macro gfx_plot
	cpx #01	;set carry if X>0
	jsr gfx_plot_sr
	.ifnref gfx_init_sr 
	.warning "The function gfx_plot requires the function gfx_init to be called beforehand, are you sure you have done this?"
	.endif
.endmacro

