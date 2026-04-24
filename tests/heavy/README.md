# Heavy Tests

Estado atual do bloco `tests/heavy`:

- `fuzz/semantic/`: corpus pesado de fixtures e campanha fuzz reproducivel;
- `run_heavy_tests.py`: runner alinhado ao repositorio atual;
- `reports/`: saida JSON + resumo markdown do runner pesado.

O que este bloco faz hoje:

1. roda replay curado dos fixtures pesados mais estaveis;
2. roda campanha semantica reproducivel com seed fixa;
3. gera relatorios simples para acompanhamento.

Suites disponiveis:

- `all`
- `curated`
- `fuzz`

Comandos:

```bash
python tests/heavy/run_heavy_tests.py
python tests/heavy/run_heavy_tests.py --suite curated
python tests/heavy/run_heavy_tests.py --suite fuzz
```

Observacoes:

- os fixtures em `tests/heavy/fuzz/semantic/*` foram modernizados para o formato atual de projeto, com `zenith.ztproj` em formato INI e fonte em `src/<namespace>.zt`;
- o replay curado oficial cobre tanto casos que devem passar quanto regressoes que agora devem falhar, como receiver `const` chamando metodo `mut`;
- o gate oficial continua em `run_suite.py`;
- `tests/heavy/run_heavy_tests.py` serve como runner local e relatorio concentrado para o bloco pesado.

