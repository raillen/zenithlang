# 🚀 Sprint 1: Fundação Lexical (Lexer)

O primeiro marco do projeto Zenith focou em transformar o texto bruto em blocos de informação compreensíveis pelo compilador: os **Tokens**.

## 🎯 Objetivos Concluídos
- [x] Definição de `TokenKind`: Identificação de keywords, operadores e literais.
- [x] Implementação do `Lexer`: Motor de tokenização com lookahead.
- [x] Tabela de Keywords: Mapeamento de 50+ palavras-chave exclusivas do Zenith.
- [x] Suporte a Comentários: Herança do estilo Lua (`--`).
- [x] Literais Básicos: Suporte a `int`, `float`, `text` e `bool`.

## 🛠️ Arquitetura
- `src/syntax/lexer/lexer.lua`: O motor principal.
- `src/syntax/tokens/token_kind.lua`: O catálogo de tokens.
- `src/syntax/tokens/keyword_table.lua`: O dicionário de termos reservados.

## ✅ Validação (Testes)
- **Suite:** `tests/lexer_tests/test_lexer.lua`
- **Status:** 100% Passando
- **Casos Cobertos:** Tokenização de strings, operadores compostos (`==`, `->`), identificadores e recuperação básica de caracteres inválidos.

---
> [!TIP]
> **Destaque do Sprint:** A implementação de `TokenKind.ARROW` (`->`) para retornos de função, um pilar da legibilidade visual do Zenith.
