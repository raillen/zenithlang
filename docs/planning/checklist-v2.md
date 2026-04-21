# Zenith Checklist 2.0

## Objetivo

Executar e validar as entregas do roadmap 2.0 de forma objetiva e auditavel.

## Escopo

- Itens de execucao por milestone.
- Gates obrigatorios de qualidade e performance.

## Dependencias

- Upstream:
  - `docs/planning/roadmap-v2.md`
- Downstream:
  - `docs/planning/cascade-v2.md`
  - `docs/reports/*`
- Codigo/Testes relacionados:
  - `python build.py`
  - `python run_all_tests.py`
  - `tests/perf/gate_pr.ps1`

## Como usar

1. Marque apenas itens com evidencia valida.
2. Sempre rode os gates obrigatorios antes de fechar milestone.
3. Registre desvios no report correspondente.

Checklist operacional derivado de `docs/planning/roadmap-v2.md`.

Status inicial: nao iniciado
Data de criacao: 2026-04-21

## Navegacao rapida para LLMs

Ordem recomendada para qualquer task:

1. `docs/planning/roadmap-v2.md`
2. `docs/planning/checklist-v2.md`
3. `docs/planning/cascade-v2.md`
4. `docs/planning/cascade-v1.md` (contexto historico M0-M38)
5. `compiler/CODE_MAP.md`
6. `*_MAP.md` da camada alvo
7. codigo
8. testes da camada

Links de referencia fixa:

- `docs/planning/cascade-v2.md`
- `docs/planning/cascade-v1.md`
- `language/spec/compiler-model.md`
- `language/spec/surface-syntax.md`
- `language/spec/conformance-matrix.md`
- `language/spec/formatter-model.md`
- `language/spec/implementation-status.md`
- `tests/behavior/README.md`
- `tests/behavior/MATRIX.md`
- `tests/perf/README.md`

## Matriz de onde mexer (feature ou bug)

### Frontend (lexer/parser/AST)

- Maps:
  - `compiler/frontend/lexer/LEXER_MAP.md`
  - `compiler/frontend/parser/PARSER_MAP.md`
  - `compiler/frontend/ast/AST_MAP.md`
- Arquivos de entrada:
  - `compiler/frontend/lexer/lexer.c`
  - `compiler/frontend/parser/parser.c`
  - `compiler/frontend/ast/model.h`
- Testes prioritarios:
  - `tests/frontend/test_lexer.c`
  - `tests/frontend/test_parser.c`
  - `tests/frontend/test_parser_error_recovery.c`

### Semantic e diagnostics

- Maps:
  - `compiler/semantic/binder/BINDER_MAP.md`
  - `compiler/semantic/types/TYPE_SYSTEM_MAP.md`
  - `compiler/semantic/diagnostics/DIAGNOSTICS_MAP.md`
- Arquivos de entrada:
  - `compiler/semantic/binder/binder.c`
  - `compiler/semantic/types/checker.c`
  - `compiler/semantic/diagnostics/diagnostics.c`
- Testes prioritarios:
  - `tests/semantic/test_binder.c`
  - `tests/semantic/test_types.c`
  - `tests/semantic/test_constraints.c`

### HIR/ZIR lowering

- Maps:
  - `compiler/hir/lowering/HIR_LOWERING_MAP.md`
  - `compiler/zir/ZIR_MODEL_MAP.md`
  - `compiler/zir/ZIR_VERIFIER_MAP.md`
- Arquivos de entrada:
  - `compiler/hir/lowering/from_ast.c`
  - `compiler/zir/lowering/from_hir.c`
  - `compiler/zir/verifier.c`
- Testes prioritarios:
  - `tests/semantic/test_hir_lowering.c`
  - `tests/zir/test_lowering.c`
  - `tests/zir/test_verifier.c`

### Backend C e runtime

- Maps:
  - `compiler/targets/c/EMITTER_MAP.md`
  - `runtime/c/RUNTIME_MAP.md`
- Arquivos de entrada:
  - `compiler/targets/c/emitter.c`
  - `compiler/targets/c/legalization.c`
  - `runtime/c/zenith_rt.c`
- Testes prioritarios:
  - `tests/targets/c/test_emitter.c`
  - `tests/targets/c/test_legalization.c`
  - `tests/runtime/c/test_shared_text.c`
  - `tests/behavior/*`

### Tooling, formatter e projeto

- Maps:
  - `compiler/tooling/FORMATTER_MAP.md`
  - `compiler/driver/DRIVER_MAP.md`
  - `compiler/project/PROJECT_MAP.md`
- Arquivos de entrada:
  - `compiler/tooling/formatter.c`
  - `compiler/driver/main.c`
  - `compiler/project/zdoc.c`
- Testes prioritarios:
  - `tests/formatter/run_formatter_golden.py`
  - `tests/conformance/test_m9.c`

### Performance (compilador e runtime)

- Ler:
  - `tests/perf/README.md`
- Suites:
  - `tests/perf/gate_pr.ps1`
  - `tests/perf/gate_nightly.ps1`
  - `tests/perf/run_perf.py`
