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
- Source files: 3
- Extracted symbols: 420

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `runtime/c/zenith_rt.c` | 5231 | 395 | 1 |
| `runtime/c/zenith_rt.h` | 917 | 1 | 0 |
| `runtime/c/zenith_rt_templates.h` | 2129 | 24 | 0 |

### Local Dependencies

- `runtime/c/zenith_rt_templates.h`

### Related Tests

- `tests/behavior/where_contract_construct_error/src/app/main.zt`
- `tests/behavior/where_contract_field_assign_error/src/app/main.zt`
- `tests/behavior/where_contract_param_error/src/app/main.zt`
- `tests/behavior/where_contract_param_where_invalid_error/src/app/main.zt`
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
| 16 | `macro` | `WIN32_LEAN_AND_MEAN` |
| 35 | `macro` | `ZT_NET_INVALID_SOCKET` |
| 38 | `macro` | `ZT_NET_INVALID_SOCKET` |
| 41 | `func_decl` | `zt_pqueue_i64_ensure_capacity` |
| 42 | `func_decl` | `zt_pqueue_text_ensure_capacity` |
| 43 | `func_decl` | `zt_btreemap_text_text_ensure_capacity` |
| 44 | `func_decl` | `zt_btreeset_text_ensure_capacity` |
| 46 | `macro` | `ZT_DYNAMIC_HEAP_BASE` |
| 47 | `macro` | `ZT_DYNAMIC_HEAP_CAPACITY` |
| 49 | `struct` | `zt_dynamic_heap_entry` |
| 58 | `func_def` | `zt_find_dynamic_heap_entry` |
| 77 | `func_def` | `zt_safe_message` |
| 81 | `func_def` | `zt_runtime_append_text` |
| 101 | `func_def` | `zt_runtime_store_error` |
| 118 | `func_def` | `zt_runtime_stable_code` |
| 135 | `func_def` | `zt_runtime_default_help` |
| 164 | `func_def` | `zt_runtime_print_error` |
| 199 | `func_def` | `zt_outcome_i64_text_failure_message` |
| 206 | `func_def` | `zt_outcome_text_text_failure_message` |
| 213 | `func_def` | `zt_outcome_optional_text_text_failure_message` |
| 220 | `func_def` | `zt_outcome_bytes_text_failure_message` |
| 227 | `func_def` | `zt_outcome_optional_bytes_text_failure_message` |
| 234 | `func_def` | `zt_outcome_net_connection_text_failure_message` |
| 241 | `func_def` | `zt_outcome_void_text_failure_message` |
| 248 | `func_def` | `zt_outcome_list_i64_text_failure_message` |
| 255 | `func_decl` | `zt_host_default_read_file` |
| 256 | `func_decl` | `zt_host_default_write_file` |
| 257 | `func_decl` | `zt_host_default_path_exists` |
| 258 | `func_decl` | `zt_host_default_read_line_stdin` |
| 259 | `func_decl` | `zt_host_default_read_all_stdin` |
| 260 | `func_decl` | `zt_host_default_write_stdout` |
| 261 | `func_decl` | `zt_host_default_write_stderr` |
| 262 | `func_decl` | `zt_host_default_time_now_unix_ms` |
| 263 | `func_decl` | `zt_host_default_time_sleep_ms` |
| 264 | `func_decl` | `zt_host_default_random_seed` |
| 265 | `func_decl` | `zt_host_default_random_next_i64` |
| 266 | `func_decl` | `zt_host_default_os_current_dir` |
| 267 | `func_decl` | `zt_host_default_os_change_dir` |
| 268 | `func_decl` | `zt_host_default_os_env` |
| 269 | `func_decl` | `zt_host_default_os_pid` |
| 270 | `func_decl` | `zt_host_default_os_platform` |
| 271 | `func_decl` | `zt_host_default_os_arch` |
| 272 | `func_decl` | `zt_host_default_process_run` |
| 295 | `func_def` | `zt_runtime_span_unknown` |
| 303 | `func_def` | `zt_runtime_make_span` |
| 311 | `func_def` | `zt_runtime_span_is_known` |
| 318 | `func_def` | `zt_runtime_last_error` |
| 322 | `func_def` | `zt_runtime_clear_error` |
| 332 | `func_def` | `zt_error_kind_name` |
| 361 | `func_def` | `zt_header_from_ref` |
| 365 | `func_def` | `zt_free_text` |
| 376 | `func_def` | `zt_free_bytes` |
| 389 | `func_def` | `zt_free_dyn_text_repr` |
| 402 | `func_def` | `zt_free_list_dyn_text_repr` |
| 419 | `func_def` | `zt_net_close_socket_handle` |
| 433 | `func_def` | `zt_free_net_connection` |
| 446 | `func_def` | `zt_runtime_require_text` |
| 452 | `func_def` | `zt_runtime_require_bytes` |
| 458 | `func_def` | `zt_runtime_require_net_connection` |
| 464 | `func_def` | `zt_runtime_require_list_i64` |
| 470 | `func_def` | `zt_runtime_require_list_text` |
| 476 | `func_def` | `zt_runtime_require_dyn_text_repr` |
| 482 | `func_def` | `zt_runtime_require_list_dyn_text_repr` |
| 487 | `func_def` | `zt_runtime_require_map_text_text` |
| 493 | `func_def` | `zt_normalize_slice_end` |
| 514 | `func_def` | `zt_utf8_is_continuation` |
| 533 | `func_def` | `zt_utf8_validate` |
| 779 | `func_def` | `zt_list_dyn_text_repr_reserve` |
| 825 | `func_decl` | `zt_free_grid2d_i64` |
| 826 | `func_decl` | `zt_free_grid2d_text` |
| 827 | `func_decl` | `zt_free_pqueue_i64` |
| 828 | `func_decl` | `zt_free_pqueue_text` |
| 829 | `func_decl` | `zt_free_circbuf_i64` |
| 830 | `func_decl` | `zt_free_circbuf_text` |
| 831 | `func_decl` | `zt_free_btreemap_text_text` |
| 832 | `func_decl` | `zt_free_btreeset_text` |
| 833 | `func_decl` | `zt_free_grid3d_i64` |
| 834 | `func_decl` | `zt_free_grid3d_text` |
| 835 | `func_decl` | `zt_free_net_connection` |
| 837 | `func_def` | `zt_register_dynamic_heap_kind` |
| 864 | `func_def` | `zt_retain` |
| 882 | `func_def` | `zt_release` |
| 976 | `func_def` | `zt_deep_copy` |
| 1141 | `struct` | `zt_shared_ops` |
| 1145 | `struct` | `zt_shared_handle` |
| 1159 | `func_def` | `zt_shared_text_snapshot_value` |
| 1166 | `func_def` | `zt_shared_bytes_snapshot_value` |
| 1181 | `func_def` | `zt_shared_handle_init` |
| 1192 | `func_def` | `zt_shared_handle_retain` |
| 1215 | `func_def` | `zt_shared_handle_release` |
| 1238 | `func_def` | `zt_shared_handle_borrow` |
| 1246 | `func_def` | `zt_shared_handle_snapshot` |
| 1251 | `func_def` | `zt_shared_handle_ref_count` |
| 1259 | `func_def` | `zt_shared_text_new` |
| 1272 | `func_def` | `zt_shared_text_retain` |
| 1281 | `func_def` | `zt_shared_text_release` |
| 1292 | `func_def` | `zt_shared_text_borrow` |
| 1298 | `func_def` | `zt_shared_text_snapshot` |
| 1304 | `func_def` | `zt_shared_text_ref_count` |
| 1310 | `func_def` | `zt_shared_bytes_new` |
| 1323 | `func_def` | `zt_shared_bytes_retain` |
| 1332 | `func_def` | `zt_shared_bytes_release` |
| 1343 | `func_def` | `zt_shared_bytes_borrow` |
| 1349 | `func_def` | `zt_shared_bytes_snapshot` |
| 1355 | `func_def` | `zt_shared_bytes_ref_count` |
| 1361 | `func_def` | `zt_runtime_report_error` |
| 1365 | `func_def` | `zt_runtime_exit_code_for_kind` |
| 1376 | `func_def` | `zt_runtime_error_ex` |
| 1382 | `func_def` | `zt_runtime_error_with_span` |
| 1386 | `func_def` | `zt_runtime_error` |
| 1390 | `func_def` | `zt_assert` |
| 1396 | `func_def` | `zt_check` |
| 1402 | `func_def` | `zt_panic` |
| 1406 | `func_def` | `zt_test_fail` |
| 1412 | `func_def` | `zt_test_skip` |
| 1418 | `func_def` | `zt_contract_failed` |
| 1422 | `func_def` | `zt_contract_failed_i64` |
| 1434 | `func_def` | `zt_contract_failed_float` |
| 1446 | `func_def` | `zt_contract_failed_bool` |
| 1456 | `func_def` | `zt_text_from_utf8` |
| 1481 | `func_def` | `zt_text_from_utf8_literal` |
| 1489 | `func_def` | `zt_text_concat` |
| 1510 | `func_def` | `zt_text_index` |
| 1520 | `func_def` | `zt_text_slice` |
| 1544 | `func_def` | `zt_text_eq` |
| 1563 | `func_def` | `zt_text_len` |
| 1568 | `func_def` | `zt_text_data` |
| 1573 | `func_def` | `zt_text_deep_copy` |
| 1578 | `func_def` | `zt_bytes_empty` |
| 1582 | `func_def` | `zt_bytes_from_array` |
| 1609 | `func_def` | `zt_bytes_from_list_i64` |
| 1632 | `func_def` | `zt_bytes_to_list_i64` |
| 1652 | `func_def` | `zt_bytes_join` |
| 1669 | `func_def` | `zt_bytes_starts_with` |
| 1684 | `func_def` | `zt_bytes_ends_with` |
| 1699 | `func_def` | `zt_bytes_contains` |
| 1721 | `func_def` | `zt_text_to_utf8_bytes` |
| 1726 | `func_def` | `zt_text_from_utf8_bytes` |
| 1752 | `func_def` | `zt_bytes_len` |
| 1757 | `func_def` | `zt_bytes_get` |
| 1767 | `func_def` | `zt_bytes_slice` |
| 1794 | `func_def` | `zt_list_i64_get_optional` |
| 1807 | `func_def` | `zt_list_text_get_optional` |
| 1821 | `func_def` | `zt_queue_i64_new` |
| 1825 | `func_def` | `zt_queue_i64_enqueue` |
| 1829 | `func_def` | `zt_queue_i64_enqueue_owned` |
| 1833 | `func_def` | `zt_queue_i64_dequeue` |
| 1849 | `func_def` | `zt_queue_i64_peek` |
| 1857 | `func_def` | `zt_queue_text_new` |
| 1861 | `func_def` | `zt_queue_text_enqueue` |
| 1865 | `func_def` | `zt_queue_text_enqueue_owned` |
| 1869 | `func_def` | `zt_queue_text_dequeue` |
| 1890 | `func_def` | `zt_queue_text_peek` |
| 1903 | `func_def` | `zt_stack_i64_new` |
| 1907 | `func_def` | `zt_stack_i64_push` |
| 1911 | `func_def` | `zt_stack_i64_push_owned` |
| 1915 | `func_def` | `zt_stack_i64_pop` |
| 1928 | `func_def` | `zt_stack_i64_peek` |
| 1936 | `func_def` | `zt_stack_text_new` |
| 1940 | `func_def` | `zt_stack_text_push` |
| 1944 | `func_def` | `zt_stack_text_push_owned` |
| 1948 | `func_def` | `zt_stack_text_pop` |
| 1966 | `func_def` | `zt_stack_text_peek` |
| 1981 | `func_def` | `zt_dyn_text_repr_alloc` |
| 1992 | `func_def` | `zt_dyn_text_repr_from_i64` |
| 1998 | `func_def` | `zt_dyn_text_repr_from_float` |
| 2004 | `func_def` | `zt_dyn_text_repr_from_bool` |
| 2010 | `func_def` | `zt_dyn_text_repr_from_text_owned` |
| 2018 | `func_def` | `zt_dyn_text_repr_from_text` |
| 2023 | `func_def` | `zt_dyn_text_repr_clone` |
| 2042 | `func_def` | `zt_dyn_text_repr_to_text` |
| 2065 | `func_def` | `zt_dyn_text_repr_text_len` |
| 2076 | `func_def` | `zt_list_dyn_text_repr_new` |
| 2087 | `func_def` | `zt_list_dyn_text_repr_from_array` |
| 2110 | `func_def` | `zt_list_dyn_text_repr_from_array_owned` |
| 2132 | `func_def` | `zt_list_dyn_text_repr_get` |
| 2146 | `func_def` | `zt_list_dyn_text_repr_len` |
| 2151 | `func_def` | `zt_list_dyn_text_repr_deep_copy` |
| 2170 | `func_def` | `zt_thread_boundary_copy_text` |
| 2175 | `func_def` | `zt_thread_boundary_copy_bytes` |
| 2180 | `func_def` | `zt_thread_boundary_copy_list_text` |
| 2185 | `func_def` | `zt_thread_boundary_copy_dyn_text_repr` |
| 2190 | `func_def` | `zt_thread_boundary_copy_list_dyn_text_repr` |
| 2195 | `func_def` | `zt_core_error_make` |
| 2221 | `func_def` | `zt_core_error_from_message` |
| 2230 | `func_def` | `zt_core_error_from_text` |
| 2241 | `func_def` | `zt_core_error_clone` |
| 2252 | `func_def` | `zt_core_error_dispose` |
| 2262 | `func_def` | `zt_core_error_message_or_default` |
| 2271 | `func_def` | `zt_outcome_text_text_eq` |
| 2295 | `func_def` | `zt_outcome_optional_text_text_success` |
| 2308 | `func_def` | `zt_outcome_optional_text_text_failure` |
| 2320 | `func_def` | `zt_outcome_optional_text_text_is_success` |
| 2324 | `func_def` | `zt_outcome_optional_text_text_value` |
| 2339 | `func_def` | `zt_outcome_optional_text_text_propagate` |
| 2343 | `func_def` | `zt_outcome_optional_bytes_text_success` |
| 2356 | `func_def` | `zt_outcome_optional_bytes_text_failure` |
| 2368 | `func_def` | `zt_outcome_optional_bytes_text_is_success` |
| 2372 | `func_def` | `zt_outcome_optional_bytes_text_value` |
| 2387 | `func_def` | `zt_outcome_optional_bytes_text_propagate` |
| 2391 | `func_def` | `zt_outcome_map_text_text_failure_message` |
| 2407 | `func_def` | `zt_outcome_void_text_success` |
| 2414 | `func_def` | `zt_outcome_void_text_failure` |
| 2426 | `func_def` | `zt_outcome_void_text_is_success` |
| 2430 | `func_def` | `zt_outcome_void_text_propagate` |
| 2435 | `func_def` | `zt_outcome_i64_core_error_success` |
| 2443 | `func_def` | `zt_outcome_i64_core_error_failure` |
| 2452 | `func_def` | `zt_outcome_i64_core_error_failure_message` |
| 2459 | `func_def` | `zt_outcome_i64_core_error_failure_text` |
| 2466 | `func_def` | `zt_outcome_i64_core_error_is_success` |
| 2470 | `func_def` | `zt_outcome_i64_core_error_value` |
| 2475 | `func_def` | `zt_outcome_i64_core_error_propagate` |
| 2480 | `func_def` | `zt_outcome_i64_core_error_dispose` |
| 2486 | `func_def` | `zt_outcome_void_core_error_success` |
| 2493 | `func_def` | `zt_outcome_void_core_error_failure` |
| 2501 | `func_def` | `zt_outcome_void_core_error_failure_message` |
| 2508 | `func_def` | `zt_outcome_void_core_error_failure_text` |
| 2515 | `func_def` | `zt_outcome_void_core_error_is_success` |
| 2519 | `func_def` | `zt_outcome_void_core_error_propagate` |
| 2524 | `func_def` | `zt_outcome_void_core_error_dispose` |
| 2530 | `func_def` | `zt_outcome_text_core_error_success` |
| 2540 | `func_def` | `zt_outcome_text_core_error_failure` |
| 2548 | `func_def` | `zt_outcome_text_core_error_failure_message` |
| 2555 | `func_def` | `zt_outcome_text_core_error_failure_text` |
| 2562 | `func_def` | `zt_outcome_text_core_error_is_success` |
| 2566 | `func_def` | `zt_outcome_text_core_error_value` |
| 2573 | `func_def` | `zt_outcome_text_core_error_propagate` |
| 2578 | `func_def` | `zt_outcome_text_core_error_dispose` |
| 2588 | `func_def` | `zt_outcome_optional_text_core_error_success` |
| 2600 | `func_def` | `zt_outcome_optional_text_core_error_failure` |
| 2609 | `func_def` | `zt_outcome_optional_text_core_error_failure_message` |
| 2616 | `func_def` | `zt_outcome_optional_text_core_error_failure_text` |
| 2623 | `func_def` | `zt_outcome_optional_text_core_error_is_success` |
| 2627 | `func_def` | `zt_outcome_optional_text_core_error_value` |
| 2638 | `func_def` | `zt_outcome_optional_text_core_error_propagate` |
| 2643 | `func_def` | `zt_outcome_optional_text_core_error_dispose` |
| 2653 | `func_def` | `zt_outcome_optional_bytes_core_error_success` |
| 2665 | `func_def` | `zt_outcome_optional_bytes_core_error_failure` |
| 2674 | `func_def` | `zt_outcome_optional_bytes_core_error_failure_message` |
| 2681 | `func_def` | `zt_outcome_optional_bytes_core_error_failure_text` |
| 2688 | `func_def` | `zt_outcome_optional_bytes_core_error_is_success` |
| 2692 | `func_def` | `zt_outcome_optional_bytes_core_error_value` |
| 2703 | `func_def` | `zt_outcome_optional_bytes_core_error_propagate` |
| 2708 | `func_def` | `zt_outcome_optional_bytes_core_error_dispose` |
| 2718 | `func_def` | `zt_outcome_net_connection_core_error_success` |
| 2728 | `func_def` | `zt_outcome_net_connection_core_error_failure` |
| 2737 | `func_def` | `zt_outcome_net_connection_core_error_failure_message` |
| 2744 | `func_def` | `zt_outcome_net_connection_core_error_failure_text` |
| 2751 | `func_def` | `zt_outcome_net_connection_core_error_is_success` |
| 2755 | `func_def` | `zt_outcome_net_connection_core_error_value` |
| 2762 | `func_def` | `zt_outcome_net_connection_core_error_propagate` |
| 2767 | `func_def` | `zt_outcome_net_connection_core_error_dispose` |
| 2776 | `func_def` | `zt_outcome_list_i64_core_error_success` |
| 2786 | `func_def` | `zt_outcome_list_i64_core_error_failure` |
| 2794 | `func_def` | `zt_outcome_list_i64_core_error_failure_message` |
| 2801 | `func_def` | `zt_outcome_list_i64_core_error_failure_text` |
| 2808 | `func_def` | `zt_outcome_list_i64_core_error_is_success` |
| 2812 | `func_def` | `zt_outcome_list_i64_core_error_value` |
| 2819 | `func_def` | `zt_outcome_list_i64_core_error_propagate` |
| 2824 | `func_def` | `zt_outcome_list_i64_core_error_dispose` |
| 2834 | `func_def` | `zt_outcome_map_text_text_core_error_success` |
| 2844 | `func_def` | `zt_outcome_map_text_text_core_error_failure` |
| 2852 | `func_def` | `zt_outcome_map_text_text_core_error_failure_message` |
| 2859 | `func_def` | `zt_outcome_map_text_text_core_error_failure_text` |
| 2866 | `func_def` | `zt_outcome_map_text_text_core_error_is_success` |
| 2870 | `func_def` | `zt_outcome_map_text_text_core_error_value` |
| 2877 | `func_def` | `zt_outcome_map_text_text_core_error_propagate` |
| 2882 | `func_def` | `zt_outcome_map_text_text_core_error_dispose` |
| 2894 | `func_def` | `zt_detect_cycles` |
| 2920 | `func_def` | `zt_runtime_check_for_cycles` |
| 2929 | `macro` | `POOL_SIZE` |
| 2931 | `struct` | `zt_pool` |
| 2938 | `func_def` | `zt_text_pool_alloc` |
| 2945 | `func_def` | `zt_text_pool_free` |
| 2953 | `func_def` | `zt_validate_pointer` |
| 2957 | `func_def` | `zt_runtime_safe_function_example` |
| 2967 | `func_def` | `zt_validate_and_free_text` |
| 2975 | `func_def` | `zt_validate_and_free_list_i64` |
| 2983 | `func_def` | `zt_validate_and_free_map_text_text` |
| 2991 | `func_def` | `zt_host_default_read_file` |
| 3052 | `func_def` | `zt_host_default_write_file` |
| 3081 | `func_def` | `zt_host_default_path_exists` |
| 3091 | `func_def` | `zt_host_default_read_line_stdin` |
| 3154 | `func_def` | `zt_host_default_read_all_stdin` |
| 3192 | `func_def` | `zt_host_default_write_stream` |
| 3211 | `func_def` | `zt_host_default_write_stdout` |
| 3215 | `func_def` | `zt_host_default_write_stderr` |
| 3219 | `func_def` | `zt_host_default_time_now_unix_ms` |
| 3231 | `func_def` | `zt_host_default_time_sleep_ms` |
| 3259 | `func_def` | `zt_host_random_next_u64` |
| 3268 | `func_def` | `zt_host_default_random_seed` |
| 3277 | `func_def` | `zt_host_default_random_next_i64` |
| 3282 | `func_def` | `zt_host_strdup_text` |
| 3296 | `func_def` | `zt_host_default_os_current_dir` |
| 3340 | `func_def` | `zt_host_default_os_change_dir` |
| 3355 | `func_def` | `zt_host_default_os_env` |
| 3373 | `func_def` | `zt_host_default_os_pid` |
| 3381 | `func_def` | `zt_host_default_os_platform` |
| 3393 | `func_def` | `zt_host_default_os_arch` |
| 3409 | `func_def` | `zt_host_command_append` |
| 3438 | `func_def` | `zt_host_default_process_run` |
| 3558 | `func_def` | `zt_host_set_api` |
| 3601 | `func_def` | `zt_host_get_api` |
| 3605 | `func_def` | `zt_host_read_file` |
| 3610 | `func_def` | `zt_host_write_file` |
| 3614 | `func_def` | `zt_host_path_exists` |
| 3617 | `func_def` | `zt_host_read_line_stdin` |
| 3621 | `func_def` | `zt_host_read_all_stdin` |
| 3625 | `func_def` | `zt_host_write_stdout` |
| 3629 | `func_def` | `zt_host_write_stderr` |
| 3633 | `func_def` | `zt_host_time_now_unix_ms` |
| 3637 | `func_def` | `zt_host_time_sleep_ms` |
| 3641 | `func_def` | `zt_host_random_seed` |
| 3645 | `func_def` | `zt_host_random_next_i64` |
| 3649 | `func_def` | `zt_host_os_current_dir` |
| 3653 | `func_def` | `zt_host_os_change_dir` |
| 3657 | `func_def` | `zt_host_os_env` |
| 3661 | `func_def` | `zt_host_os_pid` |
| 3665 | `func_def` | `zt_host_os_platform` |
| 3669 | `func_def` | `zt_host_os_arch` |
| 3673 | `func_def` | `zt_host_process_run` |
| 3677 | `func_def` | `zt_json_skip_whitespace` |
| 3684 | `func_def` | `zt_json_buffer_reserve` |
| 3712 | `func_def` | `zt_json_buffer_append_char` |
| 3719 | `func_def` | `zt_json_buffer_append_bytes` |
| 3730 | `func_def` | `zt_json_buffer_append_escaped_text` |
| 3775 | `func_def` | `zt_json_parse_string` |
| 3859 | `func_def` | `zt_json_parse_unquoted_value` |
| 3901 | `func_def` | `zt_json_parse_map_text_text` |
| 4001 | `func_decl` | `zt_outcome_map_text_text_core_error_failure_message` |
| 4004 | `func_def` | `zt_json_stringify_map_text_text` |
| 4050 | `func_def` | `zt_json_pretty_map_text_text` |
| 4112 | `func_def` | `zt_u64_magnitude` |
| 4119 | `func_def` | `zt_format_hex_i64` |
| 4132 | `func_def` | `zt_format_bin_i64` |
| 4165 | `func_def` | `zt_format_bytes_impl` |
| 4189 | `func_def` | `zt_format_bytes_binary` |
| 4194 | `func_def` | `zt_format_bytes_decimal` |
| 4199 | `func_def` | `zt_math_pow` |
| 4203 | `func_def` | `zt_math_sqrt` |
| 4207 | `func_def` | `zt_math_floor` |
| 4211 | `func_def` | `zt_math_ceil` |
| 4215 | `func_def` | `zt_math_round_half_away_from_zero` |
| 4219 | `func_def` | `zt_math_trunc` |
| 4223 | `func_def` | `zt_math_sin` |
| 4227 | `func_def` | `zt_math_cos` |
| 4231 | `func_def` | `zt_math_tan` |
| 4235 | `func_def` | `zt_math_asin` |
| 4239 | `func_def` | `zt_math_acos` |
| 4243 | `func_def` | `zt_math_atan` |
| 4247 | `func_def` | `zt_math_atan2` |
| 4251 | `func_def` | `zt_math_ln` |
| 4255 | `func_def` | `zt_math_log10` |
| 4259 | `func_def` | `zt_math_log_ten` |
| 4263 | `func_def` | `zt_math_log2` |
| 4267 | `func_def` | `zt_math_log` |
| 4274 | `func_def` | `zt_math_exp` |
| 4278 | `func_def` | `zt_math_is_nan` |
| 4282 | `func_def` | `zt_math_is_infinite` |
| 4286 | `func_def` | `zt_math_is_finite` |
| 4289 | `func_def` | `zt_net_startup` |
| 4313 | `func_def` | `zt_net_last_error_code` |
| 4321 | `func_def` | `zt_net_would_block_code` |
| 4329 | `func_def` | `zt_net_format_error` |
| 4341 | `func_def` | `zt_net_set_nonblocking` |
| 4358 | `func_def` | `zt_net_wait_socket` |
| 4402 | `func_def` | `zt_net_socket_error` |
| 4423 | `func_def` | `zt_net_connection_new` |
| 4440 | `func_def` | `zt_net_core_error_from_prefixed_message` |
| 4464 | `func_def` | `zt_net_connection_core_error_failure_prefixed` |
| 4471 | `func_def` | `zt_net_optional_bytes_core_error_failure_prefixed` |
| 4478 | `func_def` | `zt_net_void_core_error_failure_prefixed` |
| 4485 | `func_def` | `zt_net_connect` |
| 4589 | `func_def` | `zt_net_effective_timeout_ms` |
| 4593 | `func_def` | `zt_net_read_some` |
| 4658 | `func_def` | `zt_net_write_all` |
| 4708 | `func_def` | `zt_net_close` |
| 4721 | `func_def` | `zt_net_is_closed` |
| 4726 | `func_def` | `zt_net_error_kind_index` |
| 4742 | `func_def` | `zt_path_is_separator_char` |
| 4746 | `func_def` | `zt_path_is_drive_letter` |
| 4750 | `func_def` | `zt_path_parse_prefix` |
| 4795 | `func_def` | `zt_path_segment_is_dot` |
| 4799 | `func_def` | `zt_path_segment_is_dot_dot` |
| 4803 | `func_def` | `zt_path_collect_segments` |
| 4852 | `func_def` | `zt_path_normalize` |
| 4979 | `func_def` | `zt_path_is_absolute` |
| 4989 | `func_def` | `zt_path_absolute` |
| 5034 | `func_def` | `zt_path_relative` |
| 5161 | `func_def` | `zt_add_i64` |
| 5169 | `func_def` | `zt_sub_i64` |
| 5177 | `func_def` | `zt_mul_i64` |
| 5185 | `func_def` | `zt_div_i64` |
| 5195 | `func_def` | `zt_rem_i64` |
| 5206 | `func_def` | `zt_validate_between_i64` |

