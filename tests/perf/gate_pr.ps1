$ErrorActionPreference = 'Stop'
python tests\perf\run_perf.py --suite quick --release-gate
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
