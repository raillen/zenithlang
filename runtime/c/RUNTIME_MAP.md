# Runtime C - Code Map

## 📋 Descrição

Runtime do Zenith em C. Responsável por:
- Funções de runtime para código compilado
- Memory management (GC se aplicável)
- Built-in functions (strings, lists, I/O)
- Error handling e exceptions
- Platform abstraction

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `zenith_rt.c` | 231.5 KB | Implementação completa do runtime |
| `zenith_rt.h` | 39.4 KB | Runtime interfaces, types |
| `RUNTIME_DIAGNOSTICO_COMPLETO.md` | 4.9 KB | Documentação de diagnóstico |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🔴 CRÍTICA |

## ⚠️ Estado Crítico

- **Runtime state**: estado global do runtime
- **Memory pools**: gerenciamento de memória
- **Built-in cache**: functions pré-alocadas

## 🔗 Dependencies Externas

- `compiler/targets/c/` → Emitter chama runtime
- Platform libs (POSIX, Windows API)

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- ARQUIVO ENORME (231KB) → dividir urgentemente
- Runtime é linkado com TODO código compilado
- Bugs aqui afetam todos os programas Zenith

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Critical
- Source files: 2
- Extracted symbols: 592

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `runtime/c/zenith_rt.c` | 7467 | 591 | 1 |
| `runtime/c/zenith_rt.h` | 890 | 1 | 0 |

### Local Dependencies

- `runtime/c/zenith_rt.h`

### Related Tests

- `tests/behavior/where_contract_construct_error/src/app/main.zt`
- `tests/behavior/where_contract_field_assign_error/src/app/main.zt`
- `tests/behavior/where_contract_param_error/src/app/main.zt`
- `tests/behavior/where_contract_param_where_invalid_error/src/app/main.zt`
- `tests/behavior/where_contract_param_where_non_bool_error/src/app/main.zt`
- `tests/behavior/where_contracts_ok/src/app/main.zt`
- `tests/runtime/c/README.md`
- `tests/runtime/c/test_runtime.c`
- `tests/runtime/c/test_shared_text.c`
- `tests/runtime/stress_tests.c`
- `tests/runtime/test_fase11_safety.zt`

### Symbol Index

