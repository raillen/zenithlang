# Language Guides

> Guias publicos de uso da linguagem.
> Audience: user
> Surface: public
> Status: current

## Ideia central

Zenith prioriza leitura clara.

A linguagem evita atalhos implicitos quando eles tornam o codigo mais dificil de revisar.

## Forma basica de um arquivo

```zt
namespace app.main

func main() -> int
    return 0
end
```

Regras simples:

- `namespace` declara onde o arquivo vive.
- `func` declara uma funcao.
- Blocos fecham com `end`.
- Indentacao recomendada: 4 espacos.

## Importacao

```zt
import std.io as io
```

Use o alias de forma qualificada:

```zt
io.print("ola")
```

## Constantes e variaveis

```zt
const name: text = "Zenith"
var count: int = 0

count = count + 1
```

- `const` nao pode ser reatribuido.
- `var` pode ser reatribuido.
- `public var` existe apenas em escopo de namespace.
- `public` significa visibilidade, nao global.
- A linguagem nao tem sintaxe `global`.

## API publica

```zt
namespace app.counter

public var value: int = 0

public func next() -> int
    value = value + 1
    return value
end
```

Outro namespace pode ler:

```zt
import app.counter as counter

func main() -> int
    return counter.value
end
```

Outro namespace nao pode escrever:

```zt
counter.value = 0 -- erro
```

## Onde consultar detalhes

- Tour da linguagem: `docs/public/language/core-tour.md`.
- Mapeamento mental: `docs/public/language/common-mapping.md`.
- Trilha de aprendizado: `docs/public/learn/README.md`.
- Cookbook: `docs/public/cookbook/README.md`.
- Sintaxe curta: `docs/reference/zenith-kb/syntax.md`.
- Semantica: `docs/reference/zenith-kb/semantics.md`.
- Spec normativa: `language/spec/README.md`.
- Decisoes de linguagem: `language/decisions/README.md`.
