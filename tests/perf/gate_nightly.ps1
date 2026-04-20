$ErrorActionPreference = 'Stop'
python tests\perf\run_perf.py --suite nightly --release-gate
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
