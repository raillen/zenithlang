# Standard Library - Code Map

## 📋 Descrição

Standard Library do Zenith. Responsável por:
- Funções utilitárias essenciais
- Collections (list, map, set, etc.)
- I/O operations (file, console, network)
- String manipulation
- Math operations
- Platform-specific functionality

## 📁 Módulos da Stdlib

| Módulo | Arquivos | Responsabilidade | Prioridade |
|--------|----------|------------------|------------|
| `core/` | 2 files | Core types, primitives | 🔴 CRÍTICA |
| `platform/` | 2 files | Platform abstraction | 🟡 MÉDIA |
| `std/` | 17 files | Standard functions | 🟡 MÉDIA |
| `zdoc/std/` | - | Documentação gerada | 🟢 BAIXA |

## 🔍 Mapeamento por Módulo

### core/
| Arquivo | Linha | Função | Responsabilidade | Pode Quebrar Se |
|---------|-------|--------|------------------|-----------------|
| - | - | - | - | - |

### platform/
| Arquivo | Linha | Função | Responsabilidade | Pode Quebrar Se |
|---------|-------|--------|------------------|-----------------|
| - | - | - | - | - |

### std/
| Arquivo | Linha | Função | Responsabilidade | Pode Quebrar Se |
|---------|-------|--------|------------------|-----------------|
| - | - | - | - | - |

## ⚠️ Estado Crítico

- **Module initialization**: setup da stdlib
- **Type definitions**: types fundamentais
- **Platform detection**: runtime platform checks

## 🔗 Dependencies Externas

- `runtime/c/` → Runtime functions
- `compiler/semantic/` → Type checking
- System libraries (libc, etc.)

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- Stdlib deve ser ESTÁVEL → mudanças quebram user code
- Boa test coverage é essencial
- Documentação (zdoc) deve ser mantida atualizada

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Medium
- Source files: 32
- Extracted symbols: 660

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `stdlib/std/bytes.zt` | 41 | 15 | 0 |
| `stdlib/std/collections.zt` | 456 | 167 | 0 |
| `stdlib/std/format.zt` | 53 | 13 | 0 |
| `stdlib/std/fs.zt` | 22 | 7 | 0 |
| `stdlib/std/fs/path.zt` | 194 | 41 | 0 |
| `stdlib/std/io.zt` | 52 | 12 | 0 |
| `stdlib/std/json.zt` | 22 | 7 | 0 |
| `stdlib/std/math.zt` | 173 | 56 | 0 |
| `stdlib/std/net.zt` | 95 | 14 | 1 |
| `stdlib/std/os.zt` | 171 | 25 | 0 |
| `stdlib/std/os/process.zt` | 47 | 10 | 0 |
| `stdlib/std/random.zt` | 34 | 9 | 0 |
| `stdlib/std/test.zt` | 21 | 7 | 0 |
| `stdlib/std/text.zt` | 16 | 5 | 0 |
| `stdlib/std/time.zt` | 82 | 20 | 0 |
| `stdlib/std/validate.zt` | 83 | 22 | 0 |
| `stdlib/zdoc/std/bytes.zdoc` | 43 | 7 | 0 |
| `stdlib/zdoc/std/collections.zdoc` | 644 | 89 | 0 |
| `stdlib/zdoc/std/format.zdoc` | 62 | 9 | 0 |
| `stdlib/zdoc/std/fs.zdoc` | 26 | 4 | 0 |
| `stdlib/zdoc/std/fs/path.zdoc` | 93 | 13 | 0 |
| `stdlib/zdoc/std/io.zdoc` | 55 | 8 | 0 |
| `stdlib/zdoc/std/json.zdoc` | 26 | 4 | 0 |
| `stdlib/zdoc/std/math.zdoc` | 228 | 34 | 0 |
| `stdlib/zdoc/std/net.zdoc` | 32 | 6 | 0 |
| `stdlib/zdoc/std/os.zdoc` | 160 | 16 | 0 |
| `stdlib/zdoc/std/os/process.zdoc` | 95 | 9 | 0 |
| `stdlib/zdoc/std/random.zdoc` | 38 | 3 | 0 |
| `stdlib/zdoc/std/test.zdoc` | 19 | 3 | 0 |
| `stdlib/zdoc/std/text.zdoc` | 14 | 2 | 0 |
| `stdlib/zdoc/std/time.zdoc` | 108 | 11 | 0 |
| `stdlib/zdoc/std/validate.zdoc` | 73 | 12 | 0 |

### Local Dependencies

- `std.fs.path`

### Related Tests

