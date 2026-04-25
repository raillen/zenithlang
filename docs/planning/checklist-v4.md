# Zenith Checklist 4.0

> Checklist operacional derivado de `docs/planning/roadmap-v4.md`.
> Status: nao iniciado | Criado: 2026-04-21

---

## 1. Contexto

### Objetivo

Executar e validar as entregas do roadmap 4.0 de forma objetiva e auditavel.

### Escopo

- Itens de execucao por milestone (`R4.M0` ate `R4.M9`).
- Gates obrigatorios de qualidade e performance.
- Evidencias minimas para fechamento por milestone.

### Dependencias

| Direcao | Artefatos |
|---------|-----------|
| Upstream | `docs/planning/roadmap-v4.md`, `roadmap-v2.md`, `checklist-v2.md` |
| Downstream | `docs/planning/cascade-v4.md` (a criar), `docs/reports/release/*`, `docs/reports/compatibility/*`, `packages/borealis/*` |
| Borealis | `docs/planning/borealis-roadmap-v1.md`, `borealis-checklist-v1.md` |
| Scripts | `python build.py`, `python run_all_tests.py`, `tests/perf/gate_pr.ps1`, `tests/perf/gate_nightly.ps1` |

### Navegacao rapida para LLMs

1. `docs/planning/roadmap-v4.md`
2. `docs/planning/checklist-v4.md` (este arquivo)
3. `docs/planning/roadmap-v2.md`
4. `docs/planning/checklist-v2.md`
5. `language/spec/README.md`
6. `compiler/CODE_MAP.md`
7. `packages/borealis/architecture-summary.md`
8. codigo
9. testes

---

## 2. Regras operacionais (valem para todo o ciclo)

### Como usar

1. Marque item **so** com evidencia valida.
2. Rode os gates obrigatorios antes de fechar milestone.
3. Registre desvio com risco residual no report da milestone.

### Regras continuas

- [ ] Todo bug novo gera teste de regressao
- [ ] Toda feature nova entra com teste positivo e negativo
- [ ] Toda mudanca de comportamento atualiza docs no mesmo PR
- [ ] Toda regressao critica de performance bloqueia merge
- [ ] Toda divergencia spec x codigo recebe classificacao P0/P1/P2

### Evidencia minima para marcar item como concluido

- [ ] Comando executado + resultado
- [ ] Arquivo de teste novo ou alterado
- [ ] Commit/PR de fechamento
- [ ] Risco residual (se houver)

---

## 3. Gates obrigatorios

### Gate por milestone (todas)

- [ ] `python build.py` verde
- [ ] `python run_all_tests.py` verde
- [ ] `tests/perf/gate_pr.ps1` verde
- [ ] sem crash novo em fuzz/corpus
- [ ] evidencia anexada na milestone

### Gate alpha.2 release (R4.M9)

- [ ] `tests/perf/gate_nightly.ps1` verde
- [ ] budgets atualizados e justificados
- [ ] zero erro novo de compatibilidade
- [ ] artefato + checksum + install limpo validados

---

## 4. KPI minimo do ciclo R4

- [ ] Tempo para "hello web api" <= 15 min
- [ ] Plugin VSCode com instalacao e uso validados
- [ ] 2 bindings C oficiais em producao (`sqlite3`, `libcurl`)
- [ ] Alpha.2 sem P0 aberto

---

## 5. Mapa de dependencias entre milestones

```
R4.M0 (base)
  ├── R4.M1 (LSP) ──────────┐
  ├── R4.M2 (ZPM) ──────────┤
  ├── R4.M3 (Web Lite) ─────┤
  └── R4.M4 (FFI) ──────────┤
                            ├── R4.M5 (VSCode Marketplace)
                            ├── R4.M6 (CI Gate)
                            └── R4.M7 (Playground)
                                      │
                            R4.M8 (UI POC) ──┘
                                      │
                            R4.M9 (Alpha.2 release)
```

