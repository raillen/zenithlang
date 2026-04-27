# Módulo`std.collections`

Módulo con 8 tipos de estructuras de datos además de`list`y`map`nativos: Queue, Stack, Grid2D, Grid3D, PriorityQueue, CircularBuffer, BTreeMap y BTreeSet.
Todas las variantes existen para`int`y`text`. Las mutaciones devuelven nuevas copias (COW - Copia en escritura) mediante operaciones`*_owned`en el tiempo de ejecución de C.

## Constantes y funciones

### `QueueNumberDequeueResult`

```zt
public struct QueueNumberDequeueResult
    queue: list<int>
    value: optional<int>
end
```

Resultado típico de`queue_int_dequeue`: devuelve la cola actualizada y el elemento eliminado.

### `QueueTextDequeueResult`

```zt
public struct QueueTextDequeueResult
    queue: list<text>
    value: optional<text>
end
```

Resultado típico de`queue_text_dequeue`: devuelve la cola actualizada y el elemento eliminado.

### `StackNumberPopResult`

```zt
public struct StackNumberPopResult
    stack: list<int>
    value: optional<int>
end
```

Resultado típico de`stack_int_pop`: devuelve la pila actualizada y el elemento eliminado de la parte superior.

### `StackTextPopResult`

```zt
public struct StackTextPopResult
    stack: list<text>
    value: optional<text>
end
```

Resultado típico de`stack_text_pop`: devuelve la pila actualizada y el elemento eliminado de la parte superior.

### `queue_int_new`

```zt
public func queue_int_new() -> list<int>
public func queue_text_new() -> list<text>
```

Crea una cola vacía.

### `queue_text_new`

```zt
public func queue_int_new() -> list<int>
public func queue_text_new() -> list<text>
```

Crea una cola vacía.

### `queue_int_enqueue`

```zt
public func queue_int_enqueue(queue: list<int>, value: int) -> list<int>
public func queue_text_enqueue(queue: list<text>, value: text) -> list<text>
```

Agrega un elemento al final de la cola. Devuelve la cola actualizada.

### `queue_text_enqueue`

```zt
public func queue_int_enqueue(queue: list<int>, value: int) -> list<int>
public func queue_text_enqueue(queue: list<text>, value: text) -> list<text>
```

Agrega un elemento al final de la cola. Devuelve la cola actualizada.

### `queue_int_dequeue`

```zt
public func queue_int_dequeue(queue: list<int>) -> collections.QueueNumberDequeueResult
public func queue_text_dequeue(queue: list<text>) -> collections.QueueTextDequeueResult
```

Elimina el elemento del frente y devuelve un paquete con la cola actualizada y el elemento eliminado.

### `queue_text_dequeue`

```zt
public func queue_int_dequeue(queue: list<int>) -> collections.QueueNumberDequeueResult
public func queue_text_dequeue(queue: list<text>) -> collections.QueueTextDequeueResult
```

Remove o elemento da frente e devolve a fila atualizada junto com o item removido.

### `queue_int_peek`

```zt
public func queue_int_peek(queue: list<int>) -> optional<int>
public func queue_text_peek(queue: list<text>) -> optional<text>
```

Devuelve el elemento frontal sin eliminarlo.

## Pila

### `queue_text_peek`

```zt
public func queue_int_peek(queue: list<int>) -> optional<int>
public func queue_text_peek(queue: list<text>) -> optional<text>
```

Devuelve el elemento frontal sin eliminarlo.

## Pila

### `stack_int_new`

```zt
public func stack_int_new() -> list<int>
public func stack_text_new() -> list<text>
```

Crea una pila vacía.

### `stack_text_new`

```zt
public func stack_int_new() -> list<int>
public func stack_text_new() -> list<text>
```

Crea una pila vacía.

### `stack_int_push`

```zt
public func stack_int_push(stack: list<int>, value: int) -> list<int>
public func stack_text_push(stack: list<text>, value: text) -> list<text>
```

Apila un elemento encima.

### `stack_text_push`

```zt
public func stack_int_push(stack: list<int>, value: int) -> list<int>
public func stack_text_push(stack: list<text>, value: text) -> list<text>
```

Apila un elemento encima.

