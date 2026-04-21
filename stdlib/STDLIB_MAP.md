# Standard Library - Code Map

## Descricao

Mapa da stdlib do Zenith.

Objetivos:
- Deixar ownership claro entre `core`, `std.*` e `platform`.
- Evitar sobreposicao de API.
- Facilitar manutencao e onboarding.

## Modulos da Stdlib

| Modulo | Arquivos | Papel | Status |
|--------|----------|-------|--------|
| `core/` | 2 files | Documentacao e contratos internos do core implicito | Placeholder interno |
| `platform/` | 2 files | Adaptacao interna host/target para suportar `std.*` | Placeholder interno |
| `std/` | 17 files | API publica da stdlib | Ativo |
| `zdoc/std/` | - | Documentacao gerada dos modulos publicos | Ativo |

## Contrato de Ownership

- `core.*` e implicito e pertence ao nucleo da linguagem.
- API publica fica em `std.*`.
- `platform/` e interno e nao e API publica para app.

Dono de API publica:
- `std.os`: estado do processo atual, env, pid, plataforma e arquitetura.
- `std.os.process`: execucao e controle de processos filhos.

Nao permitido:
- duplicar em `platform/` APIs ja definidas em `std.os` ou `std.os.process`.

## Mapeamento por Modulo

### core/
| Arquivo | Linha | Funcao | Responsabilidade | Pode quebrar se |
|---------|-------|--------|------------------|-----------------|
| `stdlib/core/README.md` | - | contrato de camada | delimitar uso interno | virar API publica por engano |

### platform/
| Arquivo | Linha | Funcao | Responsabilidade | Pode quebrar se |
|---------|-------|--------|------------------|-----------------|
| `stdlib/platform/README.md` | - | contrato de camada | evitar conflito com `std.os*` | surgir duplicacao de ownership |

### std/
| Arquivo | Linha | Funcao | Responsabilidade | Pode quebrar se |
|---------|-------|--------|------------------|-----------------|
| `stdlib/std/os.zt` | - | `std.os` | estado do processo atual | mover API para `platform/` |
| `stdlib/std/os/process.zt` | - | `std.os.process` | processos filhos | duplicar run/spawn em outra camada |

## Estado Atual

- `core/` e `platform/` estao reservados por design.
- A superficie publica consolidada esta em `std.*`.
- O contrato formal foi fechado em `Decision 085`.

## Dependencias Externas

- `runtime/c/` -> wrappers de host e runtime
- `compiler/semantic/` -> regras de ownership e imports
- system libraries (libc e equivalentes)

## Erros Comuns

1. Tratar `platform/` como API publica.
2. Duplicar funcoes de `std.os` em camadas internas.
3. Mudar ownership sem atualizar as decisions.

## Notas de Manutencao

