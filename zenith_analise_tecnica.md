# Análise Técnica da Linguagem Zenith (zt)
### Perspectiva de Engenheiro Sênior — Abril 2026

> **Metodologia**: Análise direta do código-fonte (`syntax_test.zt`, `ztc_bootstrap.lua`, `demo.zt`, `test_v04.zt`), documentação (`MANIFESTO.md`, CHANGELOG), estrutura de diretórios (`src/`, `tests/`), e Knowledge Items internos. Linguagens de referência consultadas: Lua, Rust, Nim, Elixir, Gleam, Crystal.

---

## 1. Visão Geral e Posicionamento

Zenith é uma linguagem **transpilada para Lua** que tenta oferecer tipagem estática gradual, composição via Traits, manipulação segura de erros e uma sintaxe de baixo atrito cognitivo. O projeto tem **vocação clara**: servir de camada de abstração sobre o ecossistema Lua (jogos, automação, embeddable) com uma experiência de desenvolvimento superior.

**Isso é ambicioso e legítimo.** O problema é que a lacuna entre a visão articulada no Manifesto e o estado real do compilador é **significativa e precisa ser encarada com honestidade**.

---

## 2. Pontos Fortes ✅

### 2.1 Filosofia de Design Coerente
O princípios de "baixo atrito neural", "explícito sobre implícito", e acessibilidade para neurodivergentes são **bem fundamentados e consistentes** ao longo da sintaxe. A ausência de parênteses em `if`, o uso de `end` como delimitador, e a preferência por palavras-chave textuais (`and`, `or`, `not`) ao invés de símbolos (`&&`, `||`, `!`) são escolhas defensáveis e coerentes com o manifesto.

### 2.2 Sistema de Tratamento de Erros
`Optional<T>` e `Outcome<T, E>` são bem modelados. A implementação em Lua no runtime (`zenith_rt.lua`) está correta e o `_tag` diferenciado permite pattern matching funcional. O operador `?` para propagação é elegante na teoria.

### 2.3 O Compilador Existe e Funciona Parcialmente
O compilador bootstrap (`ztc_bootstrap.lua`, ~6.600 linhas) é um artefato técnico real. Ele implementa Lexer → Parser → Binder → Emitter de forma funcional para um subconjunto da linguagem. Isso é non-trivial para um projeto individual.

### 2.4 Pipeline Conceitual Correto
A separação em fases (Lexing → Parsing → Binding → Lowering → Codegen) é academicamente correta e reflete entendimento sólido de teoria de compiladores.

### 2.5 stdlib com Substância
A `std.text`, `std.os`, `std.io` têm implementações reais e úteis — Unicode parcial via heurísticas, `mask`, `slugify`, `truncate`. Isso é mais do que a maioria dos projetos similares entrega.

### 2.6 Ecossistema Conceitualmente Planejado
A presença de Compass (LSP), ZTest (test runner), ZPM (package manager), e o `.ztproj` mostram pensamento sistêmico. A maioria das linguagens de um único desenvolvedor nunca chega a esse nível de planejamento ecossistêmico.

---

## 3. Incongruências e Problemas Críticos ❌

### 3.1 — O Sistema de Tipos é uma Ilusão em Runtime

> **Severidade: CRÍTICA**

A linguagem declara tipagem estática (`var health: int`, `func foo(x: int) -> bool`), mas o compilador **não verifica tipos em nenhuma fase**. No `Parser`, o tipo é consumido e descartado:

```zt
-- syntax_test.zt, linha 938-940
var p = @expect(TokenKind.Identifier, "esperado parâmetro")
if @peek().kind == TokenKind.Colon
    @advance() -- :
    @advance() -- type  ← tipo simplesmente descartado!
end
```

No `Binder`, todos os símbolos têm `type_name: "any"`. O `Emitter` não emite nenhuma verificação de tipo. **A tipagem é cosmética.** Isso torna as promessas do manifesto — "contratos de dados", "segurança por padrão", "abstrações de custo zero" — **falsas no estado atual**.

**Impacto real**: Um usuário escreve `var age: int = "vinte"` e o programa compila sem erro.

---

### 3.2 — Uso Pervasivo de `any` no Próprio Código do Compilador

> **Severidade: ALTA**

O código mais crítico do projeto — o compilador em si — usa `any` em praticamente todos os campos das structs de AST:

```zt
pub struct DeclNode
    pub kind: NodeKind
    pub name: text
    pub params: any       -- qualquer coisa
    pub body: any         -- qualquer coisa
    pub fields: any       -- qualquer coisa
    pub methods: any      -- qualquer coisa
    pub members: any      -- qualquer coisa
    ...
end
```

O compilador **não confia no seu próprio sistema de tipos**. Isso é uma inconsistência filosófica grave: como vender tipagem segura se o código que gera essa tipagem a evita completamente?

---

### 3.3 — `null` Ainda Existe, Quebrando a Proposta de `Optional<T>`

> **Severidade: ALTA**

O manifesto diz que Zenith "elimina null". Mas no código real:

```zt
-- syntax_test.zt, linhas 134, 447, 500, 534...
if @diagnostics == null @diagnostics = [] end
return CompilationUnit { ..., span: null, ... }
var node: any = null
var init: any = null
```

`null` aparece **dezenas de vezes** no compilador. Existe uma dicotomia perigosa: a linguagem *descreve* `Optional<T>`, mas o próprio compilador usa `null` livremente. O runtime (`zenith_rt.lua`) também usa `nil` para representar ausência em vários pontos. A barreira é semântica, não real.

---

### 3.4 — A Sintaxe de `native lua` é um Buraco na Abstração

> **Severidade: ALTA**

`native lua` não é apenas um escape hatch para casos extremos — ele é **usado extensivamente no próprio demo oficial**:

```zt
-- demo.zt — o arquivo de demonstração principal
native lua
    if input_opt and input_opt.is_present() then
        opt = input_opt:unwrap_or("0")
    else
        running = false
    end
end
```

Se a linguagem de demonstração precisa de blocos `native lua` para funcionar, a abstração está insuficientemente madura. Isso cria dois problemas:
1. O custo cognitivo para o usuário é dobrado (aprende Zenith + Lua)
2. O código dentro do `native lua` é opaco ao compilador, quebrando análise semântica, type checking futuro e otimizações

---

### 3.5 — A Sintaxe de `@campo` vs Acesso de Membro Cria Ambiguidade Consistente

> **Severidade: MÉDIA-ALTA**

O uso de `@` para acesso a campos de `self` é inovador mas cria problemas práticos:

```zt
-- Dentro de um método:
@pos = @pos + 1     -- self.pos
@tokens[@pos]       -- self.tokens[self.pos]
```

Porém, `@` é também o token de **atributos/decoradores** (`@windows`, `@linux`). O parser trata o contexto para diferenciar, mas a ambiguidade visual é real e a análise de `is_assign_stmt` usa lookahead frágil:

```zt
func is_assign_stmt() -> bool
    var t1 = @peek().kind
    if t1 == TokenKind.At return @tokens[@pos + 2].kind == TokenKind.Equal end
    ...
```

Acesso direto a índice do array de tokens no meio de lógica de parsing é um sinal de design frágil.

---

### 3.6 — O Binder Não Trata Múltiplas Variáveis no `for`, `match` com Guards, ou Closures

> **Severidade: MÉDIA**

O `_bind_node` no `Binder` só registra a primeira variável em `ForInStmt`, não há suporte a destructuring no for, e closures/lambdas estão ausentes como conceito. A propagação semântica para `MatchStmt` só faz binding superficial dos patterns sem resolução de variáveis capturadas.

---

### 3.7 — Numeração de Arrays com Índice 0 vs Lua (Índice 1)

> **Severidade: MÉDIA**

Zenith usa arrays 0-indexed (`@tokens[@pos]`), mas Lua é 1-indexed. O compilador faz a conversão em alguns lugares mas não em outros. O `Emitter` gera código com índices inconsistentes dependendo do contexto. No `slice`:

```lua
-- zenith_rt.lua
function zt.slice(obj, start_v, end_v)
    -- usa start_v/end_v como 1-indexed para strings, mas 1-indexed para tabelas
```

Mas a sintaxe Zenith `list[0..2]` implica 0-indexed. Essa impedância semântica vai gerar bugs sutis.

---

### 3.8 — O `Emitter` Possui Lógica de `_is_platform_allowed` Duplicada

> **Severidade: BAIXA-MÉDIA (Code Smell)**

