# Driver/CLI - Code Map

## 📋 Descrição

Ponto de entrada do compiler. Responsável por:
- Parse de argumentos de linha de comando
- Orquestração do pipeline de compilação
- Integração com LSP (Language Server Protocol)
- Geração de documentação (ZDoc)

## 📁 Arquivos Modulares

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `driver_internal.h` | ~7 KB | Header compartilhado: tipos, globals `extern`, protótipos inter-módulo |
| `paths.c` | ~11 KB | File I/O, manipulação de caminhos, path filter list, git ref validation |
| `main.c` | ~139 KB | CLI, pipeline, comandos, diagnostics |
| `lsp.c` | ~9 KB | Language Server Protocol implementation |
| `doc_show.c` | ~20 KB | Documentation display/generation |

## 🏗️ Arquitetura Modular

```text
driver_internal.h (shared header)
    ├── paths.c         (pure utilities, zero cross-deps)
    ├── main.c          (CLI dispatch, pipeline, handlers)
    ├── lsp.c           (independent LSP server)
    └── doc_show.c      (documentation CLI)
```

### paths.c — Funções de utilidade pura
- `zt_read_file` / `zt_write_file` — I/O de arquivos
- `zt_join_path` / `zt_dirname` / `zt_copy_text` — manipulação de paths
- `zt_path_is_dir` / `zt_path_is_file` / `zt_path_has_extension` — queries
- `zt_normalize_path_*` / `zt_path_char_equal` — normalização cross-platform
- `zt_make_dirs` — criação recursiva de diretórios
- `zt_path_filter_list_*` — filter list para `--since`

### driver_internal.h — Contrato compartilhado
- Types: `zt_project_source_file`, `zt_project_source_file_list`, `zt_path_filter_list`
- Globals `extern`: `global_arena`, `global_pool`, `zt_ci_mode_enabled`, etc.
- Macros cross-platform: `ZT_MKDIR`, `ZT_GETCWD`, `ZT_POPEN`, `ZT_PCLOSE`

## ⚠️ Estado Crítico

- **Globals compartilhados**: definidos em `main.c`, declarados `extern` em `driver_internal.h`
- **Build system**: `build.py` auto-descobre via `os.walk('compiler')` — não precisa de mudanças
- **Error handling**: cleanup de recursos em caso de falha

## 🔗 Dependencies Externas

- `frontend/` → Lexer, Parser, AST
- `semantic/` → Binder, Type Checker
- `targets/c/` → Code Generation
- `utils/` → Arena, Diagnostics, l10n

## 📝 Notas de Manutenção

- `main.c` ainda contém pipeline + handlers (~3750 linhas) — futuras extrações possíveis
- `paths.c` tem zero dependências em outros módulos do driver — seguro para estender
- LSP é independente do pipeline principal


<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Critical
- Source files: 8
- Extracted symbols: 207

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/driver/doc_show.c` | 561 | 21 | 3 |
| `compiler/driver/doc_show.h` | 14 | 2 | 0 |
| `compiler/driver/driver_internal.h` | 197 | 64 | 18 |
| `compiler/driver/lsp.c` | 243 | 8 | 6 |
| `compiler/driver/main.c` | 2382 | 49 | 1 |
| `compiler/driver/paths.c` | 402 | 21 | 1 |
| `compiler/driver/pipeline.c` | 850 | 22 | 2 |
| `compiler/driver/project.c` | 600 | 20 | 1 |

### Local Dependencies

- `compiler/driver/doc_show.h`
- `compiler/driver/driver_internal.h`
- `compiler/frontend/lexer/lexer.h`
- `compiler/frontend/parser/parser.h`
- `compiler/hir/lowering/from_ast.h`
- `compiler/project/zdoc.h`
- `compiler/project/ztproj.h`
- `compiler/semantic/binder/binder.h`
- `compiler/semantic/diagnostics/diagnostics.h`
- `compiler/semantic/types/checker.h`
- `compiler/targets/c/emitter.h`
- `compiler/tooling/formatter.h`
- `compiler/utils/arena.h`
- `compiler/utils/cJSON.h`
- `compiler/utils/l10n.h`
- `compiler/utils/string_pool.h`
- `compiler/zir/lowering/from_hir.h`
- `compiler/zir/parser.h`
- `compiler/zir/verifier.h`
- `runtime/c/zenith_rt.h`

### Related Tests

- `tests/driver/test_project.c`
- `tests/driver/test_zdoc.c`

### Symbol Index

#### `compiler/driver/doc_show.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 10 | `func_decl` | `zt_resolve_project_paths` |
| 16 | `func_decl` | `zt_apply_manifest_lang` |
| 18 | `func_def` | `doc_read_file` |
| 58 | `func_def` | `doc_join_path` |
| 84 | `func_def` | `doc_path_exists` |
| 90 | `func_def` | `doc_ns_to_path` |
| 98 | `struct` | `doc_symbol_entry` |
| 103 | `struct` | `doc_symbol_list` |
| 109 | `func_def` | `doc_symbol_list_init` |
| 115 | `func_def` | `doc_symbol_list_push` |
| 130 | `func_def` | `doc_symbol_list_dispose` |
| 137 | `func_def` | `doc_lex_scan_zt` |
| 243 | `func_def` | `doc_symbol_exists` |
| 251 | `func_def` | `doc_symbol_kind` |
| 259 | `func_def` | `doc_locale_dir` |
| 267 | `func_def` | `doc_locale_base` |
| 340 | `func_def` | `print_doc_header` |
| 358 | `func_def` | `print_doc_block` |
| 387 | `func_def` | `print_doc_footer` |
| 391 | `func_def` | `find_target_block` |
| 449 | `func_def` | `zt_handle_doc_show` |

