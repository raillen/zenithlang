# Parser - Code Map

## 📋 Descrição

Parser do Zenith. Responsável por:
- Consumir stream de tokens do lexer
- Construir AST (Abstract Syntax Tree)
- Validar sintaxe da linguagem
- Error recovery para múltiplos erros por parse

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `*.c` | - | Implementação do parser |
| `*.h` | - | Definições de AST nodes |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🔴 CRÍTICA |

## ⚠️ Estado Crítico

- **Token atual**: lookahead do parser
- **AST root**: nodo raiz da árvore
- **Error state**: flag de erro para recovery

## 🔗 Dependencies Externas

- `frontend/lexer/` → Token stream
- `frontend/ast/` → AST node definitions
- `utils/arena.c` → Memory allocation

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- [A preencher após análise detalhada]

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Critical
- Source files: 2
- Extracted symbols: 69

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/frontend/parser/parser.c` | 1740 | 68 | 1 |
| `compiler/frontend/parser/parser.h` | 31 | 1 | 5 |

### Local Dependencies

- `compiler/frontend/ast/model.h`
- `compiler/frontend/lexer/lexer.h`
- `compiler/frontend/parser/parser.h`
- `compiler/semantic/diagnostics/diagnostics.h`
- `compiler/utils/arena.h`
- `compiler/utils/string_pool.h`

### Related Tests

- `tests/frontend/test_parser.c`
- `tests/frontend/test_parser_error_recovery.c`

### Symbol Index

#### `compiler/frontend/parser/parser.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 9 | `struct` | `zt_parser` |
| 26 | `func_def` | `zt_parser_next_non_comment_token` |
| 47 | `func_def` | `zt_parser_advance` |
| 58 | `func_def` | `zt_parser_fill_peek` |
| 65 | `func_def` | `zt_parser_check` |
| 69 | `func_def` | `zt_parser_match` |
| 77 | `func_def` | `zt_parser_is_declaration_start` |
| 96 | `func_def` | `zt_parser_is_member_start` |
| 100 | `func_def` | `zt_parser_sync_to_declaration` |
| 106 | `func_def` | `zt_parser_sync_to_member_or_end` |
| 114 | `func_def` | `zt_parser_expect` |
| 149 | `func_def` | `zt_parser_error_at` |
| 153 | `func_def` | `zt_parser_error_contextual` |
| 160 | `func_def` | `zt_parser_intern_unescaped` |
| 183 | `func_def` | `zt_parser_strdup` |
| 190 | `macro` | `ZT_PARSER_MAX_NAME_PATH_LEN` |
| 192 | `func_def` | `zt_parser_note_name_path_too_long` |
| 203 | `func_def` | `zt_parser_append_to_name_path` |
| 232 | `func_def` | `zt_parser_append_char_to_name_path` |
| 244 | `func_def` | `zt_parser_is_contextual_ident` |
| 248 | `func_def` | `zt_parser_token_is_identifier_text` |
| 255 | `func_def` | `zt_parser_hex_digit_value` |
| 262 | `func_def` | `zt_parser_normalize_hex_bytes` |
| 294 | `func_def` | `zt_parser_ast_make` |
| 311 | `func_decl` | `zt_parser_parse_type` |
| 312 | `func_decl` | `zt_parser_parse_expression` |
| 313 | `func_decl` | `zt_parser_parse_block` |
| 314 | `func_decl` | `zt_parser_parse_statement` |
| 315 | `func_decl` | `zt_parser_parse_params` |
| 316 | `func_decl` | `zt_parser_parse_generic_constraints` |
| 318 | `func_def` | `zt_parser_is_type_name` |
| 334 | `func_def` | `zt_parser_expect_type_name` |
| 341 | `func_def` | `zt_parser_parse_type_name_path` |
| 364 | `func_def` | `zt_parser_parse_type` |
| 404 | `func_def` | `zt_is_named_arg_label_token` |
| 408 | `func_def` | `zt_is_named_arg_ahead` |
| 414 | `func_def` | `zt_parser_parse_primary` |
| 606 | `enum` | `zt_precedence_level` |
| 617 | `func_def` | `zt_is_binary_op` |
| 631 | `func_def` | `zt_binary_precedence` |
| 643 | `func_def` | `zt_parser_parse_postfix` |
| 754 | `func_def` | `zt_parser_parse_binary` |
| 777 | `func_def` | `zt_parser_parse_expression` |
| 781 | `func_def` | `zt_parser_parse_params` |
| 826 | `func_def` | `zt_parser_parse_type_params` |
| 845 | `func_def` | `zt_parser_parse_generic_constraint` |
| 857 | `func_def` | `zt_parser_parse_generic_constraints` |
| 871 | `func_def` | `zt_parser_parse_block_ex` |
| 893 | `func_def` | `zt_parser_parse_block` |
| 897 | `func_def` | `zt_parser_parse_if_stmt` |
| 924 | `func_def` | `zt_parser_parse_while_stmt` |
| 939 | `func_def` | `zt_parser_parse_for_stmt` |
| 964 | `func_def` | `zt_parser_parse_repeat_stmt` |
| 980 | `func_def` | `zt_parser_parse_match_stmt` |
| 1022 | `func_def` | `zt_parser_parse_statement` |
| 1163 | `func_def` | `zt_parser_parse_func_decl` |
| 1203 | `func_def` | `zt_parser_parse_struct_field` |
| 1234 | `func_def` | `zt_parser_parse_struct_decl` |
| 1258 | `func_def` | `zt_parser_parse_trait_method` |
| 1282 | `func_def` | `zt_parser_parse_trait_decl` |
| 1318 | `func_def` | `zt_parser_parse_apply_decl` |
| 1384 | `func_def` | `zt_parser_parse_enum_decl` |
| 1442 | `func_def` | `zt_parser_parse_extern_func` |
| 1469 | `func_def` | `zt_parser_parse_extern_decl` |
| 1499 | `func_def` | `zt_parser_parse_declaration` |
| 1610 | `func_def` | `zt_parser_parse_import` |
| 1642 | `func_def` | `zt_parse` |
| 1731 | `func_def` | `zt_parser_result_dispose` |

#### `compiler/frontend/parser/parser.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_FRONTEND_PARSER_PARSER_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
