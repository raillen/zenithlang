# ゼニス料理本

> 対象者: ユーザー
> ステータス: ドラフト
> 表面: 公開
> 真実の情報源: いいえ

＃＃ 客観的

実際的な質問に答えます。

「Zenith でこれを行うにはどうすればよいですか?」

クックブックは、言語の概念を直接的な解決策に変換するために存在します。

## 範囲

一般的な問題に対するレシピが含まれています。

- なしの欠席`null`;
- 例外なくエラーが発生します。
- 安全な結合;
- 抽象メソッド;
- 仮想ディスパッチ;
- 入力検証;
- テキストへの変換;
- 安全な検索。
- 意図的な失敗。
- 読みやすいテスト。

以下は含まれません：

- 標準仕様。
- ロードマップ;
- 歴史的なデザインの議論。
- 将来の機能が現在のものであるかのように。

## 依存関係

- 上流:
  -`docs/internal/planning/documentation-roadmap-v1.md`
  - `docs/internal/standards/user-doc-template.md`- 下流:
  -`docs/public/README.md`
  - `docs/reference/language/`- 関連コード/テスト:
  -`tests/behavior/MATRIX.md`

## 主な内容

標準レシピ形式:

1. 問題。
2. 短い答え。
3. 推奨コード。
4. 説明。
5. よくある間違い。
6. 使用しない場合。

推奨されるスターターレシピ:

|レシピ | | の場合に使用します。
| --- | --- |
|`absence-without-null.md`|値が存在しない可能性があります |
|`errors-without-exceptions.md`|操作は失敗する可能性があるため、呼び出し側が決定する必要があります。
|`safe-union-with-enum.md`|値にはさまざまな形式を指定できます。
|`abstract-methods-with-trait.md`|複数のタイプが同じ契約を履行する必要がある |
|`virtual-dispatch-with-dyn.md`|異なる型の値は同じコントラクトで呼び出す必要があります。
|`partial-class-with-apply.md`|データを断片化せずにメソッドを分離したい。
|`callable-delegate.md`|関数を値として渡したい場合 |
|`zenith-equivalents-from-other-languages.md`|別の言語の概念を知っていて、同等の Zenith | が必要な場合
|`intentional-failure-builtins.md`|明らかに失敗する必要があります`check`、`todo`または`unreachable` |

## 検証

レシピを公開する前に:

```powershell
python tools/check_docs_paths.py
git diff --check
```

例はコンパイルするか、例示としてマークする必要があります。

## 更新履歴

- 2026-04-25: 初期のクックブックインデックス。
- 2026-04-26: 他の言語の概念に対応する Zenith のレシピを追加しました。
- 2026-04-26: 意図的なクラッシュのレシピを追加しました。`check`、`todo`そして`unreachable`。