### `stack_int_pop`

```zt
public func stack_int_pop(stack: list<int>) -> collections.StackNumberPopResult
public func stack_text_pop(stack: list<text>) -> collections.StackTextPopResult
```

Elimina el elemento superior y devuelve un paquete con la pila actualizada y el elemento eliminado.

### `stack_text_pop`

```zt
public func stack_int_pop(stack: list<int>) -> collections.StackNumberPopResult
public func stack_text_pop(stack: list<text>) -> collections.StackTextPopResult
```

Elimina el elemento superior y devuelve la pila actualizada junto con el elemento eliminado.

### `stack_int_peek`

```zt
public func stack_int_peek(stack: list<int>) -> optional<int>
public func stack_text_peek(stack: list<text>) -> optional<text>
```

Consulta la parte superior sin quitar.

## Grid2D (matriz bidimensional)

### `stack_text_peek`

```zt
public func stack_int_peek(stack: list<int>) -> optional<int>
public func stack_text_peek(stack: list<text>) -> optional<text>
```

Consulta la parte superior sin quitar.

## Grid2D (matriz bidimensional)

### `grid2d_int_new`

```zt
public func grid2d_int_new(rows: int, cols: int) -> grid2d<int>
public func grid2d_text_new(rows: int, cols: int) -> grid2d<text>
```

Crea una cuadrícula 2D con`rows`líneas y`cols`columnas. Los valores comienzan en cero/vacío.

### `grid2d_text_new`

```zt
public func grid2d_int_new(rows: int, cols: int) -> grid2d<int>
public func grid2d_text_new(rows: int, cols: int) -> grid2d<text>
```

Crea una cuadrícula 2D con`rows`líneas y`cols`columnas. Los valores comienzan en cero/vacío.

### `grid2d_int_get`

```zt
public func grid2d_int_get(grid: grid2d<int>, row: int, col: int) -> int
public func grid2d_text_get(grid: grid2d<text>, row: int, col: int) -> text
```

Lee el valor en la posición`[row, col]`.

### `grid2d_text_get`

```zt
public func grid2d_int_get(grid: grid2d<int>, row: int, col: int) -> int
public func grid2d_text_get(grid: grid2d<text>, row: int, col: int) -> text
```

Lee el valor en la posición`[row, col]`.

### `grid2d_int_set`

```zt
public func grid2d_int_set(grid: grid2d<int>, row: int, col: int, value: int) -> grid2d<int>
public func grid2d_text_set(grid: grid2d<text>, row: int, col: int, value: text) -> grid2d<text>
```

Establece el valor en la posición`[row, col]`. Devuelve la grilla actualizada (COW).

### `grid2d_text_set`

```zt
public func grid2d_int_set(grid: grid2d<int>, row: int, col: int, value: int) -> grid2d<int>
public func grid2d_text_set(grid: grid2d<text>, row: int, col: int, value: text) -> grid2d<text>
```

Establece el valor en la posición`[row, col]`. Devuelve la grilla actualizada (COW).

### `grid2d_int_fill`

```zt
public func grid2d_int_fill(grid: grid2d<int>, value: int) -> grid2d<int>
public func grid2d_text_fill(grid: grid2d<text>, value: text) -> grid2d<text>
```

Llena todas las celdas con el mismo valor. Devuelve la cuadrícula actualizada.

### `grid2d_text_fill`

```zt
public func grid2d_int_fill(grid: grid2d<int>, value: int) -> grid2d<int>
public func grid2d_text_fill(grid: grid2d<text>, value: text) -> grid2d<text>
```

Llena todas las celdas con el mismo valor. Devuelve la cuadrícula actualizada.

### `grid2d_int_rows`

```zt
public func grid2d_int_rows(grid: grid2d<int>) -> int
public func grid2d_text_rows(grid: grid2d<text>) -> int
```

Devuelve el número de líneas.

### `grid2d_text_rows`

```zt
public func grid2d_int_rows(grid: grid2d<int>) -> int
public func grid2d_text_rows(grid: grid2d<text>) -> int
```

Devuelve el número de líneas.

### `grid2d_int_cols`

```zt
public func grid2d_int_cols(grid: grid2d<int>) -> int
public func grid2d_text_cols(grid: grid2d<text>) -> int
```

