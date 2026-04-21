# Zenith Roadmap 2.0

## Objetivo

Definir a estrategia do ciclo 2.0 com equilibrio entre qualidade, performance e novas features.

## Escopo

- Planejamento de curto e medio prazo (R2.M0 ate R2.M12).
- Integracao entre spec, codigo, testes e medicao de performance.

## Dependencias

- Upstream:
  - `docs/planning/checklist-v1.md`
  - `language/spec/README.md`
- Downstream:
  - `docs/planning/checklist-v2.md`
  - `docs/planning/cascade-v2.md`
  - `docs/reports/*`
- Codigo/Testes relacionados:
  - `run_all_tests.py`
  - `tests/perf/run_perf.py`

## Como usar

1. Leia os objetivos e fases.
2. Abra o checklist v2 da milestone alvo.
3. Execute pelo cascade v2 para chegar em arquivos e testes.

Status: proposto
Data: 2026-04-21
Base: `docs/planning/checklist-v1.md` fechado (M0-M38)

## Leitura rapida

Objetivo do ciclo 2.0:

- manter estabilidade alta;
- acelerar deteccao de bugs;
- melhorar performance de compilador e runtime com medicao;
- liberar novas features sem quebrar o pipeline.

Modelo inicial de capacidade:

- 45% qualidade e hardening;
- 30% performance e otimizacao;
- 25% novas features.

## Documentos conectados (navegacao para LLMs)

Fluxo recomendado:

1. `docs/planning/roadmap-v2.md`
2. `docs/planning/checklist-v2.md`
3. `docs/planning/cascade-v2.md`
4. `docs/planning/cascade-v1.md` (contexto historico M0-M38)
5. `compiler/CODE_MAP.md`
6. `*_MAP.md` da area alvo
7. codigo
8. testes da area

Documentos centrais de referencia:

- `docs/planning/checklist-v2.md`
- `docs/planning/cascade-v2.md`
- `docs/planning/cascade-v1.md`
- `compiler/CODE_MAP.md`
- `language/spec/compiler-model.md`
- `language/spec/surface-syntax.md`
- `language/spec/conformance-matrix.md`
- `language/spec/formatter-model.md`
- `language/spec/implementation-status.md`
- `tests/behavior/README.md`
- `tests/behavior/MATRIX.md`
- `tests/perf/README.md`

Maps por camada:

- Frontend: `compiler/frontend/lexer/LEXER_MAP.md`, `compiler/frontend/parser/PARSER_MAP.md`, `compiler/frontend/ast/AST_MAP.md`
- Semantic: `compiler/semantic/binder/BINDER_MAP.md`, `compiler/semantic/types/TYPE_SYSTEM_MAP.md`, `compiler/semantic/diagnostics/DIAGNOSTICS_MAP.md`
- IR: `compiler/hir/lowering/HIR_LOWERING_MAP.md`, `compiler/zir/ZIR_MODEL_MAP.md`, `compiler/zir/ZIR_PARSER_MAP.md`, `compiler/zir/ZIR_VERIFIER_MAP.md`
- Backend/Runtime: `compiler/targets/c/EMITTER_MAP.md`, `runtime/c/RUNTIME_MAP.md`
- Tooling/Project: `compiler/tooling/FORMATTER_MAP.md`, `compiler/driver/DRIVER_MAP.md`, `compiler/project/PROJECT_MAP.md`, `stdlib/STDLIB_MAP.md`

## Matriz de navegação por tipo de trabalho

### 1) Bug de lexer/parser/AST

- Ler: `language/spec/surface-syntax.md`
- Maps: `LEXER_MAP.md`, `PARSER_MAP.md`, `AST_MAP.md`
- Arquivos de entrada:
  - `compiler/frontend/lexer/lexer.c`
  - `compiler/frontend/parser/parser.c`
  - `compiler/frontend/ast/model.h`
