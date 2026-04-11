# Módulo `std.text.regex`

O módulo `std.text.regex` fornece ferramentas soberanas para manipulação de texto através de expressões regulares rítmicas. Ele prioriza o uso do **Fluent Builder**, permitindo a construção de padrões complexos sem o ruído visual das strings de regex tradicionais.

## 1. O Construtor de Padrões (Fluent Builder)

O Zenith introduz o conceito de **Regex Semântico**, onde você constrói o seu padrão passo a passo.

### `regex.new_builder() -> RegexBuilder`
Inicia um novo pipeline de construção de padrão.

### Métodos do `RegexBuilder`

| Método | Regex Equivalente | Descrição |
| :--- | :--- | :--- |
| `.starts_with()` | `^` | O padrão deve começar no início da linha. |
| `.ends_with()` | `$` | O padrão deve terminar no fim da linha. |
| `.literal(text)` | (escaped) | Procura pelo texto exato informado. |
| `.digit()` | `\d` | Procura por qualquer número (0-9). |
| `.alpha()` | `[a-zA-Z]` | Procura por qualquer letra. |
| `.whitespace()` | `\s` | Procura por espaços, tabs ou quebras. |
| `.many()` | `+` | Indica que o item anterior deve ocorrer 1 ou mais vezes. |
| `.optional()` | `?` | Indica que o item anterior é opcional. |
| `.build()` | - | Finaliza a construção e retorna um objeto `Regex`. |

---

## 2. O Objeto `Regex`

Após o `.build()`, você recebe um objeto industrial pronto para operações seguras.

### `.test(input: text) -> bool`
Verifica se o padrão existe dentro do texto. Retorna `true` se encontrado.

### `.find(input: text) -> Optional<Match>`
Busca a primeira ocorrência do padrão. Retorna um objeto `Match` embrulhado em `Optional`.

### `.replace(input: text, replacement: text) -> text`
Substitui as ocorrências do padrão pelo novo texto fornecido.

---

## 3. Exemplo Industrial

```zenith
import std.text.regex as re

-- Validação de ID: Inicia com 3 letras e termina com 2 números
var id_pattern = re.new_builder()
    .starts_with()
    .alpha().many()
    .digit().many()
    .ends_with()
    .build()

if id_pattern.test("player12")
    print("ID Válido")
end
```

---
*Zenith API v0.3.1 - Módulo Regex*
