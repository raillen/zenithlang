# Módulo Core (`std.core`)

O módulo `core` contém os tipos e funções fundamentais que estão sempre disponíveis no Zenith, sem necessidade de importação explícita (padrão).

## Tipos Básicos

### `int`
Representa números inteiros de 64 bits.
- **Exemplo**: `var x: int = 42`

### `float`
Representa números de ponto flutuante de dupla precisão.
- **Exemplo**: `var y: float = 3.14159`

### `text`
Representa sequências de caracteres UTF-8.
- **Exemplo**: `var s: text = "Zenith"`

## Funções Utilitárias

### `type_of(val: any) -> text`
Retorna o nome do tipo do valor fornecido como texto.

### `to_text(val: any) -> text`
Converte qualquer valor para sua representação textual.

### `is_null(val: any) -> bool`
Verifica se o valor é `null`.

## Funções de Sistema (Prelude)

Essas funções são injetadas no escopo global pelo compilador.

### `print(value: any)`
Escreve a representação textual do valor no console.

### `error(message: text)`
Interrompe a execução atual e lança um erro com a mensagem fornecida.

### `assert(condition: bool, message: text)`
Valida uma condição. Se for `false`, encerra o programa com a mensagem de erro.

### `panic(message: text)`
Encerra o programa imediatamente de forma irrecuperável. Use apenas em casos de erro crítico.

## Tipos Algébricos (Enums)

### `Optional<T>`
Representa a presença ou ausência de um valor do tipo `T`.
- `Present(value: T)`: Contém o valor.
- `Empty`: Representa a ausência de valor.

### `Outcome<T, E>`
Representa o resultado de uma operação que pode falhar.
- `Success(value: T)`: Operação bem sucedida.
- `Failure(error: E)`: Operação falhou com o erro `E`.

## Traits Fundamentais

### `Iterable<T>`
Interface que define que um objeto pode ser percorrido.
- `func iterator() -> () => Optional<T>`: Retorna uma função (lambda) que, ao ser chamada, devolve o próximo item envolto em um `Optional`.

## Verificadores de Tipo
Você pode usar o operador `is` para verificações em tempo de execução, especialmente com Uniões (`|`).

```zt
var id: int | text = 10
if id is int
    print("É um número!")
end
```
