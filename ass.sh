#!/bin/bash

# ass.sh – assemble ca65 sources into a PRG
# Runs exprass on each source that contains "let"
# Adds BASIC stub unless code already provides one or a custom start address is given

TARGET="c64"
LIBNAME="LAMAlib.lib"
ASMDEF=""
VERBOSE=""

FILES=()
MAINFILE=""
STARTADDR=""

# --------------------------------------------------
# argument check
# --------------------------------------------------

if [[ -z "$1" ]]; then
  echo "Usage: $0 [options] file1.s [file2.s ...] [startaddr]"
  echo "startaddr must start with digit or \$"
  exit 1
fi

# --------------------------------------------------
# option parsing
# --------------------------------------------------

while true; do
  case "$1" in
    -v)
      VERBOSE=1
      shift
      ;;
    -d)
      ASMDEF="--asm-define $2"
      shift 2
      ;;
    -128|-c128)
      TARGET="c128"
      LIBNAME="LAMAlib128.lib"
      shift
      ;;
    -64|-c64)
      TARGET="c64"
      shift
      ;;
    -20|-vc20|-vic20)
      TARGET="vic20"
      LIBNAME="LAMAlib20.lib"
      shift
      ;;
    *)
      break
      ;;
  esac
done

# --------------------------------------------------
# collect files and optional start address
# --------------------------------------------------

while [[ -n "$1" ]]; do

  # start address: starts with digit or $
  if [[ "$1" =~ ^[0-9\$] ]]; then
    STARTADDR="$1"
    shift
    continue
  fi

  # first source file defines output name
  if [[ -z "$MAINFILE" ]]; then
    MAINFILE="${1%.*}"
  fi

  # exprass detection: leading whitespace (spaces or tabs) + let
  if grep -P '^[\t ]*let ' "$1" > /dev/null; then
    echo "exprass: compiling $1"
    [[ "$VERBOSE" == "1" ]] && echo "exprass -c \"$1\""

    if ! exprass -c "$1"; then
      echo "ERROR: exprass failed for $1"
      exit 1
    fi

    FILES+=( "${1%.*}.asm" )
  else
    FILES+=( "$1" )
  fi

  shift
done

if [[ -z "$MAINFILE" ]]; then
  echo "ERROR: no source files given"
  exit 1
fi

# --------------------------------------------------
# assemble
# --------------------------------------------------

if [[ -z "$STARTADDR" ]]; then
  echo "assembling ${FILES[*]} for target $TARGET..."

  if grep -q makesys "${FILES[@]}"; then
    [[ "$VERBOSE" == "1" ]] && \
      echo "cl65 -t $TARGET $ASMDEF -g ${FILES[*]} -lib $LIBNAME -C ${TARGET}-basicfriendly-asm.cfg -Ln labels.txt -o ${MAINFILE}.prg"

    cl65 -t "$TARGET" $ASMDEF -g "${FILES[@]}" \
         -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" \
         -Ln "labels.txt" -o "${MAINFILE}.prg"
  else
    [[ "$VERBOSE" == "1" ]] && \
      echo "cl65 -t $TARGET $ASMDEF -g ${FILES[*]} -lib $LIBNAME -C ${TARGET}-basicfriendly-asm.cfg -Ln labels.txt -u __EXEHDR__ -o ${MAINFILE}.prg"

    cl65 -t "$TARGET" $ASMDEF -g "${FILES[@]}" \
         -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" \
         -Ln "labels.txt" -u __EXEHDR__ -o "${MAINFILE}.prg"
  fi

else
  echo "assembling ${FILES[*]} to start address $STARTADDR for target $TARGET..."

  [[ "$VERBOSE" == "1" ]] && \
    echo "cl65 -t $TARGET $ASMDEF -g ${FILES[*]} -lib $LIBNAME -C ${TARGET}-basicfriendly-asm.cfg -Ln labels.txt --start-addr $STARTADDR -o ${MAINFILE}.prg"

  cl65 -t "$TARGET" $ASMDEF -g "${FILES[@]}" \
       -lib "$LIBNAME" -C "${TARGET}-basicfriendly-asm.cfg" \
       -Ln "labels.txt" --start-addr "$STARTADDR" -o "${MAINFILE}.prg"
fi

echo "done."
