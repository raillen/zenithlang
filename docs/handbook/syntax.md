# Guia de Sintaxe Zenith (v0.2.5)

> Zenith é uma linguagem desenhada para ser lida e compreendida sem esforço mental excessivo. Se você tem TDAH ou Dislexia, este guia foi feito pensando em você.

---

## 0. Guia de Estudo (Neurodiversidade)

Programar pode ser cansativo para o cérebro. Aqui estão algumas dicas para ler esta documentação e o código Zenith:

*   **Pule para os Exemplos:** Se as definições técnicas parecerem confusas, olhe primeiro o código. Nossos exemplos são "autoexplicativos".
*   **Âncoras Visuais:** Use os emojis nas seções para se localizar rapidamente.
*   **Carga Cognitiva:** Zenith evita "sintaxes mágicas". Se você entende o básico, o resto segue a mesma lógica.
*   **Pausa Ativa:** Se sentir que as palavras estão "dançando", faça uma pausa. O Zenith foi feito para que você possa voltar de onde parou sem perder o fio da meada.

---

## 1. Fundamentos (O Básico)

Todo programa Zenith começa com o simples.

### Comentários
Use comentários para explicar o **porquê** e não o **quê**.
```zt
-- Isso é um comentário de uma linha.
-- O compilador ignora tudo depois do '--'.
```

### Variáveis e Constantes
Zenith é **explícito**. Você precisa dizer o que é o quê.

*   `var`: Coisas que **podem** mudar (Variáveis).
*   `const`: Coisas que **nunca** mudam (Constantes).

```zt
var vida: int = 100        -- 'vida' pode diminuir ou aumentar.
const PI: float = 3.14     -- O valor de PI é fixo.
```

### Tipos Primitivos
Os "tijolos" básicos da linguagem:

| Tipo | O que é? | Exemplo |
| :--- | :--- | :--- |
| `int` | Números inteiros | `10`, `-5`, `0` |
| `float` | Números com vírgula | `3.14`, `0.5` |
| `text` | Textos (Strings) | `"Olá Zenith"` |
| `bool` | Verdadeiro ou Falso | `true`, `false` |
| `null` | Vazio/Nada | `null` |
| `void` | Retorno vazio (funções) | `void` |
| `any` | Qualquer tipo | `any` |

### Uniões de Tipos (`|`)
Às vezes uma variável pode ser mais de uma coisa. Use o `|` para unir tipos. Você pode usar o operador `is` para verificar o tipo atual. O Zenith permite definir apelidos de tipos usando `type` e uniões usando `union`.

```zt
type ID = int | text
union Resultado = Success | Error

var id: ID = 10
```

> [!TIP]
> **Dica de Visualização:** Imagine `var` como uma caixa que você pode trocar o conteúdo, e `const` como um objeto gravado na pedra.

---

## 2. Lógica e Fluxo

Como o seu programa toma decisões.

### Condicionais (if, elif, else)
Zenith usa blocos claros que terminam com `end`.

```zt
if energia > 80
    print("Correndo rápido!")
elif energia > 20
    print("Caminhando...")
else
    print("Exausto.")
end
```

Use `elif` para condições extras e `else` para quando nada mais for verdade. O Zenith também usa os operadores lógicos `and`, `or` e `not` para combinar condições.

```zt
if energia > 50 and vivo
    print("Pronto para lutar!")
end
```

### O Poder do `match` (Destruturação)
O `match` no Zenith não apenas compara valores, mas também "desmonta" objetos.

```zt
-- Destruturando uma Lista
var lista: any = [10, 20, 30]
match lista
    case [10, a, ..resto]: 
        print("Capturou a={a}")      -- Captura o segundo item
        print("Tamanho do resto: {#resto}")
    else: print("Não casou")
end

-- Destruturando uma Struct
match player
    case Player { nome, vida: 0 }:
        print("{nome} foi derrotado!")
end
```

### O Statement `check`
O `check` é usado para validações rápidas. Se a condição for falsa, ele executa o bloco `else`. É excelente para evitar `ifs` aninhados e garantir pré-condições.

```zt
func processar(n: int)
    check n > 0 else
        print("Erro: n deve ser positivo!")
        return
    end
    -- Continua o processamento com segurança...
end
```

### Blocos `native lua`
Para interoperabilidade direta com Lua, o Zenith permite blocos `native lua`. O conteúdo dentro do bloco é transpilado sem alterações.