- Antes de abrir `platform/` de verdade, aplicar os gates da Decision 085.
- Mudancas de ownership devem atualizar docs, tests e code map.
- Manter linguagem objetiva e exemplos curtos.
<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Medium
- Source files: 32
- Extracted symbols: 841

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `stdlib/std/bytes.zt` | 41 | 15 | 0 |
| `stdlib/std/collections.zt` | 456 | 167 | 0 |
| `stdlib/std/format.zt` | 70 | 21 | 0 |
| `stdlib/std/fs.zt` | 124 | 40 | 0 |
| `stdlib/std/fs/path.zt` | 205 | 43 | 0 |
| `stdlib/std/io.zt` | 52 | 15 | 0 |
| `stdlib/std/json.zt` | 111 | 34 | 0 |
| `stdlib/std/math.zt` | 179 | 57 | 0 |
| `stdlib/std/net.zt` | 95 | 14 | 1 |
| `stdlib/std/os.zt` | 88 | 20 | 0 |
| `stdlib/std/os/process.zt` | 38 | 8 | 0 |
| `stdlib/std/random.zt` | 34 | 9 | 0 |
| `stdlib/std/test.zt` | 21 | 7 | 0 |
| `stdlib/std/text.zt` | 179 | 58 | 0 |
| `stdlib/std/time.zt` | 88 | 21 | 0 |
| `stdlib/std/validate.zt` | 52 | 12 | 0 |
| `stdlib/zdoc/std/bytes.zdoc` | 89 | 8 | 0 |
| `stdlib/zdoc/std/collections.zdoc` | 828 | 88 | 0 |
| `stdlib/zdoc/std/format.zdoc` | 130 | 11 | 0 |
| `stdlib/zdoc/std/fs.zdoc` | 209 | 21 | 0 |
| `stdlib/zdoc/std/fs/path.zdoc` | 131 | 13 | 0 |
| `stdlib/zdoc/std/io.zdoc` | 117 | 11 | 0 |
| `stdlib/zdoc/std/json.zdoc` | 197 | 20 | 0 |
| `stdlib/zdoc/std/math.zdoc` | 292 | 34 | 0 |
| `stdlib/zdoc/std/net.zdoc` | 125 | 9 | 0 |
| `stdlib/zdoc/std/os.zdoc` | 111 | 11 | 0 |
| `stdlib/zdoc/std/os/process.zdoc` | 73 | 6 | 0 |
| `stdlib/zdoc/std/random.zdoc` | 41 | 4 | 0 |
| `stdlib/zdoc/std/test.zdoc` | 31 | 3 | 0 |
| `stdlib/zdoc/std/text.zdoc` | 357 | 31 | 0 |
| `stdlib/zdoc/std/time.zdoc` | 162 | 19 | 0 |
| `stdlib/zdoc/std/validate.zdoc` | 109 | 11 | 0 |

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
| 8 | `namespace` | `std.format` |
| 10 | `enum` | `BytesStyle` |
| 16 | `func` | `zt_format_number` |
| 17 | `func` | `zt_format_percent` |
| 18 | `func` | `zt_format_date` |
| 19 | `func` | `zt_format_datetime` |
| 20 | `func` | `zt_format_date_pattern` |
| 21 | `func` | `zt_format_datetime_pattern` |
| 22 | `func` | `zt_format_hex_i64` |
| 23 | `func` | `zt_format_bin_i64` |
| 24 | `func` | `zt_format_bytes_binary` |
| 25 | `func` | `zt_format_bytes_decimal` |
| 28 | `func` | `number` |
| 32 | `func` | `percent` |
| 36 | `func` | `date` |
| 40 | `func` | `datetime` |
| 44 | `func` | `date_pattern` |
| 48 | `func` | `datetime_pattern` |
| 52 | `func` | `bytes` |
| 63 | `func` | `hex` |
| 67 | `func` | `bin` |

#### `stdlib/std/fs.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 11 | `namespace` | `std.fs` |
| 13 | `enum` | `Error` |
| 24 | `struct` | `Metadata` |
| 33 | `func` | `zt_host_read_file` |
| 34 | `func` | `zt_host_write_file` |
| 35 | `func` | `zt_host_fs_append_text` |
| 36 | `func` | `zt_host_path_exists` |
| 37 | `func` | `zt_host_fs_is_file` |
| 38 | `func` | `zt_host_fs_is_dir` |
| 39 | `func` | `zt_host_fs_create_dir` |
| 40 | `func` | `zt_host_fs_create_dir_all` |
| 41 | `func` | `zt_host_fs_list` |
| 42 | `func` | `zt_host_fs_remove_file` |
| 43 | `func` | `zt_host_fs_remove_dir` |
| 44 | `func` | `zt_host_fs_remove_dir_all` |
| 45 | `func` | `zt_host_fs_copy_file` |
| 46 | `func` | `zt_host_fs_move` |
| 47 | `func` | `zt_host_fs_metadata` |
| 48 | `func` | `zt_host_fs_size` |
| 49 | `func` | `zt_host_fs_modified_at` |
| 50 | `func` | `zt_host_fs_created_at` |
| 53 | `func` | `read_text` |
| 57 | `func` | `write_text` |
| 61 | `func` | `append_text` |
| 65 | `func` | `exists` |
| 66 | `const` | `val` |
| 70 | `func` | `is_file` |
| 74 | `func` | `is_dir` |
| 78 | `func` | `create_dir` |
| 82 | `func` | `create_dir_all` |
| 86 | `func` | `list` |
| 90 | `func` | `remove_file` |
| 94 | `func` | `remove_dir` |
| 98 | `func` | `remove_dir_all` |
| 102 | `func` | `copy_file` |
| 106 | `func` | `move` |
| 110 | `func` | `metadata` |
| 114 | `func` | `size` |
| 118 | `func` | `modified_at` |
| 122 | `func` | `created_at` |

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
| 43 | `const` | `part` |
| 46 | `const` | `slash` |
| 47 | `const` | `last_char` |
| 51 | `const` | `with_sep` |
| 64 | `func` | `normalize` |
| 68 | `func` | `is_absolute` |
| 72 | `func` | `is_relative` |
| 79 | `func` | `absolute` |
| 83 | `func` | `relative` |
| 87 | `func` | `base_name` |
| 92 | `const` | `slash` |
| 93 | `const` | `slash_index` |
| 105 | `func` | `name_without_extension` |
| 106 | `const` | `file_name` |
| 111 | `const` | `dot` |
| 112 | `const` | `dot_index` |
| 120 | `func` | `extension` |
| 121 | `const` | `file_name` |
| 126 | `const` | `dot` |
| 127 | `const` | `dot_index` |
| 135 | `func` | `parent` |
| 140 | `const` | `slash` |
| 141 | `const` | `slash_index` |
| 153 | `func` | `has_extension` |
| 155 | `const` | `dot` |
| 165 | `const` | `ext_val` |
| 169 | `func` | `change_extension` |
| 170 | `const` | `file_name` |
| 175 | `const` | `directory` |
| 176 | `const` | `stem` |
| 177 | `const` | `dot` |
| 178 | `const` | `root` |
| 191 | `const` | `stem_with_dot` |

