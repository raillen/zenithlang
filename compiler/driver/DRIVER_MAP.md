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
- Source files: 11
- Extracted symbols: 515

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/driver/doc_show.c` | 561 | 21 | 3 |
| `compiler/driver/doc_show.h` | 14 | 2 | 0 |
| `compiler/driver/driver_internal.h` | 248 | 75 | 19 |
| `compiler/driver/lsp.c` | 3831 | 192 | 7 |
| `compiler/driver/main.c` | 3437 | 76 | 2 |
| `compiler/driver/paths.c` | 475 | 24 | 1 |
| `compiler/driver/pipeline.c` | 2023 | 67 | 2 |
| `compiler/driver/project.c` | 746 | 22 | 1 |
| `compiler/driver/zpm.c` | 870 | 23 | 2 |
| `compiler/driver/zpm.h` | 29 | 11 | 0 |
| `compiler/driver/zpm_main.c` | 55 | 2 | 1 |

### Local Dependencies

- `compiler/driver/doc_show.h`
- `compiler/driver/driver_internal.h`
- `compiler/driver/main.c`
- `compiler/driver/zpm.h`
- `compiler/frontend/lexer/lexer.h`
- `compiler/frontend/parser/parser.h`
- `compiler/hir/lowering/from_ast.h`
- `compiler/project/lockfile.h`
- `compiler/project/zdoc.h`
- `compiler/project/ztproj.h`
- `compiler/semantic/binder/binder.h`
- `compiler/semantic/diagnostics/diagnostics.h`
- `compiler/semantic/types/checker.h`
- `compiler/targets/c/emitter.h`
- `compiler/tooling/formatter.h`
- `compiler/utils/arena.h`
- `compiler/utils/l10n.h`
- `compiler/utils/string_pool.h`
- `compiler/zir/lowering/from_hir.h`
- `compiler/zir/parser.h`
- `compiler/zir/verifier.h`
- `runtime/c/zenith_rt.h`

### Related Tests

- `tests/driver/test_diagnostics_rendering.c`
- `tests/driver/test_explain_cli.py`
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
| 46 | `macro` | `ZT_MKDIR` |
| 47 | `macro` | `ZT_GETCWD` |
| 48 | `macro` | `ZT_POPEN` |
| 49 | `macro` | `ZT_PCLOSE` |
| 54 | `macro` | `ZT_MKDIR` |
| 55 | `macro` | `ZT_GETCWD` |
| 56 | `macro` | `ZT_POPEN` |
| 57 | `macro` | `ZT_PCLOSE` |
| 67 | `struct` | `zt_path_filter_list` |
| 73 | `struct` | `zt_driver_context` |
| 95 | `struct` | `zt_project_source_file` |
| 102 | `struct` | `zt_project_source_file_list` |
| 110 | `func_decl` | `zt_read_file` |
| 111 | `func_decl` | `zt_write_file` |
| 112 | `func_decl` | `zt_copy_text` |
| 113 | `func_decl` | `zt_heap_strdup` |
| 114 | `func_decl` | `zt_path_has_extension` |
| 115 | `func_decl` | `zt_path_is_dir` |
| 116 | `func_decl` | `zt_path_is_file` |
| 117 | `func_decl` | `zt_join_path` |
| 118 | `func_decl` | `zt_dirname` |
| 119 | `func_decl` | `zt_get_current_dir` |
| 120 | `func_decl` | `zt_native_get_home_dir` |
| 121 | `func_decl` | `zt_find_project_root_from_cwd` |
| 122 | `func_decl` | `zt_replace_extension` |
| 123 | `func_decl` | `zt_normalize_path_separators` |
| 124 | `func_decl` | `zt_git_ref_is_safe` |
| 125 | `func_decl` | `zt_make_dirs` |
| 126 | `func_decl` | `zt_normalize_path_inplace` |
| 127 | `func_decl` | `zt_path_char_equal` |
| 128 | `func_decl` | `zt_path_suffix_matches` |
| 129 | `func_decl` | `zt_normalize_system_path` |
| 130 | `func_decl` | `zt_native_make_temp_path` |
| 135 | `func_decl` | `zt_native_spawn_process` |
| 139 | `func_decl` | `zt_native_remove_file_if_exists` |
| 143 | `func_decl` | `zt_project_source_file_list_init` |
| 144 | `func_decl` | `zt_project_source_file_list_dispose` |
| 145 | `func_decl` | `zt_project_source_file_list_push` |
| 146 | `func_decl` | `zt_project_discover_zt_files` |
| 147 | `func_decl` | `zt_project_discover_packages` |
| 148 | `func_decl` | `zt_namespace_to_relative_path` |
| 149 | `func_decl` | `zt_last_namespace_segment` |
| 150 | `func_decl` | `zt_find_entry_root` |
| 151 | `func_decl` | `zt_namespace_exists` |
| 152 | `func_decl` | `zt_namespace_is_std` |
| 153 | `func_decl` | `zt_find_namespace_index` |
| 154 | `func_decl` | `zt_detect_import_cycles` |
| 155 | `func_decl` | `zt_validate_source_namespaces` |
| 159 | `func_decl` | `zt_parse_project_sources` |
| 160 | `func_decl` | `zt_build_combined_project_ast` |
| 163 | `func_decl` | `zt_load_since_filter` |
| 167 | `func_decl` | `zt_path_filter_list_dispose` |
| 168 | `func_decl` | `zt_path_filter_list_push` |
| 172 | `func_decl` | `zt_driver_context_init` |
| 173 | `func_decl` | `zt_driver_context_dispose` |
| 174 | `func_decl` | `zt_driver_context_activate_project` |
| 178 | `func_decl` | `zt_apply_manifest_lang` |
| 179 | `func_decl` | `zt_print_diagnostics` |
| 183 | `func_decl` | `zt_print_single_diag` |
| 190 | `func_decl` | `zt_resolve_project_paths` |
| 196 | `func_decl` | `zt_print_project_parse_error` |
| 200 | `func_decl` | `zt_diag_code_from_zir_verifier` |
| 201 | `func_decl` | `zt_diag_code_from_zir_parse_error` |
| 202 | `func_decl` | `zt_diag_code_from_c_emit_error` |
| 203 | `func_decl` | `zt_collect_project_sources` |
| 213 | `struct` | `zt_project_compile_result` |
| 223 | `func_decl` | `zt_project_compile_result_init` |
| 224 | `func_decl` | `zt_project_compile_result_dispose` |
| 225 | `func_decl` | `zt_compile_project` |
| 229 | `func_decl` | `zt_emit_module_to_c` |
| 233 | `func_decl` | `zt_compile_c_file` |
| 238 | `func_decl` | `zt_run_executable` |
| 239 | `func_decl` | `zt_handle_doc_check` |
| 240 | `func_decl` | `zt_handle_project_command` |

#### `compiler/driver/lsp.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 19 | `macro` | `LSP_EXE_SUFFIX` |
| 22 | `macro` | `LSP_EXE_SUFFIX` |
| 26 | `macro` | `PATH_MAX` |
| 29 | `struct` | `lsp_sb` |
| 35 | `struct` | `lsp_doc` |
| 55 | `struct` | `lsp_stdlib_module` |
| 84 | `func_def` | `lsp_strdup` |
| 95 | `func_def` | `lsp_strndup` |
| 105 | `func_def` | `sb_init` |
| 112 | `func_def` | `sb_reserve` |
| 125 | `func_def` | `sb_append_len` |
| 134 | `func_def` | `sb_append` |
| 138 | `func_def` | `sb_appendf` |
| 161 | `func_def` | `sb_append_json_string` |
| 187 | `func_def` | `sb_take` |
| 195 | `func_def` | `lsp_log` |
| 203 | `func_def` | `read_exact` |
| 216 | `func_def` | `read_message` |
| 229 | `func_def` | `send_raw_json` |
| 235 | `func_def` | `send_response_take` |
| 248 | `func_def` | `send_notification_take` |
| 263 | `func_def` | `skip_ws` |
| 268 | `func_def` | `json_unescape_string_at` |
| 321 | `func_def` | `json_get_string_key` |
| 336 | `func_def` | `json_get_number_key` |
| 349 | `func_def` | `json_get_bool_key` |
| 364 | `func_def` | `json_get_id_raw` |
| 389 | `func_decl` | `lsp_strndup` |
| 392 | `func_def` | `uri_to_path_alloc` |
| 418 | `func_decl` | `sb_take` |
| 421 | `func_def` | `path_to_file_uri_alloc` |
| 442 | `func_def` | `read_file_text_alloc` |
| 475 | `func_def` | `doc_clear_analysis` |
| 494 | `func_def` | `doc_parse_only` |
| 503 | `func_def` | `doc_reparse` |
| 518 | `func_def` | `find_doc` |
| 526 | `func_def` | `upsert_doc` |
| 542 | `func_def` | `upsert_stdlib_doc` |
| 560 | `func_def` | `ensure_stdlib_docs_loaded` |
| 603 | `func_def` | `free_doc` |
| 612 | `func_def` | `remove_doc` |
| 626 | `func_def` | `free_all_docs` |
| 636 | `func_def` | `lsp_line` |
| 640 | `func_def` | `lsp_col` |
| 644 | `func_def` | `append_diag_list_json` |
| 680 | `func_def` | `publish_diagnostics` |
| 695 | `func_def` | `publish_empty_diagnostics` |
| 704 | `func_def` | `offset_from_position` |
| 722 | `func_def` | `position_from_offset` |
| 741 | `func_def` | `word_char` |
| 745 | `func_def` | `ident_char` |
| 749 | `func_def` | `word_at_position` |
| 764 | `func_def` | `ident_segment_at_position` |
| 785 | `func_def` | `completion_member_dot_offset` |
| 801 | `func_def` | `completion_is_member_access` |
| 805 | `func_def` | `completion_receiver_name` |
| 820 | `func_def` | `completion_is_import_path` |
| 854 | `func_def` | `node_symbol_name` |
| 867 | `func_def` | `node_symbol_kind` |
| 880 | `func_def` | `node_is_public` |
| 894 | `func_def` | `doc_module_name` |
| 899 | `func_def` | `namespace_is_std` |
| 903 | `func_def` | `lsp_doc_text` |
| 907 | `func_def` | `lsp_doc_heading_what` |
| 911 | `func_def` | `lsp_doc_heading_documentation` |
| 915 | `func_def` | `lsp_doc_heading_signature` |
| 919 | `func_def` | `lsp_doc_heading_usage` |
| 923 | `func_def` | `lsp_doc_module_label` |
| 927 | `func_def` | `lsp_missing_function_doc_text` |
| 938 | `func_def` | `lsp_namespace_to_zdoc_path` |
| 952 | `func_def` | `lsp_read_relative_file_from_roots` |
| 981 | `func_def` | `lsp_current_locale_dir` |
| 989 | `func_def` | `lsp_locale_base` |
| 1002 | `func_def` | `lsp_read_zdoc_text_for_module_path` |
| 1022 | `func_def` | `lsp_zdoc_target_name_matches` |
| 1031 | `func_def` | `lsp_trimmed_copy` |
| 1041 | `func_def` | `lsp_extract_zdoc_target` |
| 1067 | `func_def` | `lsp_zdoc_documentation_for_symbol` |
| 1082 | `func_def` | `last_namespace_segment` |
| 1089 | `func_def` | `docs_same_namespace` |
| 1095 | `func_def` | `find_import_decl_for_alias` |
| 1112 | `func_def` | `import_path_for_alias` |
| 1118 | `func_def` | `import_alias_for_path` |
| 1132 | `func_def` | `find_doc_by_namespace` |
| 1142 | `func_def` | `symbol_matches` |
| 1150 | `func_def` | `find_symbol_in_doc` |
| 1161 | `func_def` | `find_symbol` |
| 1195 | `func_def` | `find_symbol_with_doc` |
| 1237 | `func_def` | `append_formatted_node` |
| 1247 | `func_def` | `hover_result_json` |
| 1299 | `func_def` | `definition_result_json` |
| 1324 | `func_def` | `document_end_position` |
| 1342 | `func_def` | `formatting_result_json` |
| 1365 | `func_def` | `append_completion_item_full` |
| 1399 | `func_def` | `append_completion_item` |
| 1410 | `func_def` | `ast_node_starts_before_position` |
| 1419 | `func_def` | `set_type_candidate` |
| 1428 | `func_decl` | `find_type_for_binding_in_node` |
| 1435 | `func_def` | `find_type_for_binding_in_list` |
| 1447 | `func_def` | `find_type_for_binding_in_node` |
| 1513 | `func_def` | `completion_receiver_type` |
| 1520 | `func_def` | `skip_type_space` |
| 1525 | `func_def` | `type_starts_with_name` |
| 1537 | `func_def` | `type_base_name` |
| 1546 | `func_def` | `split_qualified_type_base` |
| 1576 | `func_def` | `find_doc_for_type_qualifier` |
| 1588 | `func_def` | `find_struct_decl_named_in_doc` |
| 1609 | `func_def` | `find_struct_decl_in_doc` |
| 1619 | `func_def` | `find_struct_decl` |
| 1642 | `func_def` | `append_struct_field_completions` |
| 1662 | `func_decl` | `append_function_signature_text` |
| 1663 | `func_decl` | `function_documentation_markdown` |
| 1665 | `func_def` | `append_apply_method_completions_from_doc` |
| 1716 | `func_def` | `append_apply_method_completions` |
| 1734 | `func_def` | `find_apply_method_in_doc` |
| 1767 | `func_def` | `find_apply_method` |
| 1797 | `func_decl` | `append_local_completions_from_node` |
| 1804 | `func_def` | `append_local_completions_from_list` |
| 1816 | `func_def` | `append_local_completions_from_node` |
| 1881 | `func_def` | `builtin_documentation_markdown` |
| 1905 | `func_def` | `append_builtin_function_completion` |
| 1918 | `func_def` | `append_builtin_completions` |
| 2093 | `func_def` | `completion_kind_for_node` |
| 2106 | `func_def` | `symbol_kind_for_node` |
| 2124 | `func_def` | `append_lsp_range` |
| 2143 | `func_def` | `symbol_display_name` |
| 2184 | `func_def` | `symbol_children_for_node` |
| 2196 | `func_def` | `append_document_symbol` |
| 2227 | `func_def` | `document_symbol_result_json` |
| 2242 | `func_def` | `ascii_contains_ci` |
| 2264 | `func_def` | `append_workspace_symbol` |
| 2288 | `func_def` | `workspace_symbol_result_json` |
| 2305 | `func_def` | `append_reference_location` |
| 2318 | `func_def` | `append_reference_declaration` |
| 2330 | `func_def` | `reference_match_has_boundaries` |
| 2341 | `func_def` | `append_reference_text_matches` |
| 2367 | `func_def` | `qualified_reference_name` |
| 2377 | `func_def` | `references_result_json` |
| 2419 | `func_def` | `call_target_at_position` |
| 2463 | `func_def` | `lsp_is_numeric_conversion_name` |
| 2479 | `func_def` | `builtin_signature_help_result_json` |
| 2589 | `func_def` | `signature_help_result_json` |
| 2683 | `func_def` | `rename_name_is_valid` |
| 2693 | `func_def` | `line_bounds_for_offset` |
| 2706 | `func_def` | `offset_is_in_string_or_comment` |
| 2724 | `func_def` | `line_start_offset` |
| 2735 | `func_def` | `declaration_name_range` |
| 2767 | `func_def` | `append_rename_text_edit` |
| 2781 | `func_def` | `append_rename_text_matches` |
| 2810 | `func_def` | `rename_edits_for_doc` |
| 2864 | `func_def` | `rename_result_json` |
| 2890 | `func_def` | `prepare_rename_result_json` |
| 2926 | `func_def` | `semantic_keyword_kind` |
| 2941 | `func_def` | `semantic_builtin_type` |
| 2953 | `func_def` | `append_semantic_token` |
| 2966 | `func_def` | `next_nonspace_char` |
| 2972 | `func_def` | `prev_nonspace_char` |
| 2979 | `func_def` | `semantic_tokens_result_json` |
| 3107 | `func_def` | `append_import_path_completions` |
| 3125 | `func_def` | `append_import_alias_completions` |
| 3142 | `func_decl` | `append_function_signature_text` |
| 3143 | `func_decl` | `append_function_call_snippet` |
| 3144 | `func_decl` | `function_documentation_markdown` |
| 3146 | `func_def` | `append_doc_symbol_completions` |
| 3181 | `func_def` | `append_function_signature_text` |
| 3203 | `func_def` | `line_bounds_for_line` |
| 3218 | `func_def` | `append_trimmed_comment_text` |
| 3233 | `func_def` | `line_is_doc_comment` |
| 3241 | `func_def` | `line_is_blank` |
| 3251 | `func_def` | `leading_doc_comment_for_node` |
| 3293 | `func_def` | `generated_function_summary` |
| 3299 | `func_def` | `append_function_declaration_signature` |
| 3305 | `func_def` | `append_function_usage_text` |
| 3328 | `func_def` | `function_documentation_markdown` |
| 3381 | `func_def` | `append_function_call_snippet` |
| 3406 | `func_def` | `append_module_member_completions` |
| 3449 | `func_def` | `append_imported_module_shortcut_completions` |
| 3509 | `func_def` | `append_member_completions` |
| 3540 | `func_def` | `completion_result_json` |
| 3582 | `func_def` | `handle_initialize` |
| 3615 | `func_def` | `handle_hover_or_definition` |
| 3629 | `func_def` | `handle_references` |
| 3642 | `func_def` | `handle_signature_help` |
| 3655 | `func_def` | `handle_rename` |
| 3669 | `func_def` | `handle_prepare_rename` |
| 3686 | `func_def` | `handle_formatting` |
| 3694 | `func_def` | `handle_completion` |
| 3710 | `func_def` | `handle_document_symbol` |
| 3718 | `func_def` | `handle_semantic_tokens_full` |
| 3726 | `func_def` | `handle_workspace_symbol` |
| 3733 | `func_def` | `handle_execute_command` |
| 3738 | `func_def` | `main` |

