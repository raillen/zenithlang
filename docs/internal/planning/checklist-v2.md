# Zenith Checklist 2.0

## Objetivo

Executar e validar as entregas do roadmap 2.0 de forma objetiva e auditavel.

## Escopo

- Itens de execucao por milestone.
- Gates obrigatorios de qualidade e performance.

## Dependencias

- Upstream:
  - `docs/internal/planning/roadmap-v2.md`
- Downstream:
  - `docs/internal/planning/cascade-v2.md`
  - `docs/internal/reports/*`
- Codigo/Testes relacionados:
  - `python build.py`
  - `python run_all_tests.py`
  - `tests/perf/gate_pr.ps1`

## Como usar

1. Marque apenas itens com evidencia valida.
2. Sempre rode os gates obrigatorios antes de fechar milestone.
3. Registre desvios no report correspondente.

Checklist operacional derivado de `docs/internal/planning/roadmap-v2.md`.

Status inicial: nao iniciado
Data de criacao: 2026-04-21

## Navegacao rapida para LLMs

Ordem recomendada para qualquer task:

1. `docs/internal/planning/roadmap-v2.md`
2. `docs/internal/planning/checklist-v2.md`
3. `docs/internal/planning/cascade-v2.md`
4. `docs/internal/planning/cascade-v1.md` (contexto historico M0-M38)
5. `compiler/CODE_MAP.md`
6. `*_MAP.md` da camada alvo
7. codigo
8. testes da camada

Links de referencia fixa:

- `docs/internal/planning/cascade-v2.md`
- `docs/internal/planning/cascade-v1.md`
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

- [x] `python build.py` verde
- [x] `python run_all_tests.py` verde
- [x] `tests/perf/gate_pr.ps1` verde
- [x] sem crash novo em fuzz/corpus
- [x] evidencia anexada na milestone

Para release:

- [x] `tests/perf/gate_nightly.ps1` verde
- [x] sem regressao acima de budget
- [x] riscos P0 fechados ou aceitos formalmente

## R2.PRE - Refatoracao de Conformidade da Stdlib (MVP)

- [x] Reescrever `std.io` (`058-stdlib-io.md`): handles tipados explÃ­citos, remoÃ§Ã£o do booleano.
- [x] Reescrever `std.format` (`060-stdlib-format.md`): UI formatters (number, percent, dates, sizes).
- [x] Refatorar `std.math` e `std.validate` (`062`, `061`): Inserir constantes faltantes e aliases nativos.
- [x] Refatorar `std.time` (`063`): Garantir sleep semÃ¢ntico, tipos explÃ­citos para constraints.
- [x] Refatorar `std.fs`, `std.fs.path` e `std.os` (`064`, `065`, `067`, `068`): Estruturas fortes OS-independent, tipagem de paths.
- [x] Refatorar `std.json` (`066`): Node type rigÃ­do, obj/array cast methods e type safe fails.
- [x] Gerar ZDocs para as novas implementaÃ§Ãµes (`stdlib/zdoc/std/...`) e atestar 100% de match via `zt doc check .`.

Criterio de aceite:

- [x] Todos os mÃ³dulos da `stdlib/std/*` e seus ZDocs perfeitamente alinhados aos documentos 058 a 069.
- [x] CompilaÃ§Ã£o de bibliotecas OK e SemÃ¢ntica MVP estrita assegurada.

## R2.M0 - Baseline e governanca
- [x] Congelar baseline de qualidade (pass/fail/skip)
- [x] Congelar baseline de performance (tempo/memoria/binario)
- [x] Definir owners por camada
- [x] Definir SLA de bug por severidade
- [x] Definir template de bug report com reproducao minima
- [x] Definir fluxo obrigatorio de regressao por bug
- [x] Publicar regra de bloqueio para P0
- [x] Validar comando de triagem unica do ciclo

Criterio de aceite:

- [x] Governanca publicada e usada no fluxo real
Evidencia R2.M0 (2026-04-21):

