# モジュール`std.net`

ブロッキングソケットと明示的なタイムアウトを使用したクライアント TCP 通信用のモジュール。
契約のシンプルさとセキュリティを考慮した設計`where`パラメータで。

## 定数と関数

### `Error`

```zt
public enum Error
    ConnectionRefused
    HostUnreachable
    Timeout
    AddressInUse
    AlreadyConnected
    NotConnected
    NetworkDown
    Overflow
    PeerReset
    SystemLimit
    Unknown
end
```

考えられるネットワーク エラーの型付き列挙。

### `Connection`

```zt
public struct Connection
end
```

開いている TCP 接続を表す不透明なハンドル。

## 関数

### `connect`

```zt
public func connect(
    host: text,
    port: int where it >= 1 and it <= 65535,
    timeout_ms: int
) -> result<net.Connection, core.Error>
```

指定されたホストとポートへの TCP 接続を開きます。
ポートは契約によって検証されています`where`ハーフタイムに`[1, 65535]`。

@param host — ホスト名または IP アドレス。
@param port — TCP ポート (1 ～ 65535、where によって検証)。
@param timeout_ms — ミリ秒単位の最大タイムアウト。
@return 開いている接続のハンドル、またはエラー。

### `read_some`

```zt
public func read_some(
    connection: net.Connection,
    max: int where it > 0,
    timeout_ms: int = -1
) -> result<optional<bytes>, core.Error>
```

まで読む`max`接続のバイト数。戻る`none`サーバーが接続を閉じた場合 (EOF)。
パラメータ`max`によって検証されます`where`ポジティブになること。

@param 接続 — TCP 接続を開きます。
@param max — 読み取る最大バイト数 (> 0)。
@param timeout_ms — ミリ秒単位のタイムアウト (-1 = タイムアウトなし)。
@return 読み取られたバイト数、`none`EOF またはエラーです。

### `write_all`

```zt
public func write_all(
    connection: net.Connection,
    data: bytes,
    timeout_ms: int = -1
) -> result<void, core.Error>
```

すべてのバイトを送信します`data`接続を通じて。完全な配送を保証します。

@param 接続 — TCP 接続を開きます。
@param data — 送信するバイト数。
@param timeout_ms — ミリ秒単位のタイムアウト (-1 = タイムアウトなし)。
@戻る`void`成功かエラーか。

### `close`

```zt
public func close(connection: net.Connection) -> result<void, core.Error>
```

TCP接続を閉じます。

@param connection — 閉じる接続。
@戻る`void`成功かエラーか。

### `is_closed`

```zt
public func is_closed(connection: net.Connection) -> bool
```

接続が閉じられているかどうかを確認します。

@param connection — チェックする接続。
@戻る`true`接続が閉じられている場合。

### `kind`

```zt
public func kind(err: core.Error) -> net.Error
```

を変換する`core.Error`一般的な`net.Error`入力した。
フィールドに一致します`code`正しいバリアントを判断するためのエラー。

@param err — 一般的なコアエラー。
@return 型付きバリアント`net.Error`対応しています。

