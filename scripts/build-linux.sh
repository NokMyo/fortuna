#!/bin/sh
set -eu
: "${FORTUNA_MINGW_INCLUDE:=/usr/x86_64-w64-mingw32/include}"
: "${FORTUNA_MINGW_LIB:=/usr/x86_64-w64-mingw32/lib}"
cd "$(dirname "$0")/.."
mkdir -p build
resource() {
 x86_64-w64-mingw32-windres --codepage=65001 -I . -I "$FORTUNA_MINGW_INCLUDE" --preprocessor=gcc --preprocessor-arg=-E --preprocessor-arg=-xc --preprocessor-arg=-DRC_INVOKED --preprocessor-arg=-D_WIN32 --preprocessor-arg=-I"$FORTUNA_MINGW_INCLUDE" "$1" -O coff -o "$2"
}
x86_64-w64-mingw32-as engine/fortuna_oracle.s -o build/oracle.obj
resource engine/engine.rc build/engine.res
x86_64-w64-mingw32-ld --dll --entry 0 --dynamicbase --high-entropy-va --nxcompat --no-insert-timestamp --out-implib build/libFortunaOracle.a -L "$FORTUNA_MINGW_LIB" -o build/FortunaOracle.dll build/oracle.obj build/engine.res engine/FortunaOracle.def -lkernel32 -ladvapi32
x86_64-w64-mingw32-as src/fortuna.s -o build/fortuna.obj
resource resources/app.rc build/app.res
x86_64-w64-mingw32-ld --subsystem windows --entry WinMainCRTStartup --dynamicbase --high-entropy-va --nxcompat --no-insert-timestamp -L "$FORTUNA_MINGW_LIB" -o build/FebiusFortuna.exe build/fortuna.obj build/app.res build/libFortunaOracle.a -luser32 -lkernel32 -lgdi32 -lcomdlg32 -lshell32 -lcomctl32
