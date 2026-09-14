$ErrorActionPreference = 'Stop'
New-Item -ItemType Directory -Force build/standalone | Out-Null
Copy-Item build/FebiusFortuna.exe build/standalone/FebiusFortuna.exe
$exe = (Resolve-Path build/standalone/FebiusFortuna.exe).Path

function Run-Checked([string]$arguments, [int]$expected = 0) {
    $p = Start-Process -FilePath $exe -ArgumentList $arguments -PassThru
    if (-not $p.WaitForExit(180000)) { $p.Kill(); throw "Timed out: $arguments" }
    if ($p.ExitCode -ne $expected) {
        python tests/debug-windows.py $exe $arguments
        throw "Exit $($p.ExitCode), expected $expected : $arguments"
    }
}

# Diagnostics that do not expose product functionality remain runnable without an account.
Run-Checked '--self-test'
Run-Checked '--unknown-argument' 2

$imports = (& dumpbin /imports $exe | Out-String)
if ($imports -match '(?i)FortunaOracle.dll') {
    throw 'App must include the engine; external engine DLL dependency found'
}
$imports | Set-Content build/imports.txt
if ($imports -match '(?i)(msvcrt|vcruntime|ucrtbase|api-ms-win-crt)') {
    throw 'Unexpected C runtime dependency'
}
if ($imports -notmatch '(?i)WINHTTP.dll') {
    throw 'Febius Account network client is not linked'
}
if ($imports -notmatch '(?i)CRYPT32.dll') {
    throw 'DPAPI support is not linked'
}

# Authentication and license enforcement are intentionally not bypassed by CI.
# ORACLE math, research and native engine behavior are validated by the engine and SDK jobs.
Write-Host 'PASS Windows build, self-test, CLI error handling, static engine, CRT independence and account dependencies'
