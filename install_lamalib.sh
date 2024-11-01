#!/bin/bash
# install_lamalib
# an MSDOS batch file (runs on Windows) to reassemble all parts of the library for the LAMAlib project
# and to install it in cc65 
# 
# Usage: install_lamalib
# cc65 needs to be installed on your system and cc65/bin must be in your path
#
# Version: 0.31
# Date: 2024-03-03
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
if [ $? -ne 0 ]; then
  echo -e ${red}'cc65 installation not found. Please install cc65 and run this script again!'${nocolor}
  exit 1
fi
cc65dir=$(which cc65)
if [ $cc65dir == "/usr/bin/cc65" ]; then
  installdir=/usr/share/cc65
  installbindir=/usr/bin
else
  installdir=$(dirname $(dirname $(which cc65)))
  installbindir=$installdir/bin
fi

pushd lib-functions
make
popd

# find cc65 directory
echo Installing library into $installdir

cp LAMAlib*.inc "$installdir/asminc/"
cp -r modules "$installdir/asminc/"
cp LAMAlib*.lib "$installdir/lib/"
cp LAMAlib128.lib "$installdir/lib/"
cp LAMAlib20.lib "$installdir/lib/"
cp *friendly-asm.cfg "$installdir/cfg/"
cp ass.sh "$installbindir/ass"

if [ $? -ne 0 ]; then
  echo -e $red
  echo -e "*******************************************************************************"
  echo -e "* Could not install LAMAlib!                                                  *"
  echo -e "*                                                                             *"
  echo -e "* Please run the install script with superuser rights:                        *"
  echo -e "* ${cyan}sudo ./install_lamalib.sh $red                                                  *"
  echo -e "*******************************************************************************${nocolor}"
  exit 1
fi

echo -e $green
echo -e "*******************************************************************************"
echo -e "* Congratulations, LAMAlib has been installed!                                *"
echo -e "*                                                                             *"
echo -e "* To use it, add the line                                                     *"
echo -e "* ${cyan}.include "LAMAlib.inc"${green} to the top of your assembler file and assemble with    *"
echo -e "* ${cyan}cl65 yourprog.s -lib LAMAlib.lib -C c64-asm.cfg -o yourprog.prg${green}             *"
echo -e "* There is no overhead to your assembled program for unused functions         *"
echo -e "*******************************************************************************${nocolor}"
