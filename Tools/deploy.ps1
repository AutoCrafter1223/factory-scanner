param([Parameter(Mandatory=$true)][string]$PackageDirectory)
$ErrorActionPreference = 'Stop'
$workspacePath = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$packagePath = (Resolve-Path -LiteralPath $PackageDirectory).Path
$releasePath = Join-Path $workspacePath 'release'
if (-not $packagePath.StartsWith($releasePath + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Package must be inside this workspace release directory.' }
$targetPath = 'D:\SteamLibrary\steamapps\common\Satisfactory\FactoryGame\Mods\ItemScanner-Windows'
if ((Resolve-Path -LiteralPath $targetPath).Path -ne $targetPath) { throw 'Unexpected installation path.' }
if (Get-Process -Name 'FactoryGame*' -ErrorAction SilentlyContinue) { throw 'Satisfactory is running; do not overwrite loaded mod files.' }
$newManifest = Get-Content -Raw -LiteralPath (Join-Path $packagePath 'ItemScanner.uplugin') | ConvertFrom-Json
$oldManifest = Get-Content -Raw -LiteralPath (Join-Path $targetPath 'ItemScanner.uplugin') | ConvertFrom-Json
if ($newManifest.Modules.Name -notcontains 'ItemScanner' -or $oldManifest.Modules.Name -notcontains 'ItemScanner') { throw 'Not an ItemScanner package/installation.' }
foreach ($requiredFile in @('Binaries\Win64\FactoryGameSteam-ItemScanner-Win64-Shipping.dll','Content\Paks\Windows\ItemScannerFactoryGame-Windows.ucas','Content\Paks\Windows\ItemScannerFactoryGame-Windows.utoc')) {
    if (-not (Test-Path -LiteralPath (Join-Path $packagePath $requiredFile))) { throw "Missing package file: $requiredFile" }
}
$backupPath = Join-Path $releasePath ('installed-backup-' + $oldManifest.VersionName + '-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))
New-Item -ItemType Directory -Path $backupPath | Out-Null
Get-ChildItem -LiteralPath $targetPath -Force | Copy-Item -Destination $backupPath -Recurse -Force
foreach ($file in Get-ChildItem -LiteralPath $targetPath -Recurse -File) {
    $relative = $file.FullName.Substring($targetPath.Length + 1)
    if ((Get-FileHash -LiteralPath $file.FullName).Hash -ne (Get-FileHash -LiteralPath (Join-Path $backupPath $relative)).Hash) { throw "Backup hash mismatch: $relative" }
}
Write-Output "Verified backup: $backupPath"
# Keep unrelated files. Install all payloads before the version manifest.
Get-ChildItem -LiteralPath $packagePath -Force | Where-Object { $_.Name -ne 'ItemScanner.uplugin' } | Copy-Item -Destination $targetPath -Recurse -Force
Copy-Item -LiteralPath (Join-Path $packagePath 'ItemScanner.uplugin') -Destination (Join-Path $targetPath 'ItemScanner.uplugin') -Force
foreach ($file in Get-ChildItem -LiteralPath $packagePath -Recurse -File) {
    $relative = $file.FullName.Substring($packagePath.Length + 1)
    if ((Get-FileHash -LiteralPath $file.FullName).Hash -ne (Get-FileHash -LiteralPath (Join-Path $targetPath $relative)).Hash) { throw "Installed hash mismatch: $relative. Restore from $backupPath." }
}
Write-Output "Installed and hash-verified ItemScanner $($newManifest.VersionName): $targetPath"
