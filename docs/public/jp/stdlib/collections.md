# モジュール`std.collections`

8種類のデータ構造を備えたモジュール`list`そして`map`ネイティブ: Queue、Stack、Grid2D、Grid3D、PriorityQueue、CircularBuffer、BTreeMap、BTreeSet。
すべてのバリアントが存在します`int`そして`text`。ミューテーションはオペレーションを通じて新しいコピー (COW — コピーオンライト) を返します。`*_owned`C ランタイムで。

## 定数と関数

### `QueueNumberDequeueResult`

```zt
public struct QueueNumberDequeueResult
    queue: list<int>
    value: optional<int>
end
```

典型的な結果`queue_int_dequeue`: 更新されたキューと削除されたアイテムを返します。

### `QueueTextDequeueResult`

```zt
public struct QueueTextDequeueResult
    queue: list<text>
    value: optional<text>
end
```

典型的な結果`queue_text_dequeue`: 更新されたキューと削除されたアイテムを返します。

### `StackNumberPopResult`

```zt
public struct StackNumberPopResult
    stack: list<int>
    value: optional<int>
end
```

典型的な結果`stack_int_pop`: 更新されたスタックと先頭から削除された項目を返します。

### `StackTextPopResult`

```zt
public struct StackTextPopResult
    stack: list<text>
    value: optional<text>
end
```

典型的な結果`stack_text_pop`: 更新されたスタックと先頭から削除された項目を返します。

### `queue_int_new`

```zt
public func queue_int_new() -> list<int>
public func queue_text_new() -> list<text>
```

空のキューを作成します。

### `queue_text_new`

```zt
public func queue_int_new() -> list<int>
public func queue_text_new() -> list<text>
```

空のキューを作成します。

### `queue_int_enqueue`

```zt
public func queue_int_enqueue(queue: list<int>, value: int) -> list<int>
public func queue_text_enqueue(queue: list<text>, value: text) -> list<text>
```

キューの最後に要素を追加します。更新されたキューを返します。

### `queue_text_enqueue`

```zt
public func queue_int_enqueue(queue: list<int>, value: int) -> list<int>
public func queue_text_enqueue(queue: list<text>, value: text) -> list<text>
```

キューの最後に要素を追加します。更新されたキューを返します。

### `queue_int_dequeue`

```zt
public func queue_int_dequeue(queue: list<int>) -> collections.QueueNumberDequeueResult
public func queue_text_dequeue(queue: list<text>) -> collections.QueueTextDequeueResult
```

先頭から要素を削除し、キューが更新され項目が削除されたパケットを返します。

### `queue_text_dequeue`

```zt
public func queue_int_dequeue(queue: list<int>) -> collections.QueueNumberDequeueResult
public func queue_text_dequeue(queue: list<text>) -> collections.QueueTextDequeueResult
```

要素を先頭から削除し、削除された項目とともに更新されたキューを返します。

### `queue_int_peek`

```zt
public func queue_int_peek(queue: list<int>) -> optional<int>
public func queue_text_peek(queue: list<text>) -> optional<text>
```

前の要素を削除せずに返します。

## スタック

### `queue_text_peek`

```zt
public func queue_int_peek(queue: list<int>) -> optional<int>
public func queue_text_peek(queue: list<text>) -> optional<text>
```

前の要素を削除せずに返します。

## スタック

### `stack_int_new`

```zt
public func stack_int_new() -> list<int>
public func stack_text_new() -> list<text>
```

空のスタックを作成します。

### `stack_text_new`

```zt
public func stack_int_new() -> list<int>
public func stack_text_new() -> list<text>
```

空のスタックを作成します。

### `stack_int_push`

```zt
public func stack_int_push(stack: list<int>, value: int) -> list<int>
public func stack_text_push(stack: list<text>, value: text) -> list<text>
```

要素を上に積み重ねます。

### `stack_text_push`

```zt
public func stack_int_push(stack: list<int>, value: int) -> list<int>
public func stack_text_push(stack: list<text>, value: text) -> list<text>
```

要素を上に積み重ねます。

### `stack_int_pop`

