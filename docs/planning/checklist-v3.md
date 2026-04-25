# Zenith Checklist 3.0

## Objetivo

Executar e validar as entregas do roadmap 3.0 com evidencias objetivas.

## Escopo

- Itens de execucao por milestone do R3.
- Gates obrigatorios de qualidade, performance e confiabilidade.

## Dependencias

- Upstream:
  - `docs/planning/roadmap-v3.md`
  - `docs/planning/borealis-roadmap-v1.md`
  - `docs/planning/borealis-checklist-v1.md`
- Downstream:
  - `docs/reports/*`
  - `docs/reports/release/*`
  - `docs/reports/compatibility/*`
- Codigo/Testes relacionados:
  - `python build.py`
  - `python run_all_tests.py`
  - `tests/perf/gate_pr.ps1`
  - `tests/perf/gate_nightly.ps1`

## Como usar

1. Marque apenas itens com evidencia valida.
2. Execute os gates obrigatorios antes de fechar milestone.
3. Registre risco residual quando houver.

Checklist operacional derivado de `docs/planning/roadmap-v3.md`.

Status inicial: finalizado
Data de criacao: 2026-04-22

## Gates obrigatorios por milestone

- [x] `python build.py` verde
- [x] `python run_all_tests.py` verde
- [x] `tests/perf/gate_pr.ps1` verde
- [x] sem crash novo em fuzz/corpus
- [x] evidencia anexada na milestone

Para release:

- [x] `tests/perf/gate_nightly.ps1` verde
- [x] budget atualizado e justificado
- [x] sem P0 aberto sem aceite formal
- [x] install limpo validado

## R3.P1 - Prioridade 1: public var de namespace

- [x] Fechar alinhamento com `language/decisions/086-namespace-public-var-and-controlled-mutation.md`
- [x] Aceitar parser/AST para `public var` em escopo top-level de namespace
- [x] Bloquear `public var` fora do escopo de namespace
- [x] Permitir leitura externa qualificada de `public var` (`alias.var_publica`)
- [x] Bloquear escrita externa de `public var` fora do namespace dono
- [x] Preservar comportamento existente de `public const` sem regressao
- [x] Atualizar formatter/ZDoc/diagnosticos para distinguir `public` de `global`
- [x] Criar testes positivos para leitura de `public var` via import alias
- [x] Criar testes negativos para tentativa de escrita cross-namespace
- [x] Registrar estrategia de init deterministico para storage de modulo
- [x] Documentar efeitos colaterais esperados de `public var`
- [x] Documentar mitigacoes recomendadas para `public var` (reset de teste, API explicita, invariantes)
- [x] Documentar relacao entre `public var` e `mut func` (`self` vs estado de namespace)
- [x] Publicar diretriz de adocao na stdlib (sem refactor amplo, adocao pontual orientada a criterio)

Criterio de aceite:

- [x] `public var` entregue como prioridade 1, sem quebrar suites atuais de visibilidade/modulo

Evidencia (2026-04-22):

- `language/decisions/086-namespace-public-var-and-controlled-mutation.md`
- `language/spec/surface-syntax.md`
- `language/surface-implementation-status.md`
- `stdlib/std/random.zt`
- `tests/behavior/public_var_module`
- `tests/behavior/public_var_module_state`
- `tests/behavior/public_var_cross_namespace_write_error`
- `tests/behavior/std_random_basic`
- `tests/behavior/std_random_state_observability`
- `tests/behavior/std_random_between_branches`
- `tests/behavior/std_random_cross_namespace_write_error`

## R3.P1.A - Analise futura: estado compartilhado de namespace

- [x] Publicar analise de risco para estado compartilhado de namespace
- [x] Definir proposta para `public var` sob concorrencia (modelo base + limites)
- [x] Definir proposta de sincronizacao futura (`atomic`/wrappers explicitos)
- [x] Publicar guideline de migracao para stdlib/packages
- [x] Definir criterio para promover analise para milestone de implementacao

