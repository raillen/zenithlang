# Módulo`std.text`

Módulo puro y neutral para la configuración regional para texto.
Opera por punto de código en el modelo MVP actual.
En esta versión alfa, el módulo publica solo el subconjunto que ya se entregó de forma segura.

## Constantes y funciones

### `to_utf8`

```zt
public func to_utf8(value: text) -> bytes
```

Convertir`text`válido para`bytes`UTF-8.

### `from_utf8`

```zt
public func from_utf8(value: bytes) -> result<text, text>
```

Intenta decodificar`bytes`UTF-8.
En este corte alfa, el error continúa escribiéndose como`text`.

### `trim`

```zt
public func trim(value: text) -> text
```

Elimina espacios en blanco en ambos extremos.

### `trim_start`

```zt
public func trim_start(value: text) -> text
```

Elimina los espacios en blanco desde el principio.

### `trim_end`

```zt
public func trim_end(value: text) -> text
```

Elimina los espacios en blanco del final.

### `contains`

```zt
public func contains(value: text, needle: text) -> bool
```

Devolver`true`cuando`needle`aparece en`value`.

### `starts_with`

```zt
public func starts_with(value: text, prefix: text) -> bool
```

Devolver`true`cuando`value`empezar con`prefix`.

### `ends_with`

```zt
public func ends_with(value: text, suffix: text) -> bool
```

Devolver`true`cuando`value`termina con`suffix`.

### `has_prefix`

```zt
public func has_prefix(value: text, prefix: text) -> bool
```

Alias ​​semántico explícito para`starts_with`.

### `has_suffix`

```zt
public func has_suffix(value: text, suffix: text) -> bool
```

Alias ​​semántico explícito para`ends_with`.

### `has_whitespace`

```zt
public func has_whitespace(value: text) -> bool
```

Devolver`true`cuando hay al menos un carácter de espacio en blanco.

### `index_of`

```zt
public func index_of(value: text, needle: text) -> int
```

Devuelve el índice de la primera aparición o`-1`.

### `last_index_of`

```zt
public func last_index_of(value: text, needle: text) -> int
```

Devuelve el índice de la última aparición o`-1`.

### `is_empty`

```zt
public func is_empty(value: text) -> bool
```

Devolver`true`cuando el texto está vacío.

### `is_blank`

```zt
public func is_blank(value: text) -> bool
```

Devolver`true`cuando el texto está vacío o solo tiene espacios.

### `is_digits`

```zt
public func is_digits(value: text) -> bool
```

Devolver`true`sólo para texto no vacío compuesto de dígitos ASCII`0-9`.

### `limit`

```zt
public func limit(value: text, max_len: int) -> text
```

Cortar texto al máximo`max_len`puntos de código, sin agregar un sufijo.

