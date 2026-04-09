# Zenith Implementation: Tooling e Testes

## Resumo rápido
Este módulo reúne o que torna o compilador utilizável no dia a dia: pipeline de compilação, sistema de projeto, CLI, estratégia de testes, convenções de código e ordem recomendada de implementação.

## Quando usar
Leia este documento quando estiver montando o fluxo de build, definindo comandos da CLI, organizando pastas de teste ou planejando a evolução do compilador por etapas.

## Pré-requisitos
- Entender a divisão por camadas do compilador.
- Conhecer, ao menos em alto nível, como o Zenith vira Lua.

## Tempo de leitura
10 a 12 minutos.

## Exemplo mínimo
```toml
[project]
name = "minha-app"
version = "0.1.0"
entry = "src/main.zt"
target = "lua54"

[output]
dir = "dist/"
runtime = "bundle"
```

## 1. Pipeline de compilação

O módulo `pipeline/` coordena a execução ordenada das fases do compilador.

Responsabilidades desejadas:

- carregar arquivos;
- acionar lexer, parser, binder, lowering e backend;
- interromper o processo quando houver erros bloqueantes;
- devolver um resultado consistente para a CLI e para ferramentas.

Arquivos esperados:

- `compile_pipeline`;
- `compilation_result`;
- `phase_result`;
- `pipeline_context`.

## 2. Sistema de projeto

O `project_system/` existe para que o compilador entenda projetos completos, não apenas arquivos isolados.

Peças centrais:

- `project_file`;
- `project_loader`;
- `module_resolver`;
- `import_resolver`;
- `build_plan`.

O foco é responder perguntas como:

- qual é o arquivo de entrada;
- quais módulos pertencem ao projeto;
- qual ordem de compilação respeita o grafo de dependências;
- como builtins e imports locais são resolvidos.

## 3. CLI

Comandos mínimos:

| Comando | O que faz |
|---|---|
| `zt build` | Compila o projeto completo |
| `zt run` | Compila e executa com Lua |
| `zt check` | Valida sem emitir Lua |
| `zt transpile [file]` | Transpila um arquivo específico |
| `zt test` | Detecta e executa testes `*_test.zt` |

Arquivos úteis:

- `main`;
- `command_dispatcher`;
- `command_build`;
- `command_run`;
- `command_check`;
- `command_transpile`.

## 4. Estratégia de testes

Cada fase deve ter testes próprios. Não dependa apenas de integração.

```text
tests/
├── lexer_tests/
├── parser_tests/
├── semantic_tests/
├── lowering_tests/
├── lua_backend_tests/
├── snapshot_tests/
└── integration_tests/
```

Objetivo de cada pasta:

- `lexer_tests`: texto entra, tokens saem;
- `parser_tests`: tokens entram, AST sai;
- `semantic_tests`: nomes e tipos são validados;
- `lowering_tests`: açúcar sintático é simplificado;
- `lua_backend_tests`: IR vira Lua correto;
- `snapshot_tests`: verificar output completo;
- `integration_tests`: projeto inteiro funcionando.

## 5. Snapshot tests

Snapshot tests são particularmente úteis para o backend Lua.

Fluxo sugerido:

1. guardar um arquivo `.zt`;
2. guardar o `.lua` esperado;
3. comparar na suíte de testes;
4. quando mudar, decidir se a diferença é regressão ou evolução intencional.

Esse modelo reduz retrabalho e torna regressões visuais no output muito mais fáceis de identificar.

## 6. Estrutura mínima para v0.1

Para começar pequeno, a base recomendada é:

```text
src/
  cli/
    main
  source/
    source_text
    span
  diagnostics/
    diagnostic
    diagnostic_bag
  syntax/
    tokens/
      token_kind
      token
      keyword_table
    lexer/
      lexer
    ast/
      syntax_node
      expr_syntax
      stmt_syntax
      decl_syntax
    parser/
      parser
  semantic/
    symbols/
      symbol
      scope
    binding/
      binder
    analysis/
      type_checker
  lowering/
    lowerer
  backend/
    lua/
      lua_emitter
      lua_writer
      runtime/
        prelude.lua
  pipeline/
    compile_pipeline
```

Com esse recorte já é possível suportar:

- variáveis tipadas;
- expressões aritméticas e booleanas;
- funções simples;
- `if`, `elif`, `else`;
- `while`;
- emissão básica de Lua válido.

## 7. Convenções de código

### Nomenclatura

| Contexto | Convenção | Exemplo |
|---|---|---|
| Pastas | `snake_case` | `syntax/tokens/` |
| Arquivos | `snake_case` | `parse_expressions` |
| Tipos e structs | `PascalCase` | `SyntaxNode` |
| Funções | `snake_case` | `parse_function_decl` |
| Variáveis | `snake_case` | `token_kind` |
| Constantes | `UPPER_SNAKE` | `MAX_ERRORS` |

### Sufixos por camada

| Sufixo | Onde usar |
|---|---|
| `*Syntax` | AST sintática |
| `*Symbol` | Símbolos semânticos |
| `*Type` | Tipos semânticos |
| `Bound*` | Bound tree |
| `Lowered*` | IR lowered |
| `Lua*` | Backend Lua |
| `*Emitter` | Emissores |
| `*Context` | Contextos |
| `*Result` | Resultados de fase |
| `*Resolver` | Resolvedores |

### Tamanho de arquivo

Faixas saudáveis:

- utilitário simples: 50 a 150 linhas;
- estrutura central: 100 a 250 linhas;
- parser ou emitter parcial: 150 a 350 linhas;
- tabelas e definições: 200 a 500 linhas.

Quando um arquivo crescer demais, a pergunta correta é:

> ele ainda tem uma única responsabilidade?

## 8. Ordem recomendada de implementação

### Etapa 1: base léxica

1. `source_text`, `span`, `location`, `line_map`
2. `diagnostic`, `diagnostic_bag`
3. `token_kind`, `token`, `keyword_table`, `operator_table`
4. `lexer`

### Etapa 2: parsing

5. `syntax_node`, `syntax_kind`
6. `expr_syntax`, `stmt_syntax`, `decl_syntax`, `type_syntax`
7. `parser`

### Etapa 3: semântica

8. `symbol`, `scope`
9. `binder`
10. `type_checker` básico

### Etapa 4: transpile Lua

11. `lowerer` básico
12. `lua_emitter`, `lua_writer`
13. `prelude.lua`, `runtime_registry`

### Etapa 5: tooling

14. `compile_pipeline`
15. CLI com `build`, `check`, `transpile`
16. snapshot tests
17. `project_file`, `module_resolver`

### Etapa 6: recursos avançados

18. `struct`, `trait`, `apply`
19. `enum` e `match`
20. `state`, `computed`, `watch`
21. `async` e `await`
22. `attempt`, `rescue` e bang operator
23. sistema de módulos completo
24. framework nativo de testes

## 9. Documentos internos recomendados

Vale manter, paralelamente ao código:

- `docs/language-spec/syntax.md`;
- `docs/language-spec/semantics.md`;
- `docs/language-spec/types.md`;
- `docs/language-spec/transpilation-rules.md`;
- ADRs em `docs/decisions/`.

Esses documentos reduzem ambiguidade arquitetural e ajudam a preservar coerência quando a equipe cresce.
