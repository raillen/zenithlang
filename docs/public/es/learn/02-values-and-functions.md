# 02 - Valores e funções

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Objetivo

Aprender `const`, `var`, tipos explicitos e funções pequenas.

## Código

```zt
namespace app.main

func double(value: int) -> int
    return value * 2
end

public func main() -> int
    const base: int = 21
    var result: int = double(base)

    result = result + 1

    return result
end
```

## Quando usar `const`

Use `const` quando o nome não deve ser reatribuido.

```zt
const name: text = "Ada"
```

## Quando usar `var`

Use `var` quando o valor precisa mudar.

```zt
var count: int = 0
count = count + 1
```

## Regra importante

Zenith exige tipo explícito em declarações locais.

Escreva:

```zt
const score: int = 10
```

Não escreva:

```zt
const score = 10
```

## Próximo

Leia `docs/public/learn/03-errors-and-absence.md`.
