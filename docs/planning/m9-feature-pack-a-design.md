# R2.M9 - Feature Pack A (linguagem)

## Objetivo

Evoluir sintaxe/semantica com risco controlado.

## Regra

Toda feature entra com design curto, testes e criterio de rollback.

## Features Priorizadas

1. **A1: panic() como statement de fonte** - P0 fechado em M8, implementado em M9
2. **A2: optional<T> ? propagacao** - Completa o modelo de erros/ausencia
3. **A3: Enum exhaustividade + case value para optional** - Leva enum match para Conformant

---

## A1: panic() como statement de fonte

### Design curto

`panic(message: text)` e um statement core que aborta a execucao com mensagem fatal.

- Parser: `ZT_TOKEN_PANIC` + expressao entre parenteses opcionais
- Semantic: valida que o argumento e tipo `text`
- HIR: `ZT_HIR_PANIC_STMT` com campo `message`
- ZIR: `ZIR_TERM_PANIC` com `zir_make_panic_terminator_expr`
- Emitter: `zt_panic("mensagem")` para literais, `zt_panic(zt_text_data(expr))` para expressoes
- Formatter: `panic(expr)`

### Criterio de rollback

Se `panic()` causar regressao no parser ou emitter, remover ZT_TOKEN_PANIC e ZT_AST_PANIC_STMT.

### Evidencia

- Behavior tests: `tests/behavior/panic_basic/`, `tests/behavior/panic_with_message/`
- Pipeline: lexer -> parser -> binder -> checker -> HIR -> ZIR -> C emitter -> runtime
- Build verde, 123/124 tests pass

### Status: Concluido

---

## A2: optional<T> ? propagacao

### Design curto

O operador `?` ja funciona para `result<T,E>`. Estender para `optional<T>`:
dentro de funcoes que retornam `optional<T>`, `expr?` propaga `none` se a
ausencia for detectada.

- ZIR lowering: branch com `optional_is_present`, extracao com `optional_value` e retorno de `optional_empty(...)` no caminho de ausencia
- Semantic: validar que `optional<T>?` so e permitido em funcoes que retornam `optional<Other>`
- Behavior tests: `optional_question_basic` (positivo) e `optional_question_outside_optional_error` (negativo)

### Criterio de rollback

Se `optional<T>?` causar regressao em `result<T,E>?`, remover as adicoes de
ZIR lowering e emitter sem afetar o caminho existente.

### Evidencia

- `compiler/semantic/types/checker.c`: regra de contexto para `optional<T>?`
- `compiler/zir/lowering/from_hir.c`: caminho de lowering para optional `?`
- `tests/behavior/optional_question_basic/`
- `tests/behavior/optional_question_outside_optional_error/`

### Status: Concluido

---

## A3: Enum exhaustividade + case value para optional

### Design curto

1. Diagnostico de match nao-exaustivo em enums (sem `case default`): emitir erro
   quando nem todos os variants sao cobertos.
2. `case value name:` para `optional<T>`: bind do valor presente em `match`.

- Semantic: adicionar check de exhaustividade no checker
- Semantic: adicionar suporte a `case value name` para optional
- Behavior tests: `enum_match_non_exhaustive_error`, `optional_match_value`

### Criterio de rollback

Se o check de exhaustividade causar falso-positivos em enums com `case default`,
remover o check e manter apenas o parsing.

### Evidencia

- `compiler/semantic/types/checker.c`: check de exhaustividade para enum/optional
- `tests/behavior/enum_match_non_exhaustive_error/`
- `tests/behavior/optional_match_value/`

### Status: Concluido