- `tests/behavior/std_bytes_ops/src/app/main.zt`
- `tests/behavior/std_bytes_utf8/src/app/main.zt`
- `tests/behavior/std_collections_basic/src/app/main.zt`
- `tests/behavior/std_collections_managed_arc/src/app/main.zt`
- `tests/behavior/std_format_basic/src/app/main.zt`
- `tests/behavior/std_fs_basic/src/app/main.zt`
- `tests/behavior/std_fs_path_basic/src/app/main.zt`
- `tests/behavior/std_io_basic/main.zt`
- `tests/behavior/std_io_basic/run.txt`
- `tests/behavior/std_io_basic/src/app/main.zt`
- `tests/behavior/std_json_basic/src/app/main.zt`
- `tests/behavior/std_math_basic/src/app/main.zt`
- `tests/behavior/std_net_basic/loopback-server.ps1`
- `tests/behavior/std_net_basic/run-loopback.ps1`
- `tests/behavior/std_net_basic/src/app/main.zt`
- `tests/behavior/std_os_basic/src/app/main.zt`
- `tests/behavior/std_os_process_basic/src/app/main.zt`
- `tests/behavior/std_random_basic/src/app/main.zt`
- `tests/behavior/std_test_attr_fail/src/app/main.zt`
- `tests/behavior/std_test_attr_fail/src/app/tests.zt`
- `tests/behavior/std_test_attr_pass_skip/src/app/main.zt`
- `tests/behavior/std_test_attr_pass_skip/src/app/tests.zt`
- `tests/behavior/std_test_basic/src/app/main.zt`
- `tests/behavior/std_time_basic/src/app/main.zt`
- `tests/behavior/std_validate_basic/src/app/main.zt`
- `tests/stdlib/test_collections.zt`
- `tests/stdlib/test_framework.zt`
- `tests/stdlib/test_fs.zt`
- `tests/stdlib/test_grid.zt`
- `tests/stdlib/test_http.zt`
- `tests/stdlib/test_http_server.zt`
- `tests/stdlib/test_integration.zt`
- `tests/stdlib/test_json.zt`
- `tests/stdlib/test_log.zt`
- `tests/stdlib/test_math.zt`
- `tests/stdlib/test_net.zt`
- `tests/stdlib/test_optional.zt`
- `tests/stdlib/test_os.zt`
- `tests/stdlib/test_prelude.zt`
- `tests/stdlib/test_process.zt`
- `tests/stdlib/test_reflect.zt`
- `tests/stdlib/test_regex.zt`
- `tests/stdlib/test_tcp_server.zt`
- `tests/stdlib/test_text.zt`
- `tests/stdlib/test_time.zt`
- `tests/stdlib/test_udp.zt`

### Symbol Index

#### `stdlib/std/bytes.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `namespace` | `std.bytes` |
| 5 | `func` | `zt_bytes_empty` |
| 6 | `func` | `zt_bytes_from_list_i64` |
| 7 | `func` | `zt_bytes_to_list_i64` |
| 8 | `func` | `zt_bytes_join` |
| 9 | `func` | `zt_bytes_starts_with` |
| 10 | `func` | `zt_bytes_ends_with` |
| 11 | `func` | `zt_bytes_contains` |
| 14 | `func` | `empty` |
| 18 | `func` | `from_list` |
| 22 | `func` | `to_list` |
| 26 | `func` | `join` |
| 30 | `func` | `starts_with` |
| 34 | `func` | `ends_with` |
| 38 | `func` | `contains` |

