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
- Extracted symbols: 27

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/semantic/binder/binder.c` | 684 | 26 | 0 |
| `compiler/semantic/binder/binder.h` | 26 | 1 | 3 |

### Local Dependencies

- `compiler/frontend/ast/model.h`
- `compiler/semantic/diagnostics/diagnostics.h`
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
| 5 | `struct` | `zt_binder` |
| 9 | `func_def` | `zt_scope_collect_names` |
| 23 | `func_def` | `zt_bind_emit_unresolved_with_suggestion` |
| 41 | `func_def` | `zt_bind_warn_confusing_name` |
| 83 | `func_def` | `zt_is_builtin_type_name` |
| 115 | `func_def` | `zt_import_local_name` |
| 122 | `func_def` | `zt_bind_declare_name` |
| 140 | `func_decl` | `zt_bind_expression` |
| 141 | `func_decl` | `zt_bind_type_node` |
| 142 | `func_decl` | `zt_bind_block` |
| 144 | `func_def` | `zt_bind_simple_type_name` |
| 174 | `func_def` | `zt_bind_type_node` |
| 193 | `func_def` | `zt_bind_generic_constraints` |
| 215 | `func_def` | `zt_bind_expr_list` |
| 223 | `func_def` | `zt_bind_expression` |
| 303 | `func_decl` | `zt_bind_expression` |
| 305 | `func_def` | `zt_bind_match_pattern` |
| 349 | `func_def` | `zt_bind_statement` |
| 439 | `func_def` | `zt_bind_block` |
| 452 | `func_def` | `zt_bind_type_param_defs` |
| 463 | `func_def` | `zt_bind_param_list` |
| 475 | `func_def` | `zt_bind_where_clause` |
| 486 | `func_def` | `zt_bind_decl` |
| 601 | `func_def` | `zt_bind_declare_top_level` |
| 635 | `func_def` | `zt_bind_file` |
| 672 | `func_def` | `zt_bind_result_dispose` |

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
