# Semantic Binder - Code Map

## 📋 Descrição

Binder semântico. Responsável por:
- Construir symbol table
- Bind de identifiers para declarations
- Scope management (nested scopes)
- Resolution de nomes em nested blocks
- Detecção de shadowing, redeclaração

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `*.c` | - | Implementação do binder |
| `*.h` | - | Interface e structs |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🔴 CRÍTICA |

## ⚠️ Estado Crítico

- **Symbol table**: map de nomes → symbols
- **Scope stack**: pilha de scopes aninhados
- **Current scope**: scope ativo durante binding

## 🔗 Dependencies Externas

- `semantic/symbols/` → Symbol definitions
- `frontend/ast/` → AST para traversar
- `semantic/types/` → Type checking

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- MUITO CRÍTICO → bugs aqui causam erros em cascata
- Ordem de binding importa (forward declarations)

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Critical
- Source files: 2
- Extracted symbols: 43

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/semantic/binder/binder.c` | 1001 | 42 | 1 |
| `compiler/semantic/binder/binder.h` | 26 | 1 | 3 |

### Local Dependencies

- `compiler/frontend/ast/model.h`
- `compiler/semantic/diagnostics/diagnostics.h`
- `compiler/semantic/parameter_validation.h`
- `compiler/semantic/symbols/symbols.h`

### Related Tests

- `tests/behavior/multifile_duplicate_symbol/src/app/a/foo.zt`
- `tests/behavior/multifile_duplicate_symbol/src/app/b/foo.zt`
- `tests/behavior/multifile_duplicate_symbol/src/app/main.zt`
- `tests/behavior/multifile_import_alias/src/app/main.zt`
- `tests/behavior/multifile_import_alias/src/app/math.zt`
- `tests/behavior/multifile_import_cycle/src/app/main.zt`
- `tests/behavior/multifile_import_cycle/src/app/util.zt`
- `tests/behavior/multifile_missing_import/src/app/main.zt`
- `tests/behavior/multifile_namespace_mismatch/src/app/main.zt`
- `tests/behavior/multifile_namespace_mismatch/src/app/wrong.zt`
- `tests/behavior/multifile_private_access/src/app/main.zt`
- `tests/behavior/multifile_private_access/src/app/secrets.zt`
- `tests/fixtures/diagnostics/multifile_duplicate_symbol.contains.txt`
- `tests/fixtures/diagnostics/multifile_import_cycle.contains.txt`
- `tests/fixtures/diagnostics/multifile_missing_import.contains.txt`
- `tests/fixtures/diagnostics/multifile_namespace_mismatch.contains.txt`
- `tests/fixtures/diagnostics/multifile_private_access.contains.txt`
- `tests/semantic/test_binder.c`

### Symbol Index

#### `compiler/semantic/binder/binder.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 7 | `macro` | `ZT_BIND_MAX_READABLE_BLOCK_DEPTH` |
| 8 | `macro` | `ZT_BIND_MAX_READABLE_FUNCTION_STATEMENTS` |
| 10 | `struct` | `zt_binder` |
| 15 | `func_def` | `zt_scope_collect_names` |
| 29 | `func_def` | `zt_bind_emit_unresolved_with_suggestion` |
| 47 | `func_def` | `zt_bind_warn_confusing_name` |
| 90 | `func_def` | `zt_bind_normalize_name` |
| 107 | `func_def` | `zt_bind_is_prefix_pair` |
| 122 | `func_def` | `zt_bind_edit_distance_at_most_one` |
| 158 | `func_def` | `zt_bind_names_are_too_similar` |
| 172 | `func_def` | `zt_bind_warn_similar_name` |
| 196 | `func_def` | `zt_is_builtin_type_name` |
| 230 | `func_def` | `zt_is_intrinsic_name` |
| 240 | `func_def` | `zt_import_local_name` |
| 247 | `func_def` | `zt_bind_declare_name` |
| 267 | `func_decl` | `zt_bind_expression` |
| 268 | `func_decl` | `zt_bind_type_node` |
| 269 | `func_decl` | `zt_bind_block` |
| 271 | `func_def` | `zt_bind_split_prefix` |
| 284 | `func_def` | `zt_bind_symbol_is_value_like` |
| 288 | `func_def` | `zt_bind_seed_module_value_aliases` |
| 317 | `func_def` | `zt_bind_simple_type_name` |
| 347 | `func_def` | `zt_bind_type_node` |
| 366 | `func_def` | `zt_bind_generic_constraints` |
| 388 | `func_def` | `zt_bind_expr_list` |
| 396 | `func_def` | `zt_bind_expression` |
| 488 | `func_decl` | `zt_bind_expression` |
| 490 | `func_def` | `zt_bind_match_pattern` |
| 556 | `func_def` | `zt_bind_statement` |
| 646 | `func_def` | `zt_bind_block` |
| 672 | `func_def` | `zt_bind_type_param_defs` |
| 683 | `func_decl` | `zt_bind_where_clause` |
| 685 | `func_def` | `zt_bind_param_list` |
| 698 | `func_def` | `zt_bind_where_clause` |
| 709 | `func_decl` | `zt_bind_statement_count` |
| 711 | `func_def` | `zt_bind_block_statement_count` |
| 724 | `func_def` | `zt_bind_statement_count` |
| 760 | `func_def` | `zt_bind_warn_function_too_long` |
| 782 | `func_def` | `zt_bind_decl` |
| 914 | `func_def` | `zt_bind_declare_top_level` |
| 951 | `func_def` | `zt_bind_file` |
| 989 | `func_def` | `zt_bind_result_dispose` |

#### `compiler/semantic/binder/binder.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_SEMANTIC_BINDER_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
