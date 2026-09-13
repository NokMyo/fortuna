$ErrorActionPreference = 'Stop'
$version = (Get-Content VERSION -Raw).Trim()
if ($version -notmatch '^\d+\.\d+\.\d+$') { throw 'Invalid version' }
$dest = "dist/FebiusFortuna-$version-windows-x64"
New-Item -ItemType Directory -Force "$dest/docs", "$dest/data" | Out-Null
Copy-Item build/FebiusFortuna.exe $dest
Copy-Item README.md,CHANGELOG.md $dest
Copy-Item docs/USER_GUIDE.md,docs/IMPLEMENTATION.md,docs/ORACLE.md,docs/ORACLE_FIELD_ARCHITECTURE.md "$dest/docs"
Copy-Item data/FORMAT.md,data/SYNTHETIC-example.csv "$dest/data"
Compress-Archive -Path "$dest/*" -DestinationPath "$dest.zip" -Force
$files = @('build/FebiusFortuna.exe', "$dest.zip")
$hashes = foreach ($file in $files) { "$((Get-FileHash -Algorithm SHA256 $file).Hash.ToLower())  $([IO.Path]::GetFileName($file))" }
$hashes | Set-Content -Encoding ascii dist/SHA256SUMS.txt
