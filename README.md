<p align="center">
  <img src="branding/logo-with-text.svg" width="400" alt="Zenith Logo" />
</p>

# Zenith Language

> Uma linguagem de programação legível, explícita, estável visualmente e cognitivamente acessível.

> Estado atual: compilador nativo v2 com backend C, pipeline completo (lex → parse → AST → semantic → HIR → ZIR → C → native).
> Arquitetura: `.zt` → C → executável nativo via GCC/Clang.

**Versão:** v2 (desenvolvimento ativo)
**Target:** Executável nativo via C backend
**Compilador:** `zt.exe` — implementado em C, auto-compilável via `build.py`
**Pipeline:** Lexer → Parser → AST → Binder → Type Checker → HIR → ZIR → C Emitter → GCC
**Foco:** Jogos, UI, desktop e automação

## O que é Zenith?

Zenith é uma linguagem com sintaxe própria, sistema de tipos explícito e compilação nativa via C. Combina legibilidade ("reading-first"), acessibilidade cognitiva e performance nativa.

## Estado técnico atual

- Pipeline completo: lex, parse, bind, check, HIR, ZIR, C emit, native compile
- 115/118 testes de conformidade passando
- Runtime nativo em C (`zenith_rt.c`) com text, list, map, RC
- Standard library em `.zt` com carregamento automático de módulos
- Driver modular: `driver_internal.h`, `paths.c`, `main.c`
- LSP integrado
- Formatter e doc-check funcionais


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
