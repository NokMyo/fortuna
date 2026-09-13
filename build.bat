@echo off
setlocal
if not exist build mkdir build
where clang >nul 2>nul || (echo clang not found & exit /b 1)
where lld-link >nul 2>nul || (echo lld-link not found & exit /b 1)
clang --target=x86_64-pc-windows-msvc -c src\fortuna.s -o build\fortuna.obj || exit /b 1
lld-link /nologo /subsystem:windows /entry:WinMainCRTStartup /out:build\FebiusFortuna.exe build\fortuna.obj user32.lib kernel32.lib gdi32.lib comdlg32.lib advapi32.lib || exit /b 1
echo Built build\FebiusFortuna.exe
