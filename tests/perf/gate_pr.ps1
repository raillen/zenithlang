# R2.M1 - PR gate: correctness + quick perf
# Usage: pwsh tests/perf/gate_pr.ps1
$ErrorActionPreference = 'Stop'
python run_suite.py pr_gate
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
