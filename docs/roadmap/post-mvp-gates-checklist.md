# Checklist Pos-MVP: Gates e Gaps de Mercado

- Status: operacional
- Data base: 2026-04-20
- Premissa: marcos atuais fechados

## Como usar

- Marque somente quando o criterio estiver comprovado por teste ou artefato.
- Nao use "done" por percepcao.

## A. Gates obrigatorios de release (Go/No-Go)

### A1) Conformidade por camada

- [ ] Matriz final cobre parser por feature
- [ ] Matriz final cobre semantic por feature
- [ ] Matriz final cobre lowering/backend por feature
- [ ] Matriz final cobre runtime por feature
- [ ] Matriz final cobre diagnostics por feature
- [ ] Matriz final cobre formatter por feature
- [ ] Matriz final cobre stdlib base por feature
- [ ] Matriz final cobre CLI por feature
- [ ] Matriz final cobre ZDoc por feature

### A2) Riscos arquiteturais

- [ ] RC cycles com politica oficial e testes
- [ ] Monomorfizacao com budget e controle de bloat
- [ ] Heap-vs-stack wrappers com regra e validacao

### A3) Performance

- [ ] Suite curta em PR ativa
- [ ] Suite longa nightly ativa
- [ ] Budget por benchmark definido (`warn`/`fail`)
- [ ] Regressao critica bloqueia release
- [ ] Relatorio de performance anexado ao release candidate

### A4) Erro tipado e cleanup

- [ ] `core.Error` canonico definido
- [ ] `result<T,E>` generico funcional no backend C
- [ ] Cleanup correto para `E` managed em todos os caminhos

### A5) Qualidade de DX

- [ ] `zt fmt` obrigatorio no fluxo canonico
- [ ] Formatter idempotente com golden tests
- [ ] Diagnostics uniformes e estruturados em todos os estagios
- [ ] `zenith.ztproj` e CLI coerentes para usuario final

## B. Gaps tecnicos que nao podem ficar invisiveis

### B1) Linguagem e compatibilidade

- [ ] Politica de deprecacao publicada
- [ ] Processo RFC oficial ativo
- [ ] Plano de editions definido
- [ ] Regra de compatibilidade por versao documentada

### B2) Compilador

- [ ] Compilacao incremental por modulo/hash
- [ ] Daemon de compilacao implementado
- [ ] Telemetria tecnica por fase (tempo e memoria)
- [ ] Fuzzing continuo de parser/lowering

### B3) Runtime e backend

- [ ] Instrumentacao de retain/release em hot paths
- [ ] Stress test long-running de memoria/fragmentacao
- [ ] Modo sanitizer em CI noturno
- [ ] Panics com trace estruturado

### B4) Stdlib e ZPM

- [ ] Tipos canonicos completos em `std.io`, `std.time`, `std.os`, `std.os.process`
- [ ] `std.test` com harness real
- [ ] `std.net` com timeout e erro tipado robusto
- [ ] lockfile deterministico em todo fluxo
- [ ] `zpm audit` basico funcional
- [ ] assinatura/verificacao de pacote funcional

### B5) IDE e plataforma de extensao

- [ ] indexacao incremental de workspace
- [ ] diagnostics service com cancelamento e anti-stale
- [ ] file tree incremental com virtualizacao
- [ ] LSP bridge JSON-RPC robusta
- [ ] plugin sandbox com permissoes granulares

## C. Fora do escopo atual (monitorar, nao misturar no gate de agora)

- [ ] VM/bytecode como arquitetura principal
- [ ] Interop C publico amplo
- [ ] Backends LLVM/WASM/JS em producao
- [ ] Async IO/TLS/WebSocket como baseline canonica

## D. Criterio final de decisao

## GO

Pode promover release se:

- todos os itens da secao A estao marcados
- nenhum item critico da secao B esta vermelho

## NO-GO

Nao promover release se qualquer um ocorrer:

- regressao critica de performance sem override formal
- falha em risco arquitetural critico
- drift entre comportamento real e docs canonicas
- falha de reproducibilidade de build/pacote
