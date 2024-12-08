;***********************************************************************
;* Module: polychars
;* Version 0.34
;* by Wil
;*
;* Purpose:
;* This module provides dynamic multicolor character handling for the 
;* Commodore 64, enabling the use of multiple character sets to display 
;* characters beyond the standard 256-character limitation. Characters 
;* are represented as 16-bit values, where the high byte specifies the 
;* character set, and the low byte specifies the character index within 
;* that set.
;*
;* Features:
;* - Dynamic character usage and mapping:
;*   - A table-driven system determines whether a character is already 
;*     mapped to the current display character set.
;*   - If unmapped, an unused character slot is dynamically allocated, 
;*     and its bitmap is copied from the source character set.
;*   - Efficient garbage collection reclaims character slots that are 
;*     no longer displayed on screen.
;* - Transparent CHROUT integration:
;*   - Includes hooks to modify the KERNAL's CHROUT function for seamless 
;*     integration with BASIC and LAMAlib's print macros.
;* - Compatibility with multiple polychar instances:
;*   - Supports displaying more than 256 characters by configuring 
;*     independent instances of the module for different screen and 
;*     character set regions.
;*
;* Limitations:
;* - A maximum of 256 characters can be displayed on the screen at any 
;*   given time for a single polychar instance.
;* - For multiple instances, a rasterline interrupt is required to switch 
;*   VIC-II configurations.
;*
;* Configuration and Inclusion:
;* To use this module in your program:
;* .scope polychars
;*   .include "m_polychars.s"
;* .endscope
;*
;* Main Program Usage:
;* m_init polychars                     ; Initializes tables and clears the screen
;* m_call polychars,enable_chrout_hook  ; Enables CHROUT hook
;* m_call polychars,disable_chrout_hook ; Disables CHROUT hook
;* m_call polychars,putchar             ; Prints a character (16-bit encoded)
;* m_call polychars,garbage_collection  ; Reclaims unused character slots
;*
;* If you need more than 256 characters on the screen, use two polychar instances with different charset 
;* addresses and translation tables. They can share the same source charsets.
;* Example for multi-instance use:
;*
;* .scope polychar_upperscreen
;*   CHARSETS_SRC=charsets
;*   SCREEN_ADDR=$C000
;*   CHARSET_ADDR=$E000
;*   TABLES=$9000
;*   .include "m_polychars.s"
;* .endscope
;* .scope polychar_lowerscreen
;*   CHARSETS_SRC=charsets
;*   SCREEN_ADDR=$C400
;*   CHARSET_ADDR=$E800
;*   TABLES=$9800
;*   .include "m_polychars.s"
;* .endscope
;* In this case you can only use the chrout on one of the modules at the same time
;* you need to add a rasterline interrupt that switches between the two SCREEN_ADDR and SCREEN_ADDR
;* by modifying VIC register $d018
;***********************************************************************

.include "LAMAlib.inc"

;***********************************************************************
;* parameters - can be overwritten from main file
;* without a default value the constant must be set by the main program

