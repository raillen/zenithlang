param(
    [string]$Version = "0.3.0-alpha.1",
    [string]$IsccPath = "iscc",
    [string]$OutputDir = "dist\\installer",
    [string]$StageDir = ".artifacts\\installer\\stage",
    [switch]$SkipStaging
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$issPath = (Resolve-Path (Join-Path $repoRoot "installer\\zenith.iss")).Path
$stageAbs = Join-Path $repoRoot $StageDir
$outputAbs = Join-Path $repoRoot $OutputDir

function Assert-Path([string]$PathValue, [string]$Label) {
    if (-not (Test-Path -LiteralPath $PathValue)) {
        throw "$Label not found: $PathValue"
    }
}

function Copy-IfExists([string]$Source, [string]$DestinationDir) {
    if (Test-Path -LiteralPath $Source) {
        Copy-Item -LiteralPath $Source -Destination $DestinationDir -Force
    }
}

if (-not $SkipStaging) {
    New-Item -ItemType Directory -Force -Path $stageAbs | Out-Null
    Get-ChildItem -Force -Path $stageAbs | Remove-Item -Recurse -Force

    $ztExe = Join-Path $repoRoot "zt.exe"
    $stdlib = Join-Path $repoRoot "stdlib"
    $readme = Join-Path $repoRoot "README.md"
    $changelog = Join-Path $repoRoot "CHANGELOG.md"
    $license = Join-Path $repoRoot "LICENSE"
    $licenseApache = Join-Path $repoRoot "LICENSE-APACHE"
    $licenseMit = Join-Path $repoRoot "LICENSE-MIT"

    Assert-Path $ztExe "Compiler binary"
    Assert-Path $stdlib "Standard library folder"
    Assert-Path $readme "README"
    Assert-Path $changelog "CHANGELOG"
    Assert-Path $license "LICENSE"
    Assert-Path $licenseApache "LICENSE-APACHE"
    Assert-Path $licenseMit "LICENSE-MIT"

    Copy-Item -LiteralPath $ztExe -Destination $stageAbs -Force
    Copy-Item -LiteralPath $stdlib -Destination (Join-Path $stageAbs "stdlib") -Recurse -Force
    Copy-Item -LiteralPath $readme -Destination $stageAbs -Force
    Copy-Item -LiteralPath $changelog -Destination $stageAbs -Force
    Copy-Item -LiteralPath $license -Destination $stageAbs -Force
    Copy-Item -LiteralPath $licenseApache -Destination $stageAbs -Force
    Copy-Item -LiteralPath $licenseMit -Destination $stageAbs -Force
}

Assert-Path $stageAbs "Installer stage directory"
New-Item -ItemType Directory -Force -Path $outputAbs | Out-Null

$isccCommand = Get-Command $IsccPath -ErrorAction SilentlyContinue
if ($null -eq $isccCommand) {
    throw "Inno Setup compiler not found ('$IsccPath'). Install Inno Setup and ensure 'iscc' is in PATH."
}

$outputBase = "zenith-$Version-windows-amd64-setup"
$stageForInno = (Resolve-Path $stageAbs).Path
$outputForInno = (Resolve-Path $outputAbs).Path

$isccArgs = @(
    $issPath,
    "/DMyAppVersion=$Version",
    "/DSourceDir=$stageForInno",
    "/DOutputDir=$outputForInno",
    "/DOutputBase=$outputBase"
)

& $isccCommand.Source @isccArgs

$setupExe = Join-Path $outputForInno "$outputBase.exe"
Assert-Path $setupExe "Generated installer"

$sha256 = (Get-FileHash -LiteralPath $setupExe -Algorithm SHA256).Hash.ToLowerInvariant()
$sha512 = (Get-FileHash -LiteralPath $setupExe -Algorithm SHA512).Hash.ToLowerInvariant()
$checksumFile = Join-Path $outputForInno "$outputBase.checksums.txt"

$checksumText = @(
    "SHA256  $sha256  $(Split-Path -Leaf $setupExe)",
    "SHA512  $sha512  $(Split-Path -Leaf $setupExe)"
)
$checksumText | Set-Content -Path $checksumFile -Encoding utf8

Write-Host "Installer generated:"
Write-Host "  $setupExe"
Write-Host "Checksum file:"
Write-Host "  $checksumFile"
