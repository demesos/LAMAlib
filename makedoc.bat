@rem makedoc.bat
@rem an MSDOS batch file (runs on Windows) to execute parsedoc with the parameters for the LAMAlib project
@rem 
@rem Usage: makedoc
@rem python 3.x needs to be installed on your system and to be in your path
@rem
@rem Version: 0.1 
@rem Date: 2019-10-26
@rem Author: Wil Elmenreich (wilfried at gmx dot at)
@rem License: The Unlicense (public domain)

python parsedoc.py LAMAlib.inc > LAMAlibdoc.html
pause