#### `compiler/driver/main.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 9 | `func_def` | `zt_driver_context_init` |
| 16 | `func_def` | `zt_driver_context_dispose` |
| 24 | `func_def` | `zt_driver_context_activate_project` |
| 58 | `func_def` | `zt_apply_manifest_lang` |
| 74 | `func_def` | `zt_cli_env_true` |
| 84 | `func_def` | `zt_cli_locale_has_utf8` |
| 94 | `func_def` | `zt_cli_init_terminal` |
| 124 | `func_def` | `zt_is_help_flag` |
| 129 | `func_def` | `zt_cli_lang` |
| 134 | `func_def` | `zt_cli_section_main` |
| 144 | `func_def` | `zt_cli_section_compat` |
| 154 | `func_def` | `zt_cli_section_options` |
| 164 | `func_def` | `zt_cli_section_tip` |
| 174 | `func_def` | `zt_cli_label_error` |
| 184 | `func_def` | `zt_cli_label_hint` |
| 194 | `func_def` | `zt_print_help_overview` |
| 216 | `func_def` | `zt_print_help_topic` |
| 287 | `func_def` | `zt_cli_fail` |
| 300 | `func_def` | `zt_driver_context_lock_profile` |
| 308 | `func_decl` | `zt_print_diagnostics` |
| 313 | `func_def` | `zt_diag_code_from_project_error` |
| 332 | `func_def` | `zt_print_project_parse_error` |
| 363 | `func_def` | `zt_diag_matches_focus` |
| 382 | `func_def` | `zt_diag_matches_since` |
| 405 | `func_def` | `zt_diag_matches_filters` |
| 409 | `func_def` | `zt_print_diagnostics` |
| 494 | `func_def` | `zt_print_single_diag` |
| 516 | `func_def` | `zt_diag_code_from_zir_parse_error` |
| 540 | `func_def` | `zt_diag_code_from_zir_verifier` |
| 556 | `func_def` | `zt_diag_code_from_c_emit_error` |
| 568 | `func_def` | `zt_diag_code_from_zdoc_code` |
| 582 | `func_def` | `zt_append_zdoc_diagnostics` |
| 606 | `func_def` | `zt_resolve_project_paths` |
| 693 | `struct` | `zt_string_set` |
| 699 | `func_def` | `zt_string_set_init` |
| 706 | `func_def` | `zt_string_set_dispose` |
| 718 | `func_def` | `zt_string_set_add` |
| 746 | `func_def` | `zt_should_skip_test_discovery_dir` |
| 757 | `func_def` | `zt_dir_has_project_manifest` |
| 764 | `func_def` | `zt_collect_test_projects` |
| 842 | `func_decl` | `zt_handle_project_command` |
| 848 | `func_decl` | `zt_collect_project_sources` |
| 856 | `func_def` | `zt_token_is_identifier_text` |
| 864 | `func_def` | `zt_file_has_test_marker` |
| 924 | `func_def` | `zt_project_uses_test_markers` |
| 954 | `struct` | `zt_attr_test_case` |
| 960 | `struct` | `zt_attr_test_case_list` |
| 966 | `func_def` | `zt_attr_test_case_list_init` |
| 971 | `func_def` | `zt_attr_test_case_list_dispose` |
| 977 | `func_def` | `zt_attr_test_case_list_push` |
| 1007 | `func_def` | `zt_collect_attr_tests` |
| 1066 | `func_def` | `zt_generate_attr_test_runner_source` |
| 1093 | `func_def` | `zt_generate_attr_test_manifest` |
| 1128 | `func_def` | `zt_run_attr_tests_for_project` |
| 1285 | `func_def` | `zt_handle_doc_check` |
| 1367 | `func_def` | `zt_handle_fmt` |
| 1449 | `func_def` | `zt_handle_project_info` |
| 1514 | `func_def` | `zt_handle_project_command` |
| 1657 | `func_def` | `zt_handle_test` |
| 1848 | `func_def` | `zt_handle_zir_command` |
| 2042 | `func_def` | `zt_find_most_recent_file` |
| 2128 | `func_def` | `zt_find_focus_anchor` |
| 2265 | `func_def` | `zt_handle_summary` |
| 2333 | `func_def` | `zt_handle_resume` |
| 2376 | `func_def` | `zt_handle_perf` |
| 2414 | `struct` | `zt_explain_entry` |
| 2506 | `func_def` | `zt_find_explain_entry` |
| 2517 | `func_def` | `zt_print_explain_known_examples` |
| 2527 | `func_def` | `zt_handle_explain` |
| 2561 | `func_def` | `zt_dir_is_empty` |
| 2604 | `func_def` | `zt_sanitize_project_name` |
| 2647 | `func_def` | `zt_sanitize_namespace_root` |
| 2690 | `func_def` | `zt_project_name_from_target` |
| 2735 | `func_def` | `zt_create_write_scaffold_file` |
| 2782 | `func_def` | `zt_handle_create` |
| 3051 | `func_def` | `main` |

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
| 209 | `func_def` | `zt_native_get_home_dir` |
| 238 | `macro` | `ZT_PATH_MAX_ANCESTOR_STEPS` |
| 246 | `macro` | `ZT_PATH_SCRATCH_FLOOR` |
| 248 | `func_def` | `zt_find_project_root_from_cwd` |
| 293 | `func_def` | `zt_replace_extension` |
| 313 | `func_def` | `zt_normalize_path_separators` |
| 321 | `func_def` | `zt_normalize_path_inplace` |
| 332 | `func_def` | `zt_path_char_equal` |
| 341 | `func_def` | `zt_path_suffix_matches` |
| 365 | `func_def` | `zt_normalize_system_path` |
| 384 | `func_def` | `zt_git_ref_is_safe` |
| 397 | `func_def` | `zt_make_dirs` |
| 441 | `func_def` | `zt_path_filter_list_dispose` |
| 453 | `func_def` | `zt_path_filter_list_push` |

