<p align="center">
  <img src="branding/logo-with-text.svg" width="400" alt="Zenith Logo" />
</p>

# Zenith Language

> Uma linguagem de programacao legivel, explicita, estavel visualmente e cognitivamente acessivel.

> Estado atual: trilha ativa estabilizada em `v0.3.5`
> O compilador ativo em Lua (`ztc.lua`) tem Fases 1-11 concluidas e testadas. A Fase 12 segue parcial: `demo.zt` compila sem `native lua`, mas stdlib e trilha self-hosted ainda dependem de escapes.

**Versao:** v0.3.5  
**Target:** Lua 5.1 / LuaJIT / Lua 5.4  
**Compilador ativo:** `ztc.lua` + parser/binder/codegen em Lua  
**Trilha self-hosted:** parcial, sem paridade total com a trilha ativa

## O que e Zenith?

Zenith e uma linguagem que transpila para Lua. Ela combina sintaxe propria, sistema de tipos explicito, reatividade, lowering de IR e um runtime pequeno o suficiente para manter a linguagem previsivel.

## Recursos estaveis na trilha ativa

- Variaveis e constantes: `var`, `const`, tipagem explicita e inferencia basica
- Controle de fluxo: `if/elif/else`, `while`, `for in`, `repeat`, `match`
- Funcoes: `pub func`, parametros tipados, retorno `-> T`, async/await
- Structs: campos tipados, defaults, contratos `where` e `validate`
- Enums: membros simples e sum types
- Traits e `apply`: composicao de comportamento validada
- Genericos reais: type erasure, constraints e chamadas genericas explicitas
- UFCS: metodo > global > virtual runtime
- Interpolacao de string
- Optional/Outcome e operador `?`
- Atributos de declaracao com `#[...]`
- Sugar de self com `@campo`
- Indexacao 1-based para sequencias com `ZT-W002` e `ZT-R011`
- Diagnosticos com arquivo, linha, coluna e trecho de codigo
- Escape hatch `native lua` para interop controlada

## Em aberto / hardening

- Fase 12: reduzir `native lua` em `src/stdlib` e `src/compiler`
- Self-hosting: alinhar `src/compiler/*.zt` com a linguagem ativa
- `after` / defer: ainda fora do conjunto estabilizado
- Refinar docs e catalogo de diagnosticos

## Exemplo moderno

```zt
import std.validation as validation

trait Greetable
    pub func greet() -> text
        return "Ola!"
    end
end

struct Player
    pub nome: text validate validation.non_empty
    pub vida: int = 100 where it >= 0
end

apply Greetable to Player
    pub func greet() -> text
        return "Oi, eu sou " + @nome + "!"
    end
end

pub func main() -> int
    var player: Player = Player { nome: "Rafael" }

    print(player.greet().len())

    var Player { nome } = player
    print("Capturado: " + nome)

    return 0
end
```

## Uso

```bash
lua ztc.lua build <arquivo.zt>             # Compila para out.lua
lua ztc.lua build <arquivo.zt> saida.lua   # Compila com saida explicita
lua ztc.lua run <arquivo.zt>               # Compila e executa
lua ztc.lua check <arquivo.zt>             # Analise sem gerar codigo
```

## Documentacao recomendada

- `docs/roadmap_estabilizacao.md`: estado verificado da implementacao ativa
- `docs/specification/decisions/001-self-attrs-validate.md`: sintaxe atual de `@`, `#[...]` e `validate`
- `docs/api/std-core.md`: Optional, Outcome e helpers atuais de runtime/UFCS

## Estrutura do projeto

```
src/
  lowering/       # IR, otimizacao e desugaring
  syntax/         # Lexer, parser e AST
  semantic/       # Binder, tipos e simbolos
  backend/lua/    # Codegen e runtime
  stdlib/         # Modulos padrao
```

## Licenca

MIT
