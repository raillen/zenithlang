# Hardening Tests

Additional gates for critical bug prevention:

- `test_determinism.py`: deterministic `emit-c` and diagnostics.
- `test_roundtrip_emit_c.py`: round-trip `emit-c -> gcc -> run` consistency.
- `test_differential_validate_between.py`: Zenith vs standalone C reference.
- `test_runtime_sanitizers.py`: ASAN/UBSAN runtime checks (auto-skip if unsupported).
