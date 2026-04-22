# Zenith Checklist 4.0

## Objetivo

Executar e validar as entregas do roadmap 4.0 de forma objetiva e auditavel.

## Escopo

- Itens de execucao por milestone (`R4.M0` ate `R4.M9`).
- Gates obrigatorios de qualidade e performance.
- Evidencias minimas para fechamento por milestone.

## Dependencias

- Upstream:
  - `docs/planning/roadmap-v4.md`
  - `docs/planning/roadmap-v2.md`
  - `docs/planning/checklist-v2.md`
- Downstream:
  - `docs/planning/cascade-v4.md` (a criar)
  - `docs/reports/release/*`
  - `docs/reports/compatibility/*`
  - `packages/borealis/*`
  - `packages/borealis/decisions/*`
  - `docs/planning/borealis-roadmap-v1.md`
  - `docs/planning/borealis-checklist-v1.md`
- Codigo/Testes relacionados:
  - `python build.py`
  - `python run_all_tests.py`
  - `tests/perf/gate_pr.ps1`
  - `tests/perf/gate_nightly.ps1`

## Como usar

1. Marque item so com evidencia valida.
2. Rode os gates obrigatorios antes de fechar milestone.
3. Registre desvio com risco residual no report da milestone.

Checklist operacional derivado de `docs/planning/roadmap-v4.md`.

Status inicial: nao iniciado  
Data de criacao: 2026-04-21

## Navegacao rapida para LLMs

1. `docs/planning/roadmap-v4.md`
2. `docs/planning/checklist-v4.md`
3. `docs/planning/roadmap-v2.md`
4. `docs/planning/checklist-v2.md`
5. `language/spec/README.md`
6. `compiler/CODE_MAP.md`
7. `packages/borealis/architecture-summary.md`
8. codigo
9. testes

## Gates obrigatorios por milestone

- [ ] `python build.py` verde
- [ ] `python run_all_tests.py` verde
- [ ] `tests/perf/gate_pr.ps1` verde
- [ ] sem crash novo em fuzz/corpus
- [ ] evidencia anexada na milestone

Para release alpha.2:

- [ ] `tests/perf/gate_nightly.ps1` verde
- [ ] budgets atualizados e justificados
- [ ] zero erro novo de compatibilidade
- [ ] artefato + checksum + install limpo validados

## R4.M0 - Alpha.2 stabilization base

- [ ] Reduzir warnings de build para nivel controlado
- [ ] Fechar pendencia de budgets do `R2.M5`
- [ ] Consolidar baseline de qualidade/perf por plataforma
- [ ] Automatizar script de release com install limpo

Criterio de aceite:

- [ ] Base de estabilidade alpha.2 validada e reportada

Evidencia R4.M0:

- [ ] `docs/reports/release/R4.M0-stability-report.md`
- [ ] `docs/governance/baselines/perf-baseline.json` atualizado
- [ ] `docs/governance/baselines/quality-baseline.json` atualizado

## R4.M1 - Compass LSP v1 + VSCode beta

- [ ] Diagnostics em tempo real
- [ ] Hover de simbolos
- [ ] Go-to-definition
- [ ] Format-on-save com `zt fmt`
- [ ] Comando rapido `check/build/run` no editor

Criterio de aceite:

- [ ] Fluxo de edicao diario validado em projeto real

Evidencia R4.M1:

- [ ] `docs/reports/release/R4.M1-lsp-vscode-report.md`
- [ ] `docs/guides/README.md` com onboarding do editor

## R4.M2 - ZPM MVP

- [ ] `zpm init`
- [ ] `zpm add <pkg>`
- [ ] `zpm install`
- [ ] lockfile reproduzivel
- [ ] `zpm publish` (fluxo minimo)

Criterio de aceite:

- [ ] Pacote local instalado e publicado em fluxo reproduzivel

Evidencia R4.M2:

- [ ] `docs/reports/release/R4.M2-zpm-report.md`
- [ ] guia de uso inicial do ZPM

## R4.M3 - Zenith Web Lite (sinatra-like)

- [ ] Rotas `get/post/put/delete`
- [ ] Path params
- [ ] Query params
- [ ] Middleware simples
- [ ] Helpers de JSON request/response
- [ ] Tratamento padrao de erro

