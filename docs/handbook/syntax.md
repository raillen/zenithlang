# Guia de Sintaxe Zenith

> Snapshot alinhado ao estado atual do repositório em 2026-04-10.

Este guia cobre a sintaxe que o parser e a stdlib expõem hoje. Onde a implementação do runtime ainda está em transição, isso é indicado explicitamente.

## 1. Fundamentos

### Comentários

```zt
-- Comentário de linha
```

### Declarações

Zenith mantém tipagem explícita para variáveis nomeadas.

```zt
var energia: int = 100
const PI: float = 3.14
global DEBUG: bool = false
```

Palavras-chave centrais:

- `var`
- `const`
- `global`
- `pub`

### Tipos primitivos

| Tipo | Uso |
| :-- | :-- |
| `int` | Inteiros |
| `float` | Números fracionários |
| `text` | Strings |
| `bool` | Booleanos |
| `null` | Ausência de valor |
| `void` | Retorno vazio |
| `any` | Escape hatch de tipagem |

### Coleções e modificadores

As coleções principais do Zenith são `list`, `map` e `grid`.

```zt
var nomes: list<text> = ["Ana", "Ivo"]
var flags: map<text, bool> = { "debug": true }
var mapa: grid<int> = grid<int>(10, 10, 0)
var unicos: uniq list<text> = ["a", "b"]
```

### Nulabilidade

`T?` é açúcar para `T | null`.

```zt
var apelido: text? = null
```

### Alias e união

Use `type` para alias semântico e `union` para nomes públicos de tipos soma.

```zt
type NomeArquivo = text
union Resultado<T> = T | text
```

### Tipos de função

```zt
var op: (int, int) => int
```

## 2. Controle de fluxo

### Condicionais

```zt
if energia > 80
    print("Alta")
elif energia > 20
    print("Média")
else
    print("Baixa")
end
```

Observação: Zenith não usa `then`.

### Loops

```zt
while vivo
    break
end

for item in inventario
    continue
end

repeat 3 times
    print("tick")
end
```

### `match`

`match` aceita padrões literais, tipos, listas, variantes e structs.

```zt
match valor
    case 10:
        print("dez")
    case [1, a, ..resto]:
        print(a)
    case Player { nome, vida: 0 }:
        print(nome)
    else:
        print("sem match")
end
```

`case` pode usar `:` ou `=>`.

### Tratamento de erro

```zt
attempt
    throw "falha"
rescue e
    print(e)
end
```

### `check`

`check` valida uma condição e executa o bloco `else` quando falha.

```zt
check idade >= 18 else
    panic("acesso negado")
end
```

Também existe `check expr` como expressão prefixa.

## 3. Funções e lambdas

### Funções nomeadas

```zt
func somar(a: int, b: int) -> int
    return a + b
end
```

O parser aceita `:` e `->` para retorno, mas a forma preferida na documentação é `->`.

### `async func` e `await`

```zt
async func carregar() -> text
    return "ok"
end

async func main() -> int
    var valor: text = await carregar()
    print(valor)
    return 0
end
```

O token `do` também é aceito antes do corpo:

```zt
async func carregar() -> text do
    return "ok"
end
```

### Parâmetros com padrão e argumentos nomeados

```zt
func abrir(path: text, mode: text = "r")
    print(path + " / " + mode)
end

abrir("a.txt")
abrir(path: "b.txt", mode: "w")
```

### Desestruturação em parâmetros

```zt
func somar_vetor(Vec2 { x, y }) -> int
    return x + y
end

func primeiro([a, b, ..resto]: list<int>) -> int
    return a + b
end
```

### Lambdas

```zt
var dobro = (n: int) => n * 2

var soma = (a: int, b: int) => do
    var total: int = a + b
    return total
end

var worker = async (msg: text) => msg
```

## 4. Tipos compostos

### `struct`

```zt
struct Player
    pub nome: text
    pub vida: int = 100 where it >= 0
end
```

O `where` em campo define um contrato de runtime.

### `enum`

```zt
enum Status
    Loading
    Success(value: text)
    Failure(error: text)
end
```

### `trait` e `apply`

```zt
trait Greetable
    pub func greet() -> text
end

apply Greetable to Player
    pub func greet() -> text
        return "Oi, eu sou " + @nome
    end
end
```

### `redo`

`redo` redefine um método já existente.

```zt
redo func Player.greet() -> text
    return "sobrescrito"
end
```

## 5. Expressões e operadores

### Conversão e refinamento

```zt
var valor: int | text = 10

if valor is int
    print("é int")
end

if valor is not text
    print("não é text")
end

var texto = valor as text
```

### Tamanho, range, spread e slicing

```zt
var itens = [1, 2, 3]
print(#itens)

for i in 1..3
    print(i)
end

var copia = [..itens, 4, 5]
var trecho = itens[0..1]
```

`..` também aparece em padrões de lista e em chamadas:

```zt
func somar_tres(a: int, b: int, c: int) -> int
    return a + b + c
end

var nums = [10, 20, 30]
print(somar_tres(..nums))
```

### `!` e `?`

```zt
var nome = apelido!
var dados = fs.read_text_file("app.txt")?
```

- `expr!` afirma que o valor não é nulo.
- `expr?` propaga `Optional` ou `Outcome` para fora da função atual.

### UFCS e acesso a membros

`@campo` é açúcar para `self.campo`.

```zt
var resultado: int = 10
    .step1(5)
    .step2()
```

No parser atual, `.` continua existindo como acesso a membro e também como base do encadeamento UFCS.

### `native lua`

```zt
native lua
    print("bloco Lua puro")
end

var resposta = native lua 20 + 22 end
```

## 6. Módulos

### `namespace`, `import` e `export`

```zt
namespace app.main

import std.fs
import std.time as tempo

export func agora() -> float
    return tempo.now().unix
end
```

## 7. Reatividade e testes

### `state`, `computed` e `watch`

Essas palavras-chave existem e fazem parte do parser e do runtime Lua, mas a ergonomia final ainda está em estabilização.

```zt
state counter: int = 10
computed doubled: int = counter * 2

watch
    print(doubled)
end
```

Use esse conjunto com cautela em código novo até a camada de runtime async/reativa estabilizar completamente.

### `group`, `test` e `assert`

O parser reconhece DSL de testes:

```zt
group "math"
    test "soma"
        assert(true, "deveria passar")
    end
end
```

O comando de CLI `zt test` ainda está em evolução no repositório atual.

## 8. Inventário de palavras-chave

### Declaração e visibilidade

`var`, `const`, `global`, `pub`, `func`, `async`, `await`, `namespace`, `import`, `export`, `redo`

### Tipos e modelagem

`struct`, `enum`, `trait`, `apply`, `to`, `type`, `union`, `where`, `grid`, `uniq`

### Fluxo

`if`, `elif`, `else`, `end`, `while`, `for`, `in`, `repeat`, `times`, `return`, `break`, `continue`, `match`, `case`, `attempt`, `rescue`, `check`, `throw`, `watch`, `do`

### Operadores e refinamento

`as`, `is`, `and`, `or`, `not`

### Literais e referências especiais

`true`, `false`, `null`, `self`, `it`, `_`

### DSL de testes

`group`, `test`, `assert`

### Interop

`native`