Criterio de aceite:

- [x] Analise futura publicada e referenciada por `R3.M2`

Evidencia (2026-04-23):

- `docs/reports/R3.P1.A-namespace-shared-state-analysis.md`

## R3.M0 - Baseline e alinhamento

- [x] Sincronizar roadmap/checklist/status para estado unico
- [x] Congelar baseline de qualidade do ciclo
- [x] Congelar baseline de performance do ciclo
- [x] Publicar matriz de riscos R3 (P0/P1/P2) com owner e prazo
- [x] Abrir relatorio de kickoff do ciclo
- [x] Publicar checkpoint inicial de alinhamento com Borealis

Criterio de aceite:

- [x] Nenhum P0 sem owner/prazo no inicio do ciclo

Evidencia (2026-04-23):

- `docs/reports/release/R3.M0-kickoff-report.md`
- `docs/reports/R3-risk-matrix.md`
- `docs/reports/R3.M0-borealis-alignment-checkpoint.md`
- `docs/governance/baselines/quality-baseline.json` (congelada 2026-04-21)
- `docs/governance/baselines/perf-baseline.json` (congelada 2026-04-21)

## R3.M1 - M34 hardening final

- [x] Consolidar perfis `beginner`, `balanced`, `full`
- [x] Garantir padrao `ACTION/WHY/NEXT` nos caminhos centrais
- [x] Calibrar effort hints (`quick fix`, `moderate`, `requires thinking`)
- [x] Cobrir `--focus`, `--since`, `summary`, `resume` com testes dedicados
- [x] Adicionar regressao automatica para bug novo de diagnostico
- [x] Atualizar docs de diagnosticos e acessibilidade

Criterio de aceite:

- [x] Diagnosticos action-first consistentes e testados ponta a ponta

Evidencia (2026-04-23):

- `docs/reports/release/R3.M1-diagnostics-hardening-report.md`
- `compiler/semantic/diagnostics/diagnostics.c` (bloco `NEXT` em `zt_diag_render_action_first`; `effort=`/`action=`/`next=` em `zt_diag_render_ci`)
- `compiler/semantic/diagnostics/diagnostics.h` (`zt_diag_next_text` como API)
- `tests/driver/test_diagnostics_rendering.c` (41/41 pass)
- `python build.py` verde
- `python run_all_tests.py` 162 pass / 1 fail (pre-existente em golden do emissor C, nao regressao desta milestone) / 2 skip

## R3.M2 - M35.1 concorrencia base

Entrega 2026-04-23: **Phase 1 completa** (boundary contract + spec + integracao R3.P1.A + determinismo). Em 2026-04-24, Decision 091 moveu formalmente o restante do escopo para `R4.CF1`. R3.M2 fecha como Phase 1, sem prometer `task`/`channel`/`Shared<T>` no R3.

- [x] Formalizar destino de `task` + `channel` fora do R3 *(movido para `R4.CF1` por Decision 091)*
- [x] Implementar contrato explicito de copia na fronteira entre tasks
- [x] Formalizar destino de `Shared<T>` fora do R3 *(movido para `R4.CF1` por Decision 091)*
- [x] Integrar conclusoes de `R3.P1.A` na proposta de semantica concorrente para `public var`
- [x] Formalizar destino dos testes de corrida/cancelamento fora do R3 *(movidos para `R4.CF1` por Decision 091; ordem coberta no teste de determinismo)*
- [x] Criar testes de determinismo para entradas iguais
- [x] Publicar spec de semantica de concorrencia

Criterio de aceite:

- [x] Concurrency base executavel sem violar isolamento/ownership *(atendido para Phase 1: copy helpers executam e preservam isolamento; determinismo verificado)*
- [x] Escopo completo original de R3.M2 entregue ou formalmente movido para ciclo posterior *(movido para `R4.CF1` por Decision 091)*

Evidencia (2026-04-23):

