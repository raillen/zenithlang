# R2.M2 - Frontend Fuzzing Infrastructure

Infra de fuzzing leve para lexer e parser do Zenith. Cobre os itens de R2.M2
em `docs/internal/planning/checklist-v2.md`:

- Harness de fuzz para lexer (`fuzz_lexer.py`)
- Harness de fuzz para parser (`fuzz_parser.py`)
- Corpus inicial de casos validos e invalidos (`corpus/valid`, `corpus/invalid`)
- Mutadores para tokens e nesting (`mutators.py`)
- Detector de crash e timeout (`harness.py`)
- Minimizador de input (`minimizer.py`)
- Persistencia de seeds que quebram (`seeds/crashes/`)
- Reexecucao de seeds em toda rodada relevante (`replay.py`)

## Uso rapido

```
python tests/fuzz/fuzz_lexer.py  --iters 200 --seed 20260421
python tests/fuzz/fuzz_parser.py --iters 200 --seed 20260421
python tests/fuzz/replay.py
```

Exit code 0 = sem crashes novos. Exit code 1 = crash detectado
(seeds persistidas em `tests/fuzz/seeds/crashes/`).

## Definicao de "crash"

Classificamos como crash qualquer um destes comportamentos em `zt.exe check`:

- saida com `signal` / `abort` / `access violation` / `segfault`
- codigo de retorno < 0 ou > 2 (compilador usa 0=ok, 1=erro de usuario, 2=erro interno)
- timeout alem do limite configurado
- mensagem contendo `internal compiler error`, `assertion failed`, `ICE`

Erros de diagnostico legitimos do compilador (rc=1 com span/codigo `ZT_DIAG_*`)
nao sao crashes: sao aceitaveis no corpus invalido.

## Integracao com suites

- `smoke`: nao roda fuzz (muito rapido, baseline).
- `pr_gate`: replay de seeds persistidas (rapido, deterministico).
- `nightly`: replay + campanha curta com seed fixa.
- `stress`: campanha longa com seed fixa.

Ver `tests/suites/suite_definitions.py` e `run_suite.py`.

## Estrutura

```
tests/fuzz/
  README.md            este arquivo
  harness.py           runner comum (crash detector, timeout, projeto temp)
  mutators.py          mutadores de tokens/nesting
  minimizer.py         delta-debugging por linhas/tokens
  fuzz_lexer.py        harness lexer (caracteres e tokens isolados)
  fuzz_parser.py       harness parser (estruturas aninhadas)
  replay.py            reexecuta seeds persistidas
  corpus/
    valid/             snippets sintaticamente validos
    invalid/           snippets invalidos mas conhecidos/seguros
  seeds/
    crashes/           seeds que causaram crash (persistidos)
```
