# 🧱 Sprint 2: Arquitetura Sintática (Parser)

Nesta fase, o Zenith ganhou a capacidade de entender a **estrutura** do código, organizando os tokens em uma Árvore de Sintaxe Abstrata (AST).

## 🎯 Objetivos Concluídos
- [x] Implementação do `Parser`: Algoritmo Recursivo Descendente.
- [x] Árvore de Sintaxe (AST): Definição de `SyntaxNode` e seus derivados.
- [x] Expressões de Precedência: Parsing robusto de operações matemáticas e lógicas.
- [x] Blocos e Controle de Fluxo: Suporte a `if`, `while` e `for`.
- [x] Declarações Complexas: Suporte inicial a `func` e estruturas de dados.

## 🛠️ Arquitetura
- `src/syntax/parser/parser.lua`: O orquestrador do parsing.
- `src/syntax/ast/`: Diretório contendo todas as definições de nós (expr, stmt, decl).
- `src/syntax/parser/parse_expressions.lua`: Lógica dedicada para precedência de operadores.

## ✅ Validação (Testes)
- **Suite:** `tests/parser_tests/test_parser.lua`
- **Status:** 100% Passando
- **Casos Cobertos:** Parsing de nested blocks, precedência de operadores (Pratt style), declarações de variáveis e constantes.

---
> [!IMPORTANT]
> **Destaque do Sprint:** A transição para um parser modularizado, onde expressões, statements e declarações têm lógicas de parsing separadas, garantindo manutenibilidade.
