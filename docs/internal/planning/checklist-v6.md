# Zenith Language Checklist 6.0 (Ergonomics Without Magic)

> Checklist operacional derivado de `docs/internal/planning/roadmap-v6.md`.
> Status: proposto.
> Criado: 2026-04-25.

## Como usar

1. Marque apenas itens com evidencia no codigo, docs ou testes.
2. Nao feche item de linguagem sem fixture positiva ou negativa.
3. Se uma ideia exigir sintaxe grande, mova para RFC futura em vez de marcar aqui.
4. Preserve a filosofia: explicito, legivel, previsivel e acessivel.

## Gates obrigatorios do ciclo

- [x] `python build.py` verde.
- [x] `./zt.exe check zenith.ztproj --all --ci` verde.
- [x] `python run_suite.py smoke --no-perf` verde.
- [x] Novos helpers com behavior tests.
- [x] Novos diagnosticos com fixtures invalidas.
- [x] Novas regras de formatter com golden + idempotence.
- [x] Docs atualizadas quando a superficie publica mudar.

## R6.M0 - Baseline de filosofia e nao-objetivos

- [x] Documentar que inferencia local ampla continua fora do ciclo.
- [x] Documentar que union segura em Zenith e `enum` com payload.
- [x] Documentar que metodo abstrato em Zenith e `trait`.
- [x] Documentar que dispatch virtual em Zenith e `dyn<Trait>`.
- [x] Documentar que extensao separada de comportamento usa `apply`.
- [x] Documentar que `abstract`, `virtual`, `partial` e `union` nao entram como keywords.
- [x] Referenciar `roadmap-v6.md` em `docs/internal/planning/README.md`.

Criterio de aceite:

- [x] O leitor consegue ver o que R6 recomenda e o que R6 rejeita sem ler a conversa original.

Evidencia:

- `docs/public/cookbook/zenith-equivalents-from-other-languages.md`
- `docs/public/cookbook/safe-union-with-enum.md`
- `docs/public/cookbook/abstract-methods-with-trait.md`
- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `docs/public/cookbook/partial-class-with-apply.md`
- `docs/internal/planning/README.md`

## R6.M1 - Diagnosticos de sintaxe nao-canonica

- [x] Sugerir `text` quando o usuario escrever `string`.
- [x] Sugerir `const` ou `var` quando o usuario escrever `let`.
- [x] Sugerir `and` quando o usuario escrever `&&`.
- [x] Sugerir `or` quando o usuario escrever `||`.
- [x] Sugerir `not value` quando o usuario escrever `!value`.
- [x] Sugerir `optional<T>` e `none` quando o usuario escrever `null`.
- [x] Sugerir `result<T,E>`, `error(...)` ou `panic(...)` quando o usuario escrever `throw`.
- [x] Sugerir `trait` quando o usuario escrever `abstract`.
- [x] Sugerir `dyn<Trait>` quando o usuario escrever `virtual`.
- [x] Sugerir `enum` com payload quando o usuario escrever `union`.
- [x] Sugerir `apply` e organizacao por namespace/arquivo quando o usuario escrever `partial`.
- [x] Criar fixtures invalidas com fragmentos esperados para cada caso aceito no parser/lexer.

Criterio de aceite:

- [x] Cada erro comum tem mensagem action-first e proxima acao clara.

Evidencia:

- `compiler/frontend/parser/parser.c`
- `compiler/frontend/lexer/lexer.c`
- `compiler/semantic/types/checker.c`
- `tests/frontend/test_parser_error_recovery.c`
- `tests/behavior/noncanonical_*_error/`
- `tests/fixtures/diagnostics/noncanonical_*_error.contains.txt`
- `tests/suites/suite_definitions.py`

Validacao:

- `python build.py`
- `./zt.exe check zenith.ztproj --all --ci`
- `python run_suite.py smoke --no-perf`
- `zt check` direcionado para cada `tests/behavior/noncanonical_*_error`

## R6.M2 - Builtins pequenos de intencao

- [x] Definir surface de `todo(message: text)`.
- [x] Definir surface de `unreachable(message: text)`.
- [x] Implementar typecheck de `todo`.
- [x] Implementar typecheck de `unreachable`.
- [x] Implementar lowering/backend para `todo`.
- [x] Implementar lowering/backend para `unreachable`.
- [x] Permitir `check(condition, message)` mantendo `check(condition)`.
- [x] Criar behavior test para `todo`.
- [x] Criar behavior test para `unreachable`.
- [x] Criar behavior test para `check(condition, message)`.

