# Módulo `std.os`

Módulo para consultas e mutações de estado do processo atual.
Processos filhos ficam em `std.os.process`.

## Constantes e Funções

### `Platform`

```zt
public enum Platform
    Windows
    Linux
    MacOS
    Unknown
end
```

Plataforma do sistema operacional.
Use `os.platform()` para obter o valor atual.

### `Arch`

```zt
public enum Arch
    X64
    X86
    Arm64
    Unknown
end
```

Arquitetura do processador.
Use `os.arch()` para obter o valor atual.

### `Error`

```zt
public enum Error
    NotFound
    PermissionDenied
    IOError
    Unknown
end
```

Erro tipado para operações do sistema operacional.

### `args`

```zt
public func args() -> list<text>
```

Retorna o vetor de argumentos do processo.

### `env`

```zt
public func env(name: text) -> optional<text>
```

Consulta uma variável de ambiente.
Retorna `none` se a variável não existir.

@param name — Nome da variável.
@return Valor da variável, ou `none`.

### `pid`

```zt
public func pid() -> int
```

Retorna o PID do processo atual.

### `platform`

```zt
public func platform() -> os.Platform
```

Retorna a plataforma do sistema operacional.

### `arch`

```zt
public func arch() -> os.Arch
```

Retorna a arquitetura do processador.

### `current_dir`

```zt
public func current_dir() -> result<text, os.Error>
```

Retorna o diretório de trabalho atual.

### `change_dir`

```zt
public func change_dir(path: text) -> result<void, os.Error>
```

Altera o diretório de trabalho atual.

@param path — Novo diretório de trabalho.
@return `void` em sucesso, ou erro.