#### `stdlib/std/collections.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 10 | `namespace` | `std.collections` |
| 13 | `func` | `zt_queue_i64_new` |
| 14 | `func` | `zt_queue_i64_enqueue` |
| 15 | `func` | `zt_queue_i64_dequeue` |
| 16 | `func` | `zt_queue_i64_peek` |
| 18 | `func` | `zt_queue_text_new` |
| 19 | `func` | `zt_queue_text_enqueue` |
| 20 | `func` | `zt_queue_text_dequeue` |
| 21 | `func` | `zt_queue_text_peek` |
| 23 | `func` | `zt_stack_i64_new` |
| 24 | `func` | `zt_stack_i64_push` |
| 25 | `func` | `zt_stack_i64_pop` |
| 26 | `func` | `zt_stack_i64_peek` |
| 28 | `func` | `zt_stack_text_new` |
| 29 | `func` | `zt_stack_text_push` |
| 30 | `func` | `zt_stack_text_pop` |
| 31 | `func` | `zt_stack_text_peek` |
| 33 | `func` | `zt_grid2d_i64_new` |
| 34 | `func` | `zt_grid2d_i64_get` |
| 35 | `func` | `zt_grid2d_i64_set_owned` |
| 36 | `func` | `zt_grid2d_i64_fill_owned` |
| 37 | `func` | `zt_grid2d_i64_rows` |
| 38 | `func` | `zt_grid2d_i64_cols` |
| 40 | `func` | `zt_grid2d_text_new` |
| 41 | `func` | `zt_grid2d_text_get` |
| 42 | `func` | `zt_grid2d_text_set_owned` |
| 43 | `func` | `zt_grid2d_text_fill_owned` |
| 44 | `func` | `zt_grid2d_text_rows` |
| 45 | `func` | `zt_grid2d_text_cols` |
| 47 | `func` | `zt_pqueue_i64_new` |
| 48 | `func` | `zt_pqueue_i64_push_owned` |
| 49 | `func` | `zt_pqueue_i64_pop` |
| 50 | `func` | `zt_pqueue_i64_peek` |
| 51 | `func` | `zt_pqueue_i64_len` |
| 53 | `func` | `zt_pqueue_text_new` |
| 54 | `func` | `zt_pqueue_text_push_owned` |
| 55 | `func` | `zt_pqueue_text_pop` |
| 56 | `func` | `zt_pqueue_text_peek` |
| 57 | `func` | `zt_pqueue_text_len` |
| 59 | `func` | `zt_circbuf_i64_new` |
| 60 | `func` | `zt_circbuf_i64_push_owned` |
| 61 | `func` | `zt_circbuf_i64_pop` |
| 62 | `func` | `zt_circbuf_i64_peek` |
| 63 | `func` | `zt_circbuf_i64_len` |
| 64 | `func` | `zt_circbuf_i64_capacity` |
| 65 | `func` | `zt_circbuf_i64_is_full` |
| 67 | `func` | `zt_circbuf_text_new` |
| 68 | `func` | `zt_circbuf_text_push_owned` |
| 69 | `func` | `zt_circbuf_text_pop` |
| 70 | `func` | `zt_circbuf_text_peek` |
| 71 | `func` | `zt_circbuf_text_len` |
| 72 | `func` | `zt_circbuf_text_capacity` |
| 73 | `func` | `zt_circbuf_text_is_full` |
| 75 | `func` | `zt_btreemap_text_text_new` |
| 76 | `func` | `zt_btreemap_text_text_set_owned` |
| 77 | `func` | `zt_btreemap_text_text_get` |
| 78 | `func` | `zt_btreemap_text_text_get_optional` |
| 79 | `func` | `zt_btreemap_text_text_contains` |
| 80 | `func` | `zt_btreemap_text_text_remove_owned` |
| 81 | `func` | `zt_btreemap_text_text_len` |
| 83 | `func` | `zt_btreeset_text_new` |
| 84 | `func` | `zt_btreeset_text_insert_owned` |
| 85 | `func` | `zt_btreeset_text_contains` |
| 86 | `func` | `zt_btreeset_text_remove_owned` |
| 87 | `func` | `zt_btreeset_text_len` |
| 89 | `func` | `zt_grid3d_i64_new` |
| 90 | `func` | `zt_grid3d_i64_get` |
| 91 | `func` | `zt_grid3d_i64_set_owned` |
| 92 | `func` | `zt_grid3d_i64_fill_owned` |
| 93 | `func` | `zt_grid3d_i64_depth` |
| 94 | `func` | `zt_grid3d_i64_rows` |
| 95 | `func` | `zt_grid3d_i64_cols` |
| 97 | `func` | `zt_grid3d_text_new` |
| 98 | `func` | `zt_grid3d_text_get` |
| 99 | `func` | `zt_grid3d_text_set_owned` |
| 100 | `func` | `zt_grid3d_text_fill_owned` |
| 101 | `func` | `zt_grid3d_text_depth` |
| 102 | `func` | `zt_grid3d_text_rows` |
| 103 | `func` | `zt_grid3d_text_cols` |
| 106 | `func` | `queue_int_new` |
| 110 | `func` | `queue_int_enqueue` |
| 114 | `func` | `queue_int_dequeue` |
| 118 | `func` | `queue_int_peek` |
| 122 | `func` | `queue_text_new` |
| 126 | `func` | `queue_text_enqueue` |
| 130 | `func` | `queue_text_dequeue` |
| 134 | `func` | `queue_text_peek` |
| 138 | `func` | `stack_int_new` |
| 142 | `func` | `stack_int_push` |
| 146 | `func` | `stack_int_pop` |
| 150 | `func` | `stack_int_peek` |
| 154 | `func` | `stack_text_new` |
| 158 | `func` | `stack_text_push` |
| 162 | `func` | `stack_text_pop` |
| 166 | `func` | `stack_text_peek` |
| 170 | `func` | `grid2d_int_new` |
| 174 | `func` | `grid2d_int_get` |
| 178 | `func` | `grid2d_int_set` |
| 182 | `func` | `grid2d_int_fill` |
| 186 | `func` | `grid2d_int_rows` |
| 190 | `func` | `grid2d_int_cols` |
| 194 | `func` | `grid2d_text_new` |
| 198 | `func` | `grid2d_text_get` |
| 202 | `func` | `grid2d_text_set` |
| 206 | `func` | `grid2d_text_fill` |
| 210 | `func` | `grid2d_text_rows` |
| 214 | `func` | `grid2d_text_cols` |
| 218 | `func` | `pqueue_int_new` |
| 222 | `func` | `pqueue_int_push` |
| 226 | `func` | `pqueue_int_pop` |
| 230 | `func` | `pqueue_int_peek` |
| 234 | `func` | `pqueue_int_len` |
| 238 | `func` | `pqueue_text_new` |
| 242 | `func` | `pqueue_text_push` |
| 246 | `func` | `pqueue_text_pop` |
| 250 | `func` | `pqueue_text_peek` |
| 254 | `func` | `pqueue_text_len` |
| 258 | `func` | `circbuf_int_new` |
| 262 | `func` | `circbuf_int_push` |
| 266 | `func` | `circbuf_int_pop` |
| 270 | `func` | `circbuf_int_peek` |
| 274 | `func` | `circbuf_int_len` |
| 278 | `func` | `circbuf_int_capacity` |
| 282 | `func` | `circbuf_int_is_full` |
| 286 | `func` | `circbuf_text_new` |
| 290 | `func` | `circbuf_text_push` |
| 294 | `func` | `circbuf_text_pop` |
| 298 | `func` | `circbuf_text_peek` |
| 302 | `func` | `circbuf_text_len` |
| 306 | `func` | `circbuf_text_capacity` |
| 310 | `func` | `circbuf_text_is_full` |
| 314 | `func` | `btreemap_text_new` |
| 318 | `func` | `btreemap_text_set` |
| 322 | `func` | `btreemap_text_get` |
| 326 | `func` | `btreemap_text_get_optional` |
| 330 | `func` | `btreemap_text_contains` |
| 334 | `func` | `btreemap_text_remove` |
| 338 | `func` | `btreemap_text_len` |
| 342 | `func` | `btreeset_text_new` |
| 346 | `func` | `btreeset_text_insert` |
| 350 | `func` | `btreeset_text_contains` |
| 354 | `func` | `btreeset_text_remove` |
| 358 | `func` | `btreeset_text_len` |
| 362 | `func` | `grid3d_int_new` |
| 366 | `func` | `grid3d_int_get` |
| 370 | `func` | `grid3d_int_set` |
| 374 | `func` | `grid3d_int_fill` |
| 378 | `func` | `grid3d_int_depth` |
| 382 | `func` | `grid3d_int_rows` |
| 386 | `func` | `grid3d_int_cols` |
| 390 | `func` | `grid3d_text_new` |
| 394 | `func` | `grid3d_text_get` |
| 398 | `func` | `grid3d_text_set` |
| 402 | `func` | `grid3d_text_fill` |
| 406 | `func` | `grid3d_text_depth` |
| 410 | `func` | `grid3d_text_rows` |
| 414 | `func` | `grid3d_text_cols` |
| 418 | `func` | `pqueue_int_is_empty` |
| 422 | `func` | `pqueue_text_is_empty` |
| 426 | `func` | `circbuf_int_is_empty` |
| 430 | `func` | `circbuf_text_is_empty` |
| 434 | `func` | `btreemap_text_is_empty` |
| 438 | `func` | `btreeset_text_is_empty` |
| 442 | `func` | `grid2d_int_size` |
| 446 | `func` | `grid2d_text_size` |
| 450 | `func` | `grid3d_int_size` |
| 454 | `func` | `grid3d_text_size` |