A função `_is_platform_allowed` é implementada identicamente em `Binder` **e** em `Emitter`. Isso viola DRY e significa que futuras mudanças de lógica de plataforma precisarão ser sincronizadas manualmente em dois lugares.

---

### 3.9 — `native lua` no `lookup_operator_single` é um Hack Crítico

> **Severidade: MÉDIA**

```zt
-- syntax_test.zt, linhas 377-378
if op == (native lua string.char(123) end) return TokenKind.LBrace end
if op == (native lua string.char(125) end) return TokenKind.RBrace end
```

O compilador não consegue comparar `{` e `}` diretamente em strings Zenith porque o próprio bootstrapping confunde o tokenizer com esses caracteres. Isso é um problema de **autoconsistência**: a linguagem não consegue se descrever completamente em si mesma ainda, o que é um bloqueador crítico para o objetivo de "compilador escrito em Zenith".

---

### 3.10 — Sistema de Generics Existe na Sintaxe, Mas Não no Compilador

> **Severidade: ALTA**

`list<text>`, `Optional<T>`, `func process<T>(item: T) where T is Serializable` são aceitos pelo parser (`parse_type` consome `<...>`), mas são **completamente ignorados** no backend. O emitter nunca diferencia `list<int>` de `list<text>`. Generics são açúcar lexical, não um sistema real.

---

## 4. Riscos Arquiteturais do Modelo Transpilado

O modelo **Zenith → Lua** é uma escolha pragmática mas com custos que precisam ser reconhecidos:

| Aspecto | Realidade |
|---|---|
| **Performance** | Limitada pelo Lua. Sem controle de alocação, sem inlining real. As "abstrações de custo zero" do manifesto são impossíveis numa transpilação. |
| **Debugging** | Erros em runtime aparecem com stack traces Lua, não Zenith. Source maps não existem ainda. |
| **Interop** | Depende de FFI do LuaJIT para C — não funciona em Lua 5.4 padrão. Módulos `extern` são LuaJIT-only. |
| **Ecosystem lock-in** | Bibliotecas Lua existem mas exigem `native lua`. Não há uma ponte idiomática. |
| **String interpolation** | `"{name}"` é uma feature planejada (test_v04) mas não implementada. O lexer não suporta. |

---

## 5. Estado do Tooling

| Ferramenta | Estado Real |
|---|---|
| `ztc` (compilador) | Funcional para subconjunto básico. Sem type checking. |
| `ztest` | Existe (`ztest.lua`) mas sem integração automática com CI |
| `Compass` (LSP) | Em desenvolvimento. Erros de bootstrap documentados em conversas anteriores. |
| `ZPM` | `zpm_engine.lua` existe (8KB) mas sem especificação pública de formato |
| `.ztproj` | Formato definido mas o compilador usa caminhos hardcoded em vários pontos |
| Source Maps | Não existem |
| REPL | Não existe |

---

## 6. Lacunas de Especificação

Funcionalidades descritas na documentação/KI mas ausentes ou incompletas no compilador:

- **UFCS** (`obj.func()` → `func(obj)`): Não implementado no Emitter
- **`check` statement**: Parser aceita mas Emitter não gera lógica real
- **`after` / defer**: Presente em `test_v04.zt` como feature desejada, mas ausente do parser principal
- **`const`**: Presente na documentação, mas não aparece em `TokenKind` enum — não existe no parser
- **Traits / `apply`**: Ausentes do compilador. Nenhum token `KwTrait` ou `KwApply` existe
- **Closures / lambdas**: Não existem
- **Destructuring em `var`**: Não implementado
- **String interpolation `{expr}`**: Planejado, não implementado
- **Validação `where it`**: Existe na spec, ausente no Emitter
- **`?` propagation operator**: Planejado, não implementado

---

## 7. Análise do Código Bootstrap

O `ztc_bootstrap.lua` (6.652 linhas) é um compilador escrito manualmente em Lua que serve como **Stage 0** para compilar o código Zenith que vai virar o compilador real. Isso é um bootstrapping clássico e correto metodologicamente.

