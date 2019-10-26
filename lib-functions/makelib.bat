@rem makelib.bat
@rem an MSDOS batch file (runs on Windows) to reassemble all parts of the library for the LAMAlib project
@rem 
@rem Usage: makelib
@rem cc65 tools need to be installed on your system and to be in your path
@rem Note: Unless you further develop LAMAlib there should be no need to use this script
@rem
@rem Version: 0.1 
@rem Date: 2019-10-26
@rem Author: Wil Elmenreich (wilfried at gmx dot at)
@rem License: The Unlicense (public domain)

@echo off
setlocal enableextensions
set count=0
for %%f in (*.s) do (
    ca65 -t c64 %%f
    set /a count+=1
)

for %%f in (*.o) do (
    ar65 d ..\LAMAlib.lib %%f
    ar65 a ..\LAMAlib.lib %%f
)

echo Library has been created with %count% modules in it.
pause