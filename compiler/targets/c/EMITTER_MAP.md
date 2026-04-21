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
- Extracted symbols: 228

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/targets/c/emitter.c` | 7609 | 194 | 2 |
| `compiler/targets/c/emitter.h` | 52 | 1 | 1 |
| `compiler/targets/c/legalization.c` | 1035 | 32 | 1 |
| `compiler/targets/c/legalization.h` | 87 | 1 | 1 |

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
| 528 | `macro` | `C_TYPE_TABLE_COUNT` |
| 531 | `func_def` | `c_type_lookup` |
| 561 | `struct` | `c_resolved_type_mapping` |
| 570 | `func_def` | `c_parse_unary_type_name` |
| 598 | `func_def` | `c_parse_binary_type_name` |
| 648 | `func_def` | `c_build_generated_map_symbol` |
| 655 | `func_def` | `c_resolve_type_mapping` |
| 699 | `func_def` | `c_levenshtein_distance` |
| 724 | `func_def` | `c_type_suggest_closest` |
| 748 | `func_def` | `c_type_is_managed` |
| 754 | `func_def` | `c_type_is` |
| 759 | `func_def` | `c_parse_outcome_type_name` |
| 808 | `func_def` | `c_build_generated_outcome_symbol` |
| 815 | `func_def` | `c_type_is_builtin_managed_value` |
| 835 | `func_decl` | `c_type_is_struct_with_managed_fields` |
| 837 | `func_def` | `c_struct_has_managed_fields` |
| 854 | `func_def` | `c_type_is_struct_with_managed_fields` |
| 858 | `func_def` | `c_type_needs_managed_cleanup` |
| 864 | `func_def` | `c_type_to_c` |
| 926 | `func_def` | `c_emit_trimmed_text` |
| 936 | `func_def` | `c_emit_c_string_literal` |
| 973 | `func_def` | `c_hex_digit_value` |
| 980 | `func_def` | `c_emit_hex_bytes_literal` |
| 1008 | `func_def` | `c_emit_typed_name` |
| 1018 | `func_def` | `c_find_symbol_type` |
| 1047 | `func_def` | `c_copy_trimmed_alloc` |
| 1053 | `func_def` | `c_expression_is_text` |
| 1073 | `func_def` | `c_expression_is_materialized_text_ref` |
| 1087 | `func_def` | `c_expression_is_materialized_bytes_ref` |
| 1101 | `func_def` | `c_expression_is_materialized_net_connection_ref` |
| 1114 | `func_def` | `c_expression_is_materialized_core_error_ref` |
| 1128 | `func_def` | `c_expression_is_materialized_list_i64_ref` |
| 1142 | `func_def` | `c_expression_is_materialized_map_text_text_ref` |
| 1156 | `func_def` | `c_expression_is_materialized_type_ref` |
| 1177 | `struct` | `c_map_spec` |
| 1198 | `func_def` | `c_type_is_plain_value` |
| 1204 | `func_def` | `c_map_value_optional_support` |
| 1260 | `func_def` | `c_map_key_eq_support` |
| 1274 | `func_def` | `c_map_spec_for_type` |
| 1357 | `func_def` | `c_expression_is_materialized_optional_text_ref` |
| 1371 | `func_def` | `c_expression_is_materialized_optional_bytes_ref` |
| 1385 | `func_def` | `c_expression_is_materialized_optional_list_i64_ref` |
| 1399 | `func_def` | `c_expression_is_materialized_outcome_i64_text_ref` |
| 1413 | `func_def` | `c_expression_is_materialized_outcome_void_text_ref` |
| 1427 | `func_def` | `c_expression_is_materialized_outcome_text_text_ref` |
| 1441 | `func_def` | `c_expression_is_materialized_outcome_optional_text_text_ref` |
| 1455 | `func_def` | `c_expression_is_materialized_outcome_bytes_text_ref` |
| 1469 | `func_def` | `c_expression_is_materialized_outcome_optional_bytes_text_ref` |
| 1483 | `func_def` | `c_expression_is_materialized_outcome_net_connection_text_ref` |
| 1497 | `func_def` | `c_expression_is_materialized_outcome_list_i64_text_ref` |
| 1511 | `func_def` | `c_expression_is_materialized_outcome_map_text_text_ref` |
| 1525 | `struct` | `c_outcome_spec` |
| 1545 | `func_def` | `c_outcome_spec_for_type` |
| 1600 | `func_def` | `c_outcome_spec_for_expr` |
| 1618 | `func_def` | `c_outcome_spec_for_expected` |
| 1626 | `func_def` | `c_type_requires_generated_map_helper` |
| 1631 | `func_def` | `c_module_requires_template_header` |
| 1680 | `func_def` | `c_type_requires_generated_outcome_helper` |
| 1685 | `func_def` | `c_module_requires_string_header` |
| 1733 | `func_def` | `c_expression_is_copyable_managed_value_ref` |
| 1747 | `func_def` | `c_outcome_success_value_is_supported` |
| 1763 | `func_decl` | `c_emit_expr` |
| 1771 | `func_def` | `c_outcome_emit_failure_from_error_expr` |
| 1845 | `func_def` | `c_emit_value` |
| 1849 | `func_def` | `c_parse_binary` |
| 1873 | `func_def` | `c_split_two_operands` |
| 1884 | `func_def` | `c_split_three_operands` |
| 1909 | `func_def` | `c_segment_is_blank` |
| 1920 | `func_def` | `c_find_top_level_comma` |
| 1983 | `func_def` | `c_find_top_level_colon` |
| 2046 | `func_def` | `c_math_function` |
| 2055 | `func_def` | `c_binary_operator` |
| 2067 | `func_def` | `c_binary_operator_suggestion` |
| 2073 | `func_def` | `c_unary_operator` |
| 2079 | `func_decl` | `c_emit_expr` |
| 2087 | `func_def` | `c_copy_legalize_result` |
| 2110 | `func_def` | `c_emit_legalized_seq_expr` |
| 2165 | `func_def` | `c_emit_owned_managed_expr` |
| 2212 | `func_def` | `c_emit_make_list_i64_expr` |
| 2272 | `func_def` | `c_emit_make_list_text_expr` |
| 2342 | `func_def` | `c_emit_make_map_expr` |
| 2492 | `func_def` | `c_emit_call_args` |
| 2550 | `func_def` | `c_emit_call_expr` |
| 2608 | `func_def` | `c_emit_expr` |
| 3268 | `func_decl` | `c_emit_owned_managed_zir_expr` |
| 3276 | `func_decl` | `c_emit_zir_expr` |
| 3284 | `func_def` | `c_emit_zir_expr_as_legacy` |
| 3305 | `func_def` | `c_zir_expr_name_text` |
| 3314 | `func_def` | `c_zir_expr_resolve_type` |
| 3533 | `func_def` | `c_zir_expr_is_text` |
| 3542 | `func_def` | `c_zir_expr_is_materialized_text_ref` |
| 3551 | `func_def` | `c_zir_expr_is_materialized_bytes_ref` |
| 3560 | `func_def` | `c_zir_expr_is_materialized_list_i64_ref` |
| 3569 | `func_def` | `c_zir_expr_is_materialized_type_ref` |
| 3581 | `func_def` | `c_zir_expr_is_materialized_optional_text_ref` |
| 3590 | `func_def` | `c_zir_expr_is_materialized_optional_bytes_ref` |
| 3598 | `func_def` | `c_zir_expr_is_materialized_optional_list_i64_ref` |
| 3607 | `func_def` | `c_emit_zir_call_expr` |
| 3764 | `func_def` | `c_extern_call_expected_arg_type` |
| 3778 | `func_def` | `c_zir_call_extern_needs_ffi_shield` |
| 3803 | `func_def` | `c_legacy_call_extern_needs_ffi_shield` |
| 3861 | `func_def` | `c_emit_ffi_shield_retain` |
| 3866 | `func_def` | `c_emit_ffi_shield_release` |
| 3871 | `func_def` | `c_emit_zir_ffi_call_arg` |
| 3908 | `func_def` | `c_emit_legacy_ffi_call_arg` |
| 3945 | `func_def` | `c_emit_ffi_shielded_zir_call_statement` |
| 4091 | `func_def` | `c_emit_ffi_shielded_legacy_call_statement` |
| 4293 | `func_def` | `c_emit_ffi_shielded_zir_return` |
| 4344 | `func_def` | `c_is_mutating_self_expr` |
| 4353 | `func_def` | `c_find_make_struct_field_init` |
| 4365 | `func_def` | `c_emit_zir_make_struct_expr` |
| 4499 | `func_def` | `c_emit_zir_make_list_expr` |
| 4569 | `func_def` | `c_emit_zir_index_expr` |
| 4672 | `func_def` | `c_emit_zir_slice_expr` |
| 4739 | `func_decl` | `c_emit_owned_managed_zir_expr` |
| 4747 | `func_decl` | `c_emit_zir_expr` |
| 4755 | `func_def` | `c_emit_zir_expr` |
| 5164 | `func_def` | `c_emit_owned_managed_zir_expr` |
| 5202 | `func_def` | `c_emit_effect_zir_expr` |
| 5469 | `func_def` | `c_collect_locals` |
| 5517 | `func_def` | `c_has_managed_locals` |
| 5529 | `func_def` | `c_is_managed_local_name` |
| 5546 | `func_def` | `c_emit_locals` |
| 5629 | `func_def` | `c_begin_indented_line` |
| 5633 | `func_decl` | `c_emit_release_for_struct_fields` |
| 5639 | `func_decl` | `c_emit_retain_for_struct_fields` |
| 5645 | `func_def` | `c_emit_value_clone_in_place` |
| 5682 | `func_def` | `c_emit_value_dispose_in_place` |
| 5725 | `func_def` | `c_emit_release_for_struct_fields` |
| 5747 | `func_def` | `c_emit_retain_for_struct_fields` |
| 5769 | `func_def` | `c_emit_release_for_local` |
| 5783 | `func_def` | `c_emit_effect_instruction` |
| 6008 | `func_def` | `c_emit_runtime_span_literal` |
| 6024 | `enum` | `c_contract_value_format_kind` |
| 6031 | `func_def` | `c_contract_value_format` |
| 6047 | `func_def` | `c_emit_check_contract_instruction` |
| 6140 | `func_def` | `c_emit_instruction` |
| 6274 | `func_def` | `c_emit_return_cleanup_transfer` |
| 6366 | `func_def` | `c_emit_terminator` |
| 6496 | `func_def` | `c_emit_cleanup` |
| 6532 | `func_def` | `c_emit_struct_definitions` |
| 6573 | `func_def` | `c_emit_enum_definitions` |
| 6684 | `func_def` | `c_generated_map_is_emitted` |
| 6699 | `func_def` | `c_generated_map_mark_emitted` |
| 6715 | `func_def` | `c_emit_generated_map_helpers_for_type` |
| 6829 | `func_def` | `c_emit_generated_map_helpers` |
| 6906 | `func_def` | `c_generated_outcome_is_emitted` |
| 6921 | `func_def` | `c_generated_outcome_mark_emitted` |
| 6937 | `func_def` | `c_emit_generated_outcome_helpers_for_type` |
| 7224 | `func_def` | `c_emit_generated_outcome_helpers` |
| 7302 | `func_def` | `c_emit_function_signature` |
| 7364 | `func_def` | `c_emit_function_definition` |
| 7438 | `func_def` | `c_emit_main_wrapper` |
| 7527 | `func_def` | `c_emitter_emit_module` |

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
| 129 | `func_def` | `c_legalize_find_top_level_comma` |
| 150 | `func_def` | `c_legalize_parse_binary_generic` |
| 205 | `func_def` | `c_legalize_copy_sanitized` |
| 235 | `func_def` | `c_legalize_build_generated_map_symbol` |
| 242 | `func_def` | `c_legalize_build_map_runtime_name` |
| 263 | `func_def` | `c_legalize_parse_map_type_name` |
| 283 | `func_def` | `c_legalize_parse_make_map_type_name` |
| 310 | `func_def` | `c_legalize_split_two_operands` |
| 321 | `func_def` | `c_legalize_split_three_operands` |
| 346 | `func_def` | `c_legalize_is_identifier_start` |
| 352 | `func_def` | `c_legalize_is_identifier_char` |
| 356 | `func_def` | `c_legalize_is_identifier_only` |
| 374 | `func_def` | `c_legalize_find_symbol_type` |
| 406 | `func_def` | `c_legalize_resolve_sequence_type` |
| 461 | `func_def` | `c_legalize_expect_result_type` |
| 481 | `func_def` | `c_legalize_fill_expr` |
| 503 | `func_def` | `c_legalize_index_seq` |
| 590 | `func_def` | `c_legalize_slice_seq` |
| 647 | `func_def` | `c_legalize_seq_expr` |
| 686 | `func_def` | `c_legalize_list_len_expr` |
| 790 | `func_def` | `c_legalize_render_zir_expr` |
| 812 | `func_def` | `c_legalize_resolve_zir_sequence_type` |
| 853 | `func_def` | `c_legalize_zir_seq_expr` |
| 965 | `func_def` | `c_legalize_zir_list_len_expr` |

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
