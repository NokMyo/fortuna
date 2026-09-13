@echo off
setlocal
cd /d "%~dp0"
if not exist build mkdir build
where clang >nul 2>nul || (echo LLVM clang is required & exit /b 1)
where link >nul 2>nul || (echo Run from the x64 Visual Studio developer prompt & exit /b 1)
where rc >nul 2>nul || (echo Windows SDK resource compiler is required & exit /b 1)
>build\fortuna_all.s echo .intel_syntax noprefix
>>build\fortuna_all.s echo .equ WINDOWS,1
for /f "usebackq delims=" %%m in ("scripts\modules.txt") do (
  type "src\%%m.inc" >>build\fortuna_all.s
  echo.>>build\fortuna_all.s
)
clang --target=x86_64-pc-windows-msvc -c build\fortuna_all.s -o build\fortuna.obj || exit /b 1
rc /nologo /c65001 /fo build\app.res resources\app.rc || exit /b 1
link /nologo /nodefaultlib /machine:x64 /subsystem:windows,6.02 /entry:WinMainCRTStartup /dynamicbase /highentropyva /nxcompat /Brepro /out:build\FebiusFortuna.exe build\fortuna.obj build\app.res user32.lib kernel32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib || exit /b 1
exit /b 0
