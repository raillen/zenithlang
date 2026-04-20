# Driver/CLI - Code Map

## 📋 Descrição

Ponto de entrada do compiler. Responsável por:
- Parse de argumentos de linha de comando
- Orquestração do pipeline de compilação
- Integração com LSP (Language Server Protocol)
- Geração de documentação (ZDoc)

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `main.c` | 147 KB | Entry point, CLI parsing, compilation pipeline |
| `lsp.c` | 8.7 KB | Language Server Protocol implementation |
| `doc_show.c` | 19.3 KB | Documentation display/generation |

## 🔍 Funções Críticas

### main.c
| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🔴 CRÍTICA |

### lsp.c
| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟡 MÉDIA |

### doc_show.c
| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟢 BAIXA |

## ⚠️ Estado Crítico

- **Argumentos globais**: `argc`, `argv` (parsing inicial)
- **Configuração de compilação**: flags de otimização, target
- **Error handling**: cleanup de recursos em caso de falha

## 🔗 Dependencies Externas

- `frontend/` → Lexer, Parser, AST
- `semantic/` → Binder, Type Checker
- `targets/c/` → Code Generation
- `utils/` → Arena, Diagnostics

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- Este arquivo é MASSIVO (147KB) → considerar refatoração
- Pipeline de compilação é sequencial → fácil de seguir
- LSP é independente do pipeline principal

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Critical
- Source files: 4
- Extracted symbols: 150

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/driver/doc_show.c` | 561 | 21 | 3 |
| `compiler/driver/doc_show.h` | 14 | 2 | 0 |
| `compiler/driver/lsp.c` | 243 | 8 | 6 |
| `compiler/driver/main.c` | 4172 | 119 | 17 |

### Local Dependencies

- `compiler/driver/doc_show.h`
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
| 33 | `macro` | `ZT_MKDIR` |
| 34 | `macro` | `ZT_GETCWD` |
| 35 | `macro` | `ZT_POPEN` |
| 36 | `macro` | `ZT_PCLOSE` |
| 41 | `macro` | `ZT_MKDIR` |
| 42 | `macro` | `ZT_GETCWD` |
| 43 | `macro` | `ZT_POPEN` |
| 44 | `macro` | `ZT_PCLOSE` |
| 57 | `struct` | `zt_path_filter_list` |
| 65 | `func_def` | `zt_apply_manifest_lang` |
| 81 | `func_def` | `zt_read_file` |
| 129 | `func_def` | `zt_write_file` |
| 146 | `func_def` | `zt_copy_text` |
| 156 | `func_def` | `zt_heap_strdup` |
| 168 | `func_def` | `zt_path_has_extension` |
| 176 | `func_def` | `zt_path_is_dir` |
| 188 | `func_def` | `zt_path_is_file` |
| 200 | `func_def` | `zt_join_path` |
| 226 | `func_def` | `zt_dirname` |
| 254 | `func_def` | `zt_get_current_dir` |
| 260 | `func_def` | `zt_find_project_root_from_cwd` |
| 283 | `func_def` | `zt_replace_extension` |
| 301 | `func_def` | `zt_make_dirs` |
| 334 | `func_def` | `zt_path_filter_list_dispose` |
| 347 | `func_def` | `zt_path_filter_list_push` |
| 373 | `func_def` | `zt_normalize_path_separators` |
| 382 | `func_def` | `zt_git_ref_is_safe` |
| 398 | `func_def` | `zt_load_since_filter` |
| 459 | `struct` | `zt_project_source_file` |
| 466 | `struct` | `zt_project_source_file_list` |
| 472 | `func_def` | `zt_project_source_file_list_init` |
| 477 | `func_def` | `zt_project_source_file_list_dispose` |
| 492 | `func_def` | `zt_normalize_path_inplace` |
| 503 | `func_def` | `zt_project_source_file_list_push` |
| 539 | `func_def` | `zt_project_discover_zt_files` |
| 627 | `func_def` | `zt_namespace_to_relative_path` |
| 647 | `func_def` | `zt_path_char_equal` |
| 656 | `func_def` | `zt_path_suffix_matches` |
| 680 | `func_def` | `zt_last_namespace_segment` |
| 687 | `func_def` | `zt_find_entry_root` |
| 702 | `func_def` | `zt_namespace_exists` |
| 706 | `func_def` | `zt_namespace_is_std` |
| 710 | `func_def` | `zt_find_namespace_index` |
| 725 | `func_def` | `zt_print_import_cycle` |
| 760 | `func_def` | `zt_detect_import_cycle_visit` |
| 802 | `func_def` | `zt_detect_import_cycles` |
| 831 | `func_def` | `zt_alias_for_namespace` |
| 856 | `func_def` | `zt_prefix_ast_name` |
| 876 | `func_def` | `zt_prefix_declaration_for_namespace` |
| 903 | `func_def` | `zt_validate_source_namespaces` |
| 962 | `func_def` | `zt_parse_project_sources` |
| 989 | `func_def` | `zt_build_combined_project_ast` |
| 1039 | `func_def` | `zt_print_usage` |
| 1076 | `func_decl` | `zt_print_diagnostics` |
| 1078 | `func_def` | `zt_diag_code_from_project_error` |
| 1097 | `func_def` | `zt_print_project_parse_error` |
| 1125 | `func_def` | `zt_diag_matches_focus` |
| 1144 | `func_def` | `zt_diag_matches_since` |
| 1167 | `func_def` | `zt_diag_matches_filters` |
| 1170 | `func_def` | `zt_print_diagnostics` |
| 1248 | `func_def` | `zt_print_single_diag` |
| 1269 | `func_def` | `zt_diag_code_from_zir_parse_error` |
| 1293 | `func_def` | `zt_diag_code_from_zir_verifier` |
| 1309 | `func_def` | `zt_diag_code_from_c_emit_error` |
| 1321 | `func_def` | `zt_diag_code_from_zdoc_code` |
| 1335 | `func_def` | `zt_append_zdoc_diagnostics` |
| 1359 | `func_def` | `zt_resolve_project_paths` |
| 1439 | `struct` | `zt_string_set` |
| 1445 | `func_def` | `zt_string_set_init` |
| 1452 | `func_def` | `zt_string_set_dispose` |
| 1464 | `func_def` | `zt_string_set_add` |
| 1492 | `func_def` | `zt_string_set_add_slice` |
| 1511 | `func_def` | `zt_should_skip_test_discovery_dir` |
| 1522 | `func_def` | `zt_dir_has_project_manifest` |
| 1529 | `func_def` | `zt_collect_test_projects` |
| 1607 | `func_decl` | `zt_handle_project_command` |
| 1608 | `func_decl` | `zt_collect_project_sources` |
| 1615 | `func_def` | `zt_token_is_identifier_text` |
| 1623 | `func_def` | `zt_file_has_test_marker` |
| 1683 | `func_def` | `zt_project_uses_test_markers` |
| 1713 | `struct` | `zt_attr_test_case` |
| 1719 | `struct` | `zt_attr_test_case_list` |
| 1725 | `func_def` | `zt_attr_test_case_list_init` |
| 1730 | `func_def` | `zt_attr_test_case_list_dispose` |
| 1736 | `func_def` | `zt_attr_test_case_list_push` |
| 1766 | `func_def` | `zt_collect_attr_tests` |
| 1821 | `func_def` | `zt_generate_attr_test_runner_source` |
| 1848 | `func_def` | `zt_generate_attr_test_manifest` |
| 1883 | `func_def` | `zt_run_attr_tests_for_project` |
| 2031 | `func_def` | `zt_type_ident_char` |
| 2035 | `func_def` | `zt_strip_type_whitespace` |
| 2057 | `func_def` | `zt_collect_generic_instances_in_range` |
| 2111 | `func_def` | `zt_collect_generic_instances_from_type_name` |
| 2123 | `func_def` | `zt_collect_generic_instances_from_expr` |
| 2236 | `func_def` | `zt_collect_generic_instances_from_module` |
| 2291 | `func_def` | `zt_format_generic_preview` |
| 2323 | `func_def` | `zt_enforce_monomorphization_limit` |
| 2368 | `struct` | `zt_project_compile_result` |
| 2378 | `func_def` | `zt_project_compile_result_init` |
| 2382 | `func_def` | `zt_project_compile_result_dispose` |
| 2391 | `func_def` | `zt_compile_project` |
| 2624 | `func_def` | `zt_emit_module_to_c` |
| 2650 | `func_def` | `zt_compile_c_file` |
| 2684 | `func_def` | `zt_normalize_system_path` |
| 2701 | `func_def` | `zt_decode_process_exit` |
| 2710 | `func_def` | `zt_run_executable` |
| 2739 | `func_def` | `zt_collect_project_sources` |
| 2810 | `func_def` | `zt_handle_doc_check` |
| 2886 | `func_def` | `zt_handle_fmt` |
| 2948 | `func_def` | `zt_handle_project_info` |
| 3010 | `func_def` | `zt_handle_project_command` |
| 3139 | `func_def` | `zt_handle_test` |
| 3327 | `func_def` | `zt_handle_zir_command` |
| 3505 | `func_def` | `zt_find_most_recent_file` |
| 3588 | `func_def` | `zt_find_focus_anchor` |
| 3725 | `func_def` | `zt_handle_summary` |
| 3786 | `func_def` | `zt_handle_resume` |
| 3826 | `func_def` | `zt_handle_perf` |
| 3859 | `func_def` | `main` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
