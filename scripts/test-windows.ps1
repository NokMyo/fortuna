$ErrorActionPreference = 'Stop'
New-Item -ItemType Directory -Force build/standalone | Out-Null
Copy-Item build/FebiusFortuna.exe build/standalone/FebiusFortuna.exe
$exe = (Resolve-Path build/standalone/FebiusFortuna.exe).Path
function Run-Checked([string]$arguments, [int]$expected = 0) {
    $p = Start-Process -FilePath $exe -ArgumentList $arguments -PassThru
    if (-not $p.WaitForExit(180000)) { $p.Kill(); throw "Timed out: $arguments" }
    if ($p.ExitCode -ne $expected) { python tests/debug-windows.py $exe $arguments; throw "Exit $($p.ExitCode), expected $expected : $arguments" }
}
Run-Checked '--self-test'
New-Item -ItemType Directory -Force 'build/한글 경로' | Out-Null
Copy-Item engine/data/SYNTHETIC-example.csv 'build/한글 경로/시험.csv'
Run-Checked '--analyze "build/한글 경로/시험.csv" "build/한글 경로/report.txt"'
$report = Get-Content -Raw -Encoding utf8 'build/한글 경로/report.txt'
if ($report -notmatch 'Candidate-field SHA-256' -or $report -notmatch 'Independent policy confirmation') { throw 'Incomplete report' }
Set-Content -Encoding utf8 build/invalid.csv '1,1,1,3,4,5,6'
Run-Checked '--analyze build/invalid.csv build/invalid-report.txt' 1
if (Test-Path build/invalid-report.txt) { throw 'Rejected CSV produced a report' }
Run-Checked '--unknown-argument' 2
$imports = (& dumpbin /imports $exe | Out-String)
if ($imports -match '(?i)FortunaOracle.dll') { throw 'App must include the engine; external engine DLL dependency found' }
$imports | Set-Content build/imports.txt
if ($imports -match '(?i)(msvcrt|vcruntime|ucrtbase|api-ms-win-crt)') { throw 'Unexpected C runtime dependency' }
# Exercise the actual window, controls, bundle generation and clipboard.
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class WindowCheck {
 [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L,T,R,B; }
 [DllImport("user32.dll")] public static extern IntPtr GetDlgItem(IntPtr h, int id);
 [DllImport("user32.dll",CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
 [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
 [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint flags);
 [DllImport("user32.dll",EntryPoint="SendMessageW",CharSet=CharSet.Unicode)] public static extern IntPtr ReadControl(IntPtr h,uint msg,IntPtr n,[Out] StringBuilder s);
 [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr h,uint msg,IntPtr w,IntPtr l);
 [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h,uint msg,IntPtr w,IntPtr l);
}
'@
$p = Start-Process -FilePath $exe -PassThru
try {
    $p.WaitForInputIdle(10000) | Out-Null
    for ($i=0; $i -lt 100; $i++) { Start-Sleep -Milliseconds 100; $p.Refresh(); if ($p.MainWindowHandle -ne 0 -and [WindowCheck]::GetDlgItem($p.MainWindowHandle,1102) -ne [IntPtr]::Zero) { break } }
    $h = $p.MainWindowHandle
    Write-Host "GUI handle=$h title=$($p.MainWindowTitle) tickets=$([WindowCheck]::GetDlgItem($h,1102))"
    if ($h -eq 0) { throw 'GUI window was not created' }
    [WindowCheck]::SendMessage($h,0x111,[IntPtr]1005,[IntPtr]::Zero) | Out-Null
    $rect = [WindowCheck+RECT]::new()
    if (-not [WindowCheck]::GetWindowRect($h,[ref]$rect)) { throw 'Cannot measure GUI' }
    $bmp = [Drawing.Bitmap]::new($rect.R-$rect.L,$rect.B-$rect.T)
    $graphics = [Drawing.Graphics]::FromImage($bmp)
    $dc = $graphics.GetHdc()
    try { if (-not [WindowCheck]::PrintWindow($h,$dc,0)) { throw 'Cannot capture GUI' } }
    finally { $graphics.ReleaseHdc($dc) }
    $bmp.Save((Join-Path $PWD 'build/fortuna-windows.png'),[Drawing.Imaging.ImageFormat]::Png)
    $graphics.Dispose(); $bmp.Dispose()
    Write-Host ("GUI_PREVIEW_BASE64=" + [Convert]::ToBase64String([IO.File]::ReadAllBytes((Join-Path $PWD "build/fortuna-windows.png"))))
    $text = [Text.StringBuilder]::new(4096)
    $length = [WindowCheck]::ReadControl([WindowCheck]::GetDlgItem($h,1102),0xD,[IntPtr]4096,$text)
    Write-Host "Read $length UTF-16 characters from ticket control"
    $text.ToString() | Set-Content build/gui-tickets.txt
    $lines = @($text.ToString() -split '\r?\n' | Where-Object { $_.Trim() })
    if ($lines.Count -ne 5) { throw "Expected 5 ticket rows, got $($lines.Count): $text" }
    foreach ($line in $lines) {
        # Each row begins with a game index followed by exactly six ball numbers.
        $nums = @([regex]::Matches($line,'\d+') | ForEach-Object { [int]$_.Value })
        if ($nums.Count -lt 6) { throw "Incomplete ticket: $line" }
        $balls = $nums[($nums.Count-6)..($nums.Count-1)]
        if (@($balls | Select-Object -Unique).Count -ne 6 -or @($balls | Where-Object { $_ -lt 1 -or $_ -gt 45 }).Count) { throw "Invalid ticket: $line" }
    }
    [WindowCheck]::SendMessage($h,0x111,[IntPtr]1007,[IntPtr]::Zero) | Out-Null
    [WindowCheck]::PostMessage($h,0x10,[IntPtr]::Zero,[IntPtr]::Zero) | Out-Null
    if (-not $p.WaitForExit(10000)) { throw 'GUI did not close' }
    if ($p.ExitCode -ne 0) { throw "GUI exit $($p.ExitCode)" }
} finally { if (-not $p.HasExited) { $p.Kill() } }
Write-Host 'PASS Windows self-test, Unicode paths, CLI errors, CRT imports, native GUI and tickets'
