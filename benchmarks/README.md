# Benchmarks

Este diretorio e o indice da Phase 5C.

Ele nao substitui `tests/perf/`. O runner real continua em:

```powershell
python tests\perf\run_perf.py --suite quick
python tests\perf\run_perf.py --suite nightly
python tests\perf\run_perf.py --list
```

## Regra de uso

Use estes benchmarks para detectar regressao.

Nao use os numeros como material de marketing. Resultado de performance depende de maquina, sistema, carga do ambiente, compilador C e perfil de build.

## Mapa rapido

- K.01: infraestrutura em `tests/perf/run_perf.py`, gates PowerShell, baselines e `reports/perf/`.
- K.02: microproblemas em `tests/perf/micro_algorithm_core`.
- K.03: tempo de compilacao em `macro_*_build_*` e `macro_*_check`.
- K.04: runtime Zenith/C em `tests/perf/std_validate_vs_c`.
- K.05: comparacao por linguagem documentada em `benchmarks/cross-language/`.
- K.06: uso interno para regressao, registrado neste diretorio e no manifest.

## Saidas esperadas

O runner grava:

- `reports/perf/summary-quick.json`
- `reports/perf/summary-quick.md`
- `reports/perf/summary-nightly.json`
- `reports/perf/summary-nightly.md`
- `reports/perf/<suite>-<benchmark>.json`
- `reports/perf/<suite>-<benchmark>.md`

## Comando curto para validar a Phase 5C

```powershell
python tests\driver\test_phase5c_benchmarks.py
python tests\perf\run_perf.py --suite quick --benchmark micro_algorithm_core
```

