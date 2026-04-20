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
- Extracted symbols: 591

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `runtime/c/zenith_rt.c` | 7302 | 590 | 1 |
| `runtime/c/zenith_rt.h` | 889 | 1 | 0 |

### Local Dependencies

- `runtime/c/zenith_rt.h`

### Related Tests

- `tests/behavior/where_contract_construct_error/src/app/main.zt`
- `tests/behavior/where_contract_field_assign_error/src/app/main.zt`
- `tests/behavior/where_contract_param_error/src/app/main.zt`
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
| 1061 | `struct` | `zt_shared_ops` |
| 1065 | `struct` | `zt_shared_handle` |
| 1079 | `func_def` | `zt_shared_text_snapshot_value` |
| 1086 | `func_def` | `zt_shared_bytes_snapshot_value` |
| 1101 | `func_def` | `zt_shared_handle_init` |
| 1112 | `func_def` | `zt_shared_handle_retain` |
| 1135 | `func_def` | `zt_shared_handle_release` |
| 1158 | `func_def` | `zt_shared_handle_borrow` |
| 1166 | `func_def` | `zt_shared_handle_snapshot` |
| 1171 | `func_def` | `zt_shared_handle_ref_count` |
| 1179 | `func_def` | `zt_shared_text_new` |
| 1192 | `func_def` | `zt_shared_text_retain` |
| 1201 | `func_def` | `zt_shared_text_release` |
| 1212 | `func_def` | `zt_shared_text_borrow` |
| 1218 | `func_def` | `zt_shared_text_snapshot` |
| 1224 | `func_def` | `zt_shared_text_ref_count` |
| 1230 | `func_def` | `zt_shared_bytes_new` |
| 1243 | `func_def` | `zt_shared_bytes_retain` |
| 1252 | `func_def` | `zt_shared_bytes_release` |
| 1263 | `func_def` | `zt_shared_bytes_borrow` |
| 1269 | `func_def` | `zt_shared_bytes_snapshot` |
| 1275 | `func_def` | `zt_shared_bytes_ref_count` |
| 1281 | `func_def` | `zt_runtime_report_error` |
| 1285 | `func_def` | `zt_runtime_exit_code_for_kind` |
| 1296 | `func_def` | `zt_runtime_error_ex` |
| 1302 | `func_def` | `zt_runtime_error_with_span` |
| 1306 | `func_def` | `zt_runtime_error` |
| 1310 | `func_def` | `zt_assert` |
| 1316 | `func_def` | `zt_check` |
| 1322 | `func_def` | `zt_panic` |
| 1326 | `func_def` | `zt_test_fail` |
| 1332 | `func_def` | `zt_test_skip` |
| 1338 | `func_def` | `zt_contract_failed` |
| 1342 | `func_def` | `zt_contract_failed_i64` |
| 1354 | `func_def` | `zt_contract_failed_float` |
| 1366 | `func_def` | `zt_contract_failed_bool` |
| 1376 | `func_def` | `zt_text_from_utf8` |
| 1401 | `func_def` | `zt_text_from_utf8_literal` |
| 1409 | `func_def` | `zt_text_concat` |
| 1430 | `func_def` | `zt_text_index` |
| 1440 | `func_def` | `zt_text_slice` |
| 1464 | `func_def` | `zt_text_eq` |
| 1483 | `func_def` | `zt_text_len` |
| 1488 | `func_def` | `zt_text_data` |
| 1493 | `func_def` | `zt_text_deep_copy` |
| 1498 | `func_def` | `zt_bytes_empty` |
| 1502 | `func_def` | `zt_bytes_from_array` |
| 1529 | `func_def` | `zt_bytes_from_list_i64` |
| 1552 | `func_def` | `zt_bytes_to_list_i64` |
| 1572 | `func_def` | `zt_bytes_join` |
| 1589 | `func_def` | `zt_bytes_starts_with` |
| 1604 | `func_def` | `zt_bytes_ends_with` |
| 1619 | `func_def` | `zt_bytes_contains` |
| 1641 | `func_def` | `zt_text_to_utf8_bytes` |
| 1646 | `func_def` | `zt_text_from_utf8_bytes` |
| 1672 | `func_def` | `zt_bytes_len` |
| 1677 | `func_def` | `zt_bytes_get` |
| 1687 | `func_def` | `zt_bytes_slice` |
| 1711 | `func_def` | `zt_list_i64_new` |
| 1724 | `func_def` | `zt_list_i64_from_array` |
| 1738 | `func_def` | `zt_list_i64_push_owned` |
| 1750 | `func_def` | `zt_list_i64_push` |
| 1757 | `func_def` | `zt_list_i64_get` |
| 1767 | `func_def` | `zt_list_i64_get_optional` |
| 1776 | `func_def` | `zt_list_i64_set` |
| 1786 | `func_def` | `zt_list_i64_set_owned` |
| 1800 | `func_def` | `zt_list_i64_len` |
| 1805 | `func_def` | `zt_list_i64_slice` |
| 1829 | `func_def` | `zt_list_text_new` |
| 1842 | `func_def` | `zt_list_text_from_array` |
| 1865 | `func_def` | `zt_list_text_push_owned` |
| 1877 | `func_def` | `zt_list_text_push` |
| 1886 | `func_def` | `zt_list_text_get` |
| 1901 | `func_def` | `zt_list_text_get_optional` |
| 1915 | `func_def` | `zt_list_text_set` |
| 1928 | `func_def` | `zt_list_text_set_owned` |
| 1942 | `func_def` | `zt_list_text_len` |
| 1947 | `func_def` | `zt_list_text_slice` |
| 1971 | `func_def` | `zt_list_text_deep_copy` |
| 1992 | `func_def` | `zt_dyn_text_repr_alloc` |
| 2003 | `func_def` | `zt_dyn_text_repr_from_i64` |
| 2009 | `func_def` | `zt_dyn_text_repr_from_float` |
| 2015 | `func_def` | `zt_dyn_text_repr_from_bool` |
| 2021 | `func_def` | `zt_dyn_text_repr_from_text_owned` |
| 2029 | `func_def` | `zt_dyn_text_repr_from_text` |
| 2034 | `func_def` | `zt_dyn_text_repr_clone` |
| 2053 | `func_def` | `zt_dyn_text_repr_to_text` |
| 2076 | `func_def` | `zt_dyn_text_repr_text_len` |
| 2087 | `func_def` | `zt_list_dyn_text_repr_new` |
| 2098 | `func_def` | `zt_list_dyn_text_repr_from_array` |
| 2121 | `func_def` | `zt_list_dyn_text_repr_from_array_owned` |
| 2143 | `func_def` | `zt_list_dyn_text_repr_get` |
| 2157 | `func_def` | `zt_list_dyn_text_repr_len` |
| 2162 | `func_def` | `zt_list_dyn_text_repr_deep_copy` |
| 2181 | `func_def` | `zt_thread_boundary_copy_text` |
| 2186 | `func_def` | `zt_thread_boundary_copy_bytes` |
| 2191 | `func_def` | `zt_thread_boundary_copy_list_text` |
| 2196 | `func_def` | `zt_thread_boundary_copy_dyn_text_repr` |
| 2201 | `func_def` | `zt_thread_boundary_copy_list_dyn_text_repr` |
| 2205 | `func_def` | `zt_map_text_text_new` |
| 2209 | `func_def` | `zt_map_text_text_set` |
| 2233 | `func_def` | `zt_map_text_text_set_owned` |
| 2247 | `func_def` | `zt_map_text_text_from_arrays` |
| 2267 | `func_def` | `zt_map_text_text_get` |
| 2286 | `func_def` | `zt_map_text_text_get_optional` |
| 2299 | `func_def` | `zt_map_text_text_key_at` |
| 2313 | `func_def` | `zt_map_text_text_value_at` |
| 2327 | `func_def` | `zt_map_text_text_len` |
| 2332 | `func_def` | `zt_optional_i64_present` |
| 2340 | `func_def` | `zt_optional_i64_empty` |
| 2348 | `func_def` | `zt_optional_i64_is_present` |
| 2352 | `func_def` | `zt_optional_i64_coalesce` |
| 2356 | `func_def` | `zt_optional_text_present` |
| 2368 | `func_def` | `zt_optional_text_empty` |
| 2377 | `func_def` | `zt_optional_text_is_present` |
| 2381 | `func_def` | `zt_optional_text_coalesce` |
| 2391 | `func_def` | `zt_optional_bytes_present` |
| 2402 | `func_def` | `zt_optional_bytes_empty` |
| 2410 | `func_def` | `zt_optional_bytes_is_present` |
| 2414 | `func_def` | `zt_optional_bytes_coalesce` |
| 2424 | `func_def` | `zt_optional_list_i64_present` |
| 2436 | `func_def` | `zt_optional_list_i64_empty` |
| 2445 | `func_def` | `zt_optional_list_i64_is_present` |
| 2449 | `func_def` | `zt_optional_list_i64_coalesce` |
| 2459 | `func_def` | `zt_optional_list_text_present` |
| 2471 | `func_def` | `zt_optional_list_text_empty` |
| 2480 | `func_def` | `zt_optional_list_text_is_present` |
| 2484 | `func_def` | `zt_optional_list_text_coalesce` |
| 2494 | `func_def` | `zt_optional_map_text_text_present` |
| 2506 | `func_def` | `zt_optional_map_text_text_empty` |
| 2515 | `func_def` | `zt_optional_map_text_text_is_present` |
| 2519 | `func_def` | `zt_optional_map_text_text_coalesce` |
| 2529 | `func_def` | `zt_core_error_make` |
| 2547 | `func_def` | `zt_core_error_from_message` |
| 2556 | `func_def` | `zt_core_error_from_text` |
| 2567 | `func_def` | `zt_core_error_clone` |
| 2578 | `func_def` | `zt_core_error_dispose` |
| 2588 | `func_def` | `zt_core_error_message_or_default` |
| 2596 | `func_def` | `zt_outcome_i64_text_success` |
| 2605 | `func_def` | `zt_outcome_i64_text_failure` |
| 2618 | `func_def` | `zt_outcome_i64_text_is_success` |
| 2622 | `func_def` | `zt_outcome_i64_text_value` |
| 2631 | `func_def` | `zt_outcome_i64_text_propagate` |
| 2635 | `func_def` | `zt_outcome_text_text_success` |
| 2648 | `func_def` | `zt_outcome_text_text_failure` |
| 2661 | `func_def` | `zt_outcome_text_text_is_success` |
| 2665 | `func_def` | `zt_outcome_text_text_value` |
| 2676 | `func_def` | `zt_outcome_text_text_propagate` |
| 2680 | `func_def` | `zt_outcome_bytes_text_success` |
| 2692 | `func_def` | `zt_outcome_bytes_text_failure` |
| 2704 | `func_def` | `zt_outcome_bytes_text_is_success` |
| 2708 | `func_def` | `zt_outcome_bytes_text_value` |
| 2718 | `func_def` | `zt_outcome_bytes_text_propagate` |
| 2722 | `func_def` | `zt_outcome_text_text_eq` |
| 2741 | `func_def` | `zt_outcome_optional_text_text_success` |
| 2754 | `func_def` | `zt_outcome_optional_text_text_failure` |
| 2766 | `func_def` | `zt_outcome_optional_text_text_is_success` |
| 2770 | `func_def` | `zt_outcome_optional_text_text_value` |
| 2785 | `func_def` | `zt_outcome_optional_text_text_propagate` |
| 2789 | `func_def` | `zt_outcome_optional_bytes_text_success` |
| 2802 | `func_def` | `zt_outcome_optional_bytes_text_failure` |
| 2814 | `func_def` | `zt_outcome_optional_bytes_text_is_success` |
| 2818 | `func_def` | `zt_outcome_optional_bytes_text_value` |
| 2833 | `func_def` | `zt_outcome_optional_bytes_text_propagate` |
| 2837 | `func_def` | `zt_outcome_net_connection_text_success` |
| 2849 | `func_def` | `zt_outcome_net_connection_text_failure` |
| 2861 | `func_def` | `zt_outcome_net_connection_text_is_success` |
| 2865 | `func_def` | `zt_outcome_net_connection_text_value` |
| 2875 | `func_def` | `zt_outcome_net_connection_text_propagate` |
| 2878 | `func_def` | `zt_outcome_list_i64_text_success` |
| 2891 | `func_def` | `zt_outcome_list_i64_text_failure` |
| 2904 | `func_def` | `zt_outcome_list_i64_text_is_success` |
| 2908 | `func_def` | `zt_outcome_list_i64_text_value` |
| 2919 | `func_def` | `zt_outcome_list_i64_text_propagate` |
| 2923 | `func_def` | `zt_outcome_list_text_text_success` |
| 2936 | `func_def` | `zt_outcome_list_text_text_failure` |
| 2949 | `func_def` | `zt_outcome_list_text_text_is_success` |
| 2953 | `func_def` | `zt_outcome_list_text_text_value` |
| 2964 | `func_def` | `zt_outcome_list_text_text_propagate` |
| 2968 | `func_def` | `zt_outcome_map_text_text_success` |
| 2981 | `func_def` | `zt_outcome_map_text_text_failure` |
| 2994 | `func_def` | `zt_outcome_map_text_text_failure_message` |
| 3001 | `func_def` | `zt_outcome_map_text_text_is_success` |
| 3005 | `func_def` | `zt_outcome_map_text_text_value` |
| 3016 | `func_def` | `zt_outcome_map_text_text_propagate` |
| 3020 | `func_def` | `zt_outcome_void_text_success` |
| 3027 | `func_def` | `zt_outcome_void_text_failure` |
| 3039 | `func_def` | `zt_outcome_void_text_is_success` |
| 3043 | `func_def` | `zt_outcome_void_text_propagate` |
| 3048 | `func_def` | `zt_outcome_i64_core_error_success` |
| 3056 | `func_def` | `zt_outcome_i64_core_error_failure` |
| 3065 | `func_def` | `zt_outcome_i64_core_error_failure_message` |
| 3072 | `func_def` | `zt_outcome_i64_core_error_failure_text` |
| 3079 | `func_def` | `zt_outcome_i64_core_error_is_success` |
| 3083 | `func_def` | `zt_outcome_i64_core_error_value` |
| 3088 | `func_def` | `zt_outcome_i64_core_error_propagate` |
| 3093 | `func_def` | `zt_outcome_i64_core_error_dispose` |
| 3099 | `func_def` | `zt_outcome_void_core_error_success` |
| 3106 | `func_def` | `zt_outcome_void_core_error_failure` |
| 3114 | `func_def` | `zt_outcome_void_core_error_failure_message` |
| 3121 | `func_def` | `zt_outcome_void_core_error_failure_text` |
| 3128 | `func_def` | `zt_outcome_void_core_error_is_success` |
| 3132 | `func_def` | `zt_outcome_void_core_error_propagate` |
| 3137 | `func_def` | `zt_outcome_void_core_error_dispose` |
| 3143 | `func_def` | `zt_outcome_text_core_error_success` |
| 3153 | `func_def` | `zt_outcome_text_core_error_failure` |
| 3161 | `func_def` | `zt_outcome_text_core_error_failure_message` |
| 3168 | `func_def` | `zt_outcome_text_core_error_failure_text` |
| 3175 | `func_def` | `zt_outcome_text_core_error_is_success` |
| 3179 | `func_def` | `zt_outcome_text_core_error_value` |
| 3186 | `func_def` | `zt_outcome_text_core_error_propagate` |
| 3191 | `func_def` | `zt_outcome_text_core_error_dispose` |
| 3201 | `func_def` | `zt_outcome_optional_text_core_error_success` |
| 3213 | `func_def` | `zt_outcome_optional_text_core_error_failure` |
| 3222 | `func_def` | `zt_outcome_optional_text_core_error_failure_message` |
| 3229 | `func_def` | `zt_outcome_optional_text_core_error_failure_text` |
| 3236 | `func_def` | `zt_outcome_optional_text_core_error_is_success` |
| 3240 | `func_def` | `zt_outcome_optional_text_core_error_value` |
| 3251 | `func_def` | `zt_outcome_optional_text_core_error_propagate` |
| 3256 | `func_def` | `zt_outcome_optional_text_core_error_dispose` |
| 3266 | `func_def` | `zt_outcome_optional_bytes_core_error_success` |
| 3278 | `func_def` | `zt_outcome_optional_bytes_core_error_failure` |
| 3287 | `func_def` | `zt_outcome_optional_bytes_core_error_failure_message` |
| 3294 | `func_def` | `zt_outcome_optional_bytes_core_error_failure_text` |
| 3301 | `func_def` | `zt_outcome_optional_bytes_core_error_is_success` |
| 3305 | `func_def` | `zt_outcome_optional_bytes_core_error_value` |
| 3316 | `func_def` | `zt_outcome_optional_bytes_core_error_propagate` |
| 3321 | `func_def` | `zt_outcome_optional_bytes_core_error_dispose` |
| 3331 | `func_def` | `zt_outcome_net_connection_core_error_success` |
| 3341 | `func_def` | `zt_outcome_net_connection_core_error_failure` |
| 3350 | `func_def` | `zt_outcome_net_connection_core_error_failure_message` |
| 3357 | `func_def` | `zt_outcome_net_connection_core_error_failure_text` |
| 3364 | `func_def` | `zt_outcome_net_connection_core_error_is_success` |
| 3368 | `func_def` | `zt_outcome_net_connection_core_error_value` |
| 3375 | `func_def` | `zt_outcome_net_connection_core_error_propagate` |
| 3380 | `func_def` | `zt_outcome_net_connection_core_error_dispose` |
| 3389 | `func_def` | `zt_outcome_list_i64_core_error_success` |
| 3399 | `func_def` | `zt_outcome_list_i64_core_error_failure` |
| 3407 | `func_def` | `zt_outcome_list_i64_core_error_failure_message` |
| 3414 | `func_def` | `zt_outcome_list_i64_core_error_failure_text` |
| 3421 | `func_def` | `zt_outcome_list_i64_core_error_is_success` |
| 3425 | `func_def` | `zt_outcome_list_i64_core_error_value` |
| 3432 | `func_def` | `zt_outcome_list_i64_core_error_propagate` |
| 3437 | `func_def` | `zt_outcome_list_i64_core_error_dispose` |
| 3447 | `func_def` | `zt_outcome_map_text_text_core_error_success` |
| 3457 | `func_def` | `zt_outcome_map_text_text_core_error_failure` |
| 3465 | `func_def` | `zt_outcome_map_text_text_core_error_failure_message` |
| 3472 | `func_def` | `zt_outcome_map_text_text_core_error_failure_text` |
| 3479 | `func_def` | `zt_outcome_map_text_text_core_error_is_success` |
| 3483 | `func_def` | `zt_outcome_map_text_text_core_error_value` |
| 3490 | `func_def` | `zt_outcome_map_text_text_core_error_propagate` |
| 3495 | `func_def` | `zt_outcome_map_text_text_core_error_dispose` |
| 3507 | `func_def` | `zt_detect_cycles` |
| 3533 | `func_def` | `zt_runtime_check_for_cycles` |
| 3542 | `macro` | `POOL_SIZE` |
| 3544 | `struct` | `zt_pool` |
| 3551 | `func_def` | `zt_text_pool_alloc` |
| 3558 | `func_def` | `zt_text_pool_free` |
| 3566 | `func_def` | `zt_validate_pointer` |
| 3570 | `func_def` | `zt_runtime_safe_function_example` |
| 3580 | `func_def` | `zt_validate_and_free_text` |
| 3588 | `func_def` | `zt_validate_and_free_list_i64` |
| 3596 | `func_def` | `zt_validate_and_free_map_text_text` |
| 3604 | `func_def` | `zt_host_default_read_file` |
| 3665 | `func_def` | `zt_host_default_write_file` |
| 3694 | `func_def` | `zt_host_default_path_exists` |
| 3704 | `func_def` | `zt_host_default_read_line_stdin` |
| 3767 | `func_def` | `zt_host_default_read_all_stdin` |
| 3805 | `func_def` | `zt_host_default_write_stream` |
| 3824 | `func_def` | `zt_host_default_write_stdout` |
| 3828 | `func_def` | `zt_host_default_write_stderr` |
| 3832 | `func_def` | `zt_host_default_time_now_unix_ms` |
| 3844 | `func_def` | `zt_host_default_time_sleep_ms` |
| 3872 | `func_def` | `zt_host_random_next_u64` |
| 3881 | `func_def` | `zt_host_default_random_seed` |
| 3890 | `func_def` | `zt_host_default_random_next_i64` |
| 3895 | `func_def` | `zt_host_strdup_text` |
| 3909 | `func_def` | `zt_host_default_os_current_dir` |
| 3953 | `func_def` | `zt_host_default_os_change_dir` |
| 3968 | `func_def` | `zt_host_default_os_env` |
| 3986 | `func_def` | `zt_host_default_os_pid` |
| 3994 | `func_def` | `zt_host_default_os_platform` |
| 4006 | `func_def` | `zt_host_default_os_arch` |
| 4022 | `func_def` | `zt_host_command_append` |
| 4051 | `func_def` | `zt_host_command_append_quoted` |
| 4073 | `func_def` | `zt_host_default_process_run` |
| 4193 | `func_def` | `zt_host_set_api` |
| 4236 | `func_def` | `zt_host_get_api` |
| 4240 | `func_def` | `zt_host_read_file` |
| 4245 | `func_def` | `zt_host_write_file` |
| 4249 | `func_def` | `zt_host_path_exists` |
| 4252 | `func_def` | `zt_host_read_line_stdin` |
| 4256 | `func_def` | `zt_host_read_all_stdin` |
| 4260 | `func_def` | `zt_host_write_stdout` |
| 4264 | `func_def` | `zt_host_write_stderr` |
| 4268 | `func_def` | `zt_host_time_now_unix_ms` |
| 4272 | `func_def` | `zt_host_time_sleep_ms` |
| 4276 | `func_def` | `zt_host_random_seed` |
| 4280 | `func_def` | `zt_host_random_next_i64` |
| 4284 | `func_def` | `zt_host_os_current_dir` |
| 4288 | `func_def` | `zt_host_os_change_dir` |
| 4292 | `func_def` | `zt_host_os_env` |
| 4296 | `func_def` | `zt_host_os_pid` |
| 4300 | `func_def` | `zt_host_os_platform` |
| 4304 | `func_def` | `zt_host_os_arch` |
| 4308 | `func_def` | `zt_host_process_run` |
| 4312 | `func_def` | `zt_json_skip_whitespace` |
| 4319 | `func_def` | `zt_json_buffer_reserve` |
| 4347 | `func_def` | `zt_json_buffer_append_char` |
| 4354 | `func_def` | `zt_json_buffer_append_bytes` |
| 4365 | `func_def` | `zt_json_buffer_append_escaped_text` |
| 4410 | `func_def` | `zt_json_parse_string` |
| 4494 | `func_def` | `zt_json_parse_unquoted_value` |
| 4536 | `func_def` | `zt_json_parse_map_text_text` |
| 4636 | `func_decl` | `zt_outcome_map_text_text_core_error_failure_message` |
| 4639 | `func_def` | `zt_json_stringify_map_text_text` |
| 4685 | `func_def` | `zt_json_pretty_map_text_text` |
| 4747 | `func_def` | `zt_u64_magnitude` |
| 4754 | `func_def` | `zt_format_hex_i64` |
| 4767 | `func_def` | `zt_format_bin_i64` |
| 4800 | `func_def` | `zt_format_bytes_impl` |
| 4824 | `func_def` | `zt_format_bytes_binary` |
| 4829 | `func_def` | `zt_format_bytes_decimal` |
| 4834 | `func_def` | `zt_math_pow` |
| 4838 | `func_def` | `zt_math_sqrt` |
| 4842 | `func_def` | `zt_math_floor` |
| 4846 | `func_def` | `zt_math_ceil` |
| 4850 | `func_def` | `zt_math_round_half_away_from_zero` |
| 4854 | `func_def` | `zt_math_trunc` |
| 4858 | `func_def` | `zt_math_sin` |
| 4862 | `func_def` | `zt_math_cos` |
| 4866 | `func_def` | `zt_math_tan` |
| 4870 | `func_def` | `zt_math_asin` |
| 4874 | `func_def` | `zt_math_acos` |
| 4878 | `func_def` | `zt_math_atan` |
| 4882 | `func_def` | `zt_math_atan2` |
| 4886 | `func_def` | `zt_math_ln` |
| 4890 | `func_def` | `zt_math_log10` |
| 4894 | `func_def` | `zt_math_log_ten` |
| 4898 | `func_def` | `zt_math_log2` |
| 4902 | `func_def` | `zt_math_log` |
| 4909 | `func_def` | `zt_math_exp` |
| 4913 | `func_def` | `zt_math_is_nan` |
| 4917 | `func_def` | `zt_math_is_infinite` |
| 4921 | `func_def` | `zt_math_is_finite` |
| 4924 | `func_def` | `zt_net_startup` |
| 4948 | `func_def` | `zt_net_last_error_code` |
| 4956 | `func_def` | `zt_net_would_block_code` |
| 4964 | `func_def` | `zt_net_format_error` |
| 4976 | `func_def` | `zt_net_set_nonblocking` |
| 4993 | `func_def` | `zt_net_wait_socket` |
| 5037 | `func_def` | `zt_net_socket_error` |
| 5058 | `func_def` | `zt_net_connection_new` |
| 5075 | `func_def` | `zt_net_core_error_from_prefixed_message` |
| 5099 | `func_def` | `zt_net_connection_core_error_failure_prefixed` |
| 5106 | `func_def` | `zt_net_optional_bytes_core_error_failure_prefixed` |
| 5113 | `func_def` | `zt_net_void_core_error_failure_prefixed` |
| 5120 | `func_def` | `zt_net_connect` |
| 5224 | `func_def` | `zt_net_effective_timeout_ms` |
| 5228 | `func_def` | `zt_net_read_some` |
| 5293 | `func_def` | `zt_net_write_all` |
| 5343 | `func_def` | `zt_net_close` |
| 5356 | `func_def` | `zt_net_is_closed` |
| 5361 | `func_def` | `zt_net_error_kind_index` |
| 5377 | `func_def` | `zt_path_is_separator_char` |
| 5381 | `func_def` | `zt_path_is_drive_letter` |
| 5385 | `func_def` | `zt_path_parse_prefix` |
| 5430 | `func_def` | `zt_path_segment_is_dot` |
| 5434 | `func_def` | `zt_path_segment_is_dot_dot` |
| 5438 | `func_def` | `zt_path_collect_segments` |
| 5487 | `func_def` | `zt_path_normalize` |
| 5614 | `func_def` | `zt_path_is_absolute` |
| 5624 | `func_def` | `zt_path_absolute` |
| 5669 | `func_def` | `zt_path_relative` |
| 5796 | `func_def` | `zt_add_i64` |
| 5804 | `func_def` | `zt_sub_i64` |
| 5812 | `func_def` | `zt_mul_i64` |
| 5820 | `func_def` | `zt_div_i64` |
| 5830 | `func_def` | `zt_rem_i64` |
| 5841 | `func_def` | `zt_validate_between_i64` |
| 5845 | `func_def` | `zt_runtime_require_grid2d_i64` |
| 5851 | `func_def` | `zt_runtime_require_grid2d_text` |
| 5857 | `func_def` | `zt_runtime_require_pqueue_i64` |
| 5863 | `func_def` | `zt_runtime_require_pqueue_text` |
| 5869 | `func_def` | `zt_runtime_require_circbuf_i64` |
| 5875 | `func_def` | `zt_runtime_require_circbuf_text` |
| 5881 | `func_def` | `zt_free_grid2d_i64` |
| 5892 | `func_def` | `zt_free_grid2d_text` |
| 5907 | `func_def` | `zt_free_pqueue_i64` |
| 5916 | `func_def` | `zt_free_pqueue_text` |
| 5929 | `func_def` | `zt_free_circbuf_i64` |
| 5939 | `func_def` | `zt_free_circbuf_text` |
| 5953 | `func_def` | `zt_queue_i64_new` |
| 5957 | `func_def` | `zt_queue_i64_enqueue` |
| 5962 | `func_def` | `zt_queue_i64_enqueue_owned` |
| 5966 | `func_def` | `zt_queue_i64_dequeue` |
| 5983 | `func_def` | `zt_queue_i64_peek` |
| 5988 | `func_def` | `zt_queue_text_new` |
| 5992 | `func_def` | `zt_queue_text_enqueue` |
| 5997 | `func_def` | `zt_queue_text_enqueue_owned` |
| 6001 | `func_def` | `zt_queue_text_dequeue` |
| 6021 | `func_def` | `zt_queue_text_peek` |
| 6026 | `func_def` | `zt_stack_i64_new` |
| 6030 | `func_def` | `zt_stack_i64_push` |
| 6035 | `func_def` | `zt_stack_i64_push_owned` |
| 6039 | `func_def` | `zt_stack_i64_pop` |
| 6050 | `func_def` | `zt_stack_i64_peek` |
| 6058 | `func_def` | `zt_stack_text_new` |
| 6062 | `func_def` | `zt_stack_text_push` |
| 6067 | `func_def` | `zt_stack_text_push_owned` |
| 6071 | `func_def` | `zt_stack_text_pop` |
| 6083 | `func_def` | `zt_stack_text_peek` |
| 6091 | `func_def` | `zt_grid2d_i64_new` |
| 6120 | `func_def` | `zt_grid2d_i64_get` |
| 6128 | `func_def` | `zt_grid2d_i64_set` |
| 6137 | `func_def` | `zt_grid2d_i64_set_owned` |
| 6152 | `func_def` | `zt_grid2d_i64_fill` |
| 6161 | `func_def` | `zt_grid2d_i64_fill_owned` |
| 6176 | `func_def` | `zt_grid2d_i64_rows` |
| 6181 | `func_def` | `zt_grid2d_i64_cols` |
| 6186 | `func_def` | `zt_grid2d_text_new` |
| 6219 | `func_def` | `zt_grid2d_text_get` |
| 6231 | `func_def` | `zt_grid2d_text_set` |
| 6244 | `func_def` | `zt_grid2d_text_set_owned` |
| 6260 | `func_def` | `zt_grid2d_text_fill` |
| 6272 | `func_def` | `zt_grid2d_text_fill_owned` |
| 6288 | `func_def` | `zt_grid2d_text_rows` |
| 6293 | `func_def` | `zt_grid2d_text_cols` |
| 6298 | `func_def` | `zt_pqueue_i64_new` |
| 6309 | `func_def` | `zt_pqueue_i64_ensure_capacity` |
| 6321 | `func_def` | `zt_pqueue_i64_sift_up` |
| 6332 | `func_def` | `zt_pqueue_i64_sift_down` |
| 6351 | `func_def` | `zt_pqueue_i64_push` |
| 6360 | `func_def` | `zt_pqueue_i64_push_owned` |
| 6377 | `func_def` | `zt_pqueue_i64_pop` |
| 6392 | `func_def` | `zt_pqueue_i64_peek` |
| 6400 | `func_def` | `zt_pqueue_i64_len` |
| 6405 | `func_def` | `zt_pqueue_text_new` |
| 6416 | `func_def` | `zt_pqueue_text_ensure_capacity` |
| 6428 | `func_def` | `zt_pqueue_text_sift_up` |
| 6441 | `func_def` | `zt_pqueue_text_sift_down` |
| 6468 | `func_def` | `zt_pqueue_text_push` |
| 6478 | `func_def` | `zt_pqueue_text_push_owned` |
| 6496 | `func_def` | `zt_pqueue_text_pop` |
| 6514 | `func_def` | `zt_pqueue_text_peek` |
| 6522 | `func_def` | `zt_pqueue_text_len` |
| 6527 | `func_def` | `zt_circbuf_i64_new` |
| 6549 | `func_def` | `zt_circbuf_i64_push` |
| 6561 | `func_def` | `zt_circbuf_i64_push_owned` |
| 6577 | `func_def` | `zt_circbuf_i64_pop` |
| 6589 | `func_def` | `zt_circbuf_i64_peek` |
| 6597 | `func_def` | `zt_circbuf_i64_len` |
| 6602 | `func_def` | `zt_circbuf_i64_capacity` |
| 6607 | `func_def` | `zt_circbuf_i64_is_full` |
| 6612 | `func_def` | `zt_circbuf_text_new` |
| 6634 | `func_def` | `zt_circbuf_text_push` |
| 6652 | `func_def` | `zt_circbuf_text_push_owned` |
| 6669 | `func_def` | `zt_circbuf_text_pop` |
| 6685 | `func_def` | `zt_circbuf_text_peek` |
| 6693 | `func_def` | `zt_circbuf_text_len` |
| 6698 | `func_def` | `zt_circbuf_text_capacity` |
| 6703 | `func_def` | `zt_circbuf_text_is_full` |
| 6708 | `func_def` | `zt_runtime_require_btreemap_text_text` |
| 6714 | `func_def` | `zt_runtime_require_btreeset_text` |
| 6720 | `func_def` | `zt_runtime_require_grid3d_i64` |
| 6726 | `func_def` | `zt_runtime_require_grid3d_text` |
| 6732 | `func_def` | `zt_free_btreemap_text_text` |
| 6748 | `func_def` | `zt_free_btreeset_text` |
| 6761 | `func_def` | `zt_free_grid3d_i64` |
| 6773 | `func_def` | `zt_free_grid3d_text` |
| 6789 | `func_def` | `zt_btreemap_text_text_find` |
| 6809 | `func_def` | `zt_btreemap_text_text_ensure_capacity` |
| 6823 | `func_def` | `zt_btreemap_text_text_new` |
| 6834 | `func_def` | `zt_btreemap_text_text_set` |
| 6863 | `func_def` | `zt_btreemap_text_text_set_owned` |
| 6883 | `func_def` | `zt_btreemap_text_text_get` |
| 6895 | `func_def` | `zt_btreemap_text_text_get_optional` |
| 6904 | `func_def` | `zt_btreemap_text_text_contains` |
| 6912 | `func_def` | `zt_btreemap_text_text_remove` |
| 6932 | `func_def` | `zt_btreemap_text_text_remove_owned` |
| 6952 | `func_def` | `zt_btreemap_text_text_len` |
| 6957 | `func_def` | `zt_btreeset_text_find` |
| 6977 | `func_def` | `zt_btreeset_text_ensure_capacity` |
| 6989 | `func_def` | `zt_btreeset_text_new` |
| 7000 | `func_def` | `zt_btreeset_text_insert` |
| 7019 | `func_def` | `zt_btreeset_text_insert_owned` |
| 7037 | `func_def` | `zt_btreeset_text_contains` |
| 7043 | `func_def` | `zt_btreeset_text_remove` |
| 7059 | `func_def` | `zt_btreeset_text_remove_owned` |
| 7077 | `func_def` | `zt_btreeset_text_len` |
| 7082 | `func_def` | `zt_grid3d_i64_new` |
| 7108 | `func_def` | `zt_grid3d_i64_get` |
| 7116 | `func_def` | `zt_grid3d_i64_set` |
| 7125 | `func_def` | `zt_grid3d_i64_set_owned` |
| 7140 | `func_def` | `zt_grid3d_i64_fill` |
| 7149 | `func_def` | `zt_grid3d_i64_fill_owned` |
| 7164 | `func_def` | `zt_grid3d_i64_depth` |
| 7169 | `func_def` | `zt_grid3d_i64_rows` |
| 7174 | `func_def` | `zt_grid3d_i64_cols` |
| 7179 | `func_def` | `zt_grid3d_text_new` |
| 7209 | `func_def` | `zt_grid3d_text_get` |
| 7221 | `func_def` | `zt_grid3d_text_set` |
| 7236 | `func_def` | `zt_grid3d_text_set_owned` |
| 7252 | `func_def` | `zt_grid3d_text_fill` |
| 7264 | `func_def` | `zt_grid3d_text_fill_owned` |
| 7280 | `func_def` | `zt_grid3d_text_depth` |
| 7285 | `func_def` | `zt_grid3d_text_rows` |
| 7290 | `func_def` | `zt_grid3d_text_cols` |

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
