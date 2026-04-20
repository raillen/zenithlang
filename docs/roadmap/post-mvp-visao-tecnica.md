# Post-MVP: Visao Tecnica e Proximos Passos

- Status: planejamento pos-MVP
- Data: 2026-04-20
- Escopo: linguagem, compilador, backend/runtime, stdlib, tooling, IDE e governanca

## Premissa

Este documento assume que **todos os marcos atuais foram fechados**.

Com isso, o foco muda de "entregar MVP" para:

- escalar performance e confiabilidade
- fortalecer ecossistema
- sustentar evolucao sem quebrar usuarios

## Leitura Rapida

Se os marcos atuais estao verdes, os 3 gargalos de proxima fase sao:

1. Escala tecnica em projetos grandes.
2. Maturidade de ecossistema (pacotes, plugins, docs, seguranca).
3. Governanca de evolucao (compatibilidade, release, seguranca).

## 1) Linguagem (Surface e Semantica)

### Pendencias estruturais continuas

1. Manter 1 forma canonica por recurso e evitar retorno de sintaxes legadas.
2. Preservar semantica explicita para `optional` e `result`, sem ambiguidade.
3. Garantir legibilidade consistente para TDAH/dislexia em exemplos, diagnostics e docs.
4. Sustentar `dyn Trait` com custo previsivel e mensagens claras.

### Novas pendencias recomendadas

1. Criar sistema de editions (`edition = 2026`, `edition = 2028`).
2. Adotar feature gates por projeto para recursos experimentais.
3. Definir politica formal de deprecacao com janela minima.
4. Publicar guia de "custo semantico" (alocacao, copia, dispatch dinamico).
5. Criar lints de legibilidade como parte do fluxo oficial.

## 2) Compilador (Frontend, HIR/ZIR, Diagnostics)

### Pendencias estruturais continuas

1. Matriz de conformidade viva por estagio (`Parsed`, `Semantic`, `Executable`, `Conformant`).
2. Controle de bloat em monomorfizacao para casos grandes.
3. Diagnostics com bom span e baixa cascata de erro.
4. HIR/ZIR como contrato interno unico e auditavel.

### Novas pendencias recomendadas

1. Compilacao incremental real por modulo e por hash de conteudo.
2. Daemon do compilador para CLI e IDE compartilharem processo quente.
3. Pipeline de otimizacao no ZIR (DCE, simplificacao de CFG, inline controlado).
4. Perf counters por fase (`lexer`, `parser`, `binder`, `typecheck`, `lowering`, `emit`).
5. Fuzzing continuo de parser/lowering com minimizacao automatica de casos.
6. Melhorar recuperacao sintatica para reduzir "parede de erros".

## 3) Backend C e Runtime

### Pendencias estruturais continuas

1. Politica final para ciclos de RC (`weak`, handles, arenas ou cycle collector).
2. Caminho completo para `result<T,E>` generico com cleanup correto.
3. Consolidar wrappers stack-first/in-place quando viavel.
4. Concurrency por isolates com regras explicitas de transferencia.
5. Blindagem de FFI contra violacoes de ownership/ARC.

### Novas pendencias recomendadas

1. Instrumentacao nativa de memoria (retain/release hotspots, suspeita de leak).
2. Modo sanitizers (`ASan`, `UBSan`) no CI noturno.
3. Traces de panic estruturados e reproduziveis.
4. ABI publica de runtime para frameworks oficiais.
5. Stress tests long-running para fragmentacao e estabilidade.
6. Melhorias de debug nativo (`#line`, simbolos, stepping previsivel).

## 4) Stdlib e ZPM (Ecossistema)

### Pendencias estruturais continuas

1. Fechar tipos canonicos completos em `std.io`, `std.time`, `std.os`, `std.os.process`.
2. Consolidar `std.test` com harness real e estados corretos.
3. Tornar `std.net` robusto para uso real (DNS multi-IP, timeout e erro tipado).
4. Fechar ownership ARC em colecoes avancadas.
5. Completar ZDoc fisica em `zdoc/std/*`.
6. Maturidade final de lockfile e resolucao no ZPM.

### Novas pendencias recomendadas

1. Assinatura de pacotes e verificacao de proveniencia.
2. `zpm audit` com advisories de seguranca.
3. Regras para ambientes corporativos/offline.
4. Solver com ranges SemVer completos e determinismo.
5. Workspaces/monorepo no ZPM.
6. SBOM em artefatos de build/distribuicao.

## 5) Tooling CLI e IDE (Frontend + Tauri/Rust)

### Pendencias estruturais continuas

1. Perfis de diagnostico (`beginner`, `balanced`, `full`).
2. Formato action-first (`ACTION`, `WHY`, `NEXT`).
3. `zt summary` e `zt resume`.
4. `zt check --focus` e `zt check --since`.
5. LSP bridge JSON-RPC robusta.
6. Servicos Rust persistentes para jobs pesados da IDE.

### Novas pendencias recomendadas

1. Indexacao incremental do workspace com cancelamento e streaming.
2. Diagnostics service com versionamento para evitar resposta antiga.
3. Arvore de arquivos incremental com virtualizacao de UI.
4. Pipeline de build/run com streaming e cancelamento.
5. Bridge de terminal com batching e backpressure.
6. Plugin runtime com sandbox e permissoes granulares.
7. Marketplace curado com verificacao e compatibilidade por versao.
8. Suporte remoto (SSH/container) para projetos grandes.

## 6) Governanca Tecnica e Release

### Pendencias estruturais continuas

1. Manter gates de release objetivos e auditaveis.
2. Evitar drift entre docs historicas e docs canonicas.

### Novas pendencias recomendadas

1. Processo RFC oficial com criterios de aceite e rollback.
2. Politica LTS com calendario fixo.
3. Security response policy (SLA, advisory, canais de patch).
4. Matriz oficial de suporte por SO/arquitetura/toolchain.
5. Compatibility suite publica para pacotes.
6. Release train previsivel com artefatos assinados.

## 7) Prioridade Recomendada (Apos Fechamento Atual)

### P0 (0-6 meses)

1. Compilacao incremental + daemon.
2. Runtime hardening (`RC cycles` + `result<T,E>` completo).
3. Performance suite com budget e gate de CI.
4. ZPM deterministico com seguranca basica.

### P1 (6-12 meses)

1. IDE para escala (indexacao, diagnostics, file tree incremental).
2. Plugin system com sandbox.
3. Stdlib canonica final dos modulos centrais.
4. LTS + RFC + security policy.

### P2 (12-24 meses)

1. Estrategia de multi-target por fases (sem quebrar previsibilidade).
2. ABI estavel para frameworks.
3. Programa de crescimento de ecossistema.
4. Telemetria opt-in minima para guiar melhoria real.

## 8) Riscos se essa fase nao for atacada

1. A linguagem fica boa tecnicamente, mas lenta para adocao.
2. IDE vira gargalo em workspace grande.
3. Ecossistema cresce sem confianca de supply chain.
4. Regressao de performance passa sem gate.
5. Evolucao sintatica volta a conflitar sem governanca forte.

## 9) Criterio de Sucesso da Fase Pos-MVP

A fase pos-MVP esta madura quando:

1. Projetos grandes compilam e editam sem degradacao relevante.
2. Releases sao previsiveis e com compatibilidade clara.
3. Pacotes sao confiaveis, auditaveis e reproduziveis.
4. Diagnostics continuam legiveis e acionaveis em cenarios complexos.
5. O ecossistema cresce sem perder coerencia de linguagem.
