# R2.M1 - Nightly performance gate
# Usage: pwsh tests/perf/gate_nightly.ps1
$ErrorActionPreference = 'Stop'
python tests\perf\run_perf.py --suite nightly --release-gate
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
