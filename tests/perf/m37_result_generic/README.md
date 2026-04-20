# M37 Result Generic Gate

Projeto curto para gate do M37 Fase 2.

Objetivo:

- forcar monomorfizacao de `result<T,E>` fora da matriz hardcoded
- misturar `E` primitivo e `E` managed
- produzir um executavel simples para medir build, binario e memoria

O gate oficial roda por `tests/perf/run_perf.py`.