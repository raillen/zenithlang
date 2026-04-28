# Zenith Closures (v1)

> **SUPERSEDED** — This document is superseded by `language/spec/language-reference.md` (Decision 094, 2026-04-28).
> Kept for historical reference only.

- Status: superseded
- Date: 2026-04-24
- Scope: anonymous function expressions, local named functions, immutable capture, runtime representation
- Upstream: `language/decisions/090-closures-v1.md`

## Purpose

This document specifies the authoritative syntax and semantics for Closures in Zenith (v1). Closures provide the ability to declare inline or local functions that capture environment variables to be carried alongside the function pointer.

## Type Syntax and Convergence

The callable type syntax remains unchanged from M5:
```zt
func(int, text) -> bool
```

However, starting in M6, **all `func` types in Zenith represent Fat Pointers**.
A value of type `func(...)` internally holds both:
1. The C function pointer.
2. A generic `void* ctx` context pointer containing captured variables.

## Closure Expressions

Closures can be declared as anonymous expressions or nested named declarations.

### Anonymous Function Expressions
An expression starting with `func` evaluates to a closure fat pointer.

```zt
const get_multiplier = func() -> func(int) -> int
    const factor = 10
    return func(v: int) -> int
        return v * factor
    end
end

const op = get_multiplier()
print(op(5)) -- 50
```

### Expression Lambdas

Lambdas v1 are a short expression-bodied form of closures.

```zt
const twice: func(int) -> int = func(value: int) => value * 2
```

Rules:

- lambdas use the same `func(...)` type as closures
- parameters must be typed
- the return type is inferred from the expected callable type
- the body must be one expression
- captures are immutable snapshots, exactly like closures v1

Use the long `func ... end` form when the body needs statements.

### Local Named Functions
A `func` declaration inside a block acts as a local closure. The function name is tightly bound to that block's scope.

```zt
func process_items(items: list<int>, threshold: int) -> list<int>
    func is_valid(val: int) -> bool
        return val >= threshold
    end

    return list.filter(items, is_valid)
end
```

## Immutable Capture Rules

Zenith enforces strict readability and ownership logic. In v1:
- State from outer scopes is captured **by value** (an immutable snapshot).
- Mutating a captured variable inside the closure scope is a semantic error (`closure.mut_capture_unsupported`).
- `public var` is not a global variable. It is namespace-owned module state.
- A closure may read a `public var` through normal namespace access rules.
- A closure may not use capture syntax to create hidden shared mutable state.

```zt
func counter() -> func() -> int
    var count = 0
    return func() -> int
        count = count + 1   -- ERROR: cannot mutate captured variables
        return count
    end
end
```

## Relationship With `mut`, Structs and Traits

Closures v1 do not change method signatures.

Rules:

- `mut func` still means the method may mutate its receiver.
- A closure body may call a `mut func` only when it has a mutable receiver available.
- Capturing a variable does not make the captured snapshot mutable.
- Capturing `self` in a method follows the same rule: the captured value is immutable in the closure.
- Traits do not gain implicit mutable capture.
- Future mutable closure capture must be designed as a separate feature.

This keeps two ideas separate:

- method receiver mutation: controlled by `mut func`
- closure environment mutation: not available in closures v1

## ARC Integration and Lifecycle

Closures that capture variables which require Automatic Reference Counting (ARC), such as `text`, `list`, `map`, or custom structs containing managed fields, are fully supported.

1. **Retain**: When the closure expression is evaluated, the runtime executes `zt_retain` for all captured managed references, incrementing their reference counts.
2. **Persistence**: The fat pointer containing these references is safely heap-allocated. It can be passed to other functions, returned, or stored without leaking or prematurely deallocating the captured data.
3. **Release**: Every generated closure includes a compiler-emitted disposal routine. When the closure pointer itself is no longer referenced, its disposal hook executes, calling `zt_release` on all captured memory constraints.

## FFI Integration

Because `func` values are Fat Pointers, Zenith enforces boundary checks when passing them to Foreign Function Interfaces (`extern c`).

- Passing a pure, non-capturing top-level function into an `extern c` parameter typing `func(...)` is **allowed**. The compiler gracefully forwards only the underlying raw C function pointer.
- Passing an anonymous closure or a local function that captures state to `extern c` is **forbidden**. The type checker will emit `callable.extern_c_closure_unsupported` because C ABI function pointers cannot implicitly carry the required `ctx` struct.
