# Module`std.random`

Module for deterministic random number generation using host PRNG.
Allows you to define seed for reproducibility in tests.

## Constants and Functions

### `seed`

```zt
public func seed(seed: int) -> void
```

Sets the seed of the pseudorandom generator.
Using the same seed always produces the same sequence of numbers, useful for deterministic testing.

@param seed — Integer value used as the seed of the PRNG.

### `next`

```zt
public func next() -> int
```

Generates the next pseudo-random number in the sequence.

@return Pseudo-random integer.

### `between`

```zt
public func between(min: int, max: int) -> result<int, text>
```

Generates a pseudorandom number in the closed range`[min, max]`.
Return error if`max < min`.

@param min — Lower limit (inclusive).
@param max — Upper limit (inclusive).
@return Random integer in range, or error message if`max < min`.

