; Addresses of used ROM routines

.if .def(__C64__)

PRESSED_KEY_MATRIX_CODE=$C5
MATRIX_CODE_NO_KEY=$40

VAR_FLAGS      = $0B
CURR_VAR_NAME  = $45
CHARS_IN_KEYBUF= $C6
CURSORX        = $D3
CURSORY        = $D6
REVERSE_MODE_SWITCH = $C7
CURRENT_OUTPUT_DEVICE_NUMBER = $9A

KEYBUF         = $277
TEXTCOLOR_ADDR = $286
PTRSCRHI       = $288 ;high byte of pointer to screen memory for screen input/output

PAL_NTSC_FLAG  = $2a6

BASICSTART=$0801

USE_BASIC_ROM .set 1  ;add USE_BASIC_ROM=0 to your code to make LAMAlib generate code without calls to BASIC ROM

STROUT= $AB1E ;outputs a string at addr AY
LINPRT= $BDCD ;outputs number in AX
FINDARRAYELEMENT=$B218	;part of routine $b1d1 to find or make an array

DEFAULT_OUTPUT=$F1CA
CLRSCR= $E544 ;clear screen

SCINIT= $FF81 ;Initialize VIC, clear screen
IOINIT= $FF84 ;Initialize CIAs, SID volume; setup memory configuration; set and start interrupt timer
RAMTAS= $FF87 ;clear ZP, run memory test
RESTOR= $FF8A ;Fill vector table at memory addresses $0314-$0333 with default values
VECTOR= $FF8D ;Copy vector table at memory addresses $0314-$0333
SETMSG= $FF90 ;Set system error display switch at memory address $009D
LSTNSA= $FF93 ;Send LISTEN secondary address to serial bus
TALKSA= $FF96 ;Send TALK secondary address to serial bus
MEMTOP= $FF99 ;Set/read BASIC RAM top
MEMBOT= $FF9C ;Set/read BASIC RAM bottom 
SCNKEY= $FF9F ;Scan keyboard; put current matrix code into $00CB, shift keys into $28D and update keyboard buffer
SETTMO= $FFA2 ;Set serial bus timeout to A
IECIN = $FFA5 ;Read byte from serial bus into A
IECOUT= $FFA8 ;Write byte A to serial bus
UNTALK= $FFAB ;Send UNTALK command to serial bus
UNLSTN= $FFAE ;Send UNLISTEN command to serial bus
LISTEN= $FFB1 ;Send LISTEN command to serial bus
TALK  = $FFB4 ;Send TALK command to serial bus
READST= $FFB7 ;Read status of current input/output device into A
SETLFS= $FFBA ;Set logical file parameters A = Logical number; X = Device number; Y = Secondary address
SETNAM= $FFBD ;Set file name parameters A = File name length; X/Y = Pointer to file name
OPEN  = $FFC0 ;Open file, must call SETLFS and SETNAM before
CLOSE = $FFC3 ;Close file A
CHKIN = $FFC6 ;Define file as default input
CHKOUT= $FFC9 ;Define file as default output
CLRCHN= $FFCC ;Close default input/output files
CHRIN = $FFCF ;Read byte from default input
CHROUT= $FFD2 ;Outputs the character in A to default output.
LOAD  = $FFD5 ;Load file, must call SETLFS and SETNAM before
SAVE  = $FFD8 ;Save file, must call SETLFS and SETNAM before
SETTIM= $FFDB ;Set Time of Day, at memory address $00A0-$00A2
RDTIM = $FFDE ;Read Time of Day to AX/Y
STOP  = $FFE1 ;Query Stop key indicator, at memory address $0091; if pressed, call CLRCHN and clear keyboard buffer
GETIN = $FFE4 ;get a character from keyboard
CLALL = $FFE7 ;Close all channels and files
UDTIM = $FFEA ;Increment real-time clock
SCRORG= $FFED ;Fetch number of screen rows and columns
PLOT  = $FFF0 ;Save or restore cursor position
IOBASE= $FFF3 ;Fetch CIA #1 base address


INITVIC = $E5A0
INITVEC = $FD15   

CIA1_PRA  = $DC00
CIA1_PRB  = $DC01
CIA1_DDRA = $DC02
CIA1_DDRB = $DC03
CIA1_TIMERA= $DC04
CIA1_TIMERB= $DC06
CIA1_ICR= $DC0D  ;Interrupt control reg
CIA1_CRA= $DC0E ;Control Timer A
CIA1_CRB= $DC0F ;Control Timer B

CIA2_PRA  = $DD00
CIA2_PRB  = $DD01
CIA2_DDRA = $DD02
CIA2_DDRB = $DD03
CIA2_TIMERA= $DD04
CIA2_TIMERB= $DD06
CIA2_ICR= $DD0D  ;Interrupt control reg
CIA2_CRA= $DD0E ;Control Timer A
CIA2_CRB= $DD0F ;Control Timer B

