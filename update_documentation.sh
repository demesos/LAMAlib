#!/bin/bash
# update_documentation
# a Linux shell script to execute parsedoc with the parameters for the LAMAlib project
# 
# Usage: makedoc
# python 3.x needs to be installed on your system and to be in your path
#
# Version: 0.3
# Date: 2024-11-24
# Author: Wil Elmenreich (wilfried at gmx dot at)
# License: The Unlicense (public domain)

python parsedoc.py LAMAlib.inc LAMAlib-sprites.inc > LAMAlibdoc.html
