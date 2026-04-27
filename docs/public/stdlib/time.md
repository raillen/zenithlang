# Módulo `std.time`

Módulo temporal centrado em `Instant` e `Duration`.
Usa `time.now()` como ponto de entrada do relógio do sistema.
Comparação usa operadores ordinários (`==`, `<`, `>`).

## Constantes e Funções

### `Instant`

```zt
public struct Instant
    millis: int
end
```

Representa um momento absoluto (timestamp em milissegundos desde epoch).

### `Duration`

```zt
public struct Duration
    millis: int
end
```

Representa um intervalo de tempo em milissegundos.

### `Error`

```zt
public enum Error
    SleepInterrupted
    Unknown
end
```

Erro tipado para operações temporais.

### `now`

```zt
public func now() -> time.Instant
```

Retorna o instante atual do relógio do sistema.

### `sleep`

```zt
public func sleep(duration: time.Duration) -> result<void, time.Error>
```

Suspende a execução pelo período especificado.

@param duration — Duração do sleep.
@return `void` em sucesso, ou erro temporal.

### `since`

```zt
public func since(start: time.Instant) -> time.Duration
```

Retorna a duração desde `start` até o momento atual.

### `until`

```zt
public func until(target: time.Instant) -> time.Duration
```

Retorna a duração do momento atual até `target`.

### `diff`

```zt
public func diff(a: time.Instant, b: time.Instant) -> time.Duration
```

Retorna a diferença temporal entre dois instantes (`b - a`).

### `add`

```zt
public func add(at: time.Instant, duration: time.Duration) -> time.Instant
```

Avança um instante pela duração especificada.

### `sub`

```zt
public func sub(at: time.Instant, duration: time.Duration) -> time.Instant
```

Retrocede um instante pela duração especificada.

### `from_unix`

```zt
public func from_unix(ts: int) -> time.Instant
```

Cria um `Instant` a partir de timestamp Unix em segundos.

### `from_unix_ms`

```zt
public func from_unix_ms(ts: int) -> time.Instant
```

Cria um `Instant` a partir de timestamp Unix em milissegundos.

### `to_unix`

```zt
public func to_unix(at: time.Instant) -> int
```

Converte um `Instant` para timestamp Unix em segundos.

### `to_unix_ms`

```zt
public func to_unix_ms(at: time.Instant) -> int
```

Converte um `Instant` para timestamp Unix em milissegundos.

### `milliseconds`

```zt
public func milliseconds(n: int) -> time.Duration
```

Cria uma duração de `n` milissegundos.

### `seconds`

```zt
public func seconds(n: int) -> time.Duration
```

Cria uma duração de `n` segundos.

### `minutes`

```zt
public func minutes(n: int) -> time.Duration
```

Cria uma duração de `n` minutos.

### `hours`

```zt
public func hours(n: int) -> time.Duration
```

Cria uma duração de `n` horas.

