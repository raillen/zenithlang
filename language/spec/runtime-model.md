# Zenith Runtime Model Spec

- Status: canonical closure spec
- Date: 2026-04-18
- Scope: C runtime, managed values, value semantics, cleanup, panic, contracts and checks

## Purpose

The runtime must make Zenith's simple source model true.

The user sees value semantics, immutable `const` values, explicit mutation and typed recoverable errors. The implementation may use reference counting, copy-on-write and internal moves, but those choices must not leak shared mutable aliasing.

## Managed Values

Managed values include:

- `text`
- `bytes`
- `func(...)` closures
- `lazy<T>` values
- `list<T>`
- `map<K, V>`
- `optional<T>` when `T` is managed
- `result<T, E>` when either side is managed
- structs with managed fields
- enums with managed payloads

## Closure Runtime Values

Closure values are managed runtime values.

A closure value stores:

- a function pointer
- a context pointer
- an optional context drop hook

The context pointer stores captured values.

Rules:

- captures are immutable snapshots in closures v1
- managed captures are retained when the closure is created
- managed captures are released when the closure is released
- non-capturing functions still use the same `func(...)` value model
- generated Zenith functions receive an internal `zt_ctx` argument
- normal user code never writes or reads `zt_ctx`

## Lazy Runtime Values

`lazy<T>` values are managed runtime values.

R3.M8 ships `lazy<int>` as a one-shot value.

A lazy value stores:

- a closure thunk
- a consumed flag

Rules:

- creating the lazy value retains the thunk
- forcing the value calls the thunk once
- forcing releases the stored thunk after the call
- forcing the same value again raises `runtime.contract`
- ordinary expression evaluation remains eager

## Ownership Requirements

Runtime/compiler ownership rules must handle:

- local scope exit
- normal `return`
- early return through `?`
- branch exits
- loop exits through `break` and `continue`
- temporaries until end of statement
- function argument evaluation left-to-right
- construction failure
- contract panic where cleanup is viable

## Memory and Concurrency Core

The MVP is solidified on **Automatic Reference Counting (ARC)** without a tracing Garbage Collector. 

To maintain latency predictability and C-level speed, the default runtime uses **Non-Atomic ARC**. Concurrency relies on **Isolates** (message-passing/deep copy between boundaries). Atomic RC is restricted to designated explicit wrappers (e.g. `Shared<T>`).

This is a runtime contract, not a promise that the whole host process is single-threaded.

Practical reading:

- Zenith user code runs on a **single-isolate by default** path.
- The host/engine may use threads internally.
- Ordinary managed Zenith values are not shared across threads by default.
- Future user-facing concurrency should arrive as **workers/jobs/channels**, not as implicit shared mutable state.

This architecture creates a known limitation: RC cycles. 

Rules:

- ARC tracks all managed values.
- Non-Atomic ARC cannot cross thread isolates; doing so requires a safe deep-transfer structure.
- RC cycles are a leak risk, not undefined behavior
- rich callbacks, UI graphs, game object graphs and stored reference-like APIs are not stable until a cycle policy exists
- future cycle policy must choose an explicit mechanism such as `weak<T>`, handles/arenas, constrained ownership graphs or cycle collection
- the runtime must document which APIs can create cycles before those APIs become official

## Concurrency Direction

Zenith does **not** frame this as "the language is single-thread only".

The correct model is:

- default runtime path: single-isolate
- concurrency boundary: explicit isolate/worker/job boundary
- default transfer mode: deep copy
- future optimized transfer mode: move when exclusivity is provable
- explicit shared state: narrow wrappers only

This keeps ordinary code simple and predictable, while still leaving room for parallel hosts, game engines and worker-based APIs.

## Transferable Values

The long-term worker boundary should accept only transferable data.

Baseline transferable shapes:

- scalars (`int`, `float`, `bool`)
- `text`
- `bytes`
- `optional<T>` when `T` is transferable
- `result<T, E>` when both channels are transferable
- `list<T>` when `T` is transferable
- `map<K, V>` when `K` and `V` are transferable
- structs and enums whose fields/payloads are all transferable

Not transferable by default:

- live platform handles
- network connections
- raw `extern`/FFI resources
- engine scene objects with shared mutable identity
- ordinary managed values that are merely "reachable", but not explicitly transferred

## User-Facing API Direction

The intended public surface is small and explicit.

