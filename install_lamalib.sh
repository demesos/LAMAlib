#!/bin/bash
# install_lamalib
# an MSDOS batch file (runs on Windows) to reassemble all parts of the library for the LAMAlib project
# and to install it in cc65 
# 
# Usage: install_lamalib [CC65_INSTALL_PATH]
# cc65 needs to be installed on your system and cc65/bin must be in your path
# If CC65_INSTALL_PATH is given, it overrides the path found by 'which cc65'.
#
# Version: 0.33
# Date: 2025-12-08
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

if [ -n "$1" ]; then
  # Use the provided path
  installdir="$1"
  if [ ! -d "$installdir" ] || [ ! -d "$installdir/bin" ]; then
    echo -e ${red}"Error: Custom cc65 path '$installdir' is invalid or missing the 'bin' subdirectory."${nocolor}
    echo "Please ensure the path points to the root of the cc65 installation."
    exit 1
  fi
  installbindir="$installdir/bin"
else
  which cc65 >/dev/null
  if [ $? -ne 0 ]; then
    echo -e ${red}'cc65 installation not found. Please install cc65 and run this script again!'${nocolor}
    exit 1
  fi
  
  cc65execpath=$(which cc65)

  # Check for common /usr/bin installation (e.g., from package manager)
  if [ "$cc65execpath" == "/usr/bin/cc65" ]; then
    installdir="/usr/share/cc65"
    installbindir="/usr/bin"
  else
    # Deduce root directory: cc65execpath is /path/to/cc65/bin/cc65
    installdir=$(dirname $(dirname "$cc65execpath"))
    installbindir="$installdir/bin"
  fi
fi

# --- END NEW LOGIC ---

pushd lib-functions
make
popd

# find cc65 directory
echo Installing library into $installdir

cp -f LAMAlib*.inc "$installdir/asminc/"
cp -rf modules "$installdir/asminc/"
cp -f LAMAlib*.lib "$installdir/lib/"
cp -f LAMAlib128.lib "$installdir/lib/"
cp -f LAMAlib20.lib "$installdir/lib/"
cp -f *friendly-asm.cfg "$installdir/cfg/"
cp -f ass.sh "$installbindir/ass"
cp -f asdent.py "$installbindir/asdent"
cp -f exprass.py "$installbindir/exprass"

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

# Updated success message
echo -e $white
echo -e "*******************************************************************************"
echo -e "* Congratulations, LAMAlib has been installed!                                *"
echo -e "*                                                                             *"
echo -e "* To use it, add the line                                                     *"
echo -e "* ${cyan}.include \"LAMAlib.inc\"${white} at the top of your assembler file                    *"
echo -e "* and assemble with command                                                   *"
echo -e "* ${cyan}cl65 yourprog.s -lib LAMAlib.lib -C c64-asm.cfg -o yourprog.prg${white}             *"
echo -e "* or rather simpler with ${cyan}ass yourprog.s${white}                                       *"
echo -e "* There is no overhead to your assembled program for unused functions         *"
echo -e "*******************************************************************************${nocolor}"
