# Module`std.json`

This module covers the stable subset of JSON in the current runtime.

Current scope:
- JSON object with keys`text`and values`text`- representation in Zenith:`map<text,text>`

Out of scope for now:
- general arrays
- typed numbers
- bool/null with rich model
- typed JSON tree (`Value/Object/Array`)

## Constants and Functions

### `parse`

```zt
public func parse(input: text) -> result<map<text,text>, core.Error>
```

Converts JSON text to`map<text,text>`.

@param input JSON text.
@return Map on success, or error on`core.Error`.

### `stringify`

```zt
public func stringify(value: map<text,text>) -> text
```

Convert`map<text,text>`to compact JSON.

### `pretty`

```zt
public func pretty(value: map<text,text>, indent: int = 2) -> text
```

Convert`map<text,text>`to formatted JSON.

@param indent Spaces per level (default: 2).

### `read`

```zt
public func read(file_path: text) -> result<map<text,text>, core.Error>
```

Reads the file and parses it in a single call.

### `write`

```zt
public func write(file_path: text, value: map<text,text>) -> result<void, core.Error>
```

Serializes and writes to file.

