# Cómo pasar una función como valor

> Audiencia: usuario
> Estado: actual
> Superficie: pública
> Fuente de la verdad: no

## Problema

Quieres pasar una función a otra parte del código.

En Zenith, use el tipo invocable:

```zt
func(int, int) -> int
```

## Respuesta corta

Indique la firma explícitamente.

```zt
const op: func(int, int) -> int = add
```

## Ejemplo completo

```zt
namespace app.main

func add(a: int, b: int) -> int
    return a + b
end

func run_op(op: func(int, int) -> int) -> int
    return op(3, 4)
end

public func main() -> int
    const op: func(int, int) -> int = add
    return run_op(op)
end
```

## Por qué

El tipo invocable muestra:

- cuántos parámetros recibe la función;
- qué tipos recibe;
- qué tipo devuelve.

Esto evita devoluciones de llamadas ocultas.

## Error común

No intentes utilizar una función genérica como invocable en este corte.

Prefiere una función simple de nivel superior:

```zt
func add(a: int, b: int) -> int
    return a + b
end
```

## Límites actuales

Los callables v1 son estrechos a propósito.

En este corte evitar:

- invocable como campo de estructura;
- lista/mapa interior invocable;
- aplicación parcial;
- método como valor;
- función genérica como valor.

## Ver también

- `language/spec/callables.md`
- `language/decisions/089-callable-delegates-v1.md`
