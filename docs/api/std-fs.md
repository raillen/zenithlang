# Módulo `std.fs`

O `std.fs` cobre leitura, escrita e abertura de arquivos com retorno explícito via `Outcome`.

## Tipos

### `FileMode`

```zt
enum FileMode
    Read, Write, Append
end
```

### `FileHandle`

```zt
struct FileHandle
    pub func read_line() -> Optional<text>
    pub func write(content: text) -> Outcome<void, text>
    pub func close()
end
```

## Operações atômicas

| API | Descrição |
| :-- | :-- |
| `read_text_file(path: text) -> Outcome<text, text>` | Lê todo o arquivo. |
| `write_text_file(path: text, content: text) -> Outcome<void, text>` | Sobrescreve o conteúdo. |
| `append_text(path: text, content: text) -> Outcome<void, text>` | Escreve no fim do arquivo. |
| `prepend_text(path: text, content: text) -> Outcome<void, text>` | Escreve no início usando arquivo temporário. |
| `exists(path: text) -> bool` | Verifica existência. |
| `remove_file(path: text) -> Outcome<void, text>` | Remove arquivo. |
| `remove_folder(path: text, recursive: bool) -> Outcome<void, text>` | Remove pasta. |

## Operações de fluxo

| API | Descrição |
| :-- | :-- |
| `open_file(path: text, mode: FileMode) -> Outcome<FileHandle, text>` | Abre arquivo para leitura ou escrita manual. |

## Exemplo

```zt
import std.fs

pub func main() -> int
    fs.write_text_file("log.txt", "linha 1\n")
    fs.append_text("log.txt", "linha 2\n")

    match fs.read_text_file("log.txt")
        case Success(txt):
            print(txt)
        case Failure(err):
            print("Falha: " + err)
    end

    return 0
end
```

## Observações

- O módulo usa `Outcome` para operações sujeitas a erro de IO.
- Para caminhos, prefira `std.fs.path`.
- Para JSON em disco, prefira os atalhos de `std.json`.