ISR_DEFAULT=$EA31
ISR_EXIT   =$EA81

.elseif .def(__C128__)

PRESSED_KEY_MATRIX_CODE=$D4
MATRIX_CODE_NO_KEY=$58

VAR_FLAGS      = $0d
CURR_VAR_NAME  = $47
CHARS_IN_KEYBUF= $D0
CURSORX        = $EC
CURSORY        = $EB
REVERSE_MODE_SWITCH = $f3
CURRENT_OUTPUT_DEVICE_NUMBER = $9A

KEYBUF         = $277
TEXTCOLOR_ADDR = $F1
PTRSCRHI       = $A3B ;high byte of pointer to screen memory for screen input/output

PAL_NTSC_FLAG  = $A03

BASICSTART=$1C01

USE_BASIC_ROM .set 1  ;add USE_BASIC_ROM=0 to your code to make LAMAlib generate code without calls to BASIC ROM

STROUT= $55E2 ;outputs a string at addr A/Y
LINPRT= $8E32 ;outputs number in AX
FINDARRAYELEMENT=$7CF4	;part of routine $7cAB to find or make an array

DEFAULT_OUTPUT=$EF79
CLRSCR= $C142 ;clear screen
INITVIC = $E170
INITVEC = $E056 

SCINIT= $FF81 ;Initialize VIC, clear screen
IOINIT= $FF84 ;Initialize CIAs, SID volume; setup memory configuration; set and start interrupt timer
RAMTAS= $FF87 ;clear ZP, run memory test
RESTOR= $FF8A ;Fill vector table at memory addresses $0314-$0333 with default values
VECTOR= $FF8D ;Copy vector table at memory addresses $0314-$0333
SETMSG= $FF90 ;Set system error display switch at memory address $009D
LSTNSA= $FF93 ;Send LISTEN secondary address to serial bus
TALKSA= $FF96 ;Send TALK secondary address to serial bus
MEMTOP= $FF99 ;Set/read BASIC RAM top
MEMBOT= $FF9C ;Set/read BASIC RAM bottom 
SCNKEY= $FF9F ;Scan keyboard; put current matrix code into $00CB, shift keys into $28D and update keyboard buffer
SETTMO= $FFA2 ;Set serial bus timeout to A
IECIN = $FFA5 ;Read byte from serial bus into A
IECOUT= $FFA8 ;Write byte A to serial bus
UNTALK= $FFAB ;Send UNTALK command to serial bus
UNLSTN= $FFAE ;Send UNLISTEN command to serial bus
LISTEN= $FFB1 ;Send LISTEN command to serial bus
TALK  = $FFB4 ;Send TALK command to serial bus
READST= $FFB7 ;Read status of current input/output device into A
SETLFS= $FFBA ;Set logical file parameters A = Logical number; X = Device number; Y = Secondary address
SETNAM= $FFBD ;Set file name parameters A = File name length; X/Y = Pointer to file name
OPEN  = $FFC0 ;Open file, must call SETLFS and SETNAM before
CLOSE = $FFC3 ;Close file A
CHKIN = $FFC6 ;Define file as default input
CHKOUT= $FFC9 ;Define file as default output
CLRCHN= $FFCC ;Close default input/output files
CHRIN = $FFCF ;Read byte from default input
CHROUT= $FFD2 ;Outputs the character in A to default output.
LOAD  = $FFD5 ;Load file, must call SETLFS and SETNAM before
SAVE  = $FFD8 ;Save file, must call SETLFS and SETNAM before
SETTIM= $FFDB ;Set Time of Day, at memory address $00A0-$00A2
RDTIM = $FFDE ;Read Time of Day to AX/Y
STOP  = $FFE1 ;Query Stop key indicator, at memory address $0091; if pressed, call CLRCHN and clear keyboard buffer
GETIN = $FFE4 ;get a character from keyboard
CLALL = $FFE7 ;Close all channels and files
UDTIM = $FFEA ;Increment real-time clock
SCRORG= $FFED ;Fetch number of screen rows and columns
PLOT  = $FFF0 ;Save or restore cursor position
IOBASE= $FFF3 ;Fetch CIA #1 base address

;shadow registers

GRAPHM=$00D8 ;if $FF then editor leaves vic alone
CHAREN=$00D9 ;mask for 8502 CHAREN bit
VM1=   $0A2C ;VIC text mode video matrix & character base pointer
VM2=   $0A2D ;VIC graphic mode video matrix & bit map pointer
VM3=   $0A2E ;8563 text display base address.
VM4=   $0A2F ;8563 attribute base address.
SPLIT= $0A34 ;in split screen mode, contains value for middle raster irq.
CURMOD=$0A2B ;8563 CURSQR Mode.
PAUSE= $0A21 ;scroll flag (in effect = $13)