#### `stdlib/std/io.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 7 | `namespace` | `std.io` |
| 9 | `struct` | `Input` |
| 13 | `struct` | `Output` |
| 17 | `enum` | `Error` |
| 23 | `const` | `input` |
| 24 | `const` | `output` |
| 25 | `const` | `err` |
| 28 | `func` | `zt_host_read_line_stdin` |
| 29 | `func` | `zt_host_read_all_stdin` |
| 30 | `func` | `zt_host_write_stdout` |
| 31 | `func` | `zt_host_write_stderr` |
| 34 | `func` | `read_line` |
| 38 | `func` | `read_all` |
| 42 | `func` | `write` |
| 49 | `func` | `print` |

#### `stdlib/std/json.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 9 | `namespace` | `std.json` |
| 11 | `struct` | `Value` |
| 15 | `struct` | `Object` |
| 19 | `struct` | `Array` |
| 23 | `enum` | `Kind` |
| 32 | `enum` | `Error` |
| 41 | `func` | `zt_json_parse` |
| 42 | `func` | `zt_json_stringify` |
| 43 | `func` | `zt_json_pretty` |
| 44 | `func` | `zt_json_read_file` |
| 45 | `func` | `zt_json_write_file` |
| 46 | `func` | `zt_json_kind` |
| 47 | `func` | `zt_json_has_key` |
| 48 | `func` | `zt_json_get_key` |
| 49 | `func` | `zt_json_as_text` |
| 50 | `func` | `zt_json_as_int` |
| 51 | `func` | `zt_json_as_float` |
| 52 | `func` | `zt_json_as_bool` |
| 53 | `func` | `zt_json_as_array` |
| 54 | `func` | `zt_json_as_object` |
| 57 | `func` | `parse` |
| 61 | `func` | `stringify` |
| 65 | `func` | `pretty` |
| 69 | `func` | `read` |
| 73 | `func` | `write` |
| 77 | `func` | `kind` |
| 81 | `func` | `has_key` |
| 85 | `func` | `get_key` |
| 89 | `func` | `as_text` |
| 93 | `func` | `as_int` |
| 97 | `func` | `as_float` |
| 101 | `func` | `as_bool` |
| 105 | `func` | `as_array` |
| 109 | `func` | `as_object` |

