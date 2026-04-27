# Módulo`std.os.process`

Módulo para ejecutar procesos hijo.
Ejecución explícita a través de`program` + `args`, sin concha.
El código de salida distinto de cero es un resultado válido, no un error.

## Constantes y funciones

### `ExitStatus`

```zt
public struct ExitStatus
    code: int
end
```

Estado de salida de un proceso hijo.
El código 0 indica éxito convencional; otros códigos son resultados válidos.

### `CapturedRun`

```zt
public struct CapturedRun
    status: process.ExitStatus
    stdout_text: text
    stderr_text: text
end
```

Resultado de la ejecución con captura textual de stdout y stderr.

### `Error`

```zt
public enum Error
    NotFound
    PermissionDenied
    IOFailure
    DecodeFailed
    Unknown
end
```

Error de nivel de transporte (generar, esperar, capturar).
El código de salida distinto de cero NO es`process.Error`.

### `run`

```zt
public func run(program: text, args: list<text> = [], cwd: optional<text> = none) -> result<process.ExitStatus, process.Error>
```

Ejecuta un programa y espera a que se complete.
No utiliza cáscara. Los argumentos se pasan explícitamente.

@param program: ruta o nombre del programa.
@param args — Argumentos del programa (predeterminado:`[]`).
@param cwd: directorio de trabajo opcional.
@return Estado de salida o error de transporte.

### `run_capture`

```zt
public func run_capture(program: text, args: list<text> = [], cwd: optional<text> = none) -> result<process.CapturedRun, process.Error>
```

Ejecuta un programa y captura stdout y stderr como texto (UTF-8).

@param program: ruta o nombre del programa.
@param args — Argumentos del programa (predeterminado:`[]`).
@param cwd: directorio de trabajo opcional.
@return Resultado capturado o error de transporte.

