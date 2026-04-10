# Módulo `std.core`

O `std.core` é a prelude da linguagem. Seus símbolos ficam disponíveis sem `import` explícito e servem de base para `Optional`, `Outcome`, `assert`, `panic` e operações de iteração.

## Funções globais

| API | Descrição |
| :-- | :-- |
| `print(value: any)` | Escreve no console. |
| `error(message: text)` | Interrompe a execução atual lançando um erro. |
| `assert(condition: bool, message: text)` | Falha se a condição for falsa. |
| `panic(message: text)` | Variante semântica para falha irrecuperável. |

## Tipos algébricos

### `Optional<T>`

Representa presença ou ausência de valor.

```zt
var nome: Optional<text> = Present("Zenith")

match nome
    case Present(valor):
        print("Nome: " + valor)
    case Empty:
        print("Sem valor")
end
```

Casos disponíveis:

- `Present(value: T)`
- `Empty`

### `Outcome<T, E>`

Representa sucesso ou falha explícitos.

```zt
var res = fs.read_text_file("config.json")

match res
    case Success(txt):
        print(txt)
    case Failure(err):
        print("Falha: " + err)
end
```

Casos disponíveis:

- `Success(value: T)`
- `Failure(error: E)`

## Trait fundamental

### `Iterable<T>`

Contrato usado pelo `for ... in`.

```zt
trait Iterable<T>
    func iterator() -> () => Optional<T>
end
```

## Padrões úteis do core

### Valor padrão com `or`

O codegen atual usa `or` para desembrulhar `Optional` e `Outcome`.

```zt
var user: text = os.get_env_variable("USERNAME") or "desconhecido"
```

### Retorno precoce com `?`

O operador `?` funciona com `Optional` e `Outcome` dentro de funções compatíveis.

```zt
func carregar() -> Outcome<text, text>
    var txt = fs.read_text_file("app.txt")?
    return Success(txt)
end
```

## Observações

- Esta página descreve o `std.core` real do repositório atual. APIs antigas como `type_of`, `to_text` e `is_null` não fazem parte da interface atual.
- `print`, `error`, `assert` e `panic` são tratados como parte da prelude e aparecem globalmente no código Zenith.
