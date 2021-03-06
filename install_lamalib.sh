#!/bin/bash
# install_lamalib
# an MSDOS batch file (runs on Windows) to reassemble all parts of the library for the LAMAlib project
# and to install it in cc65 
# 
# Usage: install_lamalib
# cc65 tools need to be installed on your system and to be in your path
#
# Version: 0.2
# Date: 2020-05-11
# Author: Wil Elmenreich (wilfried at gmx dot at)
# License: The Unlicense (public domain)

# Define some useful colorcode vars:
red='\033[0;31m'
green='\033[0;32m'
yellow='\033[1;33m'
magenta='\033[0;35m'
cyan='\033[0;36m'
white='\033[1;37m'
black='\033[0;30m'
nocolor='\033[0m'


which cc65 >/dev/null
#retval=$?
#echo $retval
if [ $? -ne 0 ]; then
  echo -e ${red}'cc65 installation not found. Please install cc65 and run this script again!'${nocolor}
  exit 1
fi

cd lib-functions
count=0
for f in *.s; do
    ca65 -t c64 $f
    count=$[$count +1]
done

for f in *.o; do
    ar65 d ../LAMAlib.lib $f
    ar65 a ../LAMAlib.lib $f
done

echo Library has been created with $count modules in it.
cd ..

# find cc65 directory
installdir=$(dirname $(dirname $(which cc65)))
echo Installing library into $installdir

cp LAMAlib*.inc "$installdir/asminc"
cp LAMAlib.lib "$installdir/lib"
cp ass.sh "$installdir/bin/ass"
cp c64-basicfriendly-asm.cfg "$installdir/cfg"

echo -e $green
echo -e "*******************************************************************************"
echo -e "* Congratulations, LAMAlib has been installed!                                *"
echo -e "*                                                                             *"
echo -e "* To use it, add the line                                                     *"
echo -e "* ${cyan}.include "LAMAlib.inc"${green} to the top of your assembler file and assemble with    *"
echo -e "* ${cyan}cl65 yourprog.s -lib LAMAlib.lib -C c64-asm.cfg -o yourprog.prg${green}             *"
echo -e "* There is no overhead to your assembled program for unused functions         *"
echo -e "*******************************************************************************${nocolor}"
