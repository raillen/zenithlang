# Zenith Test Infrastructure 2.0 (R2.M1)

## Suites

| Suite     | Comando                          | Tempo limite | Quando usar                          |
|-----------|----------------------------------|--------------|--------------------------------------|
| `smoke`   | `python run_suite.py smoke`      | 60s          | Sanidade rapida apos qualquer mudanca |
| `pr_gate` | `python run_suite.py pr_gate`    | 300s         | Obrigatorio antes de abrir PR        |
| `nightly` | `python run_suite.py nightly`    | 1200s        | CI noturno, cobertura completa       |
| `stress`  | `python run_suite.py stress`     | 7200s        | Estabilidade longa + replay de fuzz  |

Scripts PowerShell equivalentes:

| Suite     | Script                              |
|-----------|-------------------------------------|
| `smoke`   | `pwsh tests/perf/gate_smoke.ps1`    |
| `pr_gate` | `pwsh tests/perf/gate_pr.ps1`       |
| `nightly` | `pwsh tests/perf/gate_nightly.ps1`  |
| `stress`  | `pwsh tests/perf/gate_stress.ps1`   |

## Opcoes do runner

```
python run_suite.py <suite> [--no-perf] [--report-dir DIR] [--artifacts-dir DIR] [--seed N]
```

- `--no-perf`        Pula benchmarks de performance (util para debug rapido)
- `--report-dir`     Diretorio de saida dos relatorios JSON (padrao: `reports/suites/`)
- `--artifacts-dir`  Diretorio de artifacts de falha (padrao: `reports/suites/artifacts/`)
- `--seed`           Sobrescreve seed global para testes stress/fuzz

## Relatorios

Cada execucao gera dois arquivos em `reports/suites/`:

- `<suite>__<timestamp>Z.json`  — historico imutavel
- `<suite>__latest.json`        — link para a ultima execucao

Estrutura do relatorio:

```json
{
  "suite": "pr_gate",
  "timestamp": "2026-04-21T10:00:00Z",
  "elapsed_ms": 45000,
  "total": 80,
  "pass": 78,
  "fail": 2,
  "by_layer": {
    "frontend":  { "pass": 30, "fail": 1 },
    "backend":   { "pass": 20, "fail": 1 },
    "runtime":   { "pass": 28, "fail": 0 }
  },
  "by_fail_class": {
    "wrong_exit":   1,
    "wrong_output": 1
  },
  "entries": [ ... ]
}
```

## Classes de falha

| Classe          | Significado                                      |
|-----------------|--------------------------------------------------|
| `timeout`       | Teste excedeu o tempo limite                     |
| `crash`         | Processo terminou com sinal negativo             |
| `wrong_exit`    | Codigo de saida inesperado                       |
| `wrong_output`  | Saida nao contem string esperada                 |
| `infra`         | Erro de infraestrutura (ex: exe nao encontrado)  |

## Artifacts de falha

Toda falha salva automaticamente a saida completa em:

```
reports/suites/artifacts/<test_id>__<stage>__<timestamp>Z.txt
```

## Seeds estaveis

Seeds fixos para reproducibilidade de testes stress/fuzz (definidos em `suite_definitions.py`):

```python
STABLE_SEEDS = {
    "fuzz_lexer":  20260421,
    "fuzz_parser": 20260421,
    "stress_rt":   20260421,
}
```

Use `--seed N` para sobrescrever em execucoes pontuais.

## Camadas reportadas

| Layer       | Testes incluidos                                      |
|-------------|-------------------------------------------------------|
| `frontend`  | lexer, parser, AST, check-pass, check-fail            |
| `semantic`  | binder, types, constraints                            |
| `hir_zir`   | HIR lowering, ZIR lowering, verifier                  |
| `backend`   | emitter, legalization, build-fail                     |
| `runtime`   | runtime C, run-pass, run-fail                         |
| `stdlib`    | stdlib modules                                        |
| `tooling`   | formatter golden, driver                              |
| `perf`      | benchmarks de performance                             |

## Fluxo de PR (R2.M1)

1. Desenvolva a mudanca.
2. Execute `python run_suite.py smoke` para sanidade rapida.
3. Execute `python run_suite.py pr_gate` antes de abrir o PR.
4. Anexe o relatorio `pr_gate__latest.json` ou o output no PR se houver falha.
5. CI executa `pwsh tests/perf/gate_pr.ps1` automaticamente.

## Arquivos relacionados

- `run_suite.py`                      — runner principal
- `tests/suites/suite_definitions.py` — definicoes de suites, seeds, timeouts
- `tests/perf/gate_smoke.ps1`         — gate smoke
- `tests/perf/gate_pr.ps1`            — gate PR
- `tests/perf/gate_nightly.ps1`       — gate nightly
- `tests/perf/gate_stress.ps1`        — gate stress
- `reports/suites/`                   — relatorios JSON
- `reports/suites/artifacts/`         — artifacts de falha