#### `stdlib/std/format.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 4 | `namespace` | `std.format` |
| 6 | `enum` | `BytesStyle` |
| 12 | `func` | `zt_format_hex_i64` |
| 13 | `func` | `zt_format_bin_i64` |
| 14 | `func` | `zt_format_bytes_binary` |
| 15 | `func` | `zt_format_bytes_decimal` |
| 18 | `func` | `style_binary` |
| 22 | `func` | `style_decimal` |
| 26 | `func` | `hex` |
| 30 | `func` | `bin` |
| 34 | `func` | `bytes` |
| 45 | `func` | `bytes_binary` |
| 49 | `func` | `bytes_decimal` |

#### `stdlib/std/fs.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 4 | `namespace` | `std.fs` |
| 7 | `func` | `zt_host_read_file` |
| 8 | `func` | `zt_host_write_file` |
| 9 | `func` | `zt_host_path_exists` |
| 12 | `func` | `read_text` |
| 16 | `func` | `write_text` |
| 20 | `func` | `exists` |

#### `stdlib/std/fs/path.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 6 | `namespace` | `std.fs.path` |
| 9 | `func` | `zt_text_concat` |
| 10 | `func` | `zt_text_eq` |
| 11 | `func` | `zt_path_normalize` |
| 12 | `func` | `zt_path_is_absolute` |
| 13 | `func` | `zt_path_absolute` |
| 14 | `func` | `zt_path_relative` |
| 17 | `func` | `_text_eq` |
| 21 | `func` | `_last_index_of` |
| 34 | `func` | `join` |
| 43 | `const` | `slash` |
| 44 | `const` | `last_base` |
| 49 | `const` | `with_sep` |
| 53 | `func` | `normalize` |
| 57 | `func` | `is_absolute` |
| 61 | `func` | `is_relative` |
| 68 | `func` | `absolute` |
| 72 | `func` | `relative` |
| 76 | `func` | `base` |
| 81 | `const` | `slash` |
| 82 | `const` | `slash_index` |
| 94 | `func` | `dir` |
| 99 | `const` | `slash` |
| 100 | `const` | `slash_index` |
| 112 | `func` | `ext` |
| 113 | `const` | `file_name` |
| 118 | `const` | `dot` |
| 119 | `const` | `dot_index` |
| 127 | `func` | `name_without_extension` |
| 128 | `const` | `file_name` |
| 133 | `const` | `dot` |
| 134 | `const` | `dot_index` |
| 142 | `func` | `has_ext` |
| 144 | `const` | `dot` |
| 157 | `func` | `change_ext` |
| 158 | `const` | `file_name` |
| 163 | `const` | `directory` |
| 164 | `const` | `stem` |
| 165 | `const` | `dot` |
| 166 | `const` | `root` |
| 179 | `const` | `stem_with_dot` |

