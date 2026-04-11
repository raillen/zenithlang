# Módulo 08: Implementação do Runtime (Projeto Zenith)

**Objetivo**: Analisar o código que sustenta a reatividade e as funcionalidades especiais do Zenith no Lua.  
**Público-alvo**: Contribuidores e desenvolvedores que queiram entender o motor da linguagem.  
**Contexto**: Trilha de Implementação (Código Real)

## Conteúdo Principal

O coração operacional do Zenith é o arquivo `src/backend/lua/runtime/zenith_rt.lua`. Ele é uma biblioteca Lua que fornece as "rodas" para o código transpilado correr.

### 1. O Sistema de Listeners Ativos
Para que a reatividade seja automática (sem você precisar declarar dependências), o Zenith usa uma variável global interna chamada `active_listener`.

```lua
local active_listener = nil -- Quem está "ouvindo" agora?

local function push_listener(listener)
    table.insert(listener_stack, active_listener)
    active_listener = listener
end
```

### 2. Estados como Proxies (`zt.state`)
Quando você cria um `state` no Zenith, o runtime cria uma metatabela Lua que intercepta o acesso ao valor.
- No `__index` (leitura): Se houver um `active_listener`, o estado o adiciona à sua lista de observadores.
- No `__newindex` (escrita): O estado atualiza o valor e avisa todos os observadores da lista.

### 3. Valores Computados (`zt.computed`)
O `computed` é um híbrido: ele é um observador (escuta outros estados) e um observado (outros podem escutá-lo).
Ele utiliza uma flag chamada `dirty`. O valor só é recalculado se um dos estados dos quais ele depende mudar, o que torna o Zenith extremamente eficiente.

### 4. Utilitários de Segurança
O runtime também implementa as garantias simples da linguagem, como o operador **Bang** (`!`):
```lua
function zt.bang(val, msg)
    if val == nil then
        error(msg or "acesso a valor nulo (null!)", 2)
    end
    return val
end
```

## Decisões e Justificativas

- **Decisão**: Usar as funções `get()` e `set()` virtuais nas metatabelas.
- **Motivo**: O Lua não permite interceptar mudanças em chaves que já existem de forma direta sem uma tabela "proxy". Ao usar o padrão proxy em `zenith_rt.lua`, garantimos que 100% das leituras e escritas sejam capturadas pelo sistema de reatividade.

## Exemplos de Código Real
Veja o arquivo [zenith_rt.lua](file:///C:/Users/raillen.DESKTOP-99RJ5M6/Documents/Projetos/zenith-lang-v2/src/backend/lua/runtime/zenith_rt.lua). Observe como o método `zt.watch` inicia a "primeira execução" de uma função para que ela possa registrar suas dependências automaticamente no primeiro acesso.

## Impactos na Implementação
Como o runtime é escrito em Lua puro, o Zenith pode rodar em qualquer lugar que tenha um interpretador Lua (jogos, servidores, microcontroladores), levando a reatividade moderna para ambientes onde ela antes era difícil de implementar.

---

### 🏆 Desafio Zenith
Abra o arquivo `src/backend/lua/runtime/zenith_rt.lua`.
1. Localize a função `zt.is`. Como ela diferencia um tipo primitivo (como `int`) de uma `struct` customizada?
2. Por que usamos uma `listener_stack` (pilha) em vez de apenas uma variável `active_listener` simples? (Dica: O que acontece se um `computed` depender de outro `computed`?).

**Status**: Implementação de Runtime Analisada
