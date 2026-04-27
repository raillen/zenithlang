# Module`std.fs.path`

Pure module of lexical operations on paths.
Does not access the filesystem, does not resolve symlinks.
All operations are text-based.

## Constants and Functions

### `join`

```zt
public func join(parts: list<text>) -> text
```

Join multiple path parts together using the platform separator.

@param parts — List of path parts.
@return Joined full path.

### `base_name`

```zt
public func base_name(value: text) -> text
```

Returns the file name (last part of the path).

@param value — Full path.
@return Filename, or`""`if the path ends in a separator.

### `name_without_extension`

```zt
public func name_without_extension(value: text) -> text
```

Returns the file name without extension.

### `extension`

```zt
public func extension(value: text) -> optional<text>
```

Returns the file extension, or`none`if there isn't.

@param value — Path or file name.
@return Extension without the dot, or`none`.

### `parent`

```zt
public func parent(value: text) -> optional<text>
```

Returns the parent directory, or`none`if there is no father.

@param value — Path.
@return Parent directory path, or`none`.

### `normalize`

```zt
public func normalize(value: text) -> text
```

Normalizes the path by solving`.`and`..`lexically.

### `absolute`

```zt
public func absolute(value: text, base: text) -> text
```

Convert to absolute path using`base`explicit.
Does not read the current directory implicitly.

@param value — Relative path.
@param base — Explicit basis for resolution.
@return Absolute path.

### `relative`

```zt
public func relative(value: text, from: text) -> text
```

Calculates the relative path of`value`from`from`.

### `is_absolute`

```zt
public func is_absolute(value: text) -> bool
```

Checks if the path is absolute.

### `is_relative`

```zt
public func is_relative(value: text) -> bool
```

Checks if the path is relative.

### `has_extension`

```zt
public func has_extension(value: text, expected: text) -> bool
```

Checks whether the path has the expected length.
The starting point at`expected`is handled automatically.

@param value — Path to check.
@param expected — Expected extension (with or without a dot).
@return`true`if the extension matches.

### `change_extension`

```zt
public func change_extension(value: text, new_ext: text) -> text
```

Changes the file extension in the path.
The starting point at`new_ext`is handled automatically.

@param value — Original path.
@param new_ext — New extension (with or without a dot).
@return Path with changed extension.

