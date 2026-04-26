# Zenith Language Roadmap 4.0 (Core)

> Roadmap do ciclo R4 da linguagem Zenith.
> Status: fechado para Alpha.2 local.
> Atualizado: 2026-04-25.

## Objetivo

R4 estabiliza o core da linguagem para Alpha.2.

O foco deste ciclo foi:

- manter o compilador e o runtime em estado validavel;
- fechar regressions de gate raiz;
- consolidar ZPM e LSP como ferramentas locais;
- separar claramente o que esta entregue do que exige RFC futura.

## Escopo fechado em Alpha.2

Entrou neste fechamento:

- build local sem warnings no gate principal;
- `zt.exe` e `zpm.exe` gerados por `python build.py`;
- gate raiz `zt check zenith.ztproj --all --ci` verde;
- smoke suite verde;
- FFI 1.0 documentado e validado nos casos atuais;
- `Shared<T>` validado no runtime C com contagem atomica sob carga;
- hardening de compilacao concorrente validado;
- LSP beta local documentado em relatorio proprio, com diagnostics, completion,
  hover, definition, references, rename, semantic tokens, outline, workspace symbols e formatting;
  signature help para funcoes top-level indexadas;
- ZPM MVP documentado em relatorio proprio.

## Escopo movido para roadmap futuro

Os itens abaixo nao foram removidos.
Eles foram retirados do fechamento Alpha.2 porque mudam semantica publica.

### Concorrencia de surface

- `task` como keyword oficial;
- `chan<T>`;
- checker-level `transferable` completo;
- modelo de cancelamento e determinismo em runtime.

Motivo:

- `task` pode quebrar codigo atual se virar keyword rigida;
- channels precisam de contrato proprio de fechamento, backpressure e ownership;
- `public var` nao deve virar global compartilhado por acidente.

Nota:

- scheduler interno de engine, jobs de asset e loading assinc-like do Borealis
  pertencem a `docs/internal/planning/borealis-engine-studio-roadmap-v3.md`;
- isso nao exige `task`, `async func`, `await` ou `chan<T>` na linguagem.

### FFI futuro

- raw pointer surface types;
- conversao automatica `text`/`string` para C string;
- callbacks C chamando Zenith;
- ABI annotations (`__stdcall`, `__cdecl`, etc.);
- symbol renaming em `extern c`.

Motivo:

- `language/spec/ffi.md` declara esses itens fora do corte atual;
- liberar ponteiros crus sem modelo de ownership aumenta risco de UB;
- callbacks dependem do desenho completo de callables/delegates no backend C.

### Optimizer ARC futuro

- inline ARC para structs pequenas;
- eliminacao global de retain/release redundante;
- analise de escape dedicada;
- deteccao estatica de ciclos simples.

Motivo:

- isso exige fase de optimizer propria;
- deve ser medido com benchmark antes de entrar no core;
- nao bloqueia correcao semantica da Alpha.2.

## Fases do Roadmap Core 4.0

### R4.M0 - Alpha.2 stabilization base

Status: fechado.

- [x] Reduzir warnings de build para zero.
- [x] Consolidar baseline de qualidade/perf por plataforma.
- [x] Automatizar script de release unificado.
- [x] Validar ARC/Shared em carga concorrente.
- [x] Corrigir gate raiz da stdlib.

### R4.M1 - LSP + VSCode beta local

Status: fechado como beta local.

- [x] Diagnostics em tempo real.
- [x] Completion global e contextual.
- [x] Completion de imports com stdlib e modulos do projeto.
- [x] Hover, definition, references, rename, signature help, semantic tokens e formatting.
- [x] Outline via Document Symbols.
- [x] Busca de simbolos via Workspace Symbols.
- [x] Relatorio em `docs/internal/reports/release/R4.M1-lsp-vscode-report.md`.

### R4.M2 - ZPM MVP

Status: fechado como MVP local.

- [x] `zpm init`.
- [x] `zpm add`.
- [x] `zpm install`.
- [x] `zpm publish` minimo.
- [x] `zt pkg` e `zt zpm` como entradas do driver.
- [x] `zpm.exe` standalone.
- [x] Relatorio em `docs/internal/reports/release/R4.M2-zpm-report.md`.

### R4.M3 - Unified Backend: C & Native

Status: mantido como trilha futura.

- [ ] Melhorar legibilidade do C emitido para debugging.
- [ ] Explorar backend nativo/LLVM em RFC separada.

### R4.M4 - Alpha.2 Official Release (Language Core)

Status: fechado localmente.

- [x] Congelar escopo da spec para Alpha.2.
- [x] Validar Windows local x64.
- [x] Registrar limites conhecidos.
- [x] Publicar relatorio de fechamento local.

## Gates de qualidade usados

Gates executados neste fechamento local:

- `python build.py`
- `./zt.exe check zenith.ztproj --all --ci`
- `python run_suite.py smoke --no-perf`
- `python run_all_tests.py` (`190 total`, `188 pass`, `0 fail`, `2 skip`)
- `python tests/hardening/test_concurrent_compilation.py`
- `tests/runtime/c/test_shared_text.c`
- fixtures FFI positivas e negativa.

Gates que continuam recomendados antes de distribuicao publica:

- gate de perf completo;
- matriz Windows 10 / Windows 11 / Linux;
- ASAN/Valgrind em ambiente Linux.
