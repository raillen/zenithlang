# Zenith Language

> Uma linguagem de programação legível, explícita, estável visualmente e cognitivamente acessível.

**Versão:** v0.2.0  
**Target:** Lua 5.1 / LuaJIT / Lua 5.4  
**Compilador:** Implementado em Lua

## O que é Zenith?

Zenith é uma linguagem de programação que transpila para Lua. Ela possui sintaxe própria, sistema de tipos 100% explícito, reatividade nativa e um pipeline de compilação com otimizações em nível de IR (Lowering).

## Recursos Core (v0.2.0)

Zenith v0.2 consolida a base da linguagem com foco em segurança e expressividade:

- **Sistema de Projeto**: Comando `zt build` orquestrado pelo arquivo `.ztproj`.
- **Destruturação**: Desmonte listas e structs em `match` ou declarações `var`.
- **Genéricos com Restrições**: Controle total sobre tipos genéricos usando `where T is Trait`.
- **Traits com Default Impls**: Composição poderosa com herança de comportamento padrão.
- **Spread e Slicing**: Manipulação de coleções de alto nível com operadores `..` e `[start..end]`.
- **Interpolação Avançada**: Suporte para expressões complexas dentro de strings `"{a + b}"`.
- **Lowering (IR)**: Fase de desaçucaramento e Constant Folding para performance.

### Exemplo Moderno

```zt
-- Definindo Contratos
trait Greetable
    pub func greet() -> text
        return "Olá!"
    end
end

-- Estrutura com Validação
struct Player
    pub nome: text
    pub vida: int = 100 where it >= 0
end

apply Greetable to Player
    pub func greet() -> text
        return "Oi, eu sou {@nome}!"
    end
end

pub func main() -> int
    var player: Player = Player { nome: "Rafael" }
    
    -- Encadeamento UFCS e Interpolação
    print(player.greet().len())
    
    -- Destruturação
    var Player { nome } = player
    print("Capturado: " + nome)
    
    return 0
end
```

## Uso

```bash
lua ztc.lua build          # Compila o projeto (zenith.ztproj)
lua ztc.lua <arquivo.zt>   # Compila um arquivo isolado
```

## Estrutura do Projeto

```
src/
  project_system/ — Gestão de .ztproj
  lowering/     — IR, Otimização e Desugaring
  syntax/       — Lexer, Parser e AST
  semantic/     — Binder, TypeChecker e Símbolos
  backend/lua/  — Geração de código e Runtime
```

## Licença

MIT