#### `compiler/driver/doc_show.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZT_DRIVER_DOC_SHOW_H` |
| 11 | `func_decl` | `zt_handle_doc_show` |

#### `compiler/driver/driver_internal.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 11 | `macro` | `ZT_DRIVER_INTERNAL_H` |
| 45 | `macro` | `ZT_MKDIR` |
| 46 | `macro` | `ZT_GETCWD` |
| 47 | `macro` | `ZT_POPEN` |
| 48 | `macro` | `ZT_PCLOSE` |
| 53 | `macro` | `ZT_MKDIR` |
| 54 | `macro` | `ZT_GETCWD` |
| 55 | `macro` | `ZT_POPEN` |
| 56 | `macro` | `ZT_PCLOSE` |
| 76 | `struct` | `zt_path_filter_list` |
| 86 | `struct` | `zt_project_source_file` |
| 93 | `struct` | `zt_project_source_file_list` |
| 101 | `func_decl` | `zt_read_file` |
| 102 | `func_decl` | `zt_write_file` |
| 103 | `func_decl` | `zt_copy_text` |
| 104 | `func_decl` | `zt_heap_strdup` |
| 105 | `func_decl` | `zt_path_has_extension` |
| 106 | `func_decl` | `zt_path_is_dir` |
| 107 | `func_decl` | `zt_path_is_file` |
| 108 | `func_decl` | `zt_join_path` |
| 109 | `func_decl` | `zt_dirname` |
| 110 | `func_decl` | `zt_get_current_dir` |
| 111 | `func_decl` | `zt_find_project_root_from_cwd` |
| 112 | `func_decl` | `zt_replace_extension` |
| 113 | `func_decl` | `zt_normalize_path_separators` |
| 114 | `func_decl` | `zt_git_ref_is_safe` |
| 115 | `func_decl` | `zt_make_dirs` |
| 116 | `func_decl` | `zt_normalize_path_inplace` |
| 117 | `func_decl` | `zt_path_char_equal` |
| 118 | `func_decl` | `zt_path_suffix_matches` |
| 119 | `func_decl` | `zt_normalize_system_path` |
| 123 | `func_decl` | `zt_project_source_file_list_init` |
| 124 | `func_decl` | `zt_project_source_file_list_dispose` |
| 125 | `func_decl` | `zt_project_source_file_list_push` |
| 126 | `func_decl` | `zt_project_discover_zt_files` |
| 127 | `func_decl` | `zt_namespace_to_relative_path` |
| 128 | `func_decl` | `zt_last_namespace_segment` |
| 129 | `func_decl` | `zt_find_entry_root` |
| 130 | `func_decl` | `zt_namespace_exists` |
| 131 | `func_decl` | `zt_namespace_is_std` |
| 132 | `func_decl` | `zt_find_namespace_index` |
| 133 | `func_decl` | `zt_detect_import_cycles` |
| 134 | `func_decl` | `zt_validate_source_namespaces` |
| 138 | `func_decl` | `zt_parse_project_sources` |
| 139 | `func_decl` | `zt_build_combined_project_ast` |
| 142 | `func_decl` | `zt_load_since_filter` |
| 143 | `func_decl` | `zt_path_filter_list_dispose` |
| 144 | `func_decl` | `zt_path_filter_list_push` |
| 148 | `func_decl` | `zt_apply_manifest_lang` |
| 149 | `func_decl` | `zt_print_diagnostics` |
| 150 | `func_decl` | `zt_print_single_diag` |
| 156 | `func_decl` | `zt_resolve_project_paths` |
| 162 | `func_decl` | `zt_print_project_parse_error` |
| 163 | `func_decl` | `zt_diag_code_from_zir_verifier` |
| 164 | `func_decl` | `zt_diag_code_from_zir_parse_error` |
| 165 | `func_decl` | `zt_diag_code_from_c_emit_error` |
| 166 | `func_decl` | `zt_collect_project_sources` |
| 175 | `struct` | `zt_project_compile_result` |
| 185 | `func_decl` | `zt_project_compile_result_init` |
| 186 | `func_decl` | `zt_project_compile_result_dispose` |
| 187 | `func_decl` | `zt_compile_project` |
| 188 | `func_decl` | `zt_emit_module_to_c` |
| 192 | `func_decl` | `zt_compile_c_file` |
| 193 | `func_decl` | `zt_run_executable` |

