# ZIR Model - Code Map

## 📋 Descrição

Zenith Intermediate Representation - Model. Responsável por:
- Definição da IR intermediária do compiler
- Estruturas de dados para instruções, basic blocks, functions
- Types e metadata da IR
- Serialização/deserialização da IR

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `model.c` | 42.5 KB | Implementação do model ZIR |
| `model.h` | 11.4 KB | Definições de structs, enums, types |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🔴 CRÍTICA |

## ⚠️ Estado Crítico

- **IR context**: container para toda a IR de um módulo
- **Instruction list**: linked list de instruções por basic block
- **Type system**: tipos da IR (diferente do type system semantic)

## 🔗 Dependencies Externas

- `frontend/ast/` → Input do lowering
- `utils/arena.c` → Memory allocation
- `semantic/types/` → Type mapping

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- Arquivo GRANDE (42KB) → dividir em seções menores
- Core do compiler → mudanças aqui afetam tudo

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Critical
- Source files: 2
- Extracted symbols: 100

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/zir/model.c` | 1092 | 99 | 1 |
| `compiler/zir/model.h` | 344 | 1 | 0 |

### Local Dependencies

- `compiler/zir/model.h`

### Related Tests

- `tests/zir/test_enum_lowering.c`
- `tests/zir/test_lowering.c`
- `tests/zir/test_printer.c`

### Symbol Index

#### `compiler/zir/model.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 8 | `struct` | `zir_string_buffer` |
| 14 | `func_def` | `zir_strdup_owned` |
| 25 | `func_def` | `zir_free_owned_nonempty` |
| 31 | `func_def` | `zir_grow_array` |
| 41 | `func_def` | `zir_string_buffer_init` |
| 47 | `func_def` | `zir_string_buffer_reserve` |
| 63 | `func_def` | `zir_string_buffer_append_n` |
| 72 | `func_def` | `zir_string_buffer_append` |
| 77 | `func_def` | `zir_string_buffer_append_char` |
| 81 | `func_def` | `zir_string_buffer_append_format` |
| 104 | `func_def` | `zir_render_string_literal` |
| 122 | `func_def` | `zir_make_span` |
| 130 | `func_def` | `zir_span_is_known` |
| 137 | `func_def` | `zir_expr_kind_name` |
| 176 | `func_def` | `zir_expr_list_make` |
| 184 | `func_def` | `zir_expr_list_push` |
| 195 | `func_def` | `zir_expr_list_dispose` |
| 205 | `func_def` | `zir_named_expr_list_make` |
| 213 | `func_def` | `zir_named_expr_list_push` |
| 224 | `func_def` | `zir_named_expr_list_dispose` |
| 237 | `func_def` | `zir_map_entry_list_make` |
| 245 | `func_def` | `zir_map_entry_list_push` |
| 256 | `func_def` | `zir_map_entry_list_dispose` |
| 269 | `func_def` | `zir_expr_make` |
| 277 | `func_def` | `zir_expr_make_text_like` |
| 284 | `func_def` | `zir_expr_make_name` |
| 285 | `func_def` | `zir_expr_make_int` |
| 286 | `func_def` | `zir_expr_make_float` |
| 287 | `func_def` | `zir_expr_make_string` |
| 288 | `func_def` | `zir_expr_make_bytes` |
| 290 | `func_def` | `zir_expr_make_bool` |
| 297 | `func_def` | `zir_expr_make_copy` |
| 304 | `func_def` | `zir_expr_make_unary` |
| 312 | `func_def` | `zir_expr_make_binary` |
| 321 | `func_def` | `zir_expr_make_call_like` |
| 329 | `func_def` | `zir_expr_make_call_direct` |
| 330 | `func_def` | `zir_expr_make_call_extern` |
| 331 | `func_def` | `zir_expr_make_call_runtime_intrinsic` |
| 333 | `func_def` | `zir_expr_make_make_struct` |
| 341 | `func_def` | `zir_expr_make_make_list` |
| 349 | `func_def` | `zir_expr_make_make_map` |
| 358 | `func_def` | `zir_expr_make_get_field` |
| 366 | `func_def` | `zir_expr_make_set_field` |
| 375 | `func_def` | `zir_expr_make_index_seq` |
| 383 | `func_def` | `zir_expr_make_slice_seq` |
| 392 | `func_def` | `zir_expr_make_list_len` |
| 399 | `func_def` | `zir_expr_make_map_len` |
| 406 | `func_def` | `zir_expr_make_list_push` |
| 414 | `func_def` | `zir_expr_make_list_set` |
| 423 | `func_def` | `zir_expr_make_map_set` |
| 432 | `func_def` | `zir_expr_make_optional_present` |
| 439 | `func_def` | `zir_expr_make_optional_empty` |
| 446 | `func_def` | `zir_expr_make_optional_is_present` |
| 453 | `func_def` | `zir_expr_make_coalesce` |
| 461 | `func_def` | `zir_expr_make_outcome_success` |
| 468 | `func_def` | `zir_expr_make_outcome_failure` |
| 475 | `func_def` | `zir_expr_make_outcome_is_success` |
| 482 | `func_def` | `zir_expr_make_outcome_value` |
| 489 | `func_def` | `zir_expr_make_try_propagate` |
| 496 | `func_def` | `zir_expr_call_add_arg` |
| 502 | `func_def` | `zir_expr_make_struct_add_field` |
| 511 | `func_def` | `zir_expr_make_list_add_item` |
| 516 | `func_def` | `zir_expr_make_map_add_entry` |
| 525 | `func_decl` | `zir_render_expr` |
| 527 | `func_def` | `zir_render_expr_list` |
| 537 | `func_def` | `zir_render_expr` |
| 670 | `func_def` | `zir_expr_render_alloc` |
| 681 | `func_def` | `zir_expr_dispose` |
| 762 | `func_def` | `zir_make_param` |
| 771 | `func_def` | `zir_make_field_decl` |
| 780 | `func_def` | `zir_make_assign_instruction` |
| 791 | `func_def` | `zir_make_assign_instruction_expr` |
| 798 | `func_def` | `zir_make_effect_instruction` |
| 809 | `func_def` | `zir_make_effect_instruction_expr` |
| 816 | `func_def` | `zir_make_check_contract_instruction_expr` |
| 824 | `func_def` | `zir_make_return_terminator` |
| 840 | `func_def` | `zir_make_return_terminator_expr` |
| 847 | `func_def` | `zir_make_jump_terminator` |
| 854 | `func_def` | `zir_make_branch_if_terminator` |
| 863 | `func_def` | `zir_make_branch_if_terminator_expr` |
| 870 | `func_def` | `zir_make_panic_terminator` |
| 877 | `func_def` | `zir_make_panic_terminator_expr` |
| 884 | `func_def` | `zir_make_unreachable_terminator` |
| 890 | `func_def` | `zir_make_block` |
| 900 | `func_def` | `zir_make_struct_decl` |
| 909 | `func_def` | `zir_make_enum_variant_field_decl` |
| 917 | `func_def` | `zir_make_enum_variant_decl` |
| 926 | `func_def` | `zir_make_enum_decl` |
| 935 | `func_def` | `zir_make_function` |
| 950 | `func_def` | `zir_make_module` |
| 954 | `func_def` | `zir_make_module_with_structs` |
| 967 | `func_def` | `zir_make_module_with_decls` |
| 981 | `func_def` | `zir_instruction_dispose_owned` |
| 990 | `func_def` | `zir_terminator_dispose_owned` |
| 1004 | `func_def` | `zir_block_dispose_owned` |
| 1016 | `func_def` | `zir_struct_decl_dispose_owned` |
| 1031 | `func_def` | `zir_enum_decl_dispose_owned` |
| 1051 | `func_def` | `zir_function_dispose_owned` |
| 1073 | `func_def` | `zir_module_dispose_owned` |

#### `compiler/zir/model.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_ZIR_MODEL_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
