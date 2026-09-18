@echo off
setlocal
cd /d "%~dp0"
if not exist engine\build.bat (echo Missing engine\build.bat; restore the engine folder from this repository & exit /b 1)
if not exist build mkdir build
pushd engine
call build.bat
set "engine_build_status=%errorlevel%"
popd
if not "%engine_build_status%"=="0" exit /b 1
clang --target=x86_64-pc-windows-msvc -c src\fortuna.s -o build\fortuna.obj || exit /b 1
rc /nologo /c65001 /fo build\app.res resources\app.rc || exit /b 1
link /nologo /nodefaultlib /machine:x64 /subsystem:windows,6.02 /entry:WinMainCRTStartup /dynamicbase /highentropyva /nxcompat /release /incremental:no /Brepro /map:build\fortuna.map /out:build\FebiusFortuna.exe build\fortuna.obj build\app.res engine\build\FortunaOracleStatic.lib user32.lib kernel32.lib gdi32.lib comdlg32.lib shell32.lib comctl32.lib bcrypt.lib winhttp.lib crypt32.lib || exit /b 1
exit /b 0
