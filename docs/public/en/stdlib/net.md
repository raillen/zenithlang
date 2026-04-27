# Module`std.net`

Module for client TCP communication with blocking sockets and explicit timeouts.
Designed for simplicity and security with contracts`where`in the parameters.

## Constants and Functions

### `Error`

```zt
public enum Error
    ConnectionRefused
    HostUnreachable
    Timeout
    AddressInUse
    AlreadyConnected
    NotConnected
    NetworkDown
    Overflow
    PeerReset
    SystemLimit
    Unknown
end
```

Typed enumeration of possible network errors.

### `Connection`

```zt
public struct Connection
end
```

Opaque handle representing an open TCP connection.

## Functions

### `connect`

```zt
public func connect(
    host: text,
    port: int where it >= 1 and it <= 65535,
    timeout_ms: int
) -> result<net.Connection, core.Error>
```

Opens a TCP connection to the indicated host and port.
The port is validated by contract`where`at halftime`[1, 65535]`.

@param host — Host name or IP address.
@param port — TCP port (1–65535, validated by where).
@param timeout_ms — Maximum timeout in milliseconds.
@return Handle of open connection, or error.

### `read_some`

```zt
public func read_some(
    connection: net.Connection,
    max: int where it > 0,
    timeout_ms: int = -1
) -> result<optional<bytes>, core.Error>
```

Read until`max`bytes of the connection. Return`none`if the server closes the connection (EOF).
The parameter`max`is validated by`where`to be positive.

@param connection — Open TCP connection.
@param max — Maximum number of bytes to read (> 0).
@param timeout_ms — Timeout in milliseconds (-1 = no timeout).
@return Bytes read,`none`in EOF, or error.

### `write_all`

```zt
public func write_all(
    connection: net.Connection,
    data: bytes,
    timeout_ms: int = -1
) -> result<void, core.Error>
```

Send all bytes of`data`through the connection. Guarantees full shipping.

@param connection — Open TCP connection.
@param data — Bytes to send.
@param timeout_ms — Timeout in milliseconds (-1 = no timeout).
@return`void`in success, or error.

### `close`

```zt
public func close(connection: net.Connection) -> result<void, core.Error>
```

Closes a TCP connection.

@param connection — Connection to close.
@return`void`in success, or error.

### `is_closed`

```zt
public func is_closed(connection: net.Connection) -> bool
```

Checks if the connection is closed.

@param connection — Connection to check.
@return`true`if the connection is closed.

### `kind`

```zt
public func kind(err: core.Error) -> net.Error
```

Convert a`core.Error`generic in a`net.Error`typed.
Matches the field`code`error to determine the correct variant.

@param err — Generic core error.
@return Typed variant`net.Error`corresponding.

