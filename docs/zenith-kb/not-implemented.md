# Zenith - Not Implemented (Deferred Post-MVP)

## Language Features

| Feature | Status | Notes |
|---------|--------|-------|
| `unless` keyword | Deferred | Use `if not condition` |
| Ternary expressions | Deferred | Use `if/else` |
| Pipe operators | Deferred | |
| `?.` safe navigation | Deferred | Use `?` propagation + match |
| `??` null-coalescing | Deferred | No `null` in Zenith |
| Selective imports (`from ... import ...`) | Deferred | Import whole namespaces only |
| Wildcard imports (`import *`) | Deferred | |
| Relative imports | Deferred | |
| Implicit `self.` omission | Deferred | Always use `self.field` |
| Implicit `return` | Deferred | `return` is always explicit |
| `uint` standalone type | Deferred | Use `u8`/`u16`/`u32`/`u64` |
| `char` type | Deferred | |
| `**` exponentiation operator | Deferred | Use future `math.pow` |
| `//` floor division | Deferred | |
| C-style for loops | Deferred | Use collection-oriented `for` |
| `for` with numeric ranges | Deferred | Only collection-oriented `for` |
| async/await | Deferred | |
| Exception handling beyond `result<T,E>` | Deferred | |
| Pattern matching beyond `match case`/`default` | Deferred | |
| Destructuring in patterns | Deferred | |
| Method overloading | Deferred | |
| Operator overloading beyond `==`/`!=` | Deferred | Only via `Equatable` |
| Variadic parameters | Deferred | |
| First-class functions / function types | Deferred | MVP limits to named references |
| Closures capturing mutable state | Deferred | |
| Nested functions (inner `func`) | Deferred | |
| `any` type | Deferred | |
| `null` | Deferred | Never; use `optional<T>` |
| Macros | Deferred | |
| User-facing C interop | Deferred | |
| Lambda expressions | Deferred | |
| Conditional compilation in `.zt` | Deferred | |
| Doc tests | Deferred | |
| VM/bytecode execution model | Deferred | Native C backend only |

## Standard Library

| Feature | Status | Notes |
|---------|--------|-------|
| `std.fs` public API expansion | Deferred | |
| Networking and pipes | Deferred | `std.net` is next wave |
| `std.math` - `math.pow`, `math.sin`, etc | Deferred | Use `extern c` for now |
| `std.time` - dates, timestamps | Deferred | |
| `std.regex` | Deferred | |
| Public generic stream abstraction | Deferred | |
| Async IO | Deferred | |
| TLS | Deferred | |
| Websocket | Deferred | |
| Package registry integration | Deferred | |
| Optional dependencies / feature flags | Deferred | |

## Compiler

| Feature | Status | Notes |
|---------|--------|-------|
| Incremental compilation | Deferred | |
| Optimization passes beyond C compiler | Deferred | |
| Source maps / debug info generation | Deferred | `#line` directives exist |
| Plugin or extension system | Deferred | |

## Runtime

| Feature | Status | Notes |
|---------|--------|-------|
| Cycle collection in RC | Deferred | RC cycles = leak risk |
| Thread-safe reference counting | Deferred | Use isolates |
| Custom allocator hooks | Deferred | |
| Separate compilation units in C runtime | Deferred | |
| `map` specializations beyond `map<text,text>` | Deferred | |
| `list<float>` specialization | Deferred | |
| `optional<float>` and `optional<bool>` specializations | Deferred | |
| Struct runtime support (generic field access) | Deferred | |
| Full `std.text` helper surface | Deferred | Subset shipped in alpha |

## Backend Targets

| Target | Status |
|--------|--------|
| Zig | Deferred |
| LLVM | Deferred |
| WASM | Deferred |
| JavaScript | Deferred |
| Cross-compilation policy | Deferred |
| Native package installer generation | Deferred |

## ZIR Notes

- ZIR textual format (`.zir` files) = debug, fixtures, golden tests ONLY
- ZIR structured in-memory representation = canonical contract between stages
- Frontend never produces or consumes `.zir` files directly