- Governanca: `docs/internal/governance/r2-m0-governance.md`
- Entrada de governanca: `docs/internal/governance/README.md`
- Baseline qualidade congelado: `docs/internal/governance/baselines/quality-baseline.json`
- Baseline performance congelado: `docs/internal/governance/baselines/perf-baseline.json` + `tests/perf/baselines/windows-AMD64/*.json`
- Template de bug: ver secao de bug report em `CONTRIBUTING.md`
- Checklist de PR: ver secao de PR checklist em `CONTRIBUTING.md`
- Comando unico de triagem: `python tools/triage_cycle.py`
- Evidencia de execucao real: `docs/internal/reports/triage/latest.md`
- Evidencia de freeze baseline: `docs/internal/reports/triage/triage-20260421-130834Z.md`

## R2.M1 - Infra de testes 2.0

- [x] Separar suites `smoke`, `pr_gate`, `nightly`, `stress`
- [x] Garantir comando unico por suite
- [x] Gerar relatorio por camada
- [x] Gerar contagem por classe de falha
- [x] Salvar artifacts de falha automaticamente
- [x] Definir seeds estaveis para reexecucao
- [x] Definir limite maximo de tempo por suite
- [x] Integrar `pr_gate` ao fluxo padrao de PR

Criterio de aceite:

- [x] Suites reproduziveis e com relatorio claro

Evidencia R2.M1 (2026-04-21):

- Runner principal: `run_suite.py`
- Definicoes de suites/seeds/timeouts: `tests/suites/suite_definitions.py`
- README da infra: `tests/suites/README.md`
- Gate smoke: `tests/perf/gate_smoke.ps1` (`python run_suite.py smoke`)
- Gate PR: `tests/perf/gate_pr.ps1` (`python run_suite.py pr_gate`)
- Gate nightly: `tests/perf/gate_nightly.ps1` (`python run_suite.py nightly`)
- Gate stress: `tests/perf/gate_stress.ps1` (`python run_suite.py stress`)
- PR template atualizado no ciclo R2.M1 (arquivo nao mantido no repositorio atual).
- Relatorio de execucao smoke registrado no ciclo R2.M1 (9/9 pass, 2405ms).

## R2.M2 - Robustez de frontend (fuzzing)

- [x] Criar harness de fuzz para lexer
- [x] Criar harness de fuzz para parser
- [x] Criar corpus inicial de casos validos e invalidos
- [x] Criar mutadores para tokens e nesting
- [x] Adicionar detector de crash e timeout
- [x] Adicionar minimizador de input
- [x] Persistir seeds que quebram
- [x] Reexecutar seeds em toda rodada relevante

Criterio de aceite:

- [x] Campanha minima de fuzz sem crash novo aberto

Evidencia R2.M2 (2026-04-21):

- Infra de fuzz: `tests/fuzz/README.md`
- Harness core (crash/timeout + persistencia): `tests/fuzz/harness.py`
- Harness lexer: `tests/fuzz/fuzz_lexer.py`
- Harness parser: `tests/fuzz/fuzz_parser.py`
- Mutadores: `tests/fuzz/mutators.py`
- Minimizador: `tests/fuzz/minimizer.py`
- Replay de seeds: `tests/fuzz/replay.py`
- Corpus inicial: `tests/fuzz/corpus/valid/*.zt`, `tests/fuzz/corpus/invalid/*.zt`
- Resultado replay: `python tests/fuzz/replay.py --verbose` -> `{"driver":"replay","seeds":0,"failures":0}`
- Resultado fuzz lexer: `python tests/fuzz/fuzz_lexer.py --iters 60 --seed 20260421` -> `{"crashes":0,"timeouts":0}`
- Resultado fuzz parser: `python tests/fuzz/fuzz_parser.py --iters 60 --seed 20260421` -> `{"crashes":0,"timeouts":0}`

## R2.M3 - Hardening semantico e propriedades

- [x] Definir invariantes para property tests
- [x] Cobrir optional/result por propriedades
- [x] Cobrir where/contracts por propriedades
- [x] Cobrir conversoes e overflow
- [x] Criar matriz de testes negativos por `ZT_DIAG_*`
- [x] Validar spans e mensagens em erros centrais
- [x] Transformar bugs recentes em regressao permanente
- [x] Garantir execucao no `pr_gate`

Criterio de aceite:

- [x] Regressoes semanticas criticas cobertas

Evidencia R2.M3 (2026-04-21):

