#!/bin/bash

# ass.sh – assemble ca65 sources into a PRG
# Version: 0.35
# Runs exprass on each source that contains "let"
# Adds BASIC stub unless code already provides one or a custom start address is given
#
# Usage: ass [options] file1.s [file2.s ...] [startaddr]
#
# Options:
#   -l              Link multiple files together (default: assemble separately)
#   -o <file>       Output filename (only valid with single file or -l mode)
#   -v              Verbose mode
#   -d <symbol> [value]  Define assembler symbol, optionally with value
#                        Examples: -d DEBUG  or  -d LEVEL 5  or  -d RAZY=1
#   -64, -c64       Target C64 (default)
#   -128, -c128     Target C128
#   -20, -vic20     Target VIC-20
#   -h, -?          Show this help
#
# Start address (optional last argument):
#   Must be hex ($xxxx or 0xXXXX) or decimal digits, or omit for BASIC stub

TARGET="c64"
LIBNAME="LAMAlib.lib"
ASMDEF=""
VERBOSE=""
LINKMODE=""
OUTFILE=""

FILES=()
MAINFILE=""
STARTADDR=""
FILECOUNT=0

# ============================================
# Function: is_number
# Check if argument is a number (hex or decimal)
# Returns: 0 if number, 1 if not
# ============================================
is_number() {
  local val="$1"
  
  [[ -z "$val" ]] && return 1
  
  # Check if starts with $ (hex)
  [[ "${val:0:1}" == '$' ]] && return 0
  
  # Check if starts with 0x or 0X (hex)
  [[ "${val:0:2}" =~ ^[0][xX]$ ]] && return 0
  
  # Check if pure decimal digits
  [[ "$val" =~ ^[0-9]+$ ]] && return 0
  
  return 1
}

# ============================================
# Function: show_usage
# ============================================
show_usage() {
  cat << EOF
Usage: $(basename "$0") [options] file1.s [file2.s ...] [startaddr]

Assemble ca65 sources into PRG files for C64/C128/VIC-20
Automatically runs exprass on sources containing "let" statements

Options:
  -l              Link multiple files together (default: assemble separately)
  -o <file>       Output filename (only valid with single file or -l mode)
  -v              Verbose mode
  -d <symbol> [value]  Define assembler symbol, optionally with value
                       Examples: -d DEBUG  or  -d LEVEL 5  or  -d RAZY=1
  -64, -c64       Target C64 (default)
  -128, -c128     Target C128
  -20, -vic20     Target VIC-20
  -h, -?          Show this help

Start address (optional last argument):
  Must be hex (\$xxxx or 0xXXXX) or decimal digits
  Omit for automatic BASIC stub

Examples:
  $(basename "$0") main.s                        Assemble single file to main.prg
  $(basename "$0") -l main.s util.s              Link two files into main.prg
  $(basename "$0") -l -o game.prg main.s util.s  Link into game.prg
  $(basename "$0") *.s                           Assemble all .s files separately
  $(basename "$0") -d DEBUG main.s               Define DEBUG symbol
  $(basename "$0") -d LEVEL 5 main.s             Define LEVEL=5
  $(basename "$0") -d RAZY=1 main.s              Define RAZY=1 (= gets split, then recombined)
  $(basename "$0") main.s \$c000                  Assemble to address \$c000
EOF
  exit 1
}

# --------------------------------------------------
# argument check
# --------------------------------------------------

[[ -z "$1" ]] && show_usage

# --------------------------------------------------
# option parsing
# --------------------------------------------------

while true; do
  [[ -z "$1" ]] && break
  
  case "$1" in
    -h|-\?)
      show_usage
      ;;
    -v)
      VERBOSE=1
      shift
      ;;
    -l)
      LINKMODE=1
      shift
      ;;
    -o)
      if [[ -z "$2" ]]; then
        echo "ERROR: -o requires output filename"
        exit 1
      fi
      OUTFILE="$2"
      shift 2
      ;;
    -d)
      if [[ -z "$2" ]]; then
        echo "ERROR: -d requires symbol definition"
        exit 1
      fi
      
      # Check if $3 is a number
      if is_number "$3"; then
        ASMDEF="$ASMDEF --asm-define $2=$3"
        shift 3
      else
        ASMDEF="$ASMDEF --asm-define $2"
        shift 2
      fi
      ;;
    -128|-c128)
      TARGET="c128"
      LIBNAME="LAMAlib128.lib"
      shift
      ;;
    -64|-c64)
      TARGET="c64"
      LIBNAME="LAMAlib.lib"
      shift
      ;;
    -20|-vc20|-vic20)
      TARGET="vic20"
      LIBNAME="LAMAlib20.lib"
      shift
      ;;
    -*)
      echo "ERROR: Unknown option: $1"
      echo
      show_usage
      ;;
    *)
      break
      ;;
  esac
done

# --------------------------------------------------
# Function: process_file
# Process a single file: run exprass if needed, add to FILES array
# --------------------------------------------------
process_file() {
  local srcfile="$1"
  
  # First source file defines default output name
  if [[ -z "$MAINFILE" ]]; then
    MAINFILE="${srcfile%.*}"
  fi
  
  ((FILECOUNT++))
  
  # Check for exprass: leading whitespace (spaces or tabs) + let
  if grep -P '^[\t ]*let ' "$srcfile" > /dev/null 2>&1; then
    echo "exprass: compiling $srcfile"
    [[ "$VERBOSE" == "1" ]] && echo "exprass -c \"$srcfile\""
    
    if ! exprass -c "$srcfile"; then
      echo "ERROR: exprass failed for $srcfile"
      exit 1
    fi
    
    FILES+=( "${srcfile%.*}.asm" )
  else
    FILES+=( "$srcfile" )
  fi
}

