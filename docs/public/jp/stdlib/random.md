# モジュール`std.random`

ホスト PRNG を使用して決定論的乱数を生成するモジュール。
テストでの再現性のためのシードを定義できます。

## 定数と関数

### `seed`

```zt
public func seed(seed: int) -> void
```

擬似乱数生成器のシードを設定します。
同じシードを使用すると、常に同じ一連の数値が生成されるため、決定論的なテストに役立ちます。

@param シード — PRNG のシードとして使用される整数値。

### `next`

```zt
public func next() -> int
```

シーケンス内の次の擬似乱数を生成します。

@return 擬似乱数の整数。

### `between`

```zt
public func between(min: int, max: int) -> result<int, text>
```

閉じた範囲で擬似乱数を生成します`[min, max]`。
次の場合はエラーを返します`max < min`。

@param min — 下限値 (両端を含む)。
@param max — 上限（両端を含む）。
@return 範囲内のランダムな整数、またはエラーメッセージの場合`max < min`。

