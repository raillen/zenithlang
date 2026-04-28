#Guías de biblioteca estándar

> Guías públicas Zenith stdlib.
> Público: usuario, autor del paquete
> Superficie: pública
> Estado: actual

## Estado actual

stdlib todavía está en fase alfa.

Utilice únicamente API documentadas y validadas por pruebas al escribir ejemplos públicos.

## Módulos comunes

- `std.io`: entrada y salida básica.
-`std.text`: ayudantes seguros de texto con corte alfa.
-`std.bytes`: base para datos binarios.
-`std.fs`: sistema de archivos sincrónico.
-`std.fs.path`: operaciones de ruta.
-`std.json`: JSON.
-`std.math`: matemáticas básicas.
-`std.regex`: regex simple para validar y buscar texto.
-`std.random`: alfa aleatorio, con cuidado del estado compartido.
-`std.concurrent`: copia explícita para los límites de la competencia.
-`std.test`: fallar, omitir y comparaciones simples para pruebas.
-`std.list`: pequeños ayudantes para`list<T>`.
- `std.map`: pequeños ayudantes para`map<K,V>`.

## Pequeñas pruebas con`std.test`

```zt
import std.test as test

attr test
public func score_starts_at_zero() -> void
    const score: int = 0
    test.equal_int(score, 0)
    test.is_true(score == 0)
end
```

Cuando la comparación falla, prefiera ayudantes que muestren lo esperado y lo recibido:

- `test.equal_int(actual, expected)`
- `test.equal_text(actual, expected)`
- `test.not_equal_int(actual, expected)`
- `test.not_equal_text(actual, expected)`

## Regla de uso

Prefiere pequeños ejemplos.

Si el compilador aún no admite semánticamente una API, no la publique como una guía pública.

## Referencias

- Texto y archivos:`docs/public/stdlib/text-and-files.md`.
- Validación, matemáticas y aleatoria:`docs/public/stdlib/validation-math-random.md`.
- Regex: `docs/public/es/stdlib/regex.md`.
- referencia de biblioteca estándar:`docs/reference/stdlib/README.md`.
- KB de stdlib:`docs/reference/zenith-kb/stdlib.md`.
- Modelo normativo:`language/spec/stdlib-model.md`.
- ZDoc de stdlib:`stdlib/zdoc/`.