#### `stdlib/std/io.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 6 | `namespace` | `std.io` |
| 9 | `func` | `zt_host_read_line_stdin` |
| 10 | `func` | `zt_host_read_all_stdin` |
| 11 | `func` | `zt_host_write_stdout` |
| 12 | `func` | `zt_host_write_stderr` |
| 15 | `func` | `read_line` |
| 22 | `func` | `read_all` |
| 29 | `func` | `write` |
| 36 | `func` | `print` |
| 40 | `func` | `print_line` |
| 45 | `func` | `eprint` |
| 49 | `func` | `eprint_line` |

#### `stdlib/std/json.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 4 | `namespace` | `std.json` |
| 7 | `func` | `zt_json_parse_map_text_text` |
| 8 | `func` | `zt_json_stringify_map_text_text` |
| 9 | `func` | `zt_json_pretty_map_text_text` |
| 12 | `func` | `parse` |
| 16 | `func` | `stringify` |
| 20 | `func` | `pretty` |

#### `stdlib/std/math.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `namespace` | `std.math` |
| 5 | `func` | `zt_math_pow` |
| 6 | `func` | `zt_math_sqrt` |
| 7 | `func` | `zt_math_floor` |
| 8 | `func` | `zt_math_ceil` |
| 9 | `func` | `zt_math_round_half_away_from_zero` |
| 10 | `func` | `zt_math_trunc` |
| 11 | `func` | `zt_math_sin` |
| 12 | `func` | `zt_math_cos` |
| 13 | `func` | `zt_math_tan` |
| 14 | `func` | `zt_math_asin` |
| 15 | `func` | `zt_math_acos` |
| 16 | `func` | `zt_math_atan` |
| 17 | `func` | `zt_math_atan2` |
| 18 | `func` | `zt_math_ln` |
| 19 | `func` | `zt_math_log_ten` |
| 20 | `func` | `zt_math_log2` |
| 21 | `func` | `zt_math_log` |
| 22 | `func` | `zt_math_exp` |
| 23 | `func` | `zt_math_is_nan` |
| 24 | `func` | `zt_math_is_infinite` |
| 25 | `func` | `zt_math_is_finite` |
| 28 | `const` | `PI` |
| 29 | `const` | `E` |
| 30 | `const` | `TAU` |
| 32 | `func` | `pi` |
| 36 | `func` | `e` |
| 40 | `func` | `tau` |
| 44 | `func` | `abs` |
| 51 | `func` | `min` |
| 58 | `func` | `max` |
| 65 | `func` | `clamp` |
| 75 | `func` | `pow` |
| 79 | `func` | `sqrt` |
| 83 | `func` | `floor` |
| 87 | `func` | `ceil` |
| 91 | `func` | `round` |
| 95 | `func` | `trunc` |
| 99 | `func` | `deg_to_rad` |
| 103 | `func` | `rad_to_deg` |
| 107 | `func` | `approx_equal` |
| 115 | `func` | `sin` |
| 119 | `func` | `cos` |
| 123 | `func` | `tan` |
| 127 | `func` | `asin` |
| 131 | `func` | `acos` |
| 135 | `func` | `atan` |
| 139 | `func` | `atan2` |
| 143 | `func` | `ln` |
| 147 | `func` | `log_ten` |
| 151 | `func` | `log2` |
| 155 | `func` | `log` |
| 159 | `func` | `exp` |
| 163 | `func` | `is_nan` |
| 167 | `func` | `is_infinite` |
| 171 | `func` | `is_finite` |

#### `stdlib/std/net.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `namespace` | `std.net` |
| 5 | `enum` | `Error` |
| 19 | `struct` | `Connection` |
| 23 | `func` | `zt_net_connect` |
| 24 | `func` | `zt_net_read_some` |
| 25 | `func` | `zt_net_write_all` |
| 26 | `func` | `zt_net_close` |
| 27 | `func` | `zt_net_is_closed` |
| 30 | `func` | `connect` |
| 38 | `func` | `read_some` |
| 46 | `func` | `write_all` |
| 54 | `func` | `close` |
| 58 | `func` | `is_closed` |
| 62 | `func` | `kind` |

#### `stdlib/std/os.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 8 | `namespace` | `std.os` |
| 10 | `enum` | `Platform` |
| 17 | `enum` | `Arch` |
| 24 | `enum` | `Error` |
| 34 | `func` | `zt_host_os_current_dir` |
| 35 | `func` | `zt_host_os_change_dir` |
| 36 | `func` | `zt_host_os_env` |
| 37 | `func` | `zt_host_os_pid` |
| 38 | `func` | `zt_host_os_platform` |
| 39 | `func` | `zt_host_os_arch` |
| 42 | `func` | `current_dir` |
| 46 | `func` | `change_dir` |
| 50 | `func` | `env` |
| 54 | `func` | `pid` |
| 58 | `func` | `platform` |
| 59 | `const` | `raw` |
| 72 | `func` | `arch` |
| 73 | `const` | `raw` |
| 86 | `func` | `platform_text` |
| 90 | `func` | `arch_text` |
| 94 | `func` | `is_platform` |
| 112 | `func` | `is_arch` |
| 145 | `func` | `is_windows` |
| 154 | `func` | `is_linux` |
| 163 | `func` | `is_macos` |

