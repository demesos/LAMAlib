:: ass.bat – assemble ca65 sources into a PRG
:: Version: 0.35
:: Runs exprass on each source that contains "let"
:: Adds BASIC stub unless code already provides one or a custom start address is given
::
:: Usage: ass [options] file1.s [file2.s ...] [startaddr]
::
:: Options:
::   -l              Link multiple files together (default: assemble separately)
::   -o <file>       Output filename (only valid with single file or -l mode)
::   -v              Verbose mode
::   -d <symbol> [value]  Define assembler symbol, optionally with value
::                        Examples: -d DEBUG  or  -d LEVEL 5  or  -d RAZY=1
::   -64, -c64       Target C64 (default)
::   -128, -c128     Target C128
::   -20, -vic20     Target VIC-20
::   -h, -?          Show this help
::
:: Start address (optional last argument):
::   Must be hex ($xxxx or 0xXXXX) or decimal digits, or omit for BASIC stub

@echo off
setlocal EnableDelayedExpansion

set TARGET=c64
set LIBNAME=LAMAlib.lib
set ASMDEF=
set VERBOSE=
set LINKMODE=
set OUTFILE=

set FILES=
set MAINFILE=
set STARTADDR=
set FILECOUNT=0

if "%1"=="" goto show_usage

:: --------------------------------------------------
:: option parsing
:: --------------------------------------------------

:check_options

if "%~1"=="" goto collect_args

if /i "%1"=="-h" goto show_usage
if "%1"=="-?" goto show_usage

if "%1"=="-v" (
  set VERBOSE=1
  shift
  goto check_options
)

if "%1"=="-l" (
  set LINKMODE=1
  shift
  goto check_options
)

if "%1"=="-o" (
  if "%~2"=="" (
    echo ERROR: -o requires output filename
    exit /B 1
  )
  set OUTFILE=%~2
  shift
  shift
  goto check_options
)

