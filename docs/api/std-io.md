# Módulo de I/O (`std.io`)

O módulo `io` fornece funções para entrada e saída de dados, interação com o console e manipulação de arquivos.

## Saída de Dados

### `write(val: any)`
Imprime um valor no console sem adicionar uma nova linha.

### `write_line(val: any)`
Imprime um valor no console seguido de uma nova linha. Equivalente ao `print` do Core, mas localizado no namespace `io`.

## Entrada de Dados

### `read_line() -> text`
Aguarda e lê uma linha de entrada do usuário no terminal.
- **Exemplo**: `var nome = io.read_line()`

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
