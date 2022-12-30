; <h1>LAMAlib - Lightweight Assembler MAcro library for cc65</h1>
; Version: 0.211
; Date: 2022-09-15
; Author: Wil Elmenreich (wilfried at gmx dot at)
; License: The Unlicense (public domain)
;
; <h2>Installation and Usage</h2>
; To use LAMAlib you need to have cc65 installed. Get it at https://cc65.github.io
;
; <h3>Possibility 1: Install in cc65</h3>
; There is a script <tt>install_lamalib.bat</tt> included which does the following:
; <ul><li>Copies all LAMAlib*.inc files into directory <tt>asminc</tt> of your cc65 installation.
; <li>Copies the file LAMAlib.lib into directory <tt>lib</tt> of your cc65 installation.
; </ul>
; </ul>You don't need to keep the original folder of LAMAlib, but you probably want to keep a copy of the documentation, LAMAlibdoc.html
;
;
; In your programs,
; <ul><li>add a line <tt>.include "LAMAlib.inc"</tt> at the top of your assembler file
; <li>assemble with command <tt>cl65 yourprog.s -lib LAMAlib.lib -C c64-asm.cfg -u __EXEHDR__ -o yourprog.prg</tt>
; <li>alternatively, use the provided shellscripts ass.bat / ass.sh: <tt>ass yourprog.s</tt>
; </ul>
;
; Usage: <tt>ass [-20|-128] asmfile [startaddr]</tt>
;
; Calls the cl65 assembler and linker and creates an executable <tt>.PRG</tt> for the C64, unless <tt>-20</tt> or <tt>-128</tt> is specified, then the program will be assembled for the VIC20 or C128, respectively.
;
; <h3>Possibility 2: Keep LAMAlib separately</h3>
; Keep a copy of the LAMAlib folder in a sister directory of your project. You need then to link to the library via its relative or absolute path.
;
; In your programs,
; <ul><li>add a line <tt>.include "../LAMAlib/LAMAlib.inc"</tt> at the top of your assembler file (the forward slash works on Linux as well as on Linux systems)
; <li>assemble with command <tt>cl65 yourprog.s -lib ../LAMAlib/LAMAlib.lib -C c64-asm.cfg -u __EXEHDR__ -o yourprog.prg</tt>
; <li>when you publish source code of your project you can add LAMAlib to the package. The license of LAMAlib has been chosen to be maximum permissive, so whatever project you have, there should be no problems adding the code.
; </ul>
;
; <h2>Points to remember</h2>
; Please note that the zero flag for 16 operations is not properly set for most macros except CMP. For example after a 16 bit calculation, a CMPAX #00 is necessary to test for zero in AX.
; Instead of many zero page variables, the library functions uses self-contained self-modifying codeblocks whenever possible, but some of the more complex functions like division and multiplication use zero page addresses, they are reserved in the segment "ZEROPAGE". 
; 
;
; <h1>Command documentation</h1>
;
; <h2>Switches</h2>
;
; <b>USE_BASIC_ROM</b> .set [0|1]
; This switch tells LAMAlib if it should use calls into BASIC ROM or not. If USE_BASIC_ROM ist set to 0, LAMAlib uses its own implementations for printstr and print number.
; The LAMAlib implementations are also faster, so USE_BASIC_ROM can also be set to 0 if you would like to increase performance.
; USE_BASIC_ROM does not actually change the ROM configuration.
; The switch can be changed multiple times to create program parts using the ROM and parts that do not.

.ifndef LAMAlib_inc

LAMAlib_inc=1

.include "LAMAlib-macros16.inc"
.include "LAMAlib-ROMfunctions.inc"
.include "LAMAlib-gfx.inc"
.include "LAMAlib-structured.inc"
.include "LAMAlib-routines.inc"
.include "LAMAlib-BASIC.inc"
.include "LAMAlib-C128only.inc"

.endif