#### `compiler/driver/lsp.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 13 | `func_def` | `lsp_log` |
| 22 | `func_def` | `read_exact` |
| 35 | `func_def` | `read_message` |
| 57 | `func_def` | `send_message` |
| 68 | `func_def` | `handle_initialize` |
| 100 | `func_def` | `append_diagnostics` |
| 139 | `func_def` | `process_document` |
| 173 | `func_def` | `main` |

#### `compiler/driver/main.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 21 | `func_def` | `zt_apply_manifest_lang` |
| 35 | `func_def` | `zt_print_usage` |
| 72 | `func_decl` | `zt_print_diagnostics` |
| 74 | `func_def` | `zt_diag_code_from_project_error` |
| 93 | `func_def` | `zt_print_project_parse_error` |
| 121 | `func_def` | `zt_diag_matches_focus` |
| 140 | `func_def` | `zt_diag_matches_since` |
| 163 | `func_def` | `zt_diag_matches_filters` |
| 166 | `func_def` | `zt_print_diagnostics` |
| 244 | `func_def` | `zt_print_single_diag` |
| 265 | `func_def` | `zt_diag_code_from_zir_parse_error` |
| 289 | `func_def` | `zt_diag_code_from_zir_verifier` |
| 305 | `func_def` | `zt_diag_code_from_c_emit_error` |
| 317 | `func_def` | `zt_diag_code_from_zdoc_code` |
| 331 | `func_def` | `zt_append_zdoc_diagnostics` |
| 355 | `func_def` | `zt_resolve_project_paths` |
| 435 | `struct` | `zt_string_set` |
| 441 | `func_def` | `zt_string_set_init` |
| 448 | `func_def` | `zt_string_set_dispose` |
| 460 | `func_def` | `zt_string_set_add` |
| 488 | `func_def` | `zt_should_skip_test_discovery_dir` |
| 499 | `func_def` | `zt_dir_has_project_manifest` |
| 506 | `func_def` | `zt_collect_test_projects` |
| 584 | `func_decl` | `zt_handle_project_command` |
| 585 | `func_decl` | `zt_collect_project_sources` |
| 592 | `func_def` | `zt_token_is_identifier_text` |
| 600 | `func_def` | `zt_file_has_test_marker` |
| 660 | `func_def` | `zt_project_uses_test_markers` |
| 690 | `struct` | `zt_attr_test_case` |
| 696 | `struct` | `zt_attr_test_case_list` |
| 702 | `func_def` | `zt_attr_test_case_list_init` |
| 707 | `func_def` | `zt_attr_test_case_list_dispose` |
| 713 | `func_def` | `zt_attr_test_case_list_push` |
| 743 | `func_def` | `zt_collect_attr_tests` |
| 798 | `func_def` | `zt_generate_attr_test_runner_source` |
| 825 | `func_def` | `zt_generate_attr_test_manifest` |
| 860 | `func_def` | `zt_run_attr_tests_for_project` |
| 1009 | `func_def` | `zt_handle_doc_check` |
| 1085 | `func_def` | `zt_handle_fmt` |
| 1160 | `func_def` | `zt_handle_project_info` |
| 1222 | `func_def` | `zt_handle_project_command` |
| 1351 | `func_def` | `zt_handle_test` |
| 1539 | `func_def` | `zt_handle_zir_command` |
| 1717 | `func_def` | `zt_find_most_recent_file` |
| 1800 | `func_def` | `zt_find_focus_anchor` |
| 1937 | `func_def` | `zt_handle_summary` |
| 1998 | `func_def` | `zt_handle_resume` |
| 2038 | `func_def` | `zt_handle_perf` |
| 2071 | `func_def` | `main` |

