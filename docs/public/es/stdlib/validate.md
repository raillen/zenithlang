# Módulo`std.validate`

Pequeños ayudantes de bool puro para`where`y condiciones regulares.
Sin IO, sin análisis, sin transformaciones.

## Constantes y funciones

### `between`

```zt
public func between(value: int, min: int, max: int) -> bool
```

Incluye control de alcance.

### `positive`

```zt
public func positive(value: int) -> bool
```

`true`cuando el valor es`> 0`.

### `non_negative`

```zt
public func non_negative(value: int) -> bool
```

`true`cuando el valor es`>= 0`.

### `negative`

```zt
public func negative(value: int) -> bool
```

`true`cuando el valor es`< 0`.

### `non_zero`

```zt
public func non_zero(value: int) -> bool
```

`true`cuando el valor no es cero.

### `one_of`

```zt
public func one_of(value: int, candidates: list<int>) -> bool
```

`true`cuando`value`esta dentro`candidates`.

### `one_of_text`

```zt
public func one_of_text(value: text, candidates: list<text>) -> bool
```

`true`cuando el valor del texto está dentro`candidates`.

### `not_empty`

```zt
public func not_empty(value: text) -> bool
```

`true`cuando la longitud del texto es`> 0`.

### `not_empty_text`

```zt
public func not_empty_text(value: text) -> bool
```

Borrar alias para`not_empty`cuando el valor es texto.

### `min_length`

```zt
public func min_length(value: text, min: int) -> bool
```

`true`cuando`len(value) >= min`.

### `min_len`

```zt
public func min_len(value: text, min: int) -> bool
```

Alias ​​corto para`min_length`.

### `max_length`

```zt
public func max_length(value: text, max: int) -> bool
```

`true`cuando`len(value) <= max`.

### `max_len`

```zt
public func max_len(value: text, max: int) -> bool
```

Alias ​​corto para`max_length`.

### `length_between`

```zt
public func length_between(value: text, min: int, max: int) -> bool
```

`true`cuando la longitud del texto está en`[min, max]`.