**Problemas observados:**
1. O arquivo é um **bundle monolítico** — `package.preload` inline para dezenas de módulos — gerado automaticamente. Manutenção manual é impraticável.
2. Aliases de `package.preload` proliferam (5-6 aliases por módulo) para contornar inconsistências de paths entre Windows e Unix.
3. Há dois sistemas de scope coexistindo: um em `syntax_test.zt` na struct `Scope` e outro em `semantic.symbols.scope` do Lua — com implementações ligeiramente diferentes.

---

## 8. Avaliação do Manifesto vs Realidade

| Promessa do Manifesto | Realidade Atual |
|---|---|
| "Elimina null" | `null` em dezenas de lugares no próprio compilador |
| "Segurança por padrão" | Sem type checking real |
| "Abstrações de custo zero" | Impossível via transpilação; não implementado |
| "Compilador escrito em Zenith" | Stage 0 em Lua; Stage 1 parcialmente compilável |
| "Traits com implementações padrão" | Não existem no compilador |
| "Pattern matching com destructuring" | Match básico funciona; destructuring não |
| "LSP oficial (Compass)" | Em bootstrap com erros reportados |

---

## 9. Recomendações Priorizadas

### 🔴 Prioridade Imediata (Fundação)

1. **Implementar Type Checker mínimo viável**: Mesmo inferência básica (literal → tipo primitivo) transformaria as promessas de segurança em realidade. Comece com verificação de tipos em chamadas de função e atribuições.

2. **Eliminar `null` do compilador**: Substitua `any = null` por `Optional<T>` real. O compilador precisa **usar** a linguagem que ele compila — senão a filosofia é vazia.

3. **Implementar `trait` e `apply`**: São o coração da composição prometida. Sem isso, Zenith é apenas uma sintaxe alternativa sobre Lua sem o diferencial principal.

4. **Reduzir dependência de `native lua` no código de usuário**: Se o demo.zt principal precisa de blocos nativos, a stdlib está incompleta. Feche essas lacunas primeiro.

### 🟡 Médio Prazo (Consistência)

5. **Unificar o sistema de indexação**: Decida: 0-indexed ou 1-indexed. Documente explicitamente e garanta que o runtime e o compilador sejam consistentes.

6. **Implementar string interpolation**: `"{name}"` é esperado por qualquer usuário moderno. É também um teste ácido do lexer.

7. **Remover o hack `native lua string.char(123)`**: O compilador deve conseguir referenciar `{` e `}` na sua própria sintaxe. Resolva o problema de bootstrapping com escape sequences explícitas (`\{`).

8. **Deduplicate `_is_platform_allowed`**: Mover para módulo compartilhado.

### 🟢 Médio-Longo Prazo (Maturidade)

9. **Source maps**: Essencial para debugging. Sem isso, erros Lua aparecem com contexto irrelevante para o usuário.

10. **Generics reais com monomorphization ou type erasure**: Escolha um modelo e implemente. Generics decorativos prejudicam a confiança no sistema de tipos.

11. **UFCS no Emitter**: É uma feature de ergonomia poderosa que unifica a sintaxe. Vale o investimento.

12. **REPL mínimo**: Abaixa dramaticamente a barreira de entrada para novos usuários.

---

## 10. Veredicto Geral

Zenith é um **projeto seriamente concebido por um desenvolvedor com visão clara**, mas que enfrenta o desafio clássico de prometer v1.0 enquanto entrega v0.1. A diferença não é depreciativa — é um problema de **comunicação de expectativas**.

**O que funciona bem hoje**: sintaxe, filosofia, runtime básico, stdlib parcial, compilador funcional para código simples.

**O que não funciona como prometido**: type safety, traits, generics, eliminação de null, UFCS, string interpolation.

A maior ameaça não é técnica — é **a distância entre o manifesto e a realidade**. Isso pode gerar frustração em potenciais colaboradores e usuários. A recomendação estratégica é **rebaixar o manifesto para alpha/experimental** e concentrar os próximos 3 meses exclusivamente em fechar as lacunas da fundação, especialmente o type checker.

> A linguagem tem osso. O que falta é honestidade sobre onde ela está no mapa, e disciplina para construir uma fundação antes de ampliar o teto.

---
*Análise gerada em: 2026-04-14*
*Arquivos analisados: `syntax_test.zt` (1642 linhas), `ztc_bootstrap.lua` (6652 linhas), `demo.zt`, `test_v04.zt`, `MANIFESTO.md`, `CHANGELOG.md`, KIs internos.*
