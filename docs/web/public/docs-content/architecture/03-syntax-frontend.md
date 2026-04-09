# Zenith Implementation: Frontend Sintático

## Resumo rápido
Este módulo cobre o frontend do compilador: leitura do arquivo, diagnósticos, tokens, lexer, AST e parser. O foco é transformar texto Zenith em uma árvore sintática rica, sem ainda tomar decisões semânticas.

## Quando usar
Leia este documento quando estiver implementando gramática, ajustando precedência de operadores, melhorando mensagens de erro sintático ou desenhando novos nós da AST.

## Pré-requisitos
- Entender a separação entre frontend e backend.
- Conhecer a proposta de camadas do compilador.
- Saber, em alto nível, como funciona um lexer e um parser.

## Tempo de leitura
12 a 14 minutos.

## Exemplo mínimo
```text
"state count: int = 0"
  -> tokens
  -> AST sintática
  -> semântica entra só depois
```

## 1. Entrada e localização

O frontend começa em `source/`, onde o compilador guarda:

- o texto bruto do arquivo;
- os intervalos (`span`);
- a conversão de offsets para linha e coluna.

Isso é importante porque diagnósticos bons dependem de localização precisa desde o primeiro passo.

## 2. Diagnósticos como contrato do frontend

Antes mesmo do parser, o compilador precisa ter uma maneira consistente de relatar problemas. A camada `diagnostics/` existe para isso.

Os diagnósticos do frontend devem:

- apontar o trecho exato do erro;
- explicar o que foi encontrado;
- sugerir, quando possível, o formato esperado.

Exemplo de princípio:

- “esperado `end` para fechar bloco `if` iniciado na linha 12” é melhor do que “erro de sintaxe”.

## 3. Tokens e palavras-chave

O lexer precisa reconhecer tanto a forma visual do Zenith quanto suas convenções de legibilidade.

Palavras-chave centrais:

```text
const, redo, global, state, computed, watch, namespace
pub, struct, enum, trait, apply, func, return
async, await, if, elif, else, match, case
for, in, while, repeat, times, break, continue
and, or, not, where, grid, uniq
self, it, _, import, export, as, to
attempt, rescue, true, false, null
```

Operadores e formas relevantes do Zenith:

- comentários com `--`;
- strings com interpolação `{expr}`;
- intervalos com `..`;
- nullable com `?`;
- união com `|`;
- bang operator `expr!`.

## 4. Regras do lexer

O lexer transforma caracteres em tokens, preservando o máximo de informação útil para o parser e para os diagnósticos.

Regras importantes:

- comentários de linha única com `--`;
- strings entre aspas duplas;
- interpolação embutida em strings;
- ranges como `0..5`;
- `text?` como shorthand de nullable;
- `int | text` como união;
- `expr!` como forma sintática distinta.

O lexer deve ser conservador: ele identifica forma, mas não “adivinha” significado semântico.

## 5. AST sintática

A AST deve modelar o que o usuário escreveu, não o que o compilador concluiu semanticamente.

Famílias principais de nós:

### Expressões

```text
LiteralExpr
BinaryExpr
UnaryExpr
IdentifierExpr
CallExpr
MemberExpr
IndexExpr
RangeExpr
MatchExpr
InterpolationExpr
LambdaExpr
BangExpr
```

### Statements

```text
AssignStmt
IfStmt
WhileStmt
ForInStmt
RepeatTimesStmt
BreakStmt
ContinueStmt
ReturnStmt
WatchStmt
MatchStmt
AttemptStmt
```

### Declarações

```text
VarDecl
ConstDecl
GlobalDecl
StateDecl
ComputedDecl
FuncDecl
AsyncFuncDecl
StructDecl
EnumDecl
TraitDecl
ApplyDecl
ImportDecl
ExportDecl
RedoDecl
NamespaceDecl
AttributeNode
```

## 6. Parser e recuperação de erro

O parser coordena a construção da AST.

Responsabilidades:

- consumir tokens;
- aplicar precedência de operadores;
- reconhecer blocos;
- produzir uma `compilation_unit_syntax`.

Estratégia recomendada:

- Pratt parser para expressões;
- recursive descent para declarações e statements.

Recuperação de erro:

- ao encontrar token inesperado, avançar até pontos de sincronização como `end`, `func`, `struct` ou uma nova declaração claramente iniciada;
- continuar produzindo AST parcial quando isso ajudar a emitir mais diagnósticos úteis.

## 7. Invariantes do frontend

O frontend sintático deve obedecer a estas regras:

- o parser nunca resolve nomes;
- o parser nunca verifica tipos;
- o parser nunca emite Lua;
- nenhum nó da AST contém decisões de lowering;
- diagnósticos sintáticos sempre carregam contexto suficiente para leitura rápida.

## 8. Exemplo mental do fluxo

Considere:

```zt
func soma(a: int, b: int) -> int
    return a + b
end
```

O frontend faz, em ordem:

1. reconhece tokens como `func`, identificador, parâmetros, `->`, tipo e `end`;
2. monta um `FuncDecl`;
3. cria um `ReturnStmt` com `BinaryExpr`;
4. registra spans para que erros futuros apontem para o local correto.

Nada disso ainda decide se `a` e `b` existem semanticamente ou se o retorno combina com o tipo declarado. Essa parte fica para a camada semântica.

## 9. Checklist para mudanças no frontend

Ao adicionar uma nova forma sintática:

1. atualizar tokens e palavras-chave, se necessário;
2. ajustar o lexer;
3. criar ou ampliar nós da AST;
4. atualizar o parser;
5. adicionar diagnósticos sintáticos;
6. escrever testes de lexer e parser.

Esse ciclo curto mantém o frontend previsível e reduz regressões cognitivas para quem contribui no compilador.
