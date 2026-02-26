@echo off

call ass testsuite.s
@if not errorlevel 1 (
  echo starting testsuite...
  start x64 -warp -autostartprgmode 1 testsuite.prg
) else (
  pause
)

call ass -d SAVEREGS -o testsuite_saveregs.prg testsuite.s
@if not errorlevel 1 (
  echo starting testsuite...
  start x64 -warp -autostartprgmode 1 testsuite_saveregs.prg
) else (
  pause
)
