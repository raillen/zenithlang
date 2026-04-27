# Módulo`std.format`

Módulo de formato puro para presentación.
Devolver`text`y no realiza E/S, análisis ni serialización.
Complementos`fmt`y`TextRepresentable<T>`.

## Constantes y funciones

### `BytesStyle`

```zt
public enum BytesStyle
    Binary
    Decimal
end
```

Estilo escrito para formato de tamaño en bytes.`Binary`utiliza base 1024 (KiB, MiB).`Decimal`utiliza base 1000 (KB, MB).

### `number`

```zt
public func number(value: float, decimals: int = 0) -> text
```

Formatea un número con precisión decimal configurable.

@param value: valor numérico al que se va a dar formato.
@param decimales: lugares decimales (predeterminado: 0).
@return Texto formateado del número.

### `percent`

```zt
public func percent(value: float, decimals: int = 0) -> text
```

Formatea un valor como porcentaje.`percent(0.25)`produce`"25%"`.

Valor @param: fracción a formatear (0,0 a 1,0).
@param decimales: lugares decimales (predeterminado: 0).
@return Texto formateado como porcentaje.

### `date`

```zt
public func date(millis: int, style: text = "iso") -> text
```

Los formatos datan de milisegundos de época y utilizan un estilo con nombre.

@param millis: marca de tiempo en milisegundos desde la época.
Estilo @param - Estilo de formato:`"iso"`, `"short"`, `"long"`.
@return Texto de fecha formateado.

### `datetime`

```zt
public func datetime(millis: int, style: text = "short", locale: text = "") -> text
```

Da formato a la fecha y la hora desde milisegundos de época.
La configuración regional es explícita cuando se usa.

@param millis: marca de tiempo en milisegundos desde la época.
Estilo @param: estilo de formato.
@param locale: configuración regional explícita (por ejemplo:`"pt-BR"`). Vacío por defecto.
@return Texto formateado de fecha/hora.

### `date_pattern`

```zt
public func date_pattern(millis: int, pattern: text) -> text
```

Formatea la fecha usando un patrón explícito.

@param millis: marca de tiempo en milisegundos desde la época.
@param patrón — Patrón de formato (por ejemplo:`"yyyy-mm-dd"`).
@return Texto de fecha formateado según el estándar.

### `datetime_pattern`

```zt
public func datetime_pattern(millis: int, pattern: text) -> text
```

Da formato a la fecha y la hora utilizando un patrón explícito.

@param millis: marca de tiempo en milisegundos desde la época.
@param patrón — Patrón de formato (por ejemplo:`"yyyy-mm-dd HH:mm:ss"`).
@return Texto de fecha/hora formateado según el estándar.

### `bytes`

```zt
public func bytes(value: int, style: format.BytesStyle = format.BytesStyle.Binary, decimals: int = 1) -> text
```

Formatea el tamaño en bytes para una visualización legible.

Valor @param: tamaño en bytes.
@estiloparam—`Binary`(base 1024) o`Decimal`(base 1000).
@param decimales: lugares decimales (predeterminado: 1).
@return Texto formateado (por ejemplo:`"1.5 KiB"`, `"1.5 KB"`).

### `hex`

```zt
public func hex(value: int) -> text
```

Formatea números enteros en representación hexadecimal.

Valor @param: valor entero.
@return Texto en hexadecimal.

### `bin`

```zt
public func bin(value: int) -> text
```

Formatea un número entero en representación binaria.

Valor @param: valor entero.
@return Texto en binario.

