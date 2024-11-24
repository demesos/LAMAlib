@rem update_documentation
@rem a Windows batch file to execute parsedoc with the parameters for the LAMAlib project
@rem 
@rem Usage: makedoc
@rem python 3.x needs to be installed on your system and to be in your path
@rem
@rem Version: 0.3
@rem Date: 2024-11-24
@rem Author: Wil Elmenreich (wilfried at gmx dot at)
@rem License: The Unlicense (public domain)

python parsedoc.py LAMAlib.inc LAMAlib-sprites.inc > LAMAlibdoc.html
@IF /I %0 EQU "%~dpnx0" PAUSE