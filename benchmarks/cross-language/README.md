# Cross-Language Benchmarks

Este diretorio registra o escopo comparativo da Phase 5C.

Use estes casos apenas para regressao interna. Nao publique numeros como comparacao de marketing.

## Linguagens

- Zenith: medido pelo runner `tests/perf/run_perf.py`.
- Python: harness inicial em `tests/perf/compare_zenith_python.py`.
- Go: especificado em `problems.json`; execucao depende de `go` instalado.
- Kotlin: especificado em `problems.json`; execucao depende de `kotlinc` instalado.

## Regra

Cada linguagem deve resolver o mesmo problema, com entrada fixa e saida verificavel.

Se uma toolchain nao estiver instalada, o benchmark deve ser marcado como `skipped`, nao como falha do compilador Zenith.