- Testes alvo:
  - `tests/frontend/test_lexer.c`
  - `tests/frontend/test_parser.c`
  - `tests/frontend/test_parser_error_recovery.c`

### 2) Bug semantico e diagnostico

- Ler: `language/spec/surface-syntax.md`, `language/spec/implementation-status.md`
- Maps: `BINDER_MAP.md`, `TYPE_SYSTEM_MAP.md`, `DIAGNOSTICS_MAP.md`
- Arquivos de entrada:
  - `compiler/semantic/binder/binder.c`
  - `compiler/semantic/types/checker.c`
  - `compiler/semantic/diagnostics/diagnostics.c`
- Testes alvo:
  - `tests/semantic/test_binder.c`
  - `tests/semantic/test_types.c`
  - `tests/semantic/test_constraints.c`

### 3) Bug de lowering HIR/ZIR

- Ler: `language/spec/compiler-model.md`
- Maps: `HIR_LOWERING_MAP.md`, `ZIR_MODEL_MAP.md`, `ZIR_VERIFIER_MAP.md`
- Arquivos de entrada:
  - `compiler/hir/lowering/from_ast.c`
  - `compiler/zir/lowering/from_hir.c`
  - `compiler/zir/verifier.c`
- Testes alvo:
  - `tests/semantic/test_hir_lowering.c`
  - `tests/zir/test_lowering.c`
  - `tests/zir/test_verifier.c`

### 4) Bug de backend C/runtime

- Ler: `language/spec/compiler-model.md`, `language/spec/conformance-matrix.md`
- Maps: `EMITTER_MAP.md`, `RUNTIME_MAP.md`
- Arquivos de entrada:
  - `compiler/targets/c/emitter.c`
  - `compiler/targets/c/legalization.c`
  - `runtime/c/zenith_rt.c`
- Testes alvo:
  - `tests/targets/c/test_emitter.c`
  - `tests/targets/c/test_legalization.c`
  - `tests/runtime/c/test_shared_text.c`
  - `tests/behavior/*`

### 5) Bug de formatter/tooling/doc

- Ler: `language/spec/formatter-model.md`
- Maps: `FORMATTER_MAP.md`, `DRIVER_MAP.md`, `PROJECT_MAP.md`
- Arquivos de entrada:
  - `compiler/tooling/formatter.c`
  - `compiler/driver/main.c`
  - `compiler/project/zdoc.c`
- Testes alvo:
  - `tests/formatter/run_formatter_golden.py`
  - `tests/conformance/test_m9.c`

### 6) Performance do compilador

- Ler: `tests/perf/README.md`, `language/spec/conformance-matrix.md`
- Maps: `DRIVER_MAP.md`, `PARSER_MAP.md`, `HIR_LOWERING_MAP.md`, `EMITTER_MAP.md`
- Arquivos de entrada:
  - `compiler/driver/main.c`
  - `compiler/frontend/parser/parser.c`
  - `compiler/hir/lowering/from_ast.c`
  - `compiler/targets/c/emitter.c`
- Suites alvo:
  - `tests/perf/gate_pr.ps1`
  - `tests/perf/gate_nightly.ps1`
  - `tests/perf/run_perf.py`

### 7) Performance do runtime

- Ler: `tests/perf/README.md`
- Maps: `RUNTIME_MAP.md`
- Arquivos de entrada:
  - `runtime/c/zenith_rt.c`
  - `runtime/c/zenith_rt.h`
- Suites alvo:
  - `tests/perf/gate_pr.ps1`
  - `tests/perf/gate_nightly.ps1`
  - `tests/runtime/c/*`

### 8) Drift de core implicito e prelude

- Ler: `language/spec/surface-syntax.md`, `language/decisions/040-equality-hashing-and-ordering.md`, `language/decisions/050-core-stdlib-boundary.md`
- Maps: `BINDER_MAP.md`, `TYPE_SYSTEM_MAP.md`, `HIR_LOWERING_MAP.md`, `EMITTER_MAP.md`
- Arquivos de entrada:
  - `compiler/semantic/binder/binder.c`
  - `compiler/semantic/types/checker.c`
  - `compiler/hir/lowering/from_ast.c`
  - `compiler/targets/c/emitter.c`
  - `language/spec/implementation-status.md`
