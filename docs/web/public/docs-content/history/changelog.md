# Evolução da Linguagem Zenith

Este changelog registra as mudanças na especificação e no compilador da linguagem Zenith.

## [v0.2.0-alpha] - 2026-04-08 (Versão Atual)
### Adicionado
- **Reatividade Nativa**: Keywords `state`, `computed` e `watch` integradas ao Core.
- **Grids**: Novo tipo primitivo `grid<T>` para matrizes 2D eficientes.
- **Lambdas**: Sintaxe simplificada para funções anônimas `(a, b) => expr`.
- **Match Avançado**: Suporte a destruturação de listas e structs no `match`.
- **Modificador `uniq`**: Garantia de unicidade em coleções no nível de tipo.

### Alterado
- **Sintaxe purista**: Remoção de parênteses opcionais em `if` e `while` para manter a consistência visual.
- **Self-Shortcut**: Introdução do `@` como atalho para `self`.
- **Destruturação**: Palavra-chave `as` em parâmetros para evitar conflitos visuais.

## [v0.1.0-alpha] - 2026-03-20
### Adicionado
- **Compilador v1**: Transpilação funcional para Lua 5.4.
- **Sistema de Tipos**: Verificação estática obrigatória para `var` e `const`.
- **Composição**: Implementação de `struct`, `trait` e `apply`.
- **Organização**: Sistema de `namespace` e `pub`.
- **Pratt Parser**: Expressões com precedência matemática correta.
