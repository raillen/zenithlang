# Zenith Cheat Sheet

Referência rápida da sintaxe e da stdlib principal no snapshot atual.

## Declarações

```zt
var x: int = 10
const APP_NAME: text = "Zenith"
global DEBUG: bool = false

type UserId = text
union Result<T> = T | text
```

## Fluxo

```zt
if x > 0
    print("positivo")
elif x == 0
    print("zero")
else
    print("negativo")
end

for i in 1..3
    print(i)
end

repeat 2 times
    print("tick")
end
```

## `match`, `check` e erro

```zt
match payload
    case Success(v):
        print(v)
    case Failure(err):
        print(err)
    else:
        print("fallback")
end

check idade >= 18 else
    panic("bloqueado")
end

attempt
    throw "falha"
rescue e
    print(e)
end
```

## Funções e lambdas

```zt
func soma(a: int, b: int) -> int
    return a + b
end

async func carregar() -> text
    return "ok"
end

var dobro = (n: int) => n * 2
var soma_bloco = (a: int, b: int) => do
    return a + b
end
```

## Structs, traits e contratos

```zt
struct Player
    pub nome: text
    pub vida: int where it >= 0
end

trait Greetable
    pub func greet() -> text
end

apply Greetable to Player
    pub func greet() -> text
        return "Oi, " + @nome
    end
end
```

## Operadores úteis

```zt
var trecho = lista[1..2]
var copia = [..lista, 4]
var nome = apelido!
var texto = fs.read_text_file("a.txt")?
var user = os.get_env_variable("USERNAME") or "desconhecido"
```

## Módulos

```zt
namespace app.main

import std.fs
import std.json
import std.os
import std.os.process
```

## Stdlib principal

```zt
fs.read_text_file("a.txt")
json.stringify({ "ok": true }, 2)
time.sleep(time.seconds(1))
os.run_command("echo hello")
process.spawn("echo", ["hello"], capture: true)
```

## Testes e interop

```zt
group "core"
    test "smoke"
        assert(true, "ok")
    end
end

native lua
    print("interop")
end
```