# --------------------------------------------------
# collect files and optional start address
# --------------------------------------------------

while [[ -n "$1" ]]; do
  # Detect start address vs filename
  # If it has an extension, it's definitely a file
  if [[ "$1" == *.* ]]; then
    # It's a file - process it (handles wildcards via shell expansion)
    process_file "$1"
    shift
    continue
  fi
  
  # No extension - check if it's a number
  if is_number "$1"; then
    STARTADDR="$1"
    shift
    continue
  fi
  
  # Not a number and no extension - treat as file anyway
  process_file "$1"
  shift
done

if [[ $FILECOUNT -eq 0 ]]; then
  echo "ERROR: no source files given"
  exit 1
fi

# Validate -o usage
if [[ -n "$OUTFILE" ]]; then
  if [[ $FILECOUNT -gt 1 && -z "$LINKMODE" ]]; then
    echo "ERROR: -o can only be used with a single file or with -l (link mode)"
    exit 1
  fi
fi

# Don't set OUTFILE here - let each mode handle it

# --------------------------------------------------
# Function: assemble_single
# Assemble a single file to its own PRG
# --------------------------------------------------
assemble_single() {
  local asmfile="$1"
  local asmbase="${asmfile%.*}"
  local asmout
  
  # Use -o output filename if specified, otherwise default to input basename
  if [[ -n "$OUTFILE" ]]; then
    asmout="$OUTFILE"
  else
    asmout="${asmbase}.prg"
  fi
  
  [[ "$VERBOSE" == "1" ]] && echo "Assembling $asmfile to $asmout..."
  
  if [[ -z "$STARTADDR" ]]; then
    if grep -q makesys "$asmfile"; then
      [[ "$VERBOSE" == "1" ]] && \
        echo "cl65 -t $TARGET $ASMDEF -g \"$asmfile\" -lib $LIBNAME -C ${TARGET}-basicfriendly-asm.cfg -Ln ${asmbase}_labels.txt -o \"$asmout\""
      
      cl65 -t "$TARGET" $ASMDEF -g "$asmfile" \
           -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" \
           -Ln "${asmbase}_labels.txt" -o "$asmout"
    else
      [[ "$VERBOSE" == "1" ]] && \
        echo "cl65 -t $TARGET $ASMDEF -g \"$asmfile\" -lib $LIBNAME -C ${TARGET}-basicfriendly-asm.cfg -Ln ${asmbase}_labels.txt -u __EXEHDR__ -o \"$asmout\""
      
      cl65 -t "$TARGET" $ASMDEF -g "$asmfile" \
           -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" \
           -Ln "${asmbase}_labels.txt" -u __EXEHDR__ -o "$asmout"
    fi
  else
    [[ "$VERBOSE" == "1" ]] && \
      echo "cl65 -t $TARGET $ASMDEF -g \"$asmfile\" -lib $LIBNAME -C ${TARGET}-basicfriendly-asm.cfg -Ln ${asmbase}_labels.txt --start-addr $STARTADDR -o \"$asmout\""
    
    cl65 -t "$TARGET" $ASMDEF -g "$asmfile" \
         -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" \
         -Ln "${asmbase}_labels.txt" --start-addr "$STARTADDR" -o "$asmout"
  fi
}

# --------------------------------------------------
# assemble
# --------------------------------------------------

if [[ -z "$LINKMODE" ]]; then
  # Separate assembly mode (default)
  echo "Assembling files separately for target $TARGET..."
  for file in "${FILES[@]}"; do
    assemble_single "$file"
    if [[ $? -ne 0 ]]; then
      exit 1
    fi
  done
  echo "done."
  exit 0
fi

# Link mode

# Set output filename for link mode if not provided
if [[ -z "$OUTFILE" ]]; then
  OUTFILE="${MAINFILE}.prg"
fi

if [[ -z "$STARTADDR" ]]; then
  echo "Assembling and linking ${FILES[*]} for target $TARGET..."
  
  if grep -q makesys "${FILES[@]}"; then
    [[ "$VERBOSE" == "1" ]] && \
      echo "cl65 -t $TARGET $ASMDEF -g ${FILES[*]} -lib $LIBNAME -C ${TARGET}-basicfriendly-asm.cfg -Ln labels.txt -o \"$OUTFILE\""
    
    cl65 -t "$TARGET" $ASMDEF -g "${FILES[@]}" \
         -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" \
         -Ln "labels.txt" -o "$OUTFILE"
  else
    [[ "$VERBOSE" == "1" ]] && \
      echo "cl65 -t $TARGET $ASMDEF -g ${FILES[*]} -lib $LIBNAME -C ${TARGET}-basicfriendly-asm.cfg -Ln labels.txt -u __EXEHDR__ -o \"$OUTFILE\""
    
    cl65 -t "$TARGET" $ASMDEF -g "${FILES[@]}" \
         -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" \
         -Ln "labels.txt" -u __EXEHDR__ -o "$OUTFILE"
  fi
else
  echo "Assembling and linking ${FILES[*]} to start address $STARTADDR for target $TARGET..."
  
  [[ "$VERBOSE" == "1" ]] && \
    echo "cl65 -t $TARGET $ASMDEF -g ${FILES[*]} -lib $LIBNAME -C ${TARGET}-basicfriendly-asm.cfg -Ln labels.txt --start-addr $STARTADDR -o \"$OUTFILE\""
  
  cl65 -t "$TARGET" $ASMDEF -g "${FILES[@]}" \
       -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" \
       -Ln "labels.txt" --start-addr "$STARTADDR" -o "$OUTFILE"
fi

echo "done."
