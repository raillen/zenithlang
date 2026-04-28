#標準ライブラリ ガイド

> Zenith stdlib パブリック ガイド。
> 対象者: ユーザー、パッケージ作成者
> 表面: 公開
> ステータス: 現在

## 現在のステータス

stdlib はまだアルファ段階です。

公開サンプルを作成するときは、文書化され、テストで検証された API のみを使用してください。

## 共通モジュール

- `std.io`: 基本的な入力と出力。
-`std.text`: アルファカットテキストセーフヘルパー。
-`std.bytes`: バイナリデータのベース。
-`std.fs`: 同期ファイルシステム。
-`std.fs.path`: パスの操作。
-`std.json`:JSON。
-`std.math`: 基本的な数学。
-`std.regex`: simple regex helpers for validation and search.
-`std.random`: ランダムなアルファ。共有状態に注意。
-`std.concurrent`: 競争境界の明示的なコピー。
-`std.test`: 失敗、スキップ、およびテスト用の単純な比較。
-`std.list`: 小さなヘルパー`list<T>`。
-`std.map`: 小さなヘルパー`map<K,V>`。

## 小規模なテスト`std.test`

```zt
import std.test as test

attr test
public func score_starts_at_zero() -> void
    const score: int = 0
    test.equal_int(score, 0)
    test.is_true(score == 0)
end
```

比較が失敗した場合は、期待値と受信値を示すヘルパーを優先します。

- `test.equal_int(actual, expected)`
- `test.equal_text(actual, expected)`
- `test.not_equal_int(actual, expected)`
- `test.not_equal_text(actual, expected)`

## 使用ルール

小さな例を好みます。

API がコンパイラーによってまだ意味的にサポートされていない場合は、それを公開ガイドとして投稿しないでください。

## 参考文献

- テキストとファイル:`docs/public/stdlib/text-and-files.md`。
- 検証、数学、ランダム:`docs/public/stdlib/validation-math-random.md`。
- Regex: `docs/public/jp/stdlib/regex.md`.
- 標準ライブラリ参照:`docs/reference/stdlib/README.md`。
- 標準ライブラリのKB:`docs/reference/zenith-kb/stdlib.md`。
- 標準モデル:`language/spec/stdlib-model.md`。
- 標準ライブラリからの ZDoc:`stdlib/zdoc/`。
