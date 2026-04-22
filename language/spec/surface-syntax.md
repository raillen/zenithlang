# Zenith Surface Syntax Spec

- Status: canonical consolidated spec
- Date: 2026-04-22
- Scope: `.zt` source syntax and user-visible language semantics

## Philosophy

Zenith prioritizes reading comfort, predictability and explicitness.

The language is designed for low neural friction, visual stability and accessibility for TDAH/dyslexia-friendly workflows.

Rules:

- prefer clear words over dense symbolic syntax when that improves reading
- prefer one canonical form over many equivalent forms
- keep behavior explicit at the source level
- avoid hidden imports, hidden coercions and magical defaults
- keep generated C and runtime internals outside the user-facing language model

## Source File Shape

Every `.zt` file declares exactly one namespace.

Canonical shape:

```zt
-- optional short implementation comment
namespace app.users

import std.io as io
import app.users.types as types

public func main() -> int
    return 0
end
```

Rules:

- `namespace` is the first real declaration
- only blank lines and comments may appear before `namespace`
- imports appear after `namespace`
- imports import namespaces, not individual symbols
- imported symbols remain qualified through the namespace or alias
- `import path.to.namespace as alias` is allowed
- `from ... import ...`, `import *` and relative imports are not MVP syntax

## Comments

Line comments use `--`.

Block comments use `--- ... ---`.

Comments are local implementation notes.

Long public documentation belongs in ZDoc.

## Visibility And Scope

Top-level declarations are namespace-scoped.

Declarations are private by default.

`public` exposes a namespace-scope declaration outside its namespace.

Rules:

- `pub` is not canonical
- `global` is not used
- shadowing is rejected in the MVP
- files with the same effective namespace share private namespace symbols
- duplicate names in the same effective namespace/scope are errors
- `public var` is valid only at namespace top level
- read access to `public var` is allowed through qualified imports
- write access to `public var` is allowed only inside the owner namespace
- external write to imported `public var` is rejected with `mutability.invalid_update`
- `public` means visibility, not global scope or global lifetime

## Functions

Functions use `func` and close with `end`.

Return types use `->`.

Omitting `-> Type` means the function returns no value.

Canonical examples:

```zt
func render_name(user: User) -> text
    return user.name
end

func log(message: text)
    io.print(message)
end
```

Rules:

- ordinary no-return functions omit the return clause
- `-> void` is not canonical in user-authored source
- the compiler should diagnose explicit `-> void` and suggest removing it
- `return` is explicit when returning a value
- function parameters use `name: Type`
- local variable and parameter types are explicit in the MVP

## Parameters, Named Arguments And Defaults

Parameters are declared with explicit types.

Default values use `=`.

Named call arguments use `name: expression`.

```zt
func open_file(
    path: text,
    mode: text = "read"
) -> result<File, text>
    ...
end

const file: File = open_file("data.txt", mode: "write")?
```

Rules:

- required parameters come before defaulted parameters
- defaulted parameters form a trailing suffix
- public parameter names are API surface
- after a named argument appears, remaining provided arguments are named
- default expressions are evaluated at call time
- default expressions do not reference sibling parameters or `self` in the MVP

## Variables And Mutability

Bindings use `const` or `var`.

```zt
const name: text = "Julia"
var hp: int = 100
hp = hp - 10
```

Rules:

- `const` bindings cannot be reassigned
- `var` bindings may be reassigned
- assignment is a statement, not an expression
- `let` is not canonical
- local inference is not part of the MVP

Namespace-level mutable state:

```zt
namespace app.runtime.state

public var frame_count: int = 0

public func next_frame() -> int
    frame_count = frame_count + 1
    return frame_count
end
```

Cross-namespace read is allowed:

```zt
namespace app.main

import app.runtime.state as state

func main() -> int
    return state.frame_count
end
```

Cross-namespace write is not allowed:

```zt
namespace app.main

import app.runtime.state as state

func force_reset()
    state.frame_count = 0
end
```

## Types

Builtin scalar types:

