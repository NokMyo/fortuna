@echo off
setlocal
if not exist build mkdir build
where clang >nul 2>nul || (echo clang not found & exit /b 1)
where dumpbin >nul 2>nul || (echo dumpbin not found & exit /b 1)
where link >nul 2>nul || (echo MSVC link.exe not found & exit /b 1)

rem Windows-hosted Clang has inconsistent nested .include handling for GAS-style
rem assembly. Keep source modules separate, but assemble one flattened unit.
(
  echo .intel_syntax noprefix
  type src\platform.inc
  echo.
  type src\state.inc
  echo.
  echo .section .text
  echo .globl WinMainCRTStartup
  type src\ui.inc
  echo.
  type src\data.inc
  echo.
  type src\oracle.inc
  echo.
  type src\sample.inc
  echo.
)>build\fortuna_all.s

clang --target=x86_64-pc-windows-msvc -c build\fortuna_all.s -o build\fortuna.obj || exit /b 1

dumpbin /symbols build\fortuna.obj | findstr /C:"WinMainCRTStartup" >nul || (
  echo WinMainCRTStartup was not exported by the assembler
  dumpbin /symbols build\fortuna.obj
  exit /b 1
)

link /nologo /nodefaultlib /subsystem:windows /entry:WinMainCRTStartup /out:build\FebiusFortuna.exe build\fortuna.obj user32.lib kernel32.lib gdi32.lib comdlg32.lib advapi32.lib || exit /b 1

echo Built build\FebiusFortuna.exe
