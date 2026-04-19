# Análise Profunda da Sintaxe da Linguagem Zenith v2

Data: 18 de Abril de 2026

## 1. Filosofia e Princípios de Design

A sintaxe de Zenith é orientada por cinco princípios fundamentais:

### 1.1 Leitura em Primeiro Lugar (Reading-First)
- Preferência por palavras claras sobre símbolos densos
- **Exemplos:**
  - `not`, `and`, `or` em vez de `!`, `&&`, `||`
  - `func` e `end` em vez de `fn` e `{}`
  - Uma forma canônica para cada construção (não múltiplas equivalentes)

### 1.2 Baixo Atrito Neural (Low Neural Friction)
- Projetado para ADHD/dislexia-friendly
- Estrutura visual consistente e previsível
- Redução de ruído visual através de blocos explícitos

### 1.3 Explicitude Total
- Sem imports implícitos
- Sem coercões automáticas
- Sem defaults mágicos
- Nomes totalmente qualificados para símbolos importados

### 1.4 Previsibilidade
- Uma forma canônica para cada conceito
- Comportamento consistente através da linguagem
- Visibilidade explícita de fluxo de controle

### 1.5 Rigor Industrial
- Sem ambiguidades nas especificações
- Testes comportamentais guidance para features
- Estabilidade arquitetural prioritizada

---

## 2. Estrutura de Arquivo Fonte

### 2.1 Forma Canônica

```zt
-- Comentário opcional de implementação
namespace app.users

import std.io as io
import app.users.types as types

public func main() -> int
    return 0
end
```

### 2.2 Regras Obrigatórias

1. **Namespace é primeiro:** Declara a identidade do arquivo
   - Qualquer namespace por arquivo (não múltiplos)
   - Apenas comentários e linhas em branco antes dele

2. **Imports seguem namespace:** Qualificam os nomes externos
   - `import path.to.namespace as alias` é a forma canônica
   - Nenhum `from...import`, `import *` ou imports relativos (MVP)
   - Nomes importados permanecem qualificados: `io.print()` não `print()`

3. **Declarações de topo seguem imports:**
   - Privadas por padrão
   - `public` para expor fora do namespace
   - Múltiplos arquivos podem compartilhar um namespace (symbols privados compartilhados)

### 2.3 Comentários

- **Linha:** `-- comentário até o final da linha`
- **Bloco:** `--- comentário em bloco ---`
- **Documentação pública:** ZDoc (fora da sintaxe MVP)

---

## 3. Funções

### 3.1 Sintaxe Básica

```zt
func greet(name: text) -> text
    return "Hello, " + name
end

func log(message: text)
    -- Função sem retorno (omite ->)
    io.print(message)
end
```

### 3.2 Características Principais

| Aspecto | Regra |
|---------|-------|
| **Parâmetros** | Tipados explicitamente: `name: Type` |
| **Retorno** | `-> Type` ou omitido para void |
| **Blocos** | Delimitados por `end` |
| **Explicitude** | `return` explícito quando retorna valor |
| **Defaults** | `param: Type = value` (parâmetros padrão no final) |

### 3.3 Parâmetros Nomeados e Defaults

```zt
func open_file(
    path: text,
    mode: text = "read",
    encoding: text = "utf-8"
) -> result<File, text>
    -- Implementação
end

-- Uso com argumentos nomeados
const file: File = open_file("data.txt", mode: "write")?
const file2: File = open_file("log.txt", encoding: "ASCII", mode: "append")?
```

**Regras:**
- Parâmetros obrigatórios antes dos padrão
- Parâmetros padrão formam um sufixo final
- Após um argumento nomeado, todos os restantes devem ser nomeados
- Expressões padrão avaliadas em tempo de chamada

---

## 4. Variáveis e Mutabilidade

### 4.1 Declarações

```zt
const name: text = "Julia"      -- Imutável
var hp: int = 100              -- Mutável
hp = hp - 10                    -- Reatribuição permitida
```

### 4.2 Semântica de Valor

