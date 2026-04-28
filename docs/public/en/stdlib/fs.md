# Module`std.fs`

Synchronous filesystem module.
Paths are`text`.
Path manipulation continues in`std.fs.path`.

## Constants and Functions

### `Error`

```zt
public enum Error
    NotFound
    PermissionDenied
    AlreadyExists
    NotADirectory
    IsADirectory
    IOError
    InvalidPath
    Unknown
end
```

Typed error for filesystem operations.

### `Metadata`

```zt
public struct Metadata
    size_bytes: int
    modified_at_ms: int
    created_at_ms: optional<int>
    is_file: bool
    is_dir: bool
end
```

Metadata exposed in current alpha.
Timestamps use Unix milliseconds.

### `read_text`

```zt
public func read_text(path: text) -> result<text, fs.Error>
```

Reads a UTF-8 text file.

### `write_text`

```zt
public func write_text(path: text, content: text) -> result<void, fs.Error>
```

Writes text, replacing previous content.

### `append_text`

```zt
public func append_text(path: text, content: text) -> result<void, fs.Error>
```

Appends text to the end of the file.

### `exists`

```zt
public func exists(path: text) -> result<bool, fs.Error>
```

Checks if the path exists.

### `is_file`

```zt
public func is_file(path: text) -> result<bool, fs.Error>
```

Checks if the path points to file.

### `is_dir`

```zt
public func is_dir(path: text) -> result<bool, fs.Error>
```

Checks if the path points to the directory.

### `create_dir`

```zt
public func create_dir(path: text) -> result<void, fs.Error>
```

Creates a directory.

### `create_dir_all`

```zt
public func create_dir_all(path: text) -> result<void, fs.Error>
```

Creates a directory and the necessary countries.

### `list_dir`

```zt
public func list_dir(path: text) -> result<list<text>, fs.Error>
```

Lists the names of entries in a directory.

### `remove_file`

```zt
public func remove_file(path: text) -> result<void, fs.Error>
```

Removes a file.

### `remove_dir`

```zt
public func remove_dir(path: text) -> result<void, fs.Error>
```

Removes an empty directory.

### `remove_dir_all`

```zt
public func remove_dir_all(path: text) -> result<void, fs.Error>
```

Removes a directory recursively.

### `copy_file`

```zt
public func copy_file(from: text, to: text) -> result<void, fs.Error>
```

Copies a file.

### `copy`

```zt
public func copy(from: text, to: text) -> result<void, fs.Error>
```

Alias for `copy_file`.

### `move`

```zt
public func move(from: text, to: text) -> result<void, fs.Error>
```

Move or rename an entry.

### `rename`

```zt
public func rename(from: text, to: text) -> result<void, fs.Error>
```

Alias for `move`.

### `metadata`

```zt
public func metadata(path: text) -> result<fs.Metadata, fs.Error>
```

Returns full metadata for the entry.

### `size`

```zt
public func size(path: text) -> result<int, fs.Error>
```

Returns the size in bytes.

### `file_size`

```zt
public func file_size(path: text) -> result<int, fs.Error>
```

Alias for `size`.

### `modified_at`

```zt
public func modified_at(path: text) -> result<int, fs.Error>
```

Returns the modification timestamp in Unix milliseconds.

### `created_at`

```zt
public func created_at(path: text) -> result<optional<int>, fs.Error>
```

Returns the creation timestamp in Unix milliseconds when available.

