# Módulo`std.io`

## Constantes y funciones

### `﻿--- @target: namespace
# std.io â€” Entrada e SaÃ­da PadrÃ£o

MÃ³dulo textual sÃ­ncrono para streams padrÃ£o de entrada, saÃ­da e erro.
Usa handles tipados (`io.Entrada`, `io.Salida`) e funÃ§Ãµes livres com parÃ¢metros opcionais `a:` e `de:`.
Todas as operaÃ§Ãµes retornam `resultado` com erro tipado `núcleo.Error`.
---`

Aporte

### ````zt
public struct Input
    handle: int
end
```

Identificador escrito para flujos de entrada.
El identificador predeterminado`io.input`representa la entrada estándar (stdin).
---`

Output

### ````zt
public struct Output
    handle: int
end
```

Identificador escrito para flujos de salida.
Los identificadores predeterminados son`io.output`(salida estándar) y`io.stderr`(estándar).
---`

Error

### ````zt
public enum Error
    ReadFailed
    WriteFailed
    Unknown
end
```

Error escrito para operaciones de E/S.
---`

input

### ````zt
public const input: io.Input
```

Identificador de entrada predeterminado (stdin).
---`

output

### ````zt
public const output: io.Output
```

Identificador de salida predeterminado (stdout).
---`

err

### ````zt
public const err: io.Output
```

Identificador de salida de error (stderr).
Nombrado`err`para evitar conflictos con el constructor`error(...)`de`result`.
---`

read_line

### ````zt
public func read_line(from: io.Input = io.input) -> result<optional<text>, core.Error>
```

Lee una sola línea de entrada.
Devolver`none`al llegar al EOF sin contenido.
Terminadores de línea (`\n`, `\r\n`) se eliminan.

@param from: identificador de entrada (predeterminado:`io.input`).
@return La línea se lee como`optional<text>`o error de E/S.
---`

read_all

### ````zt
public func read_all(from: io.Input = io.input) -> result<text, core.Error>
```

Lee todo el contenido disponible en la entrada hasta el EOF.

@param from: identificador de entrada (predeterminado:`io.input`).
@return Todo el texto leído o error de E/S.
---`

write

### ````zt
public func write(value: text, to: io.Output = io.output) -> result<void, core.Error>
```

Escribe texto sin formato en el flujo de salida especificado.
No agrega saltos de línea.

Valor @param: texto a escribir.
@param to – Controlador de salida (predeterminado:`io.output`).
@devolver`void`en caso de éxito o error de E/S.
---`

imprimir

