; <h1>LAMAlib - Lightweight Assembler MAcro library for cc65</h1>
; Version: 0.12
; Date: 2019-10-26
; Author: Wil Elmenreich (wilfried at gmx dot at)
; License: The Unlicense (public domain)
;
; <h2>Installation and Usage</h2>
; To use LAMAlib you need to have cc65 installed. Get it at https://cc65.github.io
;
; <h3>Possibility 1: Install in cc65</h3>
; Copy all LAMAlib*.inc files into directory <tt>asminc</tt> of your cc65 installation.
; Copy the file LAMAlib.lib into directory <tt>lib</tt> of your cc65 installation.
; Move LAMAlibdoc.html (the file you are currently viewing) either to the html directory of cc65 or to some other place where you want to conveniently keep the documentation.
; You don't need to keep the original folder of LAMAlib
;
; In your programs,
; add a line <tt>.include "LAMAlib.inc"</tt> at the top of your assembler file
; assemble with command <tt>cl65 yourprog.s -lib LAMAlib.lib -C c64-asm.cfg -u __EXEHDR__ -o yourprog.prg</tt>
;
; <h3>Possibility 2: Keep LAMAlib separately</h3>
; Keep a copy of the LAMAlib folder in a sister directory of your project.
;
; In your programs,
; add a line <tt>.include "../LAMAlib/LAMAlib.inc"</tt> at the top of your assembler file (the forward slash works on Linux as well as on Linux systems)
; assemble with command <tt>cl65 yourprog.s -lib ../LAMAlib/LAMAlib.lib -C c64-asm.cfg -u __EXEHDR__ -o yourprog.prg</tt>
; When you publish code of your project you may add LAMAlib to the package. The License has been chosen to be maximum permissive, so whatever project you have, there should be not problems adding the code.
;
; <h2>Points to remember</h2>
; Please note that the zero flag for 16/24/32 bit operations is not properly set for most macros except CMPxx, therefore a CMPxx #00 might be necessary to test for zero.
; Some of the more complex functions like division and multiplication use zero page addresses $22-$2a as temporary memory. This area is also by BASIC for temporary ptrs and results, but borrowing these addresses seem not to interfere severely with the system. Using $22-2A further plays well with cc65 parts written in C and leaves the well-known free ZP addresses at $FB to $FE to the user.
;
; <h1>Command documentation</h1>

.include "LAMAlib-macros16.inc"
.include "LAMAlib-ROMfunctions.inc"
.include "LAMAlib-functions.inc"