Devuelve el número de columnas.

### `grid2d_text_cols`

```zt
public func grid2d_int_cols(grid: grid2d<int>) -> int
public func grid2d_text_cols(grid: grid2d<text>) -> int
```

Devuelve el número de columnas.

### `grid2d_int_size`

```zt
public func grid2d_int_size(grid: grid2d<int>) -> int
public func grid2d_text_size(grid: grid2d<text>) -> int
```

Devuelve el número total de celdas (`rows * cols`).

## Grid3D (matriz tridimensional)

### `grid2d_text_size`

```zt
public func grid2d_int_size(grid: grid2d<int>) -> int
public func grid2d_text_size(grid: grid2d<text>) -> int
```

Devuelve el número total de celdas (`rows * cols`).

## Grid3D (matriz tridimensional)

### `grid3d_int_new`

```zt
public func grid3d_int_new(depth: int, rows: int, cols: int) -> grid3d<int>
public func grid3d_text_new(depth: int, rows: int, cols: int) -> grid3d<text>
```

Crea una cuadrícula 3D de dimensiones.`depth × rows × cols`.

### `grid3d_text_new`

```zt
public func grid3d_int_new(depth: int, rows: int, cols: int) -> grid3d<int>
public func grid3d_text_new(depth: int, rows: int, cols: int) -> grid3d<text>
```

Crea una cuadrícula 3D de dimensiones.`depth × rows × cols`.

### `grid3d_int_get`

```zt
public func grid3d_int_get(grid: grid3d<int>, layer: int, row: int, col: int) -> int
public func grid3d_text_get(grid: grid3d<text>, layer: int, row: int, col: int) -> text
```

Lee el valor en la posición`[layer, row, col]`.

### `grid3d_text_get`

```zt
public func grid3d_int_get(grid: grid3d<int>, layer: int, row: int, col: int) -> int
public func grid3d_text_get(grid: grid3d<text>, layer: int, row: int, col: int) -> text
```

Lee el valor en la posición`[layer, row, col]`.

### `grid3d_int_set`

```zt
public func grid3d_int_set(grid: grid3d<int>, layer: int, row: int, col: int, value: int) -> grid3d<int>
public func grid3d_text_set(grid: grid3d<text>, layer: int, row: int, col: int, value: text) -> grid3d<text>
```

Establece el valor en la posición`[layer, row, col]`. Devuelve la cuadrícula actualizada (COW).

### `grid3d_text_set`

```zt
public func grid3d_int_set(grid: grid3d<int>, layer: int, row: int, col: int, value: int) -> grid3d<int>
public func grid3d_text_set(grid: grid3d<text>, layer: int, row: int, col: int, value: text) -> grid3d<text>
```

Establece el valor en la posición`[layer, row, col]`. Devuelve la cuadrícula actualizada (COW).

### `grid3d_int_fill`

```zt
public func grid3d_int_fill(grid: grid3d<int>, value: int) -> grid3d<int>
public func grid3d_text_fill(grid: grid3d<text>, value: text) -> grid3d<text>
```

Llena todas las celdas con el mismo valor.

### `grid3d_text_fill`

```zt
public func grid3d_int_fill(grid: grid3d<int>, value: int) -> grid3d<int>
public func grid3d_text_fill(grid: grid3d<text>, value: text) -> grid3d<text>
```

Llena todas las celdas con el mismo valor.

### `grid3d_int_depth`

```zt
public func grid3d_int_depth(grid: grid3d<int>) -> int
public func grid3d_text_depth(grid: grid3d<text>) -> int
```

Devuelve el número de capas (profundidad).

### `grid3d_text_depth`

```zt
public func grid3d_int_depth(grid: grid3d<int>) -> int
public func grid3d_text_depth(grid: grid3d<text>) -> int
```

Devuelve el número de capas (profundidad).

### `grid3d_int_rows`

```zt
public func grid3d_int_rows(grid: grid3d<int>) -> int
public func grid3d_int_cols(grid: grid3d<int>) -> int
public func grid3d_int_size(grid: grid3d<int>) -> int
public func grid3d_text_rows(grid: grid3d<text>) -> int
public func grid3d_text_cols(grid: grid3d<text>) -> int
public func grid3d_text_size(grid: grid3d<text>) -> int
```

