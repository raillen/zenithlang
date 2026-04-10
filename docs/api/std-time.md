# Módulo `std.time`

O `std.time` fornece data/hora, durações, cronômetro e suspensão síncrona ou assíncrona.

## Tipos

### `Weekday`

```zt
enum Weekday
    Sunday, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday
end
```

### `Duration`

```zt
struct Duration
    pub seconds: float
    pub func to_ms() -> float
    pub func between(start: DateTime, end: DateTime) -> Duration
end
```

### `DateTime`

```zt
struct DateTime
    pub year: int
    pub month: int
    pub day: int
    pub hour: int
    pub minute: int
    pub second: int
    pub millisecond: int
    pub weekday: Weekday
    pub unix: float
    pub func format(pattern: text) -> text
end
```

### `Stopwatch`

```zt
struct Stopwatch
    pub func start()
    pub func stop() -> Duration
    pub func reset()
end
```

## Construtores

| API | Descrição |
| :-- | :-- |
| `duration(hours: float = 0.0, minutes: float = 0.0, seconds: float = 0.0, ms: float = 0.0) -> Duration` | Monta uma duração composta. |
| `seconds(s: float) -> Duration` | Atalho para segundos. |
| `ms(milliseconds: float) -> Duration` | Atalho para milissegundos. |

## Funções

| API | Descrição |
| :-- | :-- |
| `now() -> DateTime` | Horário local atual. |
| `utc_now() -> DateTime` | Horário atual em UTC. |
| `stopwatch() -> Stopwatch` | Cria um cronômetro. |
| `sleep(d: Duration)` | Suspensão síncrona. |
| `wait(d: Duration)` | Suspensão assíncrona. |

## Exemplo

```zt
import std.time

pub func main() -> int
    var sw = time.stopwatch()
    sw.start()
    time.sleep(time.seconds(1))
    var elapsed = sw.stop()
    print("Decorrido: " + elapsed.seconds)
    return 0
end
```

## Status do runtime atual

- A implementação nativa já cobre `now`, `seconds`, `ms`, `duration`, `sleep`, `wait` e `stopwatch`.
- A superfície declarada de `utc_now`, `DateTime.format` e `Duration.between` já existe no módulo `.zt`, mas a ligação Lua ainda está em finalização.
- `wait` depende da infraestrutura async do runtime, que segue em estabilização no snapshot atual do repositório.
