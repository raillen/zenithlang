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
- Extracted symbols: 41

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/semantic/diagnostics/diagnostics.c` | 776 | 38 | 1 |
| `compiler/semantic/diagnostics/diagnostics.h` | 154 | 1 | 1 |
| `compiler/semantic/parameter_validation.c` | 90 | 2 | 1 |

### Local Dependencies

- `compiler/frontend/lexer/token.h`
- `compiler/semantic/parameter_validation.h`
- `compiler/utils/l10n.h`

### Related Tests

- `tests/fixtures/diagnostics/enum_match_non_exhaustive.contains.txt`
- `tests/fixtures/diagnostics/error_syntax.contains.txt`
- `tests/fixtures/diagnostics/error_type_mismatch.contains.txt`
- `tests/fixtures/diagnostics/functions_invalid_call.contains.txt`
- `tests/fixtures/diagnostics/functions_main_signature.contains.txt`
- `tests/fixtures/diagnostics/monomorphization_limit_error.contains.txt`
- `tests/fixtures/diagnostics/multifile_duplicate_symbol.contains.txt`
- `tests/fixtures/diagnostics/multifile_import_cycle.contains.txt`
- `tests/fixtures/diagnostics/multifile_missing_import.contains.txt`
- `tests/fixtures/diagnostics/multifile_namespace_mismatch.contains.txt`
- `tests/fixtures/diagnostics/multifile_private_access.contains.txt`
- `tests/fixtures/diagnostics/mutability_const_reassign.contains.txt`
- `tests/fixtures/diagnostics/project_unknown_key_manifest.contains.txt`
- `tests/fixtures/diagnostics/result_optional_propagation.contains.txt`
- `tests/fixtures/diagnostics/runtime_index_error.contains.txt`
- `tests/fixtures/diagnostics/where_contract_construct_error.contains.txt`
- `tests/fixtures/diagnostics/where_contract_field_assign_error.contains.txt`
- `tests/fixtures/diagnostics/where_contract_param_error.contains.txt`
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
| 177 | `func_def` | `zt_diag_code_stable` |
| 237 | `func_def` | `zt_diag_default_help` |
| 300 | `func_def` | `zt_diag_severity_name` |
| 304 | `func_def` | `zt_diag_list_make` |
| 312 | `func_def` | `zt_diag_list_dispose` |
| 320 | `func_def` | `zt_diag_list_add_va` |
| 355 | `func_def` | `zt_diag_list_add` |
| 363 | `func_def` | `zt_diag_list_add_severity` |
| 377 | `func_def` | `zt_diag_list_add_suggestion` |
| 401 | `func_def` | `zt_name_edit_distance` |
| 434 | `func_def` | `zt_name_suggest` |
| 465 | `func_def` | `zt_diag_code_effort` |
| 514 | `func_def` | `zt_diag_effort_label` |
| 523 | `func_def` | `zt_diag_action_text` |
| 548 | `func_def` | `zt_cog_profile_error_limit` |
| 557 | `func_def` | `zt_cog_profile_from_text` |
| 565 | `func_def` | `zt_diag_telemetry_log` |
| 594 | `macro` | `ANSI_RED` |
| 595 | `macro` | `ANSI_GREEN` |
| 596 | `macro` | `ANSI_YELLOW` |
| 597 | `macro` | `ANSI_BLUE` |
| 598 | `macro` | `ANSI_CYAN` |
| 599 | `macro` | `ANSI_RESET` |
| 601 | `func_def` | `zt_diag_render_detailed` |
| 650 | `func_def` | `zt_diag_render_detailed_list` |
| 660 | `func_def` | `zt_diag_render_action_first` |
| 722 | `func_def` | `zt_diag_render_action_first_list` |
| 754 | `func_def` | `zt_diag_render_ci` |
| 765 | `func_def` | `zt_diag_render_ci_list` |

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
