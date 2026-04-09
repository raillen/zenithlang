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

## Verificadores de Tipo
Você pode usar o operador `is` para verificações em tempo de execução, especialmente com Uniões (`|`).

```zt
var id: int | text = 10
if id is int
    print("É um número!")
end
```