#### `compiler/driver/pipeline.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 15 | `func_decl` | `zt_runtime_root` |
| 16 | `func_decl` | `zt_home_has_stdlib` |
| 18 | `func_def` | `zt_diagnostics_profile_is_strict` |
| 22 | `func_def` | `zt_handle_stage_diagnostics` |
| 39 | `struct` | `zt_string_set` |
| 45 | `func_def` | `zt_string_set_init` |
| 52 | `func_def` | `zt_string_set_dispose` |
| 64 | `func_def` | `zt_string_set_add` |
| 90 | `func_def` | `zt_string_set_add_slice` |
| 109 | `func_def` | `zt_native_token_text_equals` |
| 118 | `func_def` | `zt_native_token_is_import_part` |
| 124 | `func_def` | `zt_native_next_non_comment_token` |
| 134 | `func_def` | `zt_native_collect_std_imports_from_source` |
| 208 | `func_def` | `zt_native_collect_std_imports_from_file` |
| 222 | `func_def` | `zt_type_ident_char` |
| 226 | `func_def` | `zt_strip_type_whitespace` |
| 248 | `func_def` | `zt_collect_generic_instances_in_range` |
| 302 | `func_def` | `zt_collect_generic_instances_from_type_name` |
| 314 | `func_def` | `zt_collect_generic_instances_from_expr` |
| 427 | `func_def` | `zt_collect_generic_instances_from_module` |
| 482 | `func_def` | `zt_format_generic_preview` |
| 514 | `func_def` | `zt_enforce_monomorphization_limit` |
| 563 | `func_def` | `zt_project_compile_result_init` |
| 567 | `func_def` | `zt_project_compile_result_dispose` |
| 576 | `func_def` | `zt_compile_project` |
| 822 | `func_def` | `zt_emit_module_to_c` |
| 848 | `func_def` | `zt_get_file_mtime` |
| 856 | `func_def` | `zt_get_executable_dir` |
| 878 | `func_def` | `zt_home_has_runtime` |
| 885 | `func_def` | `zt_home_has_stdlib` |
| 892 | `func_def` | `zt_runtime_root` |
| 912 | `func_def` | `zt_join_runtime_dep` |
| 919 | `func_def` | `zt_runtime_object_is_stale` |
| 944 | `func_def` | `zt_native_trim_line` |
| 954 | `func_def` | `zt_native_process_id` |
| 962 | `func_def` | `zt_native_sleep_millis` |
| 970 | `func_def` | `zt_native_make_temp_path` |
| 996 | `func_decl` | `zt_native_decode_status` |
| 999 | `func_def` | `zt_native_dup_prefixed_arg` |
| 1018 | `func_def` | `zt_native_display_needs_quotes` |
| 1030 | `func_def` | `zt_native_format_command` |
| 1075 | `func_def` | `zt_native_append_windows_quoted_arg` |
| 1118 | `func_def` | `zt_native_build_windows_command_line` |
| 1147 | `func_def` | `zt_native_write_capture_message` |
| 1165 | `func_def` | `zt_native_spawn_process` |
| 1304 | `func_def` | `zt_native_remove_file_if_exists` |
| 1310 | `func_def` | `zt_runtime_object_lock_release` |
| 1323 | `func_def` | `zt_runtime_object_lock_acquire` |
| 1371 | `func_decl` | `zt_ensure_runtime_object_current` |
| 1373 | `func_def` | `zt_native_line_is_warning` |
| 1378 | `func_def` | `zt_native_line_is_compiler_not_found` |
| 1386 | `func_def` | `zt_native_line_is_error_undefined_reference` |
| 1391 | `func_def` | `zt_native_line_is_error_fatal` |
| 1396 | `func_def` | `zt_native_line_is_error` |
| 1405 | `func_def` | `zt_native_line_is_filtered_warning` |
| 1414 | `func_def` | `zt_native_line_is_warning_format_truncation` |
| 1419 | `func_def` | `zt_native_line_is_warning_sign_compare` |
| 1424 | `func_def` | `zt_native_line_is_warning_unused_variable` |
| 1429 | `func_def` | `zt_native_capture_first` |
| 1436 | `func_def` | `zt_native_decode_status` |
| 1447 | `func_def` | `zt_run_native_compile_command` |
| 1666 | `func_def` | `zt_compile_runtime_object` |
| 1750 | `func_def` | `zt_ensure_runtime_object_current` |
| 1792 | `func_def` | `zt_compile_c_file` |
| 1914 | `func_def` | `zt_decode_process_exit` |
| 1923 | `func_def` | `zt_run_executable` |
| 1951 | `func_def` | `zt_collect_project_sources` |