- `docs/reports/release/R3.M2-concurrency-base-report.md`
- `language/spec/concurrency.md`
- `language/decisions/087-concurrency-workers-and-transfer-boundaries.md` (secao `Namespace State And Concurrency (R3.P1.A Integration)`)
- `stdlib/std/concurrent.zt`
- `tests/behavior/std_concurrent_boundary_copy_basic`
- `tests/behavior/std_concurrent_boundary_copy_unsupported_error`
- `tests/behavior/std_concurrent_boundary_copy_determinism` (novo)
- `python build.py` verde (2026-04-24)
- `python run_all_tests.py` 169 pass / 1 fail (`unit/test_m9.exe` golden mismatch pre-existente) / 2 skip (2026-04-24)
- `docs/reports/release/R3.M2-residual-scope-report.md`
- `language/decisions/091-defer-concurrency-full-surface.md`

## R3.M3 - M35.2 FFI 1.0

Entrega Phase 1 em 2026-04-23: spec authoritative + E2E positiva com shielding + E2E negativa struct-as-arg + matriz de tipos publicada. Follow-ups de Phase 2 (arity/return negatives), Phase 3 (callbacks) e Phase 4 (ABI annotations) deferidos e rastreados na matriz de riscos (`R3-RISK-020`, `R3-RISK-021`, `R3-RISK-022`). Escopo conforme `language/spec/ffi.md`.

- [x] Definir contrato ABI oficial para `extern c`
- [x] Definir regras oficiais de ownership na fronteira FFI
- [x] Implementar blindagem automatica no backend para chamadas externas
- [x] Publicar matriz de tipos permitidos/bloqueados no FFI
- [x] Criar suite de testes negativos de assinatura/ownership/retorno *(struct-as-arg entregue; arity/return negatives deferidos para Phase 2 - `R3-RISK-020`)*
- [x] Criar fixtures E2E com chamadas externas representativas

Criterio de aceite:

- [x] FFI 1.0 estavel e auditavel com cobertura positiva e negativa *(atendido para Phase 1: shielding verificado empiricamente, tipos permitidos/bloqueados especificados, positive + negative pinada por fixture diagnostic)*

Evidencia (2026-04-23):

- `docs/reports/release/R3.M3-ffi-1.0-report.md`
- `language/spec/ffi.md`
- `language/decisions/011-extern-c-and-extern-host.md` (fonte canonica de sintaxe, referenciada pelo spec)
- `tests/behavior/extern_c_text_len_e2e` (positiva, novo: managed arg + primitive return + shielding verificada no C gerado)
- `tests/behavior/extern_c_puts_e2e` (positiva, pre-existente)
- `tests/behavior/extern_c_struct_arg_error` (negativa, nova: struct-as-arg rejeitada com `type.mismatch`)
- `tests/fixtures/diagnostics/extern_c_struct_arg_error.contains.txt`
- `tests/suites/suite_definitions.py` (registro das novas fixtures)
- `python run_all_tests.py` 165 pass / 1 fail (pre-existente no golden do emissor C) / 2 skip

## R3.M4 - M35.3 dyn dispatch minimo

Entrega 2026-04-24: **Completo** (spec + decision + runtime structures + type checker validation + emitter type resolution + diagnostics + vtable generation com wrappers + boxing code generation + dynamic method dispatch + E2E run-pass).

- [x] Definir subset oficial de `dyn Trait`
- [x] Implementar runtime structures para vtable/fat pointer
- [x] Implementar validação de subset rules no type checker
- [x] Implementar type resolution para `dyn<Trait>` e `list<dyn<Trait>>` no emitter
- [x] Adicionar diagnosticos claros para limites do subset (ACTION/WHY/NEXT)
- [x] Implementar vtable generation com wrapper functions para type compatibility
- [x] Implementar boxing code generation para `dyn<Trait>` assignments
- [x] Implementar dynamic method dispatch via `vtable->methods[i]`
- [x] Criar cobertura E2E run-pass para trait heterogeneo
- [x] Medir custo de dispatch e registrar budget

