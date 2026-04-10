# Evolução da Linguagem Zenith

Este changelog acompanha o estado real do repositório, incluindo o snapshot de desenvolvimento atual.

## [Em desenvolvimento] - 2026-04-10

### Adicionado

- `std.os.process` com `spawn`, `wait`, `kill`, `read_output` e `current_id`.

### Alterado

- O codegen de `async func` foi ajustado para não emitir chamada inválida quando a função não possui parâmetros.

### Observações

- `std.os.process` já está documentado e testado no repositório, mas ainda carrega limitações conhecidas no runtime:
  - `pid` simbólico
  - `kill()` best effort
  - `current_id()` retornando `0`
- A integração async do runtime segue em estabilização.

## [0.2.5] - 2026-04-09

### Adicionado

- `std.os`
- `std.fs`
- `std.fs.path`
- `std.json`

### Corrigido

- Shadowing no codegen
- Ajustes de parser para aninhamentos complexos
- Escapes de string com `\"`, `\{` e `\}`
- Suporte ampliado a `#`, listas e mapas no backend Lua

## [0.2.0] - 2026-04-08

### Adicionado

- Lowering e IR
- Constant folding
- Desestruturação em `match` e `var`
- Restrições genéricas com `where T is Trait`
- Traits com validação e default impls
- Contratos `where` em campos de `struct`
- Prelude automática com `std.core`
- Spread e slicing com `..`
- Distinção entre `type` e `union`

### Alterado

- `@campo` substituiu o antigo açúcar com `self`
- `.` passou a ser usado como base do chaining UFCS
- Variáveis passaram a exigir tipo explícito
- Retornos passaram a aceitar forma padronizada com `->`

## [0.1.0] - 2026-04-07

- Primeira versão funcional da linguagem
- Variáveis, funções e structs básicas
- Transpilação inicial para Lua
