# Implementation Cascade

Este arquivo liga `IMPLEMENTATION_CHECKLIST.md`, `IMPLEMENTATION_ROADMAP.md` e os `*_MAP.md`.

Uso recomendado:

1. Ache o milestone no checklist ou no roadmap.
2. Abra a secao correspondente aqui.
3. Siga a ordem checklist -> roadmap -> mapas -> codigo -> testes.
4. Use os simbolos e linhas sugeridos como janela inicial, nao como verdade absoluta.

Regra pratica:

- checklist = item exato e status
- roadmap = objetivo e escopo
- code maps = arquivo, simbolo e linha de entrada

## M38. Hardening de Coerencia Frontend->Backend (extern/where/params)

### Cascade Order

1. Checklist: `IMPLEMENTATION_CHECKLIST.md` lines 714-727
2. Roadmap: `IMPLEMENTATION_ROADMAP.md` lines 1147-1164
3. Map: `compiler/driver/DRIVER_MAP.md`
4. Map: `compiler/frontend/ast/AST_MAP.md`
5. Map: `compiler/frontend/parser/PARSER_MAP.md`
6. Map: `compiler/hir/lowering/HIR_LOWERING_MAP.md`
7. Map: `compiler/project/PROJECT_MAP.md`
8. Map: `compiler/semantic/binder/BINDER_MAP.md`
9. Map: `compiler/semantic/diagnostics/DIAGNOSTICS_MAP.md`
10. Map: `compiler/semantic/symbols/SYMBOLS_MAP.md`
11. Map: `compiler/semantic/types/TYPE_SYSTEM_MAP.md`
12. Map: `compiler/targets/c/EMITTER_MAP.md`
13. Map: `compiler/tooling/FORMATTER_MAP.md`
14. Map: `compiler/zir/ZIR_MODEL_MAP.md`
15. Map: `compiler/zir/ZIR_PARSER_MAP.md`
16. Map: `compiler/zir/ZIR_VERIFIER_MAP.md`

### Checklist Status

- Done items: 0
- Open items: 11
- Pending work:
  - HIR lowering: coletar simbolos `extern c` e preservar nome ABI correto no call lowering
  - Backend C: impedir mangling indevido em chamadas `extern c`
  - Criar behavior test E2E de `extern c` com chamada real (`puts` ou equivalente)
  - Parser: normalizar `where` de parametro no mesmo shape usado por `where` canonico
  - Binder/checker: validar `where` de parametro no mesmo estagio de `where` de campo
  - Typechecker: exigir predicate `where` de parametro com tipo `bool`
  - Conectar `parameter_validation` ao pipeline semantico real
  - Emitir diagnostico claro para regra "required after default" em assinatura
  - Cobrir com testes: `param where` invalido, `param where` nao-booleano, ordem invalida de parametros
  - Resolver drift de interpolacao: decidir suporte final e alinhar parser/HIR/backend/formatter/docs
  - Remover risco de truncamento em nomes longos no parser e emitir erro estavel quando exceder limite

### Roadmap Summary

- Status roadmap: planejado (gate de coerencia antes de release).
- Objetivo: fechar inconsistencias entre parser/binder/typechecker/HIR/backend que hoje permitem codigo aceito quebrar tarde no pipeline.
- Dependencias: M16, M23, M33.

### Entry Files

- `compiler/driver/main.c`
  - Closest map: `compiler/driver/DRIVER_MAP.md`
  - Start near: `zt_normalize_path_separators` (func_def, line 373), `zt_diag_code_from_project_error` (func_def, line 1078), `zt_print_project_parse_error` (func_def, line 1097), `zt_diag_code_from_zir_parse_error` (func_def, line 1269), `zt_diag_code_from_c_emit_error` (func_def, line 1309), `zt_should_skip_test_discovery_dir` (func_def, line 1511), `zt_collect_test_projects` (func_def, line 1529), `zt_file_has_test_marker` (func_def, line 1623)
