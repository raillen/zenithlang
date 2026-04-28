# Zenith Foreign Function Interface (FFI) Model

- Status: authoritative spec (cut R3.M3)
- Date: 2026-04-23
- Scope: `extern c`, `extern host`, ABI contract, ownership rules at boundary, allowed/blocked type matrix
- Upstream: `language/decisions/011-extern-c-and-extern-host.md`, `language/spec/runtime-model.md`, `language/spec/concurrency.md`
- Related runtime implementation: `compiler/targets/c/emitter.c` (`c_emit_ffi_shielded_zir_call_statement`, `c_type_to_c`, `c_extern_call_expected_arg_type`)

## Purpose

Formalize the current Zenith FFI boundary so that contributors can reason about what `extern c` does at runtime, what the emitter guarantees, and what the user must not attempt today.

This document is normative. Deviation from it is a bug.

## Scope Of This Cut

FFI 1.0 ships:

- `extern c` as the only target-native binding form;
- `extern host` as the abstract boundary to runtime/embedder capabilities (used internally by stdlib; not the user-facing C interop surface);
- automatic reference-count shielding for managed arguments crossing the boundary;
- a curated allowed type matrix for extern signatures.

Not in this cut:

- symbol renaming (`extern "C" name("other")`);
- explicit ABI annotations (`__stdcall`, `__cdecl`, etc.);
- callbacks / function pointers in `extern` declarations;
- raw pointer surface types;
- `extern` variables (only function declarations);
- variadic `extern` functions;
- conditional `extern` declarations per target.

These remain `Decision 011` out-of-scope and are revisited in a later FFI revision.

## Syntax

Canonical form, at namespace scope only:

```zt
namespace app.platform

extern c
    func puts(message: text) -> int
end
```

Rules:

- `extern c` and `extern host` are block forms; each block may contain one or more function declarations;
- declarations inside the block have no body;
- the declared name is the external symbol name in this cut (no renaming);
- the declaration is addressable using its unqualified name after import.

## ABI Contract

### Target binding

- `extern c` uses the C ABI of the host toolchain. The Zenith compiler currently emits through the host C compiler (`gcc` on the reference build), so the effective ABI is whatever that toolchain implements.
- No ABI annotation is emitted. The user cannot force `__stdcall`/`__cdecl` in this cut.

### Cross-platform guarantee

- The same Zenith `extern c` declaration is portable across any platform whose C toolchain accepts the emitted C declaration with equivalent semantics.
- Platform-specific symbols (for example Windows-only or POSIX-only functions) must be wrapped behind a runtime helper or conditionally compiled at the C layer, not at the Zenith layer (conditional `extern` is out of scope).

### No silent conversion

- The compiler does not invent casts. A mismatched argument type at the call site produces `type.mismatch` at the Zenith call site (see the negative fixture).
- The emitter refuses to lower to C if the Zenith type cannot be mapped (`C_EMIT_UNSUPPORTED_TYPE`).

## Allowed / Blocked Type Matrix

Types allowed at the FFI boundary in this cut:

| Zenith type | C representation | Allowed as arg? | Allowed as return? | Notes |
|---|---|---|---|---|
| `int` | `zt_int` (64-bit signed) | yes | yes | primitive roundtrip |
| `float` | `zt_float` (double) | yes | yes | primitive roundtrip |
| `bool` | `zt_bool` | yes | yes | primitive roundtrip |
| `text` | `zt_text *` | yes (shielded) | yes (managed return) | passed as managed pointer; see Ownership |
| `bytes` | `zt_bytes *` | yes (shielded) | yes (managed return) | passed as managed pointer |
| `list<int>` | `zt_list_i64 *` | yes (shielded) | yes (managed return) | passed as managed pointer |
| `list<text>` | `zt_list_text *` | yes (shielded) | yes (managed return) | passed as managed pointer |
| `list<float>` | `zt_list_f64 *` | yes (shielded) | yes (managed return) | passed as managed pointer |
| `map<text,text>` | `zt_map_text_text *` | yes (shielded) | yes (managed return) | passed as managed pointer |
| `optional<T>` where `T` allowed | value with discriminator | yes | yes | inline `zt_optional_*` structs |
| `result<T,E>` / `outcome<T,E>` | generated struct | yes | yes | see `compiler/targets/c/emitter.c` type mapping |

Types **blocked** at the FFI boundary in this cut:

- user-defined `struct` as direct `extern c` argument (rejected by type system when a different type is declared);
- user-defined `enum` with payload (no stable C layout for payloads);
- function types (callbacks are out of scope);
- raw pointers (no surface syntax);
- `dyn Trait` values (the boxed representation is private to the runtime);
- any type whose `c_type_to_c` mapping fails (`C_EMIT_UNSUPPORTED_TYPE` at emit time).

If a direct struct argument is truly required on the C side, the user must expose a helper in the Zenith runtime / stdlib that accepts allowed primitives, and move the struct shape across the boundary as a sequence of primitive/managed arguments.