Criterio de aceite:

- [x] Os tres caminhos produzem falha clara sem adicionar alias `assert`.

Evidencia:

- `compiler/semantic/binder/binder.c`
- `compiler/semantic/types/checker.c`
- `compiler/hir/lowering/from_ast.c`
- `compiler/zir/lowering/from_hir.c`
- `compiler/targets/c/emitter.c`
- `runtime/c/zenith_rt.c`
- `runtime/c/zenith_rt.h`
- `tests/behavior/todo_builtin_fail/`
- `tests/behavior/unreachable_builtin_fail/`
- `tests/behavior/check_intrinsic_message_fail/`
- `tests/fixtures/diagnostics/todo_builtin_fail.contains.txt`
- `tests/fixtures/diagnostics/unreachable_builtin_fail.contains.txt`
- `tests/fixtures/diagnostics/check_intrinsic_message_fail.contains.txt`
- `docs/public/cookbook/intentional-failure-builtins.md`
- `language/spec/surface-syntax.md`

Validacao:

- `python build.py`
- `zt run` direcionado para `todo_builtin_fail`, `unreachable_builtin_fail` e `check_intrinsic_message_fail`
- `./zt.exe run tests/behavior/check_intrinsic_basic`
- `python tools/build_lsp.py`
- `node --check tools/vscode-zenith/extension.js`
- `./zt.exe check zenith.ztproj --all --ci`
- `python run_suite.py smoke --no-perf`

## R6.M3 - `std.test` mais expressivo

- [x] Adicionar `test.is_true(value)`.
- [x] Adicionar `test.is_false(value)`.
- [x] Adicionar `test.equal_int(actual, expected)`.
- [x] Adicionar `test.equal_text(actual, expected)`.
- [x] Adicionar `test.not_equal_int(actual, expected)`.
- [x] Adicionar `test.not_equal_text(actual, expected)`.
- [x] Garantir mensagens com esperado/recebido quando aplicavel.
- [x] Atualizar docs de `std.test`.
- [x] Criar behavior tests de sucesso.
- [x] Criar behavior tests de falha.

Criterio de aceite:

- [x] Testes falhos ajudam a corrigir o problema sem precisar depurar o helper.

Evidencias:

- `stdlib/std/test.zt` agora expõe `is_true`, `is_false`, `equal_int`, `equal_text`, `not_equal_int` e `not_equal_text`.
- As mensagens usam `expected ...; received ...` quando ha valor esperado e recebido.
- Novos projetos behavior: `std_test_helpers_pass`, `std_test_helpers_bool_fail`, `std_test_helpers_equal_fail` e `std_test_helpers_not_equal_fail`.
- Fragmentos de falha em `tests/fixtures/diagnostics/std_test_helpers_*_fail.contains.txt`.
- Docs atualizadas em `stdlib/zdoc/std/test.zdoc`, `docs/reference/stdlib/concurrency-lazy-test-net.md`, `docs/public/stdlib/README.md`, `docs/reference/stdlib/modules.md`, `docs/reference/zenith-kb/stdlib.md` e `language/spec/stdlib-model.md`.
- Mapa gerado atualizado com `python tools/generate_code_maps.py --match stdlib`.

Validacao:

- `python build.py`
- `./zt.exe run tests/behavior/std_test_helpers_pass`
- `./zt.exe run tests/behavior/std_test_helpers_bool_fail`
- `./zt.exe run tests/behavior/std_test_helpers_equal_fail`
- `./zt.exe run tests/behavior/std_test_helpers_not_equal_fail`
- `python tools/generate_code_maps.py --match stdlib --check`
- `./zt.exe check zenith.ztproj --all --ci`
- `python run_suite.py smoke --no-perf`
- `python tools/check_docs_paths.py` ainda falha em 3 referencias antigas de runtime: process run, text UTF-8 guardrails e SDK.

## R6.M4 - Helpers explicitos para `optional` e `result`

- [x] Especificar `optional<T>.or(default_value)`.
- [x] Especificar `optional<T>.or_error(message)`.
- [x] Especificar `optional<T>.is_some()`.
- [x] Especificar `optional<T>.is_none()`.
- [x] Especificar `result<T,E>.is_success()`.
- [x] Especificar `result<T,E>.is_error()`.
- [x] Especificar `result<T,E>.or_panic(message)`.
- [x] Implementar o primeiro subconjunto viavel sem `?.` ou `??`.
- [x] Criar behavior tests para fluxo positivo.
- [x] Criar behavior tests para fluxo de ausencia/erro.
- [x] Atualizar docs de optional/result.

