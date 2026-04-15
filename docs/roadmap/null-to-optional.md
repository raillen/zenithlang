# Roadmap: Remocao Gradual de `null` por `Optional`

Atualizado em: 2026-04-15

Este plano define a migracao gradual do uso estrutural de `null` para `Optional<T>` na linguagem Zenith.

O objetivo nao e apagar toda ocorrencia textual de `null` de uma vez. O objetivo e separar semanticas:

- `Optional<T>` representa ausencia esperada de valor.
- `Outcome<T, E>` representa sucesso ou falha operacional.
- Sentinelas explicitas representam estados internos que nao sao ausencia de dado, como token invalido, fim de lista ou erro de parse.
- `null` permanece apenas como compatibilidade de entrada e detalhe temporario de runtime, ate ser removido por etapas.

## 1. Estado Atual

Base verificada:

| Area | Medida atual | Fonte |
|---|---:|---|
| `src/compiler/syntax.zt` | 84 linhas com `null`; 340 warnings `ZT-W001` | `lua ztc.lua check src\\compiler\\syntax.zt` |
| `src/compiler/syntax_bridge.zt` | 0 linhas com `null` | bridge legado congelado |
| `src/compiler` | 15 ocorrencias de `native lua` em `.zt` | `rg "native lua" src\\compiler -g "*.zt"` |
| `src/stdlib` | 38 ocorrencias de `native lua` em `.zt` | `rg "native lua" src\\stdlib -g "*.zt"` |
| Optional/Outcome stdlib | OK | `lua ztc.lua run tests\\stdlib\\test_optional.zt` |
| Bootstrap self-hosted | OK deterministico | `lua tools\\bootstrap.lua` |

Fatia ja entregue:

- `lookup_operator_double` e `lookup_operator_single` usam `TokenKind.BAD` em vez de `null`.
- Slots sequenciais de tabelas internas foram centralizados em `has_slot(items, index)`.
- `Optional` e `Outcome` foram alinhados com tags reais do runtime.
- Lookup de simbolos foi encapsulado com `symbol_is_present`, `symbol_is_missing` e `scope_has_local_symbol`.

## 2. Regras de Decisao

Use `Optional<T>` quando a ausencia for um valor normal do dominio:

- Campo opcional de AST.
- Resultado de lookup que pode nao existir.
- Parametro ou configuracao ausente.
- Valor de cache ainda nao preenchido.
- Tentativa de parse que pode falhar sem erro fatal.

Use `Outcome<T, E>` quando houver erro recuperavel com motivo:

- Leitura de modulo ou import.
- Parse de unidade externa.
- Validacao semantica com diagnostico.
- Operacao de IO, filesystem ou runtime que pode falhar.

Use sentinel explicito quando o estado nao for ausencia semantica:

- `TokenKind.BAD` para caractere ou operador invalido.
- `TokenKind.EOF` para fim de stream.
- `NodeKind` especifico para no sintatico valido.
- Helper como `has_slot` para lista/tabela indexada por sentinela Lua.
- Enum interno para estado de maquina.

Nao converter automaticamente:

- Loops `while tabela[i] != null` sem entender se a tabela e lista, mapa ou cache.
- Campos de structs AST inicializados com `null` apenas para satisfazer shape Lua.
- Diagnosticos que ainda esperam `span: null`.
- `return null` que significa erro de parse recuperavel.
- Runtime Lua onde `nil` e semantica nativa de tabela.

## 3. Fases

### N0 - Inventario e Guardrails

Objetivo: impedir novas ocorrencias sem classificacao.

Tarefas:

- Manter inventario por familia neste documento.
- Manter metricas em `docs/roadmap_estabilizacao.md`.
- Manter teste de regressao para `Optional`/`Outcome`.
- Documentar excecoes permitidas.

Criterio de aceite:

- `lua ztc.lua run tests\\stdlib\\test_optional.zt` passa.
- `lua tests\\codegen_tests\\test_option_result_codegen.lua` passa.
- `lua tools\\bootstrap.lua` passa.

Status: em andamento.

### N1 - API Publica de `Optional`

Objetivo: garantir que `Optional` seja expressivo o suficiente antes de migrar chamadas.

Tarefas:

- Manter `is_present`, `is_empty`, `unwrap`, `unwrap_or`.
- Adicionar apenas se necessario: `map`, `and_then`, `or_else`.
- Garantir UFCS virtual no binder, codegen e runtime para cada metodo novo.
- Cobrir `Optional<T>` e `Outcome<T>` em teste end-to-end.