def_const NUM_CHARSETS,4	;number of source charsets
def_const CHARSETS_SRC		;place where the charsets are stored (don't have to be mmory aligned)
def_const CHARSET_ADDR,$E000	;charset address
def_const SCREEN_ADDR,$C000	;address of the display screen 
def_const SWITCHCODE1,133	;ASCII code of the switchcode for the charsets
def_const TABLES,$9000		;adress of translation tables, need to be visible when putchar is called
                                ;size of tables is 256*(2*NUM_CHARSETS+3)

def_const SCREEN_START,SCREEN_ADDR	;defines the used screen area, typically the whole screen. Affects garbage collection scope and clear screen function
def_const SCREEN_END,SCREEN_ADDR+999    ;addresses must be aligned to the begin of lines

translation_table=TABLES             ;contains the char the 16bit code is currently mapped to
translation_table_used=TABLES+NUM_CHARSETS*256           ;contains nonzero if this 16bit code is mapped
rev_translate_lo=translation_table_used+NUM_CHARSETS*256
rev_translate_hi=rev_translate_lo+256
char_usage=rev_translate_hi+256

.code

;jump table
        jmp init
        jmp set_screen_ptr              ;+3
        jmp putchar                     ;+6
        jmp garbage_collection          ;+9
        jmp enable_chrout_hook          ;+12
        jmp disable_chrout_hook         ;+15

;***********************************************************************
;* Procedure: set_screen_ptr
;* Sets the screen pointer to the current value in AX
;* AX must be an address in the respective screen area
;* the pointer to the color RAM will be updated consistently
;***********************************************************************
.proc set_screen_ptr
        stax screen_ptr
        rts
.endproc

;***********************************************************************
;* Procedure: new_charout
;* Intercepts and processes character output, including handling
;* printable characters and special control codes as does the KERNAL
;* CHROUT function. 
;* Input is in A, containing the ASCII code to print in the currently
;* selected charset.
;* Codes between SWITCHCODE1 and SWITCHCODE1+NUM_CHARSETS-1
;* will select a new charset. Default setting for switchcodes are the
;* function keys F1,F3,F5,F7,F2,F4,F6,F8 (in that order)
;***********************************************************************
.proc new_charout
        store AX
  ;is character printable?
        cmp #32
        bcc normal_chrout
        cmp #128
        bcc printable
        cmp #160
        bcs printable
        cmp #147
	if eq
	  jsr clearscreen
	  jmp exit
	endif
        cmp #SWITCHCODE1      ;default F1
        bcc normal_chrout
        cmp #SWITCHCODE1+NUM_CHARSETS
        bcs normal_chrout
        ;F-key, switch charset
        sec
        sbc #133
        sta current_charset
        jmp exit

printable:
        to_scrcode
        ldx REVERSE_MODE_SWITCH
        if ne
            ora #$80      ;reverse mode is on
        endif
        store A
        ldx CURSORY
        lda screenline_lobytes,x
        clc
        adc CURSORX
        sta screen_ptr
        lda screenline_hibytes,x
        adc #00
        sta screen_ptr+1
        restore A
        ldx current_charset
        store Y
        jsr putchar
        restore Y
        ldx CURSORX
        inx
        cpx #40
        if cs
            ldx #0
            lda CURSORY
            cmp #24
            if cc
                inc CURSORY
            endif
        endif
        stx CURSORX
exit:
        restore AX
	clc
        rts

normal_chrout:
chrout_backup=*+1
        jmp $f1ca

screenline_lobytes:
  .repeat 25,i
        .byte <(SCREEN_ADDR+i*40)
  .endrep
screenline_hibytes:
  .repeat 25,i
        .byte >(SCREEN_ADDR+i*40)
  .endrep

.endproc

;***********************************************************************
;* Procedure: init
;* Initializes the polychars module, clearing screen memory, setting
;* up translation tables, and preparing display configurations.
;***********************************************************************
.proc init
        poke current_charset,0
        ldax #SCREEN_ADDR
        stax screen_ptr
        ldax $326       ;old chrout vector
        stax new_charout::chrout_backup
        poke PTRSCRHI,>SCREEN_ADDR
        set_VIC_addr SCREEN_ADDR,CHARSET_ADDR
        set_VIC_bank SCREEN_ADDR
.endproc
	;FALL THROUGH INTENDED!

;***********************************************************************
;* Procedure: clearscreen
;* Clears the screen and initializes character usage and mapping.
;* - Fills the screen memory with spaces (character $20).
;* - Resets all translation and usage tables.
;* - Maps character $20 (space) in the charset to ensure proper display.
;* - Copies the space character's bitmap from the source charset to the
;*   active charset in memory.
;***********************************************************************
.proc clearscreen
	;place cursor in home position
	.byte $ab,$00	;LXA #0 load A and X register with 0
	tay
	clc
	jsr PLOT
	;mark table entries as available
        memset translation_table_used,translation_table_used+NUM_CHARSETS*256-1,0
        memset char_usage,char_usage+255,0
        memset SCREEN_START,SCREEN_END,32	;fill screen with spaces
	;map character $20 in charset 1 to character $20 (usually the space character)
        poke translation_table+32,32	;map to $20	
        inc translation_table_used+32	;indicate character 32 to be mapped
        poke rev_translate_lo+32,32
        poke rev_translate_hi,0	
	inc char_usage+$20
	;copy character $20
	for X,7,downto,0
	  lda CHARSETS_SRC+32*8,x
	  sta CHARSET_ADDR+32*8,x
	next
        rts
.endproc

;***********************************************************************
;* Procedure: enable_chrout_hook
;* Enables a custom CHROUT hook to process character output
;* through the polychars module.
;***********************************************************************
.proc enable_chrout_hook
        ldax #new_charout
        stax $326
        rts
.endproc

;***********************************************************************
;* Procedure: disable_chrout_hook
;* Restores the original CHROUT routine, disabling the polychars hook.
;***********************************************************************
.proc disable_chrout_hook
        ldax new_charout::chrout_backup
        stax $326
        rts
.endproc


        .assert <translation_table = 0, error, "translation_table must be aligned to a page"
        .assert <rev_translate_lo = 0, error, "rev_translate_lo must be aligned to a page"
        .assert <rev_translate_hi = 0, error, "rev_translate_hi must be aligned to a page"

;***********************************************************************
;* Procedure: putchar
;* Displays a character stored in AX on the current screen position,
;* managing charset translation and ensuring display consistency.
;***********************************************************************
.proc putchar
        store AX
        ;check if char is already in translation table
        tay
        txa
        ora #>translation_table_used
        sta addr_tbl2+1
addr_tbl2=*+1
        lda $4200,y     ;check if 16bit code is mapped
        longif eq
find_unused_char:
next_unused=*+1
            ldx #00
            lda char_usage,x
            beq found_unused
            ;didn't work, search full array
            ldx #0
search_unused_char:
            lda char_usage,x
            beq found_unused
            dex
            bne search_unused_char        ;to check: loop with differently starting X and a Y as loop counter might be more efficient
            ;no unused char found
            jsr garbage_collection
            bcc found_unused
            rts
            ;garbage collection successful, X contains index of available char
found_unused:
            inc char_usage,x      ;mark used
            lda stored_X
            sta rev_translate_hi,x
            lda stored_A
            sta rev_translate_lo,x

            ;mark entry in translation table2 as active
            lda stored_X
            ora #>translation_table_used
            sta addr_tbl2a+1
            ldy stored_A
            lda #1
addr_tbl2a=*+1
            sta $4200,y
          ;and write entry to translation table
            lda stored_X
            ora #>translation_table
            sta addr_tbl1a+1
            txa
addr_tbl1a=*+1
            sta $4200,y
            ;now install new char to place x
            ;txa not necessary, target char number is already in A
            stx char2print
            inx
            stx next_unused       ;advance pointer to find next unused char better
            ldx #0      ;highbyte 0
            aslax
            aslax
            aslax       ;*8
            addax #CHARSET_ADDR
            stax target_char
            restore AX
            aslax
            aslax
            aslax
            addax #CHARSETS_SRC
            stax source_addr
            ldx #7
cpy_char:
source_addr=*+1
            lda $1234,x
target_char=*+1
            sta $1234,x
            dex
            bpl cpy_char
char2print=*+1
            lda #0
        else

        ;we can print the char right away
char_ok:
            txa
            ora #>translation_table
            sta addr_tbl1+1
addr_tbl1=*+1
            lda $4200,y

        endif
.endproc	;screen_ptr needs to be outside .proc since it is referred to from other routines as well
screen_ptr=*+1
        sta $0400

	;now color the character we just printed
	lda screen_ptr
	sta col_ptr
	lda screen_ptr+1
	and #3
	ora #$d8
	sta col_ptr+1
	lda TEXTCOLOR_ADDR
col_ptr=*+1
	sta $d800

an_rts:
        rts


        .assert (SCREEN_START & $3ff) .mod 40 = 0, error, "screen start must be at the start of a line"
        .assert (SCREEN_END & $3ff) .mod 40 = 39, error, "screen start must be at the end of a line"


;***********************************************************************
;* Procedure: garbage_collection
;* Frees unused character slots in the charset by scanning the screen
;* for displayed characters and marking their usage, then clearing 
;* entries for unused characters in translation tables.
;***********************************************************************
garbage_collection:	;no proc because the .repeat function plays up in this case
        lda #0
        tax
clr_usage:
        sta char_usage,x
        inx
        bne clr_usage
        ldy #39
record_usage:
  .repeat 1+(SCREEN_END-SCREEN_START)/40,i
        ldx SCREEN_START+i*40,y
        sec
        rol char_usage,x
  .endrep
        dey
        bmi done
        jmp record_usage
done:
        lda #$38     ;SEC opcode
        sta unused_flag


        ldy #0
clear_unused:
        lda char_usage,y
        if eq
            sty unused_idx
            lda #$18     ;CLC opcode
            sta unused_flag
           ;clean entry X
            lda rev_translate_hi,y
            ldx rev_translate_lo,y
            clc
            adc #>translation_table_used
            sta addr_tbl2+1
            lda #0
addr_tbl2=*+1
            sta $4200,x
        endif
        iny
        bne clear_unused
unused_idx=*+1
        ldx #$42
unused_flag:
        clc
        rts

used_chars: .byte 0,0
current_charset: .byte 0

