# Project/ZDoc - Code Map

## 📋 Descrição

Gerenciamento de projeto e documentação. Responsável por:
- Parse de `zenith.ztproj` (project manifest)
- Module resolution e dependency tracking
- ZDoc generation (documentação da stdlib)
- Project configuration

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `ztproj.c` | 30.2 KB | Project management |
| `ztproj.h` | 2.9 KB | Project interfaces |
| `zdoc.c` | 38.0 KB | Documentation generation |
| `zdoc.h` | 1.7 KB | ZDoc interfaces |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟢 BAIXA |

## ⚠️ Estado Crítico

- **Project config**: settings do projeto atual
- **Module list**: módulos compilados
- **Doc cache**: documentação gerada

## 🔗 Dependencies Externas

- `stdlib/` → Standard library modules
- `compiler/driver/` → Integration com CLI

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- Tooling auxiliar, não crítico para compilação
- ZDoc é útil para developers da linguagem

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Medium
- Source files: 4
- Extracted symbols: 77

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/project/zdoc.c` | 1203 | 45 | 1 |
| `compiler/project/zdoc.h` | 71 | 1 | 1 |
| `compiler/project/ztproj.c` | 834 | 30 | 1 |
| `compiler/project/ztproj.h` | 104 | 1 | 0 |

### Local Dependencies

- `compiler/frontend/ast/model.h`
- `compiler/project/zdoc.h`
- `compiler/project/ztproj.h`

### Related Tests

- `tests/driver/test_project.c`
- `tests/driver/test_zdoc.c`

### Symbol Index

#### `compiler/project/zdoc.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 15 | `struct` | `zt_zdoc_file_item` |
| 19 | `struct` | `zt_zdoc_file_list` |
| 25 | `struct` | `zt_zdoc_symbol_item` |
| 31 | `struct` | `zt_zdoc_symbol_list` |
| 37 | `struct` | `zt_zdoc_public_symbol_item` |
| 45 | `struct` | `zt_zdoc_public_symbol_list` |
| 51 | `func_def` | `zt_copy_text` |
| 61 | `func_def` | `zt_path_char_equal` |
| 70 | `func_def` | `zt_join_path` |
| 96 | `func_def` | `zt_path_is_dir` |
| 108 | `func_def` | `zt_path_has_extension` |
| 116 | `func_def` | `zt_strip_prefix_path` |
| 141 | `func_def` | `zt_read_file_text` |
| 187 | `func_def` | `zt_trim_inplace` |
| 205 | `func_decl` | `zt_zdoc_diagnostic_push` |
| 215 | `func_def` | `zt_zdoc_file_list_push` |
| 238 | `func_def` | `zt_zdoc_file_list_dispose` |
| 244 | `func_def` | `zt_discover_zdoc_files` |
| 319 | `func_def` | `zt_symbol_equal` |
| 324 | `func_def` | `zt_zdoc_symbol_list_push` |
| 357 | `func_def` | `zt_zdoc_symbol_list_dispose` |
| 363 | `func_def` | `zt_zdoc_public_symbol_equal` |
| 371 | `func_def` | `zt_zdoc_public_symbol_list_push` |
| 408 | `func_def` | `zt_zdoc_public_symbol_list_dispose` |
| 414 | `func_def` | `zt_mark_public_symbol_documented` |
| 431 | `func_def` | `zt_emit_missing_public_doc_warnings` |
| 461 | `func_def` | `zt_symbol_exists` |
| 481 | `func_def` | `zt_symbol_exists_local` |
| 498 | `func_def` | `zt_add_symbol_pair` |
| 530 | `func_def` | `zt_collect_source_symbols` |
| 628 | `func_def` | `zt_zdoc_diagnostic_list_init` |
| 633 | `func_def` | `zt_zdoc_diagnostic_list_dispose` |
| 639 | `func_def` | `zt_zdoc_diagnostic_error_count` |
| 650 | `func_def` | `zt_zdoc_diagnostic_warning_count` |
| 661 | `func_def` | `zt_zdoc_severity_name` |
| 665 | `func_def` | `zt_zdoc_code_name` |
| 679 | `func_def` | `zt_zdoc_diagnostic_push` |
| 717 | `func_def` | `zt_extract_tag_value` |
| 742 | `func_def` | `zt_starts_with_guides` |
| 758 | `func_def` | `zt_make_paired_source_path` |
| 780 | `func_def` | `zt_resolve_target` |
| 791 | `func_def` | `zt_resolve_link` |
| 800 | `func_def` | `zt_scan_links` |
| 860 | `func_def` | `zt_check_zdoc_file` |
| 1101 | `func_def` | `zt_zdoc_check_project` |

#### `compiler/project/zdoc.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_PROJECT_ZDOC_H` |

#### `compiler/project/ztproj.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 9 | `enum` | `zt_project_section` |
| 24 | `func_def` | `zt_project_safe_text` |
| 28 | `func_def` | `zt_project_section_name` |
| 45 | `func_def` | `zt_project_set_error` |
| 57 | `func_def` | `zt_project_set_unknown_key` |
| 67 | `func_def` | `zt_project_read_file_text` |
| 113 | `func_def` | `zt_project_trim_left` |
| 118 | `func_def` | `zt_project_trim_inplace` |
| 139 | `func_def` | `zt_project_strip_comment` |
| 156 | `func_def` | `zt_project_copy_checked` |
| 166 | `func_def` | `zt_project_parse_string_value` |
| 227 | `func_def` | `zt_project_parse_positive_size_value` |
| 273 | `func_def` | `zt_project_section_from_name` |
| 288 | `func_def` | `zt_project_parse_section` |
| 324 | `func_def` | `zt_project_find_assignment` |
| 343 | `func_def` | `zt_project_parse_dependency_spec` |
| 389 | `func_def` | `zt_project_assign_dependency` |
| 442 | `func_def` | `zt_project_parse_bool_value` |
| 462 | `func_def` | `zt_project_assign_value` |
| 544 | `func_def` | `zt_project_require_field` |
| 559 | `func_def` | `zt_project_copy_default` |
| 576 | `func_def` | `zt_project_manifest_kind` |
| 583 | `func_def` | `zt_project_manifest_entry_namespace` |
| 590 | `func_def` | `zt_project_validate` |
| 665 | `func_def` | `zt_project_manifest_init` |
| 670 | `func_def` | `zt_project_parse_result_init` |
| 678 | `func_def` | `zt_project_error_code_name` |
| 697 | `func_def` | `zt_project_parse_text` |
| 772 | `func_def` | `zt_project_load_file` |
| 790 | `func_def` | `zt_project_resolve_entry_source_path` |

#### `compiler/project/ztproj.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_PROJECT_ZTPROJ_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
