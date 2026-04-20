# AST - Code Map

## 📋 Descrição

Definições da Abstract Syntax Tree. Responsável por:
- Estruturas de dados para todos os nodes AST
- Tipos de expressões, statements, declarations
- Visitors e traversals
- Serialização/deserialização

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `*.c` | - | AST utilities, visitors |
| `*.h` | - | Node type definitions, enums |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟡 MÉDIA |

## ⚠️ Estado Crítico

- **Node types**: enum de todos os tipos de nodes
- **Parent pointers**: navegação na árvore
- **Span information**: localização no source

## 🔗 Dependencies Externas

- `frontend/lexer/` → Token spans
- `utils/string_pool.c` → Identifier strings

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- [A preencher após análise detalhada]

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Medium
- Source files: 2
- Extracted symbols: 9

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/frontend/ast/model.c` | 140 | 8 | 1 |
| `compiler/frontend/ast/model.h` | 399 | 1 | 2 |

### Local Dependencies

- `compiler/frontend/ast/model.h`
- `compiler/frontend/lexer/token.h`
- `compiler/utils/arena.h`

### Related Tests

- `tests/frontend/test_parser.c`
- `tests/frontend/test_parser_error_recovery.c`

### Symbol Index

#### `compiler/frontend/ast/model.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 6 | `func_def` | `zt_ast_kind_name` |
| 66 | `func_def` | `zt_ast_make` |
| 75 | `func_def` | `zt_ast_node_list_push` |
| 88 | `func_def` | `zt_ast_named_arg_list_push` |
| 101 | `func_def` | `zt_ast_map_entry_list_push` |
| 114 | `func_def` | `zt_ast_node_list_make` |
| 122 | `func_def` | `zt_ast_map_entry_list_make` |
| 131 | `func_def` | `zt_ast_named_arg_list_make` |

#### `compiler/frontend/ast/model.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_FRONTEND_AST_MODEL_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