- `compiler/frontend/ast/model.c`
  - Closest map: `compiler/frontend/ast/AST_MAP.md`
  - Start near: `zt_ast_kind_name` (func_def, line 6), `zt_ast_make` (func_def, line 66), `zt_ast_node_list_push` (func_def, line 75), `zt_ast_named_arg_list_push` (func_def, line 88), `zt_ast_map_entry_list_push` (func_def, line 101), `zt_ast_node_list_make` (func_def, line 114), `zt_ast_map_entry_list_make` (func_def, line 122), `zt_ast_named_arg_list_make` (func_def, line 131)
- `compiler/frontend/ast/model.h`
  - Closest map: `compiler/frontend/ast/AST_MAP.md`
  - Start near: `ZENITH_NEXT_COMPILER_FRONTEND_AST_MODEL_H` (macro, line 2)
- `compiler/frontend/parser/parser.c`
  - Closest map: `compiler/frontend/parser/PARSER_MAP.md`
  - Start near: `zt_parser` (struct, line 9), `zt_parser_next_non_comment_token` (func_def, line 26), `zt_parser_advance` (func_def, line 47), `zt_parser_fill_peek` (func_def, line 58), `zt_parser_check` (func_def, line 65), `zt_parser_match` (func_def, line 69), `zt_parser_is_declaration_start` (func_def, line 77), `zt_parser_is_member_start` (func_def, line 96)
- `compiler/frontend/parser/parser.h`
  - Closest map: `compiler/frontend/parser/PARSER_MAP.md`
  - Start near: `ZENITH_NEXT_COMPILER_FRONTEND_PARSER_PARSER_H` (macro, line 2)
- `compiler/hir/nodes/model.c`
  - Closest map: `-`
  - Start near: `zt_hir_realloc_array` (func_def, line 61), `zt_hir_param_list_make` (func_def, line 108), `zt_hir_param_list_push` (func_def, line 160), `zt_hir_param_list_dispose` (func_def, line 396), `zt_hir_decl_kind_name` (func_def, line 6), `zt_hir_stmt_kind_name` (func_def, line 15), `zt_hir_expr_kind_name` (func_def, line 36), `zt_hir_module_make` (func_def, line 71)
- `compiler/hir/nodes/model.h`
  - Closest map: `-`
  - Start near: `ZENITH_NEXT_COMPILER_HIR_NODES_MODEL_H` (macro, line 2)
- `compiler/semantic/binder/binder.c`
  - Closest map: `compiler/semantic/binder/BINDER_MAP.md`
  - Start near: `zt_binder` (struct, line 5), `zt_bind_type_param_defs` (func_def, line 452), `zt_bind_param_list` (func_def, line 463), `zt_bind_where_clause` (func_def, line 475), `zt_scope_collect_names` (func_def, line 9), `zt_bind_emit_unresolved_with_suggestion` (func_def, line 23), `zt_bind_warn_confusing_name` (func_def, line 41), `zt_is_builtin_type_name` (func_def, line 83)
- `compiler/semantic/binder/binder.h`
  - Closest map: `compiler/semantic/binder/BINDER_MAP.md`
  - Start near: `ZENITH_NEXT_COMPILER_SEMANTIC_BINDER_H` (macro, line 2)
- `compiler/semantic/diagnostics/diagnostics.c`
  - Closest map: `compiler/semantic/diagnostics/DIAGNOSTICS_MAP.md`
  - Start near: `zt_diag_default_help` (func_def, line 237), `zt_cog_profile_error_limit` (func_def, line 547), `zt_diag_same_text` (func_def, line 12), `zt_diag_same_span` (func_def, line 18), `zt_diag_is_duplicate_recent` (func_def, line 25), `zt_diag_print_source_span` (func_def, line 49), `zt_diag_read_source_line` (func_def, line 63), `zt_diag_render_caret` (func_def, line 103)
- `compiler/semantic/diagnostics/diagnostics.h`
  - Closest map: `compiler/semantic/diagnostics/DIAGNOSTICS_MAP.md`
  - Start near: `ZENITH_NEXT_COMPILER_SEMANTIC_DIAGNOSTICS_H` (macro, line 2)
