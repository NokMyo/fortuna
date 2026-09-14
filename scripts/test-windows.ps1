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
Get-Content engine/data/SYNTHETIC-example.csv | Select-Object -First 61 | Set-Content -Encoding utf8 build/research-60.csv
Run-Checked '--research build/research-60.csv build/research-report.txt'
if ((Get-Content -Raw build/research-report.txt) -notmatch 'ORACLE RESEARCH 1.1') { throw 'Research command did not run the suite' }
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
 [DllImport("user32.dll")] public static extern bool IsWindowEnabled(IntPtr h);
 [DllImport("user32.dll")] public static extern IntPtr GetDlgItem(IntPtr h, int id);
 [DllImport("user32.dll",CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
 [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
 [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr dc, uint flags);
 [DllImport("user32.dll",EntryPoint="SendMessageW",CharSet=CharSet.Unicode)] public static extern IntPtr ReadControl(IntPtr h,uint msg,IntPtr n,[Out] StringBuilder s);
 [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr h,uint msg,IntPtr w,IntPtr l);
 [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h,uint msg,IntPtr w,IntPtr l);
}
'@
$p = Start-Process -FilePath $exe -ArgumentList '--open build/research-60.csv' -PassThru
try {
    $p.WaitForInputIdle(10000) | Out-Null
    for ($i=0; $i -lt 100; $i++) { Start-Sleep -Milliseconds 100; $p.Refresh(); if ($p.MainWindowHandle -ne 0 -and [WindowCheck]::GetDlgItem($p.MainWindowHandle,1102) -ne [IntPtr]::Zero) { break } }
    $h = $p.MainWindowHandle
    Write-Host "GUI handle=$h title=$($p.MainWindowTitle) tickets=$([WindowCheck]::GetDlgItem($h,1102))"
    if ($h -eq 0) { throw 'GUI window was not created' }
    function Read-Text([int]$id) {
        $t = [Text.StringBuilder]::new(4096)
        [WindowCheck]::ReadControl([WindowCheck]::GetDlgItem($h,$id),0xD,[IntPtr]4096,$t) | Out-Null
        return $t.ToString()
    }
    function Command([int]$id) { [WindowCheck]::SendMessage($h,0x111,[IntPtr]$id,[IntPtr]::Zero) | Out-Null }
    function Wait-Analysis {
        $deadline = [DateTime]::UtcNow.AddSeconds(180)
        while ([WindowCheck]::IsWindowEnabled([WindowCheck]::GetDlgItem($h,1004))) {
            if ([DateTime]::UtcNow -gt $deadline) { throw 'GUI analysis timed out' }
            Start-Sleep -Milliseconds 100
        }
    }
    foreach ($id in @(1003,1002,1019)) {
        if (-not (Read-Text $id).Contains('추첨')) { throw "Missing draw button $id" }
    }
    Command 1121
    Command 1003
    if ((Read-Text 1103) -notmatch '균등 무작위') { throw 'Random route incorrect' }
    Command 1002
    Wait-Analysis
    if ((Read-Text 1103) -notmatch 'ORACLE' -or (Read-Text 1103) -match '심층') { throw 'General route incorrect' }
    Command 1019
    Wait-Analysis
    if ((Read-Text 1103) -notmatch '심층' -or (Read-Text 1104) -notmatch '공동 확률') { throw 'Deep route incorrect' }
    Command 1003
    if ((Read-Text 1103) -notmatch '균등 무작위') { throw 'Random after deep route incorrect' }
    Command 1122
    Command 1019
    if ([WindowCheck]::IsWindowEnabled([WindowCheck]::GetDlgItem($h,1004))) { throw 'Deep cache was lost by uniform draw' }
    if ((Read-Text 1103) -notmatch '심층') { throw 'Cached deep route incorrect' }
    $ten = @((Read-Text 1102) -split '\r?\n' | Where-Object { $_.Trim() })
    if ($ten.Count -ne 10) { throw 'Shared 10-game selection failed' }
    Command 1121
    Command 1003
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