Dimensiones y celdas totales (`depth * rows * cols`).

## PriorityQueue (cola de prioridad: montón mínimo)

### `cols`

```zt
public func grid3d_int_rows(grid: grid3d<int>) -> int
public func grid3d_int_cols(grid: grid3d<int>) -> int
public func grid3d_int_size(grid: grid3d<int>) -> int
public func grid3d_text_rows(grid: grid3d<text>) -> int
public func grid3d_text_cols(grid: grid3d<text>) -> int
public func grid3d_text_size(grid: grid3d<text>) -> int
```

Dimensiones y celdas totales (`depth * rows * cols`).

## PriorityQueue (cola de prioridad: montón mínimo)
---

--- @target: tamaño y grid3d_text_rows```zt
public func grid3d_int_rows(grid: grid3d<int>) -> int
public func grid3d_int_cols(grid: grid3d<int>) -> int
public func grid3d_int_size(grid: grid3d<int>) -> int
public func grid3d_text_rows(grid: grid3d<text>) -> int
public func grid3d_text_cols(grid: grid3d<text>) -> int
public func grid3d_text_size(grid: grid3d<text>) -> int
```

Dimensiones y celdas totales (`depth * rows * cols`).

## PriorityQueue (cola de prioridad: montón mínimo)

### `cols`

```zt
public func grid3d_int_rows(grid: grid3d<int>) -> int
public func grid3d_int_cols(grid: grid3d<int>) -> int
public func grid3d_int_size(grid: grid3d<int>) -> int
public func grid3d_text_rows(grid: grid3d<text>) -> int
public func grid3d_text_cols(grid: grid3d<text>) -> int
public func grid3d_text_size(grid: grid3d<text>) -> int
```

Dimensiones y celdas totales (`depth * rows * cols`).

## PriorityQueue (cola de prioridad: montón mínimo)

### `size`

```zt
public func grid3d_int_rows(grid: grid3d<int>) -> int
public func grid3d_int_cols(grid: grid3d<int>) -> int
public func grid3d_int_size(grid: grid3d<int>) -> int
public func grid3d_text_rows(grid: grid3d<text>) -> int
public func grid3d_text_cols(grid: grid3d<text>) -> int
public func grid3d_text_size(grid: grid3d<text>) -> int
```

Dimensiones y celdas totales (`depth * rows * cols`).

## PriorityQueue (cola de prioridad: montón mínimo)

### `pqueue_int_new`

```zt
public func pqueue_int_new() -> pqueue<int>
public func pqueue_text_new() -> pqueue<text>
```

Crea un montón mínimo vacío. Los elementos más pequeños tienen mayor prioridad.

### `pqueue_text_new`

```zt
public func pqueue_int_new() -> pqueue<int>
public func pqueue_text_new() -> pqueue<text>
```

Crea un montón mínimo vacío. Los elementos más pequeños tienen mayor prioridad.

### `pqueue_int_push`

```zt
public func pqueue_int_push(heap: pqueue<int>, value: int) -> pqueue<int>
public func pqueue_text_push(heap: pqueue<text>, value: text) -> pqueue<text>
```

Inserta un elemento manteniendo la propiedad del montón.

### `pqueue_text_push`

```zt
public func pqueue_int_push(heap: pqueue<int>, value: int) -> pqueue<int>
public func pqueue_text_push(heap: pqueue<text>, value: text) -> pqueue<text>
```

Inserta un elemento manteniendo la propiedad del montón.

### `pqueue_int_pop`

```zt
public func pqueue_int_pop(heap: pqueue<int>) -> optional<int>
public func pqueue_text_pop(heap: pqueue<text>) -> optional<text>
```

Elimina y devuelve el elemento más pequeño. Devolver`none` se vazio.

### `pqueue_text_pop`

```zt
public func pqueue_int_pop(heap: pqueue<int>) -> optional<int>
public func pqueue_text_pop(heap: pqueue<text>) -> optional<text>
```

Elimina y devuelve el elemento más pequeño. Devolver`none`si está vacío.

### `pqueue_int_peek`

