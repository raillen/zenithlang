# Especificação Técnica: Gramática e Sintaxe

Este documento define as regras formais que governam a estrutura do código-fonte Zenith. O compilador utiliza um algoritmo Pratt Parser para resolver precedências de forma eficiente e previsível.

## 1. Conjunto de Caracteres e Lexer

Zenith utiliza codificação UTF-8. O analisador léxico (Lexer) ignora espaços em branco e comentários, transformando o texto em um fluxo de tokens.

### Comentários
- **Linha Única**: Iniciados por `--`. Todo o texto até o fim da linha é ignorado.
- **Bloco**: Zenith não suporta nativamente comentários de bloco para manter a simplicidade visual.

### Identificadores
Nomes de variáveis, funções e tipos devem:
- Começar com uma letra (a-z, A-Z) ou underscore (`_`).
- Seguir com letras, números ou underscores.
- Não ser uma palavra-chave reservada.

---

## 2. Palavras-Chave Reservadas

| Categoria | Palavras-Chave |
| :--- | :--- |
| Declaração | var, const, global, func, async, struct, enum, trait, apply |
| Fluxo | if, elif, else, match, case, while, for, in, repeat, times, break, continue, return |
| Lógica | and, or, not, where |
| Literais | true, false, null, self, it |
| Módulos | namespace, import, export, as, to |
| Escape | native, extern |

---

## 3. Operadores e Precedência

Os operadores são processados de acordo com os seguintes níveis de precedência (do menor para o maior):

1.  Atribuição: =, +=, -=, *=, /=
2.  Lógicos: or, and
3.  Comparação: ==, !=, <, <=, >, >=
4.  Adição/Concatenação: +, -, ..
5.  Multiplicação: *, /, %
6.  Unários: not, -, # (len), ? (optional), ! (bang)
7.  Acesso: . (membro), [ (índice), ( (chamada)

---

## 4. Estruturas de Bloco

Zenith utiliza o delimitador end para fechar todos os blocos de escopo.

### Regras de Escopo
- Blocos do/end criam um novo escopo léxico.
- Variáveis declaradas dentro de um bloco não são visíveis fora dele (shadowing é permitido).

### Estrutura de Função
```zenith
func nome(parametro: tipo) -> tipo_retorno
    -- corpo
end
```

### Estrutura Condicional
```zenith
if condicao
    -- código
elif outra_condicao
    -- código
else
    -- código
end
```

---
*Zenith Specification v0.3.0*