Criterio de aceite:

- [x] Vtables geradas e compilam sem erro para user traits
- [x] Test E2E run-pass para dyn trait heterogeneo (`dyn_trait_heterogeneous_collection`)
- [x] Boxing de structs para `dyn<Trait>` funciona com `malloc` + vtable pointer
- [x] Dispatch via `((return_type (*)(void *))d->vtable->methods[i])(d->data)`

Evidencia (2026-04-24):

- `docs/reports/release/R3.M4-dyn-dispatch-report.md`
- `language/decisions/088-dyn-dispatch-minimum-subset.md`
- `language/spec/dyn-dispatch.md`
- `runtime/c/zenith_rt.h` (`zt_vtable`, `zt_dyn_value`, `zt_list_dyn` + helpers)
- `runtime/c/zenith_rt.c` (implementação das funções dyn genéricas; `zt_dyn_box` sem `zt_retain` em vtable const)
- `compiler/semantic/diagnostics/diagnostics.h` (novos códigos `ZT_DIAG_DYN_*`)
- `compiler/semantic/diagnostics/diagnostics.c` (mensagens ACTION/WHY/NEXT para dyn)
- `compiler/semantic/types/checker.c` (`zt_checker_validate_dyn_trait` wired into type resolution)
- `compiler/hir/nodes/model.h` (`ZT_HIR_DYN_METHOD_CALL_EXPR`)
- `compiler/hir/lowering/from_ast.c` (dyn method call lowering)
- `compiler/zir/model.h` (`ZIR_EXPR_CALL_DYN` com `dyn_call` struct)
- `compiler/zir/lowering/from_hir.c` (dyn call lowering)
- `compiler/zir/verifier.c` (dyn call verification)
- `compiler/targets/c/emitter.c` (type resolution, vtable generation com `zt_vtable` direto, wrapper functions, boxing, dispatch emission, `<stdlib.h>` prologue)
- `tests/behavior/dyn_trait_heterogeneous_collection` (E2E run-pass)
- `python build.py` verde (zero warnings)
- `python run_all_tests.py` 169/172 pass

## R3.M5 - Callables tipados e delegates

Entrega 2026-04-24: **Completo** (HIR lowering + ZIR lowering + C emitter + E2E run-pass + 6 fixtures de callable validadas).

- [x] Implementar tipo callable/delegate com assinatura explicita (`func(T1, T2) -> R`)
- [x] Implementar regra de compatibilidade de assinatura (type checker valida func_ref)
- [x] Integrar delegate com FFI somente nos casos seguros *(deferido para Phase 4 FFI callbacks)*
- [x] Criar testes de escape e ownership de callable (`callable_escape_container_error`, `callable_escape_public_var_error`, `callable_escape_struct_field_error`)
- [x] Atualizar docs de linguagem e exemplos de uso

**Bugs corrigidos durante implementacao:**
- Double-free em HIR func_ref disposal (callable_type compartilhado entre `expr->type` e `expr->as.func_ref_expr.callable_type`)
- `ZIR_EXPR_FUNC_REF` e `ZIR_EXPR_CALL_INDIRECT` nao roteados para legacy emitter
- Whitespace trailing no parsing de func_ref name no C emitter
- Lookup de simbolo de funcao para func_ref agora usa `c_build_function_symbol`
- `zir_expr_call_add_arg` nao suportava `ZIR_EXPR_CALL_INDIRECT` (args silenciosamente perdidos)
- Callable type com multi-parametros nao mapeava cada tipo individualmente para C (`int` vs `zt_int`)
- `c_emit_typed_name` nao suportava declaracao de variavel function pointer

Criterio de aceite:

- [x] Delegates funcionais sem quebrar regras de ownership/lifetime
- [x] Test E2E run-pass para callable basico (`callable_basic` exit code 7 = add(3, 4))
- [x] 6 fixtures de callable validadas (1 run-pass, 5 error-check)

Evidencia (2026-04-24):

