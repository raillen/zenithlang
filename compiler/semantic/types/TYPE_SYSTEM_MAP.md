# Type System - Code Map

## 📋 Descrição

Sistema de tipos do Zenith. Responsável por:
- Type checking estático
- Type inference
- Subtyping e variance
- Generic types e monomorphization
- Type errors detalhados

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `*.c` (4 files) | - | Type checking, inference, unification |
| `*.h` (4 files) | - | Type definitions, interfaces |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🔴 CRÍTICA |

## ⚠️ Estado Crítico

- **Type environment**: tipos conhecidos no scope atual
- **Type variables**: variáveis para inference
- **Constraint set**: constraints para unification

## 🔗 Dependencies Externas

- `semantic/binder/` → Symbols com tipos
- `semantic/symbols/` → Symbol lookup
- `zir/model.c` → Type representation na IR

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- EXTREMAMENTE CRÍTICO → core do type system
- 4 arquivos → lógica complexa distribuída
- Test coverage deve ser alta

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Critical
- Source files: 4
- Extracted symbols: 98

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/semantic/types/checker.c` | 2829 | 77 | 2 |
| `compiler/semantic/types/checker.h` | 24 | 1 | 2 |
| `compiler/semantic/types/types.c` | 283 | 19 | 0 |
| `compiler/semantic/types/types.h` | 88 | 1 | 0 |

### Local Dependencies

- `compiler/frontend/ast/model.h`
- `compiler/semantic/diagnostics/diagnostics.h`
- `compiler/semantic/types/checker.h`
- `compiler/semantic/types/types.h`

### Related Tests

- `tests/core/test_types_pro.zt`
- `tests/semantic/test_constraints.c`
- `tests/semantic_tests/test_types_advanced.zt`

### Symbol Index

#### `compiler/semantic/types/checker.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 9 | `enum` | `zt_binding_kind` |
| 14 | `struct` | `zt_binding` |
| 21 | `struct` | `zt_binding_scope` |
| 28 | `struct` | `zt_decl_entry` |
| 33 | `struct` | `zt_import_entry` |
| 38 | `struct` | `zt_decl_list` |
| 44 | `struct` | `zt_import_list` |
| 50 | `struct` | `zt_apply_list` |
| 56 | `struct` | `zt_module_catalog` |
| 62 | `struct` | `zt_checker` |
| 68 | `struct` | `zt_expr_info` |
| 77 | `struct` | `zt_function_context` |
| 84 | `func_def` | `zt_checker_strdup` |
| 96 | `func_def` | `zt_expr_info_dispose` |
| 107 | `func_def` | `zt_expr_info_make` |
| 118 | `func_def` | `zt_binding_scope_init` |
| 126 | `func_def` | `zt_binding_scope_dispose` |
| 141 | `func_def` | `zt_binding_scope_declare` |
| 169 | `func_def` | `zt_binding_scope_lookup` |
| 184 | `func_def` | `zt_decl_list_push` |
| 200 | `func_def` | `zt_import_list_push` |
| 216 | `func_def` | `zt_catalog_init` |
| 221 | `func_def` | `zt_catalog_dispose` |
| 237 | `func_def` | `zt_catalog_find_decl` |
| 246 | `func_def` | `zt_catalog_has_import_alias` |
| 255 | `func_def` | `zt_catalog_import_path_for_alias` |
| 269 | `func_def` | `zt_apply_list_push` |
| 284 | `func_def` | `zt_catalog_build` |
| 341 | `func_def` | `zt_name_has_dot` |
| 345 | `func_def` | `zt_checker_is_self_prefix` |
| 359 | `func_def` | `zt_checker_decl_is_public` |
| 379 | `func_def` | `zt_type_base_name` |
| 383 | `func_def` | `zt_decl_type_param_count` |
| 394 | `func_def` | `zt_builtin_type_by_name` |
| 416 | `func_def` | `zt_type_expected_arity` |
| 432 | `func_def` | `zt_type_describe` |
| 436 | `func_def` | `zt_checker_diag_type` |
| 444 | `func_def` | `zt_parse_signed_literal_value` |
| 462 | `func_def` | `zt_parse_double_literal_value` |
| 480 | `func_def` | `zt_int_literal_fits_kind` |
| 499 | `func_def` | `zt_int_value_fits_kind` |
| 516 | `func_def` | `zt_expr_matches_integral_type` |
| 524 | `func_def` | `zt_checker_compute_integral_binary` |
| 549 | `func_def` | `zt_type_is_core_trait_name` |
| 556 | `func_def` | `zt_type_param_has_trait` |
| 569 | `func_decl` | `zt_checker_type_implements_trait` |
| 570 | `func_decl` | `zt_checker_resolve_type` |
| 571 | `func_decl` | `zt_checker_check_expression` |
| 572 | `func_decl` | `zt_checker_check_block` |
| 574 | `func_def` | `zt_checker_make_core_constraint_type` |
| 580 | `func_def` | `zt_checker_register_type_params` |
| 617 | `func_def` | `zt_checker_resolve_user_type` |
| 663 | `func_def` | `zt_checker_resolve_type` |
| 734 | `func_def` | `zt_checker_type_implements_trait` |
| 801 | `func_def` | `zt_checker_same_or_contextually_assignable` |
| 908 | `func_def` | `zt_checker_is_numeric_conversion_target` |
| 915 | `func_def` | `zt_checker_validate_conversion` |
| 951 | `func_def` | `zt_checker_bind_struct_field_type_params` |
| 963 | `func_def` | `zt_checker_find_struct_field_decl` |
| 974 | `func_def` | `zt_checker_find_enum_variant_decl` |
| 991 | `func_def` | `zt_checker_enum_variant_field_type_node` |
| 998 | `func_def` | `zt_checker_enum_variant_field_name` |
| 1010 | `func_def` | `zt_checker_find_apply_method` |
| 1033 | `func_def` | `zt_checker_build_qualified_name` |
| 1059 | `func_def` | `zt_checker_check_field_expr` |
| 1144 | `func_def` | `zt_checker_expression_is_mutable_target` |
| 1164 | `func_def` | `zt_checker_check_call_expr` |
| 1799 | `func_def` | `zt_checker_check_expression` |
| 2227 | `func_def` | `zt_checker_check_statement` |
| 2584 | `func_def` | `zt_checker_check_block` |
| 2596 | `func_def` | `zt_checker_check_where_clause` |
| 2611 | `func_def` | `zt_checker_check_generic_constraints` |
| 2636 | `func_def` | `zt_checker_check_func_like` |
| 2675 | `func_def` | `zt_checker_seed_module_const_bindings` |
| 2690 | `func_def` | `zt_checker_check_decl` |
| 2803 | `func_def` | `zt_check_file` |
| 2824 | `func_def` | `zt_check_result_dispose` |

