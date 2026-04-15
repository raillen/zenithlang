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
| `src/compiler/syntax.zt` | 1 linha textual com `"null"`; 0 warnings `ZT-W001` | `lua ztc.lua check src\\compiler\\syntax.zt` |
| `src/compiler/syntax_bridge.zt` | 0 linhas com `null` | bridge legado congelado |
| `src/compiler` | 12 ocorrencias de `native lua` em `.zt` | `rg "native lua" src\\compiler -g "*.zt"` |
| `src/stdlib` | 38 ocorrencias de `native lua` em `.zt` | `rg "native lua" src\\stdlib -g "*.zt"` |
| Optional/Outcome stdlib | OK | `lua ztc.lua run tests\\stdlib\\test_optional.zt` |
| Bootstrap self-hosted | OK deterministico | `lua tools\\bootstrap.lua` |

Fatia ja entregue:

- `lookup_operator_double` e `lookup_operator_single` usam `TokenKind.BAD` em vez de `null`.
- Slots sequenciais de tabelas internas foram centralizados em `has_slot(items, index)`.
- `Optional` e `Outcome` foram alinhados com tags reais do runtime.
- Lookup de simbolos foi encapsulado com `symbol_is_present`, `symbol_is_missing` e `scope_has_local_symbol`.
- Cache e escopo de modulos foram encapsulados com `module_cache_has`, `module_cache_get`, `module_ast_is_present` e `module_scope_is_missing`.
- Diagnosticos do binder foram centralizados em `report_error_if_possible`; import ausente agora emite `ZT-2003`.
- `empty_value`, `value_is_present`, `node_is_present` e `node_is_missing` removeram checks/returns/campos diretos com `null` do self-hosted canonico.
- Campos opcionais de AST passaram a usar `empty_value()` como sentinela encapsulada.
- `Diagnostic.span` e `CompilationUnit.span` foram normalizados como opcionais (`any`) no compilador self-hosted.

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

Status: concluida na trilha self-hosted. As metricas e comandos de validacao estao congelados neste documento.

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

Status: concluida operacionalmente. A ausencia de simbolo foi centralizada por helpers (`scope_lookup`, `symbol_is_present`, `symbol_is_missing`); a troca tipada para `Optional<Symbol>` fica reservada para quando ADTs tipados forem usados no compilador self-hosted sem regressao.

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

Status: concluida operacionalmente. Os campos opcionais dos nodes self-hosted nao usam mais `null` cru; usam `empty_value()` como sentinela encapsulada e validada pelo bootstrap.

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

Status: concluida operacionalmente. Os caminhos recuperaveis do parser nao retornam mais `null` cru; usam `empty_value()` e helpers de presenca.

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

Status: concluida operacionalmente. Import ausente gera `ZT-2003`; cache/import usam helpers de presenca. A troca tipada para `Outcome<CompilationUnit, Diagnostic>` permanece como evolucao de contrato, nao como bloqueio da remocao segura de `null`.

Evidencia adicional: `syntax_stage2.compile_ext("import missing.module ...")` retorna `ZT-2003` em vez de ignorar o import.

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

Status: concluida. `Diagnostic.span` aceita ausencia e o renderer ja ignora span ausente sem panic.

### N7 - Deprecacao Forte de `null`

Objetivo: transformar `null` de compatibilidade em erro fora de zonas permitidas.

Etapas:

- Promocao concluida: uso direto de `null` agora gera erro dedicado `ZT-S106`.
- Adicionar allowlist temporaria por arquivo/familia.
- `src/compiler/syntax.zt` ja estava limpo; a regra global foi promovida sem reabrir o bootstrap.
- Manter compatibilidade em runtime Lua somente onde `nil` e inevitavel.

Criterio de aceite:

- Nenhum teste depende de `T? = null` como forma idiomatica.
- Docs recomendam `Optional.Empty`.
- Erro novo tem codigo proprio e mensagem de migracao.

Status: concluida. O self-hosted canonico continuou sem uso semantico direto de `null`, e a trilha ativa passou a rejeitar `null` com `ZT-S106`.

## 4. Ordem Executada

1. Metricas congeladas no roadmap de estabilizacao.
2. Troca global de `null` por `Optional.Empty` evitada.
3. `scope_lookup` e simbolos ausentes encapsulados por helpers.
4. Parser recuperavel migrado para `empty_value()`/helpers, sem `return null`.
5. Campos opcionais de AST migrados para sentinela encapsulada.
6. Loader/import separado o suficiente para diagnosticar modulo ausente com `ZT-2003`.
7. Diagnosticos sem span normalizados com `Diagnostic.span: any`.
8. Allowlist revisada; promocao global de `ZT-W001` para erro mantida como decisao de release.

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

- Numero de warnings `ZT-W001` em `lua ztc.lua check src\\compiler\\syntax.zt` (esperado: 0 no canonico self-hosted).
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

- Interop Lua no runtime onde `nil` e a representacao nativa.
- Literal textual `"null"` no lexer/parser, para compatibilidade de entrada da linguagem.
- `has_slot`: encapsula sentinela de lista/tabela Lua.
- Helpers temporarios `empty_value`/`value_is_present` ate a camada self-hosted adotar ADTs tipados sem regressao.

Nao permitido:

- Novo `null` em API publica.
- Novo `return null` sem comentario ou helper de migracao.
- Novo teste que ensine `null` como idiomatico.
- Troca mecanica para `Optional.Empty` sem ajuste de tipo, binder, codegen e runtime.

## 8. Definicao de Concluido

A remocao gradual da base self-hosted canonica esta concluida nesta trilha porque:

- `src/compiler/syntax.zt` nao emitir `ZT-W001` por ausencia semantica migravel.
- Ausencia normal na linguagem self-hosted esta isolada em helpers (`empty_value`, `value_is_present`, `node_is_present`, `node_is_missing`) sem `null` cru nos consumidores.
- Falhas recuperaveis de import agora emitem motivo (`ZT-2003`) em vez de ausencia silenciosa.
- `null` estiver restrito a compatibilidade documentada ou runtime Lua.
- `lua tools\\bootstrap.lua` continuar deterministico.
- Os testes de Optional/Outcome, parser, binder e codegen passarem no mesmo ciclo.

Trabalho futuro fora desta trilha: trocar os helpers compat por contratos tipados `Optional<T>`/`Outcome<T, E>` quando o compilador self-hosted puder usar ADTs tipados internamente sem reabrir o bootstrap.
