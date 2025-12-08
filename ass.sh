#!/bin/bash

# Default settings
TARGET="c64"
LIBNAME="LAMAlib.lib"
ASMDEF=""
VERBOSE="" 

# Script to assemble a file in ca65 assembler source into an executable
# Automatically adds a BASIC execution stub, unless there is already code for a stub
# in the program or a different target address is given

# Check for command-line arguments
if [[ "$1" == "" ]]; then
  echo "Usage: $0 [-v] [-d DEFINE VALUE] [-c128|-c64|-vc20] asmfile [startaddr]"
  echo "Calls the cl65 assembler and linker and creates an executable .PRG for C64, unless -c128 or -vc20 are specified, then result is for C128 or VIC20"
  exit 1
fi

option_processed=1
while [[ $option_processed -eq 1 ]]; do
  option_processed=0
  if [[ "$1" == "-v" ]]; then 
    VERBOSE=1
    shift
    option_processed=1
  elif [[ "$1" == "-d" ]]; then
    ASMDEF="--asm-define $2"
    shift 2
    option_processed=1
  elif [[ "$1" == "-c128" || "$1" == "-128" ]]; then
    TARGET="c128"
    LIBNAME="LAMAlib128.lib"
    shift
    option_processed=1
  elif [[ "$1" == "-c64" || "$1" == "-64" ]]; then
    TARGET="c64"
    shift
    option_processed=1
  elif [[ "$1" == "-vc20" || "$1" == "-20" || "$1" == "-vic20" ]]; then
    TARGET="vic20"
    LIBNAME="LAMAlib20.lib"
    shift
    option_processed=1
  fi
done

ASMFILE="$1" # Store the original assembly filename
NEEDSCOMPILE=0

if grep -P '^\s*let ' "$ASMFILE" > /dev/null; then
  NEEDSCOMPILE=1
  ASMFILE="${ASMFILE%.*}.asm" 
fi

if [[ $NEEDSCOMPILE -eq 1 ]]; then
  echo "Compiling high-level expressions with expr2asm..."
  if [[ "$VERBOSE" == "1" ]]; then
    echo "expr2asm -c \"$1\""
  fi
  
  # Execute the compiler
  if ! expr2asm -c "$1"; then
    echo "ERROR: expr2asm compilation failed"
    exit 1
  fi
  echo "expr2asm compilation complete."
  echo ""
fi

# --- END NEW LOGIC ---

# Check if a start address is provided
if [[ -z "$2" ]]; then
  echo "assembling $ASMFILE for target $TARGET..."
  if grep -q makesys "$ASMFILE"; then
    if [[ "$VERBOSE" == "1" ]]; then 
      echo "cl65 -t \"$TARGET\" $ASMDEF -g \"$ASMFILE\" -lib \"$LIBNAME\" -C \"${TARGET}-basicfriendly-asm.cfg\" -Ln \"labels.txt\" -o \"${1%.*}.prg\""
    fi
    cl65 -t "$TARGET" $ASMDEF -g "$ASMFILE" -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" -Ln "labels.txt" -o "${1%.*}.prg"
  else
    if [[ "$VERBOSE" == "1" ]]; then 
      echo "cl65 -t \"$TARGET\" $ASMDEF -g \"$ASMFILE\" -lib \"$LIBNAME\" -C \"${TARGET}-basicfriendly-asm.cfg\" -Ln \"labels.txt\" -u __EXEHDR__ -o \"${1%.*}.prg\""
    fi
    cl65 -t "$TARGET" $ASMDEF -g "$ASMFILE" -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" -Ln "labels.txt" -u __EXEHDR__ -o "${1%.*}.prg"
  fi
else
  echo "assembling $ASMFILE to start address $2 for target $TARGET..."
  if [[ "$VERBOSE" == "1" ]]; then 
    echo "cl65 -t \"$TARGET\" $ASMDEF -g \"$ASMFILE\" -lib \"$LIBNAME\" -C \"${TARGET}-basicfriendly-asm.cfg\" -Ln \"labels.txt\" --start-addr \"$2\" -o \"${1%.*}.prg\""
  fi
  cl65 -t "$TARGET" $ASMDEF -g "$ASMFILE" -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" -Ln "labels.txt" --start-addr "$2" -o "${1%.*}.prg"
fi

echo "done."