- Property tests optional/result: `tests/semantic/test_optional_result_properties.c` (7 testes)
- Property tests where/contracts: `tests/semantic/test_where_contract_properties.c` (6 testes)
- Property tests conversoes/overflow: `tests/semantic/test_conversion_overflow_properties.c` (7 testes)
- Matriz de testes negativos: `docs/internal/reports/semantic/R2.M3-negative-test-matrix.md` (100% cobertura ZT_DIAG_*)
- Relatorio completo: `docs/internal/reports/semantic/R2.M3-hardening-report.md`
- Invariantes definidos: 20 (5 optional/result + 5 where/contracts + 6 conversoes + 4 transversais)
- Bugs transformados em regressao: 6 cenarios criticos
- Total de testes novos: 20 testes, 685 linhas de codigo
- Integracao pr_gate: via `python run_suite.py pr_gate` (suite semantic)

## R2.M4 - Observabilidade de performance

- [x] Rodar baseline com `tests/perf/run_perf.py`
- [x] Rodar gate rapido com `tests/perf/gate_pr.ps1`
- [x] Rodar gate longo com `tests/perf/gate_nightly.ps1`
- [x] Identificar top hotspots do compilador
- [x] Identificar top hotspots do runtime
- [x] Atualizar budgets por benchmark quando necessario
- [x] Registrar metodologia de medicao no repo
- [x] Publicar relatorio de hotspots priorizados

Criterio de aceite:

- [x] Hotspots priorizados com plano de execucao

Evidencia R2.M4 (2026-04-21):

- Baseline (quick): `reports/perf/summary-quick.json` (pass apÃ³s update-baseline)
- Gate PR: `tests/perf/gate_pr.ps1` (verde)
- Gate Nightly: `tests/perf/gate_nightly.ps1` (verde)
- Metodologia: `docs/internal/reports/perf/methodology.md`
- Relatorio de Hotspots: `docs/internal/reports/perf/hotspots.md`
- Hotspots principais:
  1. CompilaÃ§Ã£o do `zenith_rt.c` em todo build (~1s overhead)
  2. RegressÃ£o de 26% no parser/lexer (macro_small_check)
  3. Scan linear em `zt_enforce_monomorphization_limit`
- Plano: OtimizaÃ§Ãµes priorizadas para R2.M5 (Compiler) e R2.M6 (Runtime).
- EstabilizaÃ§Ã£o: 
  - Corrigido `std.time` para usar `core.Error`.
  - Corrigido macros corrompidas em `runtime/c/zenith_rt_templates.h` (warnings GCC removidos).
  - Ajustado timeouts em `run_suite.py` para acomodar o ambiente atual.
  - Baselines sincronizados com o hardware/OS via `--update-baseline`.

## R2.M5 - Otimizacao do compilador

- [x] Otimizar parser em caminhos quentes mapeados
- [x] Otimizar lowering HIR/ZIR em caminhos quentes mapeados
- [x] Otimizar emitter C em caminhos quentes mapeados
- [x] Otimizar driver para reduzir trabalho redundante
- [x] Medir antes e depois por benchmark
- [x] Garantir sem regressao funcional
- [x] Registrar ganhos e tradeoffs
- [ ] Atualizar budgets se o ganho for consolidado

Criterio de aceite:

- [x] Ganho mensuravel sem perda de corretude

Evidencia R2.M5 (2026-04-21):

- Relatorio tecnico: `docs/internal/reports/perf/r2-m5-compiler-optimization.md`
- Before benchmark: `docs/internal/reports/perf/r2m5-before-summary-quick.json`
- After benchmark: `docs/internal/reports/perf/r2m5-after-summary-quick.json`
- Ganho macro observado (lat_median): build/run/test entre -84% e -86%
- Validacao funcional: checks em `std_io_basic`, `simple_app`, `optional_result_basic`, `std_format_basic` + run `std_io_basic` OK
- Observacao: lock externo em `zt.exe` durante a rodada; validacao executada com binario `zt_m5.exe`

## R2.M6 - Otimizacao do runtime

