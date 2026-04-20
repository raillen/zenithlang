# Symbol Table - Code Map

## 📋 Descrição

Tabela de símbolos. Responsável por:
- Storage de symbols (functions, variables, types)
- Symbol lookup por nome
- Symbol metadata (type, scope, visibility)
- Symbol lifecycle management

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `*.c` | - | Implementação da symbol table |
| `*.h` | - | Symbol structs, enums |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟡 MÉDIA |

## ⚠️ Estado Crítico

- **Symbol map**: name → symbol lookup
- **Symbol IDs**: identificadores únicos
- **Scope information**: onde symbol foi declarado

## 🔗 Dependencies Externas

- `semantic/binder/` → Popula symbol table
- `semantic/types/` → Types dos symbols
- `utils/string_pool.c` → Symbol names

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- [A preencher após análise detalhada]

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Medium
- Source files: 2
- Extracted symbols: 9

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/semantic/symbols/symbols.c` | 117 | 8 | 1 |
| `compiler/semantic/symbols/symbols.h` | 58 | 1 | 1 |

### Local Dependencies

- `compiler/frontend/lexer/token.h`
- `compiler/semantic/symbols/symbols.h`

### Related Tests

- `tests/behavior/multifile_duplicate_symbol/src/app/a/foo.zt`
- `tests/behavior/multifile_duplicate_symbol/src/app/b/foo.zt`
- `tests/behavior/multifile_duplicate_symbol/src/app/main.zt`
- `tests/behavior/multifile_import_alias/src/app/main.zt`
- `tests/behavior/multifile_import_alias/src/app/math.zt`
- `tests/behavior/multifile_import_cycle/src/app/main.zt`
- `tests/behavior/multifile_import_cycle/src/app/util.zt`
- `tests/behavior/multifile_missing_import/src/app/main.zt`
- `tests/behavior/multifile_namespace_mismatch/src/app/main.zt`
- `tests/behavior/multifile_namespace_mismatch/src/app/wrong.zt`
- `tests/behavior/multifile_private_access/src/app/main.zt`
- `tests/behavior/multifile_private_access/src/app/secrets.zt`
- `tests/semantic/test_binder.c`

### Symbol Index

#### `compiler/semantic/symbols/symbols.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 6 | `func_def` | `zt_symbols_strdup` |
| 18 | `func_def` | `zt_symbol_kind_name` |
| 36 | `func_def` | `zt_scope_init` |
| 44 | `func_def` | `zt_scope_dispose` |
| 58 | `func_def` | `zt_scope_lookup_current` |
| 70 | `func_def` | `zt_scope_lookup_parent_chain` |
| 83 | `func_def` | `zt_scope_lookup` |
| 93 | `func_def` | `zt_scope_declare` |

#### `compiler/semantic/symbols/symbols.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_SEMANTIC_SYMBOLS_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
