:: Script to assemble a file in ca65 assembler cource into an executable
:: Automatically adds a BASIC execution stub, unless there is already code for a stub
:: in the program or a different target address is given

@echo off

set TARGET=c64

if "%1"=="" (
  echo Usage: %0 [-128^|20] asmfile [startaddr]
  echo Calls the cl65 assembler and linker and creates an executable .PRG for C64, unless -128 or -20 are specified, then result is for C128 or VIC20
  exit /b
)
if "%1"=="-128" (
  set TARGET=c128
  shift
) else if "%1"=="-c128" (
  set TARGET=c128
  shift
) else if "%1"=="-C128" (
  set TARGET=c128
  shift
)

if "%1"=="-64" (
  set TARGET=c64
  shift
) else if "%1"=="-c64" (
  set TARGET=c64
  shift
) else if "%1"=="-C64" (
  set TARGET=c64
  shift
)

if "%1"=="-20" (
  set TARGET=vic20
  shift
) else if "%1"=="-vc20" (
  set TARGET=vic20
  shift
) else if "%1"=="-vic20" (
  set TARGET=vic20
  shift
)


if "%2"=="" (
  echo assembling %1 for target %TARGET%...
  >nul findstr /c:"makesys" %1 && (
    @echo on
    cl65 -t %TARGET% "%1" -lib LAMAlib.lib -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" -o "%~n1.prg"
  ) || (
    @echo on
    cl65 -t %TARGET% "%1" -lib LAMAlib.lib -t %TARGET% -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" -u __EXEHDR__ -o "%~n1.prg"
  )
) else (
  echo assembling %1 to start address %2 for target %TARGET%...
  @echo on
  cl65 -t %TARGET% "%1" -lib LAMAlib.lib -t %TARGET% -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" --start-addr %2 -o "%~n1.prg"
)
@echo done.

@exit /B %ERRORLEVEL%