- **Semântica de cópia:** `var b = a` copia valor, não referência
- **Copy-on-Write:** Coleções otimizadas para leitura repetida
- **`const` collections:** Observavelmente imutáveis, até através indexação
- **`var` collections:** Permitem mutação de elementos via operações explícitas

**Exemplo de Value Semantics:**
```zt
var player1: Player = Player(hp: 100)
var player2: Player = player1        -- Cópia profunda
player2.hp = 50                      -- Não afeta player1
```

---

## 5. Tipos e Sistema de Tipos

### 5.1 Tipos Escalares Built-in

```zt
bool              -- Booleano (true, false)
int               -- Inteiro 64-bit (padrão)
int8, int16, int32, int64    -- Inteiros específicos
u8, u16, u32, u64            -- Unsigned (sem plain uint em MVP)
float             -- Float 64-bit (padrão)
float32, float64  -- Floats específicos
text              -- UTF-8 string
bytes             -- Dados binários
void              -- Sem valor (apenas para resultados como result<void, E>)
```

### 5.2 Tipos Genéricos

```zt
list<T>           -- Lista homogênea
map<Key, Value>   -- Mapa chave-valor
optional<T>       -- Presença/ausência
result<T, E>      -- Sucesso/erro
```

### 5.3 Conversão Numérica (Explícita)

```zt
const value: float = 3.14
const rounded: int = to_int(value)      -- Trunca para zero
const text_repr: text = to_text(value)  -- Converte para texto
```

**Regras:**
- Sem conversão implícita entre tipos numéricos
- Sem coerção de contexto (exceto literal fitting)
- Operações mistas requerem conversão explícita

---

## 6. Texto e Interpolação

### 6.1 Literais

```zt
const title: text = "Zenith"

const multiline: text = """
Hello
Zenith
"""

const concatenated: text = "a" + "b"     -- Só text + text
```

### 6.2 Interpolação com `fmt`

```zt
const player: Player = Player(name: "Julia", hp: 100)
const message: text = fmt "Player {player.name} has {player.hp} HP"

-- Cada expressão deve implementar TextRepresentable<T>
```

**Regras:**
- Literais ordinários não interpolam
- `fmt "..."` para interpolação
- Conversão explícita fora de fmt: `"X=" + to_text(x)`

---

## 7. Bytes e Dados Binários

### 7.1 Literais Hexadecimais

```zt
const magic: bytes = hex bytes "DE AD BE EF"

const packet: bytes = hex bytes """
    DE AD BE EF
    00 01 02 03
"""
```

### 7.2 Conversão UTF-8

```zt
const text_data: text = "Hello"
const binary: bytes = text.to_utf8(text_data)        -- text → bytes
const decoded: text = text.from_utf8(binary)?        -- bytes → text (result)
```

---

## 8. Coleções

### 8.1 Listas

```zt
const ports: list<int> = [3000, 3001, 3002]

var scores: list<text> = ["A", "B"]
scores.append("C")                    -- Mutação explícita
scores.prepend("X")                   -- Prepend também disponível
```

**Operações:**
- **Indexação:** `list[0]` (zero-based)
- **Slicing:** `list[1..3]` (fim inclusivo)
- **Comprimento:** `len(list)`
- **Mutação:** `append()`, `prepend()`

### 8.2 Mapas

```zt
var config: map<text, text> = {"mode": "debug"}

-- Acesso (panic se não existir)
const mode: text = config["mode"]

-- Lookup seguro
const theme: optional<text> = config.get("theme")

-- Mutação
config["theme"] = "dark"
```

**Regras:**
- `map[key]` panics se key não existe
- `map.get(key)` retorna `optional<Value>`
- Requer `Hashable<Key>` e `Equatable<Key>`

---

## 9. Opcionais e Resultados

### 9.1 Opcionais (Ausência)

```zt
func find_user(id: int) -> optional<User>
    -- Retorna none ou User
end

-- Uso com match
match maybe_user
case value user:
    io.print(user.name)
case none:
    io.print("anonymous")
end
```