#### `compiler/driver/paths.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 12 | `func_def` | `zt_read_file` |
| 56 | `func_def` | `zt_write_file` |
| 78 | `func_def` | `zt_copy_text` |
| 90 | `func_def` | `zt_heap_strdup` |
| 106 | `func_def` | `zt_path_has_extension` |
| 119 | `func_def` | `zt_path_is_dir` |
| 133 | `func_def` | `zt_path_is_file` |
| 149 | `func_def` | `zt_join_path` |
| 175 | `func_def` | `zt_dirname` |
| 203 | `func_def` | `zt_get_current_dir` |
| 209 | `func_def` | `zt_find_project_root_from_cwd` |
| 232 | `func_def` | `zt_replace_extension` |
| 252 | `func_def` | `zt_normalize_path_separators` |
| 260 | `func_def` | `zt_normalize_path_inplace` |
| 271 | `func_def` | `zt_path_char_equal` |
| 280 | `func_def` | `zt_path_suffix_matches` |
| 304 | `func_def` | `zt_normalize_system_path` |
| 323 | `func_def` | `zt_git_ref_is_safe` |
| 336 | `func_def` | `zt_make_dirs` |
| 368 | `func_def` | `zt_path_filter_list_dispose` |
| 380 | `func_def` | `zt_path_filter_list_push` |

#### `compiler/driver/pipeline.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 17 | `struct` | `zt_string_set` |
| 23 | `func_def` | `zt_string_set_init` |
| 30 | `func_def` | `zt_string_set_dispose` |
| 42 | `func_def` | `zt_string_set_add` |
| 68 | `func_def` | `zt_string_set_add_slice` |
| 87 | `func_def` | `zt_type_ident_char` |
| 91 | `func_def` | `zt_strip_type_whitespace` |
| 113 | `func_def` | `zt_collect_generic_instances_in_range` |
| 167 | `func_def` | `zt_collect_generic_instances_from_type_name` |
| 179 | `func_def` | `zt_collect_generic_instances_from_expr` |
| 292 | `func_def` | `zt_collect_generic_instances_from_module` |
| 347 | `func_def` | `zt_format_generic_preview` |
| 379 | `func_def` | `zt_enforce_monomorphization_limit` |
| 425 | `struct` | `zt_project_compile_result` |
| 435 | `func_def` | `zt_project_compile_result_init` |
| 439 | `func_def` | `zt_project_compile_result_dispose` |
| 448 | `func_def` | `zt_compile_project` |
| 681 | `func_def` | `zt_emit_module_to_c` |
| 707 | `func_def` | `zt_compile_c_file` |
| 742 | `func_def` | `zt_decode_process_exit` |
| 751 | `func_def` | `zt_run_executable` |
| 780 | `func_def` | `zt_collect_project_sources` |

#### `compiler/driver/project.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 16 | `func_def` | `zt_project_source_file_list_init` |
| 21 | `func_def` | `zt_project_source_file_list_dispose` |
| 36 | `func_def` | `zt_load_since_filter` |
| 98 | `func_def` | `zt_project_source_file_list_push` |
| 134 | `func_def` | `zt_project_discover_zt_files` |
| 222 | `func_def` | `zt_namespace_to_relative_path` |
| 243 | `func_def` | `zt_last_namespace_segment` |
| 250 | `func_def` | `zt_find_entry_root` |
| 265 | `func_def` | `zt_namespace_exists` |
| 269 | `func_def` | `zt_namespace_is_std` |
| 273 | `func_def` | `zt_find_namespace_index` |
| 288 | `func_def` | `zt_print_import_cycle` |
| 323 | `func_def` | `zt_detect_import_cycle_visit` |
| 365 | `func_def` | `zt_detect_import_cycles` |
| 394 | `func_def` | `zt_alias_for_namespace` |
| 419 | `func_def` | `zt_prefix_ast_name` |
| 439 | `func_def` | `zt_prefix_declaration_for_namespace` |
| 466 | `func_def` | `zt_validate_source_namespaces` |
| 525 | `func_def` | `zt_parse_project_sources` |
| 552 | `func_def` | `zt_build_combined_project_ast` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