#### `stdlib/std/math.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 10 | `namespace` | `std.math` |
| 13 | `func` | `zt_math_pow` |
| 14 | `func` | `zt_math_sqrt` |
| 15 | `func` | `zt_math_floor` |
| 16 | `func` | `zt_math_ceil` |
| 17 | `func` | `zt_math_round_half_away_from_zero` |
| 18 | `func` | `zt_math_trunc` |
| 19 | `func` | `zt_math_sin` |
| 20 | `func` | `zt_math_cos` |
| 21 | `func` | `zt_math_tan` |
| 22 | `func` | `zt_math_asin` |
| 23 | `func` | `zt_math_acos` |
| 24 | `func` | `zt_math_atan` |
| 25 | `func` | `zt_math_atan2` |
| 26 | `func` | `zt_math_ln` |
| 27 | `func` | `zt_math_log_ten` |
| 28 | `func` | `zt_math_log2` |
| 29 | `func` | `zt_math_log` |
| 30 | `func` | `zt_math_exp` |
| 31 | `func` | `zt_math_is_nan` |
| 32 | `func` | `zt_math_is_infinite` |
| 33 | `func` | `zt_math_is_finite` |
| 34 | `func` | `zt_math_get_infinity` |
| 35 | `func` | `zt_math_get_nan` |
| 38 | `const` | `pi` |
| 39 | `const` | `e` |
| 40 | `const` | `tau` |
| 42 | `func` | `infinity` |
| 46 | `func` | `nan` |
| 50 | `func` | `abs` |
| 57 | `func` | `min` |
| 64 | `func` | `max` |
| 71 | `func` | `clamp` |
| 81 | `func` | `pow` |
| 85 | `func` | `sqrt` |
| 89 | `func` | `floor` |
| 93 | `func` | `ceil` |
| 97 | `func` | `round` |
| 101 | `func` | `trunc` |
| 105 | `func` | `deg_to_rad` |
| 109 | `func` | `rad_to_deg` |
| 113 | `func` | `approx_equal` |
| 121 | `func` | `sin` |
| 125 | `func` | `cos` |
| 129 | `func` | `tan` |
| 133 | `func` | `asin` |
| 137 | `func` | `acos` |
| 141 | `func` | `atan` |
| 145 | `func` | `atan2` |
| 149 | `func` | `ln` |
| 153 | `func` | `log10` |
| 157 | `func` | `log2` |
| 161 | `func` | `log` |
| 165 | `func` | `exp` |
| 169 | `func` | `is_nan` |
| 173 | `func` | `is_infinite` |
| 177 | `func` | `is_finite` |

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
| 32 | `func` | `zt_host_os_args` |
| 33 | `func` | `zt_host_os_env` |
| 34 | `func` | `zt_host_os_pid` |
| 35 | `func` | `zt_host_os_platform` |
| 36 | `func` | `zt_host_os_arch` |
| 37 | `func` | `zt_host_os_current_dir` |
| 38 | `func` | `zt_host_os_change_dir` |
| 41 | `func` | `args` |
| 45 | `func` | `env` |
| 49 | `func` | `pid` |
| 53 | `func` | `platform` |
| 54 | `const` | `raw` |
| 67 | `func` | `arch` |
| 68 | `const` | `raw` |
| 81 | `func` | `current_dir` |
| 85 | `func` | `change_dir` |

#### `stdlib/std/os/process.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 6 | `namespace` | `std.os.process` |
| 8 | `struct` | `ExitStatus` |
| 12 | `struct` | `CapturedRun` |
| 18 | `enum` | `Error` |
| 27 | `func` | `zt_host_process_run` |
| 28 | `func` | `zt_host_process_run_capture` |
| 31 | `func` | `run` |
| 35 | `func` | `run_capture` |

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
| 4 | `enum` | `Error` |
| 9 | `func` | `zt_text_to_utf8_bytes` |
| 10 | `func` | `zt_text_from_utf8_bytes` |
| 12 | `func` | `zt_text_trim` |
| 13 | `func` | `zt_text_trim_start` |
| 14 | `func` | `zt_text_trim_end` |
| 16 | `func` | `zt_text_contains` |
| 17 | `func` | `zt_text_starts_with` |
| 18 | `func` | `zt_text_ends_with` |
| 19 | `func` | `zt_text_has_whitespace` |
| 21 | `func` | `zt_text_index_of` |
| 22 | `func` | `zt_text_last_index_of` |
| 24 | `func` | `zt_text_replace_all` |
| 25 | `func` | `zt_text_replace_first` |
| 27 | `func` | `zt_text_split` |
| 28 | `func` | `zt_text_split_lines` |
| 29 | `func` | `zt_text_join` |
| 31 | `func` | `zt_text_is_empty` |
| 33 | `func` | `zt_text_to_lower` |
| 34 | `func` | `zt_text_to_upper` |
| 35 | `func` | `zt_text_capitalize` |
| 36 | `func` | `zt_text_title_case` |
| 38 | `func` | `zt_text_is_digits` |
| 40 | `func` | `zt_text_pad_left` |
| 41 | `func` | `zt_text_pad_right` |
| 43 | `func` | `zt_text_mask` |
| 44 | `func` | `zt_text_limit` |
| 45 | `func` | `zt_text_truncate` |
| 50 | `func` | `to_utf8` |
| 54 | `func` | `from_utf8` |
| 60 | `func` | `trim` |
| 64 | `func` | `trim_start` |
| 68 | `func` | `trim_end` |
| 74 | `func` | `contains` |
| 78 | `func` | `starts_with` |
| 82 | `func` | `ends_with` |
| 86 | `func` | `has_prefix` |
| 90 | `func` | `has_suffix` |
| 94 | `func` | `has_whitespace` |
| 98 | `func` | `is_empty` |
| 102 | `func` | `is_digits` |
| 108 | `func` | `index_of` |
| 112 | `func` | `last_index_of` |
| 118 | `func` | `replace_all` |
| 122 | `func` | `replace_first` |
| 128 | `func` | `split` |
| 132 | `func` | `split_lines` |
| 136 | `func` | `join` |
| 142 | `func` | `to_lower` |
| 146 | `func` | `to_upper` |
| 150 | `func` | `capitalize` |
| 154 | `func` | `title_case` |
| 160 | `func` | `pad_left` |
| 164 | `func` | `pad_right` |
| 168 | `func` | `mask` |
| 172 | `func` | `limit` |
| 176 | `func` | `truncate` |

