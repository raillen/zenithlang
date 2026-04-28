# Performance Suite (M36)

Suite unica de performance E2E para Zenith Next.

## O que cobre

- micro frontend: lexer/parser/binder/typechecker (via `zt check`)
- micro lowering/backend: HIR/ZIR/emissao C (via `zt emit-c`)
- micro runtime: `text`, `bytes`, `list`, primitive numeric lists, `map`, `grid2d`, `pqueue`, `circbuf`, `btreemap`, `btreeset`, `grid3d`
- micro stdlib: `json`, `format`, `math`, `random`, `validate`
- macro: `zt check`, `zt build`, `zt run`, `zt test`
- cenarios `small`, `medium`, `large`, com build `cold` e `warm`

## Runner

```powershell
python tests\perf\run_perf.py --suite quick
python tests\perf\run_perf.py --suite nightly
```

Rodar alvo especifico:

```powershell
python tests\perf\run_perf.py macro_large
python tests\perf\run_perf.py micro_primitive_numeric_lists
python tests\perf\run_perf.py m37_result_generic
python tests\perf\run_perf.py --benchmark macro_small_build_cold
```

Listar suites/scenarios/benchmarks:

```powershell
python tests\perf\run_perf.py --list
```

## Baseline e diff

Atualizar baseline da plataforma atual:

```powershell
python tests\perf\run_perf.py --suite quick --update-baseline
```

Comparar com report da branch base:

```powershell
python tests\perf\run_perf.py --suite quick --base-report reports\perf\summary-quick.json
```

## Gate de release

Bloqueio padrao: qualquer `fail` retorna codigo != 0.

Override documentado:

```powershell
python tests\perf\run_perf.py --suite nightly --release-gate --override-file docs\perf-override.md
```

## Saida

- `reports/perf/summary-quick.json`
- `reports/perf/summary-quick.md`
- `reports/perf/summary-nightly.json`
- `reports/perf/summary-nightly.md`
- `reports/perf/<suite>-<benchmark>.json`
- `reports/perf/<suite>-<benchmark>.md`

## Metodologia

- quick: warmup=1, medicao=3, retry de instabilidade=1
- nightly: warmup=2, medicao=7, retry de instabilidade=2
- estatisticas: mediana, p95, desvio padrao, throughput
- memoria: pico de working set
- alocacoes: proxy por `PrivateMemorySize` (Windows)
- outliers: IQR (1.5x), com rerun automatico quando instavel

## Perfis de execucao

- `quick`: foco em feedback rapido para PR (micro + macro small + `macro_medium_check`).
- `nightly`: inclui cenarios pesados (`macro_medium_build_*`, `macro_large_*`, `m37_result_generic`) para regressao longa.

## Comandos equivalentes no driver

```powershell
zt perf quick
zt perf nightly
```

## Gates prontos

```powershell
powershell -ExecutionPolicy Bypass -File tests\perf\gate_pr.ps1
powershell -ExecutionPolicy Bypass -File tests\perf\gate_nightly.ps1
```

## Estado atual

- `quick`: validado e verde no ambiente local.
- `nightly`: definido e documentado; executar em janela longa para fechar gate final de release.
