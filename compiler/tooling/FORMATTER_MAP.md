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
- Extracted symbols: 29

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/tooling/formatter.c` | 888 | 28 | 1 |
| `compiler/tooling/formatter.h` | 17 | 1 | 1 |

### Local Dependencies

- `compiler/frontend/ast/model.h`
- `compiler/tooling/formatter.h`

### Related Tests

- `tests/formatter/cases/case_all/expected/src/app/main.zt`
- `tests/formatter/cases/case_all/input/src/app/main.zt`
- `tests/formatter/cases/case_comments/expected/src/app/main.zt`
- `tests/formatter/cases/case_comments/input/src/app/main.zt`
- `tests/formatter/cases/case_generics/expected/src/app/main.zt`
- `tests/formatter/cases/case_generics/input/src/app/main.zt`
- `tests/formatter/cases/case_imports/expected/src/app/main.zt`
- `tests/formatter/cases/case_imports/input/src/app/main.zt`
- `tests/formatter/cases/case_manifest/expected/src/app/main.zt`
- `tests/formatter/cases/case_manifest/input/src/app/main.zt`
- `tests/formatter/cases/case_match/expected/src/app/main.zt`
- `tests/formatter/cases/case_match/input/src/app/main.zt`
- `tests/formatter/cases/case_reading_first/expected/src/app/main.zt`
- `tests/formatter/cases/case_reading_first/input/src/app/main.zt`
- `tests/formatter/cases/case_structs/expected/src/app/main.zt`
- `tests/formatter/cases/case_structs/input/src/app/main.zt`
- `tests/formatter/cases/case_trailing_commas/expected/src/app/main.zt`
- `tests/formatter/cases/case_trailing_commas/input/src/app/main.zt`
- `tests/formatter/cases/case_triple_quoted/expected/src/app/main.zt`
- `tests/formatter/cases/case_triple_quoted/input/src/app/main.zt`
- `tests/formatter/run_formatter_golden.py`
- `tests/formatter/run_formatter_idempotence.py`

### Symbol Index

#### `compiler/tooling/formatter.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 7 | `struct` | `sb_t` |
| 14 | `func_def` | `sb_init` |
| 22 | `func_def` | `sb_append_len` |
| 37 | `func_def` | `sb_append` |
| 41 | `func_def` | `sb_append_string_value` |
| 68 | `func_def` | `sb_append_fmt_literal_value` |
| 88 | `func_def` | `sb_indent` |
| 94 | `func_decl` | `format_node` |
| 96 | `macro` | `ZT_FORMATTER_TARGET_WIDTH` |
| 98 | `func_def` | `format_comments` |
| 107 | `func_def` | `op_to_str` |
| 128 | `func_def` | `format_node_list_comma` |
| 135 | `func_def` | `format_node_list_separator` |
| 142 | `func_def` | `format_node_list_and` |
| 146 | `func_def` | `format_node_to_owned_text` |
| 153 | `func_def` | `formatted_node_length` |
| 160 | `func_def` | `formatted_node_list_length` |
| 170 | `func_def` | `formatted_type_params_length` |
| 175 | `func_def` | `formatted_func_decl_signature_length` |
| 196 | `func_def` | `formatted_trait_method_signature_length` |
| 211 | `func_def` | `param_where_count` |
| 224 | `func_def` | `param_default_count` |
| 237 | `func_def` | `should_format_param_list_multiline` |
| 254 | `func_def` | `format_param_list_multiline` |
| 268 | `func_def` | `format_func_decl_signature` |
| 311 | `func_def` | `format_trait_method_signature` |
| 331 | `func_def` | `format_node` |
| 882 | `func_def` | `zt_format_node_to_string` |

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
