; Constants for graphics library

screen=$0400
bitmap=$2000

.import gfxtables

gfx_xtablelo = gfxtables
gfx_ytablelo = gfxtables + $100
gfx_mask_or  = gfx_ytablelo + 200 ;no bug, decimal 200 here!
gfx_mask_andi= gfx_mask_or+8
gfx_ytablehi = gfxtables + $200

; ZP allocation for graphics routines

zpaddr1=$22       ;not used by cc65, used by BASIC for temporary ptrs and results, so do we
		  ;can be used until address $2A, at $2B is the pointer to BASIC area
zpaddr2=$4b	  ;2 byte temporary area for saving original pointer to current BASIC instruction during GET, INPUT and READ
zpaddr3=$2A	  ;the last byte from above
zpaddr4=$57	  ;Arithmetoc register #3, #4, #1 15 bytes

; 16 bit values
X1       = zpaddr1
Y1       = zpaddr1+2
X2       = zpaddr1+4
Y2       = zpaddr1+6
DX       = zpaddr2

; 8 bit values
DY       = zpaddr3
ROW      = zpaddr4              ;Bitmap row
COL      = zpaddr4+1            ;and column
INRANGE  = zpaddr4+2            ;Range check flag

RADIUS   = zpaddr4+3

CHUNK1   = zpaddr4+4            ;Circle routine stuff
OLDCH1   = zpaddr4+5
CHUNK2   = zpaddr4+6
OLDCH2   = zpaddr4+7
CX       = DX
CY       = DY
Xpos     = zpaddr4+8
Ypos     = zpaddr4+9
LCOL     = zpaddr4+10           ;Left column
RCOL     = zpaddr4+11
TROW     = zpaddr4+12           ;Top row
BROW     = zpaddr4+13           ;Bottom row
RANGE1   = zpaddr4+14
RANGE2   = INRANGE

POINT    = zpaddr4+15
TEMP2    = zpaddr4+16
TEMP     = zpaddr4+17           ;1 byte