- Arquivos de entrada mais provaveis:
  - `compiler/driver/main.c`
  - `compiler/frontend/parser/parser.c`
  - `compiler/hir/lowering/from_ast.c`
  - `compiler/targets/c/emitter.c`
  - `runtime/c/zenith_rt.c`

## Gates obrigatorios por milestone

- [ ] `python build.py` verde
- [ ] `python run_all_tests.py` verde
- [ ] `tests/perf/gate_pr.ps1` verde
- [ ] sem crash novo em fuzz/corpus
- [ ] evidencia anexada na milestone

Para release:

- [ ] `tests/perf/gate_nightly.ps1` verde
- [ ] sem regressao acima de budget
- [ ] riscos P0 fechados ou aceitos formalmente

## R2.PRE - Refatoracao de Conformidade da Stdlib (MVP)

- [ ] Reescrever `std.io` (`058-stdlib-io.md`): handles tipados explícitos, remoção do booleano.
- [ ] Reescrever `std.format` (`060-stdlib-format.md`): UI formatters (number, percent, dates, sizes).
- [ ] Refatorar `std.math` e `std.validate` (`062`, `061`): Inserir constantes faltantes e aliases nativos.
- [ ] Refatorar `std.time` (`063`): Garantir sleep semântico, tipos explícitos para constraints.
- [ ] Refatorar `std.fs`, `std.fs.path` e `std.os` (`064`, `065`, `067`, `068`): Estruturas fortes OS-independent, tipagem de paths.
- [ ] Refatorar `std.json` (`066`): Node type rigído, obj/array cast methods e type safe fails.
- [ ] Gerar ZDocs para as novas implementações (`stdlib/zdoc/std/...`) e atestar 100% de match via `zt doc check .`.

Criterio de aceite:

- [ ] Todos os módulos da `stdlib/std/*` e seus ZDocs perfeitamente alinhados aos documentos 058 a 069.
- [ ] Compilação de bibliotecas OK e Semântica MVP estrita assegurada.

## R2.M0 - Baseline e governanca
- [ ] Congelar baseline de qualidade (pass/fail/skip)
- [ ] Congelar baseline de performance (tempo/memoria/binario)
- [ ] Definir owners por camada
- [ ] Definir SLA de bug por severidade
- [ ] Definir template de bug report com reproducao minima
- [ ] Definir fluxo obrigatorio de regressao por bug
- [ ] Publicar regra de bloqueio para P0
- [ ] Validar comando de triagem unica do ciclo

Criterio de aceite:

- [ ] Governanca publicada e usada no fluxo real

## R2.M1 - Infra de testes 2.0

- [ ] Separar suites `smoke`, `pr_gate`, `nightly`, `stress`
- [ ] Garantir comando unico por suite
- [ ] Gerar relatorio por camada
- [ ] Gerar contagem por classe de falha
- [ ] Salvar artifacts de falha automaticamente
- [ ] Definir seeds estaveis para reexecucao
- [ ] Definir limite maximo de tempo por suite
- [ ] Integrar `pr_gate` ao fluxo padrao de PR

Criterio de aceite:

- [ ] Suites reproduziveis e com relatorio claro

## R2.M2 - Robustez de frontend (fuzzing)

- [ ] Criar harness de fuzz para lexer
- [ ] Criar harness de fuzz para parser
- [ ] Criar corpus inicial de casos validos e invalidos
- [ ] Criar mutadores para tokens e nesting
- [ ] Adicionar detector de crash e timeout
- [ ] Adicionar minimizador de input
- [ ] Persistir seeds que quebram
- [ ] Reexecutar seeds em toda rodada relevante

Criterio de aceite:

- [ ] Campanha minima de fuzz sem crash novo aberto

## R2.M3 - Hardening semantico e propriedades

- [ ] Definir invariantes para property tests
- [ ] Cobrir optional/result por propriedades
- [ ] Cobrir where/contracts por propriedades
- [ ] Cobrir conversoes e overflow
- [ ] Criar matriz de testes negativos por `ZT_DIAG_*`
- [ ] Validar spans e mensagens em erros centrais
- [ ] Transformar bugs recentes em regressao permanente
- [ ] Garantir execucao no `pr_gate`

Criterio de aceite:

- [ ] Regressoes semanticas criticas cobertas

## R2.M4 - Observabilidade de performance

- [ ] Rodar baseline com `tests/perf/run_perf.py`
- [ ] Rodar gate rapido com `tests/perf/gate_pr.ps1`
- [ ] Rodar gate longo com `tests/perf/gate_nightly.ps1`
- [ ] Identificar top hotspots do compilador
- [ ] Identificar top hotspots do runtime
- [ ] Atualizar budgets por benchmark quando necessario
- [ ] Registrar metodologia de medicao no repo
- [ ] Publicar relatorio de hotspots priorizados

Criterio de aceite:

- [ ] Hotspots priorizados com plano de execucao

## R2.M5 - Otimizacao do compilador