#### `stdlib/std/os/process.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 6 | `namespace` | `std.os.process` |
| 8 | `struct` | `ExitStatus` |
| 12 | `enum` | `Error` |
| 21 | `func` | `zt_host_process_run` |
| 24 | `func` | `run` |
| 28 | `func` | `run_program` |
| 32 | `func` | `from_code` |
| 36 | `func` | `exit_code` |
| 40 | `func` | `is_success` |
| 44 | `func` | `is_failure` |

#### `stdlib/std/random.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 4 | `namespace` | `std.random` |
| 7 | `func` | `zt_host_random_seed` |
| 8 | `func` | `zt_host_random_next_i64` |
| 11 | `func` | `seed` |
| 15 | `func` | `next` |
| 19 | `func` | `between` |
| 28 | `const` | `span` |
| 29 | `const` | `raw` |
| 30 | `const` | `offset` |

#### `stdlib/std/test.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 3 | `namespace` | `std.test` |
| 5 | `func` | `zt_test_fail` |
| 6 | `const` | `_message` |
| 10 | `func` | `zt_test_skip` |
| 11 | `const` | `_reason` |
| 15 | `func` | `fail` |
| 19 | `func` | `skip` |

#### `stdlib/std/text.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `namespace` | `std.text` |
| 5 | `func` | `zt_text_to_utf8_bytes` |
| 6 | `func` | `zt_text_from_utf8_bytes` |
| 9 | `func` | `to_utf8` |
| 13 | `func` | `from_utf8` |

#### `stdlib/std/time.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 7 | `namespace` | `std.time` |
| 9 | `struct` | `Instant` |
| 13 | `struct` | `Duration` |
| 18 | `func` | `zt_host_time_now_unix_ms` |
| 19 | `func` | `zt_host_time_sleep_ms` |
| 22 | `func` | `now` |
| 26 | `func` | `sleep` |
| 30 | `func` | `since` |
| 34 | `func` | `until` |
| 38 | `func` | `diff` |
| 42 | `func` | `add` |
| 46 | `func` | `sub` |
| 50 | `func` | `from_unix` |
| 54 | `func` | `from_unix_ms` |
| 58 | `func` | `to_unix` |
| 62 | `func` | `to_unix_ms` |
| 66 | `func` | `milliseconds` |
| 70 | `func` | `seconds` |
| 74 | `func` | `minutes` |
| 78 | `func` | `hours` |

#### `stdlib/std/validate.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 5 | `namespace` | `std.validate` |
| 7 | `func` | `between` |
| 11 | `func` | `positive` |
| 15 | `func` | `non_negative` |
| 19 | `func` | `negative` |
| 23 | `func` | `non_zero` |
| 27 | `func` | `one_of` |
| 36 | `func` | `not_empty` |
| 40 | `func` | `min_length` |
| 44 | `func` | `max_length` |
| 48 | `func` | `length_between` |
| 49 | `const` | `count` |
| 53 | `func` | `no_whitespace` |
| 54 | `const` | `space` |
| 55 | `const` | `tab` |
| 56 | `const` | `newline` |
| 57 | `const` | `carriage` |
| 68 | `func` | `has_whitespace` |
| 69 | `const` | `space` |
| 70 | `const` | `tab` |
| 71 | `const` | `newline` |
| 72 | `const` | `carriage` |

#### `stdlib/zdoc/std/bytes.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `empty` |
| 7 | `doc_target` | `from_list` |
| 13 | `doc_target` | `to_list` |
| 19 | `doc_target` | `join` |
| 25 | `doc_target` | `starts_with` |
| 31 | `doc_target` | `ends_with` |
| 37 | `doc_target` | `contains` |

