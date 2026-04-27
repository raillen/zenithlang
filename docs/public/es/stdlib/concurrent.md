# Módulo`std.concurrent`

Pequeño módulo para hacer explícita la copia entre aislados.
No crea hilos.
el no expone`jobs`todavía.

## Constantes y funciones

### `copy_int`

```zt
public func copy_int(value: int) -> int
```

Devuelve lo mismo`int`.
Los escalares ya son naturalmente transferibles.

### `copy_bool`

```zt
public func copy_bool(value: bool) -> bool
```

Devuelve lo mismo`bool`.

### `copy_float`

```zt
public func copy_float(value: float) -> float
```

Devuelve lo mismo`float`.

### `copy_text`

```zt
public func copy_text(value: text) -> text
```

Crea una copia segura de`text`para aislar fronteras.

### `copy_bytes`

```zt
public func copy_bytes(value: bytes) -> bytes
```

Crea una copia segura de`bytes`para aislar fronteras.

### `copy_list_int`

```zt
public func copy_list_int(value: list<int>) -> list<int>
```

Crea una copia segura de`list<int>`.

### `copy_list_text`

```zt
public func copy_list_text(value: list<text>) -> list<text>
```

Crea una copia segura de`list<text>`.

### `copy_map_text_text`

```zt
public func copy_map_text_text(value: map<text,text>) -> map<text,text>
```

Crea una copia segura de`map<text,text>`.
En la versión alfa actual, este es el bloque público inicial para mapas descargables.

