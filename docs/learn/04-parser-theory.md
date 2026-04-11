# Módulo 04: Teoria da Análise Sintática (Parsing)

**Objetivo**: Entender o algoritmo que transforma uma fila de tokens na Árvore Sintática Abstrata (AST).  
**Público-alvo**: Estudantes de ciência da computação e criadores de linguagens.  
**Contexto**: Trilha Teórica (Agnóstica)

## Conteúdo Principal

O **Parser** é o coração lógico do compilador. Ele recebe a lista de tokens do Lexer e tenta encontrar uma gramática válida neles. Se o Lexer é sobre "letras", o Parser é sobre "frases".

Existem duas técnicas principais que usamos juntas no Zenith:

### 1. Descida Recursiva (Recursive Descent)
É a técnica mais intuitiva. Para cada regra da linguagem (como um `if` ou uma `function`), criamos uma função no código do Parser.
- Se o Parser vê o token `IF`, ele chama a função `parse_if_statement()`.
- Dentro dessa função, ele espera encontrar uma condição, executa a leitura dela e depois espera encontrar um bloco de código.

#### Pseudocódigo de Descida Recursiva
```text
FUNCAO parse_declaração_de_variável():
    consumir(LET)
    nome = consumir(IDENTIFICADOR)
    consumir(EQUALS)
    valor = parse_expressão()
    RETORNAR NoVariavel(nome, valor)
```

### 2. Algoritmo de Pratt (Pratt Parsing)
A descida recursiva é ótima para estruturas grandes, mas falha miseravelmente em expressões matemáticas como `1 + 2 * 3` devido à precedência.
O **Pratt Parsing** usa uma tabela de "poder de ligação" (binding power) para decidir quem "puxa" quem.
- O operador `*` tem mais poder que o `+`.
- Quando o Parser está no `+`, ele vê o `*` adiante. Como o `*` é mais forte, ele permite que o `*` capture o `2` e o `3` primeiro, e só depois o `+` fecha a conta.

## Decisões e Justificativas

- **Decisão**: Usar Pratt Parsing apenas para expressões.
- **Motivo**: Pratt é extremamente elegante para lidar com profundidade e precedência, mas é overkill (complexo demais) para estruturas simples como `import` ou `func`. Usar o melhor de cada técnica simplifica o código do compilador.

## Exemplos Comentados

### O Erro de Sintaxe
O Parser é o responsável por dizer: "Esperei um `=`, mas encontrei um `WHILE`". 
- Ele mantém o estado de qual token está lendo agora. Se o próximo token não bater com a gramática, ele gera um **Diagnóstico de Erro**.

## Impactos na Implementação
O Parser é geralmente a parte mais densa do compilador. Pequenos erros aqui podem causar o temido "erro em cascata", onde um parêntese esquecido gera 50 mensagens de erro falsas. Por isso, a **Sincronização de Erro** (pular tokens até o próximo `;` ou `end` após um erro) é vital.

---

### 🏆 Desafio Zenith
Imagine que você quer adicionar o operador `**` (potência) na sua linguagem.
1. Onde ele ficaria na tabela de precedência? Acima ou abaixo da multiplicação `*`?
2. Se você estivesse usando Recursive Descent puro (sem Pratt), como você lidaria com o fato de que `2 ^ 3 ^ 2` deve ser processado da direita para a esquerda (`2 ^ (3 ^ 2)`)?

**Status**: Teoria de Parsing Concluída