#### `stdlib/zdoc/std/collections.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `std.collections` |
| 5 | `doc_target` | `queue_int_new` |
| 10 | `doc_target` | `queue_int_enqueue` |
| 17 | `doc_target` | `queue_int_dequeue` |
| 23 | `doc_target` | `queue_int_peek` |
| 29 | `doc_target` | `queue_text_new` |
| 34 | `doc_target` | `queue_text_enqueue` |
| 41 | `doc_target` | `queue_text_dequeue` |
| 47 | `doc_target` | `queue_text_peek` |
| 53 | `doc_target` | `stack_int_new` |
| 58 | `doc_target` | `stack_int_push` |
| 65 | `doc_target` | `stack_int_pop` |
| 71 | `doc_target` | `stack_int_peek` |
| 77 | `doc_target` | `stack_text_new` |
| 82 | `doc_target` | `stack_text_push` |
| 89 | `doc_target` | `stack_text_pop` |
| 95 | `doc_target` | `stack_text_peek` |
| 101 | `doc_target` | `grid2d_int_new` |
| 109 | `doc_target` | `grid2d_int_get` |
| 118 | `doc_target` | `grid2d_int_set` |
| 128 | `doc_target` | `grid2d_int_fill` |
| 136 | `doc_target` | `grid2d_int_rows` |
| 143 | `doc_target` | `grid2d_int_cols` |
| 150 | `doc_target` | `grid2d_text_new` |
| 158 | `doc_target` | `grid2d_text_get` |
| 167 | `doc_target` | `grid2d_text_set` |
| 177 | `doc_target` | `grid2d_text_fill` |
| 185 | `doc_target` | `grid2d_text_rows` |
| 192 | `doc_target` | `grid2d_text_cols` |
| 199 | `doc_target` | `pqueue_int_new` |
| 204 | `doc_target` | `pqueue_int_push` |
| 212 | `doc_target` | `pqueue_int_pop` |
| 219 | `doc_target` | `pqueue_int_peek` |
| 226 | `doc_target` | `pqueue_int_len` |
| 233 | `doc_target` | `pqueue_text_new` |
| 238 | `doc_target` | `pqueue_text_push` |
| 246 | `doc_target` | `pqueue_text_pop` |
| 253 | `doc_target` | `pqueue_text_peek` |
| 260 | `doc_target` | `pqueue_text_len` |
| 267 | `doc_target` | `circbuf_int_new` |
| 274 | `doc_target` | `circbuf_int_push` |
| 282 | `doc_target` | `circbuf_int_pop` |
| 289 | `doc_target` | `circbuf_int_peek` |
| 296 | `doc_target` | `circbuf_int_len` |
| 303 | `doc_target` | `circbuf_int_capacity` |
| 310 | `doc_target` | `circbuf_int_is_full` |
| 317 | `doc_target` | `circbuf_text_new` |
| 324 | `doc_target` | `circbuf_text_push` |
| 332 | `doc_target` | `circbuf_text_pop` |
| 339 | `doc_target` | `circbuf_text_peek` |
| 346 | `doc_target` | `circbuf_text_len` |
| 353 | `doc_target` | `circbuf_text_capacity` |
| 360 | `doc_target` | `circbuf_text_is_full` |
| 367 | `doc_target` | `btreemap_text_new` |
| 373 | `doc_target` | `btreemap_text_set` |
| 382 | `doc_target` | `btreemap_text_get` |
| 390 | `doc_target` | `btreemap_text_get_optional` |
| 398 | `doc_target` | `btreemap_text_contains` |
| 406 | `doc_target` | `btreemap_text_remove` |
| 414 | `doc_target` | `btreemap_text_len` |
| 421 | `doc_target` | `btreeset_text_new` |
| 426 | `doc_target` | `btreeset_text_insert` |
| 434 | `doc_target` | `btreeset_text_contains` |
| 442 | `doc_target` | `btreeset_text_remove` |
| 450 | `doc_target` | `btreeset_text_len` |
| 457 | `doc_target` | `grid3d_int_new` |
| 466 | `doc_target` | `grid3d_int_get` |
| 476 | `doc_target` | `grid3d_int_set` |
| 487 | `doc_target` | `grid3d_int_fill` |
| 495 | `doc_target` | `grid3d_int_depth` |
| 502 | `doc_target` | `grid3d_int_rows` |
| 509 | `doc_target` | `grid3d_int_cols` |
| 516 | `doc_target` | `grid3d_text_new` |
| 525 | `doc_target` | `grid3d_text_get` |
| 535 | `doc_target` | `grid3d_text_set` |
| 546 | `doc_target` | `grid3d_text_fill` |
| 554 | `doc_target` | `grid3d_text_depth` |
| 561 | `doc_target` | `grid3d_text_rows` |
| 568 | `doc_target` | `grid3d_text_cols` |
| 575 | `doc_target` | `pqueue_int_is_empty` |
| 582 | `doc_target` | `pqueue_text_is_empty` |
| 589 | `doc_target` | `circbuf_int_is_empty` |
| 596 | `doc_target` | `circbuf_text_is_empty` |
| 603 | `doc_target` | `btreemap_text_is_empty` |
| 610 | `doc_target` | `btreeset_text_is_empty` |
| 617 | `doc_target` | `grid2d_int_size` |
| 624 | `doc_target` | `grid2d_text_size` |
| 631 | `doc_target` | `grid3d_int_size` |
| 638 | `doc_target` | `grid3d_text_size` |

#### `stdlib/zdoc/std/format.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `std.format` |
| 5 | `doc_target` | `BytesStyle` |
| 12 | `doc_target` | `style_binary` |
| 18 | `doc_target` | `style_decimal` |
| 24 | `doc_target` | `hex` |
| 31 | `doc_target` | `bin` |
| 38 | `doc_target` | `bytes` |
| 47 | `doc_target` | `bytes_binary` |
| 55 | `doc_target` | `bytes_decimal` |

#### `stdlib/zdoc/std/fs.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `std.fs` |
| 5 | `doc_target` | `read_text` |
| 12 | `doc_target` | `write_text` |
| 20 | `doc_target` | `exists` |

#### `stdlib/zdoc/std/fs/path.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `std.fs.path` |
| 5 | `doc_target` | `join` |
| 13 | `doc_target` | `normalize` |
| 20 | `doc_target` | `is_absolute` |
| 27 | `doc_target` | `is_relative` |
| 34 | `doc_target` | `absolute` |
| 42 | `doc_target` | `relative` |
| 50 | `doc_target` | `base` |
| 57 | `doc_target` | `dir` |
| 64 | `doc_target` | `ext` |
| 71 | `doc_target` | `name_without_extension` |
| 78 | `doc_target` | `has_ext` |
| 86 | `doc_target` | `change_ext` |

