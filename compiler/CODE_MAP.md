# Compiler - Code Map Master

## 📋 Visão Geral

Este arquivo serve como índice mestre para navegação de todo o código do compiler Zenith.
Cada módulo possui seu próprio arquivo de mapeamento detalhado.

## 🏗️ Estrutura do Compiler

```
compiler/
├── driver/          → Ponto de entrada, CLI, orquestração
├── frontend/        → Lexer, Parser, AST
├── zir/             → Zenith Intermediate Representation
├── hir/             → High-level IR
├── semantic/        → Binder, Type System, Symbol Table
├── targets/         → Code Generation (C backend)
├── project/         → Project management, ZDoc
├── tooling/         → Formatter, LSP
└── utils/           → Arena, String Pool, Diagnostics
```

## 🗂️ Mapas de Módulos

### Compiler
| Módulo | Arquivo de Mapa | Status | Prioridade |
|--------|----------------|--------|------------|
| Driver/CLI | [driver/DRIVER_MAP.md](driver/DRIVER_MAP.md) | ⚙️ Gerado + notas manuais | 🔴 CRÍTICA |
| Lexer | [frontend/lexer/LEXER_MAP.md](frontend/lexer/LEXER_MAP.md) | ⚙️ Gerado + notas manuais | 🔴 CRÍTICA |
| Parser | [frontend/parser/PARSER_MAP.md](frontend/parser/PARSER_MAP.md) | ⚙️ Gerado + notas manuais | 🔴 CRÍTICA |
| AST | [frontend/ast/AST_MAP.md](frontend/ast/AST_MAP.md) | ⚙️ Gerado + notas manuais | 🟡 MÉDIA |
| ZIR Model | [zir/ZIR_MODEL_MAP.md](zir/ZIR_MODEL_MAP.md) | ⚙️ Gerado + notas manuais | 🔴 CRÍTICA |
| ZIR Parser | [zir/ZIR_PARSER_MAP.md](zir/ZIR_PARSER_MAP.md) | ⚙️ Gerado + notas manuais | 🟡 MÉDIA |
| ZIR Verifier | [zir/ZIR_VERIFIER_MAP.md](zir/ZIR_VERIFIER_MAP.md) | ⚙️ Gerado + notas manuais | 🟡 MÉDIA |
| HIR Lowering | [hir/lowering/HIR_LOWERING_MAP.md](hir/lowering/HIR_LOWERING_MAP.md) | ⚙️ Gerado + notas manuais | 🟡 MÉDIA |
| Semantic Binder | [semantic/binder/BINDER_MAP.md](semantic/binder/BINDER_MAP.md) | ⚙️ Gerado + notas manuais | 🔴 CRÍTICA |
| Type System | [semantic/types/TYPE_SYSTEM_MAP.md](semantic/types/TYPE_SYSTEM_MAP.md) | ⚙️ Gerado + notas manuais | 🔴 CRÍTICA |
| Symbol Table | [semantic/symbols/SYMBOLS_MAP.md](semantic/symbols/SYMBOLS_MAP.md) | ⚙️ Gerado + notas manuais | 🟡 MÉDIA |
| Diagnostics | [semantic/diagnostics/DIAGNOSTICS_MAP.md](semantic/diagnostics/DIAGNOSTICS_MAP.md) | ⚙️ Gerado + notas manuais | 🟢 BAIXA |
| C Emitter | [targets/c/EMITTER_MAP.md](targets/c/EMITTER_MAP.md) | ⚙️ Gerado + notas manuais | 🔴 CRÍTICA |
| Project/ZDoc | [project/PROJECT_MAP.md](project/PROJECT_MAP.md) | ⚙️ Gerado + notas manuais | 🟢 BAIXA |
| Formatter | [tooling/FORMATTER_MAP.md](tooling/FORMATTER_MAP.md) | ⚙️ Gerado + notas manuais | 🟢 BAIXA |
| Utils | [utils/UTILS_MAP.md](utils/UTILS_MAP.md) | ⚙️ Gerado + notas manuais | 🟢 BAIXA |

