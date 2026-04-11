# Idioms and Best Practices: The Zenith Standard

This document defines how to write industrial Zenith code. Following these patterns ensures that your code is readable by any other developer and optimized by the Ascension compiler.

## 1. Naming and Intent

Zenith uses visual patterns to differentiate data from structures.

| Element | Pattern | Example |
| :--- | :--- | :--- |
| Variables and Functions | `snake_case` | `current_speed` |
| Constants | `SCREAMING_SNAKE` | `MAX_GRAVITY` |
| Types and Structures | `PascalCase` | `PlayerProfile` |

---

## 2. Typing: Clarity over Guesswork

*   **Context**: The compiler can infer types, but the human brain needs anchors.
*   **The Rule**: Explicitly declare the type in module-level variables or public functions.
*   **Contrast**: Unlike JavaScript or Lua, where the type is a surprise, in Zenith, the type is a contract.

```zt
-- RECOMMENDED
var level: int = 1

-- AVOID (in public code)
var level = 1
```

---

## 3. Safe Unwrapping (Safety First)

This is the most critical point for Zenith's industrial stability.

*   **Context**: Handling null values or failures.
*   **The Rule**: Always prefer the `or` operator (default value) or `match`. Use the `!` operator only after an `is_present()` check.
*   **Contrast**: Zenith discourages "Crash-Oriented Programming" (using `!` on everything), promoting resilience.

```zt
-- ✅ SAFE: Industrial code
var user = os.get_env("USER") or "admin"

-- ❌ DANGEROUS: May cause a system Panic
var user = os.get_env("USER")!
```

---

## 4. Composition over Inheritance

*   **The Rule**: Do not try to create class hierarchies. Create data (`struct`) and attach behaviors (`trait`).
*   **Advantage**: Avoids the "Diamond Problem" and makes code 100% individually testable.

---

## 5. Limits of Native (Boundary Principle)

*   **Context**: Zenith allows the use of native Lua, but this breaks the sovereignty of the language.
*   **The Rule**: Use `native lua` only for:
    1.  Access to external C libraries via FFI.
    2.  Extreme loop optimizations that do not yet exist in the core.
    3.  Interoperability with legacy code.
*   **Contrast**: Pure Zenith code is statically verifiable; native Lua code is a "black box".

---

## 6. Quality Checklist

Upon finalizing a module, verify:
- [ ] Does the code use `Outcome` for operations that may fail?
- [ ] Do `where` clauses protect variables against invalid values?
- [ ] Are function names verbal (`load_data` instead of `data`)?
- [ ] Has documentation been moved to a `.zdoc` file to keep the `.zt` clean?

---
*Zenith Specification v0.3.1 - Idioms Handbook*