#### `stdlib/zdoc/std/io.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `std.io` |
| 5 | `doc_target` | `read_line` |
| 12 | `doc_target` | `read_all` |
| 19 | `doc_target` | `write` |
| 27 | `doc_target` | `print` |
| 35 | `doc_target` | `print_line` |
| 43 | `doc_target` | `eprint` |
| 50 | `doc_target` | `eprint_line` |

#### `stdlib/zdoc/std/json.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `std.json` |
| 5 | `doc_target` | `parse` |
| 12 | `doc_target` | `stringify` |
| 19 | `doc_target` | `pretty` |

#### `stdlib/zdoc/std/math.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `PI` |
| 5 | `doc_target` | `E` |
| 9 | `doc_target` | `TAU` |
| 13 | `doc_target` | `pi` |
| 17 | `doc_target` | `e` |
| 21 | `doc_target` | `tau` |
| 25 | `doc_target` | `abs` |
| 32 | `doc_target` | `min` |
| 40 | `doc_target` | `max` |
| 48 | `doc_target` | `clamp` |
| 57 | `doc_target` | `pow` |
| 65 | `doc_target` | `sqrt` |
| 72 | `doc_target` | `floor` |
| 79 | `doc_target` | `ceil` |
| 86 | `doc_target` | `round` |
| 93 | `doc_target` | `trunc` |
| 100 | `doc_target` | `deg_to_rad` |
| 107 | `doc_target` | `rad_to_deg` |
| 114 | `doc_target` | `approx_equal` |
| 123 | `doc_target` | `sin` |
| 130 | `doc_target` | `cos` |
| 137 | `doc_target` | `tan` |
| 144 | `doc_target` | `asin` |
| 151 | `doc_target` | `acos` |
| 158 | `doc_target` | `atan` |
| 165 | `doc_target` | `atan2` |
| 173 | `doc_target` | `ln` |
| 180 | `doc_target` | `log_ten` |
| 187 | `doc_target` | `log2` |
| 194 | `doc_target` | `log` |
| 202 | `doc_target` | `exp` |
| 209 | `doc_target` | `is_nan` |
| 216 | `doc_target` | `is_infinite` |
| 223 | `doc_target` | `is_finite` |

#### `stdlib/zdoc/std/net.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `connect` |
| 6 | `doc_target` | `read_some` |
| 11 | `doc_target` | `write_all` |
| 16 | `doc_target` | `close` |
| 21 | `doc_target` | `is_closed` |
| 26 | `doc_target` | `kind` |

#### `stdlib/zdoc/std/os.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `current_dir` |
| 10 | `doc_target` | `change_dir` |
| 21 | `doc_target` | `env` |
| 35 | `doc_target` | `pid` |
| 44 | `doc_target` | `platform` |
| 53 | `doc_target` | `arch` |
| 62 | `doc_target` | `platform_text` |
| 71 | `doc_target` | `arch_text` |
| 80 | `doc_target` | `is_platform` |
| 92 | `doc_target` | `is_arch` |
| 104 | `doc_target` | `is_windows` |
| 113 | `doc_target` | `is_linux` |
| 122 | `doc_target` | `is_macos` |
| 131 | `doc_target` | `os.Error` |
| 142 | `doc_target` | `os.Platform` |
| 151 | `doc_target` | `os.Arch` |

#### `stdlib/zdoc/std/os/process.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `std.os.process` |
| 5 | `doc_target` | `run` |
| 19 | `doc_target` | `run_program` |
| 31 | `doc_target` | `from_code` |
| 43 | `doc_target` | `exit_code` |
| 55 | `doc_target` | `is_success` |
| 67 | `doc_target` | `is_failure` |
| 79 | `doc_target` | `process.ExitStatus` |
| 85 | `doc_target` | `process.Error` |

#### `stdlib/zdoc/std/random.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `seed` |
| 12 | `doc_target` | `next` |
| 23 | `doc_target` | `between` |

#### `stdlib/zdoc/std/test.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `std.test` |
| 6 | `doc_target` | `fail` |
| 13 | `doc_target` | `skip` |

#### `stdlib/zdoc/std/text.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `to_utf8` |
| 7 | `doc_target` | `from_utf8` |

#### `stdlib/zdoc/std/time.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `now` |
| 10 | `doc_target` | `sleep` |
| 24 | `doc_target` | `since` |
| 36 | `doc_target` | `until` |
| 48 | `doc_target` | `diff` |
| 61 | `doc_target` | `add` |
| 71 | `doc_target` | `sub` |
| 81 | `doc_target` | `seconds` |
| 90 | `doc_target` | `minutes` |
| 96 | `doc_target` | `from_unix` |
| 102 | `doc_target` | `to_unix` |

#### `stdlib/zdoc/std/validate.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `between` |
| 7 | `doc_target` | `positive` |
| 13 | `doc_target` | `non_negative` |
| 19 | `doc_target` | `negative` |
| 25 | `doc_target` | `non_zero` |
| 31 | `doc_target` | `one_of` |
| 37 | `doc_target` | `not_empty` |
| 43 | `doc_target` | `min_length` |
| 49 | `doc_target` | `max_length` |
| 55 | `doc_target` | `length_between` |
| 61 | `doc_target` | `no_whitespace` |
| 67 | `doc_target` | `has_whitespace` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