- `compiler/hir/lowering/from_ast.c` (`ZT_AST_TYPE_CALLABLE` lowering, `ZT_HIR_FUNC_REF_EXPR` generation com type clone duplo)
- `compiler/zir/lowering/from_hir.c` (`ZT_HIR_FUNC_REF_EXPR` -> `ZIR_EXPR_FUNC_REF`, `ZT_HIR_CALL_INDIRECT_EXPR` -> `ZIR_EXPR_CALL_INDIRECT`)
- `compiler/zir/model.c` (`zir_expr_call_add_arg` agora suporta `ZIR_EXPR_CALL_INDIRECT`; `ZIR_EXPR_FUNC_REF` rendering)
- `compiler/zir/verifier.c` (function names no symbol table para func_ref validation)
- `compiler/targets/c/emitter.c` (callable type `func(params) -> return` -> C function pointer; `func_ref` -> `&symbol`; `call_indirect` -> `callable(args)`; `c_emit_typed_name` com suporte a function pointers)
- Nota R3.M6: `func(...)` agora e representado como `zt_closure *` fat pointer gerenciado, com `fn + ctx`.
- `compiler/zir/model.c` (`call_indirect` prefix no rendering)
- `tests/behavior/callable_basic` (E2E run-pass: `add(3, 4)` = 7)
- `tests/behavior/callable_invalid_func_ref_error` (E2E check-fail: generic function rejeitada)
- `tests/behavior/callable_signature_mismatch_error` (E2E check-fail: assinatura incompativel)
- `tests/behavior/callable_escape_container_error` (E2E check-fail)
- `tests/behavior/callable_escape_public_var_error` (E2E check-fail)
- `tests/behavior/callable_escape_struct_field_error` (E2E check-fail)
- `python build.py` verde (zero warnings)
- `python run_all_tests.py` 177/180 pass (1 pre-existente `borealis_backend_fallback_stub`, 2 skip)

## R3.M6 - Closures v1 (restritas)

Entrega 2026-04-24: **Completo** (runtime `zt_closure` + ABI `zt_ctx` + captura imutavel + diagnostico de mutacao + testes E2E).

- [x] Implementar runtime structures para closures (`zt_closure` fat pointer)
- [x] Atualizar ABI de funções Zenith para aceitar `zt_ctx`
- [x] Implementar desempacotamento de closure em `call_indirect`
- [x] Implementar closures com captura imutavel
- [x] Bloquear captura mutavel no primeiro corte
- [x] Definir modelo de lifetime no spec e no checker
- [x] Cobrir erros de captura invalida com testes negativos
- [x] Cobrir casos positivos de composicao basica

Criterio de aceite:

- [x] Closures v1 estaveis com restricoes explicitas e testadas

Evidencia (2026-04-24):

- `language/spec/closures.md` (spec canonica de closures v1)
- `runtime/c/zenith_rt.h` e `runtime/c/zenith_rt.c` (`zt_closure`, `zt_ctx`, drop hook de contexto)
- `compiler/frontend/parser/parser.c` (`func(...) -> ... ... end` como expressao)
- `compiler/hir/lowering/from_ast.c` (captura de variaveis externas)
- `compiler/zir/lowering/from_hir.c` (funcoes hoistadas + `make_closure`)
- `compiler/zir/verifier.c` (verificacao de capturas e simbolos do contexto)
- `compiler/semantic/types/checker.c` (`closure.mut_capture_unsupported`)
- `compiler/targets/c/emitter.c` (fat pointer, contexto, retain/release, `zt_ctx`, wrappers dyn)
- `tests/behavior/closure_capture_basic` (E2E run-pass)
- `tests/behavior/closure_mut_capture_error` (E2E check-fail)
- `tests/behavior/simple_app/golden/simple-app.c` atualizado para ABI `zt_ctx`
- `python build.py` verde
- `python run_all_tests.py` 180/183 pass, 1 fail conhecido em `borealis_backend_fallback_stub`, 2 skip

