# Module`std.format`

Pure formatting module for presentation.
Return`text`and does not perform I/O, parsing or serialization.
Complements`fmt`and`TextRepresentable<T>`.

## Constants and Functions

### `BytesStyle`

```zt
public enum BytesStyle
    Binary
    Decimal
end
```

Typed style for size formatting in bytes.`Binary`uses base 1024 (KiB, MiB).`Decimal`uses base 1000 (KB, MB).

### `number`

```zt
public func number(value: float, decimals: int = 0) -> text
```

Formats a number with configurable decimal precision.

@param value — Numeric value to format.
@param decimals — Decimal places (default: 0).
@return Formatted text of the number.

### `percent`

```zt
public func percent(value: float, decimals: int = 0) -> text
```

Formats a value as a percentage.`percent(0.25)`produces`"25%"`.

@param value — Fraction to format (0.0 to 1.0).
@param decimals — Decimal places (default: 0).
@return Text formatted as a percentage.

### `date`

```zt
public func date(millis: int, style: text = "iso") -> text
```

Formats date from epoch milliseconds, using named style.

@param millis — Timestamp in milliseconds since epoch.
@param style — Formatting style:`"iso"`, `"short"`, `"long"`.
@return Formatted date text.

### `datetime`

```zt
public func datetime(millis: int, style: text = "short", locale: text = "") -> text
```

Formats date and time from epoch milliseconds.
Locale is explicit when used.

@param millis — Timestamp in milliseconds since epoch.
@param style — Formatting style.
@param locale — Explicit locale (e.g.:`"pt-BR"`). Empty for default.
@return Formatted date/time text.

### `date_pattern`

```zt
public func date_pattern(millis: int, pattern: text) -> text
```

Formats date using explicit pattern.

@param millis — Timestamp in milliseconds since epoch.
@param pattern — Formatting pattern (ex:`"yyyy-mm-dd"`).
@return Date text formatted according to the standard.

### `datetime_pattern`

```zt
public func datetime_pattern(millis: int, pattern: text) -> text
```

Formats date and time using explicit pattern.

@param millis — Timestamp in milliseconds since epoch.
@param pattern — Formatting pattern (ex:`"yyyy-mm-dd HH:mm:ss"`).
@return Date/time text formatted according to the standard.

### `bytes`

```zt
public func bytes(value: int, style: format.BytesStyle = format.BytesStyle.Binary, decimals: int = 1) -> text
```

Formats size in bytes for readable display.

@param value — Size in bytes.
@param style —`Binary`(base 1024) or`Decimal`(base 1000).
@param decimals — Decimal places (default: 1).
@return Formatted text (ex:`"1.5 KiB"`, `"1.5 KB"`).

### `hex`

```zt
public func hex(value: int) -> text
```

Formats integer in hexadecimal representation.

@param value — Integer value.
@return Text in hexadecimal.

### `bin`

```zt
public func bin(value: int) -> text
```

Formats integer into binary representation.

@param value — Integer value.
@return Text in binary.

