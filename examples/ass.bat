rem assemble a file for the C64 target
@echo off
>nul findstr /c:"makesys" %1 && (
  @echo on
  cl65 %1 -lib LAMAlib.lib -C c64-asm.cfg -o %~n1.prg
) || (
  @echo on
  cl65 %1 -lib LAMAlib.lib -C c64-asm.cfg -u __EXEHDR__ -o %~n1.prg
)


