# Especificação Técnica: Regras de Transpilação

Este documento detalha como cada construção de alto nível do Zenith é mapeada para o código Lua final, garantindo Abstração de Custo Zero.

## 1. Mapeamento de Variáveis e Funções

| Zenith | Lua (v5.4 / LuaJIT) |
| :--- | :--- |
| var x = 10 | local x = 10 |
| const PI = 3.14 | local PI = 3.14 |
| global g = {} | _G.g = {} |
| func f(a) -> a end | local function f(a) return a end |

---

## 2. Estruturas de Dados (Structs)

As structs Zenith são transpiladas para tabelas Lua otimizadas com metatabelas para suporte a métodos.

### Exemplo de Mapeamento
```zenith
struct Person
    nome: text
end
```

```lua
local Person = {}
Person.__index = Person

function Person.new(fields)
    local self = setmetatable({}, Person)
    self.nome = fields.nome
    return self
end
```

---

## 3. Enums e Pattern Matching

Enums simples tornam-se tabelas de constantes. Enums com dados (ADTs) utilizam uma estrutura de tags interna.

### Pattern Matching
O statement match é desaçucarado (lowering) para uma sequência de if/elseif/else altamente eficiente, evitando o overhead de tabelas de busca em tempo de execução.

---

## 4. Reatividade e Runtime

A reatividade (state, computed, watch) depende de um pequeno helper de runtime (zenith_rt.lua) que utiliza proxies ou metatabelas __newindex para interceptar mudanças e propagar eventos.

---

## 5. Blocos Native Lua

O Zenith permite a injeção direta de código Lua através do bloco native lua. O compilador trata o conteúdo desses blocos como texto bruto, inserindo-os diretamente no fluxo de saída sem análise sintática ou semântica profunda.

---
*Zenith Specification v0.3.0*
