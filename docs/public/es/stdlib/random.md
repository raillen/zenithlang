# Módulo`std.random`

Módulo para la generación de números aleatorios deterministas utilizando el host PRNG.
Le permite definir semillas para reproducibilidad en pruebas.

## Constantes y funciones

### `seed`

```zt
public func seed(seed: int) -> void
```

Establece la semilla del generador pseudoaleatorio.
Usar la misma semilla siempre produce la misma secuencia de números, lo que resulta útil para pruebas deterministas.

@param seed: valor entero utilizado como semilla del PRNG.

### `next`

```zt
public func next() -> int
```

Genera el siguiente número pseudoaleatorio de la secuencia.

@return Entero pseudoaleatorio.

### `between`

```zt
public func between(min: int, max: int) -> result<int, text>
```

Genera un número pseudoaleatorio en el rango cerrado`[min, max]`.
Error de devolución si`max < min`.

@param min: límite inferior (inclusive).
@param max: límite superior (inclusive).
@return Entero aleatorio dentro del rango o mensaje de error si`max < min`.

