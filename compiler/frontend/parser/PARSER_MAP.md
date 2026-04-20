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
- Extracted symbols: 68

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/frontend/parser/parser.c` | 1716 | 67 | 1 |
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
| 191 | `macro` | `ZT_PARSER_STACK_BUFFER_THRESHOLD` |
| 193 | `func_def` | `zt_parser_alloc_buffer` |
| 205 | `func_def` | `zt_parser_free_buffer` |
| 211 | `func_def` | `zt_parser_is_contextual_ident` |
| 215 | `func_def` | `zt_parser_token_is_identifier_text` |
| 222 | `func_def` | `zt_parser_hex_digit_value` |
| 229 | `func_def` | `zt_parser_normalize_hex_bytes` |
| 261 | `func_def` | `zt_parser_ast_make` |
| 278 | `func_decl` | `zt_parser_parse_type` |
| 279 | `func_decl` | `zt_parser_parse_expression` |
| 280 | `func_decl` | `zt_parser_parse_block` |
| 281 | `func_decl` | `zt_parser_parse_statement` |
| 282 | `func_decl` | `zt_parser_parse_params` |
| 283 | `func_decl` | `zt_parser_parse_generic_constraints` |
| 285 | `func_def` | `zt_parser_is_type_name` |
| 301 | `func_def` | `zt_parser_expect_type_name` |
| 308 | `func_def` | `zt_parser_parse_type_name_path` |
| 342 | `func_def` | `zt_parser_parse_type` |
| 372 | `func_def` | `zt_is_named_arg_label_token` |
| 376 | `func_def` | `zt_is_named_arg_ahead` |
| 382 | `func_def` | `zt_parser_parse_primary` |
| 574 | `enum` | `zt_precedence_level` |
| 585 | `func_def` | `zt_is_binary_op` |
| 599 | `func_def` | `zt_binary_precedence` |
| 611 | `func_def` | `zt_parser_parse_postfix` |
| 722 | `func_def` | `zt_parser_parse_binary` |
| 745 | `func_def` | `zt_parser_parse_expression` |
| 749 | `func_def` | `zt_parser_parse_params` |
| 789 | `func_def` | `zt_parser_parse_type_params` |
| 808 | `func_def` | `zt_parser_parse_generic_constraint` |
| 820 | `func_def` | `zt_parser_parse_generic_constraints` |
| 834 | `func_def` | `zt_parser_parse_block_ex` |
| 856 | `func_def` | `zt_parser_parse_block` |
| 860 | `func_def` | `zt_parser_parse_if_stmt` |
| 887 | `func_def` | `zt_parser_parse_while_stmt` |
| 902 | `func_def` | `zt_parser_parse_for_stmt` |
| 927 | `func_def` | `zt_parser_parse_repeat_stmt` |
| 943 | `func_def` | `zt_parser_parse_match_stmt` |
| 985 | `func_def` | `zt_parser_parse_statement` |
| 1126 | `func_def` | `zt_parser_parse_func_decl` |
| 1166 | `func_def` | `zt_parser_parse_struct_field` |
| 1197 | `func_def` | `zt_parser_parse_struct_decl` |
| 1221 | `func_def` | `zt_parser_parse_trait_method` |
| 1245 | `func_def` | `zt_parser_parse_trait_decl` |
| 1281 | `func_def` | `zt_parser_parse_apply_decl` |
| 1347 | `func_def` | `zt_parser_parse_enum_decl` |
| 1405 | `func_def` | `zt_parser_parse_extern_func` |
| 1432 | `func_def` | `zt_parser_parse_extern_decl` |
| 1462 | `func_def` | `zt_parser_parse_declaration` |
| 1573 | `func_def` | `zt_parser_parse_import` |
| 1612 | `func_def` | `zt_parse` |
| 1706 | `func_def` | `zt_parser_result_dispose` |

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
