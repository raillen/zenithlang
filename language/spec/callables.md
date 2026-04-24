# Zenith Callables (Delegates v1)

- Status: authoritative spec (cut R3.M5)
- Date: 2026-04-24
- Scope: callable type, first-class function references, signature
  compatibility, escape rules, FFI integration
- Upstream: `language/decisions/089-callable-delegates-v1.md`
- Downstream consumers: `language/spec/concurrency.md` (`R3.M2` Phase 3),
  `stdlib/std/concurrent.zt`

## Purpose

Consolidate the official callable-delegates v1 surface into a single
authoritative document. This is the narrow first-class function-pointer
surface; captures and lambdas live on separate milestones.

## Type Syntax

```
CallableType := 'func' '(' [TypeList] ')' [ '->' Type ]
TypeList     := Type (',' Type)*
```

Examples:

```zt
func(int, int) -> int
func() -> bool
func(text)          -- void return
```

Notes:

- no parameter names in a callable type
- no default values in a callable type
- no type parameters (non-generic in v1)

## Value-Position Function Reference

A bare identifier that resolves to a top-level `func` declaration can
appear in value position when a callable type is expected:

```zt
func add(a: int, b: int) -> int
    return a + b
end

const op: func(int, int) -> int = add
```

The identifier must:

- refer to a non-generic top-level `func`
- not be a method (`func name(self, ...)` is not a callable value in v1)
- be visible under normal visibility rules (`public` / same module)

Calling through a callable variable uses the usual call syntax:

```zt
const r = op(40, 2)   -- indirect call
```

## Signature Compatibility

Two callable types `C1` and `C2` are compatible iff:

- `C1.param_count == C2.param_count`
- for every `i`, `zt_type_equals(C1.params[i], C2.params[i])`
- `zt_type_equals(C1.return, C2.return)`

No variance. No implicit conversion. A signature mismatch emits
`callable.signature_mismatch`.

## Escape Rules (v1)

A v1 callable carries no captured state (it is a bare function pointer).
The escape rules are therefore permissive for local/parametric use and
strict for stored positions:

Allowed positions:

- `let` / `var` at function scope
- function parameter
- function return
- argument to a call
- argument to an `extern c` callable parameter (see FFI below)

Rejected positions (this cut):

- `public var` at namespace scope
  (diagnostic: `callable.escape_public_var`)
- struct field
  (diagnostic: `callable.escape_struct_field`)
- collection element types: `list<func...>`, `map<K, func...>`,
  `optional<func...>`, `result<func..., E>`
  (diagnostic: `callable.escape_container`)

These restrictions keep v1 delegates trivially safe against lifetime
coupling. They will be relaxed when closures land (R3.M6) with proper
escape analysis.

## FFI Integration

This surface relaxes `Decision 011`'s MVP blanket ban on callbacks for
the narrow v1 shape only.

### Permitted

An `extern c` function may declare a parameter of callable type whose
signature uses only boundary-safe shapes:

```zt
extern c
    func qsort(base: bytes, count: int, size: int, cmp: func(int, int) -> int)
end
```

Boundary-safe shapes for callable params and return in `extern c`:

- `int`, `int32`, `int64`
- `float`, `float64`
- `bool`
- `void` return
- `text` (passed as pointer; see Decision 011)
- `bytes` (passed as pointer; see Decision 011)

### Rejected

- callable parameters or returns that use structs, enums, optional,
  result, list, map, grid, pqueue, circbuf, btreemap, btreeset, dyn
- returning a callable from an `extern c` function (deferred)
- callable appearing inside a struct passed as `extern c` argument
  (inherits the struct-arg rule from `R3.M3`)

Diagnostic for boundary violations: `callable.extern_c_signature` with
ACTION/WHY/NEXT.

## Codegen (C Target)

For each distinct callable signature that appears in a reachable position,
the emitter emits a typedef:

```
typedef <ret-c> (*zt_callable__<ret>__<p1>__<p2>__...)(<c-args>);
```

Name mangling uses short stable tokens:

- `i64` for `int` / `int64`
- `i32` for `int32`
- `f64` for `float` / `float64`
- `bool`
- `text`, `bytes`
- `void`
- `u_<name>` for user types (reserved; not emitted in v1 since structs
  are not boundary-safe)

Function-reference value: `&<mangled_func_name>`.

Indirect call: `callee(args...)` where `callee` is the callable-typed
expression.

## Examples

### Local delegate

```zt
namespace app.main

func add(a: int, b: int) -> int
    return a + b
end

public func main() -> int
    const op: func(int, int) -> int = add
    return op(40, 2)
end
```

### Higher-order function

```zt
namespace app.main

func double_it(x: int) -> int
    return x * 2
end

func apply(f: func(int) -> int, v: int) -> int
    return f(v)
end

public func main() -> int
    return apply(double_it, 21)
end
```

### FFI callback

```zt
namespace app.sort

extern c
    func qsort(base: bytes, count: int, size: int, cmp: func(int, int) -> int)
end

func compare_i64(a: int, b: int) -> int
    if a < b then
        return -1
    end
    if a > b then
        return 1
    end
    return 0
end

public func sort_inplace(buf: bytes, count: int) -> int
    sort.qsort(buf, count, 8, compare_i64)
    return 0
end
```

## Tests (authoritative fixtures)

- `tests/behavior/callable_basic` - positive, local var + indirect call
- `tests/behavior/callable_higher_order` - positive, HOF pattern
- `tests/behavior/callable_signature_mismatch_error` - negative
- `tests/behavior/callable_escape_public_var_error` - negative
- `tests/behavior/callable_escape_struct_field_error` - negative
- `tests/behavior/callable_extern_c_basic` - positive FFI
- `tests/behavior/callable_extern_c_signature_error` - negative FFI

## Non-Goals For v1

- closures with captures (R3.M6)
- lambda literals (R3.M7)
- generic callables
- returning callables from `extern c`
- callables as struct fields / container elements
- method values

## Residual Risk

- Escape rules are per-position blacklist; a later pass should switch to
  an explicit "is_callable_allowed_here" predicate once the list grows.
- Emitter mangling collides for user types with `__` in them; v1 does not
  emit user-typed callables so this is latent; must be hardened before
  v2 lands.
- FFI boundary shape list is enumerated; widening needs a spec update.
