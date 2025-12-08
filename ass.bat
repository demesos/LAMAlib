:: Script to assemble a file in ca65 assembler cource into an executable
:: Automatically adds a BASIC execution stub, unless there is already code for a stub
:: in the program or a different target address is given

@echo off

set TARGET=c64
set LIBNAME=LAMAlib.lib
set ASMDEF= 
set VERBOSE=

if "%1"=="" (
  echo Usage: %0 [-128^|20] asmfile [startaddr]
  echo Calls the cl65 assembler and linker and creates an executable .PRG for C64, unless -128 or -20 are specified, then result is for C128 or VIC20
  exit /b
)

:check_options

if "%1"=="-v" (
  set VERBOSE=1
  shift
  goto check_options
) 

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
  set LIBNAME=LAMAlib128.lib
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
  set LIBNAME=LAMAlib20.lib
  shift
  goto check_options
:not20

set NEEDSCOMPILE=0
set ASMFILE=%1
findstr /r /c:"^ *let " "%1" >nul 2>&1 && (
    :: Found uncommented let statements - compilation needed
    set NEEDSCOMPILE=1
    set ASMFILE=%~n1.asm
)

if %NEEDSCOMPILE% equ 1 (
  echo Compiling high-level expressions with expr2asm...
  if "%VERBOSE%"=="1" (
    echo expr2asm.py -c "%1"
  )
  expr2asm.py -c "%1"
  if ERRORLEVEL 1 (
    echo ERROR: expr2asm compilation failed
    exit /B 1
  )
  echo expr2asm compilation complete.
  echo.
)

if "%2"=="" (
  echo assembling %ASMFILE% for target %TARGET%...
  >nul findstr /c:"makesys" "%ASMFILE%" && (
    @echo on
    if "%VERBOSE%"=="1" (
      echo cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" -o "%~n1.prg"
    )
    cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" -o "%~n1.prg"
  ) || (
    @echo on
    if "%VERBOSE%"=="1" (
      echo cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" -u __EXEHDR__ -o "%~n1.prg"
    )
    cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" -u __EXEHDR__ -o "%~n1.prg"
  )
) else (
  echo assembling %ASMFILE% to start address %2 for target %TARGET%...
  @echo on
  if "%VERBOSE%"=="1" (
    echo cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" --start-addr %2 -o "%~n1.prg"
  )
  cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln "labels.txt" --start-addr %2 -o "%~n1.prg"
)
@echo done.

@exit /B %ERRORLEVEL%
