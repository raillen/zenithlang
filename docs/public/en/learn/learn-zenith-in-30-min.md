# Learn Zenith in 30 Minutes

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Goal

Get a first working mental model of Zenith.

This guide is short on purpose.

Read it once, then run the examples.

## 0. Build the Tool

From the repository root:

```powershell
python build.py
```

You should get:

```text
SUCCESS
```

## 1. Smallest Useful Program

```zt
namespace app.main

func main()
    print("Hello, Zenith!")
end
```

Run the existing example:

```powershell
.\zt.exe run examples\hello-world\zenith.ztproj --ci
```

Key rules:

- every file starts with `namespace`;
- blocks close with `end`;
- `func main()` is the app entry point;
- `print(value)` writes one line.

## 2. Values and Functions

Use `const` for names that should not change.

Use `var` for names that will change.

```zt
func double(value: int) -> int
    return value * 2
end

func main()
    const base: int = 21
    var answer: int = double(base)
    answer = answer + 1
    print(answer)
end
```

Local declarations use explicit types:

```zt
const score: int = 10
```

## 3. Branches and Match

Use `if` for simple branches.

Use `match` when a value has named cases.

```zt
enum Mode
    Fast
    Slow
end

func speed(mode: Mode) -> int
    match mode
        case Mode.Fast:
            return 2
        case Mode.Slow:
            return 1
    end
end
```

Dot shorthand works when the expected enum type is known:

```zt
const mode: Mode = .Fast
```

## 4. Absence

Use `optional<T>` when a value may be absent.

```zt
func find_name(id: int) -> optional<text>
    if id == 1
        return "Ada"
    end

    return none
end
```

Extract it with `match`:

```zt
match find_name(1)
    case some(value):
        print(value)
    case none:
        print("missing")
end
```

## 5. Recoverable Errors

Use `result<T, E>` when a caller must handle failure.

```zt
func divide(left: int, right: int) -> result<int, core.Error>
    if right == 0
        return error("division by zero")
    end

    return success(left / right)
end
```

Use `?` to return early on error:

```zt
const value: int = divide(10, 2)?
```

## 6. Structs and Lists

Use `struct` for named fields.

```zt
struct Todo
    title: text
    done: bool
end
```

Use a list when you have many values of the same type:

```zt
const first: Todo = {title: "Write docs", done: true}
const second: Todo = {title: "Run examples", done: false}
const items: list<Todo> = [first, second]
```

## 7. Files

Use `std.fs` for simple file work.

```zt
import std.fs as fs

func main() -> result<void, fs.Error>
    fs.write_text(".ztc-tmp/demo.txt", "hello")?
    const content: text = fs.read_text(".ztc-tmp/demo.txt")?
    print(content)
    return success()
end
```

## 8. JSON Data

Use `std.json` for the current object-shaped JSON subset.

```zt
import std.json as json

const source: text = "{\"name\":\"Zenith\"}"
const data: map<text, text> = json.parse(source)?
```

Use `.get` for safe lookup:

```zt
match data.get("name")
    case some(value):
        print(value)
    case none:
        print("missing name")
end
```

## Practice Path

Run these examples in order:

```powershell
.\zt.exe run examples\hello-world\zenith.ztproj --ci
.\zt.exe run examples\cli-calculator\zenith.ztproj --ci
.\zt.exe run examples\file-processor\zenith.ztproj --ci
.\zt.exe run examples\todo-app\zenith.ztproj --ci
.\zt.exe run examples\data-parser\zenith.ztproj --ci
```

## Next

Read:

- `docs/public/en/learn/01-first-program.md`;
- `docs/public/en/learn/02-values-and-functions.md`;
- `docs/public/en/learn/03-errors-and-absence.md`;
- `docs/public/en/cookbook/README.md`.
