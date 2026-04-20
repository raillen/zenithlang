# ZIR Verifier - Code Map

## 📋 Descrição

Verificador da Zenith Intermediate Representation. Responsável por:
- Validar correctness da IR após lowering
- Check de types na IR
- Validação de control flow graph
- Detecção de bugs no compiler early

## 📁 Arquivos Principais

| Arquivo | Tamanho | Responsabilidade |
|---------|---------|------------------|
| `verifier.c` | 33.9 KB | Implementação do verifier |
| `verifier.h` | 1.0 KB | Interface pública |

## 🔍 Funções Críticas

| Linha | Função | Responsabilidade | Dependencies | Pode Quebrar Se | Prioridade |
|-------|--------|------------------|--------------|-----------------|------------|
| - | - | - | - | - | 🟡 MÉDIA |

## ⚠️ Estado Crítico

- **Verification pass**: checklist de validações
- **Error collection**: múltiplos erros podem ser reportados
- **CFG analysis**: validação de jump targets, phi nodes

## 🔗 Dependencies Externas

- `zir/model.c` → ZIR structures para validar
- `semantic/types/` → Type checking na IR

## 🐛 Erros Comuns

1. [A preencher]
2. [A preencher]
3. [A preencher]

## 📝 Notas de Manutenção

- CRUCIAL para debugging do compiler
- Rodar verifier após cada lowering pass
- Arquivo GRANDE (34KB) → muitas validações

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Index

- Priority: Medium
- Source files: 2
- Extracted symbols: 35

Do not edit this block by hand. Re-run `python tools/generate_code_maps.py`.

### File Summary

| File | Lines | Symbols | Local deps |
| --- | ---: | ---: | ---: |
| `compiler/zir/verifier.c` | 1097 | 34 | 1 |
| `compiler/zir/verifier.h` | 42 | 1 | 1 |

### Local Dependencies

- `compiler/zir/model.h`
- `compiler/zir/verifier.h`

### Related Tests

- `tests/zir/test_verifier.c`

### Symbol Index

#### `compiler/zir/verifier.c`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 8 | `macro` | `ARRAY_COUNT` |
| 10 | `func_def` | `zir_safe_text` |
| 14 | `struct` | `zir_symbol_table` |
| 20 | `func_def` | `zir_span_ptr_is_known` |
| 24 | `func_def` | `zir_prefer_span` |
| 34 | `func_def` | `zir_verifier_set_result_at` |
| 83 | `func_def` | `zir_verifier_set_result` |
| 102 | `func_def` | `zir_verifier_result_init` |
| 106 | `func_def` | `zir_verifier_code_name` |
| 131 | `func_def` | `zir_is_identifier_start` |
| 137 | `func_def` | `zir_is_identifier_char` |
| 141 | `func_def` | `zir_text_is_blank` |
| 158 | `func_def` | `zir_starts_with` |
| 165 | `func_def` | `zir_find_token` |
| 195 | `func_def` | `zir_contains_any_type` |
| 199 | `func_def` | `zir_contains_target_leak` |
| 222 | `func_def` | `zir_verify_type_name` |
| 252 | `func_def` | `zir_symbol_table_contains` |
| 268 | `func_def` | `zir_symbol_table_add` |
| 278 | `func_def` | `zir_symbol_table_add_unique` |
| 285 | `func_def` | `zir_count_function_definitions` |
| 306 | `func_def` | `zir_collect_function_definitions` |
| 343 | `func_def` | `zir_is_reserved_identifier` |
| 415 | `func_def` | `zir_verify_identifier_usage_tokens` |
| 506 | `func_def` | `zir_copy_trimmed_segment` |
| 524 | `func_def` | `zir_verify_identifier_usage` |
| 559 | `func_def` | `zir_verify_named_reference` |
| 591 | `func_def` | `zir_verify_metadata_text` |
| 614 | `func_def` | `zir_verify_expr` |
| 732 | `func_def` | `zir_verify_instruction` |
| 792 | `func_def` | `zir_block_label_exists` |
| 809 | `func_def` | `zir_verify_terminator` |
| 922 | `func_def` | `zir_verify_function` |
| 1033 | `func_def` | `zir_verify_module` |

#### `compiler/zir/verifier.h`

| Line | Kind | Symbol |
| ---: | --- | --- |
| 2 | `macro` | `ZENITH_NEXT_COMPILER_ZIR_VERIFIER_H` |

### Manual Notes

- Critical flow:
- Break conditions:
- Related docs or decisions:
- Extra test cases worth adding:
<!-- CODEMAP:GENERATED:END -->