### 9.2 Resultados (Erro Recuperável)

```zt
func load_data(path: text) -> result<Data, text>
    if file_exists(path)
        return success(read_file(path))
    else
        return error("File not found")
    end
end

-- Propagação com ?
func process() -> result<void, text>
    const data: Data = load_data("data.txt")?
    io.print(data)
    return success()
end
```

**Operador Propagação `?`:**
- Extrai sucesso ou retorna erro ao chamador
- Não é safe navigation, é early return
- Rejeita panic (não é tratado por optional/result)

---

## 10. Expressões e Operadores

### 10.1 Tabela de Operadores

| Categoria | Operadores | Exemplo |
|-----------|-----------|---------|
| **Primárias** | `.`, `()`, `[]`, `()` | `obj.field`, `func()`, `list[0]` |
| **Unária** | `-`, `not` | `-5`, `not true` |
| **Multiplicativa** | `*`, `/`, `%` | `a * b`, `a / b`, `a % b` |
| **Aditiva** | `+`, `-` | `a + b`, `a - b` |
| **Comparação** | `==`, `!=`, `<`, `<=`, `>`, `>=` | `a == b` |
| **Lógica AND** | `and` | `a and b` |
| **Lógica OR** | `or` | `a or b` |

### 10.2 Precedência (Forte para Fraca)

1. Acesso campo, chamada, indexação
2. Unária (`-`, `not`)
3. Multiplicativa (`*`, `/`, `%`)
4. Aditiva (`+`, `-`)
5. Comparação (`==`, `!=`, `<`, `<=`, `>`, `>=`)
6. `and`
7. `or`

### 10.3 Regras Especiais

- Condições devem ser `bool` (sem truthy/falsy)
- Sem operador ternário em MVP
- Sem `++`, `--`
- Sem `&&`, `||` (use palavras: `and`, `or`)
- Sem exponentiation (use `math.pow()` no futuro)

---

## 11. Controle de Fluxo

### 11.1 If-Else

```zt
if resource_available
    allocate()
else if waiting
    queue()
else
    block()
end
```

**Regras:**
- `else if` (não `elif`)
- Sem `unless` (MVP)
- Todos os blocos fecham com `end`

### 11.2 While

```zt
var count: int = 0
while count < 10
    count = count + 1
end
```

### 11.3 For (Orientado a Coleção)

```zt
-- Iteração simples
for player in players
    display(player)
end

-- Iteração com índice
for item, idx in items
    io.print(fmt "{idx}: {item}")
end

-- Key-value iteration
for key, value in scores
    io.print(fmt "{key} = {value}")
end
```

**Regras:**
- `for` não é C-style (sem inicializer, incrementor)
- `break` e `continue` suportados

### 11.4 Repeat

```zt
repeat 5 times
    io.print("tick")
end
```

**Regras:**
- Count avaliado uma única vez
- Count negativo é inválido

---

## 12. Structs

### 12.1 Definição

```zt
struct Player
    name: text
    hp: int where hp >= 0          -- Runtime value contract
    level: int = 1                 -- Default value
end

struct Position
    x: float
    y: float where it >= 0
end
```

### 12.2 Construção e Uso

```zt
const player: Player = Player(name: "Julia", hp: 100)

var mutable_player: Player = Player(name: "Bob", hp: 50)
mutable_player.hp = 45

-- Value semantics: cópia profunda
var player2: Player = mutable_player
player2.hp = 30  -- Não afeta mutable_player
```

**Regras:**
- Construção: `Type(field: value)`
- Sem `new` (não canônico)
- Tipagem explícita de campos
- Defaults opcionais (quando implementados)
- `where` para contratos de valor em campos

---

## 13. Traits e Métodos

### 13.1 Definição de Trait

```zt
trait Healable
    mut func heal(amount: int)
end

trait Displayable
    func display() -> text
end
```

### 13.2 Apply para Métodos Simples

```zt
apply Player
    func get_status() -> text
        return fmt "{self.name}: {self.hp} HP"
    end
    
    mut func take_damage(amount: int)
        self.hp = self.hp - amount
    end
end
```