- [ ] Otimizar parser em caminhos quentes mapeados
- [ ] Otimizar lowering HIR/ZIR em caminhos quentes mapeados
- [ ] Otimizar emitter C em caminhos quentes mapeados
- [ ] Otimizar driver para reduzir trabalho redundante
- [ ] Medir antes e depois por benchmark
- [ ] Garantir sem regressao funcional
- [ ] Registrar ganhos e tradeoffs
- [ ] Atualizar budgets se o ganho for consolidado

Criterio de aceite:

- [ ] Ganho mensuravel sem perda de corretude

## R2.M6 - Otimizacao do runtime

- [ ] Otimizar `text` nos hot paths medidos
- [ ] Otimizar `collections` nos hot paths medidos
- [ ] Otimizar caminhos de `optional/result` medidos
- [ ] Revisar custo de retain/release nos hot paths
- [ ] Validar estabilidade de memoria no gate longo
- [ ] Medir antes e depois por benchmark
- [ ] Garantir sem regressao funcional
- [ ] Registrar ganhos e riscos residuais

Criterio de aceite:

- [ ] Ganho mensuravel sem regressao de memoria

## R2.M7 - Auditoria spec x implementacao

- [ ] Criar matriz `Spec/Parsed/Semantic/Lowered/Emitted/Executable`
- [ ] Auditar surface syntax x parser
- [ ] Auditar type system x checker
- [ ] Auditar diagnostics x codigos esperados
- [ ] Auditar formatter x forma canonica
- [ ] Auditar runtime/stdlib x docs
- [ ] Classificar divergencias P0/P1/P2
- [ ] Definir owner e prazo para cada P0/P1

Criterio de aceite:

- [ ] Nenhum P0 sem plano de fechamento

## R2.M8 - Fechar deferreds P0

- [ ] Definir design final de `fmt` v2
- [ ] Implementar parser de `fmt` v2
- [ ] Implementar lowering HIR de `fmt` v2
- [ ] Implementar lowering ZIR/backend/runtime de `fmt` v2
- [ ] Implementar formatter canonico de `fmt` v2
- [ ] Atualizar specs e decisions relacionadas
- [ ] Criar testes positivos de `fmt` v2
- [ ] Criar testes negativos de `fmt` v2

Criterio de aceite:

- [ ] `fmt` v2 executavel de ponta a ponta

## R2.M9 - Feature Pack A (linguagem)

- [ ] Priorizar top 3 features de linguagem
- [ ] Criar mini design doc por feature
- [ ] Implementar feature A1 com testes completos
- [ ] Implementar feature A2 com testes completos
- [ ] Implementar feature A3 com testes completos
- [ ] Atualizar docs de linguagem por feature
- [ ] Medir impacto de performance por feature
- [ ] Definir criterio de rollback por feature

Criterio de aceite:

- [ ] Features A entregues com gates verdes

## R2.M10 - Feature Pack B (stdlib e tooling)

- [ ] Priorizar modulos de stdlib por impacto
- [ ] Entregar melhorias de stdlib com behavior tests
- [ ] Fortalecer `zt fmt --check` no fluxo oficial
- [ ] Fortalecer `zt doc check` no fluxo oficial
- [ ] Melhorar diagnosticos action-first
- [ ] Melhorar relatorio para contribuidores
- [ ] Atualizar README e docs de contribuicao
- [ ] Registrar mudancas de compatibilidade

Criterio de aceite:

- [ ] Tooling e stdlib evoluidos sem regressao

## R2.M11 - Release candidate 2.0

- [ ] Congelar entrada de features novas
- [ ] Rodar `smoke + pr_gate + nightly + stress`
- [ ] Fechar bugs P0 pendentes
- [ ] Tratar bugs P1 restantes
- [ ] Atualizar matriz final de conformidade
- [ ] Gerar changelog do ciclo 2.0
- [ ] Publicar relatorio de qualidade/performance
- [ ] Validar checklist final de release

Criterio de aceite:

- [ ] Release candidate aprovado

## R2.M12 - Alpha pre-release (0.3.0-alpha.1)

- [ ] Confirmar versao de release como 0.3.0-alpha.1
- [ ] Fechar P0 de coerencia (check/panic, Comparable/Order, operadores relacionais, core.Error, u8..u64)
- [ ] Rodar python build.py com resultado verde
- [ ] Rodar python run_all_tests.py com resultado verde
- [ ] Rodar tests/perf/gate_pr.ps1 com resultado verde
- [ ] Validar ausencia de crash novo em fuzz/corpus
- [ ] Gerar pacote de distribuicao de zt.exe
- [ ] Publicar checksum e hash do artefato
- [ ] Validar instalacao limpa em ambiente sem residuos
- [ ] Validar hello world (check/build/run) no pacote publicado
- [ ] Publicar notas de alpha com limites conhecidos
- [ ] Registrar compatibilidade: platform/ continua interno e vazio

Criterio de aceite:

- [ ] Artefatos de 0.3.0-alpha.1 publicados e verificaveis
- [ ] Nenhum P0 aberto sem aceite formal
- [ ] Fluxo de instalacao e primeiro build reproduzivel

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
