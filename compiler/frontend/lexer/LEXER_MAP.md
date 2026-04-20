# Lexer - Code Map

## 📋 Descrição

Tokenizer do Zenith. Responsável por:
- Converter source code em stream de tokens
- Identificar keywords, identifiers, literals, operators
- Tracking de linha/coluna para error reporting
- Handling de strings, comments, whitespace

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `*.c` | - | Implementação do lexer |
| `*.h` | - | Definições de tokens, structs |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🔴 CRÍTICA |

## ⚠️ Estado Crítico

- **Buffer atual**: pointer para posição no source
- **Token atual**: último token emitido
- **Linha/Coluna**: posição atual para diagnostics

## 🔗 Dependencies Externas

- `utils/string_pool.c` → String interning
- `utils/arena.c` → Memory allocation
- `utils/diagnostics.h` → Error reporting

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- [A preencher após análise detalhada]

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Critical
- Source files: 4
- Extracted symbols: 18

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/frontend/lexer/lexer.c` | 502 | 11 | 2 |
| `compiler/frontend/lexer/lexer.h` | 34 | 1 | 1 |
| `compiler/frontend/lexer/token.c` | 120 | 5 | 1 |
| `compiler/frontend/lexer/token.h` | 135 | 1 | 0 |

### Local Dependencies

- `compiler/frontend/lexer/lexer.h`
- `compiler/frontend/lexer/token.h`
- `compiler/semantic/diagnostics/diagnostics.h`

### Related Tests

- `tests/frontend/test_lexer.c`

### Symbol Index

#### `compiler/frontend/lexer/lexer.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 8 | `struct` | `zt_keyword_entry` |
| 78 | `func_def` | `zt_lexer_make` |
| 93 | `func_def` | `zt_lexer_set_diagnostics` |
| 99 | `func_def` | `zt_lexer_dispose` |
| 105 | `func_def` | `zt_lexer_peek` |
| 110 | `func_def` | `zt_lexer_peek_next` |
| 115 | `func_def` | `zt_lexer_advance` |
| 133 | `func_def` | `zt_lexer_skip_trivia` |
| 146 | `func_def` | `zt_lexer_lookup_keyword` |
| 157 | `func_def` | `zt_lexer_make_token` |
| 167 | `func_def` | `zt_lexer_read_string` |

#### `compiler/frontend/lexer/lexer.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_FRONTEND_LEXER_LEXER_H` |

#### `compiler/frontend/lexer/token.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 3 | `func_def` | `zt_token_kind_name` |
| 94 | `func_def` | `zt_token_kind_is_keyword` |
| 98 | `func_def` | `zt_token_kind_is_literal` |
| 102 | `func_def` | `zt_source_span_make` |
| 111 | `func_def` | `zt_source_span_unknown` |

#### `compiler/frontend/lexer/token.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_FRONTEND_LEXER_TOKEN_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
