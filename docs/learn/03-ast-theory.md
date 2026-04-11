# Módulo 03: Teoria da Árvore Sintática Abstrata (AST)

**Objetivo**: Entender por que precisamos transformar uma lista de tokens em uma estrutura de árvore.  
**Público-alvo**: Estudantes de ciência da computação e criadores de linguagens.  
**Contexto**: Trilha Teórica (Agnóstica)

## Conteúdo Principal

Até agora, o nosso compilador vê o código como uma "fila" (lista linear) de tokens. Mas as linguagens de programação não são lineares; elas têm **hierarquia**.

### O que é uma AST?
A **Árvore Sintática Abstrata (AST)** é uma representação simplificada da estrutura do código. Chamamos de "Abstrata" porque ela ignora detalhes superficiais (como parênteses ou delimitadores) e foca apenas na relação entre as partes.

### Por que uma Árvore?
Imagine a expressão: `1 + 2 * 3`.
- Se lermos linearmente, podemos pensar que é `(1 + 2) * 3 = 9`.
- Mas a matemática diz que é `1 + (2 * 3) = 7`.

Uma árvore resolve isso naturalmente colocando a operação de maior precedência mais "fundo" na estrutura:
```text
      +
     / \
    1   *
       / \
      2   3
```

### Tipos de Nós (Base)
Numa AST, cada "caixa" é um nó. Os principais tipos são:
1. **Expressões**: Produzem um valor (ex: `1 + 1`, `true`, `minha_var`).
2. **Declarações**: Criam algo novo no sistema (ex: declarar uma variável, uma função).
3. **Instruções (Statements)**: Realizam uma ação (ex: `if`, `while`, `print`).

#### Pseudocódigo de um Nó de AST
```text
ESTRUTURA NoBinario:
    tipo: "ADICAO"
    esquerda: No
    direita: No

ESTRUTURA NoVariavel:
    nome: "x"
    tipo_esperado: NoTipo
```

## Decisões e Justificativas

- **Decisão**: A AST deve ser "burra" (semântica mínima).
- **Motivo**: O Parser deve focar apenas em descobrir a estrutura. Se tentarmos validar tipos ou nomes no meio do parsing, o código fica confuso e difícil de manter. Deixamos as "regras de negócio" da linguagem para o **Binder**.

## Exemplos Comentados

### O Código: `let x = 10`
A AST gerada teria essa cara:
- Raiz: `VariableDeclaration`
    - Nome: `x`
    - Inicializador: `Literal(10)`

Perceba que a palavra-chave `let` sumiu! Ela foi usada pelo Parser para identificar que era uma declaração de variável, mas agora que já sabemos o que é, a palavra `let` não tem mais utilidade.

## Impactos na Implementação
A AST é o "contrato" entre o Parser (quem escreve na árvore) e o Codegen (quem lê a árvore). Se você mudar a estrutura de um nó da AST, precisará atualizar todos os componentes do compilador.

---

### 🏆 Desafio Zenith
Tente desenhar (em papel ou mentalmente) a árvore para: `if x > 0 print("Oi") end`.
1. Qual nó seria a raiz?
2. Onde ficaria a condição `x > 0`? Ela é uma Expressão ou uma Instrução?

**Status**: Teoria de Estrutura Concluída
