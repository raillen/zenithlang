# Equivalentes Zenith para conceitos de outras linguagens

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Objetivo

Este guia mostra o caminho Zenith para ideias comuns em C#, Java, TypeScript,
Rust e Go.

Use este arquivo quando você souber o conceito, mas não souber o nome dele em
Zenith.

## Resumo rápido

| Se você procura | Em Zenith use |
| --- | --- |
| inferencia local ampla | tipo explícito em `const` ou `var` |
| union segura | `enum` com payload |
| método abstrato | `trait` |
| interface com implementação separada | `trait` + `apply Trait to Type` |
| dispatch virtual | `dyn<Trait>` quando houver heterogeneidade real |
| partial class | `apply Type` e organizacao por namespace/arquivo |
| exception recuperavel | `result<T, E>` |
| `null` | `optional<T>` e `none` |

## Declaração local com tipo explícito

Zenith não usa inferencia local ampla neste ciclo.

Escreva o tipo perto do nome:

```zt
const name: text = "Ana"
var retry_count: int = 0
```

Não escreva:

```zt
let name = "Ana"
```

Por que:

- o tipo fica visivel durante leitura;
- literais vazios ficam menos ambiguos;
- diagnosticos conseguem explicar melhor o erro.

## Union segura

Em TypeScript, Rust ou C, você pode pensar em union.

Em Zenith, use `enum` com payload:

```zt
enum Shape
    Circle(radius: int)
    Rectangle(width: int, height: int)
    Point
end
```

Não crie uma keyword `union`.

O `match` torna cada caso explícito.

Veja também:

- `docs/public/cookbook/safe-union-with-enum.md`
- `language/decisions/029-executable-enums-with-payload.md`

## Método abstrato

Em C# ou Java, você pode pensar em `abstract class`.

Em Zenith, use `trait`:

```zt
trait Scoreable
    func score() -> int
end
```

Depois implemente o contrato:

```zt
apply Scoreable to Player
    func score() -> int
        return self.hp
    end
end
```

Não adicione `abstract`.

Veja também:

- `docs/public/cookbook/abstract-methods-with-trait.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`

## Dispatch virtual

Em C# ou Java, você pode pensar em método `virtual`.

Em Zenith, use `dyn<Trait>` quando valores de tipos diferentes precisam passar
pelo mesmo contrato:

```zt
const shape: dyn Shape = circle
```

Não use `dyn` se todos os valores tem o mesmo tipo concreto.

Nesse caso, prefira o tipo concreto ou uma funcao genérica com constraint.

```zt
func render_one<T>(shape: T) -> int
where T is Shape
```

Regra simples:

- varios tipos concretos na mesma lista ou variavel: `dyn<Trait>`;
- um tipo concreto por chamada: generics com `where`.

Veja também:

- `docs/public/cookbook/virtual-dispatch-with-dyn.md`
- `language/spec/dyn-dispatch.md`

## Partial class

Em C#, `partial class` pode separar código em varios arquivos.

Em Zenith, dados e comportamento ficam separados por desenho:

```zt
struct Player
    hp: int
end

apply Player
    func is_alive() -> bool
        return self.hp > 0
    end
end
```

Use `apply Type` para métodos inerentes.

Use `apply Trait to Type` para implementar um contrato.

Não adicione `partial`.

Veja também:

- `docs/public/cookbook/partial-class-with-apply.md`
- `language/decisions/010-structs-traits-apply-enums-and-match.md`

## Exceção recuperavel

Em Java, C# ou TypeScript, você pode pensar em `throw`.

Em Zenith, falha recuperavel deve aparecer no tipo:

```zt
func load_user(id: int) -> result<User, core.Error>
```

Use `panic` apenas para falha fatal.

Veja também:

- `docs/public/cookbook/errors-without-exceptions.md`
- `language/decisions/037-panic-fatal-errors-and-attempt.md`

## Ausencia sem null

Em Java, TypeScript, Go ou C#, você pode pensar em `null` ou `nil`.

Em Zenith, use `optional<T>`:

```zt
const maybe_name: optional<text> = none
```

Isso deixa a ausencia visivel no tipo.

Veja também:

- `docs/public/cookbook/absence-without-null.md`
- `language/decisions/041-no-null-and-optional-absence.md`

## Palavras que R6 não adiciona

R6 não adiciona estas keywords ou atalhos:

- `abstract`;
- `virtual`;
- `partial`;
- `union`;
- `?.`;
- `??`;
- ternario;
- pipe operator;
- overload;
- macros;
- implicit return.

Essas ideias continuam fora do ciclo para preservar leitura, previsibilidade e
diagnosticos claros.
