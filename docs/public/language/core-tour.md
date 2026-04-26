# Tour da linguagem

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Resumo

Zenith e uma linguagem reading-first.

Ela prefere intencao explicita, fluxo vertical e diagnosticos claros.

## Arquivo

```zt
namespace app.main

import std.io as io

public func main() -> result<void, core.Error>
    io.print("hello")?
    return success()
end
```

## Valores

```zt
const name: text = "Ada"
var score: int = 0

score = score + 1
```

## Funcoes

```zt
func add(a: int, b: int) -> int
    return a + b
end
```

## Tipos de erro

```zt
func load_name(id: int) -> result<text, text>
    if id < 0
        return error("invalid id")
    end

    return success("Ada")
end
```

## Dados

```zt
struct Player
    name: text
    hp: int where it >= 0
end
```

## Estados

```zt
enum LoadState
    Loading
    Ready(name: text)
    Failed(message: text)
end
```

## Comportamento

```zt
trait Scoreable
    func score() -> int
end

apply Scoreable to Player
    func score() -> int
        return self.hp
    end
end
```

## Onde aprofundar

- `docs/public/learn/README.md`
- `docs/public/cookbook/README.md`
- `docs/reference/language/README.md`