### 13.3 Apply para Trait Implementation

```zt
apply Healable to Player
    mut func heal(amount: int)
        self.hp = self.hp + amount
    end
end

apply Displayable to Player
    func display() -> text
        return fmt "{self.name} (Level {self.level})"
    end
end

-- Uso
var player: Player = Player(name: "Julia", hp: 100)
player.heal(20)
io.print(player.display())
```

**Regras:**
- Métodos não listam `self` na assinatura
- `mut func` = método mutante (pode modificar receiver)
- `self` é implícito dentro de corpos de método
- `self.field` para acesso ao receiver
- Sem herança clássica OOP; composição via traits + apply

---

## 14. Enums

### 14.1 Sem Payload

```zt
enum Direction = North, South, East, West

const direction: Direction = Direction.North
```

### 14.2 Com Payloads

```zt
enum LoadUserError
    NotFound(id: int)
    InvalidName(message: text)
    NetworkError(code: int)
end

const error: LoadUserError = LoadUserError.NotFound(id: 42)
```

### 14.3 Match

```zt
match user_result
case success(user):
    io.print(user.name)
case error(msg):
    io.print(fmt "Error: {msg}")
end

match status_code
case 200, 201:
    handle_success()
case 404, 500:
    handle_error()
case default:
    handle_unknown()
end
```

**Regras:**
- Construção via `EnumType.CaseName(fields)`
- Match com casos literais e `default`
- Pattern matching com payloads

---

## 15. Genéricos

### 15.1 Declaração

```zt
func contains<Item>(items: list<Item>, target: Item) -> bool
where Item is Equatable<Item>
    for item in items
        if item == target
            return true
        end
    end
    return false
end
```

### 15.2 Constraints (Where)

```zt
struct Repository<Entity>
    entities: list<Entity>
end

apply Repository<Entity>
where Entity is Equatable<Entity>
    func find(target: Entity) -> optional<int>
        for entity, idx in self.entities
            if entity == target
                return idx
            end
        end
        return none
    end
end
```

### 15.3 Type Inference

```zt
const list_of_ints: list<int> = [1, 2, 3]
-- Tipo inferido a partir do literal
const found: bool = contains(list_of_ints, 2)
-- Item = int, inferido do argumento
```

### 15.4 Dynamic Dispatch (dyn)

```zt
-- Monomorphized (zero-cost, homogêneo)
const buttons: list<Button> = [Button(), Button()]

-- Dynamic dispatch (heterogêneo, fat pointers)
var widgets: list<dyn Widget> = []
widgets.append(Button())
widgets.append(TextBox())
-- Múltiplos tipos diferentes em uma lista
```

**Regras:**
- **Monomorphization:** Padrão para generics (cria código C especializado)
- **`dyn Trait`:** Força vtables para coleções heterogêneas
- **Zero-Cost Abstractions:** Monomorphization não tem overhead em runtime

---

## 16. Value-Level Where Contracts

### 16.1 Campos com Contratos

```zt
struct User
    name: text where len(it) > 0      -- Must be non-empty
    age: int where it >= 0 and it < 150
end
```

### 16.2 Parâmetros com Contratos

```zt
func set_age(age: int where age >= 0)
    -- age garantido ser >= 0 em tempo de execução
end

func allocate(size: int where size > 0)
    -- size deve ser positivo
end
```

### 16.3 Semantics

```zt
struct Position
    x: float where it >= 0.0
    y: float where it >= 0.0
end

-- Construtor direto, não result<T, E>
const pos: Position = Position(x: 10.0, y: 20.0)

-- Falha de contrato = panic
-- const bad: Position = Position(x: -5.0, y: 0.0)  -- Panics!

-- Para validação recuperável, use função result-returning
func try_create_position(x: float, y: float) -> result<Position, text>
    if x < 0.0
        return error("x must be non-negative")
    end
    if y < 0.0
        return error("y must be non-negative")
    end
    return success(Position(x: x, y: y))
end
```