**Ordem de execucao recomendada:**
1. **Fase 1 - Fundacao:** R4.M0 (estabilizacao)
2. **Fase 2 - Ferramentas:** R4.M1 (LSP), R4.M2 (ZPM) — podem rodar em paralelo
3. **Fase 3 - Capacidades:** R4.M3 (Web Lite), R4.M4 (FFI) — podem rodar em paralelo
4. **Fase 4 - Produtos:** R4.M5, R4.M6, R4.M7 — dependem de Fase 2+3
5. **Fase 5 - Decisao:** R4.M8 (UI POC) — independente, pode rodar em paralelo com Fase 4
6. **Fase 6 - Release:** R4.M9 — depende de todas as anteriores

---

## 6. Milestones por fase

### FASE 1 — Fundacao

#### R4.M0 - Alpha.2 stabilization base

**Entregas:**
- [x] Reduzir warnings de build para nivel controlado (zero warnings)
- [x] Fechar pendencia de budgets do `R2.M5`
- [x] Consolidar baseline de qualidade/perf por plataforma
- [x] Automatizar script de release com install limpo
- [x] Fechar R3.M5 (Callables tipados e delegates) como prerequisito

**Criterio de aceite:**
- [x] Base de estabilidade alpha.2 validada e reportada

**Evidencias:**
- [ ] `docs/reports/release/R4.M0-stability-report.md`
- [ ] `docs/governance/baselines/perf-baseline.json` atualizado
- [ ] `docs/governance/baselines/quality-baseline.json` atualizado

**Baseline atual (2026-04-24):**
- `python build.py`: verde (zero warnings)
- `python run_all_tests.py`: 177/180 pass (1 pre-existente `borealis_backend_fallback_stub`, 2 skip)
- R3.M5 completo: 6 fixtures de callable validadas (1 run-pass, 5 error-check)

---

### FASE 2 — Ferramentas de desenvolvedor

#### R4.M1 - Compass LSP v1 + VSCode beta

**Entregas:**
- [x] Diagnostics em tempo real
- [x] Hover de simbolos
- [x] Go-to-definition
- [x] Format-on-save com `zt fmt`
- [x] Autocomplete contextual global, local e de membro (`list.get`, `map.get`, `core.Error`, campos de struct)
- [x] Autocomplete de `import ...` com `std.*` e módulos `.zt` indexados do projeto
- [x] Indice de workspace para simbolos top-level cross-file
- [x] Indice ciente de `namespace`, `import ... as ...` e `public`
- [x] Comando rapido `check/build/run` no editor

**Criterio de aceite:**
- [x] Fluxo de edicao diario validado em projeto real

**Nota de status (2026-04-25):**
- Implementacao beta local concluida.
- Validado por smoke LSP automatizado e syntax check da extensao.
- Ajustado apos validacao manual do usuario: diagnostics estavam OK; autocomplete de membro precisava ser contextual.
- Endurecido com indice in-memory do workspace para autocomplete e definition cross-file.
- Endurecido com regras de import/namespace/public para evitar sugestoes cross-file fora de contexto.

**Evidencias:**
- [x] `docs/reports/release/R4.M1-lsp-vscode-report.md`
- [x] `docs/guides/README.md` com onboarding do editor

#### R4.M2 - ZPM MVP

**Entregas:**
- [x] `zpm init`
- [x] `zpm add <pkg>`
- [x] `zpm install`
- [x] lockfile reproduzivel
- [x] `zpm publish` (fluxo minimo)

**Criterio de aceite:**
- [x] Pacote local instalado e publicado em fluxo reproduzivel

**Evidencias:**
- [x] `docs/reports/release/R4.M2-zpm-report.md`
- [x] guia de uso inicial do ZPM

---

### FASE 3 — Capacidades da linguagem

#### R4.M3 - Zenith Web Lite (sinatra-like)

