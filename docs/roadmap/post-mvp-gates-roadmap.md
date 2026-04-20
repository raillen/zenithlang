# Roadmap Pos-MVP: Gates, Gaps e Escala (2026-2028)

- Status: planned
- Data base: 2026-04-20
- Premissa: todos os marcos atuais estao fechados
- Escopo: linguagem, compilador, runtime, stdlib, zpm, CLI, IDE, governanca

## Objetivo

Sair de "MVP completo" para "plataforma confiavel em escala".

Isso exige 3 resultados:

1. Escala tecnica em projetos grandes.
2. Ecossistema confiavel para times e empresas.
3. Evolucao com compatibilidade previsivel.

## Pilares

1. Performance com medicao e gate.
2. Confiabilidade com matriz de conformidade viva.
3. Ecossistema com seguranca e reproducibilidade.
4. Governanca tecnica com regras claras de mudanca.

## Fase P0 (0-6 meses)

Foco: fundacao de escala e protecao contra regressao.

### 1) Performance e Observabilidade

- criar suite oficial de benchmark (`tests/perf`) com runner unico
- medir `zt check`, `zt build`, `zt run` em cenarios `small`, `medium`, `large`
- gerar saida JSON e Markdown
- definir budget por benchmark (`warn` e `fail`)

Gate de saida:

- suite curta em PR e suite longa em nightly
- release bloqueia quando regressao critica passar budget

### 2) Compilador

- implementar compilacao incremental por modulo/hash
- implementar daemon de compilacao para reduzir cold start
- adicionar perf counters por fase (`lexer`, `parser`, `binder`, `typecheck`, `lowering`, `emit`)

Gate de saida:

- ganho medido em build warm
- sem regressao de diagnostico e sem regressao de conformidade

### 3) Runtime e Backend C

- fechar politica de RC cycles (`weak`, handles/arenas, ownership graph, ou collector)
- completar caminho `result<T,E>` generico com cleanup correto
- reforcar wrappers stack-first para `optional/result` quando viavel

Gate de saida:

- testes de stress sem leak relevante
- testes de cleanup em `return`, `?`, `break`, `continue`

### 4) ZPM e Supply Chain Basica

- lockfile deterministico completo
- verificacao de origem de pacote
- base de `zpm audit`

Gate de saida:

- build reproduzivel em maquina limpa
- erro explicito para dependencia insegura

## Fase P1 (6-12 meses)

Foco: maturidade de produto para equipes.

### 1) IDE e Ferramental

- indexacao incremental de workspace
- diagnostics service com cancelamento e anti-stale result
- file tree incremental com virtualizacao
- bridge LSP JSON-RPC robusta

Gate de saida:

- workspace grande abre e navega sem degradacao severa
- diagnostico nao chega fora de ordem

### 2) Stdlib Canonica

- fechar tipos canonicos de `std.io`, `std.time`, `std.os`, `std.os.process`
- `std.test` com harness real
- `std.net` baseline robusto (timeout e DNS multi-IP)
- finalizar ownership ARC nas colecoes avancadas

Gate de saida:

- behavior tests e integration tests para cada modulo
- docs ZDoc completas em `zdoc/std/*`

### 3) Governanca e Compatibilidade

- publicar processo RFC oficial
- publicar politica de deprecacao
- publicar politica LTS e calendario de release
- publicar security response policy

Gate de saida:

- cada mudanca de linguagem passa por fluxo RFC
- cada release tem notas de compatibilidade claras

## Fase P2 (12-24 meses)

Foco: plataforma completa sem perder previsibilidade.

### 1) Multi-target Strategy

- planejar alvos novos por fase (sem quebrar backend C)
- definir requisitos de conformidade para novos backends

Gate de saida:

- qualquer alvo novo entra com matriz de conformidade propria
- sem downgrade de semantica canonica

### 2) ABI e Framework Readiness

- congelar ABI de runtime para frameworks oficiais
- padronizar observabilidade de app (log/metrics/tracing)

Gate de saida:

- contratos estaveis para integracoes de longo prazo

### 3) Ecossistema e Marketplace

- marketplace curado com verificacao e permissao explicita
- plugin runtime com sandbox
- fluxo de assinatura e integridade de packs

Gate de saida:

- instalacao segura por padrao
- permissao de alto risco sempre explicita

## Fora do Corte Atual (reavaliar apos P2)

1. VM/bytecode como arquitetura principal.
2. Interop C publico amplo.
3. Backends LLVM/WASM/JS em producao.
4. Async IO/TLS/WebSocket como baseline canonica.

## Riscos que precisam ficar visiveis

1. Bloat de monomorfizacao.
2. RC cycles em grafo rico.
3. Regressao silenciosa de performance.
4. Drift entre docs historicas e docs canonicas.

## Definicao de sucesso da trilha pos-MVP

A trilha pos-MVP esta madura quando:

1. performance e estabilidade viram gate oficial de release
2. ecossistema e reproducivel e auditavel
3. evolucao de linguagem ocorre com compatibilidade previsivel
4. IDE e CLI escalam para projetos grandes sem friccao severa
