@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build
where clang >nul 2>nul || (echo LLVM clang is required & exit /b 1)
where link >nul 2>nul || (echo Use the x64 Visual Studio developer prompt & exit /b 1)
where rc >nul 2>nul || (echo Windows SDK resource compiler is required & exit /b 1)
clang --target=x86_64-pc-windows-msvc -c engine\fortuna_oracle.s -o build\oracle.obj || exit /b 1
rc /nologo /c65001 /fo build\engine.res engine\engine.rc || exit /b 1
link /nologo /dll /noentry /nodefaultlib /machine:x64 /dynamicbase /highentropyva /nxcompat /Brepro /def:engine\FortunaOracle.def /implib:build\FortunaOracle.lib /map:build\oracle.map /out:build\FortunaOracle.dll build\oracle.obj build\engine.res kernel32.lib advapi32.lib || exit /b 1
clang --target=x86_64-pc-windows-msvc -c src\fortuna.s -o build\fortuna.obj || exit /b 1
rc /nologo /c65001 /fo build\app.res resources\app.rc || exit /b 1
link /nologo /nodefaultlib /machine:x64 /subsystem:windows,6.02 /entry:WinMainCRTStartup /dynamicbase /highentropyva /nxcompat /Brepro /map:build\fortuna.map /out:build\FebiusFortuna.exe build\fortuna.obj build\app.res build\FortunaOracle.lib user32.lib kernel32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib || exit /b 1
clang --target=x86_64-pc-windows-msvc -c sdk\examples\generate.s -o build\example.obj || exit /b 1
link /nologo /nodefaultlib /entry:start /subsystem:console /out:build\oracle-example.exe build\example.obj build\FortunaOracle.lib kernel32.lib || exit /b 1
exit /b 0
