#Standard Library Guides

> Zenith stdlib public guides.
> Audience: user, package-author
> Surface: public
> Status: current

## Current status

stdlib is still in alpha phase.

Use only documented, test-validated APIs when writing public examples.

## Common modules

- `std.io`: basic input and output.
-`std.text`: alpha cut text safe helpers.
-`std.bytes`: base for binary data.
-`std.fs`: synchronous filesystem.
-`std.fs.path`: path operations.
-`std.json`:JSON.
-`std.math`: basic mathematics.
-`std.random`: random alpha, with care for shared state.
-`std.concurrent`: explicit copy for competition boundaries.
-`std.test`: fail, skip and simple comparisons for testing.
-`std.list`: small helpers for`list<T>`.
- `std.map`: small helpers for`map<K,V>`.

## Small tests with`std.test`

```zt
import std.test as test

attr test
public func score_starts_at_zero() -> void
    const score: int = 0
    test.equal_int(score, 0)
    test.is_true(score == 0)
end
```

When a comparison fails, prefer helpers that show expected and received:

- `test.equal_int(actual, expected)`
- `test.equal_text(actual, expected)`
- `test.not_equal_int(actual, expected)`
- `test.not_equal_text(actual, expected)`

## Usage rule

Prefer small examples.

If an API is not yet semantically supported by the compiler, do not post it as a public guide.

## References

- Text and files:`docs/public/stdlib/text-and-files.md`.
- Validation, mathematics and random:`docs/public/stdlib/validation-math-random.md`.
- stdlib reference:`docs/reference/stdlib/README.md`.
- KB of stdlib:`docs/reference/zenith-kb/stdlib.md`.
- Normative model:`language/spec/stdlib-model.md`.
- ZDoc from stdlib:`stdlib/zdoc/`.