- `compiler/semantic/parameter_validation.c`
  - Closest map: `compiler/semantic/diagnostics/DIAGNOSTICS_MAP.md`
  - Start near: `zt_validate_parameter_ordering` (func_def, line 26), `zt_validate_argument_ordering` (func_def, line 67)
- `compiler/semantic/symbols/symbols.c`
  - Closest map: `compiler/semantic/symbols/SYMBOLS_MAP.md`
  - Start near: `zt_symbols_strdup` (func_def, line 6), `zt_symbol_kind_name` (func_def, line 18), `zt_scope_init` (func_def, line 36), `zt_scope_dispose` (func_def, line 44), `zt_scope_lookup_current` (func_def, line 58), `zt_scope_lookup_parent_chain` (func_def, line 70), `zt_scope_lookup` (func_def, line 83), `zt_scope_declare` (func_def, line 93)
- `compiler/semantic/symbols/symbols.h`
  - Closest map: `compiler/semantic/symbols/SYMBOLS_MAP.md`
  - Start near: `ZENITH_NEXT_COMPILER_SEMANTIC_SYMBOLS_H` (macro, line 2)
- `compiler/semantic/types/checker.c`
  - Closest map: `compiler/semantic/types/TYPE_SYSTEM_MAP.md`
  - Start near: `zt_checker` (struct, line 62), `zt_checker_strdup` (func_def, line 84), `zt_checker_is_self_prefix` (func_def, line 345), `zt_checker_decl_is_public` (func_def, line 359), `zt_decl_type_param_count` (func_def, line 383), `zt_checker_diag_type` (func_def, line 436), `zt_checker_compute_integral_binary` (func_def, line 524), `zt_type_param_has_trait` (func_def, line 556)
- `compiler/semantic/types/checker.h`
  - Closest map: `compiler/semantic/types/TYPE_SYSTEM_MAP.md`
  - Start near: `ZENITH_NEXT_COMPILER_SEMANTIC_TYPES_CHECKER_H` (macro, line 2)
- `compiler/semantic/types/types.c`
  - Closest map: `compiler/semantic/types/TYPE_SYSTEM_MAP.md`
  - Start near: `zt_types_strdup` (func_def, line 7), `zt_type_kind_name` (func_def, line 19), `zt_type_list_make` (func_def, line 59), `zt_type_list_push` (func_def, line 67), `zt_type_list_dispose` (func_def, line 82), `zt_type_make` (func_def, line 95), `zt_type_make_named` (func_def, line 103), `zt_type_make_with_args` (func_def, line 110)
- `compiler/semantic/types/types.h`
  - Closest map: `compiler/semantic/types/TYPE_SYSTEM_MAP.md`
  - Start near: `ZENITH_NEXT_COMPILER_SEMANTIC_TYPES_TYPES_H` (macro, line 2)
- `compiler/targets/c/emitter.c`
  - Closest map: `compiler/targets/c/EMITTER_MAP.md`
  - Start near: `c_canonicalize_type` (func_def, line 48), `c_emit_error_code_name` (func_def, line 107), `c_expression_is_materialized_core_error_ref` (func_def, line 976), `c_outcome_emit_failure_from_error_expr` (func_def, line 1380), `c_emit_call_args` (func_def, line 2043), `c_emit_call_expr` (func_def, line 2101), `c_emit_zir_call_expr` (func_def, line 3129), `c_extern_call_expected_arg_type` (func_def, line 3286)
- `compiler/targets/c/emitter.h`
  - Closest map: `compiler/targets/c/EMITTER_MAP.md`
  - Start near: `ZENITH_NEXT_COMPILER_TARGETS_C_EMITTER_H` (macro, line 2)
- `compiler/targets/c/legalization.c`
  - Closest map: `compiler/targets/c/EMITTER_MAP.md`
  - Start near: `c_legalize_error_code_name` (func_def, line 36), `c_legalize_safe_text` (func_def, line 8), `c_legalize_set_result` (func_def, line 12), `c_legalize_result_init` (func_def, line 32), `c_legalize_is_blank` (func_def, line 51), `c_legalize_starts_with` (func_def, line 68), `c_legalize_copy_trimmed` (func_def, line 76), `c_legalize_copy_trimmed_segment` (func_def, line 110)
