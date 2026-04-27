# Módulo`std.fs`

Módulo de sistema de archivos síncrono.
Los caminos son`text`.
La manipulación de rutas continúa en`std.fs.path`.

## Constantes y funciones

### `Error`

```zt
public enum Error
    NotFound
    PermissionDenied
    AlreadyExists
    NotADirectory
    IsADirectory
    IOError
    InvalidPath
    Unknown
end
```

Error escrito para operaciones del sistema de archivos.

### `Metadata`

```zt
public struct Metadata
    size_bytes: int
    modified_at_ms: int
    created_at_ms: optional<int>
    is_file: bool
    is_dir: bool
end
```

Metadatos expuestos en alfa actual.
Las marcas de tiempo utilizan milisegundos de Unix.

### `read_text`

```zt
public func read_text(path: text) -> result<text, fs.Error>
```

Lee un archivo de texto UTF-8.

### `write_text`

```zt
public func write_text(path: text, content: text) -> result<void, fs.Error>
```

Escribe texto, reemplazando el contenido anterior.

### `append_text`

```zt
public func append_text(path: text, content: text) -> result<void, fs.Error>
```

Añade texto al final del archivo.

### `exists`

```zt
public func exists(path: text) -> result<bool, fs.Error>
```

Comprueba si la ruta existe.

### `is_file`

```zt
public func is_file(path: text) -> result<bool, fs.Error>
```

Comprueba si la ruta apunta al archivo.

### `is_dir`

```zt
public func is_dir(path: text) -> result<bool, fs.Error>
```

Comprueba si la ruta apunta al directorio.

### `create_dir`

```zt
public func create_dir(path: text) -> result<void, fs.Error>
```

Crea un directorio.

### `create_dir_all`

```zt
public func create_dir_all(path: text) -> result<void, fs.Error>
```

Crea un directorio y los países necesarios.

### `list_dir`

```zt
public func list_dir(path: text) -> result<list<text>, fs.Error>
```

Enumera los nombres de las entradas de un directorio.

### `remove_file`

```zt
public func remove_file(path: text) -> result<void, fs.Error>
```

Elimina un archivo.

### `remove_dir`

```zt
public func remove_dir(path: text) -> result<void, fs.Error>
```

Elimina un directorio vacío.

### `remove_dir_all`

```zt
public func remove_dir_all(path: text) -> result<void, fs.Error>
```

Elimina un directorio de forma recursiva.

### `copy_file`

```zt
public func copy_file(from: text, to: text) -> result<void, fs.Error>
```

Copia un archivo.

### `move`

```zt
public func move(from: text, to: text) -> result<void, fs.Error>
```

Mover o cambiar el nombre de una entrada.

### `metadata`

```zt
public func metadata(path: text) -> result<fs.Metadata, fs.Error>
```

Devuelve metadatos completos para la entrada.

### `size`

```zt
public func size(path: text) -> result<int, fs.Error>
```

Devuelve el tamaño en bytes.

### `modified_at`

```zt
public func modified_at(path: text) -> result<int, fs.Error>
```

Devuelve la marca de tiempo de modificación en milisegundos de Unix.

### `created_at`

```zt
public func created_at(path: text) -> result<optional<int>, fs.Error>
```

Devuelve la marca de tiempo de creación en milisegundos de Unix cuando esté disponible.