Nota:

- implementado nesta fatia: `is_some`, `is_none`, `or`, `is_success`, `is_error`;
- especificado para depois: `or_error` e `or_panic`, porque ainda precisam de lowering proprio para conversao/panic com mensagem.

Criterio de aceite:

- [x] Os helpers reduzem `match` repetitivo, mas nao escondem propagacao ou conversao de erro.

Validacao:

- `python build.py`
- `./zt.exe run tests/behavior/optional_result_helpers_pass/zenith.ztproj`
- `./zt.exe run tests/behavior/optional_result_helpers_absence_error/zenith.ztproj`

## R6.M5 - Helpers pequenos em `std.validate`, `std.text` e colecoes

### `std.validate`

- [x] `validate.positive(value)`.
- [x] `validate.non_negative(value)`.
- [x] `validate.not_empty_text(value)`.
- [x] `validate.min_len(value, min)`.
- [x] `validate.max_len(value, max)`.
- [x] `validate.between(value, min, max)`.

### `std.text`

- [x] `text.is_empty(value)`.
- [x] `text.is_blank(value)`.
- [x] `text.contains(value, needle)`.
- [x] `text.starts_with(value, prefix)`.
- [x] `text.ends_with(value, suffix)`.

### Colecoes

- [x] `list.is_empty(items)`.
- [x] `map.is_empty(values)`.
- [x] `map.has_key(values, key)`.

Criterio de aceite:

- [x] Cada helper tem nome claro, docs curtas e behavior test.

Evidencias:

- `std.validate`: `positive`, `non_negative` e `between` ja existiam; `not_empty_text`, `min_len` e `max_len` foram adicionados como aliases claros.
- `std.text`: `is_empty`, `contains`, `starts_with` e `ends_with` ja existiam; `is_blank` foi adicionado.
- `std.list` e `std.map` foram adicionados como modulos pequenos para helpers compiler-known de `list<T>` e `map<K,V>`.
- Behavior test novo: `tests/behavior/std_small_helpers/`.
- Docs curtas atualizadas em `stdlib/zdoc/std/`, `docs/reference/stdlib/` e `docs/public/stdlib/`.

Validacao:

- `python build.py`
- `./zt.exe run tests/behavior/std_small_helpers/zenith.ztproj`
- `./zt.exe check zenith.ztproj --all --ci`
- `python run_suite.py smoke --no-perf`
- `python tools/generate_code_maps.py --match stdlib --check`

Observacao:

- `./zt.exe doc check zenith.ztproj` ainda falha em dividas preexistentes de ZDoc fora de R6.M5.
- `python tools/check_docs_paths.py` ainda falha em referencias antigas de runtime: process run, text UTF-8 guardrails e SDK.

## R6.M6 - Warnings de legibilidade e acessibilidade

- [x] Warning para nomes muito parecidos no mesmo escopo.
- [x] Warning para identificadores visualmente confusos.
- [x] Manter shadowing perigoso como erro duro (`name.shadowing`), em vez de warning.
- [x] Warning para bloco muito profundo.
- [x] Warning para funcao muito longa.
- [x] Warning para `case default` em enum conhecido quando exhaustiveness seria possivel.
- [x] Modo normal nao deve bloquear build por esses warnings.
- [x] Modo estrito pode promover warnings selecionados.
- [x] Criar testes unitarios/behavior para cada warning.

Criterio de aceite:

- [x] Os warnings reduzem risco de leitura sem gerar falso positivo excessivo.

Progresso:

- `name.confusing` ja existia como warning para caracteres visualmente confusos.
- `name.similar` foi adicionado para nomes locais muito parecidos no mesmo escopo.
- `style.block_too_deep` foi adicionado para blocos com profundidade maior que 6.
- `style.function_too_long` foi adicionado para funcoes com mais de 80 instrucoes.
- `control_flow.enum_default_case` foi adicionado para `case default` em `match` de enum conhecido.
- O pipeline agora imprime warnings e continua em modo normal.
- `[diagnostics] profile = "strict"` promove warnings para erro.
- Behavior tests adicionados: `readability_warnings_pass` e `readability_warnings_strict_error`.
- Behavior tests adicionados: `readability_block_depth_pass` e `readability_block_depth_strict_error`.
- Behavior tests adicionados: `readability_function_length_pass` e `readability_function_length_strict_error`.
- Behavior tests adicionados: `readability_enum_default_pass` e `readability_enum_default_strict_error`.
- Unit tests adicionados em `tests/semantic/test_binder.c` para `name.similar`, `style.block_too_deep` e `style.function_too_long`.
- Unit test adicionado em `tests/semantic/test_types.c` para `control_flow.enum_default_case`.
- Shadowing continua erro duro no MVP (`name.shadowing`), entao nao foi rebaixado para warning nesta fatia.
- `./zt.exe check zenith.ztproj --all --ci` ficou limpo apos remover um `case default` redundante em `stdlib/std/format.zt`.

Validacao parcial:

- `python build.py`
- `./zt.exe run tests/behavior/readability_warnings_pass/zenith.ztproj`
- `./zt.exe check tests/behavior/readability_warnings_strict_error/zenith.ztproj --ci`
- `./zt.exe run tests/behavior/readability_block_depth_pass/zenith.ztproj`
- `./zt.exe check tests/behavior/readability_block_depth_strict_error/zenith.ztproj --ci`
- `./zt.exe run tests/behavior/readability_function_length_pass/zenith.ztproj`
- `./zt.exe check tests/behavior/readability_function_length_strict_error/zenith.ztproj --ci`
- `./zt.exe run tests/behavior/readability_enum_default_pass/zenith.ztproj`
- `./zt.exe check tests/behavior/readability_enum_default_strict_error/zenith.ztproj --ci`
- `./zt.exe check zenith.ztproj --all --ci`

Decisao de fechamento:

- Shadowing perigoso nao vira warning em R6 porque o MVP rejeita shadowing como erro duro.
- Nenhum warning restante foi deixado aberto nesta fatia.

## R6.M7 - Formatter reading-first

- [x] Quebrar assinatura longa em parametros um por linha.
- [x] Quebrar `where` longo em linha separada quando melhorar leitura.
- [x] Preservar comentario junto do item explicado.
- [x] Manter `match` vertical e estavel.
- [x] Adicionar golden tests para novas regras.
- [x] Adicionar casos ao gate de idempotencia.

Criterio de aceite:

- [x] `fmt(fmt(x)) == fmt(x)` nos casos novos.

## R6.M8 - `zt explain`

- [x] Definir formato de `zt explain <codigo-do-erro>`.
- [x] Criar catalogo inicial para erros comuns.
- [x] Incluir significado curto.
- [x] Incluir exemplo invalido.
- [x] Incluir exemplo corrigido.
- [x] Incluir proxima acao.
- [x] Incluir link local de doc quando existir.
- [x] Criar testes de driver para codigo conhecido.
- [x] Criar teste para codigo desconhecido.

Criterio de aceite:

- [x] Um usuario novo consegue entender o erro sem buscar fora do repo.

## R6.M9 - Polimento de `enum`, callables e `dyn`

- [x] Melhorar erro de `match` enum nao exaustivo listando cases ausentes.
- [x] Melhorar erro de callable com assinatura esperada e recebida.
- [x] Melhorar erro de callable generico usado como valor.
- [x] Melhorar erro de `dyn<Trait>` quando trait nao pode ser dinamico.
- [x] Sugerir generics/trait constraints quando `dyn` for desnecessario.
- [x] Atualizar docs comparando:
  - union segura -> `enum` com payload;
  - metodo abstrato -> `trait`;
  - virtual -> `dyn<Trait>`;
  - partial class -> `apply` + namespaces/arquivos.

Criterio de aceite:

- [x] A linguagem fica mais clara sem adicionar `union`, `abstract`, `virtual` ou `partial`.

Status atual: implementado com diagnosticos mais explicitos para enum, callables e dyn, fixtures de comportamento e docs publicas/spec sincronizadas.

## Fechamento de R6

- [x] Nenhuma sintaxe pesada adicionada sem RFC propria.
- [x] Novos helpers documentados.
- [x] Novos diagnosticos testados.
- [x] Formatter idempotente nos casos novos.
- [x] Relatorio de fechamento criado em `docs/internal/reports/release/`.

Status atual: R6 fechado localmente em `docs/internal/reports/release/R6-clarity-and-accessibility-closure-report.md`.
