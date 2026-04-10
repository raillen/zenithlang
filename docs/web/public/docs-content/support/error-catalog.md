# Catálogo de Erros

O compilador atual usa famílias de código `ZT-L`, `ZT-P` e `ZT-S`.

## Léxico

### `ZT-L001` — string não terminada

Uma aspas foi aberta e não foi fechada.

```zt
var nome: text = "Zenith
```

### `ZT-L002` — número seguido de identificador inválido

O lexer encontrou algo como `123abc`.

### `ZT-L003` — caractere desconhecido

Há um símbolo que o lexer não reconhece.

## Parsing

### `ZT-P001` — forma sintática incompleta ou token esperado

É o erro mais comum de parsing e aparece quando falta `end`, `)`, `:` ou outra peça estrutural.

Exemplos típicos:

- esquecer `end`
- escrever `apply Trait Player` em vez de `apply Trait to Player`
- omitir `=>` em tipo de função

### `ZT-P002` — expressão esperada

Aparece quando o parser precisava de uma expressão e encontrou fim de linha, token errado ou operador sem operando.

## Semântica

### `ZT-S001` — redefinição

Mesmo símbolo declarado duas vezes no mesmo escopo, ou parâmetro genérico repetido.

### `ZT-S002` — símbolo, tipo ou membro inexistente

Pode indicar:

- tipo não encontrado
- struct não encontrada
- alias não encontrado

### `ZT-S008` — membro não encontrado no tipo

Acesso a campo ou método inexistente:

```zt
player.foobar()
```

### `ZT-S009` — dependência circular em alias

Dois aliases acabam apontando um para o outro.

### `ZT-S100` — incompatibilidade de tipos

Caso mais comum do binder:

```zt
var x: int = "texto"
```

Também aparece em padrões de `match`, retornos e restrições genéricas.

### `ZT-S101` — declaração ou operador inválido

Pode significar:

- variável sem tipo obrigatório
- operador usado com tipos inválidos
- declaração sem padrão ou nome válido

### `ZT-S102` — condição, índice ou padrão com tipo inválido

Esse código é reutilizado para erros de contexto como:

- condição de `if` ou `while` que não é `bool`
- índice que não é inteiro
- `range` com limites não inteiros
- variante de padrão inexistente

### `ZT-S103` — chamada inválida ou `?` em tipo incompatível

Aparece quando algo não chamável é usado como função, ou quando `?` é aplicado fora de `Optional`/`Outcome`.

### `ZT-S104` — contexto genérico ou uso de `?` inválido

Pode indicar:

- quantidade errada de argumentos genéricos
- uso de `?` fora de uma função

### `ZT-S105` — argumentos inválidos

Usado para:

- argumento nomeado desconhecido
- argumento faltante
- parâmetros duplicados
- incompatibilidade entre `?` e o tipo de retorno da função atual

### `ZT-S201` — `self` ou `@campo` fora de método

`self` e `@nome` só podem aparecer dentro de método de `struct` ou `trait`.

### `ZT-S202` — `await` fora de função `async`

```zt
var dados = await carregar()
```

Só funciona dentro de `async func` ou lambda `async`.

### `ZT-S301`, `ZT-S302`, `ZT-S303` — erros de `trait` e `apply`

- `ZT-S301`: trait não encontrada
- `ZT-S302`: struct não encontrada
- `ZT-S303`: struct não implementa método exigido pela trait

## Dicas práticas

- Quando o erro for `ZT-P001`, procure o bloco logo acima da linha sinalizada.
- Quando o erro for `ZT-S100`, compare o tipo esperado com o tipo real do valor.
- Em `ZT-S202`, suba a função para `async func`.
- Em `ZT-S301` a `ZT-S303`, revise o `apply Trait to Struct` e a assinatura dos métodos.