#### `stdlib/std/time.zt`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 9 | `namespace` | `std.time` |
| 11 | `struct` | `Instant` |
| 15 | `struct` | `Duration` |
| 19 | `enum` | `Error` |
| 25 | `func` | `zt_host_time_now_unix_ms` |
| 26 | `func` | `zt_host_time_sleep_ms` |
| 29 | `func` | `now` |
| 33 | `func` | `sleep` |
| 37 | `func` | `since` |
| 41 | `func` | `until` |
| 45 | `func` | `diff` |
| 49 | `func` | `add` |
| 53 | `func` | `sub` |
| 57 | `func` | `from_unix` |
| 61 | `func` | `from_unix_ms` |
| 65 | `func` | `to_unix` |
| 69 | `func` | `to_unix_ms` |
| 73 | `func` | `milliseconds` |
| 77 | `func` | `seconds` |
| 81 | `func` | `minutes` |
| 85 | `func` | `hours` |

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

#### `stdlib/zdoc/std/bytes.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 8 | `doc_target` | `empty` |
| 18 | `doc_target` | `from_list` |
| 30 | `doc_target` | `to_list` |
| 42 | `doc_target` | `join` |
| 54 | `doc_target` | `starts_with` |
| 66 | `doc_target` | `ends_with` |
| 78 | `doc_target` | `contains` |

