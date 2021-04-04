# Script to assemble a file in ca65 assembler cource into an executable
# Automatically adds a BASIC execution stub, unless there is already code for a stub
# in the program or a different target address is given

if [ $# -eq 0 ]; then
  echo Usage: $0 asmfile [startaddr]
  echo Calls the cl65 assembler and linker and creates an executable .PRG for C64
  exit 1
fi

if [ $# -eq 1 ]; then
  echo assembling $1 for target C64...
  if grep -q makesys "$0"; then
    cl65 "$1" -lib LAMAlib.lib -C c64-basicfriendly-asm.cfg -Ln "labels.txt" -u __EXEHDR__ -o "${1%.*}.prg"
  else
    cl65 "$1" -lib LAMAlib.lib -C c64-basicfriendly-asm.cfg -Ln "labels.txt" -o "${1%.*}.prg"
  fi
else
  echo assembling $1 to start address $2 for target C64...
  cl65 "$1" -lib LAMAlib.lib -C c64-basicfriendly-asm.cfg -Ln "labels.txt" --start-addr $2 -o "${1%.*}.prg"
fi
echo done.

exit $?
