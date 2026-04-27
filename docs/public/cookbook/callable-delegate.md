# Como passar uma funcao como valor

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Problema

Você quer passar uma funcao para outra parte do código.

Em Zenith, use o tipo callable:

```zt
func(int, int) -> int
```

## Resposta curta

Declare a assinatura de forma explícita.

```zt
const op: func(int, int) -> int = add
```

## Exemplo completo

```zt
namespace app.main

func add(a: int, b: int) -> int
    return a + b
end

func run_op(op: func(int, int) -> int) -> int
    return op(3, 4)
end

public func main() -> int
    const op: func(int, int) -> int = add
    return run_op(op)
end
```

## Por que

O tipo callable mostra:

- quantos parametros a funcao recebe;
- quais tipos ela recebe;
- qual tipo ela retorna.

Isso evita callback escondido.

## Erro comum

Não tente usar uma funcao genérica como callable neste corte.

Prefira uma funcao top-level simples:

```zt
func add(a: int, b: int) -> int
    return a + b
end
```

## Limites atuais

Callables v1 são estreitos de propósito.

Neste corte, evite:

- callable como field de struct;
- callable dentro de list/map;
- partial application;
- método como valor;
- funcao genérica como valor.

## Veja também

- `language/spec/callables.md`
- `language/decisions/089-callable-delegates-v1.md`
