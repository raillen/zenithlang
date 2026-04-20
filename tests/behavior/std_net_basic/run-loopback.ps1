param(
    [string]$Executable = "build/std-net-basic.exe",
    [int]$Port = 41234
)

$ErrorActionPreference = "Stop"

$serverScript = Join-Path $PSScriptRoot "loopback-server.ps1"
$resolvedExecutable = if ([System.IO.Path]::IsPathRooted($Executable)) { $Executable } else { Join-Path $PSScriptRoot $Executable }
$server = Start-Process -FilePath "powershell" -ArgumentList @(
    "-NoProfile",
    "-ExecutionPolicy", "Bypass",
    "-File", $serverScript,
    "-Port", $Port
) -PassThru -WindowStyle Hidden

try {
    Start-Sleep -Milliseconds 250
    & $resolvedExecutable
    $exitCode = $LASTEXITCODE
    Wait-Process -Id $server.Id -Timeout 5 -ErrorAction SilentlyContinue
    exit $exitCode
} finally {
    if (Get-Process -Id $server.Id -ErrorAction SilentlyContinue) {
        Stop-Process -Id $server.Id -Force
    }
}
