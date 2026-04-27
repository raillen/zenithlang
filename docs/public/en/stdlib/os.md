# Module`std.os`

Module for queries and state mutations of the current process.
Child processes stay in`std.os.process`.

## Constants and Functions

### `Platform`

```zt
public enum Platform
    Windows
    Linux
    MacOS
    Unknown
end
```

Operating system platform.
Use`os.platform()`to get the current value.

### `Arch`

```zt
public enum Arch
    X64
    X86
    Arm64
    Unknown
end
```

Processor architecture.
Use`os.arch()`to get the current value.

### `Error`

```zt
public enum Error
    NotFound
    PermissionDenied
    IOError
    Unknown
end
```

Typed error for operating system operations.

### `args`

```zt
public func args() -> list<text>
```

Returns the vector of process arguments.

### `env`

```zt
public func env(name: text) -> optional<text>
```

Query an environment variable.
Return`none`if the variable does not exist.

@param name — Variable name.
@return Variable value, or`none`.

### `pid`

```zt
public func pid() -> int
```

Returns the PID of the current process.

### `platform`

```zt
public func platform() -> os.Platform
```

Returns the operating system platform.

### `arch`

```zt
public func arch() -> os.Arch
```

Returns the processor architecture.

### `current_dir`

```zt
public func current_dir() -> result<text, os.Error>
```

Returns the current working directory.

### `change_dir`

```zt
public func change_dir(path: text) -> result<void, os.Error>
```

Changes the current working directory.

@param path — New working directory.
@return`void`in success, or error.