```zt
var x: int = 0
native lua
    x = 10 -- Código Lua puro
end

-- Também pode ser usado como expressão:
var y = native lua 20 + 22 end
```


### Loops (Repetições)
Repetir tarefas sem cansar o cérebro.

*   **`while`**: Repete enquanto algo for verdade.
*   **`for`**: Percorre uma lista ou intervalo.
*   **`repeat`** e **`times`**: Repete um número exato de vezes (Exclusivo do Zenith!).

```zt
-- Repetir 5 vezes (Simples e direto):
repeat 5 times
    print("Contagem regressiva!")
end

-- Percorrendo uma lista:
for item in inventario
    if item == "Bomba" then
        break -- Para o loop imediatamente.
    end
    print("Você tem: " + item)
end
```

Use `break` para sair de um loop e `continue` para pular para a próxima volta.

### Assincronia (`async`, `await`)
Para tarefas que demoram (como baixar um arquivo), o Zenith usa `async` e `await`. Isso evita que o programa inteiro "trave".

```zt
async func baixar_fase()
    var dados = await download("level1.zt")
    print("Fase carregada!")
end
```

---

## 3. Coleções e Dados

Como organizar muitos pedaços de informação.

### Listas (`list`)
As listas guardam itens um após o outro. Use `list<T>` para dizer o que tem dentro.

```zt
var inventario: list<text> = ["Espada", "Escudo", "Poção"]
append(inventario, "Arco")           -- Adiciona no final.
```

### Mapas (`map`)
Os mapas ligam uma **Chave** a um **Valor**. Use `map<K, V>` para dicionários.

```zt
var status: map<text, int> = {
    "força": 10,
    "mana": 50
}
```

### Grids (`grid`) - Matrizes 2D
O Zenith tem suporte nativo para `grid` (tabelas de duas dimensões). Perfeito para jogos!

```zt
-- Cria um mapa de jogo 10x10 preenchido com 0:
var mapa: grid<int> = grid<int>(10, 10, 0)
mapa[5, 5] = 1        -- Coloca um '1' no centro do mapa.
```

### O Modificador `uniq` (Conjuntos)
Se você quer uma lista que **não permite** itens repetidos, use `uniq`.

```zt
-- Mesmo que você tente adicionar "Rafael" duas vezes, ele só aparecerá uma.
var nomes: uniq list<text> = ["Rafael", "Joana"]
```

### Enumerações (`enum`)
O `enum` cria uma lista de opções fixas. É muito útil com o `match`.

```zt
enum Direcao
    Norte, Sul, Leste, Oeste
end

var d: Direcao = Direcao.Norte
```

---

## 4. Funções e Organização

Como criar comandos personalizados e organizar seu código em arquivos.

### Criando Funções (`func`)
Funções são receitas de bolo que seu código executa quando você chamar o nome delas.

*   `input`: Parâmetros que a função recebe.
*   `->`: O que a função devolve para você (Tipo de Retorno).

```zt
-- Função que soma dois números:
func somar(a: int, b: int) -> int
    return a + b
end

var total: int = somar(10, 5)        -- Chama a função e guarda o '15'.
```

### Argumentos Nomeados e Padrão
Você pode dar valores padrão para os parâmetros e chamar funções usando o nome dos argumentos.

```zt
func configurar(nome: text, volume: int = 80, brilho: int = 100)
    print("Configurando {nome}: Vol={volume}, Brilho={brilho}")
end

configurar("Menu")                   -- Usa os padrões (80, 100)
configurar("Jogo", brilho: 50)       -- Muda apenas o brilho
```

### Tipos de Função (Lambdas)
Às vezes você quer passar uma função como se fosse uma variável. Zenith usa o símbolo `=>` para definir esses tipos.

```zt
-- Uma função que recebe um 'int' e devolve um 'text':
var formatador: (int) => text

-- Uma função que não recebe nada e devolve um Optional:
var proximo: () => Optional<int>
```

### Namespaces e Imports
Seu código fica em gavetas chamadas `namespace`. Use `import` para pegar algo de outra gaveta e `pub` para deixar suas gavetas abertas para outros.

```zt
namespace player_logic

import std.math                      -- Pegando comandos matemáticos.

pub func pular() -> void
    -- Lógica de pular...
end
```

Utilize `export` para disponibilizar funções de um módulo e `global` para variáveis acessíveis em qualquer lugar. O símbolo `self` refere-se à instância atual de um objeto (ou `@` como atalho), e `_` é usado para descartar valores que você não vai usar.

---

