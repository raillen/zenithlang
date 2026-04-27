# Texto y archivos

> Audiencia: usuario
> Estado: actual
> Superficie: pública
> Fuente de la verdad: no

## Resumen

Usar`std.text`para transformar texto.

Usar`std.fs`para leer y escribir archivos.

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

## Archivo de texto

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

## Normas

- `text`y UTF-8.
-`bytes`y datos binarios.
-`fs.*`regresa`result`cuando la operación puede fallar.
- Usar`?`para propagar el error cuando la función también regresa`result`.

## Ver también

- `docs/reference/stdlib/text-bytes-format.md`
- `docs/reference/stdlib/filesystem-os-time.md`