#### `runtime/c/zenith_rt.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_RUNTIME_C_ZENITH_RT_H` |

#### `runtime/c/zenith_rt_templates.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 17 | `macro` | `ZENITH_RT_TEMPLATES_H` |
| 22 | `macro` | `ZT_TEMPLATE_CAT_INNER` |
| 23 | `macro` | `ZT_TEMPLATE_CAT` |
| 24 | `macro` | `ZT_TEMPLATE_IF_0` |
| 25 | `macro` | `ZT_TEMPLATE_IF_1` |
| 26 | `macro` | `ZT_TEMPLATE_IF` |
| 27 | `macro` | `ZT_TEMPLATE_IF_NOT_0` |
| 28 | `macro` | `ZT_TEMPLATE_IF_NOT_1` |
| 29 | `macro` | `ZT_TEMPLATE_IF_NOT` |
| 49 | `macro` | `ZT_DEFINE_LIST_STRUCT` |
| 58 | `macro` | `ZT_DEFINE_LIST_IMPL` |
| 281 | `macro` | `ZT_DEFINE_LIST` |
| 285 | `macro` | `ZT_DEFINE_MAP_STRUCT` |
| 307 | `macro` | `ZT_DEFINE_MAP_IMPL` |
| 641 | `macro` | `ZT_DEFINE_MAP` |
| 675 | `macro` | `ZT_DEFINE_GRID2D_IMPL` |
| 880 | `macro` | `ZT_DEFINE_GRID3D_IMPL` |
| 1106 | `macro` | `ZT_DEFINE_PQUEUE_IMPL` |
| 1296 | `macro` | `ZT_DEFINE_CIRCBUF_IMPL` |
| 1465 | `macro` | `ZT_DEFINE_BTREEMAP_IMPL` |
| 1772 | `macro` | `ZT_DEFINE_BTREESET_IMPL` |
| 1995 | `macro` | `ZT_DEFINE_OPTIONAL` |
| 2039 | `macro` | `ZT_DEFINE_OPTIONAL_IMPL` |
| 2082 | `macro` | `ZT_DEFINE_OUTCOME_IMPL` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
