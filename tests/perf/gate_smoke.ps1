# R2.M1 - Smoke gate: fast sanity check (~30s)
# Usage: pwsh tests/perf/gate_smoke.ps1
$ErrorActionPreference = 'Stop'
python run_suite.py smoke
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
