# C Emitter - Code Map

## 📋 Descrição

Code generator para C. Responsável por:
- Traduzir ZIR para código C
- Emissão de funções, variables, types
- Handling de calling conventions
- Geração de headers
- Otimizações específicas de C

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `emitter.c` | - | Implementação do emitter |
| `emitter.h` | - | Interface pública |
| `legalization.c` | - | Transformações pré-emissão |
| `legalization.h` | - | Legalization interfaces |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🔴 CRÍTICA |

## ⚠️ Estado Crítico

- **Output buffer**: código C sendo gerado
- **Symbol mapping**: ZIR symbols → C identifiers
- **Type mapping**: ZIR types → C types

## 🔗 Dependencies Externas

- `zir/model.c` → ZIR input
- `zir/lowering/` → IR transformations
- `runtime/c/` → Runtime functions

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- CRÍTICO → é o backend principal
- Código gerado deve compilar com C standard
- Debugging difícil → código gerado é indireto

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Critical
- Source files: 4
- Extracted symbols: 203

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/targets/c/emitter.c` | 6856 | 176 | 2 |
| `compiler/targets/c/emitter.h` | 52 | 1 | 1 |
| `compiler/targets/c/legalization.c` | 741 | 25 | 1 |
| `compiler/targets/c/legalization.h` | 85 | 1 | 1 |

### Local Dependencies

- `compiler/targets/c/emitter.h`
- `compiler/targets/c/legalization.h`
- `compiler/zir/model.h`

### Related Tests

- `tests/conformance/test_m10.c`
- `tests/conformance/test_m11.c`
- `tests/conformance/test_m12.c`
- `tests/conformance/test_m13.c`
- `tests/conformance/test_m14.c`
- `tests/conformance/test_m16.c`
- `tests/conformance/test_m9.c`
- `tests/targets/c/README.md`
- `tests/targets/c/test_emitter.c`
- `tests/targets/c/test_enum_payload.c`
- `tests/targets/c/test_legalization.c`

### Symbol Index

#### `compiler/targets/c/emitter.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 9 | `struct` | `c_local_decl` |
| 15 | `macro` | `ZT_EMITTER_STACK_BUFFER_THRESHOLD` |
| 17 | `func_def` | `c_emitter_alloc_buffer` |
| 29 | `func_def` | `c_emitter_free_buffer` |
| 36 | `func_def` | `c_buffer_mark` |
| 40 | `func_def` | `c_buffer_truncate` |
| 48 | `func_def` | `c_canonicalize_type` |
| 79 | `func_def` | `c_safe_text` |
| 83 | `func_def` | `c_emit_set_result` |
| 103 | `func_def` | `c_emit_result_init` |
| 107 | `func_def` | `c_emit_error_code_name` |
| 124 | `func_def` | `c_buffer_reserve` |
| 148 | `func_def` | `c_buffer_append_n` |
| 159 | `func_def` | `c_buffer_append` |
| 164 | `func_def` | `c_buffer_append_vformat` |
| 185 | `func_def` | `c_buffer_append_format` |
| 195 | `func_def` | `c_emitter_init` |
| 202 | `func_def` | `c_emitter_reset` |
| 209 | `func_def` | `c_emitter_dispose` |
| 216 | `func_def` | `c_emitter_text` |
| 220 | `func_def` | `c_begin_line` |
| 227 | `func_def` | `c_emit_line` |
| 231 | `func_def` | `c_copy_sanitized` |
| 259 | `func_def` | `c_find_user_struct` |
| 271 | `func_def` | `c_find_user_enum` |
| 284 | `func_def` | `c_find_function_decl` |
| 297 | `func_def` | `c_build_function_symbol` |
| 306 | `func_def` | `c_build_struct_symbol` |
| 314 | `func_def` | `c_build_enum_symbol` |
| 323 | `func_def` | `c_find_struct_field` |
| 336 | `func_def` | `c_find_enum_variant_field` |
| 348 | `func_def` | `c_build_block_label` |
| 354 | `func_def` | `c_is_blank` |
| 371 | `func_def` | `c_is_identifier_start` |
| 377 | `func_def` | `c_is_identifier_char` |
| 381 | `func_def` | `c_is_identifier_only` |
| 399 | `func_def` | `c_copy_trimmed` |
| 431 | `func_def` | `c_copy_trimmed_segment` |
| 451 | `enum` | `c_type_category` |
| 458 | `struct` | `c_type_mapping` |
| 527 | `macro` | `C_TYPE_TABLE_COUNT` |
| 530 | `func_def` | `c_type_lookup` |
| 561 | `func_def` | `c_levenshtein_distance` |
| 586 | `func_def` | `c_type_suggest_closest` |
| 610 | `func_def` | `c_type_is_managed` |
| 616 | `func_def` | `c_type_is` |
| 621 | `func_def` | `c_parse_outcome_type_name` |
| 670 | `func_def` | `c_build_generated_outcome_symbol` |
| 677 | `func_def` | `c_type_is_builtin_managed_value` |
| 697 | `func_decl` | `c_type_is_struct_with_managed_fields` |
| 699 | `func_def` | `c_struct_has_managed_fields` |
| 716 | `func_def` | `c_type_is_struct_with_managed_fields` |
| 720 | `func_def` | `c_type_needs_managed_cleanup` |
| 726 | `func_def` | `c_type_to_c` |
| 788 | `func_def` | `c_emit_trimmed_text` |
| 798 | `func_def` | `c_emit_c_string_literal` |
| 835 | `func_def` | `c_hex_digit_value` |
| 842 | `func_def` | `c_emit_hex_bytes_literal` |
| 870 | `func_def` | `c_emit_typed_name` |
| 880 | `func_def` | `c_find_symbol_type` |
| 909 | `func_def` | `c_copy_trimmed_alloc` |
| 915 | `func_def` | `c_expression_is_text` |
| 935 | `func_def` | `c_expression_is_materialized_text_ref` |
| 949 | `func_def` | `c_expression_is_materialized_bytes_ref` |
| 963 | `func_def` | `c_expression_is_materialized_net_connection_ref` |
| 976 | `func_def` | `c_expression_is_materialized_core_error_ref` |
| 990 | `func_def` | `c_expression_is_materialized_list_i64_ref` |
| 1004 | `func_def` | `c_expression_is_materialized_map_text_text_ref` |
| 1018 | `func_def` | `c_expression_is_materialized_optional_text_ref` |
| 1032 | `func_def` | `c_expression_is_materialized_optional_bytes_ref` |
| 1046 | `func_def` | `c_expression_is_materialized_optional_list_i64_ref` |
| 1060 | `func_def` | `c_expression_is_materialized_outcome_i64_text_ref` |
| 1074 | `func_def` | `c_expression_is_materialized_outcome_void_text_ref` |
| 1088 | `func_def` | `c_expression_is_materialized_outcome_text_text_ref` |
| 1102 | `func_def` | `c_expression_is_materialized_outcome_optional_text_text_ref` |
| 1116 | `func_def` | `c_expression_is_materialized_outcome_bytes_text_ref` |
| 1130 | `func_def` | `c_expression_is_materialized_outcome_optional_bytes_text_ref` |
| 1144 | `func_def` | `c_expression_is_materialized_outcome_net_connection_text_ref` |
| 1158 | `func_def` | `c_expression_is_materialized_outcome_list_i64_text_ref` |
| 1172 | `func_def` | `c_expression_is_materialized_outcome_map_text_text_ref` |
| 1186 | `struct` | `c_outcome_spec` |
| 1206 | `func_def` | `c_outcome_spec_for_type` |
| 1262 | `func_def` | `c_outcome_spec_for_expr` |
| 1280 | `func_def` | `c_outcome_spec_for_expected` |
| 1288 | `func_def` | `c_type_requires_generated_outcome_helper` |
| 1293 | `func_def` | `c_module_requires_string_header` |
| 1342 | `func_def` | `c_expression_is_copyable_managed_value_ref` |
| 1356 | `func_def` | `c_outcome_success_value_is_supported` |
| 1372 | `func_decl` | `c_emit_expr` |
| 1380 | `func_def` | `c_outcome_emit_failure_from_error_expr` |
| 1454 | `func_def` | `c_emit_value` |
| 1458 | `func_def` | `c_parse_binary` |
| 1482 | `func_def` | `c_split_two_operands` |
| 1493 | `func_def` | `c_split_three_operands` |
| 1518 | `func_def` | `c_segment_is_blank` |
| 1529 | `func_def` | `c_find_top_level_comma` |
| 1592 | `func_def` | `c_find_top_level_colon` |
| 1655 | `func_def` | `c_math_function` |
| 1664 | `func_def` | `c_binary_operator` |
| 1676 | `func_def` | `c_binary_operator_suggestion` |
| 1682 | `func_def` | `c_unary_operator` |
| 1688 | `func_decl` | `c_emit_expr` |
| 1696 | `func_def` | `c_copy_legalize_result` |
| 1719 | `func_def` | `c_emit_legalized_seq_expr` |
| 1757 | `func_def` | `c_emit_owned_managed_expr` |
| 1804 | `func_def` | `c_emit_make_list_i64_expr` |
| 1864 | `func_def` | `c_emit_make_list_text_expr` |
| 1934 | `func_def` | `c_emit_make_map_text_text_expr` |
| 2043 | `func_def` | `c_emit_call_args` |
| 2101 | `func_def` | `c_emit_call_expr` |
| 2159 | `func_def` | `c_emit_expr` |
| 2813 | `func_decl` | `c_emit_owned_managed_zir_expr` |
| 2821 | `func_decl` | `c_emit_zir_expr` |
| 2829 | `func_def` | `c_emit_zir_expr_as_legacy` |
| 2850 | `func_def` | `c_zir_expr_name_text` |
| 2859 | `func_def` | `c_zir_expr_resolve_type` |
| 3067 | `func_def` | `c_zir_expr_is_text` |
| 3076 | `func_def` | `c_zir_expr_is_materialized_text_ref` |
| 3085 | `func_def` | `c_zir_expr_is_materialized_bytes_ref` |
| 3094 | `func_def` | `c_zir_expr_is_materialized_list_i64_ref` |
| 3103 | `func_def` | `c_zir_expr_is_materialized_optional_text_ref` |
| 3112 | `func_def` | `c_zir_expr_is_materialized_optional_bytes_ref` |
| 3120 | `func_def` | `c_zir_expr_is_materialized_optional_list_i64_ref` |
| 3129 | `func_def` | `c_emit_zir_call_expr` |
| 3286 | `func_def` | `c_extern_call_expected_arg_type` |
| 3300 | `func_def` | `c_zir_call_extern_needs_ffi_shield` |
| 3325 | `func_def` | `c_legacy_call_extern_needs_ffi_shield` |
| 3383 | `func_def` | `c_emit_ffi_shield_retain` |
| 3388 | `func_def` | `c_emit_ffi_shield_release` |
| 3393 | `func_def` | `c_emit_zir_ffi_call_arg` |
| 3430 | `func_def` | `c_emit_legacy_ffi_call_arg` |
| 3467 | `func_def` | `c_emit_ffi_shielded_zir_call_statement` |
| 3613 | `func_def` | `c_emit_ffi_shielded_legacy_call_statement` |
| 3815 | `func_def` | `c_emit_ffi_shielded_zir_return` |
| 3866 | `func_def` | `c_is_mutating_self_expr` |
| 3875 | `func_def` | `c_find_make_struct_field_init` |
| 3887 | `func_def` | `c_emit_zir_make_struct_expr` |
| 4021 | `func_def` | `c_emit_zir_make_list_expr` |
| 4091 | `func_def` | `c_emit_zir_index_expr` |
| 4187 | `func_def` | `c_emit_zir_slice_expr` |
| 4254 | `func_decl` | `c_emit_owned_managed_zir_expr` |
| 4262 | `func_decl` | `c_emit_zir_expr` |
| 4270 | `func_def` | `c_emit_zir_expr` |
| 4671 | `func_def` | `c_emit_owned_managed_zir_expr` |
| 4709 | `func_def` | `c_emit_effect_zir_expr` |
| 4958 | `func_def` | `c_collect_locals` |
| 5006 | `func_def` | `c_has_managed_locals` |
| 5018 | `func_def` | `c_is_managed_local_name` |
| 5035 | `func_def` | `c_emit_locals` |
| 5118 | `func_def` | `c_begin_indented_line` |
| 5122 | `func_decl` | `c_emit_release_for_struct_fields` |
| 5128 | `func_decl` | `c_emit_retain_for_struct_fields` |
| 5134 | `func_def` | `c_emit_value_clone_in_place` |
| 5171 | `func_def` | `c_emit_value_dispose_in_place` |
| 5214 | `func_def` | `c_emit_release_for_struct_fields` |
| 5236 | `func_def` | `c_emit_retain_for_struct_fields` |
| 5258 | `func_def` | `c_emit_release_for_local` |
| 5272 | `func_def` | `c_emit_effect_instruction` |
| 5483 | `func_def` | `c_emit_runtime_span_literal` |
| 5499 | `enum` | `c_contract_value_format_kind` |
| 5506 | `func_def` | `c_contract_value_format` |
| 5522 | `func_def` | `c_emit_check_contract_instruction` |
| 5615 | `func_def` | `c_emit_instruction` |
| 5749 | `func_def` | `c_emit_return_cleanup_transfer` |
| 5841 | `func_def` | `c_emit_terminator` |
| 5971 | `func_def` | `c_emit_cleanup` |
| 6007 | `func_def` | `c_emit_struct_definitions` |
| 6048 | `func_def` | `c_emit_enum_definitions` |
| 6159 | `func_def` | `c_generated_outcome_is_emitted` |
| 6174 | `func_def` | `c_generated_outcome_mark_emitted` |
| 6190 | `func_def` | `c_emit_generated_outcome_helpers_for_type` |
| 6477 | `func_def` | `c_emit_generated_outcome_helpers` |
| 6555 | `func_def` | `c_emit_function_signature` |
| 6617 | `func_def` | `c_emit_function_definition` |
| 6691 | `func_def` | `c_emit_main_wrapper` |
| 6780 | `func_def` | `c_emitter_emit_module` |