### Runtime & Stdlib
| Módulo | Arquivo de Mapa | Status | Prioridade |
|--------|----------------|--------|------------|
| Runtime C | [runtime/c/RUNTIME_MAP.md](../runtime/c/RUNTIME_MAP.md) | ⚙️ Gerado + notas manuais | 🔴 CRÍTICA |
| Stdlib | [stdlib/STDLIB_MAP.md](../stdlib/STDLIB_MAP.md) | ⚙️ Gerado + notas manuais | 🟡 MÉDIA |

## 📊 Estatísticas

- **Total de módulos**: 18
- **Blocos gerados**: 18/18 ✅
- **Notas manuais validadas**: 0/18

## 🎯 Como Usar

1. **Para debug**: Abra o mapa do módulo onde o erro ocorreu
2. **Para navegação**: Use o bloco `Generated Index` para achar arquivo, símbolo e linha
3. **Para features novas**: Consulte dependencies e testes relacionados
4. **Para IA**: Cole só o trecho do mapa e as linhas do código relevantes

## 📝 Legenda

- 🔴 **CRÍTICA**: Se quebrar, o compiler não funciona
- 🟡 **MÉDIA**: Afeta features específicas
- 🟢 **BAIXA**: Tooling, utilitários, não bloqueante
- ⬜ **Pendente**: Mapa ainda não criado
- ✅ **Completo**: Mapa revisado e validado

<!-- CODEMAP:GENERATED:BEGIN -->
## Generated Module Catalog

| Module | Priority | Source files | Symbols | Map |
| --- | --- | ---: | ---: | --- |
| Driver / CLI Code Map | Critical | 4 | 150 | [DRIVER_MAP.md](driver/DRIVER_MAP.md) |
| Lexer Code Map | Critical | 4 | 25 | [LEXER_MAP.md](frontend/lexer/LEXER_MAP.md) |
| Parser Code Map | Critical | 2 | 69 | [PARSER_MAP.md](frontend/parser/PARSER_MAP.md) |
| AST Code Map | Medium | 2 | 9 | [AST_MAP.md](frontend/ast/AST_MAP.md) |
| ZIR Model Code Map | Critical | 2 | 100 | [ZIR_MODEL_MAP.md](zir/ZIR_MODEL_MAP.md) |
| ZIR Parser Code Map | Medium | 2 | 23 | [ZIR_PARSER_MAP.md](zir/ZIR_PARSER_MAP.md) |
| ZIR Verifier Code Map | Medium | 2 | 35 | [ZIR_VERIFIER_MAP.md](zir/ZIR_VERIFIER_MAP.md) |
| HIR Lowering Code Map | Medium | 2 | 66 | [HIR_LOWERING_MAP.md](hir/lowering/HIR_LOWERING_MAP.md) |
| Binder Code Map | Critical | 2 | 28 | [BINDER_MAP.md](semantic/binder/BINDER_MAP.md) |
| Type System Code Map | Critical | 4 | 98 | [TYPE_SYSTEM_MAP.md](semantic/types/TYPE_SYSTEM_MAP.md) |
| Symbols Code Map | Medium | 2 | 9 | [SYMBOLS_MAP.md](semantic/symbols/SYMBOLS_MAP.md) |
| Diagnostics Code Map | Low | 3 | 41 | [DIAGNOSTICS_MAP.md](semantic/diagnostics/DIAGNOSTICS_MAP.md) |
| C Emitter Code Map | Critical | 4 | 203 | [EMITTER_MAP.md](targets/c/EMITTER_MAP.md) |
| Project / ZDoc Code Map | Medium | 4 | 77 | [PROJECT_MAP.md](project/PROJECT_MAP.md) |
| Formatter Code Map | Low | 2 | 13 | [FORMATTER_MAP.md](tooling/FORMATTER_MAP.md) |
| Utils Code Map | Low | 7 | 36 | [UTILS_MAP.md](utils/UTILS_MAP.md) |
| Runtime Code Map | Critical | 2 | 592 | [RUNTIME_MAP.md](../runtime/c/RUNTIME_MAP.md) |
| Stdlib Code Map | Medium | 32 | 660 | [STDLIB_MAP.md](../stdlib/STDLIB_MAP.md) |

### Totals

- Source files indexed: 82
- Symbols indexed: 2234

### Workflow

1. Open the closest module map.
2. Find the file and top-level symbol.
3. Jump to the line in the source file.
4. Add manual notes only for real risks or recurring bugs.
<!-- CODEMAP:GENERATED:END -->
