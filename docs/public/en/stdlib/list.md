# `std.list`

Small helpers for built-in `list<T>` values.

## Core Operations

- `list.is_empty(items)` returns `true` when the list has no items.
- `list.first(items)` returns the first item as `optional<T>`.
- `list.last(items)` returns the last item as `optional<T>`.
- `list.rest(items)` returns all items except the first one.
- `list.skip(items, count)` returns all items after `count` skipped items.

```zt
import std.list as list

const values: list<int> = [4, 8]
const first: optional<int> = list.first(values)
const last: optional<int> = list.last(values)
const rest: list<int> = list.rest(values)
const after_two: list<int> = list.skip(values, 2)

if first.is_some()
    print(first.or(0))
end
```

## Notes

- `list.first(empty_values)` returns `none` when `empty_values` has no items.
- `list.last(empty_values)` also returns `none`.
- `list.rest(empty_values)` returns an empty list.
- `list.rest(single_item)` also returns an empty list.
- `list.skip(items, 0)` returns a copy of `items`.
- `list.skip(items, count)` returns an empty list when `count` is greater than or equal to the list length.
- Use `optional<T>` helpers such as `.is_some()`, `.is_none()`, and `.or(fallback)`.
- Current backend support covers `list<int>` and `list<text>`.
