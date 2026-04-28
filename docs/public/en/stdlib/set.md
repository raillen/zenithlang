# `std.set`

> Status: alpha.
> Current backend support: `set<int>` and `set<text>`.

Use `std.set` for hash-based sets.

```zt
import std.set as set
```

## Core Operations

- `set.empty()` creates an empty set.
- `set.of(a, b, c)` creates a set from values.
- `set.is_empty(values)` returns `true` when the set has no items.
- `set.len(values)` returns the number of unique items.
- `set.has(values, value)` checks membership.
- `set.add(values, value)` adds one item to a mutable set.
- `set.remove(values, value)` removes one item from a mutable set.
- `set.union(left, right)` returns all items from both sets.
- `set.intersect(left, right)` returns only shared items.
- `set.difference(left, right)` returns items from `left` that are not in `right`.
- `for item in values` visits each item once.

`set.union`, `set.intersect`, and `set.difference` return a new set.

```zt
var values: set<int> = set.empty()
set.add(values, 1)
set.add(values, 2)
set.remove(values, 1)

const left: set<int> = {1, 2, 3}
const right: set<int> = {3, 4}

const both: set<int> = set.union(left, right)
const shared: set<int> = set.intersect(left, right)
const only_left: set<int> = set.difference(left, right)

var total: int = 0
for item in both
    total = total + item
end
```

## Notes

- `set.empty()` needs an expected type, such as `var values: set<int> = set.empty()`.
- `set.of(...)` can infer the element type from its values.
- Use `var`, not `const`, when calling `set.add` or `set.remove`.
- Both arguments must have the same set type.
- A `set<int>` can only combine with `set<int>`.
- A `set<text>` can only combine with `set<text>`.
- Set iteration order is not part of the language contract.