```zt
public func stack_int_pop(stack: list<int>) -> collections.StackNumberPopResult
public func stack_text_pop(stack: list<text>) -> collections.StackTextPopResult
```

最上位の要素を削除し、更新されたスタックと削除された項目を含むパッケージを返します。

### `stack_text_pop`

```zt
public func stack_int_pop(stack: list<int>) -> collections.StackNumberPopResult
public func stack_text_pop(stack: list<text>) -> collections.StackTextPopResult
```

最上位の要素を削除し、削除された項目とともに更新されたスタックを返します。

### `stack_int_peek`

```zt
public func stack_int_peek(stack: list<int>) -> optional<int>
public func stack_text_peek(stack: list<text>) -> optional<text>
```

削除せずに上部をクエリします。

## Grid2D (二次元行列)

### `stack_text_peek`

```zt
public func stack_int_peek(stack: list<int>) -> optional<int>
public func stack_text_peek(stack: list<text>) -> optional<text>
```

削除せずに上部をクエリします。

## Grid2D (二次元行列)

### `grid2d_int_new`

```zt
public func grid2d_int_new(rows: int, cols: int) -> grid2d<int>
public func grid2d_text_new(rows: int, cols: int) -> grid2d<text>
```

2Dグリッドを作成します`rows`線と`cols`列。値はゼロ/空から始まります。

### `grid2d_text_new`

```zt
public func grid2d_int_new(rows: int, cols: int) -> grid2d<int>
public func grid2d_text_new(rows: int, cols: int) -> grid2d<text>
```

2Dグリッドを作成します`rows`線と`cols`列。値はゼロ/空から始まります。

### `grid2d_int_get`

```zt
public func grid2d_int_get(grid: grid2d<int>, row: int, col: int) -> int
public func grid2d_text_get(grid: grid2d<text>, row: int, col: int) -> text
```

位置の値を読み取ります`[row, col]`。

### `grid2d_text_get`

```zt
public func grid2d_int_get(grid: grid2d<int>, row: int, col: int) -> int
public func grid2d_text_get(grid: grid2d<text>, row: int, col: int) -> text
```

位置の値を読み取ります`[row, col]`。

### `grid2d_int_set`

```zt
public func grid2d_int_set(grid: grid2d<int>, row: int, col: int, value: int) -> grid2d<int>
public func grid2d_text_set(grid: grid2d<text>, row: int, col: int, value: text) -> grid2d<text>
```

位置に値を設定します`[row, col]`。更新されたグリッド (COW) を返します。

### `grid2d_text_set`

```zt
public func grid2d_int_set(grid: grid2d<int>, row: int, col: int, value: int) -> grid2d<int>
public func grid2d_text_set(grid: grid2d<text>, row: int, col: int, value: text) -> grid2d<text>
```

位置に値を設定します`[row, col]`。更新されたグリッド (COW) を返します。

### `grid2d_int_fill`

```zt
public func grid2d_int_fill(grid: grid2d<int>, value: int) -> grid2d<int>
public func grid2d_text_fill(grid: grid2d<text>, value: text) -> grid2d<text>
```

すべてのセルに同じ値を入力します。更新されたグリッドを返します。

### `grid2d_text_fill`

```zt
public func grid2d_int_fill(grid: grid2d<int>, value: int) -> grid2d<int>
public func grid2d_text_fill(grid: grid2d<text>, value: text) -> grid2d<text>
```

すべてのセルに同じ値を入力します。更新されたグリッドを返します。

### `grid2d_int_rows`

```zt
public func grid2d_int_rows(grid: grid2d<int>) -> int
public func grid2d_text_rows(grid: grid2d<text>) -> int
```

行数を返します。

### `grid2d_text_rows`

```zt
public func grid2d_int_rows(grid: grid2d<int>) -> int
public func grid2d_text_rows(grid: grid2d<text>) -> int
```

行数を返します。

### `grid2d_int_cols`

```zt
public func grid2d_int_cols(grid: grid2d<int>) -> int
public func grid2d_text_cols(grid: grid2d<text>) -> int
```

