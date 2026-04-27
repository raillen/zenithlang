# Text and files

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Summary

Use`std.text`to transform text.

Use`std.fs`to read and write files.

## Text

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

## Text file

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

## Rules

- `text`and UTF-8.
-`bytes`and binary data.
-`fs.*`returns`result`when the operation may fail.
- Use`?`to propagate error when the function also returns`result`.

## See also

- `docs/reference/stdlib/text-bytes-format.md`
- `docs/reference/stdlib/filesystem-os-time.md`
