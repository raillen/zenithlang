# HIR Lowering - Code Map

## 📋 Descrição

High-level IR Lowering. Responsável por:
- Transformar AST em HIR
- Desugar constructs sintáticos
- Resolution inicial de nomes
- Preparação para ZIR lowering

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `*.c` | - | Implementação do lowering AST→HIR |
| `*.h` | - | Definições HIR nodes |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟡 MÉDIA |

## ⚠️ Estado Crítico

- **AST input**: árvore sendo transformada
- **HIR output**: resultado do lowering
- **Scope stack**: resolução de nomes

## 🔗 Dependencies Externas

- `frontend/ast/` → Input AST
- `hir/nodes/` → HIR structures
- `semantic/binder/` → Name resolution

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
- Extracted symbols: 66

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/hir/lowering/from_ast.c` | 2029 | 65 | 1 |
| `compiler/hir/lowering/from_ast.h` | 26 | 1 | 3 |

### Local Dependencies

- `compiler/frontend/ast/model.h`
- `compiler/hir/lowering/from_ast.h`
- `compiler/hir/nodes/model.h`
- `compiler/semantic/diagnostics/diagnostics.h`

### Related Tests

- `tests/semantic/test_binder.c`
- `tests/semantic/test_hir_lowering.c`

### Symbol Index

#### `compiler/hir/lowering/from_ast.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 7 | `struct` | `zt_scope_binding` |
| 12 | `struct` | `zt_scope` |
| 19 | `struct` | `zt_struct_field_meta` |
| 25 | `struct` | `zt_struct_meta` |
| 32 | `struct` | `zt_func_param_meta` |
| 38 | `struct` | `zt_func_meta` |
| 49 | `struct` | `zt_method_meta` |
| 56 | `struct` | `zt_const_meta` |
| 63 | `struct` | `zt_lower_ctx` |
| 81 | `func_def` | `zt_lower_strdup` |
| 92 | `func_def` | `zt_lower_join_with_dot` |
| 107 | `func_def` | `zt_lower_grow_array` |
| 117 | `func_def` | `zt_text_eq` |
| 123 | `func_def` | `zt_last_segment` |
| 130 | `func_def` | `zt_name_eq` |
| 135 | `func_def` | `zt_unknown_type` |
| 139 | `func_def` | `zt_add_diag` |
| 149 | `func_def` | `zt_scope_init` |
| 155 | `func_def` | `zt_scope_dispose` |
| 166 | `func_def` | `zt_scope_set` |
| 187 | `func_def` | `zt_scope_get` |
| 199 | `func_def` | `zt_builtin_kind` |
| 222 | `func_decl` | `zt_lower_type_from_ast` |
| 224 | `func_def` | `zt_lower_type_from_generic` |
| 278 | `func_def` | `zt_lower_type_from_ast` |
| 293 | `func_def` | `zt_push_struct_meta` |
| 304 | `func_def` | `zt_push_func_meta` |
| 315 | `func_def` | `zt_push_method_meta` |
| 326 | `func_def` | `zt_push_const_meta` |
| 337 | `func_def` | `zt_push_struct_field_meta` |
| 348 | `func_def` | `zt_push_func_param_meta` |
| 359 | `func_def` | `zt_find_struct_meta` |
| 368 | `func_def` | `zt_find_enum_decl_ast` |
| 379 | `func_def` | `zt_find_enum_variant_ast` |
| 394 | `func_def` | `zt_enum_variant_field_type_node` |
| 401 | `func_def` | `zt_enum_variant_field_name` |
| 413 | `func_def` | `zt_find_struct_field_meta` |
| 422 | `func_def` | `zt_find_func_meta` |
| 436 | `func_def` | `zt_find_method_meta` |
| 450 | `func_def` | `zt_find_const_meta` |
| 459 | `func_def` | `zt_build_apply_name` |
| 470 | `func_def` | `zt_collect_struct_symbols` |
| 489 | `func_def` | `zt_collect_func_symbol` |
| 512 | `func_def` | `zt_collect_apply_symbols` |
| 536 | `func_def` | `zt_collect_const_symbol` |
| 547 | `func_def` | `zt_collect_extern_symbols` |
| 565 | `func_def` | `zt_collect_symbols` |
| 587 | `func_def` | `zt_expr_to_path` |
| 612 | `func_def` | `zt_make_expr` |
| 623 | `func_decl` | `zt_lower_expr` |
| 625 | `func_def` | `zt_lower_call_args` |
| 707 | `func_def` | `zt_lower_struct_constructor` |
| 787 | `func_def` | `zt_lower_enum_unit_variant` |
| 814 | `func_def` | `zt_lower_enum_constructor` |
| 932 | `func_def` | `zt_lower_call_expr` |
| 1163 | `func_def` | `zt_lower_expr` |
| 1504 | `func_decl` | `zt_lower_stmt` |
| 1506 | `func_def` | `zt_lower_block` |
| 1522 | `func_def` | `zt_lower_stmt` |
| 1724 | `func_def` | `zt_lower_struct_decl` |
| 1769 | `func_def` | `zt_lower_enum_decl` |
| 1827 | `func_def` | `zt_lower_function_decl_core` |
| 1907 | `func_def` | `zt_lower_apply_decl` |
| 1930 | `func_def` | `zt_lower_ast_to_hir` |
| 2017 | `func_def` | `zt_hir_lower_result_dispose` |

#### `compiler/hir/lowering/from_ast.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_HIR_LOWERING_FROM_AST_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