列の数を返します。

### `grid2d_text_cols`

```zt
public func grid2d_int_cols(grid: grid2d<int>) -> int
public func grid2d_text_cols(grid: grid2d<text>) -> int
```

列の数を返します。

### `grid2d_int_size`

```zt
public func grid2d_int_size(grid: grid2d<int>) -> int
public func grid2d_text_size(grid: grid2d<text>) -> int
```

セルの総数を返します (`rows * cols`）。

## Grid3D (三次元マトリックス)

### `grid2d_text_size`

```zt
public func grid2d_int_size(grid: grid2d<int>) -> int
public func grid2d_text_size(grid: grid2d<text>) -> int
```

セルの総数を返します (`rows * cols`）。

## Grid3D (三次元マトリックス)

### `grid3d_int_new`

```zt
public func grid3d_int_new(depth: int, rows: int, cols: int) -> grid3d<int>
public func grid3d_text_new(depth: int, rows: int, cols: int) -> grid3d<text>
```

寸法の 3D グリッドを作成します`depth × rows × cols`。

### `grid3d_text_new`

```zt
public func grid3d_int_new(depth: int, rows: int, cols: int) -> grid3d<int>
public func grid3d_text_new(depth: int, rows: int, cols: int) -> grid3d<text>
```

寸法の 3D グリッドを作成します`depth × rows × cols`。

### `grid3d_int_get`

```zt
public func grid3d_int_get(grid: grid3d<int>, layer: int, row: int, col: int) -> int
public func grid3d_text_get(grid: grid3d<text>, layer: int, row: int, col: int) -> text
```

位置の値を読み取ります`[layer, row, col]`。

### `grid3d_text_get`

```zt
public func grid3d_int_get(grid: grid3d<int>, layer: int, row: int, col: int) -> int
public func grid3d_text_get(grid: grid3d<text>, layer: int, row: int, col: int) -> text
```

位置の値を読み取ります`[layer, row, col]`。

### `grid3d_int_set`

```zt
public func grid3d_int_set(grid: grid3d<int>, layer: int, row: int, col: int, value: int) -> grid3d<int>
public func grid3d_text_set(grid: grid3d<text>, layer: int, row: int, col: int, value: text) -> grid3d<text>
```

位置に値を設定します`[layer, row, col]`。更新されたグリッド (COW) を返します。

### `grid3d_text_set`

```zt
public func grid3d_int_set(grid: grid3d<int>, layer: int, row: int, col: int, value: int) -> grid3d<int>
public func grid3d_text_set(grid: grid3d<text>, layer: int, row: int, col: int, value: text) -> grid3d<text>
```

位置に値を設定します`[layer, row, col]`。更新されたグリッド (COW) を返します。

### `grid3d_int_fill`

```zt
public func grid3d_int_fill(grid: grid3d<int>, value: int) -> grid3d<int>
public func grid3d_text_fill(grid: grid3d<text>, value: text) -> grid3d<text>
```

すべてのセルに同じ値を入力します。

### `grid3d_text_fill`

```zt
public func grid3d_int_fill(grid: grid3d<int>, value: int) -> grid3d<int>
public func grid3d_text_fill(grid: grid3d<text>, value: text) -> grid3d<text>
```

すべてのセルに同じ値を入力します。

### `grid3d_int_depth`

```zt
public func grid3d_int_depth(grid: grid3d<int>) -> int
public func grid3d_text_depth(grid: grid3d<text>) -> int
```

レイヤーの数 (深さ) を返します。

### `grid3d_text_depth`

```zt
public func grid3d_int_depth(grid: grid3d<int>) -> int
public func grid3d_text_depth(grid: grid3d<text>) -> int
```

レイヤーの数 (深さ) を返します。

### `grid3d_int_rows`

```zt
public func grid3d_int_rows(grid: grid3d<int>) -> int
public func grid3d_int_cols(grid: grid3d<int>) -> int
public func grid3d_int_size(grid: grid3d<int>) -> int
public func grid3d_text_rows(grid: grid3d<text>) -> int
public func grid3d_text_cols(grid: grid3d<text>) -> int
public func grid3d_text_size(grid: grid3d<text>) -> int
```

寸法とセルの合計 (`depth * rows * cols`）。

## PriorityQueue (優先キュー — 最小ヒープ)

### `cols`

```zt
public func grid3d_int_rows(grid: grid3d<int>) -> int
public func grid3d_int_cols(grid: grid3d<int>) -> int
public func grid3d_int_size(grid: grid3d<int>) -> int
public func grid3d_text_rows(grid: grid3d<text>) -> int
public func grid3d_text_cols(grid: grid3d<text>) -> int
public func grid3d_text_size(grid: grid3d<text>) -> int
```

寸法とセルの合計 (`depth * rows * cols`）。

## PriorityQueue (優先キュー — 最小ヒープ)
---

--- @target: サイズとgrid3d_text_rows```zt
public func grid3d_int_rows(grid: grid3d<int>) -> int
public func grid3d_int_cols(grid: grid3d<int>) -> int
public func grid3d_int_size(grid: grid3d<int>) -> int
public func grid3d_text_rows(grid: grid3d<text>) -> int
public func grid3d_text_cols(grid: grid3d<text>) -> int
public func grid3d_text_size(grid: grid3d<text>) -> int
```

寸法とセルの合計 (`depth * rows * cols`）。

## PriorityQueue (優先キュー — 最小ヒープ)

### `cols`

```zt
public func grid3d_int_rows(grid: grid3d<int>) -> int
public func grid3d_int_cols(grid: grid3d<int>) -> int
public func grid3d_int_size(grid: grid3d<int>) -> int
public func grid3d_text_rows(grid: grid3d<text>) -> int
public func grid3d_text_cols(grid: grid3d<text>) -> int
public func grid3d_text_size(grid: grid3d<text>) -> int
```

寸法とセルの合計 (`depth * rows * cols`）。

## PriorityQueue (優先キュー — 最小ヒープ)

### `size`

```zt
public func grid3d_int_rows(grid: grid3d<int>) -> int
public func grid3d_int_cols(grid: grid3d<int>) -> int
public func grid3d_int_size(grid: grid3d<int>) -> int
public func grid3d_text_rows(grid: grid3d<text>) -> int
public func grid3d_text_cols(grid: grid3d<text>) -> int
public func grid3d_text_size(grid: grid3d<text>) -> int
```

寸法とセルの合計 (`depth * rows * cols`）。

## PriorityQueue (優先キュー — 最小ヒープ)

### `pqueue_int_new`

```zt
public func pqueue_int_new() -> pqueue<int>
public func pqueue_text_new() -> pqueue<text>
```

空の最小ヒープを作成します。小さい要素ほど優先度が高くなります。

### `pqueue_text_new`

```zt
public func pqueue_int_new() -> pqueue<int>
public func pqueue_text_new() -> pqueue<text>
```

空の最小ヒープを作成します。小さい要素ほど優先度が高くなります。

### `pqueue_int_push`

```zt
public func pqueue_int_push(heap: pqueue<int>, value: int) -> pqueue<int>
public func pqueue_text_push(heap: pqueue<text>, value: text) -> pqueue<text>
```

ヒープの所有権を維持しながら要素を挿入します。

### `pqueue_text_push`

```zt
public func pqueue_int_push(heap: pqueue<int>, value: int) -> pqueue<int>
public func pqueue_text_push(heap: pqueue<text>, value: text) -> pqueue<text>
```

ヒープの所有権を維持しながら要素を挿入します。

### `pqueue_int_pop`

```zt
public func pqueue_int_pop(heap: pqueue<int>) -> optional<int>
public func pqueue_text_pop(heap: pqueue<text>) -> optional<text>
```

最小の要素を削除して返します。戻る`none`空の場合。

### `pqueue_text_pop`

```zt
public func pqueue_int_pop(heap: pqueue<int>) -> optional<int>
public func pqueue_text_pop(heap: pqueue<text>) -> optional<text>
```

最小の要素を削除して返します。戻る`none`空の場合。

### `pqueue_int_peek`

```zt
public func pqueue_int_peek(heap: pqueue<int>) -> optional<int>
public func pqueue_text_peek(heap: pqueue<text>) -> optional<text>
```

最小の要素を削除せずにクエリします。

### `pqueue_text_peek`

```zt
public func pqueue_int_peek(heap: pqueue<int>) -> optional<int>
public func pqueue_text_peek(heap: pqueue<text>) -> optional<text>
```

最小の要素を削除せずにクエリします。

### `pqueue_int_len`

```zt
public func pqueue_int_len(heap: pqueue<int>) -> int
public func pqueue_text_len(heap: pqueue<text>) -> int
```

ヒープ内の要素の数を返します。

### `pqueue_text_len`

```zt
public func pqueue_int_len(heap: pqueue<int>) -> int
public func pqueue_text_len(heap: pqueue<text>) -> int
```

ヒープ内の要素の数を返します。

### `pqueue_int_is_empty`

```zt
public func pqueue_int_is_empty(heap: pqueue<int>) -> bool
public func pqueue_text_is_empty(heap: pqueue<text>) -> bool
```

戻る`true`ヒープが空の場合。

## 循環バッファ

### `pqueue_text_is_empty`

```zt
public func pqueue_int_is_empty(heap: pqueue<int>) -> bool
public func pqueue_text_is_empty(heap: pqueue<text>) -> bool
```

戻る`true`ヒープが空の場合。

## 循環バッファ

### `circbuf_int_new`

```zt
public func circbuf_int_new(capacity: int) -> circbuf<int>
public func circbuf_text_new(capacity: int) -> circbuf<text>
```

固定容量のリングバッファを作成します。充填時は新品`push`古いものを上書きします。

### `circbuf_text_new`

```zt
public func circbuf_int_new(capacity: int) -> circbuf<int>
public func circbuf_text_new(capacity: int) -> circbuf<text>
```

固定容量のリングバッファを作成します。充填時は新品`push`古いものを上書きします。

### `circbuf_int_push`

```zt
public func circbuf_int_push(buf: circbuf<int>, value: int) -> circbuf<int>
public func circbuf_text_push(buf: circbuf<text>, value: text) -> circbuf<text>
```

要素を挿入します。いっぱいの場合は、最も古いものを上書きします。

### `circbuf_text_push`

```zt
public func circbuf_int_push(buf: circbuf<int>, value: int) -> circbuf<int>
public func circbuf_text_push(buf: circbuf<text>, value: text) -> circbuf<text>
```

要素を挿入します。いっぱいの場合は、最も古いものを上書きします。

### `circbuf_int_pop`

```zt
public func circbuf_int_pop(buf: circbuf<int>) -> optional<int>
public func circbuf_text_pop(buf: circbuf<text>) -> optional<text>
```

最も古い要素を削除して返します。戻る`none`空の場合。

### `circbuf_text_pop`

```zt
public func circbuf_int_pop(buf: circbuf<int>) -> optional<int>
public func circbuf_text_pop(buf: circbuf<text>) -> optional<text>
```

最も古い要素を削除して返します。戻る`none`空の場合。

### `circbuf_int_peek`

```zt
public func circbuf_int_peek(buf: circbuf<int>) -> optional<int>
public func circbuf_text_peek(buf: circbuf<text>) -> optional<text>
```

削除せずに最も古いものをクエリします。

### `circbuf_text_peek`

```zt
public func circbuf_int_peek(buf: circbuf<int>) -> optional<int>
public func circbuf_text_peek(buf: circbuf<text>) -> optional<text>
```

削除せずに最も古いものをクエリします。

### `circbuf_int_len`

```zt
public func circbuf_int_len(buf: circbuf<int>) -> int
public func circbuf_text_len(buf: circbuf<text>) -> int
```

現在保存されている要素の数。

### `circbuf_text_len`

```zt
public func circbuf_int_len(buf: circbuf<int>) -> int
public func circbuf_text_len(buf: circbuf<text>) -> int
```

現在保存されている要素の数。

### `circbuf_int_capacity`

```zt
public func circbuf_int_capacity(buf: circbuf<int>) -> int
public func circbuf_text_capacity(buf: circbuf<text>) -> int
```

最大バッファ容量。

### `circbuf_text_capacity`

```zt
public func circbuf_int_capacity(buf: circbuf<int>) -> int
public func circbuf_text_capacity(buf: circbuf<text>) -> int
```

最大バッファ容量。

### `circbuf_int_is_full`

```zt
public func circbuf_int_is_full(buf: circbuf<int>) -> bool
public func circbuf_text_is_full(buf: circbuf<text>) -> bool
```

戻る`true`もし`len == capacity`。

### `circbuf_text_is_full`

```zt
public func circbuf_int_is_full(buf: circbuf<int>) -> bool
public func circbuf_text_is_full(buf: circbuf<text>) -> bool
```

戻る`true`もし`len == capacity`。

### `circbuf_int_is_empty`

```zt
public func circbuf_int_is_empty(buf: circbuf<int>) -> bool
public func circbuf_text_is_empty(buf: circbuf<text>) -> bool
```

戻る`true`空の場合。

## BTreeMap (キーでソートされたマップ)

### `circbuf_text_is_empty`

```zt
public func circbuf_int_is_empty(buf: circbuf<int>) -> bool
public func circbuf_text_is_empty(buf: circbuf<text>) -> bool
```

戻る`true`空の場合。

## BTreeMap (キーでソートされたマップ)

### `btreemap_text_new`

```zt
public func btreemap_text_new() -> btreemap<text, text>
```

空の順序付きマップを作成します。ソートされた配列 + 二分探索で実装されます。

### `btreemap_text_set`

```zt
public func btreemap_text_set(self_map: btreemap<text, text>, key: text, value: text) -> btreemap<text, text>
```

キーと値のペアを挿入または更新します。更新されたマップ (COW) を返します。

### `btreemap_text_get`

```zt
public func btreemap_text_get(self_map: btreemap<text, text>, key: text) -> text
```

積極的な検索: 値を返すか、キーが存在しない場合はパニックを引き起こします。

### `btreemap_text_get_optional`

```zt
public func btreemap_text_get_optional(self_map: btreemap<text, text>, key: text) -> optional<text>
```

セーフサーチ: リターン`none`キーが存在しない場合。

### `btreemap_text_contains`

```zt
public func btreemap_text_contains(self_map: btreemap<text, text>, key: text) -> bool
```

キーがマップ内に存在するかどうかを確認します。

### `btreemap_text_remove`

```zt
public func btreemap_text_remove(self_map: btreemap<text, text>, key: text) -> btreemap<text, text>
```

キーを削除します。更新されたマップ (COW) を返します。

### `btreemap_text_len`

```zt
public func btreemap_text_len(self_map: btreemap<text, text>) -> int
```

キーと値のペアの数を返します。

### `btreemap_text_is_empty`

```zt
public func btreemap_text_is_empty(self_map: btreemap<text, text>) -> bool
```

戻る`true`マップが空の場合。

## BTreeSet (順序付きセット)

### `btreeset_text_new`

```zt
public func btreeset_text_new() -> btreeset<text>
```

空の順序付きセットを作成します。重複は許可されません。

### `btreeset_text_insert`

```zt
public func btreeset_text_insert(set: btreeset<text>, value: text) -> btreeset<text>
```

要素を挿入します。すでに存在する場合、セットは変更されません。

### `btreeset_text_contains`

```zt
public func btreeset_text_contains(set: btreeset<text>, value: text) -> bool
```

要素が存在するかどうかを確認します。

### `btreeset_text_remove`

```zt
public func btreeset_text_remove(set: btreeset<text>, value: text) -> btreeset<text>
```

要素を削除します。更新されたセットを返します。

### `btreeset_text_len`

```zt
public func btreeset_text_len(set: btreeset<text>) -> int
```

要素の数を返します。

### `btreeset_text_is_empty`

```zt
public func btreeset_text_is_empty(set: btreeset<text>) -> bool
```

戻る`true`セットが空の場合。

