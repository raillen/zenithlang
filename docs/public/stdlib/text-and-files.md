# Texto e arquivos

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Resumo

Use `std.text` para transformar texto.

Use `std.fs` para ler e escrever arquivos.

## Texto

```zt
namespace app.main

import std.text as text

public func main() -> int
    const raw: text = "  Ada  "
    const name: text = text.trim(raw)

    if text.is_blank(name)
        return 1
    end

    if not text.starts_with(name, "A")
        return 2
    end

    return 0
end
```

## Arquivo de texto

```zt
namespace app.main

import std.fs as fs

public func main() -> result<void, fs.Error>
    fs.write_text("hello.txt", "hello")?

    const content: text = fs.read_text("hello.txt")?

    if content != "hello"
        return error(fs.Error.Unknown)
    end

    return success()
end
```

## Regras

- `text` e UTF-8.
- `bytes` e dado binario.
- `fs.*` retorna `result` quando a operacao pode falhar.
- Use `?` para propagar erro quando a funcao tambem retorna `result`.

## Veja tambem

- `docs/reference/stdlib/text-bytes-format.md`
- `docs/reference/stdlib/filesystem-os-time.md`