#### `compiler/driver/project.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 16 | `func_def` | `zt_project_source_file_list_init` |
| 21 | `func_def` | `zt_project_source_file_list_dispose` |
| 36 | `func_def` | `zt_load_since_filter` |
| 124 | `func_def` | `zt_project_source_file_list_push` |
| 175 | `func_def` | `zt_join_path_alloc` |
| 198 | `func_def` | `zt_project_discover_zt_files` |
| 302 | `func_def` | `zt_project_discover_packages` |
| 357 | `func_def` | `zt_namespace_to_relative_path` |
| 378 | `func_def` | `zt_last_namespace_segment` |
| 385 | `func_def` | `zt_find_entry_root` |
| 400 | `func_def` | `zt_namespace_exists` |
| 404 | `func_def` | `zt_namespace_is_std` |
| 408 | `func_def` | `zt_find_namespace_index` |
| 423 | `func_def` | `zt_print_import_cycle` |
| 458 | `func_def` | `zt_detect_import_cycle_visit` |
| 500 | `func_def` | `zt_detect_import_cycles` |
| 529 | `func_def` | `zt_alias_for_namespace` |
| 554 | `func_def` | `zt_prefix_ast_name` |
| 574 | `func_def` | `zt_prefix_declaration_for_namespace` |
| 603 | `func_def` | `zt_validate_source_namespaces` |
| 662 | `func_def` | `zt_parse_project_sources` |
| 698 | `func_def` | `zt_build_combined_project_ast` |

