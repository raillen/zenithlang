# Operational Reports

Este diretorio guarda saidas operacionais.

Ele nao e a area principal de documentacao.
Use `docs/reports/` para relatorios curados e leitura futura.

## O que fica aqui

- `pending-language-issues-current.md`
  Fonte atual para pendencias corretivas residuais.
- `deep-analysis-report.md`
  Relatorio operacional historico do ciclo corretivo.
- `coverage/`
  Snapshot de cobertura.
- `hardening/`
  Evidencias de hardening.
- `perf/`
  Saidas locais de benchmark. Apenas `.gitkeep` deve ficar versionado por padrao.

## O que nao deve ficar aqui

- builds;
- caches;
- screenshots temporarios;
- logs locais;
- pacotes instalados;
- resultados repetiveis de suites locais.

Se um resultado precisa virar referencia do projeto, mova ou resuma em `docs/reports/`.