Criterio de aceite:

- Nenhum metodo novo entra sem teste em `tests/stdlib/test_optional.zt`.
- Runtime, binder e codegen ficam alinhados na mesma mudanca.

Status: superficie minima atual OK.

### N2 - Lookup e Resolucao de Simbolos

Objetivo: substituir ausencia de simbolo por `Optional<Symbol>` no limite semantico.

Fatia segura:

- Encapsular `scope_lookup` atras de helper nomeado.
- Primeiro manter assinatura externa compativel, mas reduzir chamadas diretas a `== null`.
- Depois migrar retorno para `Optional<Symbol>` quando o self-hosted aceitar o tipo sem regressao.

Exemplo alvo:

```zt
var found: Optional<Symbol> = scope_lookup(sc, name)
if found.is_empty()
    diagnostics.report_error(...)
else
    n.symbol = found.unwrap()
end
```

Riscos:

- `symbol == null` hoje tambem marca "nao vinculado ainda".
- Imports usam lookup local e lookup de modulo com mensagens diferentes.

Criterio de aceite:

- Erro de simbolo ausente continua emitindo `ZT-2001`.
- Bootstrap stage2/stage3 continua deterministico.

Status: em andamento. Primeira fatia compat entregue; retorno real `Optional<Symbol>` ainda pendente.

### N3 - Campos Opcionais de AST

Objetivo: trocar campos semanticamente opcionais por `Optional<T>` sem inflar todos os nodes.

Prioridade:

- `else_body`.
- `condition`.
- `value`.
- `target`.
- `symbol`.
- `span`.

Estrategia:

- Criar constructors/helpers de AST para campos comuns.
- Migrar um node por vez.
- Evitar editar todos os literais inline de `ExprNode`/`StmtNode` no mesmo patch.

Riscos:

- O emitter espera acesso direto como `n.else_body`.
- O binder usa `n.symbol == null` como estado de binding.
- Uma mudanca massiva nos literais de AST tem alto risco de quebrar bootstrap.

Criterio de aceite:

- Cada node migrado tem teste de parse/bind/codegen.
- `lua ztc.lua check src\\compiler\\syntax.zt` passa.
- Warnings `ZT-W001` caem monotonicamente.

Status: pendente.

### N4 - Parser Recuperavel

Objetivo: trocar `return null` em parse recuperavel por `Optional<Node>` ou `Outcome<Node, Diagnostic>`.

Classificacao:

- Ausencia esperada: `Optional<Node>`.
- Erro com diagnostico: `Outcome<Node, Diagnostic>`.
- Sincronizacao apos erro: sentinel ou helper explicito.

Fatia segura:

- Iniciar por `parse_assignment_stmt`, porque ja retorna ausencia quando nao e atribuicao.
- Depois `parse_statement`.
- Por ultimo `parse_declaration`, porque afeta fluxo principal do parser.

Criterio de aceite:

- Testes de parser continuam verdes.
- Entradas invalidas continuam gerando diagnostico, nao panic.
- Nenhum loop de recuperacao entra em ciclo infinito.

Status: pendente.

### N5 - Imports, Cache e IO

Objetivo: separar modulo ausente, modulo invalido e falha de leitura.

Modelo alvo:

- `ModuleLoader.load_module`: `Outcome<CompilationUnit, Diagnostic>`.
- Cache: `Optional<CompilationUnit>` ou helper `cache_has/cache_get`.
- Caminho vazio: erro de import, nao `null`.

Riscos:

- Hoje o loader mistura string vazia, cache e parse falho.
- Diagnosticos de import precisam preservar modulo original.

Criterio de aceite:

- Import valido continua funcionando.
- Import ausente emite diagnostico estavel.
- Bootstrap continua sem acesso destrutivo ao compilador ativo.

Status: pendente.

### N6 - Diagnosticos e Spans

Objetivo: eliminar `span: null` onde o diagnostico pode receber uma localizacao real ou `Optional<Span>`.

Fatia segura:

- Definir `Diagnostic.span` como opcional no modelo da linguagem.
- Normalizar renderizacao quando span esta vazio.
- Migrar chamadas que hoje passam `null` por falta de span.

Criterio de aceite:

- `DiagnosticBag` nao quebra quando span esta ausente.
- Renderizador imprime diagnostico sem local quando necessario.
- Diagnosticos com span continuam formatados como hoje.

