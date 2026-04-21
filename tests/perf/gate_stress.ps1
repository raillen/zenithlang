# R2.M1 - Stress gate: long-running stability + fuzz replay (~60min+)
# Usage: pwsh tests/perf/gate_stress.ps1
$ErrorActionPreference = 'Stop'
python run_suite.py stress
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
