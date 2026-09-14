#!/bin/sh
set -eu
: "${FORTUNA_MINGW_INCLUDE:=/usr/x86_64-w64-mingw32/include}"
: "${FORTUNA_MINGW_LIB:=/usr/x86_64-w64-mingw32/lib}"
cd "$(dirname "$0")/.."
mkdir -p build
x86_64-w64-mingw32-as fortuna_oracle.s -o build/oracle.obj
x86_64-w64-mingw32-ar rcs build/libFortunaOracleStatic.a build/oracle.obj
x86_64-w64-mingw32-ld --dll --entry 0 --dynamicbase --high-entropy-va --nxcompat --no-insert-timestamp --out-implib build/libFortunaOracle.a -L "$FORTUNA_MINGW_LIB" -o build/FortunaOracle.dll build/oracle.obj FortunaOracle.def -lkernel32 -ladvapi32
