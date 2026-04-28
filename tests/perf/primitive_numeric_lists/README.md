# Primitive Numeric Lists Benchmark

Measures hot-path reads, writes, length checks, and slices for primitive numeric lists.

This benchmark is tied to roadmap item C.14. It keeps a dedicated target in the normal perf runner so future changes can compare against a stable baseline.

Run it with:

```powershell
python tests\perf\run_perf.py micro_primitive_numeric_lists
```

