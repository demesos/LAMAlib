:: ass.bat – assemble ca65 sources into a PRG
:: Runs exprass on each source that contains "let"
:: Adds BASIC stub unless code already provides one or a custom start address is given

@echo off
setlocal EnableDelayedExpansion

set TARGET=c64
set LIBNAME=LAMAlib.lib
set ASMDEF=
set VERBOSE=

set FILES=
set MAINFILE=
set STARTADDR=

if "%1"=="" (
  echo Usage: %0 [options] file1.s [file2.s ...] [startaddr]
  echo startaddr must start with digit or $
  exit /B 1
)

:: --------------------------------------------------
:: option parsing
:: --------------------------------------------------

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
if /i "%~1"=="-c128" goto do128
goto not128
:do128
  set TARGET=c128
  set LIBNAME=LAMAlib128.lib
  shift
  goto check_options
:not128

if "%~1"=="-64" goto do64
if /i "%~1"=="-c64" goto do64
goto not64
:do64
  set TARGET=c64
  shift
  goto check_options
:not64

if "%~1"=="-20" goto do20
if /i "%~1"=="-vic20" goto do20
if /i "%~1"=="-vc20" goto do20
goto not20
:do20
  set TARGET=vic20
  set LIBNAME=LAMAlib20.lib
  shift
  goto check_options
:not20

:: --------------------------------------------------
:: collect files and optional start address
:: --------------------------------------------------

:collect_args
if "%1"=="" goto args_done

:: start address = begins with digit or $
echo %1 | findstr /r "^[0-9$]" >nul && (
  set STARTADDR=%1
  shift
  goto collect_args
)

:: first source file defines output name
if not defined MAINFILE (
  set MAINFILE=%~n1
)

:: check for exprass ("let" at line start, ignoring comments)
findstr /r /c:"^[	 ]*let " "%1" >nul 2>&1 && (
  echo exprass: compiling %1
  if "%VERBOSE%"=="1" (
    echo exprass.py -c "%1"
  )
  exprass.py -c "%1"
  if ERRORLEVEL 1 (
    echo ERROR: exprass failed for %1
    exit /B 1
  )
  set FILES=!FILES! "%~n1.asm"
  shift
  goto collect_args
)

:: no exprass needed ? use original source
set FILES=!FILES! "%1"
shift
goto collect_args

:args_done

if not defined MAINFILE (
  echo ERROR: no source files given
  exit /B 1
)

:: --------------------------------------------------
:: assemble
:: --------------------------------------------------

if "%STARTADDR%"=="" (
  echo assembling%FILES% for target %TARGET%...

  >nul findstr /c:"makesys" %FILES% && (
    if "%VERBOSE%"=="1" (
      echo cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt -o "%MAINFILE%.prg"
    )
    cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt -o "%MAINFILE%.prg"
  ) || (
    if "%VERBOSE%"=="1" (
      echo cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt -u __EXEHDR__ -o "%MAINFILE%.prg"
    )
    cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt -u __EXEHDR__ -o "%MAINFILE%.prg"
  )

) else (

  echo assembling%FILES% to start address %STARTADDR% for target %TARGET%...

  if "%VERBOSE%"=="1" (
    echo cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt --start-addr %STARTADDR% -o "%MAINFILE%.prg"
  )
  cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt --start-addr %STARTADDR% -o "%MAINFILE%.prg"
)

echo done.
exit /B %ERRORLEVEL%
