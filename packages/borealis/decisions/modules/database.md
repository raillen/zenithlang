# Borealis Module Decision - Database

- Status: proposed
- Date: 2026-04-22
- Type: module / database backend
- Scope: `borealis.game.database`

## Summary

Database is reserved for actual database backends such as SQLite in the future.

## Implementation

- keep this module separate from save and storage;
- use it only when query/update semantics matter;
- start with a small API and a clear backend boundary.

## Proposed API

- `db_open(config) -> Database`: opens a database.
- `db_close(db)`: closes a database.
- `db_exec(db, sql)`: executes SQL without returning rows.
- `db_query(db, sql) -> Rows`: executes SQL and returns rows.
- `db_prepare(db, sql) -> Statement`: prepares a statement.
- `db_bind(statement, values)`: binds values to a statement.
- `db_step(statement) -> bool`: advances a statement cursor.
- `db_transaction(db, fn)`: runs a transaction block.

## Notes

- SQLite is the most likely first backend.
- this module is future-facing and should not block save/storage.
- the implementation will likely sit on top of FFI or a native backend layer.