```zt
bool
int
int8
int16
int32
int64
u8
u16
u32
u64
float
float32
float64
text
bytes
void
```

Generic surface types:

```zt
list<T>
map<Key, Value>
optional<T>
result<Success, Error>
```

Rules:

- `text` is canonical; `string` is not a separate user type
- `int` means fixed `int64` on all platforms
- `float` means fixed `float64` on all platforms
- plain `uint` is not in the MVP
- width-specific numeric types are opt-in
- `bytes` is a fundamental binary type in the surface language
- `void` is restricted to no-value type positions such as `result<void, E>`
- `void` is not valid for locals, fields, parameters, lists, maps or optionals
- `grid` is not a core MVP type

## Numeric Model

Numeric conversions are explicit.

Canonical conversion functions:

```zt
const count: int = to_int(value)
const ratio: float = to_float(value)
const label: text = to_text(value)
```

Rules:

- no implicit conversion between numeric types
- contextual literal fitting is allowed
- mixed numeric operations require explicit conversion
- integer overflow is checked by default
- division by zero is a runtime numeric error
- float to int conversion truncates toward zero unless a future explicit rounding API is used

## Text

Text is UTF-8.

Text literals use double quotes.

Multiline text uses triple double quotes.

```zt
const title: text = "Zenith"
const page: text = """
Hello
Zenith
"""
```

Concatenation uses `+` only for `text + text`. Implicit conversions do not exist. To concatenate numbers or booleans outside interpolation, use `to_text(value)` explicitly.

Interpolation and Formatting:

```zt
-- Day-to-day interpolation uses fmt.
const message: text = fmt "Player {player.name} has {player.hp} HP"

-- Explicit conversion outside fmt remains available.
const report: text = "Coordinates: X=" + to_text(x) + ", Y=" + to_text(y)
```

Rules:

- ordinary text literals do not interpolate.
- interpolation requires `fmt`.
- expressions inside `{...}` in `fmt` are ordinary expressions.
- each interpolated expression must implement `TextRepresentable<T>`.
- formatting helpers belong to `std.format`.
- explicit UTF-8 conversion between `text` and `bytes` belongs to `std.text`.

## Bytes

`bytes` is immutable binary data.

Canonical byte literal:

```zt
const magic: bytes = hex bytes "DE AD BE EF"
const packet: bytes = hex bytes """
    DE AD BE EF
    00 01 02 03
"""
```

Rules:

- byte literals use the `hex bytes` prefix
- byte literals produce immutable `bytes`
- byte literals contain hexadecimal data, not text data
- ASCII whitespace and `_` may separate byte pairs
- each byte is exactly two hexadecimal digits after separators are ignored
- `0x` prefixes inside byte literals are not canonical
- indexing `bytes` yields `u8`
- slicing `bytes` yields `bytes`
- `len(...)` applies to `bytes`
- no implicit conversion exists between `text` and `bytes`

Explicit UTF-8 conversion:

```zt
const data: bytes = text.to_utf8("Hello")
const decoded: text = text.from_utf8(data)?
```

## Collections

Canonical literals:

```zt
const ports: list<int> = [3000, 3001]
const scores: map<text, int> = {"Julia": 10}
```

Rules:

- `const` collections are observably immutable, including through indexed access.
- `const` collections do not permit deep observable mutation through their elements.
- `var` collections allow explicit element/key updates.
- List mutations are strictly performed via `list.append(item)` and `list.prepend(item)` (banishing ambiguous names like `push` or `unshift`). These are mutating operations utilizing Copy-On-Write under the hood.
- list, text and bytes indexing are zero-based.
- slice end is inclusive.
- omitted slice bounds are allowed.
- **List lookups**: Using the indexer `list[index]` predictably **panics** when the index is out of bounds. For safe lookups checking bounds, use `list.get(index) -> optional<Item>`.
- **Map lookups**: Using the indexer `map[key]` predictably **panics** when the key is absent. For safe lookups checking absence, use `map.get(key) -> optional<Value>`.
- map keys require `Hashable<Key>` and `Equatable<Key>`.
- direct deep mutation such as `players[0].hp = 80` is not MVP syntax.
- explicit read-modify-write is the MVP form for deep indexed updates.
- future update blocks may provide readable read-modify-write sugar.

