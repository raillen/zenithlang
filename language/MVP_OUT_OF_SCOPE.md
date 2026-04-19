# MVP Out of Scope

Explicit list of what is **not** part of the minimum viable product frontend.

## Language features deferred post-MVP

- `unless` keyword
- ternary expressions
- pipe operators
- `?.` safe navigation
- `??` null-coalescing
- selective imports (`from ... import ...`)
- wildcard imports (`import *`)
- implicit `self.` omission
- implicit `return`
- `uint` as standalone type
- `char` type
- math operators as syntax (`**`, `//`)
- C-style for loops
- `for` with numeric ranges (only collection-oriented `for`)
- async/await
- exception handling beyond `result<T,E>`
- pattern matching beyond `match case`/`default`
- destructuring in patterns
- method overloading
- operator overloading beyond `==`/`!=` via `Equatable`
- variadic parameters
- first-class functions / function types as generic arguments (MVP limits to named references)
- closures capturing mutable state
- nested functions (inner `func`)

## Standard library deferred post-MVP

- `stdlib/process` — CLI args, env vars, process exit codes beyond `main` return
- `stdlib/io` — filesystem, networking, pipes
- `stdlib/math` — `math.pow`, `math.sin`, etc (use `extern c` for now)
- `stdlib/time` — dates, timestamps
- `stdlib/regex` — pattern matching on text

## Compiler features deferred post-MVP

- Multi-file compilation (MVP: single file, single namespace)
- `zenith.ztproj` parsing (MVP: single-file mode via CLI argument)
- ZDoc parsing
- Incremental compilation
- Optimization passes beyond what the C compiler provides
- Source maps / debug information generation
- Plugin or extension system

## Runtime features deferred post-MVP

- Cycle collection in RC
- Thread-safe reference counting
- Custom allocator hooks
- Separate compilation units in the C runtime
- `map` specializations beyond `map<text,text>`
- `list<float>` specialization
- `optional<float>` and `optional<bool>` specializations
- Struct runtime support (generic field access)
- Text operations beyond basic concat/index/slice/eq/len

## Backend targets deferred post-MVP

- Zig target
- LLVM target
- WASM target
- JavaScript target

## ZIR contractual status

- ZIR **textual** format (`.zir` files) is for debug, fixtures, and golden tests only
- ZIR **structured** in-memory representation is the canonical contract between compiler stages
- The frontend never produces or consumes `.zir` files directly