## Ownership And Lifetime At The Boundary

### Managed arguments: automatic shielding

When a managed value (`text`, `bytes`, `list<T>`, `map<K,V>`, ...) is passed to an `extern c` function, the emitter inserts a shielding block around the call:

```
{
    zt_text *zt_ffi_arg0 = greeting;
    if (zt_ffi_arg0 != NULL) { zt_retain(zt_ffi_arg0); }
    length = zt_text_len(zt_ffi_arg0);
    if (zt_ffi_arg0 != NULL) { zt_release(zt_ffi_arg0); }
}
```

Guarantees:

- the managed value's reference count is bumped before the call;
- the managed value is released after the call returns (even if the call stores the pointer internally);
- the value remains alive for the duration of the call;
- the caller's original binding is unaffected.

Implementation entry points:

- `c_emit_ffi_shielded_zir_call_statement` (statement form);
- `c_emit_ffi_shielded_zir_return` (return form);
- legacy variants `c_emit_ffi_shielded_legacy_call_statement` / `c_emit_ffi_shielded_legacy_return`.

### Managed returns

A managed return value (`zt_text *`, `zt_list_*_t *`, etc.) is treated as newly owned by the caller. The runtime ARC path then takes over normal lifetime management.

### What the C side must NOT do

- do not store the received managed pointer beyond the call duration unless the C code is part of the Zenith runtime and uses `zt_retain`/`zt_release` explicitly;
- do not free the managed pointer directly (ARC is managed by Zenith);
- do not mutate the internal layout of `zt_text`, `zt_bytes`, `zt_list_*`, `zt_map_*` directly; use the runtime helpers.

### Raw `owned` across the boundary

- there is no surface `owned<T>` in this cut, so the concept of "transferring ownership to C" is handled implicitly by the copy/shielding contract;
- the Phase 4 move-based transfer described in `language/spec/concurrency.md` is **not** available for FFI in this cut.

## Diagnostics

Expected diagnostics at the FFI boundary:

- `type.mismatch` at the call site when an argument type does not match the declared signature (covered by `tests/behavior/extern_c_struct_arg_error`);
- `type.invalid_call` / `type.invalid_argument` for arity mismatches at an `extern c` call site;
- `backend.c.emit` (`C_EMIT_UNSUPPORTED_TYPE`) when a Zenith type cannot be lowered to C (emitted during code generation, not during semantic check);
- `project.unresolved_import` when the namespace that declares the `extern c` block is not importable.

All diagnostics use the action-first renderer (ACTION / WHY / NEXT) per `R3.M1`.

## Tests

Current coverage:

- positive, primitive return: `tests/behavior/extern_c_puts_e2e` (`puts(text) -> int`);
- positive, managed-arg shielding + primitive return: `tests/behavior/extern_c_text_len_e2e` (`zt_text_len(text) -> int`);
- working external-library example: `examples/c-bindings-sqlite3` uses a C shim around SQLite so raw pointers stay outside Zenith;
- negative, struct-as-arg: `tests/behavior/extern_c_struct_arg_error` (user struct where `text` expected; `type.mismatch` at call site);
- negative, non-transferable at related `std.concurrent` boundary: `tests/behavior/std_concurrent_boundary_copy_unsupported_error` (for the concurrent helper wrapper, not the direct `extern c` surface).

Deferred (FFI 1.x follow-ups):

- arity negative fixture at `extern c` call site;
- invalid return type negative fixture;
- callback / function-pointer support with tests;
- wider binding suite (libcurl, platform helpers).

## Implementation Phases (FFI)

| Phase | Scope | Status |
|---|---|---|
| 1 | `extern c` declaration + emit + shielding + spec + positive/negative minimal tests | delivered (this cut) |
| 2 | Arity and invalid return negative fixtures; explicit runtime helper matrix | follow-up in R3 |
| 3 | Callbacks / function pointers | deferred beyond R3 |
| 4 | ABI annotations and symbol renaming | deferred beyond R3 |

## Cross References

- `language/decisions/011-extern-c-and-extern-host.md` (canonical syntax decision)
- `language/spec/runtime-model.md` (ARC + boundary policy)
- `language/spec/concurrency.md` (transferable shapes, R3.P1.A integration)
- `docs/internal/reports/release/R3.M3-ffi-1.0-report.md` (milestone report)
- `docs/internal/reports/R3-risk-matrix.md` (`R3-RISK-020`, `R3-RISK-021`, `R3-RISK-022`)

## Residual Risk

- ABI stability across compilers (MSVC vs MinGW vs gcc on Linux) is not fully explored; the current tree is tested on `gcc` / Windows. See `R3-RISK-020`.
- Managed-return ownership is handled by the runtime, but a long-lived C pointer cached in a C static struct would be unsafe. The spec forbids it, but no static analysis catches it. See `R3-RISK-021`.
- Callback interop requires `R3.M5` (callables + delegates). Tracked as `R3-RISK-022`.
