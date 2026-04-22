param(
    [string]$InstallRoot = "",
    [string]$ProjectPath = "examples\\hello-world\\zenith.ztproj",
    [string]$LogPath = "docs\\reports\\release\\artifacts\\installer-validation.log"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

function Resolve-InstallRoot {
    param([string]$Value)

    if ($Value -and $Value.Trim().Length -gt 0) {
        return $Value
    }

    if ($env:ZENITH_HOME -and $env:ZENITH_HOME.Trim().Length -gt 0) {
        return $env:ZENITH_HOME
    }

    $defaultPath = Join-Path $env:ProgramFiles "Zenith"
    if (Test-Path -LiteralPath $defaultPath) {
        return $defaultPath
    }

    throw "Install root not found. Pass -InstallRoot or set ZENITH_HOME."
}

function Assert-Exists([string]$PathValue, [string]$Label) {
    if (-not (Test-Path -LiteralPath $PathValue)) {
        throw "$Label not found: $PathValue"
    }
}

function Run-Step([string]$Name, [string]$Exe, [string[]]$Args, [string]$LogFile) {
    Add-Content -Path $LogFile -Value ""
    Add-Content -Path $LogFile -Value ">>> $Name"
    Add-Content -Path $LogFile -Value "$Exe $($Args -join ' ')"
    & $Exe @Args 2>&1 | Tee-Object -FilePath $LogFile -Append | Out-Host
    if ($LASTEXITCODE -ne 0) {
        throw "$Name failed with exit code $LASTEXITCODE."
    }
}

$installResolved = Resolve-InstallRoot -Value $InstallRoot
$installAbs = [System.IO.Path]::GetFullPath($installResolved)
$ztExe = Join-Path $installAbs "zt.exe"
$runtimeSource = Join-Path $installAbs "runtime\\c\\zenith_rt.c"
$stdlibDir = Join-Path $installAbs "stdlib"
$projectAbs = [System.IO.Path]::GetFullPath((Join-Path $repoRoot $ProjectPath))
$logAbs = [System.IO.Path]::GetFullPath((Join-Path $repoRoot $LogPath))

Assert-Exists $installAbs "Install root"
Assert-Exists $ztExe "Installed compiler binary"
Assert-Exists $runtimeSource "Installed runtime source"
Assert-Exists $stdlibDir "Installed stdlib folder"
Assert-Exists $projectAbs "Validation project"

New-Item -ItemType Directory -Force -Path (Split-Path -Parent $logAbs) | Out-Null

$timestamp = Get-Date -Format "yyyy-MM-ddTHH:mm:ssK"
"Zenith installer validation - $timestamp" | Set-Content -Path $logAbs -Encoding utf8
"Install root: $installAbs" | Add-Content -Path $logAbs
"Project: $projectAbs" | Add-Content -Path $logAbs

Run-Step -Name "zt check" -Exe $ztExe -Args @("check", $projectAbs) -LogFile $logAbs
Run-Step -Name "zt build" -Exe $ztExe -Args @("build", $projectAbs) -LogFile $logAbs
Run-Step -Name "zt run" -Exe $ztExe -Args @("run", $projectAbs) -LogFile $logAbs

Add-Content -Path $logAbs -Value ""
Add-Content -Path $logAbs -Value "Validation result: PASS"
Write-Host "Installer validation PASS. Log:"
Write-Host "  $logAbs"
