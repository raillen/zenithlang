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
- Extracted symbols: 132

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/semantic/types/checker.c` | 4437 | 106 | 2 |
| `compiler/semantic/types/checker.h` | 25 | 1 | 3 |
| `compiler/semantic/types/types.c` | 359 | 24 | 0 |
| `compiler/semantic/types/types.h` | 103 | 1 | 0 |

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
| 13 | `enum` | `zt_binding_kind` |
| 18 | `struct` | `zt_binding` |
| 25 | `struct` | `zt_binding_scope` |
| 32 | `struct` | `zt_decl_entry` |
| 37 | `struct` | `zt_decl_index_entry` |
| 42 | `struct` | `zt_import_entry` |
| 47 | `struct` | `zt_decl_list` |
| 53 | `struct` | `zt_import_list` |
| 59 | `struct` | `zt_apply_list` |
| 65 | `struct` | `zt_module_catalog` |
| 74 | `struct` | `zt_checker` |
| 80 | `struct` | `zt_transfer_stack` |
| 86 | `struct` | `zt_expr_info` |
| 95 | `struct` | `zt_function_context` |
| 103 | `func_def` | `zt_checker_strdup` |
| 115 | `func_def` | `zt_expr_info_dispose` |
| 126 | `func_def` | `zt_expr_info_make` |
| 137 | `func_def` | `zt_binding_scope_init` |
| 145 | `func_def` | `zt_binding_scope_dispose` |
| 160 | `func_def` | `zt_binding_scope_declare` |
| 188 | `func_def` | `zt_binding_scope_lookup` |
| 203 | `func_def` | `zt_binding_scope_lookup_extended` |
| 220 | `func_def` | `zt_decl_list_push` |
| 236 | `func_def` | `zt_import_list_push` |
| 252 | `func_def` | `zt_catalog_init` |
| 257 | `func_def` | `zt_catalog_dispose` |
| 274 | `func_def` | `zt_transfer_stack_init` |
| 281 | `func_def` | `zt_transfer_stack_dispose` |
| 294 | `func_def` | `zt_transfer_stack_contains` |
| 306 | `func_def` | `zt_transfer_stack_push` |
| 325 | `func_def` | `zt_transfer_stack_pop` |
| 332 | `func_def` | `zt_catalog_hash_name` |
| 347 | `func_def` | `zt_catalog_build_decl_index` |
| 389 | `func_def` | `zt_catalog_find_decl` |
| 413 | `func_def` | `zt_catalog_has_import_alias` |
| 422 | `func_def` | `zt_catalog_import_path_for_alias` |
| 436 | `func_def` | `zt_apply_list_push` |
| 451 | `func_def` | `zt_catalog_build` |
| 515 | `func_def` | `zt_name_has_dot` |
| 519 | `func_def` | `zt_checker_is_self_prefix` |
| 533 | `func_def` | `zt_checker_decl_is_public` |
| 555 | `func_def` | `zt_type_base_name` |
| 559 | `func_def` | `zt_decl_type_param_count` |
| 570 | `func_def` | `zt_builtin_type_by_name` |
| 596 | `func_def` | `zt_type_expected_arity` |
| 613 | `func_def` | `zt_type_describe` |
| 617 | `func_def` | `zt_checker_diag_type` |
| 625 | `func_def` | `zt_checker_append_text` |
| 640 | `func_def` | `zt_checker_diag_callable_signature` |
| 656 | `func_def` | `zt_checker_diag_invalid_callable_ref_expected` |
| 670 | `func_def` | `zt_checker_format_missing_enum_cases` |
| 694 | `func_def` | `zt_checker_add_dyn_static_dispatch_note` |
| 704 | `func_def` | `zt_parse_signed_literal_value` |
| 723 | `func_def` | `zt_parse_double_literal_value` |
| 744 | `func_def` | `zt_int_literal_fits_kind` |
| 763 | `func_def` | `zt_int_value_fits_kind` |
| 780 | `func_def` | `zt_expr_matches_integral_type` |
| 788 | `func_def` | `zt_try_add_i64` |
| 798 | `func_def` | `zt_try_sub_i64` |
| 808 | `func_def` | `zt_try_mul_i64` |
| 841 | `func_def` | `zt_checker_compute_integral_binary` |
| 865 | `func_def` | `zt_type_is_core_trait_name` |
| 873 | `func_def` | `zt_checker_split_prefix` |
| 886 | `func_def` | `zt_type_param_has_trait` |
| 899 | `func_decl` | `zt_checker_type_implements_trait` |
| 900 | `func_decl` | `zt_checker_resolve_type` |
| 901 | `func_decl` | `zt_checker_check_expression` |
| 902 | `func_decl` | `zt_checker_check_block` |
| 903 | `func_decl` | `zt_checker_type_is_transferable_inner` |
| 907 | `func_def` | `zt_checker_validate_dyn_trait` |
| 1014 | `func_def` | `zt_checker_make_core_constraint_type` |
| 1020 | `func_def` | `zt_checker_register_type_params` |
| 1057 | `func_def` | `zt_checker_resolve_user_type` |
| 1108 | `func_def` | `zt_checker_resolve_type` |
| 1232 | `func_def` | `zt_checker_type_implements_trait` |
| 1305 | `func_def` | `zt_checker_same_or_contextually_assignable` |
| 1412 | `func_def` | `zt_checker_is_numeric_conversion_target` |
| 1419 | `func_def` | `zt_checker_validate_conversion` |
| 1455 | `func_def` | `zt_checker_make_std_concurrent_copy_type` |
| 1488 | `func_def` | `zt_checker_bind_struct_field_type_params` |
| 1500 | `func_def` | `zt_decl_type_params` |
| 1509 | `func_def` | `zt_checker_bind_decl_type_params` |
| 1530 | `func_def` | `zt_checker_find_struct_field_decl` |
| 1541 | `func_def` | `zt_checker_find_enum_variant_decl` |
| 1558 | `func_def` | `zt_checker_enum_variant_field_type_node` |
| 1565 | `func_def` | `zt_checker_enum_variant_field_name` |
| 1577 | `func_def` | `zt_checker_find_apply_method` |
| 1600 | `func_def` | `zt_checker_type_is_transferable_user_decl` |
| 1662 | `func_def` | `zt_checker_type_is_transferable_inner` |
| 1725 | `func_def` | `zt_checker_build_qualified_name` |
| 1751 | `func_def` | `zt_checker_lookup_import_member` |
| 1785 | `func_def` | `zt_checker_check_field_expr` |
| 1876 | `func_def` | `zt_checker_expression_is_mutable_target` |
| 1910 | `func_def` | `zt_checker_check_call_expr` |
| 2943 | `func_def` | `zt_checker_check_expression` |
| 3510 | `func_def` | `zt_checker_check_statement` |
| 4085 | `func_def` | `zt_checker_check_block` |
| 4097 | `func_def` | `zt_checker_check_where_clause` |
| 4112 | `func_def` | `zt_checker_check_generic_constraints` |
| 4137 | `func_def` | `zt_checker_check_func_like` |
| 4178 | `func_def` | `zt_checker_seed_module_value_bindings` |
| 4206 | `func_def` | `zt_checker_seed_module_value_aliases` |
| 4252 | `func_def` | `zt_checker_check_decl` |
| 4387 | `func_def` | `zt_check_file` |
| 4408 | `func_def` | `zt_checker_type_is_transferable` |
| 4432 | `func_def` | `zt_check_result_dispose` |

#### `compiler/semantic/types/checker.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_SEMANTIC_TYPES_CHECKER_H` |

#### `compiler/semantic/types/types.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 7 | `func_def` | `zt_types_strdup` |
| 19 | `func_def` | `zt_type_kind_name` |
| 61 | `func_def` | `zt_type_list_make` |
| 69 | `func_def` | `zt_type_list_push` |
| 84 | `func_def` | `zt_type_list_dispose` |
| 97 | `func_def` | `zt_type_make` |
| 105 | `func_def` | `zt_type_make_named` |
| 112 | `func_def` | `zt_type_make_with_args` |
| 122 | `func_def` | `zt_type_clone` |
| 135 | `func_def` | `zt_type_dispose` |
| 142 | `func_def` | `zt_type_equals` |
| 157 | `func_def` | `zt_type_append` |
| 171 | `func_def` | `zt_type_format_inner` |
| 232 | `func_def` | `zt_type_format` |
| 240 | `func_def` | `zt_type_is_integral` |
| 258 | `func_def` | `zt_type_is_signed_integral` |
| 272 | `func_def` | `zt_type_is_unsigned_integral` |
| 285 | `func_def` | `zt_type_is_float` |
| 292 | `func_def` | `zt_type_is_numeric` |
| 296 | `func_def` | `zt_type_make_callable` |
| 321 | `func_def` | `zt_type_callable_return` |
| 327 | `func_def` | `zt_type_callable_param_count` |
| 333 | `func_def` | `zt_type_callable_param` |
| 339 | `func_def` | `zt_type_callable_signatures_equal` |

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