#### `stdlib/zdoc/std/collections.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 8 | `doc_target` | `queue_int_new` |
| 17 | `doc_target` | `queue_text_new` |
| 26 | `doc_target` | `queue_int_enqueue` |
| 35 | `doc_target` | `queue_text_enqueue` |
| 44 | `doc_target` | `queue_int_dequeue` |
| 53 | `doc_target` | `queue_text_dequeue` |
| 62 | `doc_target` | `queue_int_peek` |
| 73 | `doc_target` | `queue_text_peek` |
| 84 | `doc_target` | `stack_int_new` |
| 93 | `doc_target` | `stack_text_new` |
| 102 | `doc_target` | `stack_int_push` |
| 111 | `doc_target` | `stack_text_push` |
| 120 | `doc_target` | `stack_int_pop` |
| 129 | `doc_target` | `stack_text_pop` |
| 138 | `doc_target` | `stack_int_peek` |
| 149 | `doc_target` | `stack_text_peek` |
| 160 | `doc_target` | `grid2d_int_new` |
| 169 | `doc_target` | `grid2d_text_new` |
| 178 | `doc_target` | `grid2d_int_get` |
| 187 | `doc_target` | `grid2d_text_get` |
| 196 | `doc_target` | `grid2d_int_set` |
| 205 | `doc_target` | `grid2d_text_set` |
| 214 | `doc_target` | `grid2d_int_fill` |
| 223 | `doc_target` | `grid2d_text_fill` |
| 232 | `doc_target` | `grid2d_int_rows` |
| 241 | `doc_target` | `grid2d_text_rows` |
| 250 | `doc_target` | `grid2d_int_cols` |
| 259 | `doc_target` | `grid2d_text_cols` |
| 268 | `doc_target` | `grid2d_int_size` |
| 279 | `doc_target` | `grid2d_text_size` |
| 290 | `doc_target` | `grid3d_int_new` |
| 299 | `doc_target` | `grid3d_text_new` |
| 308 | `doc_target` | `grid3d_int_get` |
| 317 | `doc_target` | `grid3d_text_get` |
| 326 | `doc_target` | `grid3d_int_set` |
| 335 | `doc_target` | `grid3d_text_set` |
| 344 | `doc_target` | `grid3d_int_fill` |
| 353 | `doc_target` | `grid3d_text_fill` |
| 362 | `doc_target` | `grid3d_int_depth` |
| 371 | `doc_target` | `grid3d_text_depth` |
| 380 | `doc_target` | `grid3d_int_rows` |
| 395 | `doc_target` | `cols` |
| 410 | `doc_target` | `size e grid3d_text_rows` |
| 425 | `doc_target` | `cols` |
| 440 | `doc_target` | `size` |
| 455 | `doc_target` | `pqueue_int_new` |
| 464 | `doc_target` | `pqueue_text_new` |
| 473 | `doc_target` | `pqueue_int_push` |
| 482 | `doc_target` | `pqueue_text_push` |
| 491 | `doc_target` | `pqueue_int_pop` |
| 500 | `doc_target` | `pqueue_text_pop` |
| 509 | `doc_target` | `pqueue_int_peek` |
| 518 | `doc_target` | `pqueue_text_peek` |
| 527 | `doc_target` | `pqueue_int_len` |
| 536 | `doc_target` | `pqueue_text_len` |
| 545 | `doc_target` | `pqueue_int_is_empty` |
| 556 | `doc_target` | `pqueue_text_is_empty` |
| 567 | `doc_target` | `circbuf_int_new` |
| 576 | `doc_target` | `circbuf_text_new` |
| 585 | `doc_target` | `circbuf_int_push` |
| 594 | `doc_target` | `circbuf_text_push` |
| 603 | `doc_target` | `circbuf_int_pop` |
| 612 | `doc_target` | `circbuf_text_pop` |
| 621 | `doc_target` | `circbuf_int_peek` |
| 630 | `doc_target` | `circbuf_text_peek` |
| 639 | `doc_target` | `circbuf_int_len` |
| 648 | `doc_target` | `circbuf_text_len` |
| 657 | `doc_target` | `circbuf_int_capacity` |
| 666 | `doc_target` | `circbuf_text_capacity` |
| 675 | `doc_target` | `circbuf_int_is_full` |
| 684 | `doc_target` | `circbuf_text_is_full` |
| 693 | `doc_target` | `circbuf_int_is_empty` |
| 704 | `doc_target` | `circbuf_text_is_empty` |
| 715 | `doc_target` | `btreemap_text_new` |
| 723 | `doc_target` | `btreemap_text_set` |
| 731 | `doc_target` | `btreemap_text_get` |
| 739 | `doc_target` | `btreemap_text_get_optional` |
| 747 | `doc_target` | `btreemap_text_contains` |
| 755 | `doc_target` | `btreemap_text_remove` |
| 763 | `doc_target` | `btreemap_text_len` |
| 771 | `doc_target` | `btreemap_text_is_empty` |
| 781 | `doc_target` | `btreeset_text_new` |
| 789 | `doc_target` | `btreeset_text_insert` |
| 797 | `doc_target` | `btreeset_text_contains` |
| 805 | `doc_target` | `btreeset_text_remove` |
| 813 | `doc_target` | `btreeset_text_len` |
| 821 | `doc_target` | `btreeset_text_is_empty` |

#### `stdlib/zdoc/std/format.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 9 | `doc_target` | `BytesStyle` |
| 21 | `doc_target` | `number` |
| 33 | `doc_target` | `percent` |
| 46 | `doc_target` | `date` |
| 58 | `doc_target` | `datetime` |
| 72 | `doc_target` | `date_pattern` |
| 84 | `doc_target` | `datetime_pattern` |
| 96 | `doc_target` | `bytes` |
| 109 | `doc_target` | `hex` |
| 120 | `doc_target` | `bin` |

