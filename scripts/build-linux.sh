#!/bin/sh
set -eu
: "${FORTUNA_MINGW_INCLUDE:=/usr/x86_64-w64-mingw32/include}"
: "${FORTUNA_MINGW_LIB:=/usr/x86_64-w64-mingw32/lib}"
export FORTUNA_MINGW_INCLUDE FORTUNA_MINGW_LIB
cd "$(dirname "$0")/.."
sh engine/scripts/build-linux.sh
mkdir -p build
x86_64-w64-mingw32-as src/fortuna.s -o build/fortuna.obj
x86_64-w64-mingw32-windres --codepage=65001 -I . -I "$FORTUNA_MINGW_INCLUDE" --preprocessor=gcc --preprocessor-arg=-E --preprocessor-arg=-xc --preprocessor-arg=-DRC_INVOKED --preprocessor-arg=-D_WIN32 --preprocessor-arg=-I"$FORTUNA_MINGW_INCLUDE" resources/app.rc -O coff -o build/app.res
x86_64-w64-mingw32-ld --subsystem windows --entry WinMainCRTStartup --dynamicbase --high-entropy-va --nxcompat --no-insert-timestamp -L "$FORTUNA_MINGW_LIB" -o build/FebiusFortuna.exe build/fortuna.obj build/app.res engine/build/libFortunaOracleStatic.a -luser32 -lkernel32 -lgdi32 -lcomdlg32 -lshell32 -lcomctl32 -lbcrypt -lwinhttp -lcrypt32
