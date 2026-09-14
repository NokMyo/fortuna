$ErrorActionPreference = 'Stop'
$version = (Get-Content VERSION -Raw).Trim()
$dest = "dist/FebiusFortuna-$version-windows-x64"
New-Item -ItemType Directory -Force "$dest/docs", "$dest/engine/docs", "$dest/engine/data" | Out-Null
Copy-Item build/FebiusFortuna.exe $dest
Copy-Item README.md,CHANGELOG.md $dest
Copy-Item docs/USER_GUIDE.md,docs/ARCHITECTURE.md "$dest/docs"
Copy-Item engine/README.md,engine/VERSION "$dest/engine"
Copy-Item engine/docs/ENGINE_API.md,engine/docs/IMPLEMENTATION.md,engine/docs/ORACLE.md,engine/docs/ORACLE_FIELD_ARCHITECTURE.md "$dest/engine/docs"
Copy-Item engine/data/FORMAT.md,engine/data/SYNTHETIC-example.csv "$dest/engine/data"
Compress-Archive -Path "$dest/*" -DestinationPath "$dest.zip" -Force
$hashes = foreach ($file in @('build/FebiusFortuna.exe', "$dest.zip")) { "$((Get-FileHash -Algorithm SHA256 $file).Hash.ToLower())  $([IO.Path]::GetFileName($file))" }
$hashes | Set-Content -Encoding ascii dist/SHA256SUMS.txt