#### `stdlib/zdoc/std/fs.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 9 | `doc_target` | `Error` |
| 26 | `doc_target` | `Metadata` |
| 41 | `doc_target` | `read_text` |
| 52 | `doc_target` | `write_text` |
| 64 | `doc_target` | `append_text` |
| 76 | `doc_target` | `exists` |
| 88 | `doc_target` | `is_file` |
| 96 | `doc_target` | `is_dir` |
| 104 | `doc_target` | `create_dir` |
| 112 | `doc_target` | `create_dir_all` |
| 120 | `doc_target` | `list` |
| 131 | `doc_target` | `remove_file` |
| 139 | `doc_target` | `remove_dir` |
| 147 | `doc_target` | `remove_dir_all` |
| 155 | `doc_target` | `copy_file` |
| 166 | `doc_target` | `move` |
| 177 | `doc_target` | `metadata` |
| 185 | `doc_target` | `size` |
| 193 | `doc_target` | `modified_at` |
| 201 | `doc_target` | `created_at` |

#### `stdlib/zdoc/std/fs/path.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 9 | `doc_target` | `join` |
| 20 | `doc_target` | `base_name` |
| 31 | `doc_target` | `name_without_extension` |
| 39 | `doc_target` | `extension` |
| 50 | `doc_target` | `parent` |
| 61 | `doc_target` | `normalize` |
| 69 | `doc_target` | `absolute` |
| 82 | `doc_target` | `relative` |
| 90 | `doc_target` | `is_absolute` |
| 98 | `doc_target` | `is_relative` |
| 106 | `doc_target` | `has_extension` |
| 119 | `doc_target` | `change_extension` |

#### `stdlib/zdoc/std/io.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 9 | `doc_target` | `Input` |
| 20 | `doc_target` | `Output` |
| 31 | `doc_target` | `Error` |
| 43 | `doc_target` | `input` |
| 51 | `doc_target` | `output` |
| 59 | `doc_target` | `err` |
| 68 | `doc_target` | `read_line` |
| 81 | `doc_target` | `read_all` |
| 92 | `doc_target` | `write` |
| 105 | `doc_target` | `print` |

#### `stdlib/zdoc/std/json.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 9 | `doc_target` | `Value` |
| 19 | `doc_target` | `Object` |
| 30 | `doc_target` | `Array` |
| 40 | `doc_target` | `Kind` |
| 56 | `doc_target` | `Error` |
| 70 | `doc_target` | `parse` |
| 81 | `doc_target` | `stringify` |
| 89 | `doc_target` | `pretty` |
| 101 | `doc_target` | `read` |
| 113 | `doc_target` | `write` |
| 126 | `doc_target` | `kind` |
| 134 | `doc_target` | `has_key` |
| 142 | `doc_target` | `get_key` |
| 150 | `doc_target` | `as_text` |
| 158 | `doc_target` | `as_int` |
| 166 | `doc_target` | `as_float` |
| 174 | `doc_target` | `as_bool` |
| 182 | `doc_target` | `as_array` |
| 190 | `doc_target` | `as_object` |

#### `stdlib/zdoc/std/math.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 8 | `doc_target` | `pi` |
| 16 | `doc_target` | `e` |
| 24 | `doc_target` | `tau` |
| 32 | `doc_target` | `infinity` |
| 40 | `doc_target` | `nan` |
| 48 | `doc_target` | `abs` |
| 59 | `doc_target` | `min` |
| 67 | `doc_target` | `max` |
| 75 | `doc_target` | `clamp` |
| 88 | `doc_target` | `pow` |
| 96 | `doc_target` | `sqrt` |
| 104 | `doc_target` | `floor` |
| 112 | `doc_target` | `ceil` |
| 120 | `doc_target` | `round` |
| 128 | `doc_target` | `trunc` |
| 136 | `doc_target` | `deg_to_rad` |
| 144 | `doc_target` | `rad_to_deg` |
| 152 | `doc_target` | `approx_equal` |
| 165 | `doc_target` | `sin` |
| 173 | `doc_target` | `cos` |
| 181 | `doc_target` | `tan` |
| 189 | `doc_target` | `asin` |
| 197 | `doc_target` | `acos` |
| 205 | `doc_target` | `atan` |
| 213 | `doc_target` | `atan2` |
| 225 | `doc_target` | `ln` |
| 233 | `doc_target` | `log10` |
| 241 | `doc_target` | `log2` |
| 249 | `doc_target` | `log` |
| 261 | `doc_target` | `exp` |
| 269 | `doc_target` | `is_nan` |
| 277 | `doc_target` | `is_infinite` |
| 285 | `doc_target` | `is_finite` |