- `compiler/targets/c/legalization.h`
  - Closest map: `compiler/targets/c/EMITTER_MAP.md`
  - Start near: `ZENITH_NEXT_COMPILER_TARGETS_C_LEGALIZATION_H` (macro, line 2)
- `compiler/tooling/formatter.c`
  - Closest map: `compiler/tooling/FORMATTER_MAP.md`
  - Start near: `sb_t` (struct, line 7), `sb_init` (func_def, line 14), `sb_free` (func_def, line 22), `sb_append_len` (func_def, line 26), `sb_append` (func_def, line 41), `sb_indent` (func_def, line 45), `format_comments` (func_def, line 53), `op_to_str` (func_def, line 62)
- `compiler/tooling/formatter.h`
  - Closest map: `compiler/tooling/FORMATTER_MAP.md`
  - Start near: `ZENITH_TOOLING_FORMATTER_H` (macro, line 2)
- `compiler/zir/lowering/from_hir.c`
  - Closest map: `-`
  - Start near: `zir_call_is_module_func` (func_def, line 123), `zir_call_add_lowered_args` (func_def, line 332), `zir_lower_len_call` (func_def, line 348), `zir_lower_call_expr` (func_def, line 405), `zir_instruction_buffer` (struct, line 12), `zir_block_state` (struct, line 18), `zir_block_state_buffer` (struct, line 26), `zir_loop_target` (struct, line 32)
- `compiler/zir/lowering/from_hir.h`
  - Closest map: `-`
  - Start near: `ZENITH_NEXT_COMPILER_ZIR_LOWERING_FROM_HIR_H` (macro, line 2)

### Tests And Docs

- Tests:
  - `tests/formatter/run_formatter_golden.py`
  - `tests/frontend/test_parser.c`
  - `tests/frontend/test_parser_error_recovery.c`
  - `tests/semantic/test_binder.c`
  - `tests/semantic/test_constraints.c`
  - `tests/semantic/test_hir_lowering.c`
  - `tests/semantic/test_types.c`
  - `tests/targets/c/test_emitter.c`
  - `tests/targets/c/test_legalization.c`
  - `tests/test_diagnostics.zt`
  - `tests/zir/test_lowering.c`
  - `tests/zir/test_printer.c`
  - `tests/zir/test_verifier.c`

### Prompt Seed

```text
Implement M38 (Hardening de Coerencia Frontend->Backend (extern/where/params)).
Start from IMPLEMENTATION_CHECKLIST.md lines 714-727.
Then read IMPLEMENTATION_ROADMAP.md lines 1147-1164.
Then open these maps:
- compiler/driver/DRIVER_MAP.md
- compiler/frontend/ast/AST_MAP.md
- compiler/frontend/parser/PARSER_MAP.md
- compiler/hir/lowering/HIR_LOWERING_MAP.md
- compiler/project/PROJECT_MAP.md
- compiler/semantic/binder/BINDER_MAP.md
- compiler/semantic/diagnostics/DIAGNOSTICS_MAP.md
- compiler/semantic/symbols/SYMBOLS_MAP.md
- compiler/semantic/types/TYPE_SYSTEM_MAP.md
- compiler/targets/c/EMITTER_MAP.md
- compiler/tooling/FORMATTER_MAP.md
- compiler/zir/ZIR_MODEL_MAP.md
- compiler/zir/ZIR_PARSER_MAP.md
- compiler/zir/ZIR_VERIFIER_MAP.md
Then inspect these source files first:
- compiler/driver/main.c
- compiler/frontend/ast/model.c
- compiler/frontend/ast/model.h
- compiler/frontend/parser/parser.c
- compiler/frontend/parser/parser.h
- compiler/hir/nodes/model.c
Validate with these tests first:
- tests/formatter/run_formatter_golden.py
- tests/frontend/test_parser.c
- tests/frontend/test_parser_error_recovery.c
- tests/semantic/test_binder.c
- tests/semantic/test_constraints.c
- tests/semantic/test_hir_lowering.c
Return only: files to edit, first symbols to inspect, validation plan, and main regression risk.
```
