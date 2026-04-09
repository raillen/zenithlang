# Módulo 03: Implementação da AST (Projeto Zenith)

**Objetivo**: Analisar como os nós da árvore são definidos e categorizados no Zenith.  
**Público-alvo**: Contribuidores e desenvolvedores que queiram entender a estrutura de dados central do compilador.  
**Contexto**: Trilha de Implementação (Código Real)

## Conteúdo Principal

No Zenith, a AST não é uma classe única, mas um conjunto de tabelas Lua que seguem um contrato comum. O "DNA" de cada nó é o seu `kind`.

### O Catálogo de Espécies (`SyntaxKind`)
O arquivo `src/syntax/ast/syntax_kind.lua` funciona como um dicionário de todos os tipos de nós possíveis. Eles são divididos em quatro grandes grupos:
1. **Expressões** (`LITERAL_EXPR`, `BINARY_EXPR`...): Coisas que retornam valor.
2. **Statements** (`IF_STMT`, `WHILE_STMT`...): Instruções de fluxo.
3. **Declarações** (`FUNC_DECL`, `VAR_DECL`...): Criação de novos nomes.
4. **Tipos** (`NAMED_TYPE`, `UNION_TYPE`...): A estrutura do sistema de tipos.

### Anatomia de um Nó Zenith
Cada nó (table Lua) no Zenith possui, obrigatoriamente:
- `kind`: O tipo do nó (do enum `SyntaxKind`).
- `span`: A localização exata no código-fonte (para mensagens de erro).

#### Exemplo: Um Nó Binário (`1 + 2`)
Um nó do tipo `BINARY_EXPR` teria essa estrutura interna:
```lua
{
    kind = "BINARY_EXPR",
    left = { kind = "LITERAL_EXPR", value = 1, ... },
    operator_token = <Token PLUS>,
    right = { kind = "LITERAL_EXPR", value = 2, ... },
    span = <Span total>
}
```

### Por que usar Strings como Kinds?
No Zenith, as chaves do `SyntaxKind` são strings (ex: `"FUNC_DECL"`).
- **Vantagem**: Facilita o debug. Quando você imprime a árvore, você vê o nome do nó em vez de um número aleatório (como `42`). Isso reduz a carga cognitiva durante o desenvolvimento do compilador.

## Decisões e Justificativas

- **Decisão**: Nós são imutáveis após a criação pelo Parser.
- **Motivo**: Garante que o **Binder** e o **Codegen** possam ler a árvore sem medo de que ela mude "por baixo" deles. Se precisarmos adicionar informações (como o tipo resolvido de uma variável), fazemos isso em uma tabela secundária ou em campos específicos (`node.symbol`), mas a estrutura da árvore permanece a mesma.

## Exemplos de Código Real
Veja o arquivo [syntax_kind.lua](file:///C:/Users/raillen.DESKTOP-99RJ5M6/Documents/Projetos/zenith-lang-v2/src/syntax/ast/syntax_kind.lua). Note como as `STATE_DECL` e `COMPUTED_DECL` estão no mesmo grupo que `VAR_DECL` e `FUNC_DECL`, pois todas elas introduzem novos nomes no escopo.

## Impactos na Implementação
Como o Zenith é transpilado para Lua, a AST deve ser otimizada para ser percorrida rapidamente. Usamos o padrão de "Dispatcher" (um grande `if` ou tabela de busca) no Binder e no Codegen para processar os nós com base no seu `kind`.

---

### 🏆 Desafio Zenith
Abra o arquivo `src/syntax/ast/syntax_kind.lua`.
1. Localize os nós relacionados ao `match`. Existe uma diferença entre `MATCH_EXPR` (que retorna valor) e `MATCH_STMT` (que apenas executa código)?
2. Por que você acha que o Zenith separa esses dois, enquanto outras linguagens tratam tudo como expressão? (Dica: Pense na simplicidade da transpilação para Lua).

**Status**: Implementação de Dados Concluída
