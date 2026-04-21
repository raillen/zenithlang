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
| `compiler/hir/lowering/from_ast.c` | 2035 | 65 | 1 |
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
| 299 | `func_def` | `zt_push_struct_meta` |
| 310 | `func_def` | `zt_push_func_meta` |
| 321 | `func_def` | `zt_push_method_meta` |
| 332 | `func_def` | `zt_push_const_meta` |
| 343 | `func_def` | `zt_push_struct_field_meta` |
| 354 | `func_def` | `zt_push_func_param_meta` |
| 365 | `func_def` | `zt_find_struct_meta` |
| 374 | `func_def` | `zt_find_enum_decl_ast` |
| 385 | `func_def` | `zt_find_enum_variant_ast` |
| 400 | `func_def` | `zt_enum_variant_field_type_node` |
| 407 | `func_def` | `zt_enum_variant_field_name` |
| 419 | `func_def` | `zt_find_struct_field_meta` |
| 428 | `func_def` | `zt_find_func_meta` |
| 442 | `func_def` | `zt_find_method_meta` |
| 456 | `func_def` | `zt_find_const_meta` |
| 465 | `func_def` | `zt_build_apply_name` |
| 476 | `func_def` | `zt_collect_struct_symbols` |
| 495 | `func_def` | `zt_collect_func_symbol` |
| 518 | `func_def` | `zt_collect_apply_symbols` |
| 542 | `func_def` | `zt_collect_const_symbol` |
| 553 | `func_def` | `zt_collect_extern_symbols` |
| 571 | `func_def` | `zt_collect_symbols` |
| 593 | `func_def` | `zt_expr_to_path` |
| 618 | `func_def` | `zt_make_expr` |
| 629 | `func_decl` | `zt_lower_expr` |
| 631 | `func_def` | `zt_lower_call_args` |
| 713 | `func_def` | `zt_lower_struct_constructor` |
| 793 | `func_def` | `zt_lower_enum_unit_variant` |
| 820 | `func_def` | `zt_lower_enum_constructor` |
| 938 | `func_def` | `zt_lower_call_expr` |
| 1169 | `func_def` | `zt_lower_expr` |
| 1510 | `func_decl` | `zt_lower_stmt` |
| 1512 | `func_def` | `zt_lower_block` |
| 1528 | `func_def` | `zt_lower_stmt` |
| 1730 | `func_def` | `zt_lower_struct_decl` |
| 1775 | `func_def` | `zt_lower_enum_decl` |
| 1833 | `func_def` | `zt_lower_function_decl_core` |
| 1913 | `func_def` | `zt_lower_apply_decl` |
| 1936 | `func_def` | `zt_lower_ast_to_hir` |
| 2023 | `func_def` | `zt_hir_lower_result_dispose` |

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
