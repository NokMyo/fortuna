#!/bin/sh
# Cross-build identical assembly with MinGW binutils; no compiler runtime linked.
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
{
  echo '.intel_syntax noprefix'
  echo '.equ WINDOWS,1'
  while IFS= read -r module; do cat "src/$module.inc"; echo; done < scripts/modules.txt
} > build/fortuna_all.s
x86_64-w64-mingw32-as build/fortuna_all.s -o build/fortuna.obj
x86_64-w64-mingw32-windres --codepage=65001 -I . -I "$FORTUNA_MINGW_INCLUDE" --preprocessor=gcc --preprocessor-arg=-E --preprocessor-arg=-xc --preprocessor-arg=-DRC_INVOKED --preprocessor-arg=-D_WIN32 --preprocessor-arg=-I"$FORTUNA_MINGW_INCLUDE" resources/app.rc -O coff -o build/app.res
x86_64-w64-mingw32-ld --subsystem windows --entry WinMainCRTStartup --dynamicbase --high-entropy-va --nxcompat --no-insert-timestamp -L "$FORTUNA_MINGW_LIB" -o build/FebiusFortuna.exe build/fortuna.obj build/app.res -luser32 -lkernel32 -lgdi32 -lcomdlg32 -ladvapi32 -lshell32 -lcomctl32