**Regras:**
- `where` em campos e parâmetros = runtime value contracts
- Violação = `panic()`
- Construtores diretos, não result-returning
- Validação recuperável via `try_create_*` functions

---

## 17. Atributos (Attributes)

### 17.1 Sintaxe

```zt
attr test
func users_can_be_loaded()
    check(true)
end

attr deprecated
public func legacy_function()
    -- Função marcada como deprecated
end
```

**Regras:**
- Um `attr` por linha
- Aplicado à declaração seguinte
- Metadados, não macros
- Atributos desconhecidos são erros

---

## 18. Panic vs Recoverable Errors

### 18.1 Panic

```zt
func must_parse_int(s: text) -> int
    match parse_int_safe(s)
    case success(value):
        return value
    case error(msg):
        panic(msg)
    end
end
```

### 18.2 Recoverable (Result)

```zt
func parse_int_safe(s: text) -> result<int, text>
    if is_valid_int(s)
        return success(to_int_unchecked(s))
    else
        return error("Invalid integer format")
    end
end
```

**Regras:**
- `panic()` = erro fatal (não capturável)
- `result<T, E>` = erro recuperável
- Sem try-catch em MVP
- Propagação via `?` operator

---

## 19. Padrões de Acessibilidade

### 19.1 Legibilidade Word-Based

```zt
-- ✓ Alto contraste visual, fácil scanability
if ready and not waiting
    process()
end

-- Vs. simbólico (menos acessível para dislexia)
-- if (ready && !waiting) { process(); }
```

### 19.2 Estrutura Explícita

```zt
-- ✓ Blocos delimitados claramente
while index < total
    process(index)
end

-- Vs. `{}`
-- while (index < total) { process(index); }
```

### 19.3 Tipagem Clara

```zt
-- ✓ Tipos explícitos em todos os lugares
const name: text = "Julia"
var count: int = 0

-- Não deixa à inferência
```

---

## 20. Memory Safety e Value Semantics

### 20.1 Reference Counting (ARC)

```zt
const list1: list<int> = [1, 2, 3]
var list2: list<int> = list1        -- Cópia com refcount

-- list1 ainda valido (refcount > 1)
-- Quando list2 sai de escopo, refcount decremente
```

### 20.2 Copy-on-Write

```zt
var bag1: Bag = Bag(nums: [1, 2, 3])
var bag2: Bag = bag1                -- Shallow copy (COW)
bag2.nums[0] = 99                   -- Trigger deep copy

-- bag1.nums ainda [1, 2, 3]
-- bag2.nums é [99, 2, 3]
```

### 20.3 Sem Null

```zt
-- Zenith não tem null/nil
-- Ausência é expressa via optional<T>
func find_user(id: int) -> optional<User>
    if user_exists(id)
        return User(...)
    else
        return none
    end
end
```

---

## 21. Resumo de Características Sintáticas

| Característica | Canônico | Não Canônico |
|----------------|----------|-------------|
| **Negação** | `not` | `!` |
| **Lógica AND** | `and` | `&&` |
| **Lógica OR** | `or` | `\|\|` |
| **Blocos** | `end` | `{}` |
| **Função** | `func` | `fn` |
| **Retorno** | `->` | `:` |
| **Deixar** | `let` não canônico | `let` |
| **Público** | `public` | `pub` |
| **String** | `text` | `string` |
| **Else If** | `else if` | `elif` |
| **Unsigned Plain** | Não MVP | `uint` |
| **Conversão** | `to_int()` | Implícita |

---

## Conclusão

A sintaxe de Zenith representa um design deliberado onde **legibilidade, acessibilidade e explicitude** são prioritárias sobre brevidade. Cada escolha sintática é fundamentada em pesquisa sobre design acessível, redução de carga cognitiva e previsibilidade. O resultado é uma linguagem que é simultaneamente rigorosa em seus contratos e agradável de ler para programadores com ADHD/dislexia e qualquer pessoa que valoriza clareza técnica.