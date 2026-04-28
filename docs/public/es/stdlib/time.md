# Módulo`std.time`

Módulo temporal centrado en`Instant`y`Duration`.
Usar`time.now()`como punto de entrada del reloj del sistema.
La comparación utiliza operadores ordinarios (`==`, `<`, `>`).

## Constantes y funciones

### `Instant`

```zt
public struct Instant
    millis: int
end
```

Representa un momento absoluto (marca de tiempo en milisegundos desde la época).

### `Duration`

```zt
public struct Duration
    millis: int
end
```

Representa un intervalo de tiempo en milisegundos.

### `now`

```zt
public func now() -> time.Instant
```

Devuelve el instante actual del reloj del sistema.

### `now_ms`

```zt
public func now_ms() -> int
```

Devuelve la marca Unix actual en milisegundos.

### `sleep`

```zt
public func sleep(duration: time.Duration) -> result<void, core.Error>
```

Suspende la ejecución durante el período especificado.

@param duración: duración del sueño.
@devolver `void` en éxito o error del host.

### `sleep_ms`

```zt
public func sleep_ms(ms: int) -> result<void, core.Error>
```

Suspende la ejecución por `ms` milisegundos.

### `since`

```zt
public func since(start: time.Instant) -> time.Duration
```

Devuelve la duración desde`start`hasta el momento actual.

### `until`

```zt
public func until(target: time.Instant) -> time.Duration
```

Devuelve la duración desde el momento actual hasta`target`.

### `diff`

```zt
public func diff(a: time.Instant, b: time.Instant) -> time.Duration
```

Devuelve la diferencia temporal entre dos instantes (`b - a`).

### `elapsed`

```zt
public func elapsed(start: time.Instant, finish: time.Instant) -> int
```

Devuelve la diferencia en milisegundos entre `start` y `finish`.

### `add`

```zt
public func add(at: time.Instant, duration: time.Duration) -> time.Instant
```

Avanza un instante durante la duración especificada.

### `sub`

```zt
public func sub(at: time.Instant, duration: time.Duration) -> time.Instant
```

Retrocede un instante durante la duración especificada.

### `from_unix`

```zt
public func from_unix(ts: int) -> time.Instant
```

Crear un`Instant`de la marca de tiempo de Unix en segundos.

### `from_unix_ms`

```zt
public func from_unix_ms(ts: int) -> time.Instant
```

Crear un`Instant`de la marca de tiempo de Unix en milisegundos.

### `to_unix`

```zt
public func to_unix(at: time.Instant) -> int
```

Convertir un`Instant`para la marca de tiempo de Unix en segundos.

### `to_unix_ms`

```zt
public func to_unix_ms(at: time.Instant) -> int
```

Convertir un`Instant`para la marca de tiempo de Unix en milisegundos.

### `milliseconds`

```zt
public func milliseconds(n: int) -> time.Duration
```

Crea una duración de`n`milisegundos.

### `seconds`

```zt
public func seconds(n: int) -> time.Duration
```

Crea una duración de`n`artículos de segunda clase.

### `minutes`

```zt
public func minutes(n: int) -> time.Duration
```

Crea una duración de`n`minutos.

### `hours`

```zt
public func hours(n: int) -> time.Duration
```

Crea una duración de`n`horas.

