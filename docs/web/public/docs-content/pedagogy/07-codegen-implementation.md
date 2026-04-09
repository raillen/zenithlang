# Módulo 07: Implementação do Codegen (Projeto Zenith)

**Objetivo**: Analisar como o Zenith transforma a AST em código Lua funcional.  
**Público-alvo**: Contribuidores e desenvolvedores que queiram entender o backend do compilador.  
**Contexto**: Trilha de Implementação (Código Real)

## Conteúdo Principal

O backend do Zenith reside em `src/backend/lua/lua_codegen.lua`. Ele é responsável por emitir texto Lua que seja eficiente e compatível com Lua 5.1/LuaJIT.

### 1. O Padrão Visitor Dinâmico
O Codegen utiliza uma técnica onde o nome do método de emissão é construído dinamicamente a partir do `kind` do nó.
```lua
function LuaCodegen:_emit_node(node)
    local method_name = "_emit_" .. node.kind:lower()
    if self[method_name] then
        return self[method_name](self, node)
    end
end
```
Isso torna o código extremamente modular. Quer adicionar suporte a um novo `kind` de nó? Basta criar a função correspondente `_emit_meu_novo_no`.

### 2. Gerenciando Recuo (Indentation)
Para que o código Lua gerado seja legível para humanos, o Codegen mantém um `indent_level`.
- Chamamos `self:indent()` ao entrar em blocos (`if`, `func`).
- Chamamos `self:dedent()` ao sair.
O método `emit` adiciona automaticamente os espaços necessários no início de cada linha.

### 3. Traduzindo Reatividade
Este é o ponto onde o Zenith brilha. Quando o Codegen vê um `STATE_DECL`, ele não gera uma variável comum, mas uma chamada para o Runtime:
```lua
-- Zenith: state life = 100
-- Lua Gerado:
local life = zt.state(100)
```
Isso permite que o sistema de reatividade do Zenith funcione de forma transparente sobre o Lua.

### 4. Exports e Namespaces
Ao final da geração, o Codegen coleta todos os membros marcados como `pub` e gera uma tabela de retorno:
```lua
-- No final do arquivo Lua
return {
    minha_func = minha_func,
    MinhaStruct = MinhaStruct,
}
```

## Decisões e Justificativas

- **Decisão**: Declaração antecipada (Forward Declaration) de todos os símbolos locais.
- **Motivo**: O Lua exige que uma variável local seja declarada antes de ser usada. Para suportar a liberdade do Zenith (onde funções podem se chamar em qualquer ordem), o Codegen primeiro emite uma linha `local funcA, funcB, VarC` e só depois emite as implementações.

## Exemplos de Código Real
Veja o arquivo [lua_codegen.lua](file:///C:/Users/raillen.DESKTOP-99RJ5M6/Documents/Projetos/zenith-lang-v2/src/backend/lua/lua_codegen.lua). Observe como o método `_emit_compilation_unit` coordena as diferentes passagens de geração para garantir que o arquivo Lua seja válido.

## Impactos na Implementação
Como geramos texto Lua, o tempo de compilação do Zenith é quase instantâneo. No entanto, o Codegen deve ser cuidadoso para não gerar nomes que conflitem com palavras-chave reservadas do Lua (como `then` ou `repeat`).

---

### 🏆 Desafio Zenith
Abra o arquivo `src/backend/lua/lua_codegen.lua`.
1. Localize o método `_emit_if_stmt`. Como ele lida com a tradução do `elif` e do `else` do Zenith para a sintaxe `elseif` do Lua?
2. Se você quisesse adicionar um comentário `--- Gerado automaticamente` no topo de cada arquivo, em qual método do Codegen você faria essa alteração?

**Status**: Implementação de Backend Analisadora
