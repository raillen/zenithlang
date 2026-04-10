# Módulo `std.fs.path`

O `std.fs.path` reúne helpers para manipular caminhos sem misturar regras de plataforma com a lógica da aplicação.

## Funções

| API | Descrição |
| :-- | :-- |
| `join(parts: list<text>) -> text` | Junta partes de caminho. |
| `extension(path: text) -> text` | Retorna a extensão. |
| `basename(path: text) -> text` | Retorna o último segmento com extensão. |
| `dirname(path: text) -> text` | Retorna a pasta pai. |
| `is_absolute(path: text) -> bool` | Detecta caminho absoluto. |
| `normalize(path: text) -> text` | Resolve `.` e `..`. |

## Exemplo

```zt
import std.fs.path

pub func main() -> int
    var arquivo = path.join(["docs", "api", "std-fs.md"])
    print("Basename: " + path.basename(arquivo))
    print("Dirname: " + path.dirname(arquivo))
    return 0
end
```
