$ErrorActionPreference = 'Stop'
$version = (Get-Content VERSION -Raw).Trim()
$engineVersion = (Get-Content engine/VERSION -Raw).Trim()
if ($version -notmatch '^\d+\.\d+\.\d+$' -or $engineVersion -notmatch '^\d+\.\d+\.\d+$') { throw 'Invalid version' }
$dest = "dist/FebiusFortuna-$version-windows-x64"
New-Item -ItemType Directory -Force "$dest/docs", "$dest/data" | Out-Null
Copy-Item build/FebiusFortuna.exe,build/FortunaOracle.dll $dest
Copy-Item README.md,CHANGELOG.md $dest
Copy-Item docs/USER_GUIDE.md,docs/IMPLEMENTATION.md,docs/ENGINE_API.md,docs/ORACLE.md,docs/ORACLE_FIELD_ARCHITECTURE.md "$dest/docs"
Copy-Item data/FORMAT.md,data/SYNTHETIC-example.csv "$dest/data"
Compress-Archive -Path "$dest/*" -DestinationPath "$dest.zip" -Force
$sdk = "dist/FortunaOracle-Engine-$engineVersion-windows-x64-sdk"
New-Item -ItemType Directory -Force $sdk | Out-Null
Copy-Item build/FortunaOracle.dll,build/FortunaOracle.lib $sdk
Copy-Item sdk/include,sdk/examples -Destination $sdk -Recurse -Force
Copy-Item docs/ENGINE_API.md "$sdk/README.md"
Copy-Item engine/FortunaOracle.def $sdk
Compress-Archive -Path "$sdk/*" -DestinationPath "$sdk.zip" -Force
$files = @('build/FebiusFortuna.exe','build/FortunaOracle.dll', "$dest.zip", "$sdk.zip")
$hashes = foreach ($file in $files) { "$((Get-FileHash -Algorithm SHA256 $file).Hash.ToLower())  $([IO.Path]::GetFileName($file))" }
$hashes | Set-Content -Encoding ascii dist/SHA256SUMS.txt