## Optional, Result And Absence

Zenith has no `null`.

Absence uses `optional<T>` and `none`.

Recoverable failure uses `result<T, E>`.

Optional match form:

```zt
match maybe_user
case value user:
    return user.name
case none:
    return "anonymous"
end
```

Propagation uses `?` for `optional<T>` and `result<T, E>`.

```zt
const user: User = load_user(id)?
```

```zt
const value: int = read_cached_score()?
```

Rules:

- `some(value)` is not required in MVP user code
- `case value name` binds an optional present value
- `?` is reserved for propagation/early return in functions that return `optional<...>` or `result<...>`
- `?` is not safe navigation
- panic is not caught by result/optional flow

## Expressions And Operators

Expression forms:

- function call: `name(args)`
- field access: `value.field`
- indexing: `value[index]`
- grouping: `(expression)`
- unary: `-value`, `not value`
- arithmetic: `*`, `/`, `%`, `+`, `-`
- comparison: `==`, `!=`, `<`, `<=`, `>`, `>=`
- logical: `and`, `or`

Rules:

- conditions must be `bool`
- no truthiness
- no ternary operator in MVP
- no `++` or `--`
- no `&&`, `||` or symbolic `!`
- no exponentiation operator in MVP; use future `math.pow`

Precedence from strongest to weakest:

1. field access, call, indexing
2. unary
3. multiplicative
4. additive
5. comparison
6. `and`
7. `or`

## Control Flow

Supported forms:

```zt
if ready
    start()
else if waiting
    queue()
else
    stop()
end

while index < total
    index = index + 1
end

for player in players
    display(player)
end

for key, value in scores
    show_score(key, value)
end

repeat 5 times
    io.print("tick")
end
```

Rules:

- all control blocks close with `end`
- `else if` is canonical; `elif` is not
- `unless` is not MVP syntax
- `for` is collection-oriented, not C-style
- `break` and `continue` apply to the nearest loop
- `repeat` count is evaluated once before the loop
- negative repeat count is invalid

## Structs

Structs are value-oriented composition units.

```zt
struct Player
    name: text
    hp: int where it >= 0
end

var p1: Player = Player(name: "Julia", hp: 100)
p1.hp = 80
```

Rules:

- construction uses `Type(field: value)`
- `new` is not canonical
- fields are explicitly typed
- field defaults are allowed when implemented by the compiler cut
- assigning fields requires a writable receiver

## Traits, Apply And Methods

Traits define behavior contracts.

`apply` attaches behavior to a type.

```zt
trait Healable
    mut func heal(amount: int)
end

apply Healable to Player
    mut func heal(amount: int)
        self.hp = self.hp + amount
    end
end
```

Rules:

- Zenith does not use classical OOP inheritance
- composition uses structs, traits and apply
- mutating methods are explicitly marked with the `mut` prefix (`mut func`)
- `mut` means the receiver may be changed through `self`
- receiver methods do not list `self` in the signature
- `self` is a reserved receiver value inside method bodies
- method bodies use explicit `self.field` for receiver access

## Enums And Match

Enums may have payloads.

Block and inline forms are valid when readable.

```zt
enum LoadUserError
    NotFound(id: int)
    InvalidName(message: text)
end

enum Direction = North, South, East, West
```

Construction uses qualified cases:

```zt
const error: LoadUserError = LoadUserError.NotFound(id: 10)
```

`match` supports multiple literal cases and default:

```zt
match status
case 10, 20, 30:
    handle_known()
case default:
    handle_other()
end
```

## Generics, Constraints and Dynamic Dispatch

Generic declarations are explicit.

The implementation strategy for standard generics is **Monomorphization**. It creates specialized, highly performant C code for each generic type substitution.

Generic call type arguments may be omitted when inference is unambiguous.

