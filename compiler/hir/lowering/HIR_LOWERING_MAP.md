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
- Extracted symbols: 64

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/hir/lowering/from_ast.c` | 1970 | 63 | 1 |
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
| 46 | `struct` | `zt_method_meta` |
| 53 | `struct` | `zt_const_meta` |
| 60 | `struct` | `zt_lower_ctx` |
| 78 | `func_def` | `zt_lower_strdup` |
| 89 | `func_def` | `zt_lower_grow_array` |
| 99 | `func_def` | `zt_text_eq` |
| 105 | `func_def` | `zt_last_segment` |
| 112 | `func_def` | `zt_name_eq` |
| 117 | `func_def` | `zt_unknown_type` |
| 121 | `func_def` | `zt_add_diag` |
| 131 | `func_def` | `zt_scope_init` |
| 137 | `func_def` | `zt_scope_dispose` |
| 148 | `func_def` | `zt_scope_set` |
| 169 | `func_def` | `zt_scope_get` |
| 181 | `func_def` | `zt_builtin_kind` |
| 204 | `func_decl` | `zt_lower_type_from_ast` |
| 206 | `func_def` | `zt_lower_type_from_generic` |
| 260 | `func_def` | `zt_lower_type_from_ast` |
| 275 | `func_def` | `zt_push_struct_meta` |
| 286 | `func_def` | `zt_push_func_meta` |
| 297 | `func_def` | `zt_push_method_meta` |
| 308 | `func_def` | `zt_push_const_meta` |
| 319 | `func_def` | `zt_push_struct_field_meta` |
| 330 | `func_def` | `zt_push_func_param_meta` |
| 341 | `func_def` | `zt_find_struct_meta` |
| 350 | `func_def` | `zt_find_enum_decl_ast` |
| 361 | `func_def` | `zt_find_enum_variant_ast` |
| 376 | `func_def` | `zt_enum_variant_field_type_node` |
| 383 | `func_def` | `zt_enum_variant_field_name` |
| 395 | `func_def` | `zt_find_struct_field_meta` |
| 404 | `func_def` | `zt_find_func_meta` |
| 413 | `func_def` | `zt_find_method_meta` |
| 427 | `func_def` | `zt_find_const_meta` |
| 436 | `func_def` | `zt_build_apply_name` |
| 447 | `func_def` | `zt_collect_struct_symbols` |
| 466 | `func_def` | `zt_collect_func_symbol` |
| 488 | `func_def` | `zt_collect_apply_symbols` |
| 512 | `func_def` | `zt_collect_const_symbol` |
| 523 | `func_def` | `zt_collect_symbols` |
| 543 | `func_def` | `zt_expr_to_path` |
| 568 | `func_def` | `zt_make_expr` |
| 579 | `func_decl` | `zt_lower_expr` |
| 581 | `func_def` | `zt_lower_call_args` |
| 663 | `func_def` | `zt_lower_struct_constructor` |
| 743 | `func_def` | `zt_lower_enum_unit_variant` |
| 770 | `func_def` | `zt_lower_enum_constructor` |
| 888 | `func_def` | `zt_lower_call_expr` |
| 1106 | `func_def` | `zt_lower_expr` |
| 1447 | `func_decl` | `zt_lower_stmt` |
| 1449 | `func_def` | `zt_lower_block` |
| 1465 | `func_def` | `zt_lower_stmt` |
| 1667 | `func_def` | `zt_lower_struct_decl` |
| 1712 | `func_def` | `zt_lower_enum_decl` |
| 1770 | `func_def` | `zt_lower_function_decl_core` |
| 1850 | `func_def` | `zt_lower_apply_decl` |
| 1873 | `func_def` | `zt_lower_ast_to_hir` |
| 1958 | `func_def` | `zt_hir_lower_result_dispose` |

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
