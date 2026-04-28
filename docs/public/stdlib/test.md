# Módulo `std.test`

Módulo complementar para testes.
Testes são declarados com `attr test`.
Use `check(...)` para condições simples.
Use `std.test` quando quiser falhar, pular ou comparar valores com mensagem mais clara.

## Constantes e Funções

### `fail`

```zt
public func fail(message: text = "test failed") -> void
```

Marca o teste atual como falho imediatamente.
Útil em branches de controle de fluxo, por exemplo depois de um `match` sobre `result`.

@param message - Mensagem de falha.

### `skip`

```zt
public func skip(reason: text = "") -> void
```

Marca o teste atual como pulado.
Skip e um resultado distinto: não e pass nem fail.

@param reason - Motivo do skip.

### `throws`

```zt
public func throws(body: func() -> void) -> void
```

Falha se `body` terminar sem erro.
Use para testar caminhos que devem chamar `panic(...)` ou outro erro fatal.

@param body - Funcao sem parametros que deve falhar.

### `is_true`

```zt
public func is_true(value: bool) -> void
```

Falha se `value` for `false`.
A mensagem mostra `expected true` e `received false`.

### `is_false`

```zt
public func is_false(value: bool) -> void
```

Falha se `value` for `true`.
A mensagem mostra `expected false` e `received true`.

### `equal_int`

```zt
public func equal_int(actual: int, expected: int) -> void
```

Falha se `actual != expected`.
A mensagem mostra esperado e recebido.

### `equal_text`

```zt
public func equal_text(actual: text, expected: text) -> void
```

Falha se `actual != expected`.
A mensagem mostra esperado e recebido.

### `not_equal_int`

```zt
public func not_equal_int(actual: int, expected: int) -> void
```

Falha se `actual == expected`.
A mensagem mostra que o valor deveria ser diferente.

### `not_equal_text`

```zt
public func not_equal_text(actual: text, expected: text) -> void
```

Falha se `actual == expected`.
A mensagem mostra que o valor deveria ser diferente.