## 5. Composição (OOP com Structs e Traits)

Zenith não usa o modelo tradicional de "Classes". Ele usa algo mais fácil e modular: **Composição**.

### Structs (Dados)
O `struct` define a forma de um objeto e seus campos. Você pode usar o operador `where` para validar dados.

```zt
struct Player
    pub nome: text
    pub vida: int = 100 where it >= 0  -- Não permite vida negativa!
end
```

### Traits (Comportamentos)
O `trait` define o que um objeto pode **fazer**. Agora suporta **implementações padrão** (corpo opcional).

```zt
trait Greetable
    pub func greet()
        print("Olá, eu sou um Zenith!") -- Padrão
    end
    
    pub func identify() -- Obrigatório implementar
end

### Traits com Tipos Genéricos
Assim como structs, traits podem trabalhar com tipos desconhecidos (`<T>`).

```zt
trait Comparavel<T>
    func e_igual_a(outro: T) -> bool
end
```
```

### Genéricos com Restrições (`where`)
Você pode exigir que um tipo genérico implemente uma `trait` específica.

```zt
-- T deve ser algo que saiba saudar
struct Box<T where T is Greetable>
    pub item: T
end

pub func saudar_tudo<T where T is Greetable>(lista: list<T>)
    for item in lista
        item.greet()
    end
end
```

O `apply` diz que um `Struct` agora também tem os comportamentos de um `Trait`.

```zt
apply Voador to Player

var p: Player = Player { nome: "Zenith" }
p.voar()                             -- Agora o Player sabe voar!
```

### Redefinição (`redo`)
O `redo` permite alterar o comportamento de uma função que já existe em um `struct` ou `trait`.

```zt
redo func Player.voar()
    print("Voo supersônico!")
end
```

> [!IMPORTANT]
> **Por que isso é bom para TDAH?** Você pode montar seus objetos como se fossem peças de LEGO, em vez de heranças gigantes e confusas.

---

## 6. Reatividade e Erros

Zenith ajuda você a lidar com mudanças de estado e problemas que podem acontecer.

### Sistema Reativo (`state`, `computed`, `watch`)
Ideal para interfaces e sistemas que precisam reagir a mudanças automaticamente.

*   `state`: Uma variável que "avisa" quando muda.
*   `computed`: Um valor que se recalcula sozinho quando um `state` muda.
*   `watch`: Um bloco de código que roda sempre que um `state` muda.

```zt
state contador: int = 0
computed dobro: int = contador * 2

watch contador
    print("O contador mudou! Dobro agora é: {dobro}")
end

contador = 5                         -- Isso dispara o 'watch' automaticamente.
```

### Tratamento de Erros (`attempt`, `rescue`)
Não deixe seu programa travar! Use `attempt` para tentar algo perigoso e `throw` para lançar seus próprios erros.

```zt
func validar(idade: int)
    if idade < 0
        throw "Idade não pode ser negativa"
    end
end

attempt
    validar(-1)
rescue e
    print("Ops, algo deu errado: " + e.message)
end
```

### O Operador Bang (`!`)
Se você sabe que uma função pode dar erro, mas quer que o erro seja tratado por quem chamou esta função, use `!`.

```zt
-- Tenta carregar o nível. Se falhar, o erro "sobe" para a função pai.
var nivel = carregar_nivel(1)!
```

---

## 7. Contratos e Metadados (Avançado)

Para quando você quer garantir que os dados sigam regras estritas.

### Cláusulas `where`
Você pode colocar regras diretamente na definição da variável.

```zt
struct Item
    -- A quantidade nunca pode ser menor que zero.
    pub quantidade: int = 1 where it >= 0
end
```

### Atributos `@`
Metadados que ajudam ferramentas e o próprio Zenith a entender melhor seu código.

```zt
struct Boss
    @min(100) @max(9999)
    pub vida: int = 500
end
```

### Testes Unitários (`group`, `test`)
O Zenith tem suporte nativo para testes, facilitando a vida do desenvolvedor. Use o comando `assert` para validar resultados.

```zt
group "Matemática"
    test "soma básica"
        assert(1 + 1 == 2)
    end
end
```

---

## Parabéns!
Você agora conhece a alma da linguagem Zenith. 

**Próximos passos:**
1. Tente rodar os exemplos da pasta `/examples`.
2. Se o cérebro cansar, lembre-se: **Zenith foi feito para ser lido com calma.**
3. Em caso de dúvidas, os erros do compilador Zenith tentam ser o mais amigáveis possível!