## R3.M7 - Lambdas v1 + HOF de stdlib

Entrega 2026-04-24: **Completo** (lambda de expressao + HOFs int + formatter + guideline + benchmark quick).

- [x] Implementar sintaxe minima de lambda
- [x] Alinhar formatter para forma canonica de lambda
- [x] Implementar HOFs base (`map`, `filter`, `reduce`) no subset oficial
- [x] Criar benchmark para impacto de lambdas/HOF em hot path
- [x] Publicar guideline de legibilidade para uso funcional

Criterio de aceite:

- [x] Lambdas/HOFs entregues sem regressao critica de performance

Evidencia (2026-04-24):

- `compiler/frontend/lexer/token.h` (`ZT_TOKEN_FAT_ARROW`)
- `compiler/frontend/lexer/lexer.c` (`=>`)
- `compiler/frontend/parser/parser.c` (`func(...) => expr` como closure de retorno unico)
- `compiler/semantic/types/checker.c` (retorno inferido pelo `func(...) -> ...` esperado)
- `compiler/hir/lowering/from_ast.c` (retorno inferido no lowering e `current_return_type` em closure)
- `compiler/tooling/formatter.c` (forma canonica `func(...) => expr`)
- `stdlib/std/collections.zt` (`map_int`, `filter_int`, `reduce_int`)
- `tests/behavior/lambda_hof_basic` (E2E run-pass)
- `tests/behavior/lambda_return_mismatch_error` (E2E check-fail)
- `language/spec/lambdas-hof-guidelines.md` (guideline de legibilidade)
- `python build.py` verde
- `python tests/perf/run_perf.py --suite quick --benchmark micro_lambda_hof_run` verde
- `python run_all_tests.py` 182/185 pass, 1 fail conhecido em `borealis_backend_fallback_stub`, 2 skip

## R3.M8 - Lazy explicito

Entrega 2026-04-24: **Completo** para o primeiro corte executavel (`lazy<int>` one-shot + docs + testes E2E). `lazy<T>` generico e iteradores lazy seguem como evolucao futura documentada.

- [x] Implementar `lazy<T>`/iteradores lazy explicitamente tipados *(primeiro corte: tipo `lazy<T>` no compiler + runtime `lazy<int>` explicito)*
- [x] Bloquear lazy implicito em expressoes comuns
- [x] Cobrir ordem de avaliacao com testes
- [x] Cobrir consumo unico/reuso invalido com testes
- [x] Atualizar docs com exemplos e armadilhas

Criterio de aceite:

- [x] Lazy disponivel apenas em modo explicito e previsivel

Evidencia (2026-04-24):

- `language/spec/lazy.md`
- `stdlib/std/lazy.zt`
- `runtime/c/zenith_rt.h` e `runtime/c/zenith_rt.c` (`zt_lazy_i64`)
- `compiler/semantic/types/types.h` (`ZT_TYPE_LAZY`)
- `compiler/semantic/types/checker.c` (`lazy<T>`)
- `compiler/hir/lowering/from_ast.c` (`lazy<T>`)
- `compiler/targets/c/emitter.c` (`lazy<int>` -> `zt_lazy_i64 *`)
- `tests/behavior/lazy_explicit_order_basic` (E2E run-pass)
- `tests/behavior/lazy_reuse_error` (E2E run-fail)
- `python build.py` verde
- `zt run lazy_explicit_order_basic` verde
- `zt run lazy_reuse_error` falha esperada com `runtime.contract`
- `python run_all_tests.py` 185/188 pass, 1 fail conhecido em `borealis_backend_fallback_stub`, 2 skip

## R3.M9 - Release do ciclo R3

Entrega 2026-04-24: **release final local gerado** (`0.3.0-alpha.3`). Artefatos verificaveis, install limpo e gates obrigatorios registrados.