#### `runtime/c/zenith_rt.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 15 | `macro` | `WIN32_LEAN_AND_MEAN` |
| 34 | `macro` | `ZT_NET_INVALID_SOCKET` |
| 37 | `macro` | `ZT_NET_INVALID_SOCKET` |
| 40 | `func_def` | `zt_hash_text` |
| 49 | `func_def` | `zt_safe_message` |
| 53 | `func_def` | `zt_runtime_append_text` |
| 73 | `func_def` | `zt_runtime_store_error` |
| 90 | `func_def` | `zt_runtime_stable_code` |
| 107 | `func_def` | `zt_runtime_default_help` |
| 136 | `func_def` | `zt_runtime_print_error` |
| 171 | `func_def` | `zt_outcome_i64_text_failure_message` |
| 178 | `func_def` | `zt_outcome_text_text_failure_message` |
| 185 | `func_def` | `zt_outcome_optional_text_text_failure_message` |
| 192 | `func_def` | `zt_outcome_bytes_text_failure_message` |
| 199 | `func_def` | `zt_outcome_optional_bytes_text_failure_message` |
| 206 | `func_def` | `zt_outcome_net_connection_text_failure_message` |
| 213 | `func_def` | `zt_outcome_void_text_failure_message` |
| 220 | `func_def` | `zt_outcome_list_i64_text_failure_message` |
| 227 | `func_decl` | `zt_host_default_read_file` |
| 228 | `func_decl` | `zt_host_default_write_file` |
| 229 | `func_decl` | `zt_host_default_path_exists` |
| 230 | `func_decl` | `zt_host_default_read_line_stdin` |
| 231 | `func_decl` | `zt_host_default_read_all_stdin` |
| 232 | `func_decl` | `zt_host_default_write_stdout` |
| 233 | `func_decl` | `zt_host_default_write_stderr` |
| 234 | `func_decl` | `zt_host_default_time_now_unix_ms` |
| 235 | `func_decl` | `zt_host_default_time_sleep_ms` |
| 236 | `func_decl` | `zt_host_default_random_seed` |
| 237 | `func_decl` | `zt_host_default_random_next_i64` |
| 238 | `func_decl` | `zt_host_default_os_current_dir` |
| 239 | `func_decl` | `zt_host_default_os_change_dir` |
| 240 | `func_decl` | `zt_host_default_os_env` |
| 241 | `func_decl` | `zt_host_default_os_pid` |
| 242 | `func_decl` | `zt_host_default_os_platform` |
| 243 | `func_decl` | `zt_host_default_os_arch` |
| 244 | `func_decl` | `zt_host_default_process_run` |
| 267 | `func_def` | `zt_runtime_span_unknown` |
| 275 | `func_def` | `zt_runtime_make_span` |
| 283 | `func_def` | `zt_runtime_span_is_known` |
| 290 | `func_def` | `zt_runtime_last_error` |
| 294 | `func_def` | `zt_runtime_clear_error` |
| 304 | `func_def` | `zt_error_kind_name` |
| 333 | `func_def` | `zt_header_from_ref` |
| 337 | `func_def` | `zt_free_text` |
| 348 | `func_def` | `zt_free_bytes` |
| 359 | `func_def` | `zt_free_list_i64` |
| 371 | `func_def` | `zt_free_list_text` |
| 389 | `func_def` | `zt_free_dyn_text_repr` |
| 402 | `func_def` | `zt_free_list_dyn_text_repr` |
| 419 | `func_def` | `zt_free_map_text_text` |
| 441 | `func_def` | `zt_net_close_socket_handle` |
| 455 | `func_def` | `zt_free_net_connection` |
| 468 | `func_def` | `zt_runtime_require_text` |
| 474 | `func_def` | `zt_runtime_require_bytes` |
| 480 | `func_def` | `zt_runtime_require_net_connection` |
| 486 | `func_def` | `zt_runtime_require_list_i64` |
| 492 | `func_def` | `zt_runtime_require_list_text` |
| 498 | `func_def` | `zt_runtime_require_dyn_text_repr` |
| 504 | `func_def` | `zt_runtime_require_list_dyn_text_repr` |
| 509 | `func_def` | `zt_runtime_require_map_text_text` |
| 515 | `func_def` | `zt_normalize_slice_end` |
| 535 | `func_def` | `zt_utf8_is_continuation` |
| 539 | `func_def` | `zt_utf8_validate` |
| 783 | `func_def` | `zt_list_i64_reserve` |
| 810 | `func_def` | `zt_list_text_reserve` |
| 837 | `func_def` | `zt_list_dyn_text_repr_reserve` |
| 863 | `func_def` | `zt_map_text_text_reserve` |
| 897 | `func_def` | `zt_map_text_text_find_index` |
| 934 | `func_def` | `zt_map_text_text_alloc` |
| 947 | `func_decl` | `zt_free_grid2d_i64` |
| 948 | `func_decl` | `zt_free_grid2d_text` |
| 949 | `func_decl` | `zt_free_pqueue_i64` |
| 950 | `func_decl` | `zt_free_pqueue_text` |
| 951 | `func_decl` | `zt_free_circbuf_i64` |
| 952 | `func_decl` | `zt_free_circbuf_text` |
| 953 | `func_decl` | `zt_free_btreemap_text_text` |
| 954 | `func_decl` | `zt_free_btreeset_text` |
| 955 | `func_decl` | `zt_free_grid3d_i64` |
| 956 | `func_decl` | `zt_free_grid3d_text` |
| 957 | `func_decl` | `zt_free_net_connection` |
| 959 | `func_def` | `zt_retain` |
| 977 | `func_def` | `zt_release` |
| 1061 | `func_def` | `zt_deep_copy` |
| 1226 | `struct` | `zt_shared_ops` |
| 1230 | `struct` | `zt_shared_handle` |
| 1244 | `func_def` | `zt_shared_text_snapshot_value` |
| 1251 | `func_def` | `zt_shared_bytes_snapshot_value` |
| 1266 | `func_def` | `zt_shared_handle_init` |
| 1277 | `func_def` | `zt_shared_handle_retain` |
| 1300 | `func_def` | `zt_shared_handle_release` |
| 1323 | `func_def` | `zt_shared_handle_borrow` |
| 1331 | `func_def` | `zt_shared_handle_snapshot` |
| 1336 | `func_def` | `zt_shared_handle_ref_count` |
| 1344 | `func_def` | `zt_shared_text_new` |
| 1357 | `func_def` | `zt_shared_text_retain` |
| 1366 | `func_def` | `zt_shared_text_release` |
| 1377 | `func_def` | `zt_shared_text_borrow` |
| 1383 | `func_def` | `zt_shared_text_snapshot` |
| 1389 | `func_def` | `zt_shared_text_ref_count` |
| 1395 | `func_def` | `zt_shared_bytes_new` |
| 1408 | `func_def` | `zt_shared_bytes_retain` |
| 1417 | `func_def` | `zt_shared_bytes_release` |
| 1428 | `func_def` | `zt_shared_bytes_borrow` |
| 1434 | `func_def` | `zt_shared_bytes_snapshot` |
| 1440 | `func_def` | `zt_shared_bytes_ref_count` |
| 1446 | `func_def` | `zt_runtime_report_error` |
| 1450 | `func_def` | `zt_runtime_exit_code_for_kind` |
| 1461 | `func_def` | `zt_runtime_error_ex` |
| 1467 | `func_def` | `zt_runtime_error_with_span` |
| 1471 | `func_def` | `zt_runtime_error` |
| 1475 | `func_def` | `zt_assert` |
| 1481 | `func_def` | `zt_check` |
| 1487 | `func_def` | `zt_panic` |
| 1491 | `func_def` | `zt_test_fail` |
| 1497 | `func_def` | `zt_test_skip` |
| 1503 | `func_def` | `zt_contract_failed` |
| 1507 | `func_def` | `zt_contract_failed_i64` |
| 1519 | `func_def` | `zt_contract_failed_float` |
| 1531 | `func_def` | `zt_contract_failed_bool` |
| 1541 | `func_def` | `zt_text_from_utf8` |
| 1566 | `func_def` | `zt_text_from_utf8_literal` |
| 1574 | `func_def` | `zt_text_concat` |
| 1595 | `func_def` | `zt_text_index` |
| 1605 | `func_def` | `zt_text_slice` |
| 1629 | `func_def` | `zt_text_eq` |
| 1648 | `func_def` | `zt_text_len` |
| 1653 | `func_def` | `zt_text_data` |
| 1658 | `func_def` | `zt_text_deep_copy` |
| 1663 | `func_def` | `zt_bytes_empty` |
| 1667 | `func_def` | `zt_bytes_from_array` |
| 1694 | `func_def` | `zt_bytes_from_list_i64` |
| 1717 | `func_def` | `zt_bytes_to_list_i64` |
| 1737 | `func_def` | `zt_bytes_join` |
| 1754 | `func_def` | `zt_bytes_starts_with` |
| 1769 | `func_def` | `zt_bytes_ends_with` |
| 1784 | `func_def` | `zt_bytes_contains` |
| 1806 | `func_def` | `zt_text_to_utf8_bytes` |
| 1811 | `func_def` | `zt_text_from_utf8_bytes` |
| 1837 | `func_def` | `zt_bytes_len` |
| 1842 | `func_def` | `zt_bytes_get` |
| 1852 | `func_def` | `zt_bytes_slice` |
| 1876 | `func_def` | `zt_list_i64_new` |
| 1889 | `func_def` | `zt_list_i64_from_array` |
| 1903 | `func_def` | `zt_list_i64_push_owned` |
| 1915 | `func_def` | `zt_list_i64_push` |
| 1922 | `func_def` | `zt_list_i64_get` |
| 1932 | `func_def` | `zt_list_i64_get_optional` |
| 1941 | `func_def` | `zt_list_i64_set` |
| 1951 | `func_def` | `zt_list_i64_set_owned` |
| 1965 | `func_def` | `zt_list_i64_len` |
| 1970 | `func_def` | `zt_list_i64_slice` |
| 1994 | `func_def` | `zt_list_text_new` |
| 2007 | `func_def` | `zt_list_text_from_array` |
| 2030 | `func_def` | `zt_list_text_push_owned` |
| 2042 | `func_def` | `zt_list_text_push` |
| 2051 | `func_def` | `zt_list_text_get` |
| 2066 | `func_def` | `zt_list_text_get_optional` |
| 2080 | `func_def` | `zt_list_text_set` |
| 2093 | `func_def` | `zt_list_text_set_owned` |
| 2107 | `func_def` | `zt_list_text_len` |
| 2112 | `func_def` | `zt_list_text_slice` |
| 2136 | `func_def` | `zt_list_text_deep_copy` |
| 2157 | `func_def` | `zt_dyn_text_repr_alloc` |
| 2168 | `func_def` | `zt_dyn_text_repr_from_i64` |
| 2174 | `func_def` | `zt_dyn_text_repr_from_float` |
| 2180 | `func_def` | `zt_dyn_text_repr_from_bool` |
| 2186 | `func_def` | `zt_dyn_text_repr_from_text_owned` |
| 2194 | `func_def` | `zt_dyn_text_repr_from_text` |
| 2199 | `func_def` | `zt_dyn_text_repr_clone` |
| 2218 | `func_def` | `zt_dyn_text_repr_to_text` |
| 2241 | `func_def` | `zt_dyn_text_repr_text_len` |
| 2252 | `func_def` | `zt_list_dyn_text_repr_new` |
| 2263 | `func_def` | `zt_list_dyn_text_repr_from_array` |
| 2286 | `func_def` | `zt_list_dyn_text_repr_from_array_owned` |
| 2308 | `func_def` | `zt_list_dyn_text_repr_get` |
| 2322 | `func_def` | `zt_list_dyn_text_repr_len` |
| 2327 | `func_def` | `zt_list_dyn_text_repr_deep_copy` |
| 2346 | `func_def` | `zt_thread_boundary_copy_text` |
| 2351 | `func_def` | `zt_thread_boundary_copy_bytes` |
| 2356 | `func_def` | `zt_thread_boundary_copy_list_text` |
| 2361 | `func_def` | `zt_thread_boundary_copy_dyn_text_repr` |
| 2366 | `func_def` | `zt_thread_boundary_copy_list_dyn_text_repr` |
| 2370 | `func_def` | `zt_map_text_text_new` |
| 2374 | `func_def` | `zt_map_text_text_set` |
| 2398 | `func_def` | `zt_map_text_text_set_owned` |
| 2412 | `func_def` | `zt_map_text_text_from_arrays` |
| 2432 | `func_def` | `zt_map_text_text_get` |
| 2451 | `func_def` | `zt_map_text_text_get_optional` |
| 2464 | `func_def` | `zt_map_text_text_key_at` |
| 2478 | `func_def` | `zt_map_text_text_value_at` |
| 2492 | `func_def` | `zt_map_text_text_len` |
| 2497 | `func_def` | `zt_optional_i64_present` |
| 2505 | `func_def` | `zt_optional_i64_empty` |
| 2513 | `func_def` | `zt_optional_i64_is_present` |
| 2517 | `func_def` | `zt_optional_i64_coalesce` |
| 2521 | `func_def` | `zt_optional_text_present` |
| 2533 | `func_def` | `zt_optional_text_empty` |
| 2542 | `func_def` | `zt_optional_text_is_present` |
| 2546 | `func_def` | `zt_optional_text_coalesce` |
| 2556 | `func_def` | `zt_optional_bytes_present` |
| 2567 | `func_def` | `zt_optional_bytes_empty` |
| 2575 | `func_def` | `zt_optional_bytes_is_present` |
| 2579 | `func_def` | `zt_optional_bytes_coalesce` |
| 2589 | `func_def` | `zt_optional_list_i64_present` |
| 2601 | `func_def` | `zt_optional_list_i64_empty` |
| 2610 | `func_def` | `zt_optional_list_i64_is_present` |
| 2614 | `func_def` | `zt_optional_list_i64_coalesce` |
| 2624 | `func_def` | `zt_optional_list_text_present` |
| 2636 | `func_def` | `zt_optional_list_text_empty` |
| 2645 | `func_def` | `zt_optional_list_text_is_present` |
| 2649 | `func_def` | `zt_optional_list_text_coalesce` |
| 2659 | `func_def` | `zt_optional_map_text_text_present` |
| 2671 | `func_def` | `zt_optional_map_text_text_empty` |
| 2680 | `func_def` | `zt_optional_map_text_text_is_present` |
| 2684 | `func_def` | `zt_optional_map_text_text_coalesce` |
| 2694 | `func_def` | `zt_core_error_make` |
| 2712 | `func_def` | `zt_core_error_from_message` |
| 2721 | `func_def` | `zt_core_error_from_text` |
| 2732 | `func_def` | `zt_core_error_clone` |
| 2743 | `func_def` | `zt_core_error_dispose` |
| 2753 | `func_def` | `zt_core_error_message_or_default` |
| 2761 | `func_def` | `zt_outcome_i64_text_success` |
| 2770 | `func_def` | `zt_outcome_i64_text_failure` |
| 2783 | `func_def` | `zt_outcome_i64_text_is_success` |
| 2787 | `func_def` | `zt_outcome_i64_text_value` |
| 2796 | `func_def` | `zt_outcome_i64_text_propagate` |
| 2800 | `func_def` | `zt_outcome_text_text_success` |
| 2813 | `func_def` | `zt_outcome_text_text_failure` |
| 2826 | `func_def` | `zt_outcome_text_text_is_success` |
| 2830 | `func_def` | `zt_outcome_text_text_value` |
| 2841 | `func_def` | `zt_outcome_text_text_propagate` |
| 2845 | `func_def` | `zt_outcome_bytes_text_success` |
| 2857 | `func_def` | `zt_outcome_bytes_text_failure` |
| 2869 | `func_def` | `zt_outcome_bytes_text_is_success` |
| 2873 | `func_def` | `zt_outcome_bytes_text_value` |
| 2883 | `func_def` | `zt_outcome_bytes_text_propagate` |
| 2887 | `func_def` | `zt_outcome_text_text_eq` |
| 2906 | `func_def` | `zt_outcome_optional_text_text_success` |
| 2919 | `func_def` | `zt_outcome_optional_text_text_failure` |
| 2931 | `func_def` | `zt_outcome_optional_text_text_is_success` |
| 2935 | `func_def` | `zt_outcome_optional_text_text_value` |
| 2950 | `func_def` | `zt_outcome_optional_text_text_propagate` |
| 2954 | `func_def` | `zt_outcome_optional_bytes_text_success` |
| 2967 | `func_def` | `zt_outcome_optional_bytes_text_failure` |
| 2979 | `func_def` | `zt_outcome_optional_bytes_text_is_success` |
| 2983 | `func_def` | `zt_outcome_optional_bytes_text_value` |
| 2998 | `func_def` | `zt_outcome_optional_bytes_text_propagate` |
| 3002 | `func_def` | `zt_outcome_net_connection_text_success` |
| 3014 | `func_def` | `zt_outcome_net_connection_text_failure` |
| 3026 | `func_def` | `zt_outcome_net_connection_text_is_success` |
| 3030 | `func_def` | `zt_outcome_net_connection_text_value` |
| 3040 | `func_def` | `zt_outcome_net_connection_text_propagate` |
| 3043 | `func_def` | `zt_outcome_list_i64_text_success` |
| 3056 | `func_def` | `zt_outcome_list_i64_text_failure` |
| 3069 | `func_def` | `zt_outcome_list_i64_text_is_success` |
| 3073 | `func_def` | `zt_outcome_list_i64_text_value` |
| 3084 | `func_def` | `zt_outcome_list_i64_text_propagate` |
| 3088 | `func_def` | `zt_outcome_list_text_text_success` |
| 3101 | `func_def` | `zt_outcome_list_text_text_failure` |
| 3114 | `func_def` | `zt_outcome_list_text_text_is_success` |
| 3118 | `func_def` | `zt_outcome_list_text_text_value` |
| 3129 | `func_def` | `zt_outcome_list_text_text_propagate` |
| 3133 | `func_def` | `zt_outcome_map_text_text_success` |
| 3146 | `func_def` | `zt_outcome_map_text_text_failure` |
| 3159 | `func_def` | `zt_outcome_map_text_text_failure_message` |
| 3166 | `func_def` | `zt_outcome_map_text_text_is_success` |
| 3170 | `func_def` | `zt_outcome_map_text_text_value` |
| 3181 | `func_def` | `zt_outcome_map_text_text_propagate` |
| 3185 | `func_def` | `zt_outcome_void_text_success` |
| 3192 | `func_def` | `zt_outcome_void_text_failure` |
| 3204 | `func_def` | `zt_outcome_void_text_is_success` |
| 3208 | `func_def` | `zt_outcome_void_text_propagate` |
| 3213 | `func_def` | `zt_outcome_i64_core_error_success` |
| 3221 | `func_def` | `zt_outcome_i64_core_error_failure` |
| 3230 | `func_def` | `zt_outcome_i64_core_error_failure_message` |
| 3237 | `func_def` | `zt_outcome_i64_core_error_failure_text` |
| 3244 | `func_def` | `zt_outcome_i64_core_error_is_success` |
| 3248 | `func_def` | `zt_outcome_i64_core_error_value` |
| 3253 | `func_def` | `zt_outcome_i64_core_error_propagate` |
| 3258 | `func_def` | `zt_outcome_i64_core_error_dispose` |
| 3264 | `func_def` | `zt_outcome_void_core_error_success` |
| 3271 | `func_def` | `zt_outcome_void_core_error_failure` |
| 3279 | `func_def` | `zt_outcome_void_core_error_failure_message` |
| 3286 | `func_def` | `zt_outcome_void_core_error_failure_text` |
| 3293 | `func_def` | `zt_outcome_void_core_error_is_success` |
| 3297 | `func_def` | `zt_outcome_void_core_error_propagate` |
| 3302 | `func_def` | `zt_outcome_void_core_error_dispose` |
| 3308 | `func_def` | `zt_outcome_text_core_error_success` |
| 3318 | `func_def` | `zt_outcome_text_core_error_failure` |
| 3326 | `func_def` | `zt_outcome_text_core_error_failure_message` |
| 3333 | `func_def` | `zt_outcome_text_core_error_failure_text` |
| 3340 | `func_def` | `zt_outcome_text_core_error_is_success` |
| 3344 | `func_def` | `zt_outcome_text_core_error_value` |
| 3351 | `func_def` | `zt_outcome_text_core_error_propagate` |
| 3356 | `func_def` | `zt_outcome_text_core_error_dispose` |
| 3366 | `func_def` | `zt_outcome_optional_text_core_error_success` |
| 3378 | `func_def` | `zt_outcome_optional_text_core_error_failure` |
| 3387 | `func_def` | `zt_outcome_optional_text_core_error_failure_message` |
| 3394 | `func_def` | `zt_outcome_optional_text_core_error_failure_text` |
| 3401 | `func_def` | `zt_outcome_optional_text_core_error_is_success` |
| 3405 | `func_def` | `zt_outcome_optional_text_core_error_value` |
| 3416 | `func_def` | `zt_outcome_optional_text_core_error_propagate` |
| 3421 | `func_def` | `zt_outcome_optional_text_core_error_dispose` |
| 3431 | `func_def` | `zt_outcome_optional_bytes_core_error_success` |
| 3443 | `func_def` | `zt_outcome_optional_bytes_core_error_failure` |
| 3452 | `func_def` | `zt_outcome_optional_bytes_core_error_failure_message` |
| 3459 | `func_def` | `zt_outcome_optional_bytes_core_error_failure_text` |
| 3466 | `func_def` | `zt_outcome_optional_bytes_core_error_is_success` |
| 3470 | `func_def` | `zt_outcome_optional_bytes_core_error_value` |
| 3481 | `func_def` | `zt_outcome_optional_bytes_core_error_propagate` |
| 3486 | `func_def` | `zt_outcome_optional_bytes_core_error_dispose` |
| 3496 | `func_def` | `zt_outcome_net_connection_core_error_success` |
| 3506 | `func_def` | `zt_outcome_net_connection_core_error_failure` |
| 3515 | `func_def` | `zt_outcome_net_connection_core_error_failure_message` |
| 3522 | `func_def` | `zt_outcome_net_connection_core_error_failure_text` |
| 3529 | `func_def` | `zt_outcome_net_connection_core_error_is_success` |
| 3533 | `func_def` | `zt_outcome_net_connection_core_error_value` |
| 3540 | `func_def` | `zt_outcome_net_connection_core_error_propagate` |
| 3545 | `func_def` | `zt_outcome_net_connection_core_error_dispose` |
| 3554 | `func_def` | `zt_outcome_list_i64_core_error_success` |
| 3564 | `func_def` | `zt_outcome_list_i64_core_error_failure` |
| 3572 | `func_def` | `zt_outcome_list_i64_core_error_failure_message` |
| 3579 | `func_def` | `zt_outcome_list_i64_core_error_failure_text` |
| 3586 | `func_def` | `zt_outcome_list_i64_core_error_is_success` |
| 3590 | `func_def` | `zt_outcome_list_i64_core_error_value` |
| 3597 | `func_def` | `zt_outcome_list_i64_core_error_propagate` |
| 3602 | `func_def` | `zt_outcome_list_i64_core_error_dispose` |
| 3612 | `func_def` | `zt_outcome_map_text_text_core_error_success` |
| 3622 | `func_def` | `zt_outcome_map_text_text_core_error_failure` |
| 3630 | `func_def` | `zt_outcome_map_text_text_core_error_failure_message` |
| 3637 | `func_def` | `zt_outcome_map_text_text_core_error_failure_text` |
| 3644 | `func_def` | `zt_outcome_map_text_text_core_error_is_success` |
| 3648 | `func_def` | `zt_outcome_map_text_text_core_error_value` |
| 3655 | `func_def` | `zt_outcome_map_text_text_core_error_propagate` |
| 3660 | `func_def` | `zt_outcome_map_text_text_core_error_dispose` |
| 3672 | `func_def` | `zt_detect_cycles` |
| 3698 | `func_def` | `zt_runtime_check_for_cycles` |
| 3707 | `macro` | `POOL_SIZE` |
| 3709 | `struct` | `zt_pool` |
| 3716 | `func_def` | `zt_text_pool_alloc` |
| 3723 | `func_def` | `zt_text_pool_free` |
| 3731 | `func_def` | `zt_validate_pointer` |
| 3735 | `func_def` | `zt_runtime_safe_function_example` |
| 3745 | `func_def` | `zt_validate_and_free_text` |
| 3753 | `func_def` | `zt_validate_and_free_list_i64` |
| 3761 | `func_def` | `zt_validate_and_free_map_text_text` |
| 3769 | `func_def` | `zt_host_default_read_file` |
| 3830 | `func_def` | `zt_host_default_write_file` |
| 3859 | `func_def` | `zt_host_default_path_exists` |
| 3869 | `func_def` | `zt_host_default_read_line_stdin` |
| 3932 | `func_def` | `zt_host_default_read_all_stdin` |
| 3970 | `func_def` | `zt_host_default_write_stream` |
| 3989 | `func_def` | `zt_host_default_write_stdout` |
| 3993 | `func_def` | `zt_host_default_write_stderr` |
| 3997 | `func_def` | `zt_host_default_time_now_unix_ms` |
| 4009 | `func_def` | `zt_host_default_time_sleep_ms` |
| 4037 | `func_def` | `zt_host_random_next_u64` |
| 4046 | `func_def` | `zt_host_default_random_seed` |
| 4055 | `func_def` | `zt_host_default_random_next_i64` |
| 4060 | `func_def` | `zt_host_strdup_text` |
| 4074 | `func_def` | `zt_host_default_os_current_dir` |
| 4118 | `func_def` | `zt_host_default_os_change_dir` |
| 4133 | `func_def` | `zt_host_default_os_env` |
| 4151 | `func_def` | `zt_host_default_os_pid` |
| 4159 | `func_def` | `zt_host_default_os_platform` |
| 4171 | `func_def` | `zt_host_default_os_arch` |
| 4187 | `func_def` | `zt_host_command_append` |
| 4216 | `func_def` | `zt_host_command_append_quoted` |
| 4238 | `func_def` | `zt_host_default_process_run` |
| 4358 | `func_def` | `zt_host_set_api` |
| 4401 | `func_def` | `zt_host_get_api` |
| 4405 | `func_def` | `zt_host_read_file` |
| 4410 | `func_def` | `zt_host_write_file` |
| 4414 | `func_def` | `zt_host_path_exists` |
| 4417 | `func_def` | `zt_host_read_line_stdin` |
| 4421 | `func_def` | `zt_host_read_all_stdin` |
| 4425 | `func_def` | `zt_host_write_stdout` |
| 4429 | `func_def` | `zt_host_write_stderr` |
| 4433 | `func_def` | `zt_host_time_now_unix_ms` |
| 4437 | `func_def` | `zt_host_time_sleep_ms` |
| 4441 | `func_def` | `zt_host_random_seed` |
| 4445 | `func_def` | `zt_host_random_next_i64` |
| 4449 | `func_def` | `zt_host_os_current_dir` |
| 4453 | `func_def` | `zt_host_os_change_dir` |
| 4457 | `func_def` | `zt_host_os_env` |
| 4461 | `func_def` | `zt_host_os_pid` |
| 4465 | `func_def` | `zt_host_os_platform` |
| 4469 | `func_def` | `zt_host_os_arch` |
| 4473 | `func_def` | `zt_host_process_run` |
| 4477 | `func_def` | `zt_json_skip_whitespace` |
| 4484 | `func_def` | `zt_json_buffer_reserve` |
| 4512 | `func_def` | `zt_json_buffer_append_char` |
| 4519 | `func_def` | `zt_json_buffer_append_bytes` |
| 4530 | `func_def` | `zt_json_buffer_append_escaped_text` |
| 4575 | `func_def` | `zt_json_parse_string` |
| 4659 | `func_def` | `zt_json_parse_unquoted_value` |
| 4701 | `func_def` | `zt_json_parse_map_text_text` |
| 4801 | `func_decl` | `zt_outcome_map_text_text_core_error_failure_message` |
| 4804 | `func_def` | `zt_json_stringify_map_text_text` |
| 4850 | `func_def` | `zt_json_pretty_map_text_text` |
| 4912 | `func_def` | `zt_u64_magnitude` |
| 4919 | `func_def` | `zt_format_hex_i64` |
| 4932 | `func_def` | `zt_format_bin_i64` |
| 4965 | `func_def` | `zt_format_bytes_impl` |
| 4989 | `func_def` | `zt_format_bytes_binary` |
| 4994 | `func_def` | `zt_format_bytes_decimal` |
| 4999 | `func_def` | `zt_math_pow` |
| 5003 | `func_def` | `zt_math_sqrt` |
| 5007 | `func_def` | `zt_math_floor` |
| 5011 | `func_def` | `zt_math_ceil` |
| 5015 | `func_def` | `zt_math_round_half_away_from_zero` |
| 5019 | `func_def` | `zt_math_trunc` |
| 5023 | `func_def` | `zt_math_sin` |
| 5027 | `func_def` | `zt_math_cos` |
| 5031 | `func_def` | `zt_math_tan` |
| 5035 | `func_def` | `zt_math_asin` |
| 5039 | `func_def` | `zt_math_acos` |
| 5043 | `func_def` | `zt_math_atan` |
| 5047 | `func_def` | `zt_math_atan2` |
| 5051 | `func_def` | `zt_math_ln` |
| 5055 | `func_def` | `zt_math_log10` |
| 5059 | `func_def` | `zt_math_log_ten` |
| 5063 | `func_def` | `zt_math_log2` |
| 5067 | `func_def` | `zt_math_log` |
| 5074 | `func_def` | `zt_math_exp` |
| 5078 | `func_def` | `zt_math_is_nan` |
| 5082 | `func_def` | `zt_math_is_infinite` |
| 5086 | `func_def` | `zt_math_is_finite` |
| 5089 | `func_def` | `zt_net_startup` |
| 5113 | `func_def` | `zt_net_last_error_code` |
| 5121 | `func_def` | `zt_net_would_block_code` |
| 5129 | `func_def` | `zt_net_format_error` |
| 5141 | `func_def` | `zt_net_set_nonblocking` |
| 5158 | `func_def` | `zt_net_wait_socket` |
| 5202 | `func_def` | `zt_net_socket_error` |
| 5223 | `func_def` | `zt_net_connection_new` |
| 5240 | `func_def` | `zt_net_core_error_from_prefixed_message` |
| 5264 | `func_def` | `zt_net_connection_core_error_failure_prefixed` |
| 5271 | `func_def` | `zt_net_optional_bytes_core_error_failure_prefixed` |
| 5278 | `func_def` | `zt_net_void_core_error_failure_prefixed` |
| 5285 | `func_def` | `zt_net_connect` |
| 5389 | `func_def` | `zt_net_effective_timeout_ms` |
| 5393 | `func_def` | `zt_net_read_some` |
| 5458 | `func_def` | `zt_net_write_all` |
| 5508 | `func_def` | `zt_net_close` |
| 5521 | `func_def` | `zt_net_is_closed` |
| 5526 | `func_def` | `zt_net_error_kind_index` |
| 5542 | `func_def` | `zt_path_is_separator_char` |
| 5546 | `func_def` | `zt_path_is_drive_letter` |
| 5550 | `func_def` | `zt_path_parse_prefix` |
| 5595 | `func_def` | `zt_path_segment_is_dot` |
| 5599 | `func_def` | `zt_path_segment_is_dot_dot` |
| 5603 | `func_def` | `zt_path_collect_segments` |
| 5652 | `func_def` | `zt_path_normalize` |
| 5779 | `func_def` | `zt_path_is_absolute` |
| 5789 | `func_def` | `zt_path_absolute` |
| 5834 | `func_def` | `zt_path_relative` |
| 5961 | `func_def` | `zt_add_i64` |
| 5969 | `func_def` | `zt_sub_i64` |
| 5977 | `func_def` | `zt_mul_i64` |
| 5985 | `func_def` | `zt_div_i64` |
| 5995 | `func_def` | `zt_rem_i64` |
| 6006 | `func_def` | `zt_validate_between_i64` |
| 6010 | `func_def` | `zt_runtime_require_grid2d_i64` |
| 6016 | `func_def` | `zt_runtime_require_grid2d_text` |
| 6022 | `func_def` | `zt_runtime_require_pqueue_i64` |
| 6028 | `func_def` | `zt_runtime_require_pqueue_text` |
| 6034 | `func_def` | `zt_runtime_require_circbuf_i64` |
| 6040 | `func_def` | `zt_runtime_require_circbuf_text` |
| 6046 | `func_def` | `zt_free_grid2d_i64` |
| 6057 | `func_def` | `zt_free_grid2d_text` |
| 6072 | `func_def` | `zt_free_pqueue_i64` |
| 6081 | `func_def` | `zt_free_pqueue_text` |
| 6094 | `func_def` | `zt_free_circbuf_i64` |
| 6104 | `func_def` | `zt_free_circbuf_text` |
| 6118 | `func_def` | `zt_queue_i64_new` |
| 6122 | `func_def` | `zt_queue_i64_enqueue` |
| 6127 | `func_def` | `zt_queue_i64_enqueue_owned` |
| 6131 | `func_def` | `zt_queue_i64_dequeue` |
| 6148 | `func_def` | `zt_queue_i64_peek` |
| 6153 | `func_def` | `zt_queue_text_new` |
| 6157 | `func_def` | `zt_queue_text_enqueue` |
| 6162 | `func_def` | `zt_queue_text_enqueue_owned` |
| 6166 | `func_def` | `zt_queue_text_dequeue` |
| 6186 | `func_def` | `zt_queue_text_peek` |
| 6191 | `func_def` | `zt_stack_i64_new` |
| 6195 | `func_def` | `zt_stack_i64_push` |
| 6200 | `func_def` | `zt_stack_i64_push_owned` |
| 6204 | `func_def` | `zt_stack_i64_pop` |
| 6215 | `func_def` | `zt_stack_i64_peek` |
| 6223 | `func_def` | `zt_stack_text_new` |
| 6227 | `func_def` | `zt_stack_text_push` |
| 6232 | `func_def` | `zt_stack_text_push_owned` |
| 6236 | `func_def` | `zt_stack_text_pop` |
| 6248 | `func_def` | `zt_stack_text_peek` |
| 6256 | `func_def` | `zt_grid2d_i64_new` |
| 6285 | `func_def` | `zt_grid2d_i64_get` |
| 6293 | `func_def` | `zt_grid2d_i64_set` |
| 6302 | `func_def` | `zt_grid2d_i64_set_owned` |
| 6317 | `func_def` | `zt_grid2d_i64_fill` |
| 6326 | `func_def` | `zt_grid2d_i64_fill_owned` |
| 6341 | `func_def` | `zt_grid2d_i64_rows` |
| 6346 | `func_def` | `zt_grid2d_i64_cols` |
| 6351 | `func_def` | `zt_grid2d_text_new` |
| 6384 | `func_def` | `zt_grid2d_text_get` |
| 6396 | `func_def` | `zt_grid2d_text_set` |
| 6409 | `func_def` | `zt_grid2d_text_set_owned` |
| 6425 | `func_def` | `zt_grid2d_text_fill` |
| 6437 | `func_def` | `zt_grid2d_text_fill_owned` |
| 6453 | `func_def` | `zt_grid2d_text_rows` |
| 6458 | `func_def` | `zt_grid2d_text_cols` |
| 6463 | `func_def` | `zt_pqueue_i64_new` |
| 6474 | `func_def` | `zt_pqueue_i64_ensure_capacity` |
| 6486 | `func_def` | `zt_pqueue_i64_sift_up` |
| 6497 | `func_def` | `zt_pqueue_i64_sift_down` |
| 6516 | `func_def` | `zt_pqueue_i64_push` |
| 6525 | `func_def` | `zt_pqueue_i64_push_owned` |
| 6542 | `func_def` | `zt_pqueue_i64_pop` |
| 6557 | `func_def` | `zt_pqueue_i64_peek` |
| 6565 | `func_def` | `zt_pqueue_i64_len` |
| 6570 | `func_def` | `zt_pqueue_text_new` |
| 6581 | `func_def` | `zt_pqueue_text_ensure_capacity` |
| 6593 | `func_def` | `zt_pqueue_text_sift_up` |
| 6606 | `func_def` | `zt_pqueue_text_sift_down` |
| 6633 | `func_def` | `zt_pqueue_text_push` |
| 6643 | `func_def` | `zt_pqueue_text_push_owned` |
| 6661 | `func_def` | `zt_pqueue_text_pop` |
| 6679 | `func_def` | `zt_pqueue_text_peek` |
| 6687 | `func_def` | `zt_pqueue_text_len` |
| 6692 | `func_def` | `zt_circbuf_i64_new` |
| 6714 | `func_def` | `zt_circbuf_i64_push` |
| 6726 | `func_def` | `zt_circbuf_i64_push_owned` |
| 6742 | `func_def` | `zt_circbuf_i64_pop` |
| 6754 | `func_def` | `zt_circbuf_i64_peek` |
| 6762 | `func_def` | `zt_circbuf_i64_len` |
| 6767 | `func_def` | `zt_circbuf_i64_capacity` |
| 6772 | `func_def` | `zt_circbuf_i64_is_full` |
| 6777 | `func_def` | `zt_circbuf_text_new` |
| 6799 | `func_def` | `zt_circbuf_text_push` |
| 6817 | `func_def` | `zt_circbuf_text_push_owned` |
| 6834 | `func_def` | `zt_circbuf_text_pop` |
| 6850 | `func_def` | `zt_circbuf_text_peek` |
| 6858 | `func_def` | `zt_circbuf_text_len` |
| 6863 | `func_def` | `zt_circbuf_text_capacity` |
| 6868 | `func_def` | `zt_circbuf_text_is_full` |
| 6873 | `func_def` | `zt_runtime_require_btreemap_text_text` |
| 6879 | `func_def` | `zt_runtime_require_btreeset_text` |
| 6885 | `func_def` | `zt_runtime_require_grid3d_i64` |
| 6891 | `func_def` | `zt_runtime_require_grid3d_text` |
| 6897 | `func_def` | `zt_free_btreemap_text_text` |
| 6913 | `func_def` | `zt_free_btreeset_text` |
| 6926 | `func_def` | `zt_free_grid3d_i64` |
| 6938 | `func_def` | `zt_free_grid3d_text` |
| 6954 | `func_def` | `zt_btreemap_text_text_find` |
| 6974 | `func_def` | `zt_btreemap_text_text_ensure_capacity` |
| 6988 | `func_def` | `zt_btreemap_text_text_new` |
| 6999 | `func_def` | `zt_btreemap_text_text_set` |
| 7028 | `func_def` | `zt_btreemap_text_text_set_owned` |
| 7048 | `func_def` | `zt_btreemap_text_text_get` |
| 7060 | `func_def` | `zt_btreemap_text_text_get_optional` |
| 7069 | `func_def` | `zt_btreemap_text_text_contains` |
| 7077 | `func_def` | `zt_btreemap_text_text_remove` |
| 7097 | `func_def` | `zt_btreemap_text_text_remove_owned` |
| 7117 | `func_def` | `zt_btreemap_text_text_len` |
| 7122 | `func_def` | `zt_btreeset_text_find` |
| 7142 | `func_def` | `zt_btreeset_text_ensure_capacity` |
| 7154 | `func_def` | `zt_btreeset_text_new` |
| 7165 | `func_def` | `zt_btreeset_text_insert` |
| 7184 | `func_def` | `zt_btreeset_text_insert_owned` |
| 7202 | `func_def` | `zt_btreeset_text_contains` |
| 7208 | `func_def` | `zt_btreeset_text_remove` |
| 7224 | `func_def` | `zt_btreeset_text_remove_owned` |
| 7242 | `func_def` | `zt_btreeset_text_len` |
| 7247 | `func_def` | `zt_grid3d_i64_new` |
| 7273 | `func_def` | `zt_grid3d_i64_get` |
| 7281 | `func_def` | `zt_grid3d_i64_set` |
| 7290 | `func_def` | `zt_grid3d_i64_set_owned` |
| 7305 | `func_def` | `zt_grid3d_i64_fill` |
| 7314 | `func_def` | `zt_grid3d_i64_fill_owned` |
| 7329 | `func_def` | `zt_grid3d_i64_depth` |
| 7334 | `func_def` | `zt_grid3d_i64_rows` |
| 7339 | `func_def` | `zt_grid3d_i64_cols` |
| 7344 | `func_def` | `zt_grid3d_text_new` |
| 7374 | `func_def` | `zt_grid3d_text_get` |
| 7386 | `func_def` | `zt_grid3d_text_set` |
| 7401 | `func_def` | `zt_grid3d_text_set_owned` |
| 7417 | `func_def` | `zt_grid3d_text_fill` |
| 7429 | `func_def` | `zt_grid3d_text_fill_owned` |
| 7445 | `func_def` | `zt_grid3d_text_depth` |
| 7450 | `func_def` | `zt_grid3d_text_rows` |
| 7455 | `func_def` | `zt_grid3d_text_cols` |

#### `runtime/c/zenith_rt.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_RUNTIME_C_ZENITH_RT_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
