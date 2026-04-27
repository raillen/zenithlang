# Módulo `std.text`

Módulo puro e locale-neutral para texto.
Opera por code point no modelo MVP atual.
Neste corte alpha, o módulo publica apenas o subset que já esta entregue com seguranca.

## Constantes e Funções

### `to_utf8`

```zt
public func to_utf8(value: text) -> bytes
```

Converte `text` válido para `bytes` UTF-8.

### `from_utf8`

```zt
public func from_utf8(value: bytes) -> result<text, text>
```

Tenta decodificar `bytes` UTF-8.
Neste corte alpha, o erro continua tipado como `text`.

### `trim`

```zt
public func trim(value: text) -> text
```

Remove whitespace nas duas extremidades.

### `trim_start`

```zt
public func trim_start(value: text) -> text
```

Remove whitespace do início.

### `trim_end`

```zt
public func trim_end(value: text) -> text
```

Remove whitespace do fim.

### `contains`

```zt
public func contains(value: text, needle: text) -> bool
```

Retorna `true` quando `needle` aparece em `value`.

### `starts_with`

```zt
public func starts_with(value: text, prefix: text) -> bool
```

Retorna `true` quando `value` comeca com `prefix`.

### `ends_with`

```zt
public func ends_with(value: text, suffix: text) -> bool
```

Retorna `true` quando `value` termina com `suffix`.

### `has_prefix`

```zt
public func has_prefix(value: text, prefix: text) -> bool
```

Alias semântico explícito para `starts_with`.

### `has_suffix`

```zt
public func has_suffix(value: text, suffix: text) -> bool
```

Alias semântico explícito para `ends_with`.

### `has_whitespace`

```zt
public func has_whitespace(value: text) -> bool
```

Retorna `true` quando existe ao menos um caractere de whitespace.

### `index_of`

```zt
public func index_of(value: text, needle: text) -> int
```

Retorna o índice da primeira ocorrencia ou `-1`.

### `last_index_of`

```zt
public func last_index_of(value: text, needle: text) -> int
```

Retorna o índice da última ocorrencia ou `-1`.

### `is_empty`

```zt
public func is_empty(value: text) -> bool
```

Retorna `true` quando o texto esta vazio.

### `is_blank`

```zt
public func is_blank(value: text) -> bool
```

Retorna `true` quando o texto esta vazio ou tem apenas espaços.

### `is_digits`

```zt
public func is_digits(value: text) -> bool
```

Retorna `true` somente para texto não vazio composto por digitos ASCII `0-9`.

### `limit`

```zt
public func limit(value: text, max_len: int) -> text
```

Corta o texto para no máximo `max_len` code points, sem adicionar sufixo.

