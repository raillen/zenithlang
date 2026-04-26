# Zenith Language Checklist 4.0 (Core)

> Checklist operacional do ciclo R4 da linguagem Zenith.
> Status: fechado para Alpha.2 local.
> Atualizado: 2026-04-25.

## Criterio de fechamento

R4 foi fechado como estabilizacao do core da linguagem para Alpha.2.

O criterio usado foi pratico:

- o compilador precisa compilar sem warnings no build principal;
- o projeto raiz precisa passar em `zt check --all --ci`;
- os fluxos ja entregues precisam ter teste executavel;
- o LSP beta local precisa ter smoke test executavel para recursos de editor;
- itens que mudam semantica grande precisam ficar no roadmap futuro, nao entrar sem RFC propria.

Nao marcamos como implementado o que ainda nao existe na linguagem.
Quando um item era grande demais para este corte, ele foi fechado como
"reescopado" e registrado no roadmap futuro.

## 1. Fase 1: Estabilizacao & Infra

### R4.M0 - Alpha.2 stabilization base

- [x] Reduzir warnings de build para zero.
- [x] Consolidar baseline de qualidade/perf por plataforma.
- [x] Automatizar script de release unificado (`Build.bat`, `python build.py`, `tools/release.py`).
- [x] Validar ARC/Shared em cenario de carga concorrente.
- [x] Corrigir regressao do gate raiz da stdlib.

Evidencias:

- `python build.py`
- `./zt.exe check zenith.ztproj --all --ci`
- `python run_suite.py smoke --no-perf`
- `python run_all_tests.py`
- `tests/runtime/c/test_shared_text.c`

## 2. Fase 2: Concorrencia (Surface)

### R4.CF1 - Concurrency Full

- [x] Validar `Shared<T>` runtime com contagem atomica de referencia.
- [x] Adicionar teste de race condition no compilador.
- [x] Reescopar keyword `task` para roadmap futuro.
- [x] Reescopar canais (`chan<T>`) para roadmap futuro.

Motivo do reescopo:

- `task` ainda aparece como identificador em codigo atual;
- promover `task` para keyword sem migracao quebraria codigo existente;
- `chan<T>` precisa de contrato proprio de scheduling, fechamento, backpressure e ownership;
- concorrencia de surface nao deve entrar como detalhe de release.

Regra para o futuro:

- `task` deve ser contextual ou vir com migracao documentada;
- `chan<T>` deve ter RFC propria;
- `public var` nao vira estado global compartilhado entre tasks automaticamente;
- compartilhamento entre tasks continua exigindo primitiva explicita.

## 3. Fase 3: Interoperabilidade (FFI)

### R4.M2 - FFI Architecture 1.0

- [x] Estabilizar tipos FFI na spec.
- [x] Validar FFI positivo com `extern c`.
- [x] Validar FFI negativo para tipo nao permitido.
- [x] Reescopar conversao automatica `string` para `*int8` para FFI futuro.
- [x] Reescopar `extern fn` com convencao de chamada customizavel para FFI futuro.

Motivo do reescopo:

- a linguagem usa `text`, nao `string`, como tipo canonico de texto;
- raw pointers nao fazem parte da surface atual;
- ABI annotations estao explicitamente fora de `language/spec/ffi.md` neste corte;
- callbacks e calling conventions precisam de desenho conjunto com callables/delegates.

Evidencias:

- `language/spec/ffi.md`
- `tests/behavior/extern_c_text_len_e2e`
- `tests/behavior/extern_c_puts_e2e`
- `tests/behavior/extern_c_struct_arg_error`

## 4. Fase 4: Otimizacao de Memoria

### R4.M1 - ARC Optimization

- [x] Validar isolamento ARC nos gates principais.
- [x] Validar `Shared<T>` atomico sob contencao.
- [x] Reescopar "Inline ARC" para fase futura de optimizer.
- [x] Reescopar eliminacao de retain/release redundante para fase futura de optimizer.
- [x] Manter budgets de latencia como gate de perf/CI, nao como bloqueio local deste fechamento.

Motivo do reescopo:

- otimizar ARC sem IR/optimizer dedicado aumenta risco de regressao;
- o ganho esperado e de performance, nao de correcao semantica;
- Alpha.2 prioriza estabilidade da linguagem e gates verdes.

## 5. Fase 5: Release Alpha.2

### R4.M4 - Alpha.2 Official Release

- [x] Gerar binarios locais `zt.exe` e `zpm.exe`.
- [x] Validar Windows local x64.
- [x] Gerar relatorio de evidencias de estabilidade.
- [x] Registrar limites conhecidos sem mascarar escopo.

Evidencias:

- `docs/internal/reports/release/R4.M0-stability-report.md`
- `docs/internal/reports/release/R4.M4-alpha2-language-core-report.md`

## Resultado final

O checklist v4 da linguagem esta fechado para Alpha.2 local.

Ultima validacao completa registrada neste fechamento:

- `python run_all_tests.py`: 190 total, 188 pass, 0 fail, 2 skip.
- `python tests/lsp/test_lsp_smoke.py`: `lsp smoke ok`.

Itens que continuam fora deste corte:

- `task` como keyword oficial;
- `chan<T>`;
- raw pointers na surface FFI;
- conversao automatica `text`/`string` para C string;
- calling convention customizavel;
- callbacks C chamando Zenith;
- optimizer ARC dedicado.

Esses itens pertencem ao roadmap futuro, nao a correcoes residuais deste corte.

Nota:

- scheduler interno da engine e APIs simples de jobs/assets async-like foram
  movidos para `docs/internal/planning/borealis-engine-studio-roadmap-v3.md`;
- esse plano tem checklist proprio em
  `docs/internal/planning/borealis-engine-studio-checklist-v3.md`;
- isso nao depende de `async func`/`await` na linguagem.
