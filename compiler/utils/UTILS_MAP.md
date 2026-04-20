# Utils - Code Map

## 📋 Descrição

Utilitários fundamentais. Responsável por:
- Arena allocation (memory management)
- String pooling (string interning)
- Diagnostics infrastructure
- Internacionalização (l10n)

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `arena.c/h` | 2.1K/1.3K | Arena memory allocator |
| `string_pool.c/h` | 3.1K/1.4K | String interning |
| `diagnostics.h` | 4.8 KB | Error/warning reporting |
| `l10n.c/h` | 6.4K/0.7K | Internacionalização |

## 🔍 Funções Críticas

### Arena (arena.c)
| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟡 MÉDIA |

### String Pool (string_pool.c)
| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟡 MÉDIA |

### Diagnostics (diagnostics.h)
| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟢 BAIXA |

### L10n (l10n.c)
| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟢 BAIXA |

## ⚠️ Estado Crítico

- **Arena state**: blocos alocados, current pointer
- **String table**: strings interned
- **Diagnostic callbacks**: handlers customizados

## 🔗 Dependencies Externas

- Usado por TODO o compiler
- Dependencies circulares devem ser evitadas

## 🐛 Erros Comuns

1. **Arena overflow** → crash se não houver memory
2. **String pool leak** → memory waste se não cleared
3. **Diagnostic flood** → performance issue com muitos erros

## 📝 Notas de Manutenção

- UTILITÁRIOS → usados em todo lugar
- Bugs aqui são difíceis de debugar (efeito cascata)
- Manter simples e bem testado

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Low
- Source files: 7
- Extracted symbols: 36

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/utils/arena.c` | 85 | 5 | 1 |
| `compiler/utils/arena.h` | 58 | 1 | 0 |
| `compiler/utils/diagnostics.h` | 154 | 1 | 1 |
| `compiler/utils/l10n.c` | 133 | 10 | 0 |
| `compiler/utils/l10n.h` | 26 | 12 | 1 |
| `compiler/utils/string_pool.c` | 99 | 6 | 1 |
| `compiler/utils/string_pool.h` | 54 | 1 | 1 |

### Local Dependencies

- `compiler/frontend/lexer/token.h`
- `compiler/utils/arena.h`
- `compiler/utils/diagnostics.h`
- `compiler/utils/string_pool.h`

### Related Tests

- `tests/frontend/README.md`
- `tests/frontend/test_lexer.c`
- `tests/frontend/test_parser.c`
- `tests/frontend/test_parser_error_recovery.c`
- `tests/semantic/README.md`
- `tests/semantic/mini_destruct.zt`
- `tests/semantic/mini_test.zt`
- `tests/semantic/test_alias_unions.zt`
- `tests/semantic/test_async_full.zt`
- `tests/semantic/test_async_v1.zt`
- `tests/semantic/test_binder.c`
- `tests/semantic/test_constraints.c`
- `tests/semantic/test_enums_sum_types.zt`
- `tests/semantic/test_errors_lambdas.zt`
- `tests/semantic/test_fase1.zt`
- `tests/semantic/test_fase11_indexing.zt`
- `tests/semantic/test_generics_hardening.zt`
- `tests/semantic/test_hir_lowering.c`
- `tests/semantic/test_match_hardening.zt`
- `tests/semantic/test_simple.zt`
- `tests/semantic/test_stabilization_final.zt`
- `tests/semantic/test_try_operator.zt`
- `tests/semantic/test_types.c`
- `tests/semantic/test_ufcs_hardening.zt`
- `tests/zir/README.md`
- `tests/zir/test_enum_lowering.c`
- `tests/zir/test_lowering.c`
- `tests/zir/test_printer.c`
- `tests/zir/test_verifier.c`

### Symbol Index

#### `compiler/utils/arena.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 5 | `func_def` | `zt_arena_init` |
| 12 | `func_def` | `zt_arena_dispose` |
| 29 | `func_def` | `zt_arena_alloc_chunk` |
| 38 | `func_def` | `zt_arena_alloc` |
| 72 | `func_def` | `zt_arena_strdup` |

#### `compiler/utils/arena.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZT_ARENA_H` |

#### `compiler/utils/diagnostics.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_SEMANTIC_DIAGNOSTICS_H` |

#### `compiler/utils/l10n.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 7 | `func_def` | `zt_l10n_set_lang` |
| 11 | `func_def` | `zt_l10n_is_explicitly_set` |
| 15 | `func_def` | `zt_l10n_from_str` |
| 24 | `func_def` | `zt_l10n_current_lang` |
| 37 | `func_def` | `zt_l10n_label_where` |
| 45 | `func_def` | `zt_l10n_label_code` |
| 53 | `func_def` | `zt_l10n_label_note` |
| 61 | `func_def` | `zt_l10n_label_help` |
| 69 | `func_def` | `zt_l10n_severity_name` |
| 96 | `func_def` | `zt_l10n_default_help` |

#### `compiler/utils/l10n.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_UTILS_L10N_H` |
| 6 | `enum` | `zt_lang` |
| 14 | `func_decl` | `zt_l10n_current_lang` |
| 15 | `func_decl` | `zt_l10n_set_lang` |
| 16 | `func_decl` | `zt_l10n_is_explicitly_set` |
| 17 | `func_decl` | `zt_l10n_from_str` |
| 18 | `func_decl` | `zt_l10n_label_where` |
| 19 | `func_decl` | `zt_l10n_label_code` |
| 20 | `func_decl` | `zt_l10n_label_note` |
| 21 | `func_decl` | `zt_l10n_label_help` |
| 22 | `func_decl` | `zt_l10n_severity_name` |
| 23 | `func_decl` | `zt_l10n_default_help` |

#### `compiler/utils/string_pool.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 4 | `macro` | `ZT_STRING_POOL_INITIAL_CAPACITY` |
| 7 | `func_def` | `zt_hash_string` |
| 17 | `func_def` | `zt_string_pool_init` |
| 28 | `func_def` | `zt_string_pool_grow` |
| 51 | `func_def` | `zt_string_pool_intern_len` |
| 95 | `func_def` | `zt_string_pool_intern` |

#### `compiler/utils/string_pool.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZT_STRING_POOL_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
