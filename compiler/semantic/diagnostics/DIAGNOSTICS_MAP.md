# Diagnostics - Code Map

## 📋 Descrição

Sistema de diagnósticos. Responsável por:
- Formatação de error messages
- Warning reporting
- Error codes e categorização
- Source location tracking
- Pretty-printing de erros

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `*.c` | - | Implementation |
| `*.h` | - | Error codes, interfaces |
| `parameter_validation.c` | 2.8 KB | Validação específica de parâmetros |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟢 BAIXA |

## ⚠️ Estado Crítico

- **Error buffer**: mensagens sendo coletadas
- **Error count**: tally de erros/warnings
- **Source mapping**: linha/coluna → source code

## 🔗 Dependencies Externas

- `utils/diagnostics.h` → Core definitions
- `utils/l10n.c` → Internacionalização (se aplicável)

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- Crítico para UX do compiler, mas não bloqueante
- Messages devem ser claras e acionáveis

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Low
- Source files: 3
- Extracted symbols: 45

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/semantic/diagnostics/diagnostics.c` | 1021 | 42 | 2 |
| `compiler/semantic/diagnostics/diagnostics.h` | 180 | 1 | 1 |
| `compiler/semantic/parameter_validation.c` | 90 | 2 | 1 |

### Local Dependencies

- `compiler/frontend/lexer/token.h`
- `compiler/semantic/diagnostics/diagnostics.h`
- `compiler/semantic/parameter_validation.h`
- `compiler/utils/l10n.h`

### Related Tests

- `tests/fixtures/diagnostics/callable_escape_container_error.contains.txt`
- `tests/fixtures/diagnostics/callable_escape_public_var_error.contains.txt`
- `tests/fixtures/diagnostics/callable_escape_struct_field_error.contains.txt`
- `tests/fixtures/diagnostics/callable_invalid_func_ref_error.contains.txt`
- `tests/fixtures/diagnostics/callable_signature_mismatch_error.contains.txt`
- `tests/fixtures/diagnostics/check_intrinsic_message_fail.contains.txt`
- `tests/fixtures/diagnostics/check_intrinsic_type_error.contains.txt`
- `tests/fixtures/diagnostics/closure_mut_capture_error.contains.txt`
- `tests/fixtures/diagnostics/dyn_generic_trait_error.contains.txt`
- `tests/fixtures/diagnostics/enum_match_non_exhaustive.contains.txt`
- `tests/fixtures/diagnostics/error_syntax.contains.txt`
- `tests/fixtures/diagnostics/error_type_mismatch.contains.txt`
- `tests/fixtures/diagnostics/extern_c_struct_arg_error.contains.txt`
- `tests/fixtures/diagnostics/fmt_interpolation_type_error.contains.txt`
- `tests/fixtures/diagnostics/functions_invalid_call.contains.txt`
- `tests/fixtures/diagnostics/functions_main_signature.contains.txt`
- `tests/fixtures/diagnostics/functions_param_ordering_error.contains.txt`
- `tests/fixtures/diagnostics/lambda_return_mismatch_error.contains.txt`
- `tests/fixtures/diagnostics/lazy_reuse_error.contains.txt`
- `tests/fixtures/diagnostics/methods_mutating_const_receiver_error.contains.txt`
- `tests/fixtures/diagnostics/monomorphization_limit_error.contains.txt`
- `tests/fixtures/diagnostics/multifile_duplicate_symbol.contains.txt`
- `tests/fixtures/diagnostics/multifile_import_cycle.contains.txt`
- `tests/fixtures/diagnostics/multifile_missing_import.contains.txt`
- `tests/fixtures/diagnostics/multifile_namespace_mismatch.contains.txt`
- `tests/fixtures/diagnostics/multifile_private_access.contains.txt`
- `tests/fixtures/diagnostics/mutability_const_reassign.contains.txt`
- `tests/fixtures/diagnostics/noncanonical_abstract_error.contains.txt`
- `tests/fixtures/diagnostics/noncanonical_and_error.contains.txt`
- `tests/fixtures/diagnostics/noncanonical_let_error.contains.txt`
- `tests/fixtures/diagnostics/noncanonical_not_error.contains.txt`
- `tests/fixtures/diagnostics/noncanonical_null_error.contains.txt`
- `tests/fixtures/diagnostics/noncanonical_or_error.contains.txt`
- `tests/fixtures/diagnostics/noncanonical_partial_error.contains.txt`
- `tests/fixtures/diagnostics/noncanonical_string_error.contains.txt`
- `tests/fixtures/diagnostics/noncanonical_throw_error.contains.txt`
- `tests/fixtures/diagnostics/noncanonical_union_error.contains.txt`
- `tests/fixtures/diagnostics/noncanonical_virtual_error.contains.txt`
- `tests/fixtures/diagnostics/optional_question_outside_optional_error.contains.txt`
- `tests/fixtures/diagnostics/project_unknown_key_manifest.contains.txt`
- `tests/fixtures/diagnostics/public_var_cross_namespace_write_error.contains.txt`
- `tests/fixtures/diagnostics/readability_block_depth_strict_error.contains.txt`
- `tests/fixtures/diagnostics/readability_enum_default_strict_error.contains.txt`
- `tests/fixtures/diagnostics/readability_function_length_strict_error.contains.txt`
- `tests/fixtures/diagnostics/readability_warnings_strict_error.contains.txt`
- `tests/fixtures/diagnostics/result_optional_propagation.contains.txt`
- `tests/fixtures/diagnostics/runtime_index_error.contains.txt`
- `tests/fixtures/diagnostics/std_concurrent_boundary_copy_unsupported_error.contains.txt`
- `tests/fixtures/diagnostics/std_random_cross_namespace_write_error.contains.txt`
- `tests/fixtures/diagnostics/std_test_helpers_bool_fail.contains.txt`
- `tests/fixtures/diagnostics/std_test_helpers_equal_fail.contains.txt`
- `tests/fixtures/diagnostics/std_test_helpers_not_equal_fail.contains.txt`
- `tests/fixtures/diagnostics/todo_builtin_fail.contains.txt`
- `tests/fixtures/diagnostics/unreachable_builtin_fail.contains.txt`
- `tests/fixtures/diagnostics/where_contract_construct_error.contains.txt`
- `tests/fixtures/diagnostics/where_contract_field_assign_error.contains.txt`
- `tests/fixtures/diagnostics/where_contract_param_error.contains.txt`
- `tests/fixtures/diagnostics/where_contract_param_where_invalid_error.contains.txt`
- `tests/fixtures/diagnostics/where_contract_param_where_non_bool_error.contains.txt`
- `tests/test_diagnostics.zt`

### Symbol Index

#### `compiler/semantic/diagnostics/diagnostics.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 9 | `macro` | `ZT_DIAG_MAX_TOTAL` |
| 10 | `macro` | `ZT_DIAG_RECENT_DEDUP_WINDOW` |
| 12 | `func_def` | `zt_diag_same_text` |
| 18 | `func_def` | `zt_diag_same_span` |
| 25 | `func_def` | `zt_diag_is_duplicate_recent` |
| 49 | `func_def` | `zt_diag_print_source_span` |
| 63 | `func_def` | `zt_diag_read_source_line` |
| 103 | `func_def` | `zt_diag_render_caret` |
| 118 | `func_def` | `zt_diag_code_name` |
| 197 | `func_def` | `zt_diag_code_stable` |
| 277 | `func_def` | `zt_diag_default_help` |
| 360 | `func_def` | `zt_diag_severity_name` |
| 364 | `func_def` | `zt_diag_list_make` |
| 372 | `func_def` | `zt_diag_list_dispose` |
| 380 | `func_def` | `zt_diag_list_error_count` |
| 393 | `func_def` | `zt_diag_list_has_errors` |
| 397 | `func_def` | `zt_diag_list_promote_warnings` |
| 408 | `func_def` | `zt_diag_list_add_va` |
| 443 | `func_def` | `zt_diag_list_add` |
| 451 | `func_def` | `zt_diag_list_add_severity` |
| 465 | `func_def` | `zt_diag_list_add_suggestion` |
| 489 | `func_def` | `zt_name_edit_distance` |
| 522 | `func_def` | `zt_name_suggest` |
| 553 | `func_def` | `zt_diag_code_effort` |
| 622 | `func_def` | `zt_diag_effort_label` |
| 631 | `func_def` | `zt_diag_action_text` |
| 673 | `func_def` | `zt_diag_next_text` |
| 765 | `func_def` | `zt_cog_profile_error_limit` |
| 774 | `func_def` | `zt_cog_profile_from_text` |
| 782 | `func_def` | `zt_diag_telemetry_log` |
| 811 | `macro` | `ANSI_RED` |
| 812 | `macro` | `ANSI_GREEN` |
| 813 | `macro` | `ANSI_YELLOW` |
| 814 | `macro` | `ANSI_BLUE` |
| 815 | `macro` | `ANSI_CYAN` |
| 816 | `macro` | `ANSI_RESET` |
| 818 | `func_def` | `zt_diag_render_detailed` |
| 867 | `func_def` | `zt_diag_render_detailed_list` |
| 877 | `func_def` | `zt_diag_render_action_first` |
| 947 | `func_def` | `zt_diag_render_action_first_list` |
| 979 | `func_def` | `zt_diag_render_ci` |
| 1010 | `func_def` | `zt_diag_render_ci_list` |

#### `compiler/semantic/diagnostics/diagnostics.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_SEMANTIC_DIAGNOSTICS_H` |

#### `compiler/semantic/parameter_validation.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 25 | `func_def` | `zt_validate_parameter_ordering` |
| 66 | `func_def` | `zt_validate_argument_ordering` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
