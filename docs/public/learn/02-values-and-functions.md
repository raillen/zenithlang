# 02 - Valores e funcoes

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Objetivo

Aprender `const`, `var`, tipos explicitos e funcoes pequenas.

## Codigo

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

Use `const` quando o nome nao deve ser reatribuido.

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

Zenith exige tipo explicito em declaracoes locais.

Escreva:

```zt
const score: int = 10
```

Nao escreva:

```zt
const score = 10
```

## Proximo

Leia `docs/public/learn/03-errors-and-absence.md`.
