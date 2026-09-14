@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build
clang --target=x86_64-pc-windows-msvc -c fortuna_oracle.s -o build\oracle.obj || exit /b 1
lib /nologo /out:build\FortunaOracleStatic.lib build\oracle.obj || exit /b 1
rc /nologo /c65001 /fo build\engine.res engine.rc || exit /b 1
link /nologo /dll /noentry /nodefaultlib /machine:x64 /dynamicbase /highentropyva /nxcompat /Brepro /def:FortunaOracle.def /implib:build\FortunaOracle.lib /out:build\FortunaOracle.dll build\oracle.obj build\engine.res kernel32.lib advapi32.lib || exit /b 1
clang --target=x86_64-pc-windows-msvc -c sdk\examples\generate.s -o build\example.obj || exit /b 1
link /nologo /nodefaultlib /entry:start /subsystem:console /out:build\oracle-example.exe build\example.obj build\FortunaOracle.lib kernel32.lib || exit /b 1
exit /b 0
