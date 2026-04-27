# Módulo `std.concurrent`

Módulo pequeno para tornar explícita a cópia entre isolates.
Ele não cria threads.
Ele não expoe `jobs` ainda.

## Constantes e Funções

### `copy_int`

```zt
public func copy_int(value: int) -> int
```

Retorna o mesmo `int`.
Escalares já são naturalmente transferiveis.

### `copy_bool`

```zt
public func copy_bool(value: bool) -> bool
```

Retorna o mesmo `bool`.

### `copy_float`

```zt
public func copy_float(value: float) -> float
```

Retorna o mesmo `float`.

### `copy_text`

```zt
public func copy_text(value: text) -> text
```

Cria uma cópia segura de `text` para fronteiras de isolate.

### `copy_bytes`

```zt
public func copy_bytes(value: bytes) -> bytes
```

Cria uma cópia segura de `bytes` para fronteiras de isolate.

### `copy_list_int`

```zt
public func copy_list_int(value: list<int>) -> list<int>
```

Cria uma cópia segura de `list<int>`.

### `copy_list_text`

```zt
public func copy_list_text(value: list<text>) -> list<text>
```

Cria uma cópia segura de `list<text>`.

### `copy_map_text_text`

```zt
public func copy_map_text_text(value: map<text,text>) -> map<text,text>
```

Cria uma cópia segura de `map<text,text>`.
No alpha atual, este e o bloco publico inicial para mapas transferiveis.

