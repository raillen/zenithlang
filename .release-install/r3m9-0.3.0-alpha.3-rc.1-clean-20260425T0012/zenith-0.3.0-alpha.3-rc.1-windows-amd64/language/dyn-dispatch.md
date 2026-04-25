# Zenith Dynamic Dispatch Specification

- Status: authoritative spec (cut R3.M4)
- Date: 2026-04-23
- Scope: `dyn<Trait>` syntax, vtable generation, fat pointers, heterogeneous collections, subset limits
- Upstream: `language/decisions/088-dyn-dispatch-minimum-subset.md`
- Cross-reference: `language/spec/surface-syntax.md` (section `Generics, Constraints and Dynamic Dispatch`), `language/decisions/079-memory-and-dispatch-architecture.md`

## Purpose

Define the official semantics of `dyn<Trait>` for dynamic dispatch in Zenith, including the minimum subset delivered in R3.M4.

## Syntax

```zt
dyn<TraitName>
dyn<TraitName<TypeArg>>  -- only for core traits with type params (e.g., TextRepresentable)
```

`dyn` is a type constructor that takes a trait name and produces a dynamic dispatch type.

## Fat Pointer Representation

A `dyn<Trait>` value is a **fat pointer** containing:

1. A pointer to the boxed concrete value
2. A pointer to the vtable for that concrete type implementing the trait

The box is a reference-counted managed value. When the RC reaches zero, the boxed value is dropped.

## Vtable Structure

For each `(concrete_type, trait)` pair, the compiler generates a vtable containing:

- `drop`: function to free the boxed value
- `clone_out`: function to deep-copy the boxed value
- One function pointer per trait method, in declaration order

Vtables are generated once per `(concrete_type, trait)` pair and are shared across all instances.

## Boxing

When a value of type `S` is used where `dyn<T>` is expected:

1. The compiler checks that `apply T to S` exists
2. The value is deep-coped into a new box
3. The box is tagged with the vtable for `(S, T)`
4. The fat pointer (box + vtable) is the `dyn<T>` value

Boxing takes ownership: the original value is moved into the box.

## Method Dispatch

Calling a method on `dyn<T>`:

```zt
let drawable: dyn Drawable = Circle(radius: 10)
drawable.draw()  -- indirect call through vtable
```

The compiler:

1. Loads the vtable pointer from the fat pointer
2. Loads the function pointer for `draw` from the vtable
3. Calls the function with the data pointer as receiver

## Minimum Subset (R3.M4)

### Allowed

- Traits with only non-mutating methods (`func`, not `mut func`)
- Traits with up to 8 methods
- Traits with copyable parameter and return types
- Non-generic traits
- `list<dyn<Trait>>` and other generic collections with dyn elements

### Disallowed (with diagnostic)

| Case | Diagnostic |
|------|------------|
| `mut func` in trait used with dyn | `dyn.mut_method` |
| Generic trait with dyn | `dyn.generic_trait` |
| Trait with >8 methods | `dyn.too_many_methods` |
| Uncopyable param/return type | `dyn.uncopyable` |
| No `apply` found for concrete type | `dyn.no_apply` |
| `dyn<T>` in `extern c` signature | `dyn.ffi_unsafe` |

## Copyable Types

A type is copyable for dyn purposes if it is:

- A scalar (`int`, `float`, `bool`, `int8`..`int64`, `uint8`..`uint64`)
- `text` or `bytes`
- `optional<T>` where `T` is copyable
- A struct or enum where all fields/payloads are copyable
- `list<T>` where `T` is copyable

Not copyable:

- `dyn<T>` itself (no nested dyn in this subset)
- `extern` handles or FFI resources
- Types with non-copyable fields

## Ownership Rules

1. Boxing moves the value into the box (original is consumed)
2. The box is RC-managed; cloning clones the inner value
3. `dyn<T>` values cannot cross isolate boundaries
4. `dyn<T>` values cannot be passed to `extern c` functions
5. `dyn<T>` values cannot be stored in `public var` (namespace state)

## Examples

### Basic heterogeneous collection

```zt
trait Drawable
    func draw() -> text
end

struct Circle
    radius: int
end

apply Drawable to Circle
    func draw() -> text
        return "Circle(r=" + to_text(self.radius) + ")"
    end
end

struct Rectangle
    width: int
    height: int
end

apply Drawable to Rectangle
    func draw() -> text
        return "Rectangle(" + to_text(self.width) + "x" + to_text(self.height) + ")"
    end
end

func render_all(shapes: list<dyn Drawable>) -> text
    var result: text = ""
    for shape in shapes
        result = result + shape.draw() + "\n"
    end
    return result
end
```

### Boxing at call site

```zt
var shapes: list<dyn Drawable> = []
shapes.append(Circle(radius: 10))       -- Circle is boxed
shapes.append(Rectangle(width: 5, height: 3))  -- Rectangle is boxed

render_all(shapes)
```

## Performance Characteristics

- **Memory overhead per dyn value**: one box allocation + one RC counter
- **Memory overhead per (type, trait) pair**: one vtable (static, shared)
- **Dispatch overhead**: one indirect call per method invocation
- **No inline caching** in R3.M4 (deferred to future optimization)

## Implementation Status

| Component | Status |
|-----------|--------|
| Parser `dyn<Trait>` | Delivered (pre-R3.M4) |
| Type checker `dyn<Trait>` validation | Delivered R3.M4 |
| Vtable generation (C backend) | Delivered R3.M4 |
| Boxing code generation | Delivered R3.M4 |
| Dynamic method dispatch | Delivered R3.M4 |
| `list<dyn<Trait>>` support | Delivered R3.M4 |
| Diagnostics for subset limits | Delivered R3.M4 |
| `dyn<TextRepresentable>` legacy path | Migrated to vtable model R3.M4 |
