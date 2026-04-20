# Performance Runner

Runner unico para benchmarks e gates curtos em `tests/perf`.

Estado atual:

- `m37_result_generic`: gate curto para `result<T,E>` generico no backend C
- `std_validate_vs_c`: benchmark manual, ainda fora do runner

## Uso

Rodar um cenario:

```powershell
python tests\perf\run_perf.py m37_result_generic
```

Rodar todos os cenarios ligados ao runner:

```powershell
python tests\perf\run_perf.py
```

## Reproducao Local

Pre-requisitos:

- `python` no `PATH`
- toolchain C funcional para `python build.py`
- executar a partir da raiz do repo

Fluxo curto para devs e contribuidores:

```powershell
python build.py
python tests\perf\run_perf.py m37_result_generic
```

Para inspecionar os numeros gerados:

```powershell
Get-Content reports\perf\m37_result_generic.md
```

## Saida

Cada execucao gera:

- `reports/perf/<cenario>.json`
- `reports/perf/<cenario>.md`

Contrato minimo de output:

- amostras brutas de build e run
- mediana de tempo de build
- tamanho do binario final
- pico de memoria por execucao
- spread de memoria entre execucoes
- status por metrica: `pass`, `warn` ou `fail`

## Regra atual

O runner retorna codigo diferente de zero quando qualquer metrica bate em `fail`.