#### `compiler/targets/c/emitter.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_TARGETS_C_EMITTER_H` |

#### `compiler/targets/c/legalization.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 8 | `func_def` | `c_legalize_safe_text` |
| 12 | `func_def` | `c_legalize_set_result` |
| 32 | `func_def` | `c_legalize_result_init` |
| 36 | `func_def` | `c_legalize_error_code_name` |
| 51 | `func_def` | `c_legalize_is_blank` |
| 68 | `func_def` | `c_legalize_starts_with` |
| 76 | `func_def` | `c_legalize_copy_trimmed` |
| 110 | `func_def` | `c_legalize_copy_trimmed_segment` |
| 129 | `func_def` | `c_legalize_split_two_operands` |
| 140 | `func_def` | `c_legalize_split_three_operands` |
| 165 | `func_def` | `c_legalize_is_identifier_start` |
| 171 | `func_def` | `c_legalize_is_identifier_char` |
| 175 | `func_def` | `c_legalize_is_identifier_only` |
| 193 | `func_def` | `c_legalize_find_symbol_type` |
| 225 | `func_def` | `c_legalize_resolve_sequence_type` |
| 270 | `func_def` | `c_legalize_expect_result_type` |
| 290 | `func_def` | `c_legalize_fill_expr` |
| 312 | `func_def` | `c_legalize_index_seq` |
| 377 | `func_def` | `c_legalize_slice_seq` |
| 434 | `func_def` | `c_legalize_seq_expr` |
| 473 | `func_def` | `c_legalize_list_len_expr` |
| 543 | `func_def` | `c_legalize_render_zir_expr` |
| 565 | `func_def` | `c_legalize_resolve_zir_sequence_type` |
| 606 | `func_def` | `c_legalize_zir_seq_expr` |
| 695 | `func_def` | `c_legalize_zir_list_len_expr` |

#### `compiler/targets/c/legalization.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_TARGETS_C_LEGALIZATION_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
