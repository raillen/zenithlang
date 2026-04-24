# Hardening Tests

Additional gates for critical bug prevention:

- `test_determinism.py`: deterministic `emit-c` and diagnostics.
- `test_roundtrip_emit_c.py`: round-trip `emit-c -> gcc -> run` consistency.
- `test_differential_validate_between.py`: Zenith vs standalone C reference.
- `test_heavy_semantic_curated.py`: curated replay over modernized `tests/heavy/fuzz/semantic` fixtures.
- `test_concurrent_compilation.py`: concurrent native builds with cold/warm runtime cache.
- `test_coverage_snapshot.py`: focused `gcov` snapshot for parser/checker/lowering/emitter/runtime.
- `test_emitter_memory_profile.py`: synthetic `emit-c` memory profile comparing buffered vs spill/stream output.
- `test_runtime_sanitizers.py`: ASAN/UBSAN runtime checks (auto-skip if unsupported).
