# Stdlib Practical Recipes

> Audience: user
> Status: current
> Surface: public
> Source of truth: no

## Goal

Keep the cookbook growing with the standard library.

Each recipe here is small.

Each one points to a runnable example when possible.

## Recipe: Read, Clean, and Write a Text File

Use when a tool needs to process a text file.

Recommended example:

```text
examples/file-processor/
```

Core shape:

```zt
import std.fs as fs
import std.text as text

func main() -> result<void, fs.Error>
    const raw: text = fs.read_text("input.txt")?
    const clean: text = text.trim(raw)
    fs.write_text("summary.txt", clean)?
    return success()
end
```

Common mistake:

Do not ignore the `result`.

Use `?`, `match`, or return the error.

## Recipe: Parse Required JSON Fields

Use when JSON input must contain a small set of fields.

Recommended example:

```text
examples/data-parser/
```

Core shape:

```zt
import std.json as json

func required_field(data: map<text, text>, key: text) -> result<text, core.Error>
    match data.get(key)
        case some(value):
            return success(value)
        case none:
            return error("missing field")
    end
end
```

Common mistake:

Do not use direct map indexing for data that may be missing.

Use `.get` and handle `none`.

## Recipe: Count Small In-Memory Items

Use when a tool needs a small summary.

Recommended example:

```text
examples/todo-app/
```

Core shape:

```zt
func item_score(item: Todo) -> int
    if item.done
        return 0
    end

    return 1
end

func open_count(first: Todo, second: Todo, third: Todo) -> int
    var count: int = 0
    count = count + item_score(first)
    count = count + item_score(second)
    count = count + item_score(third)
    return count
end
```

Common mistake:

Do not hide too much logic in one expression.

Keep the runnable example inside the backend surface that exists today.

`list<Todo>` is a natural future shape, but it is not native-backend ready yet.

## Add a New Recipe

Use this checklist:

1. Start with the problem.
2. Show the shortest recommended code.
3. Mention one common mistake.
4. Link a runnable example or behavior test.
5. Keep the text short.
