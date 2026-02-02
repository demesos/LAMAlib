# LAMAlib Documentation

## Table of Contents
- [API Reference](#api-reference)
  - [16-bit Emulation and Other Short Macros](#16-bit-emulation-and-other-short-macros)
  - [Hires Graphics Functions - CURRENTLY UNDER DEVELOPMENT](#hires-graphics-functions-currently-under-development)
  - [Structured Programming](#structured-programming)
  - [Useful Routines](#useful-routines)
  - [String Routines](#string-routines)
  - [Interacting with BASIC](#interacting-with-basic)
  - [Special Macros for C128 in C128 Mode](#special-macros-for-c128-in-c128-mode)
  - [Other Macros](#other-macros)
- [Modules](#modules)
  - [bigcharout](#bigcharout)
  - [copycharset](#copycharset)
  - [mousedriver](#mousedriver)
  - [PETSCII decode and display](#petscii-decode-and-display)
  - [polychars](#polychars)


Version: 0.351  
Date: 2026-01-04  
Author: Wil Elmenreich (wilfried at gmx dot at)  
License: The Unlicense (public domain)  

## Installation and Usage

To use LAMAlib you need to have cc65 installed. Get it at https://cc65.github.io  

### Possibility 1: Install in cc65

There is a script install_lamalib included which does the following:   
Copies all LAMAlib*.inc files into directory asminc of your cc65 installation.  
Copies the file LAMAlib.lib into directory lib of your cc65 installation.  
Copies the ass script and the asdent indentation tool into the bin directory of your cc65 installation.  
You don't need to keep the original folder of LAMAlib, but you probably want to keep a copy of the documentation, LAMAlibdoc.html  
In your programs,  
add a line .include "LAMAlib.inc" at the top of your assembler file  
assemble with command cl65 yourprog.s -lib LAMAlib.lib -C c64-asm.cfg -u __EXEHDR__ -o yourprog.prg  
alternatively, use the provided shellscripts ass.bat / ass.sh: ass yourprog.s  

### Possibility 2: Keep LAMAlib separately

Keep a copy of the LAMAlib folder in a sister directory of your project. You need then to link to the library via its relative or absolute path.  
In your programs,  
add a line .include "../LAMAlib/LAMAlib.inc" at the top of your assembler file (the forward slash works on Linux as well as on Linux systems)  
assemble with command cl65 yourprog.s -lib ../LAMAlib/LAMAlib.lib -C c64-asm.cfg -u __EXEHDR__ -o yourprog.prg  
when you publish source code of your project you can add LAMAlib to the package. The license of LAMAlib has been chosen to be maximum permissive, so whatever project you have, there should be no problems adding the code.  

## Points to remember

Please note that the zero flag for 16 operations is not properly set for most macros except CMP. For example after a 16 bit calculation, a CMPAX #00 is necessary to test for zero in AX.  
Instead of many zero page variables, the library functions uses self-contained self-modifying codeblocks whenever possible, but some of the more complex functions like division and multiplication use zero page addresses, they are reserved in the segment "ZEROPAGE".   
When assembling for the C128 or VIC20 target systems, it needs to be linked to LAMAlib128.lib or LAMAlib20.lib, respectively. The include command in your files will stay the same: .include "LAMAlib.inc"  

# Command documentation


## ass: Assemble Source with LAMAlib

Usage: ass [-20|-128] s-file[,s-file] [startaddr]  
Calls the cl65 assembler and linker and creates an executable .prg for the C64, unless -20 or -128 is specified, then the program will be assembled for the VIC20 or C128, respectively.  

## asdent: Assembler Source Formatting Tool

The Assembly Indentation tool asdent checks and fixes the indentation of assembler files, automatically recognizing LAMAlib's structured programming keywords as well as cc65's dot commands.  
Usage: asdent [-c | -v] <file.s> [<file2.s> ...]  

## exprass: Expression to Assembly Translator

The Expression to Assembly Translator exprass converts mathematical experession indicated by let into native 6502 assembly code using unsigned 16 bit arithmetic.   
A,X,Y, and AX can be used as register variables to provide data or be assigned with a result, for example let A=PEEK(1024+X+40*Y) will generate code that gives you the character at position X, Y at the screen back in A  
This tool is typically invoked automatically by the ass script when it detects high-level expressions within an assembler source file.  
Usage: exprass [-c] [-v | -q] [-o <output.asm>] <input.s>  

## Switches in your source code

USE_BASIC_ROM .set [0|1]  
This switch tells LAMAlib if it should use calls into BASIC ROM or not. If USE_BASIC_ROM ist set to 0, LAMAlib uses its own implementations for printstr and print number.  
The LAMAlib implementations are also faster, so USE_BASIC_ROM can also be set to 0 if you would like to increase performance.  
USE_BASIC_ROM does not actually change the ROM configuration.  
The switch can be changed multiple times to create program parts using the ROM and parts that do not.  

---

# API Reference

## 16-bit Emulation and Other Short Macros

### `A_between`

**Syntax:** `A_between lower,higher`

Checks if the value in A is between the lower and higher value (including the values themselves)  
Arguments are constant immediate values between 0 and 255, A is changed in the progress  
If lower <= A <= higher, the carry bit is cleared, otherwise carry is set  

**Registers modified: A**

### `abs`

**Syntax:** `abs`

Converts the signed 8-bit value in A to an absolute value  

**Returns:** Result is returned in A

**Registers modified: A**

### `absax`

**Syntax:** `absax`

Converts the signed 16-bit value in AX to an absolute value  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `adcax`

**Syntax:** `adcax addr`

**Alternate:** `adcax #arg`

Supports zero page addressing mode  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `addax`

**Syntax:** `addax addr`

**Alternate:** `addax #arg`

Supports zero page addressing mode  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `andax`

**Syntax:** `andax addr`

**Alternate:** `andax #arg`

Calculates the bitwise AND operation between AX and a 16 bit value at an addr or as immediate value  
Supports zero page addressing mode  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `asl16`

**Syntax:** `asl16 addr`

Performs an arithmetic shift left of a 16 bit number at addr  

**Returns:** Result at addr, addr+1

**Registers modified: none**

### `aslax`

**Syntax:** `aslax`

Performs an arithmetic shift left of AX (essentially a multiplication with 2, MSB goes into carry)  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `AX_between`

**Syntax:** `AX_between lower,higher`

Checks if the value in A is between the lower and higher value (including the values themselves)  
Arguments are constant immediate values between 0 and 65535, AX is changed in the progress  
If lower <= AX <= higher, the carry bit is cleared, otherwise carry is set  

**Registers modified: A,X**

### `cmpax`

**Syntax:** `cmpax addr`

**Alternate:** `cmpax #arg`

Compares the value in AX with the 16 bit value in addr or the immediate value  
Supports zero page addressing mode  

**Returns:** Result is returned in AX

**Registers modified: none**

### `dec16`

**Syntax:** `dec16 addr[,decrement]`

Decrements the value stored at addr (lo-byte) and addr+1 (hi-byte) as a 16 bit value  
addr can also be specified by AX  
decrement can also be specified by Y and, if AX is not used as addr, by AX,A, or X  
If decrement is not specified, an decrement of 1 is used  

**Registers modified: none with decrement = 1, AX otherwise**

### `dec8`

**Syntax:** `dec8 addr[,decrement]`

Decrements the byte stored at addr by decrement  
addr or decrement can also be specified by AX  
If addr is specified by AX, the decrement can also be specified by Y  
If decrement is not specified, a decrement of 1 is used; with a given fixed addr this is identical to a DEC command  

**Registers modified: none in decrement = 1, A otherwise**

### `decax`

**Syntax:** `decax`

**Registers modified: A,X**

### `decx`

**Syntax:** `decx n`

Flags affected: N,Z,C  

**Registers modified: X**

### `decy`

**Syntax:** `decy n`

Flags affected: N,Z,C  

**Registers modified: Y**

### `eorax`

**Syntax:** `eorax addr`

**Alternate:** `eorax #arg`

Calculates the bitwise exclusive-or operation between AX and a 16 bit value at addr or as immediate value  
Supports zero page addressing mode  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `inc16`

**Syntax:** `inc16 addr[,increment]`

Increments the value stored at addr (lo-byte) and addr+1 (hi-byte) as a 16 bit value  
addr can also be specified by AX  
increment can also be specified by Y and, if AX is not used as addr, by AX,A, or X  
If increment is not specified, an increment of 1 is used  

**Registers modified: none in increment = 1, AX otherwise**

### `inc8`

**Syntax:** `inc8 addr[,increment]`

Increments the byte stored at addr by increment  
addr or increment can also be specified by AX  
If addr is specified by AX, the increment can also be specified by Y  
If increment is not specified, an increment of 1 is used, with a given addr this is identical to an INC command  

**Registers modified: none in increment = 1, A otherwise**

### `incax`

**Syntax:** `incax`

**Registers modified: A,X**

### `incx`

**Syntax:** `incx n`

Flags affected: N,Z,C  

**Registers modified: X**

### `incy`

**Syntax:** `incy n`

Flags affected: N,Z,C  

**Registers modified: Y**

### `jsr_ind`

**Syntax:** `jsr_ind addr`

arg1 will be a 16 bit address containing the vector to jump to  

**Returns:** Executes a subroutine by a vector stored in addr and returns to the command after jsr_ind upon completion with an rts

**Registers modified: none **

### `ldax`

**Syntax:** `ldax addr`

**Alternate:** `ldax #arg`

Loads a 16-bit value into AX, either from an address or as immediate value  
Supports zero page addressing mode  

**Registers modified: A,X**

**Notes:**
- Note that the zero flag is not indicating 0 but indicating a value <256

### `lsr16`

**Syntax:** `lsr16 addr`

Performs a logic shift right of a 16 bit number at addr  

**Returns:** Result at addr, addr+1

**Registers modified: none**

### `lsrax`

**Syntax:** `lsrax`

Performs a logic shift right of AX (essentially a division by 2, LSB goes into carry)  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `neg`

**Syntax:** `neg`

Negate A  

**Registers modified: A**

### `negax`

**Syntax:** `negax`

Negates the value in AX  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `orax`

**Syntax:** `orax addr`

**Alternate:** `orax #arg`

Calculates the bitwise OR operation between AX and a 16 bit value at an addr or as immediate value  
Supports zero page addressing mode  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `peek`

**Syntax:** `peek addr[,reg]`

When the address is a constant this defaults to lda addr  

**Returns:** returns the content of addr in the given register (default A)

**Registers modified: only the specified register (default: A)**

### `peekw`

**Syntax:** `peekw addr`

**Returns:** returns the content of addr and addr+1 as 16bit value in AX

**Registers modified: A,X**

### `poke`

**Syntax:** `poke arg1,arg2 `

Copies arg2 into the address of arg1  
arg1 can be a constant or AX  
arg2 can be a constant or A, X or Y  

### `pokew`

**Syntax:** `pokew arg1,arg2`

poke word: copies 16 bit value arg2 into the address of arg1 and arg1+1  
arg1 will be filled with low byte of arg2  
arg2 will be filled with high byte of arg2  
arg1, arg2 can be both constants or one can be AX and the other a constant  

**Registers modified: A, Y (in case AX is used as address)**

### `pullax`

**Syntax:** `pullax `

Pulls AX from the stack  

### `pushax`

**Syntax:** `pushax `

Pushes AX to the stack and preserves AX  

### `rol16`

**Syntax:** `rol16 addr`

Performs a rotate left of a 16 bit number at addr  

**Returns:** Result at addr, addr+1

**Registers modified: none**

### `rolax`

**Syntax:** `rolax`

Performs a rotate left of AX (essentially a multiplication with 2, carry goes into LSB, MSB goes into carry)  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `ror16`

**Syntax:** `ror16 addr`

Performs a rotate right of a 16 bit number at addr  

**Returns:** Result at addr, addr+1

**Registers modified: none**

### `rorax`

**Syntax:** `rorax`

Performs a rotate right of AX (essentially a division by 2, carry goes into MSB, LSB goes into carry)  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `rsb`

**Syntax:** `rsb arg`

**Alternate:** `rsb #arg`

Reverse subtraction, calculate the value of arg - A   
If there is an underflow (arg is larger than A), the carry bit will be set, otherwise carry is clear  

**Returns:** The result is not influenced by the carry

### `rsbax`

**Syntax:** `rsbax arg`

**Alternate:** `rsbax #arg`

16 bit reverse subtraction, calculate the value of arg - AX   
If there is an underflow (arg is larger than AX), the carry bit will be set, otherwise carry is clear  

**Returns:** The result is not influenced by the carry

### `rsc`

**Syntax:** `rsc arg`

**Alternate:** `rsc #arg`

Reverse subtraction with carry, calculate the value of arg - A - C  
If the carry is clear before the command, this behaves like rsb  
If there is an underflow (arg is larger than A), the carry bit will be set, otherwise carry is clear  

### `rscax`

**Syntax:** `rscax arg`

**Alternate:** `rscax #arg`

16 bit reverse subtraction, calculate the value of arg - AX - C  
If the carry is clear before the command, this behaves like rsbax  
If there is an underflow (arg is larger than AX), the carry bit will be set, otherwise carry is clear  

### `sbcax`

**Syntax:** `sbcax addr`

**Alternate:** `sbcax #arg`

Supports zero page addressing mode  

**Returns:** Result is returned in AX

**Registers modified: A,X**

### `stax`

**Syntax:** `stax addr`

Supports zero page addressing mode  

**Registers modified: none**

### `subax`

**Syntax:** `subax addr`

**Alternate:** `sbcax #arg`

Supports zero page addressing mode  

**Returns:** Result is returned in AX

**Registers modified: A,X**

## Hires Graphics Functions - CURRENTLY UNDER DEVELOPMENT

### `bitmap_off`

**Syntax:** `bitmap_off`

Turn bitmap mode off  

### `bitmap_on`

**Syntax:** `bitmap_on`

Turn bitmap mode on and initialize tables in case the project uses plotting commands (plot, line, circle, ...)  

### `blank_screen`

**Syntax:** `blank_screen`

Waits until rasterbar is below screen area, then blanks the screen and shows only the border color  

### `gfx_clrscr`

**Syntax:** `gfx_clrscr bgcolor,fgcolor`

Clear graphics screen and set background and foreground colors  

### `gfx_init`

**Syntax:** `gfx_init [gfxtablebase]`

Initializes the look up tables used by the gfx_plot function  
The optional argument defines where the look up tables needed by gfx_pset and gfx_pclr are placed ($2c9 bytes). This address should be page-aligned ($xx00). Without the argument, the address $9000 is used as a default.  
This macro needs to be called once before using gfx_plot or any function that uses gfx_plot (e.g. gfx_line)  

### `gfx_pclr`

**Syntax:** `gfx_pclr `

### `gfx_pget`

**Syntax:** `gfx_pget `

**Returns:** Return value (0 or 1) in A

### `gfx_pset`

**Syntax:** `gfx_pset `

### `set_VIC_addr`

**Syntax:** `set_VIC_addr screen_addr,charset_addr`

screen_addr must be a constant that is a multiple of $400, charset_addr a multiple of $800  
This macro does not adjust the VIC bank, see set_VIC_bank  

### `set_VIC_bank`

**Syntax:** `set_VIC_bank addr`

addr must be a constant that is a multiple of $4000  
These commands allow you to use constructs like if .. else .. endif, do...loop, for...next, and switch...case in assembly language! The structures can even be nested. The implementation of these structures is basically as efficient as a a handcoded composure of branches, jumps as labels, while it is much easier to write and read.  
All macros can be nested.  

### `set_VIC_charset`

**Syntax:** `set_VIC_charset addr`

addr must be a constant that is a multiple of $800  
This macro does not adjust the VIC bank, see set_VIC_bank  

### `set_VIC_screen`

**Syntax:** `set_VIC_screen addr`

addr must be a constant that is a multiple of $400  
This macro does not adjust the VIC bank, see set_VIC_bank  

### `unblank_screen`

**Syntax:** `unblank_screen`

Shows the screen again after it was blanked, effective with next frame  

## Structured Programming

### `do`

**Syntax:** `do`

...  
**Alternate:** `[until|while cond]`

...  
Defines a loop that is exit based on a while or until condition  
This corresponds to assembler commands BEQ, BNE, BMI, BPL, BCC, BCS, BVC, BVS  
There can be any number of until or while conditions, also none, which defines an endless loop  
Within a do...loop, the macros break and continue can be used to exit the loop or go to next iteration.  
Any line with a until, while, loop until, or while until will be typically preceded with code that sets the respective processor flags, in many cases this  
will be a compare instruction. For example the C code while loop:  
**Alternate:** `while(i>1) {`

**Alternate:** `do_something()`

   i--;  
}  
would translate into  
do  
**Alternate:** `cpy #2 ;1+1`

   while ge  
   jsr do_something  
   dey  
loop  
Code example that waits for joystick 2 button to be pressed:  
**Alternate:** `lda #$10`

do  
   and $dc00  
loop until eq  

**Registers modified: A, if the loop variable is X or Y also the respective X or Y register**

### `do_every`

**Syntax:** `do_every interval[,phase]`

...  
Defines a block that is executed every n-th time  
interval: Specifies the interval between each execution. Maximum value 255.  
phase: Determines the phase offset of the first iteration. If 0 the event is triggered in the first iteration.  
Default value for phase is the interval-1  

**Registers modified: A**

### `do_once`

**Syntax:** `do_once [maxcalls]`

...  
Defines a block that is executed a specified number of times.  
maxcalls: Specifies the number of times the code within the block will be executed (default=1).  

**Registers modified: A**

### `do_skip_every`

**Syntax:** `do_skip_every interval[,phase]`

...  
Defines a block that is skipped every n-th time (runs n-1 out of n times)  
interval: Specifies the interval at which to skip execution. Maximum value 255.  
phase: Determines the phase offset of the first skip. If 0 the skip occurs in the first iteration.  
Default value for phase is the interval-1  

**Registers modified: A**

### `for`

**Syntax:** `for X|Y|A|AX|addr,start,to|downto,end,step`

...  
The for loop iterates from the start value to the end value, inclusive. This is similar to the behavior of FOR in BASIC  
Memory references can also go to zero page. In this case the zero page addressing mode is used which speeds up the code.  
It is possible to nest multiple for loops but each for must be followed by exactly one corresponding next later in the code.  
Within a for loop, the macros break and continue can be used to exit the loop or go to next iteration.  
Code example that outputs '9876543210':  
**Alternate:** `for A,$39,downto,$30`

   jsr $ffd2  
next  

**Registers modified: the loop register and A for indirectly given step values**

### `if`

**Syntax:** `if cond`

...  
**Alternate:** `[else]`

...  
This is a structure for conditional execution  
This corresponds to assembler commands BEQ, BNE, BMI, BPL, BCC, BCS, BVC, BVS  
Therefore the amount of code between if and else must not exceed the range of a branch instruction (127 byte for a forward branch)  
using else is optional  

**Registers modified: none**

### `on_A_jmp`

**Syntax:** `on_A_jmp addr1, addr2, ...`

If A equals 1, execution jumps to the first specified address.  
If A equals 2, 3, 4, etc., execution jumps to the corresponding address in sequence.  
The address list can contain up to 42 entries  
If A is 0 or greater than the number of provided addresses, no jump occurs, and execution continues after the macro.  

**Returns:** The jump transfers control to the target address and does not return to the original point.

### `on_A_jmp0`

**Syntax:** `on_A_jmp0 addr1, addr2, ...`

If A equals 0, execution jumps to the first specified address.  
If A equals 1, 2, 3, etc., execution jumps to the corresponding address in sequence.  
The address list can contain up to 43 entries  
If the value of A exceeds the number of provided addresses, no jump occurs, and execution continues after the macro.  

**Returns:** The jump transfers control to the target address without returning to the original point.

### `on_A_jmp0_nocheck`

**Syntax:** `on_A_jmp0_nocheck addr1, addr2, ...`

If A equals 0, execution jumps to the first specified address.  
If A equals 1, 2, 3, etc., execution jumps to the corresponding address in sequence.  
The address list can contain up to 43 entries  
If A exceeds the number of provided addresses, the program will jump to an undefined location, likely causing a crash.  
No safety checks are performed to ensure A is within bounds, so the programmer must ensure correct values for A.  
**Alternate:** `on_A_jmp0_nocheck is used internally by other jump constructs.`

Depending on the number of provided addresses, it selects between two different implementations: `on_A_jmp_mul3` and `on_A_jmp_mul4`.  
These variations handle address alignment and access optimization based on the provided number of addresses.  

### `on_A_jsr`

**Syntax:** `on_A_jsr addr1, addr2, ...`

If A equals 1, the subroutine at the address provided as the first argument is called.  
If A equals 2, 3, 4, etc., the subroutine at the corresponding address is called in sequence.  
The address list can contain up to 42 entries  
If A is zero or greater than the number of specified addresses, no subroutine is called, and execution continues past the macro.  
Once the called subroutine completes, execution resumes immediately after the macro.  

### `on_A_jsr0`

**Syntax:** `on_A_jsr0 addr1, addr2, ...`

Similar to on_A_jsr, but uses 0-based indexing.  
If A equals 0, the subroutine at the first address is called.  
If A equals 1, 2, 3, etc., the corresponding subroutine at the given address is called in sequence.  
The address list can contain up to 43 entries  
If the value of A exceeds the number of provided addresses, no subroutine is called, and execution continues past the macro.  

**Returns:** Once the subroutine returns, execution resumes immediately after the macro.

### `restore`

**Syntax:** `restore reg[,to,targetreg]`

**Registers modified: the restored register**

### `store`

**Syntax:** `store reg`

**Registers modified: none**

**Notes:**
- inbetween store and restore, the stored value can be also accessed via address stored_A, stored_X, or stored_Y. Note that a stored AX is just a stored A and a stored X. The two addresses stored_A and stored_X are not consecutive.

### `switch`

**Syntax:** `switch [A|X|Y|AX]`

...  
...  
...  
...  
Only a fallthrough into the default part works correctly.  
Example:  
**Alternate:** `switch A`

	case 1:  
	   print "one"  
	   break  
	case 2:  
	   print "two"  
	   break  
	case 3:  
	   print "one"  
	   break  
	default:  
	   print "?"  
	endswitch  

**Registers modified: none**

## Useful Routines

### `_ld_reg`

**Syntax:** `_ld_reg reg,arg`

For example, _ld_reg A,#12 translates into lda #12 while _st_reg $1234,Y translates into sty $1234  
If reg is blank, A is used as a default  
Those functions are mostly used in other macros.  

### `check_C128`

**Syntax:** `check_C128`

Detects if we are on a C128 in C64 mode  

**Returns:** Returns with carry set for C128

### `checksum_eor`

**Syntax:** `checksum_eor startaddr,endaddr`

**Returns:** Returns an 8-bit checksum calculated by EOR-conjunction over all bytes

### `clear_matrix`

**Syntax:** `clear_matrix array,width,height`

Clears a 2-dimensional matrix of size width x height, starting from the base address of array, by filling it with 0.  
The memory range from array to array + (width * height) is set to 0.  
Uses the memset macro to perform the operation.  

**Registers modified: A, X, (potentially Y if the total memory size exceeds 256 bytes)**

### `clear_window`

**Syntax:** `clear_window`

Clears the window that was defined by the window parametes of chrout2window and sets the cursor to the home position. When chrout2window is enabled, the same effect can be achieved by lda #147, jsr $FFD2  

### `clrscr`

**Syntax:** `clrscr`

Clears the screen  
KERNAL ROM needs to be enabled when using this function  

**Registers modified: A,Y,X**

### `decimal_flag_to_N`

**Syntax:** `decimal_flag_to_N`

Copies the decimal flag into the negative flag to detect if decimal mode is on  

**Returns:** Macro always returns with a cleared Carry

**Registers modified: A**

### `delay_cycles`

**Syntax:** `delay_cycles arg`

Delays for arg cycles using a busy waiting approach. This does not account for interrupts or stolen cycles by VIC badlines.  
arg must be a constant >=2  
generated code does not need to be aligned, but requires around 0.3 bytes/cycle in memory  

**Registers modified: none (but flags may be messed up)**

### `delay_ms`

**Syntax:** `delay_ms arg`

Delays for arg milliseconds using a busy waiting loop.  
The waiting loop is calibrated to avergage available CPU cycles on a C64 with VIC and interrupts enabled.  
Depending on interrupts or stolen cycles by VIC badlines the actual delay time may vary.  
When AX is given as argument it waits as many ms as the 16 bit value in AX  
When a number 1-65536 is given as argument it waits this long  

**Registers modified: A,X,Y**

### `delay_ms_abort_on_fire`

**Syntax:** `delay_ms_abort_on_fire arg`

Delays for arg milliseconds using a busy waiting loop.  
The waiting loop is calibrated to avergage available CPU cycles on a C64 with VIC and interrupts enabled.  
Depending on interrupts or stolen cycles by VIC badlines the actual delay time may vary.  
When AX is given as argument it waits as many ms as the 16 bit value in AX  
When a number 1-65536 is given as argument it waits this long  

**Returns:** If firebutton was detected, the loop is aborted and the carry flag is cleared upon return

**Registers modified: A,X,Y**

### `disable_cbm_shift`

**Syntax:** `disable_cbm_shift`

disable Commodore-Shift  

### `disable_chrout2window`

**Syntax:** `disable_chrout2window`

Restores the original Kernal vector and disables the chrout2window mode  

### `disable_NMI`

**Syntax:** `disable_NMI`

Executes a short routine to disable the NMI  
the trick is to cause an NMI but don't ackowledge it  
Uses CIA2 Timer A, but the timer can be used afterwards (without IRQ function)  
Interrupt flag will be turned off while executing the routine and reset to the previous state afterwards  

**Registers modified: A**

### `diskstatus`

**Syntax:** `diskstatus devicenr`

This code only reads the number without further text  
**Alternate:** `Error codes (listed in decimal) are:`

0   OK, no error exists  
1   Files scratched response. Not an error condition  
20  Block header not found on disk  
21  Sync character not found  
22  Data block not present  
23  Checksum error in data  
24  Byte decoding error  
25  Write-verify error  
26  Attempt to write with write protect on  
27  Checksum error in header  
28  Data extends into next block  
29  Disk id mismatch  
30  General syntax error  
31  Invalid command  
32  Long line  
33  Invalid filename  
34  No file given  
39  Command file not found  
50  Record not present  
51  Overflow in record  
52  File too large  
60  File open for write  
61  File not open  
62  File not found  
63  File exists  
64  File type mismatch  
65  No block  
66  Illegal track or sector  
67  Illegal system track or sector  
70  No channels available  
71  Directory error  
72  Disk full or directory full  
73  Power up message, or write attempt with DOS Mismatch  
74  Drive not ready  

**Returns:** Returns the disk status error message number in A

### `div16`

**Syntax:** `div16 arg`

**Alternate:** `div16 #arg`

Divides the unsigned 16 bit value in AX by an immediate value or the 16 bit value stored at addr (lo-byte) and addr+1 (hi-byte)  
Implemented as a subroutinge, link with -lib lamalib.lib  
When using this function in interrupt save _div16_sr, _div16_rem, _div16_arg_lo, and _div16_arg_hi before calling and restore those values afterwards.  

**Returns:** Result is returned in AX

**Registers modified: all**

### `div16by8`

**Syntax:** `div16by8 arg`

**Alternate:** `div16by8 #arg`

Divides the unsigned 16 bit value in AX by an immediate value or the 8 bit value stored at addr  

**Returns:** Quotient is returned in A, remainder in X

**Registers modified: A,X**

### `div8`

**Syntax:** `div8 arg`

**Alternate:** `div8 #arg`

Divides the unsigned 8 bit value in A by an immediate value or the 8 bit value stored at addr  

**Returns:** Quotient is returned in A, remainder in X

**Registers modified: A,X**

### `draw_frame`

**Syntax:** `draw_frame [x1 [, y1 [, x2 [, y2 [, color]]]]]`

Draws a frame around the window defined by the window parameters  
When using in interrupt, save contents of _llzp_word1,_llzp_word2 before calling and restore afterwards  
Window parameters:  
Further configuration parameters (default is a white frame using PETSCII characters):  
.import _frame_upper_right  
.import _frame_lower_left  
.import _frame_lower_right  
.import _frame_vertical  
.import _frame_horizontal  
.import _frame_color  
For example to change the lower left corner to a rounded one, add  
poke _frame_lower_left,74  

**Notes:**
- Note that the frame will go around the window, so it is larger than the defined window

### `enable_cbm_shift`

**Syntax:** `enable_cbm_shift`

enable Commodore-Shift, this key combination toggles between the uppercase/graphics and lowercase/uppercase character set  

### `enable_chrout2window`

**Syntax:** `enable_chrout2window`

Switches the Kernal chrout vector to a routine that prints within a window  
The page of the textscreen (stored in $288 / 648) is used to determine the output screen, but if you change the screen page, enable_chrout2window needs to be called again.  
Limitations: no backspace, no insert  
Window parameters:  
For example to set a window starting on column 5, write  
poke _window_x1,5  

**Notes:**
- Note that control character keypresses in direct mode are not handled via $FFD2, therefore pressing for example CLR/HOME will leave the window in direct mode.

### `fill_matrix`

**Syntax:** `fill_matrix array,width,height,fillvalue`

Fills a 2-dimensional matrix of size width x height, starting from the base address of array, with the byte fillvalue.  
The memory range from array to array + (width * height) is filled.  

**Registers modified: A, X, (potentially Y if the total memory size exceeds 256 bytes)**

### `fill_window`

**Syntax:** `fill_window A|char|#char`

Fills the window with the given char and the current textcolor. When using $20 as char this function works similar to clear_window, but the cursor position is not affected.  

### `force_load_timerA`

**Syntax:** `force_load_timerA [CIA base address]`

Makes timer A of the Complex Interface Adapter (CIA) load the latch value  
If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be passed as second argument.  

### `get_matrix_element`

**Syntax:** `get_matrix_element array,col,row`

Gets the element indexed by col and row from the 2-dimensional array.  
Needs tables with suffix _row_lo and _row_hi to be defined with macro matrix_ptr_tables.  

**Registers modified: A. If col is a constant, the X register is used.**

### `getkey`

**Syntax:** `getkey`

Function depends on Kernal ROM and the IRQ routine regularily scanning the keyboard  

**Returns:** clears the key buffer, waits for a keypress and returns the ASCII value of the pressed key in A

### `identify_SID`

**Syntax:** `identify_SID [baseaddress]`

Detects the SID soundchip model  
SID detection routine from codebase64 by SounDemon and a tip from Dag Lem  
If no base address is given, the standard base address $d400 is used  
Carry flag is set for 6581, and clear for 8580  

**Returns:** Result is returned in carry

### `include_file_as`

**Syntax:** `include_file_as filename,identifier`

This macro helps in linking external files to the project that should be copied into a target area later or accessed directly  
filename should be given as a quoted string. If the filename ends with ".prg" it is assumed that the first two byter are the loading address, these bytes are skipped for the calculation of start address and length  
Example: include_file_as "sprites.bin",sprites  
(identifier)_filestart contains the first address where the file was included (in the example that would be sprites_filestart)s  
(identifier)_filestart contains the last address (+1) where the file was included  
(identifier)_length contains the length in bytes  
This macro is convenient for small projects, consider using a linker for projects with many different large components  

### `install_file`

**Syntax:** `install_file identifier[,target_addr]`

If identifier is a string, the file with that name is imported here and copied to its start address,  
or if a target address is given to target_addr.  
If the filetype is different from PRG, target_addr must be specified.  
If identifier is not a string it refers to a previously included file. In this case, there must be a  
command include_file_as filename,identifier placed before calling install_file.  

### `install_irq_catcher`

**Syntax:** `install_irq_catcher`

install_irq_brk_catcher  
Installs an irq vector in $FFFE/$FFFF pointing to a routine that catches the IRQ, turns on Kernal ROM, calls the ISR via the vector $314/315, and restores the previous state after the isr terminates.  
This allows to use different memory configurations without the need to turn off the interrupt.  
The install_irq_brk_catcher uses code that distinguishes BRK from an IRQ. Since this costs some cycles, this should be used only if BRK is used in your code.  

### `is_alpha`

**Syntax:** `is_alpha`

**Alternate:** `is_alpha tests if value in Accu is between the values 'a' and 'z' (in lowercase mode).`


**Returns:** Return value: Carry set if value is in range, carry cleared otherwise

**Registers modified: none**

### `is_digit`

**Syntax:** `is_digit`

**Alternate:** `is_digit tests if value in Accu is between the values '0' and '9'.`


**Returns:** Return value: Carry set if value is a digit, carry cleared otherwise

**Registers modified: none**

### `is_in_range`

**Syntax:** `is_in_range lower,higher`

Tests if value in Accu is between the values lower and higher  

**Returns:** Return value: Carry set if value is in range, carry cleared otherwise

**Registers modified: none**

### `is_in_range_trash_A`

**Syntax:** `is_in_range_trash_A lower,higher`

Tests if value in Accu is between the values lower and higher  
If the value was inside, the Carry is set, otherwise the Carry is cleared  

**Registers modified: A**

### `is_not_alpha`

**Syntax:** `is_not_alpha`

is_alpha tests if value in Accu is not between the values 'a' and 'z' (in lowercase mode).  

**Returns:** Return value: Carry set if value is outside range, carry cleared otherwise

**Registers modified: none**

### `is_not_digit`

**Syntax:** `is_not_digit`

**Alternate:** `is_not_digit tests if value in Accu is not between the values '0' and '9'.`


**Returns:** Return value: Carry clear if value is a digit, carry set otherwise

**Registers modified: none**

### `is_not_in_range`

**Syntax:** `is_not_in_range lower,higher`

Tests if value in Accu is outside the values lower and higher  

**Returns:** Return value: Carry set if value is outside range, carry cleared otherwise

**Registers modified: none**

### `load_prg`

**Syntax:** `load_prg filename[,devicenr[,loadaddr]]`

Wrapper around ROM load function, prg means that the file is assumed to have a two-byte load address at its start  
filename can be a string or a pointer in AX  
If device number is 0 or not stated, the last used device numner stored in address $BA is used. In case $BA contains 0, 8 is used as default  
If loadaddr is omitted, the load address is defined by the first two bytes of the file  
**Alternate:** `A = $05 (DEVICE NOT PRESENT)`

**Alternate:** `A = $04 (FILE NOT FOUND)`

**Alternate:** `A = $1D (LOAD ERROR)`

A = $00 (BREAK, RUN/STOP has been pressed during loading)  

**Returns:** Return value in carry, if carry is set, an error has happened and error code is returned in A:

**Registers modified: A,X,Y**

### `lowercase_mode`

**Syntax:** `lowercase_mode`

Switches charset to upper/lowercase (text) mode setting and locks the CBM+Shift switch  
To switch back, use the macro PETSCII_mode  

**Registers modified: A**

### `makesys`

**Syntax:** `makesys [linenumber[,text[, address]]]`

Generates the code for a BASIC line with a SYS command, an optional text behind the sys command, and an optional specified entry address  
This is similar to the command line option  -u __EXEHDR__  
Difference is that with this function the code segment starts at $801, so .align is off only by 1 and the string encoding for the text supports escape characters like "\n" or "\xHH" with HH being a hexadecimal number.  
If no SYS address is given, the target address is calculated to be the address right after the BASIC stub  
Default line number is 2020  

### `matrix_ptr_tables`

**Syntax:** `matrix_ptr_tables array,width,height`

Generates two pointer tables derived from the array name, one with suffix _row_lo and one with suffix _row_hi.  
Each matrix entry is considered to be a byte.  
These tables are referenced by other matrix macros, for example, get_matrix_element and set_matrix_element.  

### `memcopy`

**Syntax:** `memcopy src_addr,target_addr,length`

Copies the memory area src_addr to src_addr+length over target_addr  
If the areas are overlapping, then target_addr must be < src_addr  
The three parameter version takes three constant numbers, alternatively, the function can be configured parameter by parameter, either with AX or a constant  
When using this function in interrupt save _llzp_word1 and _llzp_word2 before calling and restore those values afterwards.  

**Registers modified: A,X,Y**

### `memset`

**Syntax:** `memset start_addr,end_addr[,fillvalue]`

Fills the memory range from start_addr to end_addr with the byte fillvalue.  
This macro is interrupt-safe and can be used concurrently in both the main program and an interrupt routine.  

**Registers modified: A, X, (Y if the memory size to fill exceeds 256 bytes)**

### `memswap`

**Syntax:** `memswap src_addr,target_addr,length`

Swaps length bytes of memory starting at src_addr and target_addr, respectively.  
Areas must not overlap.  
The three parameter version takes three constant numbers, alternatively, the function can be configured parameter by parameter, either with AX or a constant  
When using this function in interrupt save _llzp_word1 and _llzp_word2 before calling and restore those values afterwards.  

**Registers modified: A,X,Y**

### `mod16`

**Syntax:** `mod16 arg`

**Alternate:** `mod16 #arg`

Implemented as a subroutinge, link with -lib lamalib.lib  
When using this function in interrupt save _div16_sr, _div16_rem, _div16_arg_lo, and _div16_arg_hi before calling and restore those values afterwards.  

**Returns:** Result is returned in AX

**Registers modified: all**

### `mul16`

**Syntax:** `mul16 addr`

compactmul16 addr  
Multiplies the unsigned 16 bit value in AX with the 16 bit value stored at addr (lo-byte) and addr+1 (hi-byte)  
Implemented as a subroutinge, link with -lib lamalib.lib  
**Alternate:** `mul16 adds a routine of 51 byte to your program the first time you use it`

compactmul16 adds a routine of 32 byte to your program the first time you use it  
**Alternate:** `mul16 is about 20% faster than compactmul, we recommend using mul16 in most cases`

When using this function in interrupt save _llzp_word1 and _llzp_word2 before calling and restore those values afterwards.  

**Returns:** Result is returned in AX

**Registers modified: A,X,Y**

### `newline`

**Syntax:** `newline`

Prints a newline character  
KERNAL ROM needs to be enabled when using this function  

**Registers modified: A**

### `PETSCII_mode`

**Syntax:** `PETSCII_mode`

Switches charset to uppercase plus graphical characters (graphics mode) setting and locks the CBM+Shift switch  
To switch to upper/lowercase (text) mode, use the macro lowercase_mode  

**Registers modified: A**

### `primm`

**Syntax:** `primm str`

Prints the given string, string is inlined in program code  
Uses ROM functions, BASIC and KERNAL ROM need to be enabled when using this macro  
The string encoding supports escape characters like "\n" or "\xHH" with HH being a hexadecimal number.  

**Registers modified: A,Y,X**

### `print`

**Syntax:** `print arg1 [arg2 ...]`

An argument in parenthesis will print the 16bit value stored at this address  
uses ROM functions, BASIC and KERNAL ROM need to be enabled when using this macro  
If USE_BASIC_ROM .set 0 was done before, the BASIC ROM functions are replaced by onw implementations. This creates larger but also fatser code.  

**Returns:** Use .FEATURE STRING_ESCAPES to enable escapes codes like "\x05" (white) or "\0x0a" (carriage return)

**Registers modified: none**

### `print_wrapped`

**Syntax:** `print_wrapped arg`

Write arg to the screen with word wrapping. arg can be a string, an address or AX as a pointer to a null-terminated string.  
Left margin and width of print window are defined via print_wrapped_setpars  
Default parameters for the printing window are 0,40, meaning full screen  
The printing starts at the current cursor position, to support successive print_wrapped calls.  
The string encoding supports escape characters like "\n" or "\xHH" with HH being a hexadecimal number.  
If unsure about the cursor position, it is suggested to place the cursor with set_cursor_pos beforehand.  

### `print_wrapped_setpars`

**Syntax:** `print_wrapped_setpars [x1],[width],[endchar]`

set parameters for print_wrapped command  
x1     first column of print window  
width  width of print window in characters  
endchar character to print after print_wrapped command, typical values are 13, 32, 0 (no extra char)  
If not all parameter should be changed, the others can be omitted with commas, e.g. print_wrapped_setpars ,,13 sets only the endchar  

### `printa`

**Syntax:** `printa`

Prints the number in A as a 8 bit unsigned decimal number  
Does not use BASIC or KERNAL ROM functions  

**Registers modified: A,X**

### `printax`

**Syntax:** `printax`

Prints the number in AX as a 16 bit unsigned decimal number  
Does not use BASIC or KERNAL ROM functions  

**Registers modified: A,Y,X**

### `printax_signed`

**Syntax:** `printax_signed`

Prints the number in AX as a 16 bit signed decimal number  
Does not use BASIC or KERNAL ROM functions  

**Registers modified: A,Y,X**

### `println`

**Syntax:** `println arg1 [arg2 ...]`

An argument in parenthesis will print the 16bit value stored at this address  
uses ROM functions, BASIC and KERNAL ROM need to be enabled when using this macro  
If USE_BASIC_ROM .set 0 was done before, the BASIC ROM functions are replaced by onw implementations. This creates larger but also fatser code.  

**Returns:** Use .FEATURE STRING_ESCAPES to enable escapes codes like "\x05" (white) or "\0x0a" (carriage return)

**Registers modified: none**

### `printstr`

**Syntax:** `printstr addr`

Prints the null-terminated string at addr using the STROUT function  
BASIC and KERNAL ROM need to be enabled when using this function  

**Registers modified: A,Y,X**

### `rand16`

**Syntax:** `rand16 [maxvalue-1]`

Get a random number in AX betwenn 0 and 0xFFFF  
The argument is optional and can be a number or AX  
If an argument is given, the value is caclulated between 0 and given number-1  
This function uses the same routine as rand8, therefore calling rand16 will change the state of rand8 as well  
When using this function in interrupt save _llzp_word1 and _llzp_word2 before calling and restore those values afterwards.  

**Registers modified: A,X,Y (Y is only used when an argument is used)**

### `rand8`

**Syntax:** `rand8 [maxvalue-1]`

If an argument is given, the value is caclulated between 0 and given number-1  
When using this function in interrupt save _llzp_word1 before calling and restore those values afterwards.  

**Returns:** The function uses a 16-bit 798 Xorshift algorithm and returns the lower byte from its state.

**Registers modified: A,Y (Y is only used when an argument is used)**

### `rand_setseed`

**Syntax:** `rand_setseed AX | [[arg1|A|X|Y], arg2|A|X|Y]`

When setting with AX, there is a check to avoid the seed 0 which would cause the PRNG to lock up. The seed affects both, rand8 and rand16 functions.  

**Registers modified: normally none, except for the case where lo(AX)==0, then A is set to $FF**

### `randomize`

**Syntax:** `randomize [timer][,raster][,sid]`

Sets the seed of the pseudo random number generator using entropy from the system.  
When argument "timer" is given, the current timer value is used.  
"raster" uses the value in $D012, containing the current rasterline.  
"sid" uses the value from the SID noise generator voice #3  
If multiple arguments are given tha values are combined with EOR.  
On a VIC-20, only "timer" is supported.  
If no argument is given, all suported values are combined and used.  

### `read_keys_ACSEP`

**Syntax:** `read_keys_ACSEP`

Reads keys @, :, ;, =, P  
The keys are interpreted as a four-way control like WASD, P is the fire key  
Routine should not be interrupted by another keyboard scan, therefore it is  
recommended to:  
- run the function in the interrupt, or  
- have the keyboard scan in the interrupt turned off, or  
- put a sei / cli around the function call  
Function is independent of the Kernal keyboard routine  

**Returns:** Return value is in A, where the respective bit is zero if the key is pressed

**Registers modified: A**

### `read_keys_CACFFPMX`

**Syntax:** `read_keys_CACFFPMX`

Reads keys CBM, Arrowleft, Cursor right, F7, F1, +, M, X  
Routine should not be interrupted by another keyboard scan, therefore it is  
recommended to:  
- run the function in the interrupt, or  
- have the keyboard scan in the interrupt turned off, or  
- put a sei / cli around the function call  
Function is independent of the Kernal keyboard routine  

**Returns:** Return value is in A, where the respective bit is zero if the key is pressed

**Registers modified: A**

### `read_keys_WASDspace`

**Syntax:** `read_keys_WASDspace`

Checks the keyboard for keypresses of W, A ,S, D and Space  
Output is a byte in A in the same format as a joystick value  
Function is independent of the Kernal keyboard routine  

**Returns:** Since movement of joystick 1 disturbs the reading, no keys are returned if joystick 1 is moved in any direction

**Registers modified: A,X**

### `read_timerA`

**Syntax:** `read_timerA [AX[,cia base address]]`

Reads the current timer value and puts it into AX  
If the high byte changes during the reading, the reading is done again  

### `restore_screen_area`

**Syntax:** `restore_screen_area addr`

Restores characters and colors according to the stored dimensions of the window  
The screen address is derived from memory address 648 (or the respective address with that function on the C128 or the VIC20).  
Don't use this function in the interrupt service routine and in the main program at same time.  

**Registers modified: A,X,Y**

### `save_prg`

**Syntax:** `save_prg filename,devicenr,startaddr,endaddr`

Saves the memory from startaddr to endaddr (including the endaddr) using the CBM ROM save function.  
The file will have a two-byte load address at its start  
If device number is 0, the last used device numner stored in address $BA is used. In case $BA contains 0, 8 is used as default  
**Alternate:** `A = $05 (DEVICE NOT PRESENT)`

A = $00 (BREAK, RUN/STOP has been pressed during saving)  

**Returns:** Return value in carry, if carry is set, an error has happened and error code is returned in A, for example

**Registers modified: A,X,Y**

### `save_screen_area`

**Syntax:** `save_screen_area x1,y1,x2,y2,targetaddr`

Saves characters and color of the specified screen window to the given memory address.  
The screen address is derived from memory address 648 (or the respective address with that function on the C128 or the VIC20).  
The window parameters are saved as well.  
The address _save_screen_addr contains a pointer to the next free byte after the saved bytes at targetaddr  
Don't use this function in the interrupt service routine and in the main program at same time.  

**Registers modified: A,X,Y**

### `scramble`

**Syntax:** `scramble startaddr,endaddr`

Scrambles/unscrambles a memory area. To unscramble, call scramble with the same parameters again  

### `scratch_file`

**Syntax:** `scratch_file filename[,devicenr]`

Deletes a file on disk. If device number is not stated or 0, the last used device number stored in address $BA is used. In case $BA contains 0, 8 is used as default.  

**Registers modified: A,X,Y**

### `screen_off`

**Syntax:** `screen_off`

turns off the screen via $d011  
The function always sets a 0 as high bit to avoid an unreachable raster irq line.  

### `screen_on`

**Syntax:** `screen_on`

turns the screen back on. Since it is necessary to write to $d011, the high bit of the next raster IRQ will be reset to 0 as a side effect  

### `set_cursor_pos`

**Syntax:** `set_cursor_pos line,column`

place the cursor at screen position line,column (counted in characters)  
0,0 is upper left corner, 24,39 the lower right corner  

### `set_irq_rasterline`

**Syntax:** `set_irq_rasterline rasterline|A|AX|X|Y`

Sets the raster line where the IRQ should occur. This macro does not set the IRQ source, use set_raster_irq for this.  

### `set_isr_for_stabilize_raster_cycle`

**Syntax:** `set_isr_for_stabilize_raster_cycle`

Puts the isr address of the IRQ wedge routine of stabilize_raster_cycle into $FFFE/$FFFF  
When using the raster stabilization from a Kernal IRQ ($314) this macro should be used once during setup  
When using the raster stabilization with banked out Kernal, this macro should be always before the stabilize macro (or stabilize_raster_cycle_with_isr_set is used, which puts both macros together).  
When you have multiple instances of the stabilization routine, the macros refering to each other should be put into a named scope under the same name.  

### `set_matrix_element`

**Syntax:** `set_matrix_element array,col,row[,value]`

Sets the element indexed by col and row from the 2-dimensional array to the value stored in A.  
Needs tables with suffix _row_lo and _row_hi to be defined with macro matrix_ptr_tables.  
If value is not specified or value is A, the content of A is used as the value.  

**Registers modified: A (if value is a constant), X (if col is a constant).**

### `set_raster_irq`

**Syntax:** `set_raster_irq rasterline[, isr]`

Changes the IRQ source to VIC raster interrupt  
Turns off the CIA IRQ source  
If isr is given, the address is set in Kernals IRQ vector $314/315  
IRQ routine must acknowledge the IRQ source  
Interrupt flag will be turned off while executing the macro and reset to the previous state afterwards  

### `set_timerA`

**Syntax:** `set_timerA value|AX[,CIA base address]`

Sets the timer A latch value of the Complex Interface Adapter (CIA) and makes it load this value  
If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be passed as second argument.  

### `set_timerA_latch`

**Syntax:** `set_timerA_latch value|AX[,CIA base address]`

Sets the timer A value of the Complex Interface Adapter (CIA)  
If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be passed as second argument.  

**Notes:**
- Note that the value specifies the reload value of the timer, not the current counter value

### `set_timerB_latch`

**Syntax:** `set_timerB_latch value|AX[,cia base address]`

Sets the timer B value of the Complex Interface Adapter (CIA)  
If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be specified.  

**Notes:**
- Note that the value specifies the reload value of the timer, not the current counter value

### `sqrt16`

**Syntax:** `sqrt16 [arg]`

**Alternate:** `sqrt16 #arg`

Calculates the squareroot of the argument or, if no argument is given, of AX  
Implemented as a subroutinge, link with -lib lamalib.lib  
When using this function in interrupt save _llzp_word1 before calling and restore those values afterwards.  

**Returns:** Result is returned in A, X will contain the remainder

**Registers modified: A,X,Y**

### `sqrt8`

**Syntax:** `sqrt8 [arg]`

**Alternate:** `sqrt8 #arg`

Calculates the squareroot of the argument or, if no argument is given, of A  
Implemented as a subroutinge, link with -lib lamalib.lib  
When using this function in interrupt save _llzp_word1 before calling and restore those values afterwards.  

**Returns:** Result is returned in A, X will contain the remainder

**Registers modified: A,X,Y**

### `stabilize_raster_cycle`

**Syntax:** `stabilize_raster_cycle [extranops]`

Syncs to raster cycle 3 at current raster line+2 (cycle perfect)  
The current and the next two rasterlines must not be badlines  
Uses the double IRQ method with a NOP slide and a conditional jump to eliminate jitter  
Assembling may fail if the conditional branch goes of a page boundary, enable the option extranops in this case.  
Minimal example showing a typical usage:  
        set_raster_irq 48,isr  
        rts  
isr:    poke 1,$35  
        stabilize_raster_cycle  
        ; we are perfectly synchronized, do stuff  
        poke 1,$37  
        set_irq_rasterline 48  
        asl $d019  
        jmp $ea31  

### `stabilize_raster_cycle_with_isr_set`

**Syntax:** `stabilize_raster_cycle_with_isr_set`

Combination of set_irq_for_stabilize_raster_cycle and stabilize_raster_cycle  
This macro is useful when you have your interrupt service routine called via $FFFE/$FFFF  
The macros are encapsuled into a common scope, so that this macro can be used multiple times.  
Minimal example showing a typical usage:  
        set_raster_irq 48  
        pokew $fffe,isr  
        poke 1,$35  
        cli  
:       jmp :-  
isr:    pha  
        txa  
        pha  
        tya  
        pha  
        stabilize_raster_cycle_with_isr_set  
        ; we are perfectly synchronized, do stuff  
        set_irq_rasterline 48  
        asl $d019  
        pokew $fffe,isr  
        pla  
        tay  
        pla  
        tax  
        pla  
        rti  

### `start_timerA`

**Syntax:** `start_timerA [CIA base address]`

Starts timer A of the Complex Interface Adapter (CIA)  
If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be passed as second argument.  

### `stop_timerA`

**Syntax:** `stop_timerA [CIA base address]`

Stops timer A of the Complex Interface Adapter (CIA)  
If no base address is specified, the base address $DC00 (CIA #1) is used. For CIA #2, a based address of $DD00 must be passed as second argument.  

### `str_enc`

**Syntax:** `str_enc string`

Encodes the given string while respecting escape codes like \n and \xHH, where HH is the hexadecimal code of the character to be inserted.  
This macro serves as an alternative to using .feature string_escapes  
When using .feature string_escapes, string escapes are converted to platform-specific characters in the same way that other characters are converted.  
To avoid this issue, this macro directly parses and converts string escapes into numerical .byte values.  
Application example: str_enc "\x93\x05hello world in white after a clear screen\n"  

**Notes:**
- Note: If .feature string_escapes is enabled, only the escape codes compatible with it will be processed.

### `str_enc0`

**Syntax:** `str_enc0 string`

Same as str_enc, but the string will be terminated with a 0 byte  

### `sync_to_rasterline256`

**Syntax:** `sync_to_rasterline256`

A sturdy version of waiting for the rasterline transition from 255 to 256.  
Does busy waiting and comes with some jitter.  
Might be delayed if an IRQ occurs, but will still trigger.  
see also wait_for_rasterline  

**Registers modified: none, affects N,V,Z flags**

### `textcolor`

**Syntax:** `textcolor color`

sets the text color  

### `to_scrcode`

**Syntax:** `to_scrcode`

Control codes will lead to an arbitrary byte.  

**Returns:** If the PETSCII value in A belongs to a printable character, it is converted to the corresponding screencode and returned in A

### `toggle_carry`

**Syntax:** `toggle_carry`

Toggles the carry  

**Registers modified: none**

### `turn_off_cursor`

**Syntax:** `turn_off_cursor`

Turns off the blinking cursor and restores the character and color under cursor if necessary.  
Requires the KERNAL IRQ routines to be active  
If this function is used on the C128, a bank with visible ROM must be active.  
Use this before output of a char, otherwise you get inverse character artifacts when blink phase is on  

### `turn_on_cursor`

**Syntax:** `turn_on_cursor`

Shows the blinking cursor during program execution  
Requires the KERNAL IRQ routines to be active  
If this function is used on the C128, a bank with visible ROM must be active.  
Turn off cursor briefly before output of a char, otherwise you get inverse character artifacts when blink phase is on  

### `wait_firebutton`

**Syntax:** `wait_firebutton [joy]`

Waits until fire button is pressed. If the fire button is already pressed, the command continues immediately.  
Argument can be 1, 2 or 3 for both joysticks. If no argument is given, Joystick 2 is used.  

**Registers modified: A**

### `wait_firebutton_released`

**Syntax:** `wait_firebutton_released [joy]`

Waits until fire button is released. If the fire button is already released the command continues immediately.  
Argument can be 1, 2 or 3 for both joysticks. If no argument is given, Joystick 2 is used.  

**Registers modified: A**

### `wait_for_rasterline`

**Syntax:** `wait_for_rasterline rasterline[,reg]`

Macro inserting code doing busy waiting until the given rasterline is reached  
rasterline can be a value between 0 and 311 (for PAL systems) or 261 for NTSC systems, respectively  
The routine does not turn off the IRQ, so an IRQ might make it miss the rasterline it is waiting for.  
see also sync_to_rasterline256  

**Registers modified: A or the register (A,X, or Y) given as second argument**

### `wait_key`

**Syntax:** `wait_key`

Function works independly of IRQ routine or ROM  

**Returns:** Waits until a key is pressed, no meaningful return value. If a key is already pressed, the command continues immediately.

**Registers modified: A**

### `wait_key_or_button`

**Syntax:** `wait_key_or_button`

Waits until either a key is pressed or a joystick button is pressed.  
If either is already pressed, the command continues immediately.  
Function works independently of IRQ routine or ROM.  

**Registers modified: A**

### `wait_key_or_button_released`

**Syntax:** `wait_key_or_button_released`

Waits until both all keys and joystick buttons are released.  
If both are already released, the command continues immediately.  
Function works independently of IRQ routine or ROM.  

**Registers modified: A**

### `wait_key_released`

**Syntax:** `wait_key_released`

Waits until all keys are released. If no key is already pressed, the command continues immediately.  
Function works independly of IRQ routine  

**Registers modified: A**

### `wait_screen_off`

**Syntax:** `wait_screen_off`

Waits until raster>249 and turns screen off. Since it is necessary to write to $d011 to turn the screen off, we cannot avoid setting the high bit.  
The function always sets a 0 as high bit to avoid an unreachable raster irq line.  

## String Routines

### `strlen16`

**Syntax:** `strlen16 address`

**Alternate:** `strlen16 AX`


**Returns:** Return value in AX

### `strlen8`

**Syntax:** `strlen8 address`

**Alternate:** `strlen8 AX`

String maximum length is 255, use strlen16 to handle longer strings  

**Returns:** Return value in X

## Interacting with BASIC

### `getAddrOfBasicArryVar`

**Syntax:** `getAddrOfBasicArryVar varnam,arrayidx`

The arrayidx must be not larger than decimal 10 for arrays to be created  
Example usage:   
uses zero page addresses $0b, $0c, $0d, $0e, $45, $46  
on C128 it uses zero page addresses $0d, $0e, $0f, $10, $47, $48  

**Returns:** On the C128, the bank will be set to 1 after return, so this code needs to be in common RAM or in bank 1.

### `setBasicVarnam`

**Syntax:** `setBasicVarnam "varnam"`

This macro puts name and type of current variable into $45/46 (on C128 $47/48) in preparation for using ROM routines  
The varname can be of 1 or 2 characters length plus an optional $ or % to indicate string or integer variables  
Example usage:   
**Alternate:** `setBasicVarnam "ab%" sets the integer variable ab%`

**Alternate:** `setBasicVarnam "s$" sets the string variable s$`

**Alternate:** `setBasicVarnam "x1" sets the floating point variable x1`


## Special Macros for C128 in C128 Mode

### `set_VIC_RAMbank`

**Syntax:** `set_VIC_RAMbank bank`

Tell the MMU to feed RAM bank 0 or 1 to the VIC  
This is a C128-only feature  
Bank must be 0 or 1  
I/O must be enabled for this macro to work  

### `shadowIRQ`

**Syntax:** `shadowIRQ off|on`

screen. It further handles the BASIC commands SOUND, PLAY, and SPRITE. To  
avoid this, the macro  shadowIRQ off puts a 0 into memory address $0A04,   
telling the Kernal that BASIC has not been initialized yet.  
Cutting the IRQ routine provides a speed gain of about 2.5%  

**Registers modified: A**

## Other Macros

### `disableMultiColorSprite`

**Syntax:** `disableMultiColorSprite n`

Disable multicolor mode for sprite n  

**Registers modified: A**

### `disableXexpandSprite`

**Syntax:** `disableXexpandSprite n`

Disable horizontal expansion for sprite n  

**Registers modified: A**

### `disableYexpandSprite`

**Syntax:** `disableYexpandSprite n`

Disable vertical expansion for sprite n  

**Registers modified: A**

### `enableMultiColorSprite`

**Syntax:** `enableMultiColorSprite n`

Enable multicolor mode for sprite n  

**Registers modifi**Registers modified: X**

### `getSpriteY`

**Syntax:** `getSpriteY n,reg`

**Registers modified: A, X, Y**

### `hideSprite`

**Syntax:** `hideSprite n`

Hide sprite n  

**Registers modified: A**

### `setSpriteColor`

**Syntax:** `setSpriteColor n,arg`

Set the color for sprite n  

**Registers modified: A, X, or Y (depending on input)**

### `setSpriteCostume`

**Syntax:** `setSpriteCostume n,arg`

Set the costume for sprite n  
If arg is a value > 255, it is interpreted as the absolute address of the sprite data (needs to align to a 64 byte block)  

**Registers modified: A, X, Y**

### `setSpriteMultiColor1`

**Syntax:** `setSpriteMultiColor1 arg`

Set the first multicolor for sprites  

**Registers modified: A, X, Y**

### `setSpriteMultiColor2`

**Syntax:** `setSpriteMultiColor2 arg`

Set the second multicolor for sprites  

**Registers modified: A, X, Y**

### `setSpriteX`

**Syntax:** `setSpriteX n,arg`

Set the X position of sprite n  
arg can be a number or AX  

**Registers modified: A, X**

### `setSpriteXY`

**Syntax:** `setSpriteXY n,arg1,arg2`

Set the X and Y positions of sprite n  

**Registers modified: A, X, Y**

### `setSpriteY`

**Syntax:** `setSpriteY n,arg`

Set the Y position of sprite n  

**Registers modified: A, X, Y**

### `showSprite`

**Syntax:** `showSprite n`

Make sprite n visible  

**Registers modified: A**

### `spriteBeforeBackground`

**Syntax:** `spriteBeforeBackground n`

Make sprite n appear in front of the background  

**Registers modified: A**

### `spriteBehindBackground`

**Syntax:** `spriteBehindBackground n`

Make sprite n appear behind the background  

**Registers modified: A**

### `updateSpriteAttributes`

**Syntax:** `updateSpriteAttributes n`

Update attributes for sprite n  

**Registers modified: A, X, Y**


---

# Modules

LAMAlib modules are reusable, configurable components that can be included in your programs. Each module is configured using `def_const` parameters and included within a scope.

## bigcharout

**Version:** 0.2  
**Author:** a screen character (default: space ;* for 0, reverse space for 1).  

**Configuration Parameters:**

| Parameter | Default | Required | Description |
|-----------|---------|----------|-------------|
| `CHARSET_BASE` | `$3800` |  |  |
| `SCREEN_WIDTH` | `40` |  |  |
| `SET_PIXEL` | `160` |  | character to be used when a pixel is set, as screencode |
| `EMPTY_PIXEL` | `32` |  | character to be used when a pixel is empty, as screencode |
| `COLOR_SUPPORT` | `1` |  |  |
| `END_OF_SCREEN_CHECK` | `1` |  |  |

**Usage:**

```assembly
.scope bigcharout
  .include "modules/m_bigcharout.s"
.endscope

m_init bigcharout
m_run bigcharout
```

---

## copycharset

**Version:** 0.2  
**Author:** Wil  

**Configuration Parameters:**

| Parameter | Default | Required | Description |
|-----------|---------|----------|-------------|
| `CHARSET_SRC` | `$D000` |  | use $d800 to copy from upper/lower charset |
| `CHARSET_BASE` | `$3800` |  |  |
| `CHARSET_LENGTH` | `$800` |  |  |
| `EFFECT` | `0` |  | Effects: 1 italic |
| `EFFECT_RVS` | `0` |  | if 1 the modified chars will be placed instead of reverse chars |
| `MATCH_RVS` | `1` |  | match rvs chars |
| `MEM_CONFIG` | `51` |  | memory configuration ($1 value) during copying |

**Usage:**

```assembly
.scope copycharset
  .include "modules/m_copycharset.s"
.endscope

m_init copycharset
m_run copycharset
```

---

## mousedriver

**Version:** 0.1  

**Configuration Parameters:**

| Parameter | Default | Required | Description |
|-----------|---------|----------|-------------|
| `JOY_CONTROL` | `2` |  | 0..none, 1..joyport1, 2..joyport2, 4..wasdspace, combinations possible by adding the up the respective values |
| `MOUSE_CONTROL` | `1` |  | 0..no mouse control, 1..1351 mouse on port 1 |
| `MIN_X` | `24` |  |  |
| `MAX_X` | `343` |  |  |
| `MIN_Y` | `50` |  |  |
| `MAX_Y` | `249` |  |  |
| `CHECK_BOUNDS` | `1` |  |  |
| `INSTALL_SPRITES` | `$340` |  | if 0 no sprite is initialized, otherwise sprite pointer is copied to this address |
| `PLACE_SPRITE` | `1` |  |  |
| `OVERLAY` | `1` |  | if 1 then the pointer is shown as two sprites overlaying each other |
| `SPR_NUM` | `0` |  |  |

**Usage:**

```assembly
.scope mousedriver
  .include "modules/m_mousedriver.s"
.endscope

m_init mousedriver
m_run mousedriver
```

---

## PETSCII decode and display

**Version:** 4.00 February 2026  

**Configuration Parameters:**

| Parameter | Default | Required | Description |
|-----------|---------|----------|-------------|
| `DECODE_FROM_D000` | `0` |  | if 1 the compressed PETSCII can lie anywhere in RAM,including I/O area $D000-$DFFF |
| `ENABLE_TRANSPARENT` | `1` |  | if 1 a selectable character (default 0) will be treated as being transparent |
| `TRANSPARENT_CHARACTER` | `0` |  | index of the character treated as transparent |
| `TRANSPARENT_MODIFIERS` | `0` |  | adds procedures for disable_transparent and set_transparent screencode |
| `COMPACT_ZEROPAGE` | `0` |  | if 1 the module operates in a compact mode using only 2 zeropage addresses, resulting in 3% performance decrease |
| `TARGET_COLORMAP` | `0` |  | if 0 the value of $d800 is used as default |
| `TARGET_SCREEN` | `0` |  | if 0 the value in 648 is used as the high byte default value |
| `DISPLAY_BY_NUM` | `0` |  | if 1 a function display_by_num is added, taking the image number to display as argument |
| `PETSCIIDATA` | `petsciinum` |  | label to start of petscii data for display_by_num |
| `MEM_1_VALUE` | `$36` |  | value in address $1 during decode for display_by_num |

**Usage:**

```assembly
.scope PETSCII decode and display
  .include "modules/m_displayPETSCII.s"
.endscope

m_run PETSCII decode and display
```

---

## polychars

**Version:** 0.34  
**Author:** Wil  

This module provides dynamic multicolor character handling for the
Commodore 64, enabling the use of multiple character sets to display
characters beyond the standard 256-character limitation. Characters
are represented as 16-bit values, where the high byte specifies the
character set, and the low byte specifies the character index within
that set.

**Features:**
- Dynamic character usage and mapping:
- A table-driven system determines whether a character is already mapped to the current display character set.
- If unmapped, an unused character slot is dynamically allocated, and its bitmap is copied from the source character set.
- Efficient garbage collection reclaims character slots that are no longer displayed on screen.
- Transparent CHROUT integration:
- Includes hooks to modify the KERNAL's CHROUT function for seamless integration with BASIC and LAMAlib's print macros.
- Compatibility with multiple polychar instances:
- Supports displaying more than 256 characters by configuring independent instances of the module for different screen and character set regions.
- A maximum of 256 characters can be displayed on the screen at any given time for a single polychar instance.
- For multiple instances, a rasterline interrupt is required to switch VIC-II configurations.

**Configuration Parameters:**

| Parameter | Default | Required | Description |
|-----------|---------|----------|-------------|
| `NUM_CHARSETS` | `4` |  | number of source charsets |
| `CHARSETS_SRC` | — | ✓ | place where the charsets are stored (don't have to be mmory aligned) |
| `CHARSET_ADDR` | `$E000` |  | charset address |
| `SCREEN_ADDR` | `$C000` |  | address of the display screen |
| `SWITCHCODE1` | `133` |  | ASCII code of the switchcode for the charsets |
| `TABLES` | `$9000` |  | adress of translation tables, need to be visible when putchar is called |
| `SCREEN_START` | `SCREEN_ADDR` |  | defines the used screen area, typically the whole screen. Affects garbage collection scope and clear screen function |
| `SCREEN_END` | `SCREEN_ADDR+999` |  | addresses must be aligned to the begin of lines |

**Usage:**

```assembly
.scope polychars
  ; Set required parameters
  CHARSETS_SRC=value
  .include "modules/m_polychars.s"
.endscope

```

**API:**

```
m_init polychars                     ; Initializes tables and clears the screen
m_call polychars,enable_chrout_hook  ; Enables CHROUT hook
m_call polychars,disable_chrout_hook ; Disables CHROUT hook
m_call polychars,putchar             ; Prints a character (16-bit encoded)
m_call polychars,garbage_collection  ; Reclaims unused character slots
If you need more than 256 characters on the screen, use two polychar instances with different charset
addresses and translation tables. They can share the same source charsets.
Example for multi-instance use:
.scope polychar_upperscreen
CHARSETS_SRC=charsets
SCREEN_ADDR=$C000
CHARSET_ADDR=$E000
TABLES=$9000
.include "m_polychars.s"
.endscope
.scope polychar_lowerscreen
CHARSETS_SRC=charsets
SCREEN_ADDR=$C400
CHARSET_ADDR=$E800
TABLES=$9800
.include "m_polychars.s"
.endscope
In this case you can only use the chrout on one of the modules at the same time
you need to add a rasterline interrupt that switches between the two SCREEN_ADDR and SCREEN_ADDR
by modifying VIC register $d018
```

---