CIA1_PRA  = $DC00
CIA1_PRB  = $DC01
CIA1_DDRA = $DC02
CIA1_DDRB = $DC03
CIA1_TIMERA= $DC04
CIA1_TIMERB= $DC06
CIA1_ICR= $DC0D  ;Interrupt control reg
CIA1_CRA= $DC0E ;Control Timer A
CIA1_CRB= $DC0F ;Control Timer B

CIA2_PRA  = $DD00
CIA2_PRB  = $DD01
CIA2_DDRA = $DD02
CIA2_DDRB = $DD03
CIA2_TIMERA= $DD04
CIA2_TIMERB= $DD06
CIA2_ICR= $DD0D  ;Interrupt control reg
CIA2_CRA= $DD0E ;Control Timer A
CIA2_CRB= $DD0F ;Control Timer B

ISR_DEFAULT=$FA65
ISR_EXIT   =$FF33

.elseif .def(__VIC20__)

USE_BASIC_ROM .set 1  ;add USE_BASIC_ROM=0 to your code to make LAMAlib generate code without calls to BASIC ROM

PRESSED_KEY_MATRIX_CODE=$C5
MATRIX_CODE_NO_KEY=$40

VAR_FLAGS      = $0b
CURR_VAR_NAME  = $45
CHARS_IN_KEYBUF= $C6
CURSORX        = $D3
CURSORY        = $D6
REVERSE_MODE_SWITCH = $c7
CURRENT_OUTPUT_DEVICE_NUMBER = $9A

KEYBUF         = $277
TEXTCOLOR_ADDR = $286
PTRSCRHI       = $288 ;high byte of pointer to screen memory for screen input/output

BASICSTART=$1001

STROUT= $CB1E ;outputs a string at addr A/Y
LINPRT= $DDCD ;outputs number in AX
FINDARRAYELEMENT=$D218	;part of routine $D1D1 to find or make an array

DEFAULT_OUTPUT=$F27A
CLRSCR= $E55F ;clear screen

SCINIT= $FF81 ;Initialize VIC, clear screen
IOINIT= $FF84 ;Initialize CIAs, SID volume; setup memory configuration; set and start interrupt timer
RAMTAS= $FF87 ;clear ZP, run memory test
RESTOR= $FF8A ;Fill vector table at memory addresses $0314-$0333 with default values
VECTOR= $FF8D ;Copy vector table at memory addresses $0314-$0333
SETMSG= $FF90 ;Set system error display switch at memory address $009D
LSTNSA= $FF93 ;Send LISTEN secondary address to serial bus
TALKSA= $FF96 ;Send TALK secondary address to serial bus
MEMTOP= $FF99 ;Set/read BASIC RAM top
MEMBOT= $FF9C ;Set/read BASIC RAM bottom 
SCNKEY= $FF9F ;Scan keyboard; put current matrix code into $00CB, shift keys into $28D and update keyboard buffer
SETTMO= $FFA2 ;Set serial bus timeout to A
IECIN = $FFA5 ;Read byte from serial bus into A
IECOUT= $FFA8 ;Write byte A to serial bus
UNTALK= $FFAB ;Send UNTALK command to serial bus
UNLSTN= $FFAE ;Send UNLISTEN command to serial bus
LISTEN= $FFB1 ;Send LISTEN command to serial bus
TALK  = $FFB4 ;Send TALK command to serial bus
READST= $FFB7 ;Read status of current input/output device into A
SETLFS= $FFBA ;Set logical file parameters A = Logical number; X = Device number; Y = Secondary address
SETNAM= $FFBD ;Set file name parameters A = File name length; X/Y = Pointer to file name
OPEN  = $FFC0 ;Open file, must call SETLFS and SETNAM before
CLOSE = $FFC3 ;Close file A
CHKIN = $FFC6 ;Define file as default input
CHKOUT= $FFC9 ;Define file as default output
CLRCHN= $FFCC ;Close default input/output files
CHRIN = $FFCF ;Read byte from default input
CHROUT= $FFD2 ;Outputs the character in A to default output.
LOAD  = $FFD5 ;Load file, must call SETLFS and SETNAM before
SAVE  = $FFD8 ;Save file, must call SETLFS and SETNAM before
SETTIM= $FFDB ;Set Time of Day, at memory address $00A0-$00A2
RDTIM = $FFDE ;Read Time of Day to AX/Y
STOP  = $FFE1 ;Query Stop key indicator, at memory address $0091; if pressed, call CLRCHN and clear keyboard buffer
GETIN = $FFE4 ;get a character from keyboard
CLALL = $FFE7 ;Close all channels and files
UDTIM = $FFEA ;Increment real-time clock
SCRORG= $FFED ;Fetch number of screen rows and columns
PLOT  = $FFF0 ;Save or restore cursor position
IOBASE= $FFF3 ;Fetch CIA #1 base address

VIABASE=$9110

TIMERAcounter = VIABASE+4
TIMERAlatch= VIABASE+6
TIMERBlatch= VIABASE+8

ISR_DEFAULT=$EABF
ISR_EXIT   =$EB18

.endif


