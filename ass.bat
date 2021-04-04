:: Script to assemble a file in ca65 assembler cource into an executable
:: Automatically adds a BASIC execution stub, unless there is already code for a stub
:: in the program or a different target address is given

@echo off

if "%1"=="" (
  echo Usage: %0 asmfile [startaddr]
  echo Calls the cl65 assembler and linker and creates an executable .PRG for C64
  exit /b
)
if "%2"=="" (
  echo assembling %1 for target C64...
  >nul findstr /c:"makesys" %1 && (
    @echo on
    cl65 "%1" -lib LAMAlib.lib -C c64-basicfriendly-asm.cfg -Ln "labels.txt" -o "%~n1.prg"
  ) || (
    @echo on
    cl65 "%1" -lib LAMAlib.lib -C c64-basicfriendly-asm.cfg -Ln "labels.txt" -u __EXEHDR__ -o "%~n1.prg"
  )
) else (
  echo assembling %1 to start address %2 for target C64...
  @echo on
  cl65 "%1" -lib LAMAlib.lib -C c64-basicfriendly-asm.cfg -Ln "labels.txt" --start-addr %2 -o "%~n1.prg"
)
@echo done.

@exit /B %ERRORLEVEL%