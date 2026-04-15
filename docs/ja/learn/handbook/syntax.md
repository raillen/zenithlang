# 構文ガイド：コードのリズム

このガイドでは、Zenith言語 v0.3.1の構造を詳しく説明します。各機能は、認知摩擦を最小限に抑えるための「黄金律」とともに提示されています。

## 1. 基本と宣言

### 変数と定数
Zenithは、変更可能なものと固定されているものを明示的に区別します。

*   **ルール**: 変更可能なデータには `var` を、コンパイル時に固定される値には `const` を使用します。
*   **対比**: すべてがデフォルトで変更可能なLuaとは異なり、Zenithは状態を保護します。

```zt
var level: int = 1
const VERSION: text = "0.3.1"
```

### 可視性キーワード
*   `pub`: シンボルを現在のファイルの外から見えるようにします。
*   `global`: VMスコープで宣言します（細心の注意を払って使用してください）。

---

## 2. データコレクション

Zenithは、情報を整理するための3つの基本構造を提供しています。

| コレクション | 型 | 推奨される用途 |
| :--- | :--- | :--- |
| `list<T>` | 動的 | 順序付けられたアイテムのシーケンス。 |
| `map<K, V>` | ハッシュ | キーと値のディクショナリ。 |
| `grid<T>` | 静的 2D | ゲームやグラフィックスのための高性能な行列。 |

### 使用例
```zt
var items: list<text> = ["剣", "盾"]
var config: map<text, int> = { "volume": 80 }
var map: grid<int> = collections.new_grid(10, 10, 0)
```

---

## 3. 制御フロー

### 条件分岐 (`if`)
Zenithは、過剰な括弧によるノイズを避けるためにテキストキーワードを使用します。

```zt
if energy > 80
    print("システム正常")
elif energy > 20
    print("エネルギー低下")
else
    print("致命的なエラー")
end
```

### 決定の神託 (`match`)
`match` は、長い `if/else` チェーンを視覚的に明快なものに置き換えます。

*   **黄金律**: 1行の応答には `=>` を使用します。複雑なコードブロックには `:` を使用します。

```zt
match status
    case "active" => print("オンライン")
    case "error":
        log.error("エラーを検出")
        try_recover()
    case _ => print("不明")
end
```

---

## 4. エラー処理と安全性

Zenithは、明示的な構造を通じて失敗への恐怖を排除します。

### `check` コマンド
条件を検証します。失敗した場合、 `else` ブロックを実行するか、フローを中断します。

```zt
-- ステートメントとして
check age >= 18 else
    return Failure("年齢が不足しています")
end

-- 式として（伝播）
var file = fs.read_text_file("save.dat")?
```

### `attempt / rescue` ブロック
外部関数によってスローされたエラーや重大な失敗をキャッチします。

```zt
attempt
    process_data()
rescue error
    print("エラーをキャッチ: " + error)
end
```

---

## 5. 関数とモデリング

### 関数の定義
矢印 `->` は処理の結果を指します。

```zt
func calculate_damage(strength: int, weapon: int) -> int
    return strength + weapon
end
```

### 構造体 (`struct`) と契約 (`trait`)
Zenithは継承よりも合成を好みます。

```zt
struct Player
    pub name: text
    pub hp: int where it >= 0
end

trait Attacker
    func attack(target: Player)
end

apply Attacker to Player
    func attack(target: Player)
        target.hp -= 10
    end
end
```

---
*Zenith仕様 v0.3.1 - 構文ハンドブック*