```zt
public func pqueue_int_peek(heap: pqueue<int>) -> optional<int>
public func pqueue_text_peek(heap: pqueue<text>) -> optional<text>
```

Consulta el elemento más pequeño sin eliminarlo.

### `pqueue_text_peek`

```zt
public func pqueue_int_peek(heap: pqueue<int>) -> optional<int>
public func pqueue_text_peek(heap: pqueue<text>) -> optional<text>
```

Consulta el elemento más pequeño sin eliminarlo.

### `pqueue_int_len`

```zt
public func pqueue_int_len(heap: pqueue<int>) -> int
public func pqueue_text_len(heap: pqueue<text>) -> int
```

Devuelve el número de elementos del montón.

### `pqueue_text_len`

```zt
public func pqueue_int_len(heap: pqueue<int>) -> int
public func pqueue_text_len(heap: pqueue<text>) -> int
```

Devuelve el número de elementos del montón.

### `pqueue_int_is_empty`

```zt
public func pqueue_int_is_empty(heap: pqueue<int>) -> bool
public func pqueue_text_is_empty(heap: pqueue<text>) -> bool
```

Devolver`true`si el montón está vacío.

## Búfer circular

### `pqueue_text_is_empty`

```zt
public func pqueue_int_is_empty(heap: pqueue<int>) -> bool
public func pqueue_text_is_empty(heap: pqueue<text>) -> bool
```

Devolver`true`si el montón está vacío.

## Búfer circular

### `circbuf_int_new`

```zt
public func circbuf_int_new(capacity: int) -> circbuf<int>
public func circbuf_text_new(capacity: int) -> circbuf<text>
```

Crea un buffer circular con capacidad fija. Al llenar, nuevo`push`sobrescribir los más antiguos.

### `circbuf_text_new`

```zt
public func circbuf_int_new(capacity: int) -> circbuf<int>
public func circbuf_text_new(capacity: int) -> circbuf<text>
```

Crea un buffer circular con capacidad fija. Al llenar, nuevo`push`sobrescribir los más antiguos.

### `circbuf_int_push`

```zt
public func circbuf_int_push(buf: circbuf<int>, value: int) -> circbuf<int>
public func circbuf_text_push(buf: circbuf<text>, value: text) -> circbuf<text>
```

Inserta un elemento. Si está lleno, sobrescribe el más antiguo.

### `circbuf_text_push`

```zt
public func circbuf_int_push(buf: circbuf<int>, value: int) -> circbuf<int>
public func circbuf_text_push(buf: circbuf<text>, value: text) -> circbuf<text>
```

Inserta un elemento. Si está lleno, sobrescribe el más antiguo.

### `circbuf_int_pop`

```zt
public func circbuf_int_pop(buf: circbuf<int>) -> optional<int>
public func circbuf_text_pop(buf: circbuf<text>) -> optional<text>
```

Elimina y devuelve el elemento más antiguo. Devolver`none`si está vacío.

### `circbuf_text_pop`

```zt
public func circbuf_int_pop(buf: circbuf<int>) -> optional<int>
public func circbuf_text_pop(buf: circbuf<text>) -> optional<text>
```

Elimina y devuelve el elemento más antiguo. Devolver`none`si está vacío.

### `circbuf_int_peek`

```zt
public func circbuf_int_peek(buf: circbuf<int>) -> optional<int>
public func circbuf_text_peek(buf: circbuf<text>) -> optional<text>
```

Consulta el más antiguo sin eliminar.

### `circbuf_text_peek`

```zt
public func circbuf_int_peek(buf: circbuf<int>) -> optional<int>
public func circbuf_text_peek(buf: circbuf<text>) -> optional<text>
```

Consulta el más antiguo sin eliminar.

### `circbuf_int_len`

```zt
public func circbuf_int_len(buf: circbuf<int>) -> int
public func circbuf_text_len(buf: circbuf<text>) -> int
```

Número de elementos almacenados actualmente.

### `circbuf_text_len`

```zt
public func circbuf_int_len(buf: circbuf<int>) -> int
public func circbuf_text_len(buf: circbuf<text>) -> int
```

Número de elementos almacenados actualmente.

### `circbuf_int_capacity`

