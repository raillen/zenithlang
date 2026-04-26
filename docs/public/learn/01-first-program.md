# 01 - Primeiro programa

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Objetivo

Entender o menor arquivo Zenith util.

## Codigo

```zt
namespace app.main

public func main() -> int
    return 0
end
```

## O que cada linha faz

`namespace app.main` diz o nome logico do arquivo no projeto.

`public func main() -> int` declara a funcao de entrada.

`return 0` devolve codigo de sucesso.

`end` fecha o bloco.

## Regras importantes

- `namespace` vem primeiro.
- Blocos fecham com `end`.
- `main` precisa ter assinatura aceita pelo driver.
- Tipos de retorno ficam depois de `->`.

## Erro comum

Nao omita o `namespace`:

```zt
public func main() -> int
    return 0
end
```

Use:

```zt
namespace app.main

public func main() -> int
    return 0
end
```

## Proximo

Leia `docs/public/learn/02-values-and-functions.md`.
