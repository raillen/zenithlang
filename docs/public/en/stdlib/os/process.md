# Module`std.os.process`

Module for executing child processes.
Explicit execution via`program` + `args`, without shell.
Non-zero exit code is valid result, not error.

## Constants and Functions

### `ExitStatus`

```zt
public struct ExitStatus
    code: int
end
```

Exit status of a child process.
Code 0 indicates conventional success; other codes are valid results.

### `CapturedRun`

```zt
public struct CapturedRun
    status: process.ExitStatus
    stdout_text: text
    stderr_text: text
end
```

Execution result with textual capture from stdout and stderr.

### `Error`

```zt
public enum Error
    NotFound
    PermissionDenied
    IOFailure
    DecodeFailed
    Unknown
end
```

Transport level error (spawn, wait, capture).
Non-zero exit code is NOT`process.Error`.

### `run`

```zt
public func run(program: text, args: list<text> = [], cwd: optional<text> = none) -> result<process.ExitStatus, process.Error>
```

Runs a program and waits for it to complete.
Does not use shell. Arguments are passed explicitly.

@param program — Path or name of the program.
@param args — Program arguments (default:`[]`).
@param cwd — Optional working directory.
@return Exit status, or transport error.

### `run_capture`

```zt
public func run_capture(program: text, args: list<text> = [], cwd: optional<text> = none) -> result<process.CapturedRun, process.Error>
```

Runs a program and captures stdout and stderr as text (UTF-8).

@param program — Path or name of the program.
@param args — Program arguments (default:`[]`).
@param cwd — Optional working directory.
@return Result captured, or transport error.