```zt
public func circbuf_int_capacity(buf: circbuf<int>) -> int
public func circbuf_text_capacity(buf: circbuf<text>) -> int
```

Capacidad máxima de buffer.

### `circbuf_text_capacity`

```zt
public func circbuf_int_capacity(buf: circbuf<int>) -> int
public func circbuf_text_capacity(buf: circbuf<text>) -> int
```

Capacidad máxima de buffer.

### `circbuf_int_is_full`

```zt
public func circbuf_int_is_full(buf: circbuf<int>) -> bool
public func circbuf_text_is_full(buf: circbuf<text>) -> bool
```

Devolver`true`si`len == capacity`.

### `circbuf_text_is_full`

```zt
public func circbuf_int_is_full(buf: circbuf<int>) -> bool
public func circbuf_text_is_full(buf: circbuf<text>) -> bool
```

Devolver`true`si`len == capacity`.

### `circbuf_int_is_empty`

```zt
public func circbuf_int_is_empty(buf: circbuf<int>) -> bool
public func circbuf_text_is_empty(buf: circbuf<text>) -> bool
```

Devolver`true`si está vacío.

## BTreeMap (mapa ordenado por clave)

### `circbuf_text_is_empty`

```zt
public func circbuf_int_is_empty(buf: circbuf<int>) -> bool
public func circbuf_text_is_empty(buf: circbuf<text>) -> bool
```

Devolver`true`si está vacío.

## BTreeMap (mapa ordenado por clave)

### `btreemap_text_new`

```zt
public func btreemap_text_new() -> btreemap<text, text>
```

Crea un mapa ordenado vacío. Implementado con matriz ordenada + búsqueda binaria.

### `btreemap_text_set`

```zt
public func btreemap_text_set(self_map: btreemap<text, text>, key: text, value: text) -> btreemap<text, text>
```

Inserta o actualiza un par clave-valor. Devuelve el mapa actualizado (COW).

### `btreemap_text_get`

```zt
public func btreemap_text_get(self_map: btreemap<text, text>, key: text) -> text
```

Búsqueda asertiva: devuelve el valor o provoca pánico si la clave no existe.

### `btreemap_text_get_optional`

```zt
public func btreemap_text_get_optional(self_map: btreemap<text, text>, key: text) -> optional<text>
```

Búsqueda segura: devoluciones`none`si la clave no existe.

### `btreemap_text_contains`

```zt
public func btreemap_text_contains(self_map: btreemap<text, text>, key: text) -> bool
```

Comprueba si existe una clave en el mapa.

### `btreemap_text_remove`

```zt
public func btreemap_text_remove(self_map: btreemap<text, text>, key: text) -> btreemap<text, text>
```

Elimina una llave. Devuelve el mapa actualizado (COW).

### `btreemap_text_len`

```zt
public func btreemap_text_len(self_map: btreemap<text, text>) -> int
```

Devuelve el número de pares clave-valor.

### `btreemap_text_is_empty`

```zt
public func btreemap_text_is_empty(self_map: btreemap<text, text>) -> bool
```

Devolver`true`si el mapa está vacío.

## BTreeSet (conjunto ordenado)

### `btreeset_text_new`

```zt
public func btreeset_text_new() -> btreeset<text>
```

Crea un conjunto ordenado vacío. No permite duplicados.

### `btreeset_text_insert`

```zt
public func btreeset_text_insert(set: btreeset<text>, value: text) -> btreeset<text>
```

Inserta un elemento. Si ya existe, el conjunto no cambia.

### `btreeset_text_contains`

```zt
public func btreeset_text_contains(set: btreeset<text>, value: text) -> bool
```

Comprueba si el elemento está presente.

### `btreeset_text_remove`

```zt
public func btreeset_text_remove(set: btreeset<text>, value: text) -> btreeset<text>
```

Elimina un elemento. Devuelve el conjunto actualizado.

### `btreeset_text_len`

```zt
public func btreeset_text_len(set: btreeset<text>) -> int
```

Devuelve el número de elementos.

### `btreeset_text_is_empty`

```zt
public func btreeset_text_is_empty(set: btreeset<text>) -> bool
```

Devolver`true`si el conjunto está vacío.

