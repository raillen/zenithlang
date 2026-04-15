# Especificacao: Transpilacao da Trilha Ativa

Este documento resume como a trilha ativa mapeia Zenith para Lua hoje.

## 1. Declaracoes simples

Mapeamentos diretos:

- var x = 10 -> local x = 10
- const PI = 3.14 -> local PI = 3.14
- global g = {} -> _G.g = {}

## 2. Structs e metodos

Structs continuam sendo emitidas como tabelas Lua com construtor new e metatable __index.

Leitura simplificada:

~~~lua
local Person = {}
Person.__index = Person

function Person.new(fields)
    local self = setmetatable({}, Person)
    self.name = fields.name
    return self
end
~~~

## 3. @campo e self

No dominio de expressao:

- @campo desce para self.campo
- @metodo(args) desce para self:metodo(args)

## 4. UFCS virtual

Quando o binder resolve um membro como metodo virtual, o codegen emite chamada para o namespace zt:

- lista.len() -> zt.len(lista)
- "a,b".split() -> zt.split("a,b", ",") quando o argumento padrao e usado
- opt.unwrap() -> zt.unwrap(opt)
- n.to_text() -> zt.to_text(n)

## 5. Indexacao e slices

A trilha ativa nao emite indexacao direta para sequencias em todos os casos.

Mapeamento atual:

- lista[i] -> zt.index_seq(lista, i)
- texto[i] -> zt.index_text(texto, i)
- obj[i] dinamico -> zt.index_any(obj, i)
- lista[a..b] ou texto[a..b] -> zt.slice(obj, a, b)

Esses helpers sao a base do erro runtime ZT-R011.

## 6. where e validate

validate nao sobrevive como forma propria no backend. Ele e combinado semanticamente com where antes da emissao.

Leitura simplificada:

- validate min_value(18), max_value(120)
- where it <= 120

vira uma unica condicao booleana equivalente a:

~~~zt
where min_value(it, 18) and max_value(it, 120) and it <= 120
~~~

## 7. Atributos de declaracao

#[...] pertence ao parser e ao binder. Em geral, atributos de declaracao nao geram estrutura Lua propria na trilha ativa, salvo quando alguma etapa especifica de compilacao decidir consumi-los.

## 8. Fallback com or

Na trilha ativa, o operador or desce para a infraestrutura de unwrap_or.

Leitura de backend:

- a or b -> zt.unwrap_or(a, b)

Isso sustenta a semantica de fallback usada com Optional, Outcome e valores comuns.
