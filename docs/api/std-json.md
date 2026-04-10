# Módulo `std.json`

O `std.json` serializa e desserializa dados estruturados usando `Outcome` para falhas de parse ou IO.

## Funções principais

| API | Descrição |
| :-- | :-- |
| `parse(json: text) -> Outcome<any, text>` | Lê JSON e devolve mapas/listas Zenith. |
| `decode<T>(json: text) -> Outcome<T, text>` | Atalho tipado sobre `parse`. |
| `stringify(value: any, indent: int = 0) -> text` | Serializa um valor. |
| `minify(json: text) -> Outcome<text, text>` | Remove espaços supérfluos. |
| `beautify(json: text, indent: int = 4) -> Outcome<text, text>` | Formata o conteúdo. |
| `read_file(path: text) -> Outcome<any, text>` | Lê arquivo e faz parse. |
| `write_file(path: text, value: any, indent: int = 4) -> Outcome<void, text>` | Serializa e grava. |

## Regras úteis

- `indent = 0` gera saída compacta.
- `indent = -1` usa tabulação.
- `Present` e `Success` são serializados pelo valor interno.
- `Empty` e `Failure` viram `null` na implementação nativa atual.

## Exemplo

```zt
import std.json

pub func main() -> int
    var data = { "nome": "Zenith", "versao": "0.2.5" }
    var raw = json.stringify(data, 2)
    print(raw)

    match json.parse(raw)
        case Success(obj):
            print(obj.nome)
        case Failure(err):
            print("JSON inválido: " + err)
    end

    return 0
end
```

## Observações

- O parse atual cobre objetos, listas, números, booleanos, `null` e strings com escapes básicos.
- `decode<T>` ainda usa a mesma base de `parse`; a hidratação mais profunda para structs continua em evolução.