#### `compiler/semantic/types/checker.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_SEMANTIC_TYPES_CHECKER_H` |

#### `compiler/semantic/types/types.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 7 | `func_def` | `zt_types_strdup` |
| 19 | `func_def` | `zt_type_kind_name` |
| 59 | `func_def` | `zt_type_list_make` |
| 67 | `func_def` | `zt_type_list_push` |
| 82 | `func_def` | `zt_type_list_dispose` |
| 95 | `func_def` | `zt_type_make` |
| 103 | `func_def` | `zt_type_make_named` |
| 110 | `func_def` | `zt_type_make_with_args` |
| 120 | `func_def` | `zt_type_clone` |
| 133 | `func_def` | `zt_type_dispose` |
| 140 | `func_def` | `zt_type_equals` |
| 155 | `func_def` | `zt_type_append` |
| 169 | `func_def` | `zt_type_format_inner` |
| 216 | `func_def` | `zt_type_format` |
| 224 | `func_def` | `zt_type_is_integral` |
| 242 | `func_def` | `zt_type_is_signed_integral` |
| 256 | `func_def` | `zt_type_is_unsigned_integral` |
| 269 | `func_def` | `zt_type_is_float` |
| 276 | `func_def` | `zt_type_is_numeric` |

#### `compiler/semantic/types/types.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_SEMANTIC_TYPES_TYPES_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