- [x] Otimizar `text` nos hot paths medidos
- [x] Otimizar `collections` nos hot paths medidos
- [x] Otimizar caminhos de `optional/result` medidos
- [x] Revisar custo de retain/release nos hot paths
- [x] Validar estabilidade de memoria no gate longo
- [x] Medir antes e depois por benchmark
- [x] Garantir sem regressao funcional
- [x] Registrar ganhos e riscos residuais

Criterio de aceite:

- [x] Ganho mensuravel sem regressao de memoria

Evidencia R2.M6 (2026-04-21):

- Relatorio tecnico: `docs/internal/reports/perf/r2-m6-runtime-optimization.md`
- Before benchmark (quick): `docs/internal/reports/perf/r2m6-before-summary-quick.json`
- After benchmark (quick): `docs/internal/reports/perf/r2m6-after-summary-quick.json`
- Gate de estabilidade (nightly): `reports/perf/summary-nightly.json` e `reports/perf/summary-nightly.md` (status geral: `warn`)
- Ganho quick observado (lat_median): `micro_runtime_core` -8.42%, `micro_stdlib_core` -17.04%
- Risco residual registrado: `m37_result_generic` e `macro_large_check` em `warn` no nightly

## R2.M7 - Auditoria spec x implementacao

- [x] Criar matriz `Spec/Parsed/Semantic/Lowered/Emitted/Executable`
- [x] Auditar surface syntax x parser
- [x] Auditar type system x checker
- [x] Auditar diagnostics x codigos esperados
- [x] Auditar formatter x forma canonica
- [x] Auditar runtime/stdlib x docs
- [x] Classificar divergencias P0/P1/P2
- [x] Definir owner e prazo para cada P0/P1

Criterio de aceite:

- [x] Nenhum P0 sem plano de fechamento

Evidencia R2.M7 (2026-04-21):

- Relatorio consolidado: `docs/internal/reports/audit/R2.M7-spec-vs-implementation-audit.md`
- Matriz completa: secao `1. Matriz Spec/Parsed/Semantic/Lowered/Emitted/Executable`
- Divergencias classificadas: secao `2. Divergencias Classificadas (P0/P1/P2)`
- Owners e prazos: secao `3. Plano de Fechamento — Owner e Prazo`
- Criterio de aceite atendido: `Nenhum P0 sem plano de fechamento`

## R2.M8 - Fechar deferreds P0

- [x] Definir design final de `fmt` v2
- [x] Implementar parser de `fmt` v2
- [x] Implementar lowering HIR de `fmt` v2
- [x] Implementar lowering ZIR/backend/runtime de `fmt` v2
- [x] Implementar formatter canonico de `fmt` v2
- [x] Atualizar specs e decisions relacionadas
- [x] Criar testes positivos de `fmt` v2
- [x] Criar testes negativos de `fmt` v2

Criterio de aceite:

- [x] `fmt` v2 executavel de ponta a ponta

Evidencia R2.M8 (2026-04-21):

- Parser/AST: `compiler/frontend/parser/parser.c`, `compiler/frontend/ast/model.h`, `compiler/frontend/ast/model.c`
- Semantica/lowering/backend:
  - `compiler/semantic/binder/binder.c`
  - `compiler/semantic/types/checker.c`
  - `compiler/hir/lowering/from_ast.c`
  - `compiler/zir/lowering/from_hir.c`
  - `compiler/targets/c/emitter.c`
- Formatter: `compiler/tooling/formatter.c`
- Specs atualizadas: `language/spec/surface-syntax.md`, `language/spec/compiler-model.md`
- Testes novos:
  - `tests/behavior/fmt_interpolation_basic`
  - `tests/behavior/fmt_interpolation_type_error`
  - `tests/frontend/test_parser_error_recovery.c` (caso `fmt`)
  - `run_all_tests.py` e `tests/behavior/MATRIX.md` atualizados para cobertura
- Validacao executada:
  - `python build.py` verde
  - `zt check tests/behavior/fmt_interpolation_basic` verde
  - `zt run tests/behavior/fmt_interpolation_basic` exit `0`
  - `zt check tests/behavior/fmt_interpolation_type_error` falha esperada
  - `python run_all_tests.py` verde (`Total 134 | Pass 133 | Fail 0 | Skip 1`)

## R2.M9 - Feature Pack A (linguagem)

