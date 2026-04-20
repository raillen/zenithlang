# Formatter - Code Map

## 📋 Descrição

Formatador de código Zenith. Responsável por:
- Pretty-printing de código source
- Auto-formatting (`zt format`)
- Consistência de estilo
- Preservation de comentários

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `formatter.c` | 23.0 KB | Implementação do formatter |
| `formatter.h` | 0.3 KB | Interface pública |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟢 BAIXA |

## ⚠️ Estado Crítico

- **AST input**: árvore sendo formatada
- **Output buffer**: código formatado
- **Style config**: indentação, line width, etc.

## 🔗 Dependencies Externas

- `frontend/ast/` → AST para formatar
- `frontend/parser/` → Reparse após formatting

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- Tooling opcional, não bloqueante
- Importante para UX mas pode ser desenvolvido depois

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Low
- Source files: 2
- Extracted symbols: 13

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/tooling/formatter.c` | 587 | 12 | 1 |
| `compiler/tooling/formatter.h` | 17 | 1 | 1 |

### Local Dependencies

- `compiler/frontend/ast/model.h`
- `compiler/tooling/formatter.h`

### Related Tests

- `tests/formatter/cases/case_all/expected/src/app/main.zt`
- `tests/formatter/cases/case_all/input/src/app/main.zt`
- `tests/formatter/run_formatter_golden.py`

### Symbol Index

#### `compiler/tooling/formatter.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 7 | `struct` | `sb_t` |
| 14 | `func_def` | `sb_init` |
| 22 | `func_def` | `sb_free` |
| 26 | `func_def` | `sb_append_len` |
| 41 | `func_def` | `sb_append` |
| 45 | `func_def` | `sb_indent` |
| 51 | `func_decl` | `format_node` |
| 53 | `func_def` | `format_comments` |
| 62 | `func_def` | `op_to_str` |
| 83 | `func_def` | `format_node_list_comma` |
| 90 | `func_def` | `format_node` |
| 581 | `func_def` | `zt_format_node_to_string` |

#### `compiler/tooling/formatter.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_TOOLING_FORMATTER_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