if "%1"=="-d" (
  if "%~2"=="" (
    echo ERROR: -d requires symbol definition
    exit /B 1
  )
  
  :: Check if %3 is a number
  call :is_number "%~3"
  if "!IS_NUM!"=="1" (
    set ASMDEF=!ASMDEF! --asm-define %2=%3
    shift
    shift
    shift
  ) else (
    set ASMDEF=!ASMDEF! --asm-define %2
    shift
    shift
  )
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
  set LIBNAME=LAMAlib.lib
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

:: Check for unknown option (starts with -)
if not "%~1"=="" (
  if "%~1:~0,1%"=="-" (
    echo ERROR: Unknown option: %1
    echo.
    goto show_usage
  )
)

goto collect_args

:: --------------------------------------------------
:: collect files and optional start address
:: --------------------------------------------------

:collect_args
if "%~1"=="" goto args_done

:: Detect start address vs filename
:: Address = starts with $, 0x, 0X OR (pure digits AND no file extension)
set ARG=%~1

:: Check if it has a file extension - if yes, it's definitely a file
if not "%~x1"=="" goto not_address

:: No extension - check if it's a number
call :is_number "%ARG%"
if "!IS_NUM!"=="1" (
  set STARTADDR=%~1
  shift
  goto collect_args
)

:not_address

:: It's a file - expand wildcards
for %%f in (%~1) do (
  call :process_file "%%f"
)

shift
goto collect_args

:args_done

if !FILECOUNT! EQU 0 (
  echo ERROR: no source files given
  exit /B 1
)

:: Validate -o usage
if defined OUTFILE (
  if !FILECOUNT! GTR 1 (
    if not defined LINKMODE (
      echo ERROR: -o can only be used with a single file or with -l ^(link mode^)
      exit /B 1
    )
  )
)

:: Don't set OUTFILE here - let each mode handle it

:: --------------------------------------------------
:: assemble
:: --------------------------------------------------

if defined LINKMODE goto do_link

:: Separate assembly mode (default)
echo Assembling files separately for target %TARGET%...
for %%f in (%FILES%) do (
  call :assemble_single %%f
  if ERRORLEVEL 1 exit /B 1
)
echo done.
exit /B 0

:do_link
:: Link mode

:: Set output filename for link mode if not provided
if not defined OUTFILE (
  set OUTFILE=%MAINFILE%.prg
)

if "%STARTADDR%"=="" (
  echo Assembling and linking%FILES% for target %TARGET%...

  >nul findstr /c:"makesys" %FILES% && (
    if "%VERBOSE%"=="1" (
      echo cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt -o "%OUTFILE%"
    )
    cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt -o "%OUTFILE%"
  ) || (
    if "%VERBOSE%"=="1" (
      echo cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt -u __EXEHDR__ -o "%OUTFILE%"
    )
    cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt -u __EXEHDR__ -o "%OUTFILE%"
  )

) else (

  echo Assembling and linking%FILES% to start address %STARTADDR% for target %TARGET%...

  if "%VERBOSE%"=="1" (
    echo cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt --start-addr %STARTADDR% -o "%OUTFILE%"
  )
  cl65 -t %TARGET% %ASMDEF% -g %FILES% -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln labels.txt --start-addr %STARTADDR% -o "%OUTFILE%"
)

echo done.
exit /B %ERRORLEVEL%

:: --------------------------------------------------
:: subroutines
:: --------------------------------------------------

:process_file
:: Process a single file: run exprass if needed, add to FILES list
set SRCFILE=%~1

:: first source file defines default output name
if not defined MAINFILE (
  set MAINFILE=%~n1
)

set /A FILECOUNT+=1

:: check for exprass ("let" at line start, ignoring comments)
findstr /r /c:"^[	 ]*let " "%SRCFILE%" >nul 2>&1 && (
  echo exprass: compiling %SRCFILE%
  if "%VERBOSE%"=="1" (
    echo exprass.py -c "%SRCFILE%"
  )
  exprass.py -c "%SRCFILE%"
  if ERRORLEVEL 1 (
    echo ERROR: exprass failed for %SRCFILE%
    exit /B 1
  )
  set FILES=!FILES! "%~dpn1.asm"
  goto :eof
)

:: no exprass needed - use original source
set FILES=!FILES! "%SRCFILE%"
goto :eof

:assemble_single
:: Assemble a single file to its own PRG
set ASMFILE=%~1
set ASMBASE=%~n1

:: Use -o output filename if specified, otherwise default to input basename
if defined OUTFILE (
  set ASMOUT=%OUTFILE%
) else (
  set ASMOUT=%ASMBASE%.prg
)

if "%VERBOSE%"=="1" (
  echo Assembling %ASMFILE% to %ASMOUT%...
)

if "%STARTADDR%"=="" (
  >nul findstr /c:"makesys" "%ASMFILE%" && (
    if "%VERBOSE%"=="1" (
      echo cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln %ASMBASE%_labels.txt -o "%ASMOUT%"
    )
    cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln %ASMBASE%_labels.txt -o "%ASMOUT%"
  ) || (
    if "%VERBOSE%"=="1" (
      echo cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln %ASMBASE%_labels.txt -u __EXEHDR__ -o "%ASMOUT%"
    )
    cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln %ASMBASE%_labels.txt -u __EXEHDR__ -o "%ASMOUT%"
  )
) else (
  if "%VERBOSE%"=="1" (
    echo cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln %ASMBASE%_labels.txt --start-addr %STARTADDR% -o "%ASMOUT%"
  )
  cl65 -t %TARGET% %ASMDEF% -g "%ASMFILE%" -lib %LIBNAME% -C %TARGET%-basicfriendly-asm.cfg -Ln %ASMBASE%_labels.txt --start-addr %STARTADDR% -o "%ASMOUT%"
)

goto :eof

:: ============================================
:: Subroutine: is_number
:: Check if argument is a number (hex or decimal)
:: Input: %1 = value to check
:: Output: IS_NUM=1 if number, IS_NUM=0 if not
:: ============================================
:is_number
set IS_NUM=0
set TESTVAL=%~1

if "%TESTVAL%"=="" goto :eof

:: Check if starts with $ (hex)
if "!TESTVAL:~0,1!"=="$" (
  set IS_NUM=1
  goto :eof
)

:: Check if starts with 0x or 0X (hex)
if /i "!TESTVAL:~0,2!"=="0x" (
  set IS_NUM=1
  goto :eof
)

:: Check if pure decimal digits using findstr
echo !TESTVAL!| findstr /r "^[0-9][0-9]*$" >nul 2>&1
if !errorlevel!==0 set IS_NUM=1

goto :eof

:show_usage
echo Usage: %~n0 [options] file1.s [file2.s ...] [startaddr]
echo.
echo Assemble ca65 sources into PRG files for C64/C128/VIC-20
echo Automatically runs exprass on sources containing "let" statements
echo.
echo Options:
echo   -l              Link multiple files together ^(default: assemble separately^)
echo   -o ^<file^>       Output filename ^(only valid with single file or -l mode^)
echo   -v              Verbose mode
echo   -d ^<symbol^> [value]  Define assembler symbol, optionally with value
echo                        Examples: -d DEBUG  or  -d LEVEL 5  or  -d RAZY=1
echo   -64, -c64       Target C64 ^(default^)
echo   -128, -c128     Target C128
echo   -20, -vic20     Target VIC-20
echo   -h, -?          Show this help
echo.
echo Start address ^(optional last argument^):
echo   Must be hex ^($xxxx or 0xXXXX^) or decimal digits
echo   Omit for automatic BASIC stub
echo.
echo Examples:
echo   %~n0 main.s                        Assemble single file to main.prg
echo   %~n0 -l main.s util.s              Link two files into main.prg
echo   %~n0 -l -o game.prg main.s util.s  Link into game.prg
echo   %~n0 *.s                           Assemble all .s files separately
echo   %~n0 -d DEBUG main.s               Define DEBUG symbol
echo   %~n0 -d LEVEL 5 main.s             Define LEVEL=5
echo   %~n0 -d RAZY=1 main.s              Define RAZY=1 ^(= gets split, then recombined^)
echo   %~n0 main.s $c000                  Assemble to address $c000
exit /B 1