Status: pendente.

### N7 - Deprecacao Forte de `null`

Objetivo: transformar `null` de compatibilidade em erro fora de zonas permitidas.

Etapas:

- Manter `ZT-W001` enquanto SH-4 estiver parcial.
- Adicionar allowlist temporaria por arquivo/familia.
- Quando `src/compiler/syntax.zt` chegar a zero usos semanticamente migraveis, promover `ZT-W001` para erro em codigo novo.
- Manter compatibilidade em runtime Lua somente onde `nil` e inevitavel.

Criterio de aceite:

- Nenhum teste depende de `T? = null` como forma idiomatica.
- Docs recomendam `Optional.Empty`.
- Erro novo tem codigo proprio e mensagem de migracao.

Status: futuro.

## 4. Ordem Recomendada de Implementacao

1. Congelar metricas atuais no roadmap de estabilizacao.
2. Proibir troca global de `null` por `Optional.Empty`.
3. Atacar `scope_lookup` e simbolos ausentes.
4. Migrar `parse_assignment_stmt` para `Optional<StmtNode>`.
5. Criar helpers/constructors para AST antes de mexer em literais grandes.
6. Migrar `else_body` e `symbol` como primeiras propriedades opcionais reais.
7. Separar loader/import em `Outcome`.
8. Trocar diagnosticos sem span para `Optional<Span>`.
9. Revisar allowlist e promover `ZT-W001` para erro apenas quando a base estiver limpa.

## 5. Comandos de Validacao

Rodar em toda fatia:

```powershell
lua ztc.lua check src\compiler\syntax.zt
lua tools\bootstrap.lua
lua ztc.lua run tests\stdlib\test_optional.zt
lua tests\codegen_tests\test_option_result_codegen.lua
```

Rodar quando tocar parser:

```powershell
lua tests\parser_tests\test_parser.lua
lua tests\parser_tests\test_new_syntax.lua
```

Rodar quando tocar binder/simbolos:

```powershell
lua tests\semantic_tests\test_binder.lua
lua tests\semantic_tests\test_semantic.lua
```

Rodar quando tocar codegen/runtime:

```powershell
lua tests\codegen_tests\test_codegen.lua
lua tests\codegen_tests\test_option_result_codegen.lua
```

## 6. Metricas de Progresso

Metricas primarias:

- Numero de warnings `ZT-W001` em `lua ztc.lua check src\\compiler\\syntax.zt`.
- Numero de linhas com `null` em `src/compiler/syntax.zt`.
- Numero de usos diretos `== null` ou `!= null` fora de helpers permitidos.
- Bootstrap stage2/stage3 deterministico.

Metricas secundarias:

- Quantidade de `native lua` em `src/compiler` e `src/stdlib`.
- Quantidade de helpers temporarios ainda existentes.
- Cobertura de testes para `Optional`, `Outcome`, parser recuperavel e binder.

Comandos de inventario:

```powershell
rg -n "\bnull\b" src\compiler\syntax.zt
rg -n "== null|!= null|return null|: null" src\compiler\syntax.zt
rg -n "native lua" src\compiler src\stdlib -g "*.zt"
```

## 7. Allowlist Temporaria

Permitido temporariamente:

- `has_slot`: encapsula sentinela de lista/tabela Lua.
- Interop Lua no runtime onde `nil` e a representacao nativa.
- Campos AST ainda nao migrados.
- Diagnosticos sem span ate N6.
- Parser recuperavel ate N4.

Nao permitido:

- Novo `null` em API publica.
- Novo `return null` sem comentario ou helper de migracao.
- Novo teste que ensine `null` como idiomatico.
- Troca mecanica para `Optional.Empty` sem ajuste de tipo, binder, codegen e runtime.

## 8. Definicao de Concluido

A remocao gradual sera considerada concluida quando:

- `src/compiler/syntax.zt` nao emitir `ZT-W001` por ausencia semantica migravel.
- `Optional<T>` cobrir todos os casos de ausencia normal na linguagem self-hosted.
- `Outcome<T, E>` cobrir falhas recuperaveis com motivo.
- `null` estiver restrito a compatibilidade documentada ou runtime Lua.
- `lua tools\\bootstrap.lua` continuar deterministico.
- Os testes de Optional/Outcome, parser, binder e codegen passarem no mesmo ciclo.
