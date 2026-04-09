# Módulo 04: Implementação do Parser (Projeto Zenith)

**Objetivo**: Analisar a arquitetura híbrida do Parser do Zenith e como ele lida com a precedência.  
**Público-alvo**: Contribuidores e desenvolvedores que queiram entender a lógica de processamento sintático.  
**Contexto**: Trilha de Implementação (Código Real)

## Conteúdo Principal

O Parser do Zenith não é um arquivo gigante, mas sim um conjunto de módulos especializados localizados em `src/syntax/parser/`.

### 1. O Coordenador (`parser.lua`)
O ponto de entrada principal é o método `Parser.parse()`. Ele coordena o Lexer e depois entra num loop simples de leitura de declarações:
```lua
while not ctx:is_at_end() do
    local node = ParseDeclarations.parse_declaration_or_statement(ctx)
    table.insert(declarations, node)
    ctx:skip_newlines()
end
```

### 2. Descida Recursiva (`parse_declarations.lua`)
Para grandes estruturas como `func`, `struct` ou `if`, usamos a técnica de Descida Recursiva. O Parser "decide" o que ler com base no primeiro token (Lookahead).
- Se encontrar o token `KW_FUNC`, ele chama a função específica para processar uma função.

### 3. A Magia do Pratt (`parse_expressions.lua`)
O Zenith usa um **Pratt Parser** para expressões. A função central é `parse_expression(ctx, precedence)`.
Ela utiliza uma tabela de precedência (`OperatorTable`) para decidir quando parar de "puxar" operandos para a esquerda.

#### Exemplo de Chamada Recursiva do Pratt:
```lua
local prec = OperatorTable.binary_precedence(kind)
if prec <= precedence then break end -- Para se o próximo operador for mais fraco

local operator = ctx:advance()
local assoc = OperatorTable.binary_assoc(kind)
-- Chama-se a si mesmo recursivamente com a nova precedência
local right = ParseExpressions.parse_expression(ctx, prec - (assoc == "RIGHT" and 1 or 0))
```

### 4. O Contexto do Parser (`parser_context.lua`)
Para não carregar o estado em todos os lugares, usamos um objeto `ctx` (Context) que mantém:
- A lista de tokens.
- A posição atual (`ctx.pos`).
- Métodos utilitários como `expect` (garante que o próximo token seja o correto) e `match` (consome se bater, ignora se não).

## Decisões e Justificativas

- **Decisão**: Modularizar o Parser por tipo de nó (`parse_types.lua`, `parse_expressions.lua`, etc).
- **Motivo**: Facilita a manutenção. Se você quiser mudar como o `match` funciona, você só precisa mexer no arquivo de `parse_statements.lua`. Isso reduz a carga cognitiva de lidar com arquivos de mil linhas.

## Exemplos de Código Real
Veja como o Zenith lida com a precedência no arquivo [parse_expressions.lua](file:///C:/Users/raillen.DESKTOP-99RJ5M6/Documents/Projetos/zenith-lang-v2/src/syntax/parser/parse_expressions.lua). Procure pela função `parse_expression` e veja como ela diferencia prefixos de infixos.

## Impactos na Implementação
A escolha por um Parser manual (em vez de um gerador como Bison ou ANTLR) permite que o Zenith tenha mensagens de erro muito mais amigáveis e específicas, pois temos controle total sobre o que acontece quando a gramática falha.

---

### 🏆 Desafio Zenith
Abra o arquivo `src/syntax/parser/parse_expressions.lua`.
1. Localize a função `_parse_primary`. Esta função lida com os "átomos" da linguagem (literais, nomes, parênteses).
2. Tente adicionar suporte a um novo literal literal `nil` (caso não exista) consumindo o token `KW_NULL`.
3. Por que chamamos os parênteses de "agrupamento" e como eles resetam a precedência para zero?

**Status**: Implementação de Parsing Analisada
