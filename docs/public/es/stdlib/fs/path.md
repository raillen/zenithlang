# Módulo`std.fs.path`

Módulo puro de operaciones léxicas sobre caminos.
No accede al sistema de archivos, no resuelve enlaces simbólicos.
Todas las operaciones están basadas en texto.

## Constantes y funciones

### `join`

```zt
public func join(parts: list<text>) -> text
```

Une varias partes del camino usando el separador de plataforma.

@param parts: lista de partes de la ruta.
@return Se unió a la ruta completa.

### `base_name`

```zt
public func base_name(value: text) -> text
```

Devuelve el nombre del archivo (última parte de la ruta).

Valor @param: ruta completa.
@return Nombre de archivo, o`""`si el camino termina en un separador.

### `name_without_extension`

```zt
public func name_without_extension(value: text) -> text
```

Devuelve el nombre del archivo sin extensión.

### `extension`

```zt
public func extension(value: text) -> optional<text>
```

Devuelve la extensión del archivo, o`none`si no lo hay.

Valor @param: ruta o nombre de archivo.
@return Extensión sin el punto, o`none`.

### `parent`

```zt
public func parent(value: text) -> optional<text>
```

Devuelve el directorio principal, o`none`si no hay padre.

Valor @param: ruta.
@return Ruta del directorio principal, o`none`.

### `normalize`

```zt
public func normalize(value: text) -> text
```

Normaliza el camino resolviendo`.`y`..`léxicamente.

### `absolute`

```zt
public func absolute(value: text, base: text) -> text
```

Convertir a ruta absoluta usando`base`explícito.
No lee implícitamente el directorio actual.

Valor @param: ruta relativa.
@param base: base explícita para la resolución.
@return Camino absoluto.

### `relative`

```zt
public func relative(value: text, from: text) -> text
```

Calcula la ruta relativa de`value`de`from`.

### `is_absolute`

```zt
public func is_absolute(value: text) -> bool
```

Comprueba si la ruta es absoluta.

### `is_relative`

```zt
public func is_relative(value: text) -> bool
```

Comprueba si la ruta es relativa.

### `has_extension`

```zt
public func has_extension(value: text, expected: text) -> bool
```

Comprueba si la ruta tiene la longitud esperada.
El punto de partida en`expected`se maneja automáticamente.

Valor @param: ruta a verificar.
@param esperado: extensión esperada (con o sin punto).
@devolver`true`si la extensión coincide.

### `change_extension`

```zt
public func change_extension(value: text, new_ext: text) -> text
```

Cambia la extensión del archivo en la ruta.
El punto de partida en`new_ext`se maneja automáticamente.

Valor @param: ruta original.
@param new_ext: nueva extensión (con o sin punto).
@return Ruta con extensión modificada.

