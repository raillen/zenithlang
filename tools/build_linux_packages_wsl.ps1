param(
    [string]$Version = "0.4.1-alpha.1",
    [string]$Distro = "",
    [string]$OutputRoot = "dist",
    [string]$Fpm = "fpm",
    [string]$Iteration = "1",
    [string]$Maintainer = "Zenith Team <maintainers@zenithlang.dev>",
    [switch]$SkipBuild
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

function Convert-ToBashSingleQuoted([string]$Value) {
    return "'" + ($Value -replace "'", "'\''") + "'"
}

function Convert-WindowsPathToWsl([string]$PathValue) {
    $fullPath = (Resolve-Path -LiteralPath $PathValue).Path
    if ($fullPath -match '^([A-Za-z]):\\(.*)$') {
        $drive = $Matches[1].ToLowerInvariant()
        $rest = $Matches[2] -replace '\\', '/'
        return "/mnt/$drive/$rest"
    }
    throw "Unsupported Windows path for WSL conversion: $fullPath"
}

$wslArgs = @()
if ($Distro -ne "") {
    $wslArgs += @("-d", $Distro)
}

$repoLinux = Convert-WindowsPathToWsl $repoRoot

$skipBuildArg = ""
if ($SkipBuild) {
    $skipBuildArg = " --skip-build"
}

$bash = @(
    "set -euo pipefail",
    "cd $(Convert-ToBashSingleQuoted $repoLinux)",
    "command -v python3 >/dev/null || { echo 'missing python3 in WSL' >&2; exit 127; }",
    "command -v gcc >/dev/null || { echo 'missing gcc in WSL' >&2; exit 127; }",
    "test -x $(Convert-ToBashSingleQuoted $Fpm) || command -v $(Convert-ToBashSingleQuoted $Fpm) >/dev/null || { echo 'missing fpm in WSL' >&2; exit 127; }",
    "python3 tools/build_installers.py --target linux --version $(Convert-ToBashSingleQuoted $Version) --output-root $(Convert-ToBashSingleQuoted $OutputRoot) --fpm $(Convert-ToBashSingleQuoted $Fpm) --iteration $(Convert-ToBashSingleQuoted $Iteration) --maintainer $(Convert-ToBashSingleQuoted $Maintainer)$skipBuildArg"
) -join "; "

& wsl @wslArgs -- bash -lc $bash
if ($LASTEXITCODE -ne 0) {
    throw "WSL Linux package build failed with exit code $LASTEXITCODE."
}