#### `stdlib/zdoc/std/net.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 8 | `doc_target` | `Error` |
| 28 | `doc_target` | `Connection` |
| 39 | `doc_target` | `connect` |
| 57 | `doc_target` | `read_some` |
| 75 | `doc_target` | `write_all` |
| 92 | `doc_target` | `close` |
| 103 | `doc_target` | `is_closed` |
| 114 | `doc_target` | `kind` |

#### `stdlib/zdoc/std/os.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 8 | `doc_target` | `Platform` |
| 22 | `doc_target` | `Arch` |
| 36 | `doc_target` | `Error` |
| 49 | `doc_target` | `args` |
| 57 | `doc_target` | `env` |
| 69 | `doc_target` | `pid` |
| 77 | `doc_target` | `platform` |
| 85 | `doc_target` | `arch` |
| 93 | `doc_target` | `current_dir` |
| 101 | `doc_target` | `change_dir` |

#### `stdlib/zdoc/std/os/process.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 9 | `doc_target` | `ExitStatus` |
| 20 | `doc_target` | `CapturedRun` |
| 32 | `doc_target` | `Error` |
| 47 | `doc_target` | `run` |
| 61 | `doc_target` | `run_capture` |

#### `stdlib/zdoc/std/random.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 8 | `doc_target` | `seed` |
| 19 | `doc_target` | `next` |
| 29 | `doc_target` | `between` |

#### `stdlib/zdoc/std/test.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 9 | `doc_target` | `fail` |
| 20 | `doc_target` | `skip` |

#### `stdlib/zdoc/std/text.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 9 | `doc_target` | `Error` |
| 14 | `doc_target` | `to_utf8` |
| 26 | `doc_target` | `from_utf8` |
| 38 | `doc_target` | `trim` |
| 49 | `doc_target` | `trim_start` |
| 60 | `doc_target` | `trim_end` |
| 71 | `doc_target` | `contains` |
| 83 | `doc_target` | `starts_with` |
| 95 | `doc_target` | `ends_with` |
| 107 | `doc_target` | `has_prefix` |
| 119 | `doc_target` | `has_suffix` |
| 131 | `doc_target` | `has_whitespace` |
| 142 | `doc_target` | `is_empty` |
| 153 | `doc_target` | `is_digits` |
| 164 | `doc_target` | `index_of` |
| 176 | `doc_target` | `last_index_of` |
| 188 | `doc_target` | `replace_all` |
| 201 | `doc_target` | `replace_first` |
| 214 | `doc_target` | `split` |
| 226 | `doc_target` | `split_lines` |
| 237 | `doc_target` | `join` |
| 249 | `doc_target` | `to_lower` |
| 260 | `doc_target` | `to_upper` |
| 271 | `doc_target` | `capitalize` |
| 282 | `doc_target` | `title_case` |
| 293 | `doc_target` | `pad_left` |
| 306 | `doc_target` | `pad_right` |
| 319 | `doc_target` | `mask` |
| 333 | `doc_target` | `limit` |
| 345 | `doc_target` | `truncate` |

#### `stdlib/zdoc/std/time.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 9 | `doc_target` | `Instant` |
| 19 | `doc_target` | `Duration` |
| 29 | `doc_target` | `Error` |
| 40 | `doc_target` | `now` |
| 48 | `doc_target` | `sleep` |
| 59 | `doc_target` | `since` |
| 67 | `doc_target` | `until` |
| 75 | `doc_target` | `diff` |
| 83 | `doc_target` | `add` |
| 91 | `doc_target` | `sub` |
| 99 | `doc_target` | `from_unix` |
| 107 | `doc_target` | `from_unix_ms` |
| 115 | `doc_target` | `to_unix` |
| 123 | `doc_target` | `to_unix_ms` |
| 131 | `doc_target` | `milliseconds` |
| 139 | `doc_target` | `seconds` |
| 147 | `doc_target` | `minutes` |
| 155 | `doc_target` | `hours` |

#### `stdlib/zdoc/std/validate.zdoc`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 1 | `doc_target` | `namespace` |
| 8 | `doc_target` | `between` |
| 21 | `doc_target` | `positive` |
| 29 | `doc_target` | `non_negative` |
| 37 | `doc_target` | `negative` |
| 45 | `doc_target` | `non_zero` |
| 53 | `doc_target` | `one_of` |
| 65 | `doc_target` | `not_empty` |
| 73 | `doc_target` | `min_length` |
| 85 | `doc_target` | `max_length` |
| 97 | `doc_target` | `length_between` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->

