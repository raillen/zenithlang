# Módulo 07: Teoria da Geração de Código (Codegen)

**Objetivo**: Entender o processo final de tradução da AST para código que o computador (ou outra linguagem) possa executar.  
**Público-alvo**: Estudantes de ciência da computação e criadores de linguagens.  
**Contexto**: Trilha Teórica (Agnóstica)

## Conteúdo Principal

A **Geração de Código** é a etapa onde o compilador age como um tradutor. Ele recebe a Árvore Sintática Abstrata (AST) — que é uma estrutura lógica pura — e emite texto ou bytes em uma linguagem alvo (como Lua, JavaScript, C ou Assembly).

### 1. O Padrão Emitter
A técnica mais comum é o **Emitter**. Imagine que você está percorrendo a árvore de cima para baixo (Top-Down). Para cada nó, você tem uma "regra de tradução".

- Viu um nó de **Adição**? Escreva `(esquerda + direita)`.
- Viu um nó de **Função**? Escreva a palavra-chave da linguagem alvo para função, os nomes dos parâmetros e o corpo.

### 2. Preservação de Semântica
O maior desafio da geração de código é garantir que o que o programador quis dizer na Linguagem A (Zenith) se comporte exatamente da mesma forma na Linguagem B (Lua).
- Se o Zenith diz que `1 / 2` deve ser divisão inteira, o Codegen deve emitir o comando de divisão inteira da linguagem alvo (ex: `//` em Python ou `math.floor(1/2)` em Lua), mesmo que a linguagem alvo use `/` para divisão decimal.

### 3. Mapeamento de Conceitos (High to Low Level)
Às vezes, a linguagem alvo não possui um conceito que a sua linguagem tem. 
- **Exemplo**: O Zenith tem `traits`, mas o Lua não tem. 
- **Solução**: O Codegen "desenha" as traits no Lua usando tabelas e metatabelas. Esse processo de simplificação é chamado de **Lowering**.

#### Pseudocódigo de um Emitter Simples
```text
FUNCAO emitir_if(no_if):
    escrever("if ")
    emitir_expressao(no_if.condicao)
    escrever(" then\n")
    emitir_bloco(no_if.corpo)
    escrever("end\n")
```

## Decisões e Justificativas

- **Decisão**: Gerar código legível ("Pretty Printing").
- **Motivo**: Para compiladores que transpilam para outras linguagens de alto nível (Source-to-Source), é vital que o código gerado seja legível para humanos. Isso facilita o debug e a confiança do desenvolvedor na ferramenta.

## Exemplos Comentados

### Traduzindo um `match` para `if/else`
Se a linguagem alvo não tem um comando `match`, o Codegen a transforma em uma sequência de `if/elseif`. 
- **Origem**: `match x case 1: print("A") end`
- **Destino**: `if x == 1 then print("A") end`

## Impactos na Implementação
O Codegen é o componente mais "viciado" na linguagem alvo. Se você quiser que o Zenith rode no navegador, você não precisa mudar o Lexer ou o Parser; basta criar um novo **JavaScript Codegen**.

---

### 🏆 Desafio Zenith
Imagine que você está criando um Codegen para uma linguagem que não tem o operador `!=` (diferente), apenas o `NOT` e o `==`.
1. Como o seu Codegen traduziria a expressão `x != y`?
2. Por que é importante que o Codegen lide com isso em vez de obrigar o usuário a escrever o formato mais longo?

**Status**: Teoria de Geração Concluída