Current delivered alpha slice:

- `std.concurrent.copy_int`
- `std.concurrent.copy_bool`
- `std.concurrent.copy_float`
- `std.concurrent.copy_text`
- `std.concurrent.copy_bytes`
- `std.concurrent.copy_list_int`
- `std.concurrent.copy_list_text`
- `std.concurrent.copy_map_text_text`

These helpers make the boundary explicit today.
They are copy-based only.
`jobs.spawn/join` and channels remain the next public phase, not the current one.

Canonical direction:

```zt
const job = jobs.spawn(build_navmesh, snapshot)
const mesh = jobs.join(job)?
```

Possible later direction:

```zt
const channel = channels.create<Chunk>()
channels.send(channel, chunk)
```

Not part of the initial surface:

- raw thread handles
- mutex/condvar-first programming
- implicit cross-thread sharing of ordinary `text`, `list`, `map` and structs

## Implementation Phases

1. Document the runtime contract and make boundary-copy helpers explicit.
2. Teach the checker what "transferable" means.
3. Expose `jobs.spawn/join` on top of copy-based transfer.
4. Add move-based optimization where exclusivity is provable.
5. Add narrow explicit shared wrappers only where they are truly needed.

Current progress in the alpha tree:

- Phase 1 delivered.
- The first public boundary helpers are available in `std.concurrent`.
- Transferability analysis exists in the checker as groundwork for the next worker-facing surface.

## Stack Vs Heap Representation

`optional<T>` and `result<T,E>` should be stack/in-place values whenever practical.

Rules:

- scalar optionals/results should not allocate
- `result<void, E>` success should not allocate in the final runtime model
- managed payloads may store RC-managed pointers internally while the wrapper stays stack/in-place
- heap allocation is acceptable for escaping values, oversized payloads, managed payload internals or bootstrap implementation cuts
- heap-first wrappers are performance debt and must be tracked explicitly

## Value Semantics

Assignment and parameter passing create semantic value copies.

Implementation may use:

- direct copy for scalars
- retain/release for immutable managed values
- copy-on-write for mutable managed containers
- internal move when source is provably unused

Observable rule:

```zt
var b: list<int> = a
b[0] = 10
```

must not change `a`.

## Const Collections

`const` collections are observably immutable.

Invalid:

```zt
const items: list<Player> = [Player(name: "Julia", hp: 100)]
items[0] = Player(name: "Julia", hp: 80)
items[0].hp = 80
```

## Checks

Runtime checks include:

- list/text/bytes bounds
- map missing key for direct lookup
- division by zero where applicable
- contract `where`
- invalid UTF-8 conversion
- allocation failure
- platform failure

Expected platform failures should become `result<T, E>` in stdlib APIs. Broken invariants and direct invalid access produce panic.

## Panic (Safe Bounds and Contracts)

Zenith injects **Panic with Unwinding** (or controlled abort) rather than allowing Undefined Behavior when bounds check contracts are fundamentally broken at runtime.

Rules:

- Indexing out-of-bounds (e.g., `list[999]`) or arithmetic overflow checks inject an unconditional panic, avoiding segfaults and heap corruption.
- Panic is fatal language control flow, guaranteeing safe behavior defaults.
- Panic output must use the diagnostics model when source span and value context are available.
- Panic is not caught by result/optional flow.

## Runtime Contracts

Value-level `where` contracts run at runtime.

A struct constructor remains a constructor. It does not secretly become `result<T, E>` when fields contain `where`.

Expected recoverable validation should be written as an explicit result-returning API, such as `try_create_*`.

Canonical recoverable validation example:

```zt
struct User
    age: int where it >= 0
end

func try_create_user(age: int) -> result<User, text>
    if age < 0
        return error("age must be >= 0")
    end

    return success(User(age: age))
end
```

Required MVP sites:

- struct field construction
- struct field assignment
- function and method parameter boundary

Failed contracts report `runtime.contract`.

## Definition Of Done

Runtime hardening is complete only when:

- managed values have explicit ownership rules
- retain/release paths are tested
- COW/value semantics are behavior-tested
- all supported early exits release correctly
- runtime failures report structured diagnostics
- const collection mutation is rejected or impossible through supported operations
- RC cycle policy is documented before cycle-prone APIs become stable
- optional/result wrappers use stack/in-place representation where practical
- heap-first wrappers are tracked as performance debt
