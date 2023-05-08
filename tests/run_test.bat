@echo off

call ass testsuite.s
@if not errorlevel 1 (
  echo starting testsuite...
  start x64 -autostartprgmode 1 testsuite.prg
) else (
  pause
)
