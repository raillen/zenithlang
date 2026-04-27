# Módulo`std.os`

Módulo para consultas y mutaciones de estado del proceso actual.
Los procesos secundarios permanecen en`std.os.process`.

## Constantes y funciones

### `Platform`

```zt
public enum Platform
    Windows
    Linux
    MacOS
    Unknown
end
```

Plataforma del sistema operativo.
Usar`os.platform()`para obtener el valor actual.

### `Arch`

```zt
public enum Arch
    X64
    X86
    Arm64
    Unknown
end
```

Arquitectura del procesador.
Usar`os.arch()`para obtener el valor actual.

### `Error`

```zt
public enum Error
    NotFound
    PermissionDenied
    IOError
    Unknown
end
```

Error escrito para las operaciones del sistema operativo.

### `args`

```zt
public func args() -> list<text>
```

Devuelve el vector de argumentos del proceso.

### `env`

```zt
public func env(name: text) -> optional<text>
```

Consultar una variable de entorno.
Devolver`none`si la variable no existe.

@param nombre: nombre de la variable.
@return Valor de variable, o`none`.

### `pid`

```zt
public func pid() -> int
```

Devuelve el PID del proceso actual.

### `platform`

```zt
public func platform() -> os.Platform
```

Devuelve la plataforma del sistema operativo.

### `arch`

```zt
public func arch() -> os.Arch
```

Devuelve la arquitectura del procesador.

### `current_dir`

```zt
public func current_dir() -> result<text, os.Error>
```

Devuelve el directorio de trabajo actual.

### `change_dir`

```zt
public func change_dir(path: text) -> result<void, os.Error>
```

Cambia el directorio de trabajo actual.

@param ruta: nuevo directorio de trabajo.
@devolver`void`en éxito o error.

