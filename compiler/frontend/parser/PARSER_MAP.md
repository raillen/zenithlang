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
- Extracted symbols: 71

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/frontend/parser/parser.c` | 1766 | 70 | 1 |
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
| 159 | `func_decl` | `zt_parser_ast_make` |
| 161 | `func_def` | `zt_parser_intern_unescaped` |
| 184 | `func_def` | `zt_parser_make_string_expr_from_token` |
| 205 | `func_def` | `zt_parser_strdup` |
| 212 | `macro` | `ZT_PARSER_MAX_NAME_PATH_LEN` |
| 214 | `func_def` | `zt_parser_note_name_path_too_long` |
| 225 | `func_def` | `zt_parser_append_to_name_path` |
| 254 | `func_def` | `zt_parser_append_char_to_name_path` |
| 266 | `func_def` | `zt_parser_is_contextual_ident` |
| 270 | `func_def` | `zt_parser_token_is_identifier_text` |
| 277 | `func_def` | `zt_parser_hex_digit_value` |
| 284 | `func_def` | `zt_parser_normalize_hex_bytes` |
| 316 | `func_def` | `zt_parser_ast_make` |
| 333 | `func_decl` | `zt_parser_parse_type` |
| 334 | `func_decl` | `zt_parser_parse_expression` |
| 335 | `func_decl` | `zt_parser_parse_block` |
| 336 | `func_decl` | `zt_parser_parse_statement` |
| 337 | `func_decl` | `zt_parser_parse_params` |
| 338 | `func_decl` | `zt_parser_parse_generic_constraints` |
| 340 | `func_def` | `zt_parser_is_type_name` |
| 356 | `func_def` | `zt_parser_expect_type_name` |
| 363 | `func_def` | `zt_parser_parse_type_name_path` |
| 386 | `func_def` | `zt_parser_parse_type` |
| 432 | `func_def` | `zt_is_named_arg_label_token` |
| 436 | `func_def` | `zt_is_named_arg_ahead` |
| 442 | `func_def` | `zt_parser_parse_primary` |
| 633 | `enum` | `zt_precedence_level` |
| 644 | `func_def` | `zt_is_binary_op` |
| 658 | `func_def` | `zt_binary_precedence` |
| 670 | `func_def` | `zt_parser_parse_postfix` |
| 781 | `func_def` | `zt_parser_parse_binary` |
| 804 | `func_def` | `zt_parser_parse_expression` |
| 808 | `func_def` | `zt_parser_parse_params` |
| 853 | `func_def` | `zt_parser_parse_type_params` |
| 872 | `func_def` | `zt_parser_parse_generic_constraint` |
| 884 | `func_def` | `zt_parser_parse_generic_constraints` |
| 898 | `func_def` | `zt_parser_parse_block_ex` |
| 920 | `func_def` | `zt_parser_parse_block` |
| 924 | `func_def` | `zt_parser_parse_if_stmt` |
| 951 | `func_def` | `zt_parser_parse_while_stmt` |
| 966 | `func_def` | `zt_parser_parse_for_stmt` |
| 991 | `func_def` | `zt_parser_parse_repeat_stmt` |
| 1007 | `func_def` | `zt_parser_parse_match_stmt` |
| 1049 | `func_def` | `zt_parser_parse_statement` |
| 1190 | `func_def` | `zt_parser_parse_func_decl` |
| 1230 | `func_def` | `zt_parser_parse_struct_field` |
| 1261 | `func_def` | `zt_parser_parse_struct_decl` |
| 1285 | `func_def` | `zt_parser_parse_trait_method` |
| 1309 | `func_def` | `zt_parser_parse_trait_decl` |
| 1345 | `func_def` | `zt_parser_parse_apply_decl` |
| 1411 | `func_def` | `zt_parser_parse_enum_decl` |
| 1469 | `func_def` | `zt_parser_parse_extern_func` |
| 1496 | `func_def` | `zt_parser_parse_extern_decl` |
| 1526 | `func_def` | `zt_parser_parse_declaration` |
| 1637 | `func_def` | `zt_parser_parse_import` |
| 1669 | `func_def` | `zt_parse` |
| 1758 | `func_def` | `zt_parser_result_dispose` |

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