- Testes alvo:
  - `tests/semantic/test_binder.c`
  - `tests/semantic/test_types.c`
  - `tests/semantic/test_constraints.c`
  - `tests/behavior/*`
  - `tests/conformance/*`

## Gates obrigatorios do ciclo

Nenhuma milestone avanca sem:

1. `python build.py` verde
2. `python run_all_tests.py` verde
3. `tests/perf/gate_pr.ps1` verde
4. sem crash novo em fuzz/corpus
5. checklist da milestone com evidencia

Para fechamento de release:

1. `tests/perf/gate_nightly.ps1` verde
2. sem regressao acima dos budgets acordados
3. riscos P0 fechados ou aceitos formalmente

## Fases do Roadmap 2.0

## R2.PRE - Refatoracao de Conformidade da Stdlib (MVP)

Objetivo:
- Sanar discrepancias criticas entre os arquivos `.zt` (interfaces fisicas) e as definicoes arquiteturais dos Decisions (`058` ao `069`), garantindo que o escopo de MVP seja estritamente respeitado antes de features ou mudancas radicais.

Entregas:
- Modulos `std.io`, `std.format`, `std.math`, `std.time`, `std.validate`, `std.fs`, `std.os`, `std.json` contendo as funcoes reais e tipadas estipuladas.
- Assinaturas de C (`extern c`) integradas ou marcadas.
- Cobertura estrita ao padrao esperado e ZDocs em conformidade (`zt doc check`).

## R2.M0 - Baseline e governanca

Objetivo:

- organizar processo unico de execucao, triagem e fechamento.

Entregas:

- baseline de qualidade atual;
- baseline de performance atual;
- owners por area;
- SLA por severidade;
- template padrao de bug e regressao.

## R2.M1 - Infra de testes 2.0

Objetivo:

- aumentar cobertura com operacao simples.

Entregas:

- suites `smoke`, `pr_gate`, `nightly`, `stress`;
- relatorio por camada;
- artifacts de falha;
- seeds reproduziveis.

## R2.M2 - Robustez de frontend (fuzzing)

Objetivo:

- reduzir crash e loop em lexer/parser.

Entregas:

- harness de fuzz;
- corpus inicial;
- minimizacao de casos quebrados;
- suite de replay de seeds.

## R2.M3 - Hardening semantico e propriedades

Objetivo:

- reduzir bugs semanticos silenciosos.

Entregas:

- property tests;
- matriz de negativos por `ZT_DIAG_*`;
- spans e mensagens validados;
- regressao permanente para bugs criticos.

## R2.M4 - Observabilidade de performance

Objetivo:

- medir certo antes de otimizar.

Entregas:

- baseline de tempo/memoria/binario;
- top hotspots do compilador;
- top hotspots do runtime;
- budgets por benchmark atualizados.

## R2.M5 - Otimizacao do compilador

Objetivo:

- reduzir tempo de `check/build/test` e uso de memoria.

Entregas:

- otimizar hot paths do parser/lowering/emitter/driver;
- reduzir trabalho redundante no pipeline;
- garantir sem regressao funcional;
- registrar ganho por benchmark.

## R2.M6 - Otimizacao do runtime

Objetivo:

- reduzir custo em caminhos quentes de execucao.

Entregas:

- otimizar `text`, `collections`, `optional/result`;
- otimizar caminhos de retain/release frequentes;
- validar estabilidade de memoria no gate longo;
- registrar ganho por benchmark.

## R2.M7 - Auditoria spec x implementacao

Objetivo:

- remover drift entre documentos e comportamento real.

Entregas:

- matriz `Spec/Parsed/Semantic/Lowered/Emitted/Executable`;
- classificacao P0/P1/P2 das divergencias;
- plano de fechamento com owner e prazo.

Correcoes confirmadas para entrar na auditoria (abrir ticket com severidade):

- P0: resolver contrato final de `check(...)` e `panic(...)` no core implicito (implementar ou ajustar spec/docs sem ambiguidade);
- P0: incluir `Comparable` (e onde aplicavel `Order`) no prelude implicito e no checker;
- P0: alinhar `< <= > >=` para regra baseada em trait (`Comparable`) em vez de regra hardcoded de tipo;
- P0: corrigir construcao de `core.Error` por caminho qualificado (sem depender de workaround);
- P1: alinhar nomes canonicos de inteiros sem sinal (`u8/u16/u32/u64`) e estrategia de compatibilidade com `uint8/uint16/uint32/uint64`;
- P1: corrigir drift documental de snapshot em `language/spec/implementation-status.md` (referencia para arquivo inexistente).

## R2.M8 - Fechar deferreds P0

Objetivo:

- fechar pendencias que quebram coerencia de ponta a ponta.

Escopo inicial:

- Core/prelude P0:
  - fechar `check/panic` ponta a ponta (parser/semantic/lowering/backend/docs);
  - fechar `Comparable/Order` no binder + checker + regras de operadores relacionais;
  - fechar construcao qualificada de `core.Error` com testes de comportamento;
- `fmt` v2 completo: parser, HIR, ZIR, emitter, runtime, formatter e docs;
- demais deferreds P0 encontrados na auditoria.

## R2.M9 - Feature Pack A (linguagem)

Objetivo:

- evoluir sintaxe/semantica com risco controlado.

Regra:

- toda feature entra com design curto, testes e criterio de rollback.

## R2.M10 - Feature Pack B (stdlib e tooling)

Objetivo:

- evoluir experiencia pratica de uso e contribuicao.

Regra:

- preservar compatibilidade e gates de qualidade/performance.

## R2.M11 - Release candidate 2.0

Objetivo:

- consolidar corte estavel de release.

Entregas:

- freeze de features;
- foco em bugfix;
- bateria completa de suites;
- relatorio final de qualidade, performance e riscos.

## R2.M12 - Alpha pre-release (0.3.0-alpha.1)

Objetivo:

- publicar primeira pre-release externa com escopo controlado e repetivel.

Versao proposta:

- 0.3.0-alpha.1

Escopo minimo:

- P0 de coerencia fechados (check/panic, Comparable/Order, operadores relacionais por trait, core.Error, alinhamento u8..u64);
- gates obrigatorios verdes (python build.py, python run_all_tests.py, tests/perf/gate_pr.ps1, sem crash novo em fuzz/corpus);
- pacote de distribuicao do zt.exe com checksum e teste de instalacao limpa;
- docs de alpha publicadas (instalacao, uso rapido, limites conhecidos, politica de compatibilidade pre-1.0).

Fora de escopo da alpha:

- stdlib/platform/ continua placeholder interno;
- LSP/Compass nao bloqueia a publicacao da alpha;
- features grandes novas ficam para alpha.2+.

Criterio de aceite:

- release 0.3.0-alpha.1 publicada com artefatos verificaveis;
- nenhum P0 aberto sem aceite formal de risco;
- exemplo minimo (hello world) compila e executa em ambiente limpo.

## Definicao de pronto por feature

Uma feature so e considerada pronta quando:

1. spec atualizada;
2. parser/semantic/lowering/backend/runtime alinhados;
3. formatter/docs alinhados;
4. testes positivos e negativos;
5. benchmark relevante sem regressao acima do budget;
6. checklist atualizado com evidencia.

## Indicadores de sucesso

- queda de bugs reabertos por regressao;
- queda de tempo medio de investigacao;
- reducao do tempo de compilacao e do consumo de memoria;
- aumento de cobertura em areas criticas;
- features entregues com gates verdes.
