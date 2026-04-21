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
- Extracted symbols: 25

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/frontend/lexer/lexer.c` | 490 | 18 | 2 |
| `compiler/frontend/lexer/lexer.h` | 34 | 1 | 1 |
| `compiler/frontend/lexer/token.c` | 121 | 5 | 1 |
| `compiler/frontend/lexer/token.h` | 136 | 1 | 0 |

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
| 79 | `func_def` | `zt_lexer_make` |
| 94 | `func_def` | `zt_lexer_set_diagnostics` |
| 100 | `func_def` | `zt_lexer_dispose` |
| 106 | `func_def` | `zt_lexer_peek` |
| 111 | `func_def` | `zt_lexer_peek_next` |
| 116 | `func_def` | `zt_lexer_advance` |
| 134 | `func_def` | `zt_lexer_skip_trivia` |
| 147 | `func_def` | `zt_lexer_lookup_keyword` |
| 158 | `func_def` | `zt_lexer_make_token` |
| 168 | `func_def` | `zt_lexer_read_string` |
| 194 | `func_def` | `zt_lexer_resume_string` |
| 221 | `func_def` | `zt_lexer_read_triple_quoted` |
| 246 | `func_def` | `zt_lexer_read_number` |
| 324 | `func_def` | `zt_lexer_read_identifier` |
| 340 | `func_def` | `zt_lexer_next_token` |
| 481 | `func_def` | `zt_lexer_current_span` |
| 486 | `func_def` | `zt_lexer_is_at_end` |

#### `compiler/frontend/lexer/lexer.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_FRONTEND_LEXER_LEXER_H` |

#### `compiler/frontend/lexer/token.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 3 | `func_def` | `zt_token_kind_name` |
| 95 | `func_def` | `zt_token_kind_is_keyword` |
| 99 | `func_def` | `zt_token_kind_is_literal` |
| 103 | `func_def` | `zt_source_span_make` |
| 112 | `func_def` | `zt_source_span_unknown` |

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
