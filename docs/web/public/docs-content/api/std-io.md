# Módulo `std.io`

O `std.io` cobre somente entrada e saída básica de terminal. Operações de arquivo pertencem a `std.fs` e `std.json`.

## Funções

| API | Descrição |
| :-- | :-- |
| `write(value: any)` | Escreve sem quebra de linha. |
| `write_line(value: any)` | Escreve com quebra de linha. |
| `read_line() -> text` | Lê uma linha do terminal. |

## Exemplo

```zt
import std.io

pub func main() -> int
    io.write("Seu nome: ")
    var nome: text = io.read_line()
    io.write_line("Olá, " + nome)
    return 0
end
```

## Observações

- O módulo é propositalmente pequeno.
- Para ler e gravar arquivos de texto, use `std.fs`.
- Para JSON em disco, use `std.json.read_file` e `std.json.write_file`.
