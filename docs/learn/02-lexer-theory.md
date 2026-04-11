# Módulo 02: Teoria da Tokenização (Lexing)

**Objetivo**: Entender como um compilador "lê" o texto e o transforma em símbolos lógicos.  
**Público-alvo**: Estudantes de ciência da computação e criadores de linguagens.  
**Contexto**: Trilha Teórica (Agnóstica)

## Conteúdo Principal

A primeira etapa de qualquer compilador é o **Lexing** (ou Escaneamento). Imagine que você está lendo uma frase. Seus olhos não veem palavras inteiras de uma vez; eles veem letras individuais e seu cérebro as agrupa em unidades com significado.

### O que é um Token?
Um **Token** é a menor unidade significativa de uma linguagem. 
- No texto `let x = 10`, as letras `l`, `e`, `t` sozinhas não significam nada para o compilador, mas o grupo `let` é um token que significa "Vou declarar algo".

### Como o Lexer funciona?
O Lexer funciona como um cursor que percorre o arquivo caractere por caractere. Ele decide o que cada grupo de letras significa com base em regras simples:

1. **Ignorar Ruído**: Espaços em branco e comentários são úteis para humanos, mas o compilador os descarta.
2. **Reconhecer Padrões**:
   - Começa com número? É um **Literal Numérico**.
   - Começa com `"`? É uma **String**.
   - Começa com letra? É um **Identificador** (nome de variável) ou uma **Palavra-Chave** (`if`, `func`).
   - É um símbolo (`+`, `-`, `*`)? É um **Operador**.

#### Pseudocódigo de um Scaneamento Simples
```text
ENQUANTO não chegar ao fim do arquivo:
    caractere_atual = ler_próximo()
    
    SE caractere_atual é espaço:
        pular()
        
    SE caractere_atual é número:
        ler_todos_os_dígitos_seguidos()
        CRIAR_TOKEN(NUMERO, valor)
        
    SE caractere_atual é letra:
        ler_todas_as_letras_seguintes()
        SE palavra é "func":
            CRIAR_TOKEN(PALAVRA_CHAVE_FUNC)
        SENÃO:
            CRIAR_TOKEN(IDENTIFICADOR, nome)
FIM_ENQUANTO
```

## Decisões e Justificativas

- **Decisão**: O Lexer deve ser "guloso" (Greedy).
- **Motivo**: Se ele encontrar `>`, ele não para. Ele olha o próximo. Se for `=`, ele cria um único token `>=` em vez de dois tokens separados `>` e `=`. Isso reduz a carga de trabalho do Parser.

## Exemplos Comentados

### A Frase: `x = 42`
1. O Lexer vê `x`. É uma letra. Ele lê até o final da palavra. Token: `IDENTIFIER(x)`.
2. O Lexer vê `=`. É um símbolo. Token: `EQUALS`.
3. O Lexer vê `42`. É um número. Token: `NUMBER(42)`.

## Impactos na Implementação
Mudar uma regra no Lexer (ex: permitir que nomes de variáveis comecem com números) pode quebrar toda a lógica de reconhecimento, por isso o Lexer deve ser extremamente rígido e testado.

---

### 🏆 Desafio Zenith
Imagine que sua linguagem permite nomes de variáveis como `player-1`.
1. Como o seu Lexer diferenciaria isso de uma subtração (`palyer` menos `1`)?
2. Que regra você teria que mudar no seu algoritmo de reconhecimento de letras?

**Status**: Teoria Base Concluída
