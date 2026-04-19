# Decision 064 - Stdlib Fs Module

- Status: accepted
- Date: 2026-04-17
- Type: stdlib / filesystem
- Scope: synchronous filesystem MVP, text paths, text file IO, directory operations, metadata convenience and module boundaries

## Summary

`std.fs` is the synchronous, side-effecting filesystem module.

Paths are represented as `text` in the MVP.

Path manipulation stays in `std.fs.path`, while `std.fs` focuses on filesystem effects such as reading, writing, creating, removing and querying entries.

Process-state concerns such as the current working directory belong in `std.os`, not in `std.fs`.

## Decision

Canonical import:

```zt
import std.fs as fs
```

Accepted principal types:

- `fs.Error`
- `fs.Metadata`

Accepted first-wave API direction:

- `fs.read_text(path: text) -> result<text, fs.Error>`
- `fs.write_text(path: text, text: text) -> result<void, fs.Error>`
- `fs.append_text(path: text, text: text) -> result<void, fs.Error>`
- `fs.exists(path: text) -> result<bool, fs.Error>`
- `fs.is_file(path: text) -> result<bool, fs.Error>`
- `fs.is_dir(path: text) -> result<bool, fs.Error>`
- `fs.create_dir(path: text) -> result<void, fs.Error>`
- `fs.create_dir_all(path: text) -> result<void, fs.Error>`
- `fs.list(path: text) -> result<list<text>, fs.Error>`
- `fs.remove_file(path: text) -> result<void, fs.Error>`
- `fs.remove_dir(path: text) -> result<void, fs.Error>`
- `fs.remove_dir_all(path: text) -> result<void, fs.Error>`
- `fs.copy_file(from: text, to: text) -> result<void, fs.Error>`
- `fs.move(from: text, to: text) -> result<void, fs.Error>`
- `fs.metadata(path: text) -> result<fs.Metadata, fs.Error>`
- `fs.size(path: text) -> result<int64, fs.Error>`
- `fs.modified_at(path: text) -> result<time.Instant, fs.Error>`
- `fs.created_at(path: text) -> result<optional<time.Instant>, fs.Error>`

Examples:

```zt
import std.fs as fs
import std.fs.path as path

const config_path: text = path.join(["config", "app.json"])
const content: text = fs.read_text(config_path)?

fs.write_text("log.txt", "started\n")?
fs.append_text("log.txt", "running\n")?

const names: list<text> = fs.list("config")?

const exists: bool = fs.exists(config_path)?
const changed: time.Instant = fs.modified_at(config_path)?
const created: optional<time.Instant> = fs.created_at(config_path)?
```

Normative rules for this cut:

- paths are represented as `text` in the MVP
- path joining, normalization and decomposition belong in `std.fs.path`, not in `std.fs`
- `std.fs` is synchronous in this cut
- `read_text`, `write_text` and `append_text` are the accepted text-file entrypoints
- text file operations use UTF-8 in the MVP and do not expose an encoding parameter
- invalid UTF-8 causes text-reading failure rather than silent replacement
- newline normalization is not automatic in this cut
- `exists`, `is_file` and `is_dir` return `result<bool, fs.Error>` rather than plain `bool`
- `list(path)` returns entry names only, not richer entry objects
- current-directory queries and mutation are not part of `std.fs`
- removal APIs are explicit by entry kind and do not collapse file and directory removal into a single `remove(...)` name
- `created_at(...)` is optional because creation time is not uniformly available across filesystems and platforms

## Naming Direction

Accepted canonical names in this cut:

- `remove_file`
- `remove_dir`
- `remove_dir_all`
- `copy_file`
- `move`

Rejected as canonical MVP names:

- `remove`
- `remove_all`
- `copy`
- `read`
- `write`
- `read_dir`

Rationale:

- explicit removal names are clearer than a generic `remove(...)`
- `copy_file(...)` avoids ambiguity with future directory-copy semantics
- generic names such as `read(...)` and `write(...)` are deferred until the byte-oriented API exists
- `read_dir(...)` is deferred because the MVP keeps directory listing simple and name-based

## Metadata Direction

`fs.metadata(path)` is accepted as the richer metadata entrypoint.

The exact public field surface of `fs.Metadata` remains implementation work, but the accepted convenience layer already commits the module to supporting these metadata concepts:

- file size in bytes
- modification time
- optional creation time
- file or directory kind checks

`fs.size(...)`, `fs.modified_at(...)` and `fs.created_at(...)` exist as convenience helpers for common cases.

## Deferred Technical Work

The following are intentionally deferred beyond this cut:

- `read_bytes(...)`
- `write_bytes(...)`
- rich directory entry APIs such as `read_dir(...)`
- file handles or streaming file IO
- async filesystem APIs
- symlink-specific APIs
- permission APIs
- glob or recursive walk APIs

## Rationale

Using `text` for paths keeps the MVP direct and readable, especially when paired with a separate `std.fs.path` module for pure path operations.

Returning `result<bool, fs.Error>` from existence and kind checks preserves the distinction between "false" and "could not be checked".

Keeping the first directory listing API as `list(...) -> list<text>` avoids introducing entry-object complexity too early.

Treating `created_at` as optional makes the API honest across platforms instead of pretending the value is always available.

Keeping current-directory state out of `std.fs` preserves a cleaner boundary between filesystem effects and process-state operations.

## Non-Canonical Forms

Using `std.fs` to join or normalize paths.

Collapsing all removal into one generic API:

```zt
fs.remove(path)
```

Using plain `bool` for existence checks that can fail because of permissions or IO errors.

Making bytes the default file surface before the byte model is settled.

Using `fs.current_dir()` as the canonical current-directory API after the responsibility moved to `std.os`.

## Out of Scope

This decision does not yet define:

- the byte-array type used by future byte APIs
- the exact `fs.Metadata` field layout
- directory entry object types
- path canonicalization semantics
- temporary files or directories
- filesystem watchers
