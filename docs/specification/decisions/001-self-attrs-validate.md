# RFC 001 - `@` para self, `#[...]` para atributos de declaracao, `validate` para predicados

- Status: implementada na trilha ativa (transicao legada em curso)
- Verificado em: 2026-04-14
- Tipo: linguagem / sintaxe
- Escopo: parser, binder, docs, stdlib

## Resumo

Esta proposta separa tres conceitos que hoje colidem no parser e na leitura humana:

- `@nome` passa a existir somente como sugar para acesso a `self`.
- `#[...]` passa a ser a sintaxe unica de atributos de declaracao.
- `validate` passa a declarar validadores booleanos reutilizaveis em campos.

Objetivo principal: reduzir ambiguidade sintatica e tornar a linguagem mais previsivel.

## Implementacao validada

A trilha ativa foi revalidada com as seguintes garantias:

- `@campo` em metodos continua sendo sugar de `self.campo` e nao conflita mais com atributos de declaracao
- `#[...]` entra no parser como caminho sintatico proprio
- `@atributo` legado ainda e aceito, mas emite `ZT-W003`
- `validate` baixa para predicados booleanos sobre `it`
- `where` e `validate` podem aparecer em qualquer ordem no campo
- referencias qualificadas como `validation.min_value(18)` sao aceitas
- o MVP de `std.validation` ja existe na trilha ativa

## Estado atual x proposta

Este RFC agora descreve a direcao adotada pela trilha ativa e o estado de transicao legado.

Estado atual relevante na implementacao ativa:

- `@campo` ja existe e permanece como sugar de `self.campo`
- `#[...]` ja faz parte da gramatica para atributos de declaracao
- `validate` ja existe em campos de `struct` e baixa para contratos booleanos
- `std.validation` ja existe como MVP em modulo unico
- imports qualificados como `validation.min_value(18)` ja resolvem no binder/codegen
- `@atributo` legado ainda e aceito por compatibilidade, mas emite warning `ZT-W003`

Leitura correta deste documento:

- a sintaxe nova ja esta implementada na trilha ativa
- o legado `@atributo` segue temporariamente como caminho de migracao
- o passo pendente de linguagem e remover o legado da gramatica oficial quando a base estiver pronta

## Motivacao

No estado atual, `@` tenta servir a tres papeis:

- atributo de declaracao: `@windows`
- atributo de validacao: `@min(18)`
- sugar de instancia: `@health`

Isso cria duas dores:

1. Ambiguidade no parser.
2. Sobrecarga cognitiva para quem le o codigo.

A proposta assume que:

- `@` pertence ao dominio de expressao, como atalho para `self`.
- atributos de declaracao pertencem ao dominio de metadados da declaracao.
- validacao pertence ao dominio semantico de regras booleanas sobre valores.

## Decisoes

### 1. `@` fica restrito a `self`

Casos validos:

```zt
@health = 100
@reset()
print(@name)
```

Equivalencias:

```zt
self.health = 100
self:reset()
print(self.name)
```

Nao e mais permitido usar `@` para atributos de declaracao.

### 2. `#[...]` vira a sintaxe de atributos de declaracao

Casos validos:

```zt
#[windows]
func main()
end
```

```zt
#[deprecated("use new_main")]
func old_main()
end
```

```zt
#[ffi.link("ssl")]
extern func open()
```

Esses atributos sao metadados da declaracao seguinte. Eles nao representam expressao executavel.

### 3. `validate` vira a sintaxe de validacao declarativa

Casos validos:

```zt
struct User
    age: int validate min_value(18), max_value(120)
    email: text validate email
    name: text validate non_empty, len_max(80)
end
```

`validate` aceita somente referencias a predicados booleanos:

- nome de funcao: `email`
- chamada de funcao: `len_max(80)`

O valor do campo e passado implicitamente como primeiro argumento via `it`.

## Gramatica proposta

```ebnf
declaration
  ::= declaration_attribute* base_declaration

declaration_attribute
  ::= "#[" attribute_item ("," attribute_item)* "]"

attribute_item
  ::= qualified_name
   |  qualified_name "(" argument_list? ")"

qualified_name
  ::= IDENTIFIER ("." IDENTIFIER)*

argument_list
  ::= expression ("," expression)*

field_declaration
  ::= declaration_attribute* visibility? IDENTIFIER ":" type field_tail*

field_tail
  ::= default_clause
   |  validate_clause
   |  where_clause

default_clause
  ::= "=" expression

validate_clause
  ::= "validate" validator_ref ("," validator_ref)*

validator_ref
  ::= qualified_name
   |  qualified_name "(" argument_list? ")"

where_clause
  ::= "where" expression
```

## Semantica de `validate`

`validate` e sugar para composicao de predicados booleanos com `and`.

Regras:

- cada validador precisa retornar `bool`
- o campo atual entra como primeiro argumento implicito
- `validate` nao aceita expressoes arbitrarias
- expressoes livres continuam pertencendo a `where`

Exemplos de lowering:

```zt
age: int validate min_value(18), max_value(120)
```

equivale a:

```zt
age: int where min_value(it, 18) and max_value(it, 120)
```

```zt
email: text validate email
```

equivale a:

```zt
email: text where email(it)
```

```zt
name: text validate non_empty, len_max(80) where not text.contains(it, "admin")
```

equivale a:

```zt
name: text where non_empty(it) and len_max(it, 80) and not text.contains(it, "admin")
```

## Regras da implementacao ativa

Na trilha ativa:

- `validate` so e permitido em campos de `struct`
- cada `validator_ref` precisa resolver para funcao
- a funcao precisa retornar `bool`
- a funcao precisa aceitar pelo menos um argumento compativel com o tipo do campo
- os argumentos explicitos declarados no codigo sao anexados depois de `it`
- `where` e `validate` podem coexistir no mesmo campo
- a composicao final continua sendo um contrato booleano unico

Exemplos:

- `validate email` -> `email(it)`
- `validate len_max(80)` -> `len_max(it, 80)`
- `validate one_of(["draft", "ready"])` -> `one_of(it, ["draft", "ready"])`

Casos invalidos:

- `validate it > 10`
- `validate a + b`
- `validate make_rule()`

Para logica livre, usar `where`.

## Atributos de declaracao propostos

Conjunto inicial que a linguagem pode reconhecer formalmente:

- `#[windows]`
- `#[linux]`
- `#[macos]`
- `#[deprecated("msg")]`
- `#[ffi.link("name")]`
- `#[doc.hidden]`

Atributos fora desse conjunto podem inicialmente ser preservados na AST como metadado, mesmo sem efeito semantico imediato.

## Biblioteca padrao recomendada

Sim: criar `std.validation`.

Motivo:

- evita hardcode de `min`, `max`, `email` e similares dentro do compilador
- deixa `validate` pequeno e generico
- facilita reaproveitamento em DTOs, schemas, forms e APIs

Estrutura inicial sugerida:

- MVP: `std.validation`
- expansao posterior: `std.validation.text`
- expansao posterior: `std.validation.number`
- expansao posterior: `std.validation.collection`

Conjunto inicial sugerido:

- `min_value`
- `max_value`
- `between`
- `non_empty`
- `not_blank`
- `len_min`
- `len_max`
- `len_between`
- `matches`
- `email`
- `url`
- `uuid`
- `one_of`

Exemplos de assinatura:

```zt
namespace std.validation

pub func min_value(it: int, min: int) -> bool
pub func max_value(it: int, max: int) -> bool
pub func between(it: int, min: int, max: int) -> bool
pub func one_of<T>(it: T, values: list<T>) -> bool
```

```zt
namespace std.validation.text

pub func non_empty(it: text) -> bool
pub func not_blank(it: text) -> bool
pub func len_min(it: text, min: int) -> bool
pub func len_max(it: text, max: int) -> bool
pub func len_between(it: text, min: int, max: int) -> bool
pub func matches(it: text, pattern: text) -> bool
pub func email(it: text) -> bool
pub func url(it: text) -> bool
pub func uuid(it: text) -> bool
```

Observacao de nomenclatura:

- preferir `min_value` e `max_value` em vez de `min` e `max`
- isso evita colisao mental com funcoes de agregacao matematica

## Ordem recomendada de implementacao

A ordem sugerida foi executada assim na trilha ativa:

1. `#[...]` entrou no parser mantendo `@atributo` legado com warning `ZT-W003`.
2. `@` ficou reservado ao caminho de expressao para sugar de `self`.
3. `validate` passou a baixar para o mesmo contrato booleano usado por `where`.
4. O MVP de `std.validation` foi criado em modulo unico.
5. A separacao em submodulos continua opcional e so deve acontecer se o uso real justificar.

## Exemplos completos

### Antes

```zt
@windows
func main()
end

struct User
    @min(18) @max(120)
    age: int
end
```

### Depois

```zt
#[windows]
func main()
end

import std.validation as validation

struct User
    age: int validate validation.min_value(18), validation.max_value(120)
end
```

### Com `@` apenas para self

```zt
struct Player
    health: int

    func reset()
        @health = 100
    end
end
```

## Plano de migracao

Estado da migracao:

- concluido: aceitar `#[...]` no parser
- concluido: manter `@atributo` com warning de deprecacao (`ZT-W003`)
- concluido: manter `@campo` como sugar de `self`
- concluido: introduzir `validate`
- concluido: disponibilizar `std.validation`
- pendente: remover `@atributo` da gramatica oficial quando a base legada estiver pronta

## Impacto no compilador

Parser:

- remover ambiguidade entre atributos e `@campo`
- `@` deixa de participar do inicio de declaracao
- `#[...]` passa a abrir um caminho sintatico proprio

Binder:

- atributos de declaracao continuam em `node.attributes`
- `validate` baixa para predicados booleanos sobre `it`
- checagem de assinatura garante retorno `bool`

Stdlib:

- adicionar `std.validation`
- reaproveitar `std.text` e `std.text.regex` onde fizer sentido

## Perguntas em aberto

- `validate` deve continuar exclusiva de campos em V1 ou evoluir para parametros?
- atributos desconhecidos devem ser warning ou permanecer como metadado silencioso?
- quando remover de vez o legado `@atributo` sem quebrar a base self-hosted?

## Recomendacao

Adotar esta proposta em V1:

- `@` somente para `self`
- `#[...]` para atributos de declaracao
- `validate` somente para predicados booleanos
- `std.validation` como biblioteca oficial de validadores reutilizaveis, com MVP pequeno e expansao posterior

Essa combinacao minimiza ambiguidade, preserva legibilidade e mantem o compilador pequeno.
