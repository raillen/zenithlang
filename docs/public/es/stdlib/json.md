# Módulo`std.json`

Este módulo cubre el subconjunto estable de JSON en el tiempo de ejecución actual.

Alcance actual:
- Objeto JSON con claves`text`y valores`text`- representación en Zenith:`map<text,text>`

Fuera de alcance por ahora:
- matrices generales
- números escritos
- bool/nulo con modelo rico
- árbol JSON escrito (`Value/Object/Array`)

## Constantes y funciones

### `parse`

```zt
public func parse(input: text) -> result<map<text,text>, core.Error>
```

Convierte texto JSON a`map<text,text>`.

@param ingresa texto JSON.
@return Map en caso de éxito o error en`core.Error`.

### `stringify`

```zt
public func stringify(value: map<text,text>) -> text
```

Convertir`map<text,text>`para compactar JSON.

### `pretty`

```zt
public func pretty(value: map<text,text>, indent: int = 2) -> text
```

Convertir`map<text,text>`a JSON formateado.

@param sangría Espacios por nivel (predeterminado: 2).

### `read`

```zt
public func read(file_path: text) -> result<map<text,text>, core.Error>
```

Lee el archivo y lo analiza en una sola llamada.

### `write`

```zt
public func write(file_path: text, value: map<text,text>) -> result<void, core.Error>
```

Serializa y escribe en el archivo.

