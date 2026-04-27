# Módulo`std.net`

Módulo para comunicación TCP del cliente con sockets de bloqueo y tiempos de espera explícitos.
Diseñado para simplicidad y seguridad con contratos`where`en los parámetros.

## Constantes y funciones

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

Enumeración escrita de posibles errores de red.

### `Connection`

```zt
public struct Connection
end
```

Identificador opaco que representa una conexión TCP abierta.

## Funciones

### `connect`

```zt
public func connect(
    host: text,
    port: int where it >= 1 and it <= 65535,
    timeout_ms: int
) -> result<net.Connection, core.Error>
```

Abre una conexión TCP al host y puerto indicados.
El puerto está validado por contrato.`where`en el entretiempo`[1, 65535]`.

@param host: nombre de host o dirección IP.
Puerto @param: puerto TCP (1–65535, validado por dónde).
@param timeout_ms: tiempo de espera máximo en milisegundos.
@return Identificador de conexión abierta o error.

### `read_some`

```zt
public func read_some(
    connection: net.Connection,
    max: int where it > 0,
    timeout_ms: int = -1
) -> result<optional<bytes>, core.Error>
```

Leer hasta`max`bytes de la conexión. Devolver`none`si el servidor cierra la conexión (EOF).
El parámetro`max`es validado por`where`ser positivo.

Conexión @param: abre la conexión TCP.
@param max: número máximo de bytes para leer (> 0).
@param timeout_ms: tiempo de espera en milisegundos (-1 = sin tiempo de espera).
@return Bytes leídos,`none`en EOF, o error.

### `write_all`

```zt
public func write_all(
    connection: net.Connection,
    data: bytes,
    timeout_ms: int = -1
) -> result<void, core.Error>
```

Enviar todos los bytes de`data`a través de la conexión. Garantiza envío completo.

Conexión @param: abre la conexión TCP.
@param data: bytes para enviar.
@param timeout_ms: tiempo de espera en milisegundos (-1 = sin tiempo de espera).
@devolver`void`en éxito o error.

### `close`

```zt
public func close(connection: net.Connection) -> result<void, core.Error>
```

Cierra una conexión TCP.

Conexión @param: conexión para cerrar.
@devolver`void`en éxito o error.

### `is_closed`

```zt
public func is_closed(connection: net.Connection) -> bool
```

Comprueba si la conexión está cerrada.

Conexión @param: conexión para comprobar.
@devolver`true`si la conexión está cerrada.

### `kind`

```zt
public func kind(err: core.Error) -> net.Error
```

Convertir un`core.Error`genérico en un`net.Error`mecanografiado.
Coincide con el campo`code`error para determinar la variante correcta.

@param err: error central genérico.
@return Variante escrita`net.Error`correspondiente.