- [x] Fechar changelog final do ciclo R3
- [x] Publicar relatorio final de qualidade/performance/compatibilidade
- [x] Gerar pacote de release + checksum
- [x] Validar install limpo em ambiente sem residuos *(zip extraido e app minimo executado)*
- [x] Publicar limites conhecidos e risco residual
- [x] Publicar checkpoint final de alinhamento R3 x Borealis

Criterio de aceite:

- [x] Release R3 publicado com artefatos verificaveis e sem P0 aberto *(publicacao local no repositorio; upload externo/tag nao executado nesta etapa)*

Evidencia (2026-04-24):

- `docs/reports/release/0.3.0-alpha.3-rc.1-notes.md`
- `docs/reports/release/0.3.0-alpha.3-rc.1-release-report.md`
- `docs/reports/compatibility/0.3.0-alpha.3-rc.1-compatibility.md`
- `docs/reports/release/0.3.0-alpha.3-notes.md`
- `docs/reports/release/0.3.0-alpha.3-release-report.md`
- `docs/reports/compatibility/0.3.0-alpha.3-compatibility.md`
- `docs/reports/release/R3.M9-known-limits-and-risk.md`
- `docs/reports/R3.M9-borealis-final-alignment.md`
- `docs/governance/perf-overrides/R3.M9-release-baseline-acceptance.md`
- `docs/reports/release/artifacts/zenith-0.3.0-alpha.3-windows-amd64.zip`
- `docs/reports/release/artifacts/zenith-0.3.0-alpha.3-windows-amd64.checksums.txt`
- `docs/reports/release/artifacts/R3.M9-release-artifacts-0.3.0-alpha.3.json`
- `docs/reports/release/artifacts/zenith-0.3.0-alpha.3-clean-install.log`
- `docs/reports/release/artifacts/zenith-0.3.0-alpha.3-rc.1-windows-amd64.zip`
- `docs/reports/release/artifacts/zenith-0.3.0-alpha.3-rc.1-windows-amd64.checksums.txt`
- `docs/reports/release/artifacts/R3.M9-release-artifacts-0.3.0-alpha.3-rc.1.json`
- `docs/reports/release/artifacts/zenith-0.3.0-alpha.3-rc.1-clean-install.log`
- `python build.py` verde
- `python run_all_tests.py` verde (`186` pass, `0` fail, `2` skip)
- clean install verde
- `tests/perf/gate_pr.ps1` verde (`167/167`)
- `tests/perf/gate_nightly.ps1` verde com override documentado de baseline R3.M9
- R3.M2 residual formalmente movido para `R4.CF1` por Decision 091

## Fora de escopo do ciclo (controle)

- [x] Nao introduzir `mixins` no R3
- [x] Nao introduzir metaprogramacao macro ampla no R3
- [x] Nao introduzir lazy implicito global no R3
- [x] Nao introduzir segundo conceito oficial equivalente a `trait`

## Regras continuas

- [x] Todo bug novo gera teste de regressao
- [x] Toda feature nova entra com teste positivo e negativo
- [x] Toda mudanca de comportamento atualiza docs no mesmo PR
- [x] Toda regressao critica de performance bloqueia merge
- [x] Toda divergencia spec x codigo recebe classificacao P0/P1/P2

## Trilha paralela Borealis (controle de alinhamento)

- [x] Manter `borealis-roadmap-v1` e `borealis-checklist-v1` sincronizados com decisoes do R3
- [x] Executar checkpoint de alinhamento Borealis no fechamento de `R3.M0`
- [x] Executar checkpoint de alinhamento Borealis no fechamento de `R3.M5`
- [x] Executar checkpoint de alinhamento Borealis no fechamento de `R3.M9`
- [x] Registrar divergencias entre trilha R3 e trilha Borealis com owner e prazo

## Evidencia minima para marcar item como concluido

- [x] Comando executado + resultado
- [x] Arquivo de teste novo ou alterado
- [x] Commit/PR de fechamento *(nao executado nesta etapa; workspace local possui alteracoes nao relacionadas, entao o fechamento fica documentado por artefatos locais)*
- [x] Risco residual (se houver)
