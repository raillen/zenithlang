# Módulo `std.format`

Módulo puro de formatação para apresentação.
Retorna `text` e não realiza I/O, parsing ou serialização.
Complementa `fmt` e `TextRepresentable<T>`.

## Constantes e Funções

### `BytesStyle`

```zt
public enum BytesStyle
    Binary
    Decimal
end
```

Estilo tipado para formatação de tamanho em bytes.
`Binary` usa base 1024 (KiB, MiB). `Decimal` usa base 1000 (KB, MB).

### `number`

```zt
public func number(value: float, decimals: int = 0) -> text
```

Formata um número com precisão decimal configurável.

@param value — Valor numérico a formatar.
@param decimals — Casas decimais (padrão: 0).
@return Texto formatado do número.

### `percent`

```zt
public func percent(value: float, decimals: int = 0) -> text
```

Formata um valor como porcentagem.
`percent(0.25)` produz `"25%"`.

@param value — Fração a formatar (0.0 a 1.0).
@param decimals — Casas decimais (padrão: 0).
@return Texto formatado como porcentagem.

### `date`

```zt
public func date(millis: int, style: text = "iso") -> text
```

Formata data a partir de milissegundos epoch, usando estilo nomeado.

@param millis — Timestamp em milissegundos desde epoch.
@param style — Estilo de formatação: `"iso"`, `"short"`, `"long"`.
@return Texto da data formatada.

### `datetime`

```zt
public func datetime(millis: int, style: text = "short", locale: text = "") -> text
```

Formata data e hora a partir de milissegundos epoch.
Locale é explícito quando utilizado.

@param millis — Timestamp em milissegundos desde epoch.
@param style — Estilo de formatação.
@param locale — Locale explícito (ex: `"pt-BR"`). Vazio para padrão.
@return Texto da data/hora formatada.

### `date_pattern`

```zt
public func date_pattern(millis: int, pattern: text) -> text
```

Formata data usando padrão explícito.

@param millis — Timestamp em milissegundos desde epoch.
@param pattern — Padrão de formatação (ex: `"yyyy-mm-dd"`).
@return Texto da data formatada segundo o padrão.

### `datetime_pattern`

```zt
public func datetime_pattern(millis: int, pattern: text) -> text
```

Formata data e hora usando padrão explícito.

@param millis — Timestamp em milissegundos desde epoch.
@param pattern — Padrão de formatação (ex: `"yyyy-mm-dd HH:mm:ss"`).
@return Texto da data/hora formatada segundo o padrão.

### `bytes`

```zt
public func bytes(value: int, style: format.BytesStyle = format.BytesStyle.Binary, decimals: int = 1) -> text
```

Formata tamanho em bytes para exibição legível.

@param value — Tamanho em bytes.
@param style — `Binary` (base 1024) ou `Decimal` (base 1000).
@param decimals — Casas decimais (padrão: 1).
@return Texto formatado (ex: `"1.5 KiB"`, `"1.5 KB"`).

### `hex`

```zt
public func hex(value: int) -> text
```

Formata inteiro em representação hexadecimal.

@param value — Valor inteiro.
@return Texto em hexadecimal.

### `bin`

```zt
public func bin(value: int) -> text
```

Formata inteiro em representação binária.

@param value — Valor inteiro.
@return Texto em binário.

