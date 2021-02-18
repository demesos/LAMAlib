@echo off
echo assembling a file for the C64 target...
if "%2"=="" (
  >nul findstr /c:"makesys" %1 && (
    @echo on
    cl65 "%1" -lib LAMAlib.lib -C c64-basicfriendly-asm.cfg -Ln "labels.txt" -o "%~n1.prg"
  ) || (
    @echo on
    cl65 "%1" -lib LAMAlib.lib -C c64-basicfriendly-asm.cfg -Ln "labels.txt" -u __EXEHDR__ -o "%~n1.prg"
  )
) else (
  @echo on
  cl65 "%1" -lib LAMAlib.lib -C c64-basicfriendly-asm.cfg -Ln "labels.txt" --start-addr %2 -o "%~n1.prg"
)
@echo done.

@exit /B %ERRORLEVEL%