# Module`std.io`

## Constants and Functions

### `﻿--- @target: namespace
# std.io â€” Entrada e SaÃ­da PadrÃ£o

MÃ³dulo textual sÃ­ncrono para streams padrÃ£o de entrada, saÃ­da e erro.
Usa handles tipados (`io.Input`, `io.Output`) e funÃ§Ãµes livres com parÃ¢metros opcionais `to:` e `from:`.
Todas as operaÃ§Ãµes retornam `result` com erro tipado `core.Error`.
---`

Input

### ````zt
public struct Input
    handle: int
end
```

Typed handle for input streams.
The default handle`io.input`represents standard input (stdin).
---`

Output

### ````zt
public struct Output
    handle: int
end
```

Typed handle for output streams.
The default handles are`io.output`(stdout) and`io.stderr`(stderr).
---`

Error

### ````zt
public enum Error
    ReadFailed
    WriteFailed
    Unknown
end
```

Typed error for I/O operations.
---`

input

### ````zt
public const input: io.Input
```

Default input handle (stdin).
---`

output

### ````zt
public const output: io.Output
```

Default output handle (stdout).
---`

err

### ````zt
public const err: io.Output
```

Error output handle (stderr).
Named`err`to avoid conflict with the constructor`error(...)`of`result`.
---`

read_line

### ````zt
public func read_line(from: io.Input = io.input) -> result<optional<text>, core.Error>
```

Reads a single line of input.
Return`none`when reaching EOF with no content.
Line terminators (`\n`, `\r\n`) are removed.

@param from – Input handle (default:`io.input`).
@return Line reads as`optional<text>`, or I/O error.
---`

read_all

### ````zt
public func read_all(from: io.Input = io.input) -> result<text, core.Error>
```

Reads all available content at input until EOF.

@param from – Input handle (default:`io.input`).
@return All text read, or I/O error.
---`

write

### ````zt
public func write(value: text, to: io.Output = io.output) -> result<void, core.Error>
```

Writes raw text to the specified output stream.
Does not add line breaks.

@param value – Text to be written.
@param to – Output handle (default:`io.output`).
@return`void`on success, or I/O error.
---`

print

