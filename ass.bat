:: Script to assemble a file in ca65 assembler cource into an executable
:: Automatically adds a BASIC execution stub, unless there is already code for a stub
:: in the program or a different target address is given

@echo off

set TARGET=c64
set ASMDEF= 

if "%1"=="" (
  echo Usage: %0 [-128^|20] asmfile [startaddr]
  echo Calls the cl65 assembler and linker and creates an executable .PRG for C64, unless -128 or -20 are specified, then result is for C128 or VIC20
  exit /b
)

:check_options

if "%1"=="-d" (
  set ASMDEF=--asm-define %2
  shift
  shift
  goto check_options
) 

if "%~1"=="-128" goto do128
if /i "%~1" neq "-c128" goto not128
:do128
  set TARGET=c128
  shift
  goto check_options
:not128

if "%~1"=="-64" goto do64
if /i "%~1" neq "-c64" goto not64
:do64
  set TARGET=c64
  shift
  goto check_options
:not64

if "%~1"=="-20" goto do20
if /i "%~1"=="-vic20" goto do20
if /i "%~1" neq "-vc20" goto not20
:do20
  set TARGET=vic20
  shift
  goto check_options
:not20

if "%2"=="" (
  echo assembling %1 for target %TARGET%...
  >nul findstr /c:"makesys" %1 && (
    @echo on
    cl65 -t %TARGET% %ASMDEF% "%1" -lib LAMAlib.lib -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" -o "%~n1.prg"
  ) || (
    @echo on
    cl65 -t %TARGET% %ASMDEF% "%1" -lib LAMAlib.lib -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" -u __EXEHDR__ -o "%~n1.prg"
  )
) else (
  echo assembling %1 to start address %2 for target %TARGET%...
  @echo on
  cl65 -t %TARGET% %ASMDEF% "%1" -lib LAMAlib.lib -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" --start-addr %2 -o "%~n1.prg"
)
@echo done.

@exit /B %ERRORLEVEL%