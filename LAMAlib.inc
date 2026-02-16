; <h1>LAMAlib - Lightweight Assembler MAcro library for cc65</h1>
; Version: 0.37
; Date: 2026-02-16
; Author: Wil Elmenreich (wilfried at gmx dot at)
; License: The Unlicense (public domain)
;
; <h2>Installation and Usage</h2>
; To use LAMAlib you need to have cc65 installed. Get it at https://cc65.github.io
;
; <h3>Possibility 1: Install in cc65</h3>
; There is a script <tt>install_lamalib</tt> included which does the following: 
; <ul><li>Copies all LAMAlib*.inc files into directory <tt>asminc</tt> of your cc65 installation.
; <li>Copies the file LAMAlib.lib into directory <tt>lib</tt> of your cc65 installation.
; <li>Copies the <tt>ass</tt> script and the <tt>asdent</tt> indentation tool into the <tt>bin</tt> directory of your cc65 installation.
; </ul>
; You don't need to keep the original folder of LAMAlib, but you probably want to keep a copy of the documentation, LAMAlibdoc.html
;
; In your programs,
; <ul><li>add a line <tt>.include "LAMAlib.inc"</tt> at the top of your assembler file
; <li>assemble with command <tt>cl65 yourprog.s -lib LAMAlib.lib -C c64-asm.cfg -u __EXEHDR__ -o yourprog.prg</tt>
; <li>alternatively, use the provided shellscripts ass.bat / ass.sh: <tt>ass yourprog.s</tt>
; </ul>
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
; <br>
; <p>When assembling for the C128 or VIC20 target systems, it needs to be linked to LAMAlib128.lib or LAMAlib20.lib, respectively. The include command in your files will stay the same: <tt>.include "LAMAlib.inc"</tt>
; 
;
; <h1>Command documentation</h1>
; <h2>ass: Assemble Source with LAMAlib</h2>
;
; Usage: <tt>ass [-20|-128] s-file[,s-file] [startaddr]</tt>
;
; Calls the cl65 assembler and linker and creates an executable <tt>.prg</tt> for the C64, unless <tt>-20</tt> or <tt>-128</tt> is specified, then the program will be assembled for the VIC20 or C128, respectively.
;
; <h2>asdent: Assembler Source Formatting Tool</h2>
; 
; The Assembly Indentation tool <b>asdent</b> checks and fixes the indentation of assembler files, automatically recognizing LAMAlib's structured programming keywords as well as cc65's dot commands.
; 
; Usage: <tt>asdent [-c | -v] &lt;file.s&gt; [&lt;file2.s&gt; ...]</tt>
;
; <h2>exprass: Expression to Assembly Translator</h2>
;
; The Expression to Assembly Translator <b>exprass</b> converts mathematical experession indicated by <b>let</b> into native 6502 assembly code using unsigned 16 bit arithmetic. 
; A,X,Y, and AX can be used as register variables to provide data or be assigned with a result, for example <tt>let A=PEEK(1024+X+40*Y)</tt> will generate code that gives you the character at position X, Y at the screen back in A
; This tool is typically invoked automatically by the <tt>ass</tt> script when it detects high-level expressions within an assembler source file.
; 
; Usage: <tt>exprass [-c] [-v | -q] [-o &lt;output.asm&gt;] &lt;input.s&gt;</tt>
;
; <h2>Switches in your source code</h2>
;
; <b>USE_BASIC_ROM</b> .set [0|1]
; This switch tells LAMAlib if it should use calls into BASIC ROM or not. If USE_BASIC_ROM ist set to 0, LAMAlib uses its own implementations for printstr and print number.
; The LAMAlib implementations are also faster, so USE_BASIC_ROM can also be set to 0 if you would like to increase performance.
; USE_BASIC_ROM does not actually change the ROM configuration.
; The switch can be changed multiple times to create program parts using the ROM and parts that do not.

.ifndef LAMAlib_inc

LAMAlib_inc=1

.include "LAMAlib-macros16.inc"
.include "LAMAlib-systemaddresses.inc"
.include "LAMAlib-gfx.inc"
.include "LAMAlib-structured.inc"
.include "LAMAlib-routines.inc"
.include "LAMAlib-strings.inc"
.include "LAMAlib-BASIC.inc"
.include "LAMAlib-C128only.inc"
.include "LAMAlib-modulemacros.inc"

.endif
