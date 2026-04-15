# Syntax Guide: The Rhythm of Code

This guide details the structure of the Zenith language v0.3.1. Each feature is presented with its Golden Rule to ensure the lowest possible cognitive friction.

## 1. Fundamentals and Declarations

### Variables and Constants
Zenith explicitly separates what can change from what is fixed.

*   **Rule**: Use `var` for mutable data and `const` for fixed values at compile-time.
*   **Contrast**: Unlike Lua, where everything is mutable by default, Zenith protects your state.

```zt
var level: int = 1
const VERSION: text = "0.3.1"
```

### Visibility Keywords
*   `pub`: Makes a symbol visible outside the current file.
*   `global`: Declares in the VM scope (use with extreme caution).

---

## 2. Data Collections

Zenith offers three fundamental structures for organizing information.

| Collection | Type | Recommended Use |
| :--- | :--- | :--- |
| `list<T>` | Dynamic | Ordered sequences of items. |
| `map<K, V>` | Hash | Key-value dictionaries. |
| `grid<T>` | Static 2D | High-performance matrices for games/graphics. |

### Usage Example
```zt
var items: list<text> = ["Sword", "Shield"]
var config: map<text, int> = { "volume": 80 }
var map: grid<int> = collections.new_grid(10, 10, 0)
```

## 2. Data Structures and Indexing

Zenith uses high-level collections with safe indexing.

*   **list<T>**: Ordered sequences. **Zenith is 1-based** (index starts at 1).
*   **map<K, V>**: Key-value associations.

```zt
var items: list<text> = ["sword", "shield"]
var inventory: map<text, int> = { "gold": 100 }

var first_item = items[1] -- Zenith is 1-based!
var gold_amount = inventory["gold"]
```

*Note: Accessing index 0 in lists or strings will generate a warning (ZT-W002).*

---

## 3. Control Flow

### Conditionals (`if`)
Zenith uses textual keywords to avoid the noise of excessive parentheses.

```zt
if energy > 80
    print("Systems OK")
elif energy > 20
    print("Low Energy")
else
    print("Critical Failure")
end
```

### The Decision Oracle (`match`)
The `match` replaces long `if/else` chains with visual clarity.

*   **Golden Rule**: Use `=>` for single-line responses. Use `:` for complex code blocks.

```zt
match status
    case "active" => print("Online")
    case "error":
        log.error("Failure detected")
        try_recover()
    case _ => print("Unknown")
end
```

---

## 4. Error Handling and Safety

Zenith eliminates the fear of failures through explicit structures.

### The `check` Command
Validates a condition. If it fails, it executes the `else` block or interrupts the flow.

```zt
-- As a Statement
check age >= 18 else
    return Failure("Insufficient age")
end

-- As an Expression (Propagation)
var file = fs.read_text_file("save.dat")?
```

### `attempt / rescue` Blocks
To catch errors thrown by external functions or major failures.

```zt
attempt
    process_data()
rescue error
    print("Error caught: " + error)
end
```

---

## 5. Functions and Modeling

### Function Definition
The arrow `->` points to the result of the processing.

```zt
func calculate_damage(strength: int, weapon: int) -> int
    return strength + weapon
end
```

### Structures (`struct`) and Contracts (`trait`)
Zenith prefers Composition over Inheritance.

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
*Zenith Specification v0.3.1 - Syntax Handbook*
