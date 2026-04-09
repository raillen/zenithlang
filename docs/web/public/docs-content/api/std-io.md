# Módulo de I/O (`std.io`)

O módulo `io` fornece funções para entrada e saída de dados, interação com o console e manipulação de arquivos.

## Saída de Dados

### `print(val: any)`
Imprime um valor no console seguido de uma nova linha.
- **Exemplo**: `print("Olá Mundo")`

### `write(val: any)`
Imprime um valor no console sem adicionar uma nova linha.

## Entrada de Dados

### `input(prompt: text) -> text`
Exibe um prompt e aguarda a entrada do usuário no teclado.
- **Exemplo**: `var nome = input("Qual seu nome? ")`

## Manipulação de Arquivos

### `read_file(path: text) -> text`
Lê o conteúdo completo de um arquivo.
- **Exemplo**: `var config = read_file("config.json")`

### `write_file(path: text, content: text)`
Escreve um texto em um arquivo, sobrescrevendo o conteúdo anterior.

```zt
import std.io

var log: text = "Sistema iniciado."
io.write_file("log.txt", log)
```
