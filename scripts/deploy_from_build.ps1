param(
    [string]$BuildDir = "build",
    [string]$AppName = "Link2BOM",
    [string]$QtBin = "C:/Qt/6.10.1/mingw_64/bin",
    [switch]$NoQml
)

$ErrorActionPreference = "Stop"

$exePath = Join-Path $BuildDir "$AppName.exe"
if (-not (Test-Path $exePath)) {
    throw "No:$exePath`nPlease first build Release/MinSizeRel"
}

$distDir = Join-Path $BuildDir "dist"
if (Test-Path $distDir) {
    Remove-Item -Recurse -Force $distDir
}
New-Item -ItemType Directory -Force $distDir | Out-Null

$distExe = Join-Path $distDir "$AppName.exe"
Copy-Item -Force $exePath $distExe

$deployTool = Join-Path $QtBin "windeployqt.exe"
if (-not (Test-Path $deployTool)) {
    throw "no windeployqt:$deployTool"
}

$args = @("--release", "--compiler-runtime")
if ($NoQml) {
    $args += "--no-quick-import"
} else {
    $args += @("--qmldir", "src")
}
$args += $distExe

Write-Host "[deploy] $deployTool $($args -join ' ')"
& $deployTool @args

$zipPath = Join-Path $BuildDir "$AppName-windows-portable.zip"
if (Test-Path $zipPath) {
    Remove-Item -Force $zipPath
}
Compress-Archive -Path (Join-Path $distDir "*") -DestinationPath $zipPath -CompressionLevel Optimal

Write-Host "`nComplete:"
Write-Host "- release dir: $distDir"
Write-Host "- zip:   $zipPath"
