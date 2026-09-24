#!/usr/bin/env bash

set -euo pipefail

# AndroidTouchGrab build script
#
# Builds a statically linked ARM64/AArch64 binary suitable for
# compatible Android devices.

PROGRAM_NAME="touch_grab"
SOURCE_FILE="touch_grab.c"
COMPILER="${CC:-aarch64-linux-gnu-gcc}"

echo "AndroidTouchGrab build"
echo "======================"
echo

if [[ ! -f "$SOURCE_FILE" ]]; then
    echo "Error: source file '$SOURCE_FILE' not found." >&2
    exit 1
fi

if ! command -v "$COMPILER" >/dev/null 2>&1; then
    echo "Error: compiler '$COMPILER' not found." >&2
    echo
    echo "On Debian/Ubuntu, install it with:"
    echo "  sudo apt install gcc-aarch64-linux-gnu"
    exit 1
fi

echo "Compiler : $COMPILER"
echo "Source   : $SOURCE_FILE"
echo "Output   : $PROGRAM_NAME"
echo

"$COMPILER" \
    -static \
    -O2 \
    -Wall \
    -Wextra \
    -Wpedantic \
    -o "$PROGRAM_NAME" \
    "$SOURCE_FILE"

echo
echo "Build completed successfully."
echo

if command -v file >/dev/null 2>&1; then
    file "$PROGRAM_NAME"
fi

echo
echo "Binary created:"
echo "  ./$PROGRAM_NAME"