Criterio de aceite:

- [ ] "hello web api" em ate 15 min com docs oficiais

Evidencia R4.M3:

- [ ] `docs/reports/release/R4.M3-web-lite-report.md`
- [ ] exemplo oficial em `examples/` rodando no CI

## R4.M4 - FFI 1.0 + Binding Pack C (fase 1)

- [ ] Contrato FFI estavel documentado
- [ ] Binding oficial `sqlite3`
- [ ] Binding oficial `libcurl`
- [ ] Mapeamento de erros para `core.Error`
- [ ] Exemplos E2E com testes automatizados

Criterio de aceite:

- [ ] Bindings oficiais com comportamento validado no gate

Evidencia R4.M4:

- [ ] `docs/reports/release/R4.M4-ffi-bindings-report.md`
- [ ] exemplos e testes E2E para `sqlite3` e `libcurl`

## R4.M5 - Produto 1: VSCode Marketplace

- [ ] Publicar extensao no Marketplace
- [ ] Entregar templates de projeto
- [ ] Entregar snippets basicos
- [ ] Publicar guia de onboarding de 15 minutos

Criterio de aceite:

- [ ] Instalacao publica funcionando em conta externa

Evidencia R4.M5:

- [ ] `docs/reports/release/R4.M5-vscode-marketplace-report.md`
- [ ] link publico do Marketplace registrado em docs

## R4.M6 - Produto 2: Zenith CI Gate

- [ ] Action/app para `check + test + perf`
- [ ] Relatorio de falha com links de artefato
- [ ] Regra de bloqueio por regressao critica

Criterio de aceite:

- [ ] Repositorio piloto usando gate em PR real

Evidencia R4.M6:

- [ ] `docs/reports/release/R4.M6-ci-gate-report.md`
- [ ] workflow de referencia publicado

## R4.M7 - Produto 3: Playground + trilha de aprendizado

- [ ] Playground web com compilacao/execucao de exemplos curtos
- [ ] Colecao de exemplos oficiais curada
- [ ] Trilha curta (iniciante -> app real)

Criterio de aceite:

- [ ] Usuario novo consegue executar exemplo sem setup local

Evidencia R4.M7:

- [ ] `docs/reports/release/R4.M7-playground-learning-report.md`
- [ ] links publicos de playground e trilha

## R4.M8 - UI/Grafica: decisao com POC

- [ ] Implementar 1 POC pequeno de UI
- [ ] Comparar custo x impacto
- [ ] Registrar decisao formal (continuar ou adiar)

Criterio de aceite:

- [ ] Decisao tomada com dados de uso e manutencao

Evidencia R4.M8:

- [ ] `docs/reports/release/R4.M8-ui-poc-decision.md`

## R4.M9 - Alpha.2 release

- [ ] Publicar `0.3.0-alpha.2`
- [ ] Publicar artefato + checksums
- [ ] Validar install limpo + hello world
- [ ] Publicar notas de release
- [ ] Atualizar matriz de compatibilidade
- [ ] Publicar relatorio final de risco residual

Criterio de aceite:

- [ ] Release `0.3.0-alpha.2` publicada e verificavel

Evidencia R4.M9:

- [ ] `docs/reports/release/R4.M9-alpha2-release-report.md`
- [ ] `docs/reports/compatibility/R4.M9-alpha2-compatibility.md`

## KPI minimo do ciclo R4

- [ ] Tempo para "hello web api" <= 15 min
- [ ] Plugin VSCode com instalacao e uso validados
- [ ] 2 bindings C oficiais em producao (`sqlite3`, `libcurl`)
- [ ] Alpha.2 sem P0 aberto

## Regras continuas

- [ ] Todo bug novo gera teste de regressao
- [ ] Toda feature nova entra com teste positivo e negativo
- [ ] Toda mudanca de comportamento atualiza docs no mesmo PR
- [ ] Toda regressao critica de performance bloqueia merge
- [ ] Toda divergencia spec x codigo recebe classificacao P0/P1/P2

## Evidencia minima para marcar item como concluido

- [ ] Comando executado + resultado
- [ ] Arquivo de teste novo ou alterado
- [ ] Commit/PR de fechamento
- [ ] Risco residual (se houver)
