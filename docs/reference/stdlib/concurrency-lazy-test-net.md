# Concurrency, Lazy, Test and Net Reference

> Surface: reference
> Status: current

## `std.concurrent`

Current delivered surface is explicit copy helpers.

| API | Description |
| --- | --- |
| `concurrent.copy_int(value: int) -> int` | Copies an integer for an explicit boundary. |
| `concurrent.copy_bool(value: bool) -> bool` | Copies a boolean for an explicit boundary. |
| `concurrent.copy_float(value: float) -> float` | Copies a float for an explicit boundary. |
| `concurrent.copy_text(value: text) -> text` | Copies text for an explicit boundary. |
| `concurrent.copy_bytes(value: bytes) -> bytes` | Copies bytes for an explicit boundary. |
| `concurrent.copy_list_int(value: list<int>) -> list<int>` | Copies a list of integers. |
| `concurrent.copy_list_text(value: list<text>) -> list<text>` | Copies a list of text values. |
| `concurrent.copy_map_text_text(value: map<text,text>) -> map<text,text>` | Copies a text-to-text map. |

Notes:

- Full `task`, `channel` and `Shared<T>` surface is deferred.
- Engine/runtime hosts may use internal scheduling without exposing language-level async.
- These helpers make boundary-copy intent explicit while the language remains small.

## `std.lazy`

| API | Description |
| --- | --- |
| `lazy.once_int(thunk: func() -> int) -> lazy<int>` | Creates a one-shot lazy integer from a thunk. |
| `lazy.force_int(value: lazy<int>) -> int` | Evaluates and consumes the lazy integer. |
| `lazy.is_consumed_int(value: lazy<int>) -> bool` | Checks whether the lazy integer was already forced. |

The alpha surface is specialized for `int`.

`lazy<int>` is explicit: creating it does not run the thunk. `force_int` runs it.

## `std.test`

| API | Description |
| --- | --- |
| `test.fail(message: text = "test failed") -> void` | Fails the current test with a message. |
| `test.skip(reason: text = "") -> void` | Skips the current test with a reason. |
| `test.is_true(value: bool) -> void` | Fails when the value is false. |
| `test.is_false(value: bool) -> void` | Fails when the value is true. |
| `test.equal_int(actual: int, expected: int) -> void` | Compares ints and reports expected/received values. |
| `test.equal_text(actual: text, expected: text) -> void` | Compares text and reports expected/received values. |
| `test.not_equal_int(actual: int, expected: int) -> void` | Fails when both ints are equal. |
| `test.not_equal_text(actual: text, expected: text) -> void` | Fails when both text values are equal. |
| `test.zt_test_fail(message: text) -> void` | Low-level exported fail helper used by the runtime/test bridge. |
| `test.zt_test_skip(reason: text) -> void` | Low-level exported skip helper used by the runtime/test bridge. |

Prefer `test.fail` and `test.skip` in user code.
Prefer `equal_*` and `not_equal_*` when expected/received values make the failure easier to fix.

## `std.net`

Types:

| Type | Description |
| --- | --- |
| `net.Error` | Network error category. |
| `net.Connection` | Network connection handle. |

Functions:

| API | Description |
| --- | --- |
| `net.connect(host: text, port: int, timeout_ms: int = 0) -> result<net.Connection, core.Error>` | Opens a connection to a host and port. |
| `net.read_some(connection: net.Connection, max_bytes: int) -> result<bytes, core.Error>` | Reads up to `max_bytes` from a connection. |
| `net.write_all(connection: net.Connection, data: bytes) -> result<void, core.Error>` | Writes all bytes to a connection. |
| `net.close(connection: net.Connection) -> result<void, core.Error>` | Closes a connection. |
| `net.is_closed(connection: net.Connection) -> bool` | Checks whether a connection is closed. |
| `net.kind(err: core.Error) -> net.Error` | Maps a core error to a network error category. |

Networking remains alpha. Validate behavior with project tests before relying on it in packages.