#### `compiler/driver/zpm.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 4 | `func_def` | `zt_zpm_print_help` |
| 22 | `struct` | `zt_semver` |
| 28 | `enum` | `zt_semver_range_kind` |
| 34 | `struct` | `zt_semver_range` |
| 39 | `func_def` | `zt_semver_parse` |
| 44 | `func_def` | `zt_semver_range_parse` |
| 57 | `func_def` | `zt_semver_satisfies` |
| 76 | `func_def` | `zt_semver_compare` |
| 82 | `func_def` | `zt_zpm_resolve_remote_version` |
| 152 | `macro` | `ZT_ZPM_REGISTRY_URL` |
| 153 | `macro` | `ZT_ZPM_BASE_PATH_CAP` |
| 154 | `macro` | `ZT_ZPM_PATH_CAP` |
| 155 | `macro` | `ZT_ZPM_LONG_PATH_CAP` |
| 157 | `func_def` | `zt_zpm_sync_registry` |
| 184 | `func_def` | `zt_zpm_load_url_from_cache` |
| 226 | `func_def` | `zt_zpm_get_package_url` |
| 236 | `func_def` | `zt_zpm_resolve_project` |
| 241 | `func_def` | `zt_zpm_unquote_spec_value` |
| 261 | `func_def` | `zt_zpm_lock_version_or_git` |
| 285 | `func_def` | `zt_handle_zpm` |
| 371 | `func_def` | `zt_handle_zpm_init` |
| 438 | `func_def` | `zt_handle_zpm_add` |
| 544 | `func_def` | `zt_handle_zpm_install` |

#### `compiler/driver/zpm.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 7 | `macro` | `ZT_ZPM_H` |
| 15 | `func_decl` | `zt_handle_zpm` |
| 18 | `func_decl` | `zt_handle_zpm_init` |
| 19 | `func_decl` | `zt_handle_zpm_add` |
| 20 | `func_decl` | `zt_handle_zpm_remove` |
| 21 | `func_decl` | `zt_handle_zpm_install` |
| 22 | `func_decl` | `zt_handle_zpm_update` |
| 23 | `func_decl` | `zt_handle_zpm_list` |
| 24 | `func_decl` | `zt_handle_zpm_find` |
| 25 | `func_decl` | `zt_handle_zpm_publish` |
| 26 | `func_decl` | `zt_handle_zpm_run` |

#### `compiler/driver/zpm_main.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 4 | `macro` | `ZT_DRIVER_NO_MAIN` |
| 14 | `func_def` | `main` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
