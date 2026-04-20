# ZIR Parser - Code Map

## 📋 Descrição

Parser da Zenith Intermediate Representation. Responsável por:
- Parse de arquivos ZIR text format
- Reconstrução da IR a partir de texto
- Validation durante o parse
- Debug tooling (print/read IR)

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `parser.c` | 20.4 KB | Implementação do parser ZIR |
| `parser.h` | 1.4 KB | Interface pública |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟡 MÉDIA |

## ⚠️ Estado Crítico

- **Parser state**: tokens sendo processados
- **IR builder**: construindo IR gradualmente
- **Error handling**: recovery de syntax errors

## 🔗 Dependencies Externas

- `zir/model.c` → ZIR data structures
- `utils/diagnostics.h` → Error reporting

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- Usado principalmente para debugging e tests
- Não é crítico para o pipeline principal de compilação

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Medium
- Source files: 2
- Extracted symbols: 23

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/zir/parser.c` | 553 | 22 | 1 |
| `compiler/zir/parser.h` | 56 | 1 | 1 |

### Local Dependencies

- `compiler/zir/model.h`
- `compiler/zir/parser.h`

### Related Tests

- `tests/zir/test_enum_lowering.c`
- `tests/zir/test_lowering.c`

### Symbol Index

#### `compiler/zir/parser.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 8 | `macro` | `ZIR_PARSER_MAX_LINE` |
| 9 | `macro` | `ZIR_PARSER_MAX_PARAMS` |
| 10 | `macro` | `ZIR_PARSER_MAX_FUNCTIONS` |
| 11 | `macro` | `ZIR_PARSER_MAX_BLOCKS` |
| 12 | `macro` | `ZIR_PARSER_MAX_INSTRUCTIONS` |
| 14 | `func_def` | `zir_parse_set_error` |
| 21 | `func_def` | `zir_parse_error_code_name` |
| 44 | `func_def` | `zir_parse_result_init` |
| 51 | `func_def` | `zir_parse_result_dispose` |
| 55 | `func_def` | `zir_is_blank` |
| 63 | `func_def` | `zir_trim_inplace` |
| 74 | `func_def` | `zir_skip_trim` |
| 79 | `func_def` | `zir_strdup` |
| 84 | `func_def` | `zir_starts_with` |
| 88 | `func_def` | `zir_parse_type_at` |
| 133 | `func_def` | `zir_find_colon_for_assign` |
| 161 | `func_def` | `zir_find_equals_after_type` |
| 180 | `func_def` | `zir_parse_params` |
| 242 | `func_def` | `zir_starts_with_keyword` |
| 250 | `func_def` | `zir_is_terminator_line` |
| 261 | `func_def` | `zir_parse_terminator_text` |
| 332 | `func_def` | `zir_parse_module` |

#### `compiler/zir/parser.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_ZIR_PARSER_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