Constraints use `where` for trait bounds.

To support heterogeneous collections (e.g., UI elements, Game nodes) where monomorphization is impossible, Zenith introduces **Dynamic Dispatch via `dyn` (Fat Pointers)**.

```zt
-- Monomorphized (Zero-Cost Abstraction, homogenous)
const list_of_ints: list<int> = [1, 2, 3]

-- Dynamic Dispatch (Heterogeneous collection of traits)
var ui_nodes: list<dyn Widget> = []
ui_nodes.append(Button())
```

Rules:
- Monomorphization is the default for generic struct and function declarations.
- `dyn Trait` forces the compiler to generate Fat Pointers (vtable + data) allowing diverse structs conforming to `Trait` to sit in the same generic collection.
- Constraints use `where` for trait bounds.

```zt
func contains<Item>(items: list<Item>, target: Item) -> bool
where Item is Equatable<Item>
    ...
end
```

Core traits are implicit and do not require `import core...`.

Core traits include:

- `Equatable<Item>`
- `Hashable<Item>`
- `Comparable<Item>`
- `TextRepresentable<Item>`

## Value-Level Where Contracts

`where` has two roles separated by syntax position.

- `where` attached to a typed field or parameter is a runtime value contract.
- `where` after a generic declaration header is a compile-time type constraint.

Value-level `where` is a runtime contract for fields and parameters.

```zt
struct User
    name: text where len(it) > 0
end

func set_age(age: int where age >= 0)
    ...
end
```

Rules:

- field `where` is allowed
- parameter `where` is allowed
- local variable `where` is not MVP syntax
- return type `where` is not MVP syntax
- older decision examples with return `where` are superseded by this spec
- failed checks produce contract panic
- `it` and the parameter name may refer to the checked value
- future `std.validate` helpers may improve readability

A constructor for a struct with `where` fields remains a direct constructor. It does not secretly return `result<T, E>`.

Recoverable validation should be expressed by an explicit result-returning function such as `try_create_*`.

Canonical pattern:

```zt
struct Player
    hp: int where it >= 0
end

func try_create_player(hp: int) -> result<Player, text>
    if hp < 0
        return error("hp must be >= 0")
    end

    return success(Player(hp: hp))
end
```

## Panic And Recoverable Errors

`panic(message: text)` is a core fatal error mechanism.

Panic is not ordinary recoverable control flow.

Recoverable errors use `result<T, E>`.

Future `attempt/rescue/finally` may make result/optional flow more readable, but it does not catch panic.

## Attributes

Attributes use `attr`.

```zt
attr test
func loads_user()
    check(true)
end
```

Rules:

- one `attr` per line
- attributes apply to the next declaration
- attributes are metadata, not macros
- unknown attributes are errors unless explicitly introduced later

## Tests In Source

Tests are ordinary functions marked with `attr test`.

```zt
attr test
func creates_user()
    check(true)
end
```

Rules:

- tests take no parameters
- tests return no value
- `check(...)` is the canonical assertion helper
- ordinary build does not run tests
- `zt test` discovers and runs tests

## UI And Abstract Trees (Zero-Magic-DSL)

Zenith strictly repudiates syntactic-sugar macros and secret DSLs (Domain Specific Languages) for UI or Graph generation (like JSX or HTML). 
Building generic UI Trees uses the explicit structural composition paradigm.

```zt
var interface = Column(
    alignment: Align.Center,
    children: [
        Image(source: "logo.png"),
        Text("Bem-vindo!", size: 24)
    ]
)
```

Rules:
- Interfaces are just standard parameterized structs (e.g. `Widget`).
- Children are simply arrays (`list<dyn Widget>`) passed to constructor arguments.
- The compiler performs no specialized parsing or compilation magic for UI development.

## Explicitly Not MVP

Not part of the MVP surface:

- `any`
- `null`
- macros
- user-facing C interop
- overloads
- lambdas
- safe navigation with `?`
- conditional compilation in `.zt`
- optional dependencies/features in source syntax
- doc tests
- VM/bytecode execution model

