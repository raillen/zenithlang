# Módulo 02: Implementação do Lexer (Projeto Zenith)

**Objetivo**: Analisar como o Lexer do Zenith é construído em Lua.  
**Público-alvo**: Contribuidores e desenvolvedores que queiram entender o código-fonte.  
**Contexto**: Trilha de Implementação (Código Real)

## Conteúdo Principal

O Lexer do Zenith está localizado em `src/syntax/lexer/lexer.lua`. Ele é uma classe Lua que mantém o estado da leitura (posição atual, texto-fonte e diagnósticos).

### A Estrutura da Classe
```lua
function Lexer.new(source_text)
    local self = setmetatable({}, Lexer)
    self.source = source_text
    self.text = source_text.text
    self.pos = 1
    self.diagnostics = DiagnosticBag.new()
    return self
end
```
O Zenith utiliza um cursor (`self.pos`) que avança conforme os tokens são reconhecidos.

### Curiosidades do Lexer Zenith

#### 1. Verificando sem Consumir (`peek`)
Muitas vezes precisamos olhar o próximo caractere sem mover o cursor (ex: para saber se um `.` é o início de um float ou um operador `..`).
```lua
function Lexer:peek(offset)
    local p = self.pos + (offset or 1)
    if p > #self.text then return "\0" end
    return self.text:sub(p, p)
end
```

#### 2. Scan de Números
O Zenith suporta inteiros e floats. A lógica verifica a presença de um ponto decimal, mas garante que não seja o operador de intervalo (`..`).
```lua
function Lexer:_read_number()
    local start = self.pos
    local has_dot = false
    while not self:is_at_end() and (is_digit(self:current()) or self:current() == ".") do
        if self:current() == "." then
            if self:peek() == "." then break end -- É o início de um range '..'
            has_dot = true
        end
        self:advance()
    end
    -- ... cria o token
end
```

#### 3. Interpolação de Strings (O Truque Sujo)
O diferencial do Lexer do Zenith é como ele trata `"Olá {nome}"`. Em vez de criar um nó complexo na AST, o Lexer **desmembra** a string em múltiplos tokens em tempo de leitura:
`STRING_LITERAL("Olá ")` + `PLUS` + `LPAREN` + `IDENTIFIER(nome)` + `RPAREN` + `PLUS` + `STRING_LITERAL("")`.
Isso simplifica drasticamente o trabalho do Parser, que enxerga apenas uma soma de strings comum.

## Decisões e Justificativas

- **Decisão**: Retornar uma lista de tokens de uma vez (`tokenize()`).
- **Motivo**: Para uma linguagem leve como o Zenith, processar todos os tokens na memória é extremamente rápido e simplifica o debug, pois podemos imprimir a lista completa de tokens antes de iniciar o parsing.

## Exemplos de Código Real
Veja o arquivo [lexer.lua](file:///C:/Users/raillen.DESKTOP-99RJ5M6/Documents/Projetos/zenith-lang-v2/src/syntax/lexer/lexer.lua) para ver a implementação completa do método `_read_operator`, que lida com todos os símbolos da linguagem.

## Impactos na Implementação
O uso de uma `buffer` interna no Lexer permite que ele "injete" tokens extras (como parênteses em strings interpoladas) sem precisar alterar o arquivo original no disco.

---

### 🏆 Desafio Zenith
Abra o arquivo `src/syntax/lexer/lexer.lua` e localize a função `_read_operator`. 
1. Adicione um novo operador fictício `@@` retornando um `TokenKind.AT_AT` (que você deve criar em `token_kind.lua`).
2. Como você garantiria que o Lexer não confundisse `@@` com dois tokens `@` seguidos?

**Status**: Implementação Zenith Analisada
