# Módulo`std.test`

Módulo complementario para pruebas.
Las pruebas se declaran con`attr test`.
Usar`check(...)`para condiciones simples.
Usar`std.test`cuando quieras fallar, salta o compara valores con un mensaje más claro.

## Constantes y funciones

### `fail`

```zt
public func fail(message: text = "test failed") -> void
```

Marca la prueba actual como fallida inmediatamente.
Útil en ramas de control de flujo, por ejemplo después de un`match`en`result`.

Mensaje @param: mensaje de error.

### `skip`

```zt
public func skip(reason: text = "") -> void
```

Marca la prueba actual como omitida.
Saltar es un resultado diferente: ni aprobar ni suspender.

@param Reason: motivo del salto.

### `is_true`

```zt
public func is_true(value: bool) -> void
```

falla si`value`para`false`.
El mensaje muestra`expected true`y`received false`.

### `is_false`

```zt
public func is_false(value: bool) -> void
```

falla si`value`para`true`.
El mensaje muestra`expected false`y`received true`.

### `equal_int`

```zt
public func equal_int(actual: int, expected: int) -> void
```

falla si`actual != expected`.
El mensaje muestra Esperado y Recibido.

### `equal_text`

```zt
public func equal_text(actual: text, expected: text) -> void
```

falla si`actual != expected`.
El mensaje muestra Esperado y Recibido.

### `not_equal_int`

```zt
public func not_equal_int(actual: int, expected: int) -> void
```

falla si`actual == expected`.
El mensaje muestra que el valor debería ser diferente.

### `not_equal_text`

```zt
public func not_equal_text(actual: text, expected: text) -> void
```

falla si`actual == expected`.
El mensaje muestra que el valor debería ser diferente.