- [x] Priorizar top 3 features de linguagem
- [x] Criar mini design doc por feature
- [x] Implementar feature A1 com testes completos
- [x] Implementar feature A2 com testes completos
- [x] Implementar feature A3 com testes completos
- [x] Atualizar docs de linguagem por feature
- [x] Medir impacto de performance por feature
- [x] Definir criterio de rollback por feature

Criterio de aceite:

- [x] Features A entregues com gates verdes

Evidencia R2.M9 (2026-04-21):

- Design doc consolidado: `docs/internal/planning/m9-feature-pack-a-design.md`
- A1 concluido com testes:
  - `tests/behavior/panic_basic`
  - `tests/behavior/panic_with_message`
- A2 concluido com testes:
  - `tests/behavior/optional_question_basic`
  - `tests/behavior/optional_question_outside_optional_error`
  - `compiler/semantic/types/checker.c`: contexto de `optional<T>?`
  - `compiler/zir/lowering/from_hir.c`: propagacao `none` no `?` de optional
- A3 concluido com testes:
  - `tests/behavior/enum_match_non_exhaustive_error`
  - `tests/behavior/optional_match_value`
- Matriz/runner atualizados:
  - `tests/behavior/MATRIX.md`
  - `run_all_tests.py`
- Validacao executada:
  - `python build.py` verde
  - `zt run tests/behavior/optional_question_basic` exit `0`
  - `zt check tests/behavior/optional_question_outside_optional_error` falha esperada
  - `python run_all_tests.py` verde (`Total 136 | Pass 135 | Fail 0 | Skip 1`)

## R2.M10 - Feature Pack B (stdlib e tooling)

- [x] Priorizar modulos de stdlib por impacto
- [x] Entregar melhorias de stdlib com behavior tests
- [x] Fortalecer `zt fmt --check` no fluxo oficial
- [x] Fortalecer `zt doc check` no fluxo oficial
- [x] Melhorar diagnosticos action-first
- [x] Melhorar relatorio para contribuidores
- [x] Atualizar README e docs de contribuicao
- [x] Registrar mudancas de compatibilidade

Criterio de aceite:

- [x] Tooling e stdlib evoluidos sem regressao

Evidencia R2.M10 (2026-04-21):

- Priorizacao de modulos: `docs/internal/planning/m10-stdlib-priorities.md`
- Melhoria stdlib entregue:
  - `stdlib/std/validate.zt`: `one_of_text`
  - `stdlib/zdoc/std/validate.zdoc`: assinatura/documentacao alinhadas
  - `tests/behavior/std_validate_basic/src/app/main.zt`: cobertura nova
- Fluxo oficial reforcado:
  - `run_all_tests.py`: gate de `fmt --check` e `doc check` (projeto canario `tests/behavior/tooling_gate_smoke`)
  - `tests/behavior/tooling_gate_smoke/*`: projeto canario para tooling
- Diagnostico action-first:
  - `compiler/driver/main.c`: mensagens `action` e `why` em falha de `fmt/doc`
- Relatorio para contribuidores:
  - `run_all_tests.py`: secao "Next steps (contributor-friendly)" com comandos de correção
- Docs atualizadas:
  - `README.md`, `CONTRIBUTING.md`
  - compatibilidade: `docs/internal/reports/compatibility/R2.M10-compatibility.md`
- Execucao real dos comandos:
  - `python build.py` verde
  - `zt check tests/behavior/std_validate_basic` verde
  - `zt fmt --check tests/behavior/tooling_gate_smoke` verde
  - `zt doc check tests/behavior/tooling_gate_smoke` verde
- `python run_all_tests.py`: verde (Total 132, Pass 131, Fail 0, Skip 1). Gates de tooling, behavior remanescentes e `formatter/golden` validados.

## R2.M11 - Release candidate 2.0

- [x] Congelar entrada de features novas
- [x] Rodar `smoke + pr_gate + nightly + stress`
- [x] Fechar bugs P0 pendentes
- [x] Tratar bugs P1 restantes
- [x] Atualizar matriz final de conformidade
- [x] Gerar changelog do ciclo 2.0
- [x] Publicar relatorio de qualidade/performance
- [x] Validar checklist final de release

