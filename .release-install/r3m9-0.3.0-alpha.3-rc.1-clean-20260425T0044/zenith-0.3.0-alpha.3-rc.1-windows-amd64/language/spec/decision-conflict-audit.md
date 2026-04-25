# Decision Conflict Audit (M21)

- Status: completed audit
- Date: 2026-04-18
- Scope: conflicts between older decision documents and canonical specs

## Purpose

This file records explicit reconciliations where older accepted decisions still contain examples or rules that conflict with current canonical specs.

Canonical specs remain the source of truth.

## Reconciled Conflicts

### C-001: Value-level `where` on return types

Conflict source:

- `decisions/014-contracts-refinement-and-field-invariants.md` includes return-level `where`.

Canonical rule:

- `surface-syntax.md` and `decisions/031-runtime-where-contracts.md` disallow value-level `where` on function return types in the MVP.

Resolution:

- return-level `where` from Decision 014 is superseded.
- runtime `where` applies only to fields and parameters in MVP.

### C-002: `map[key]` returning optional

Conflict source:

- `decisions/013-mvp-semantic-validation.md` states `map[key]` has type `optional<V>`.

Canonical rule:

- `surface-syntax.md` and `decisions/032-generic-collections-and-update-block.md` define strict indexing:
  - `map[key]` returns `V`
  - missing key panics
  - safe lookup belongs to explicit API (`map.get` direction)

Resolution:

- Decision 013 map-lookup typing is superseded by Decision 032 and current specs.

### C-003: Manifest schema `[layout]`, `source_root`, `zdoc_root`

Conflict source:

- `decisions/015-zenith-ztproj-and-package-model.md` uses `[layout]`, `source_root`, `zdoc_root`.
- `decisions/016-zdoc-and-doc-linking.md` also references `source_root` and `zdoc_root`.

Canonical rule:

- `project-model.md` and `decisions/051-zenith-ztproj-project-manifest.md` define:
  - `[source] root`
  - `[zdoc] root`
  - `[app] entry`
  - `[lib] root_namespace`

Resolution:

- Decision 015/016 manifest field names are superseded by Decision 051 and canonical project spec.

### C-004: Optional match notation using `some(...)` case labels

Conflict source:

- `decisions/030-question-propagation-for-result-and-optional.md` and `decisions/037-panic-fatal-errors-and-attempt.md` contain examples with `case some(value)`.

Canonical rule:

- `surface-syntax.md` defines optional pattern as:
  - `case value name:`
  - `case none:`
- `some(value)` is not canonical MVP syntax.

Resolution:

- optional match examples using `case some(...)` are historical and superseded by current surface syntax.

### C-005: Entrypoint error channel narrowing

Conflict source:

- `decisions/022-entrypoint-and-program-model.md` examples emphasize `result<..., text>`.

Canonical rule:

- `decisions/057-main-entrypoint-typed-error-results.md` amends entrypoint error handling:
  - typed error channels are allowed when renderable at process boundary.

Resolution:

- Decision 022 should be read as amended by Decision 057.

### C-006: Mutating method marker migration to `mut func`

Conflict source:

- older decisions and examples use legacy suffix-based mutating method names.

Canonical rule:

- `surface-syntax.md` and `decisions/082-syntax-accessibility-ergonomics.md` define mutating method declarations with `mut func`.

Resolution:

- suffix-based mutating method examples are historical and superseded by `mut func`.
- the compiler does not provide transition compatibility for suffix-based mutating declarations or calls.

### C-007: Interpolation model (`fmt`) versus escaped interpolation in ordinary strings

Conflict source:

- older examples use escaped interpolation inside ordinary string literals.

Canonical rule:

- `surface-syntax.md` and `decisions/036-text-concatenation-and-interpolation.md` define `fmt "..."` as the canonical interpolation form.

Resolution:

- escaped interpolation examples in ordinary strings are superseded by `fmt` interpolation.

### C-008: Text conversion surface (`to_text(value)`) versus method-style conversion

Conflict source:

- older examples use method-style value conversion.

Canonical rule:

- `surface-syntax.md` defines explicit conversion as `to_text(value)` in user-facing source.

Resolution:

- method-style conversion examples are historical and superseded for user-facing docs.

### C-009: Collection append naming (`append/prepend`) versus `push`

Conflict source:

- older examples and deferred notes mention `push`.

Canonical rule:

- `surface-syntax.md` defines list mutation names as `append` and `prepend`.

Resolution:

- `push` examples are non-canonical and superseded by `append/prepend`.
## Non-Conflicts (Clarified)

- `-> void` appears in older decisions as non-canonical examples; current spec keeps omission as canonical and may diagnose explicit `-> void`.
- `pub` examples in older decisions are explicitly listed as non-canonical and do not conflict with `public`.
- strict map indexing and safe lookup API are intentionally split between core syntax and future stdlib APIs.

## Operational Rule

When an older decision conflicts with canonical specs:

1. canonical specs under `language/spec/` win.
2. explicit later decisions that amend earlier ones win.
3. this audit document acts as reconciliation index for maintainers.

## Follow-up

Maintainers should prefer updating historical decision headers with `Superseded in part by ...` notes when touching those files, but that annotation pass is not required to keep the canonical rules valid.


