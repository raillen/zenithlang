<p align="center">
  <img src="branding/logo-with-text.svg" width="400" alt="Zenith Logo" />
</p>

# Zenith Language

> Uma linguagem de programacao legivel, explicita, estavel visualmente e cognitivamente acessivel.

> Estado atual: produto 100% self-hosted com fechamento operacional/editorial concluido em `selfhost-pos100`.
> Front door oficial: `ztc.lua` em caminho self-hosted estrito.
> Base de runtime/backend: Lua 5.1 / LuaJIT / Lua 5.4.

**Versao do front door:** v0.3.6
**Target:** Lua 5.1 / LuaJIT / Lua 5.4
**Compilador oficial:** `ztc.lua` + core self-hosted canonico em `src/compiler/syntax.zt`
**Trilha Lua ativa:** referencia operacional para parser, binder, lowering, codegen e runtime
**Status self-hosted:** oficializado, corte 100% fechado e residual pos-100 encerrado

## O que e Zenith?

Zenith e uma linguagem que transpila para Lua. Ela combina sintaxe propria, sistema de tipos explicito, lowering, um runtime pequeno e um compilador que hoje se apresenta por um front door self-hosted, sem fallback legado no caminho oficial.

## Estado tecnico atual

- Fases 1-12 concluidas no recorte estabilizado;
- front door oficial 100% self-hosted;
- bootstrap deterministico e gate recorrente de release;
- CLI oficial com `zpm`, `zman` e `ztest` em modo estrito;
- politica de artefatos explicita para bootstrap, release, auditoria e smokes locais;
- trilha legada isolada apenas para recuperacao extraordinaria.

## Recursos estaveis

- Variaveis e constantes: `var`, `const`, tipagem explicita e inferencia basica
- Controle de fluxo: `if/elif/else`, `while`, `for in`, `repeat`, `match`
- Funcoes: `pub func`, parametros tipados, retorno `-> T`, async/await
- Structs: campos tipados, defaults, contratos `where` e `validate`
- Enums: membros simples e sum types
- Traits e `apply`
- Genericos reais
- UFCS
- Interpolacao de string
- `Optional` / `Outcome` e operador `?`
- Atributos com `#[...]`
- Sugar de self com `@campo`
- Indexacao 1-based para sequencias com `ZT-W002` e `ZT-R011`
- Diagnosticos com arquivo, linha, coluna e trecho de codigo

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
lua ztc.lua check <arquivo.zt>             # Analisa sem gerar codigo

lua ztc.lua zpm help
lua ztc.lua zman list
lua ztc.lua ztest --help
```

## Documentacao recomendada

- `docs/language/current.md`: linha correta da linguagem atual
- `docs/specification/current-core.md`: contrato curto do core atual
- `docs/roadmap/selfhost-pos100.md`: fechamento do residual pos-100
- `docs/specification/selfhost-abi.md`: contrato de host ABI do corte self-hosted
- `docs/specification/selfhost-artifacts.md`: politica de artefatos
- `docs/roadmap/MASTER.md`: mapa editorial/historico da evolucao

## Estrutura do projeto

```
src/
  lowering/       # IR, otimizacao e desugaring
  syntax/         # Lexer, parser e AST
  semantic/       # Binder, tipos e simbolos
  backend/lua/    # Codegen e runtime
  stdlib/         # Modulos padrao
  compiler/       # Core self-hosted canonico
```

## Licenca

MIT