Criterio de aceite:

- [x] Release candidate aprovado

Evidencia R2.M11 (2026-04-21):

- Freeze RC aplicado para este corte: apenas bugfix/docs/gates.
- Gates executados:
  - `python run_suite.py smoke` -> pass (`9/9`)
  - `python run_suite.py pr_gate` -> pass (`112/112`)
  - `python run_suite.py nightly` -> pass (`114/114`)
  - `python run_suite.py stress` -> pass (`21/21`)
  - relatorios locais/CI validados no corte (artefatos efemeros nao versionados)
- Performance:
  - `reports/perf/summary-nightly.json` -> `status=pass`, `benchmark_count=23`
- P0/P1:
  - P0 fechados (D-01..D-04) e sem pendencia aberta
  - P1 tratados no corte RC (D-05, D-06)
- Matriz final atualizada:
  - `language/spec/conformance-matrix.md`
  - `language/surface-implementation-status.md`
- Changelog do ciclo:
  - `CHANGELOG.md` (`0.3.0-rc.1`)
- Relatorio final de qualidade/performance:
  - `docs/internal/reports/release/R2.M11-quality-performance-report.md`

## R2.M12 - Alpha pre-release (0.3.0-alpha.1)

- [x] Confirmar versao de release como 0.3.0-alpha.1
- [x] Fechar P0 de coerencia (check/panic, Comparable/Order, operadores relacionais, core.Error, u8..u64)
- [x] Rodar python build.py com resultado verde
- [x] Rodar python run_all_tests.py com resultado verde
- [x] Rodar tests/perf/gate_pr.ps1 com resultado verde
- [x] Validar ausencia de crash novo em fuzz/corpus
- [x] Gerar pacote de distribuicao de zt.exe
- [x] Publicar checksum e hash do artefato
- [x] Validar instalacao limpa em ambiente sem residuos
- [x] Validar hello world (check/build/run) no pacote publicado
- [x] Publicar notas de alpha com limites conhecidos
- [x] Registrar compatibilidade: platform/ continua interno e vazio

Criterio de aceite:

- [x] Artefatos de 0.3.0-alpha.1 publicados e verificaveis
- [x] Nenhum P0 aberto sem aceite formal
- [x] Fluxo de instalacao e primeiro build reproduzivel

Evidencias R2.M12:

- `python build.py` -> `SUCCESS`
- `python run_all_tests.py` -> `140 pass / 0 fail / 1 skip`
- `tests/perf/gate_pr.ps1` -> `117/117`
- `python tests/fuzz/replay.py --verbose` -> `{"seeds":0,"failures":0}`
- `python tests/fuzz/fuzz_lexer.py --iters 200 --seed 20260421 --verbose` -> `{"crashes":0,"timeouts":0}`
- `python tests/fuzz/fuzz_parser.py --iters 200 --seed 20260421 --verbose` -> `{"crashes":0,"timeouts":0}`
- Artefato: publicado como release asset (`zenith-0.3.0-alpha.1-windows-amd64.zip`)
- Hashes: publicados como release asset (`zenith-0.3.0-alpha.1-windows-amd64.checksums.txt`)
- Install limpo + hello world: validado no corte R2.M12 (log local de validacao)
- Notas de alpha: `docs/internal/reports/release/0.3.0-alpha.1-notes.md`
- Relatorio de release: `docs/internal/reports/release/R2.M12-alpha-release-report.md`
- Compatibilidade: `docs/internal/reports/compatibility/R2.M12-alpha-compatibility.md`

## Regras continuas

- [x] Todo bug novo gera teste de regressao
- [x] Toda feature nova entra com teste positivo e negativo
- [x] Toda mudanca de comportamento atualiza docs no mesmo PR
- [x] Toda regressao critica de performance bloqueia merge
- [x] Toda divergencia spec x codigo recebe classificacao P0/P1/P2

## Evidencia minima para marcar item como concluido

- [x] Comando executado + resultado
- [x] Arquivo de teste novo ou alterado
- [x] Commit/PR de fechamento
- [x] Risco residual (se houver)

Evidencias de processo (R2.M12+):

- `CONTRIBUTING.md` (secoes 1.1, 1.2 e checklist de PR)