**Entregas:**
- [ ] Rotas `get/post/put/delete`
- [ ] Path params
- [ ] Query params
- [ ] Middleware simples
- [ ] Helpers de JSON request/response
- [ ] Tratamento padrao de erro

**Criterio de aceite:**
- [ ] "hello web api" em ate 15 min com docs oficiais

**Evidencias:**
- [ ] `docs/reports/release/R4.M3-web-lite-report.md`
- [ ] exemplo oficial em `examples/` rodando no CI

#### R4.M4 - FFI 1.0 + Binding Pack C (fase 1)

**Entregas:**
- [ ] Contrato FFI estavel documentado
- [ ] Binding oficial `sqlite3`
- [ ] Binding oficial `libcurl`
- [ ] Mapeamento de erros para `core.Error`
- [ ] Exemplos E2E com testes automatizados

**Criterio de aceite:**
- [ ] Bindings oficiais com comportamento validado no gate

**Evidencias:**
- [ ] `docs/reports/release/R4.M4-ffi-bindings-report.md`
- [ ] exemplos e testes E2E para `sqlite3` e `libcurl`

---

### FASE 4 — Produtos

#### R4.M5 - Produto 1: VSCode Marketplace

**Entregas:**
- [ ] Publicar extensao no Marketplace
- [ ] Entregar templates de projeto
- [ ] Entregar snippets basicos
- [ ] Adicionar modo rapido de arquivo unico no CLI (`zt check/run arquivo.zt`) para onboarding e exemplos curtos, mantendo `zenith.ztproj` como fluxo canonico de projeto
- [ ] Publicar guia de onboarding de 15 minutos

**Criterio de aceite:**
- [ ] Instalacao publica funcionando em conta externa

**Evidencias:**
- [ ] `docs/reports/release/R4.M5-vscode-marketplace-report.md`
- [ ] link publico do Marketplace registrado em docs

#### R4.M6 - Produto 2: Zenith CI Gate

**Entregas:**
- [ ] Action/app para `check + test + perf`
- [ ] Relatorio de falha com links de artefato
- [ ] Regra de bloqueio por regressao critica

**Criterio de aceite:**
- [ ] Repositorio piloto usando gate em PR real

**Evidencias:**
- [ ] `docs/reports/release/R4.M6-ci-gate-report.md`
- [ ] workflow de referencia publicado

#### R4.M7 - Produto 3: Playground + trilha de aprendizado

**Entregas:**
- [ ] Playground web com compilacao/execucao de exemplos curtos
- [ ] Colecao de exemplos oficiais curada
- [ ] Trilha curta (iniciante -> app real)

**Criterio de aceite:**
- [ ] Usuario novo consegue executar exemplo sem setup local

**Evidencias:**
- [ ] `docs/reports/release/R4.M7-playground-learning-report.md`
- [ ] links publicos de playground e trilha

---

### FASE 5 — Decisao estrategica

#### R4.M8 - UI/Grafica: decisao com POC

**Entregas:**
- [ ] Implementar 1 POC pequeno de UI
- [ ] Comparar custo x impacto
- [ ] Registrar decisao formal (continuar ou adiar)

**Criterio de aceite:**
- [ ] Decisao tomada com dados de uso e manutencao

**Evidencias:**
- [ ] `docs/reports/release/R4.M8-ui-poc-decision.md`

---

### FASE 6 — Release

#### R4.M9 - Alpha.2 release

**Entregas:**
- [ ] Publicar `0.3.0-alpha.2`
- [ ] Publicar artefato + checksums
- [ ] Validar install limpo + hello world
- [ ] Publicar notas de release
- [ ] Atualizar matriz de compatibilidade
- [ ] Publicar relatorio final de risco residual

**Criterio de aceite:**
- [ ] Release `0.3.0-alpha.2` publicada e verificavel

**Evidencias:**
- [ ] `docs/reports/release/R4.M9-alpha2-release-report.md`
- [ ] `docs/reports/compatibility/R4.M9-alpha2-compatibility.md`
