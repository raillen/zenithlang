# Módulo 05: Teoria da Análise Semântica (Binding)

**Objetivo**: Entender como o compilador liga nomes ("x") a conceitos reais (uma variável na memória).  
**Público-alvo**: Estudantes de ciência da computação e criadores de linguagens.  
**Contexto**: Trilha Teórica (Agnóstica)

## Conteúdo Principal

Até o Parser, o compilador sabe que `x = 10` é uma atribuição válida. Mas ele ainda não sabe:
- Quem é `x`? 
- Ele já foi declarado? 
- Ele é uma variável, uma função ou um tipo?

A **Análise Semântica** (ou **Binding**) é o processo de dar significado aos nomes na AST.

### 1. A Tabela de Símbolos (Symbol Table)
Pense na Tabela de Símbolos como a "lista telefônica" do seu código. Ela guarda informações sobre cada nome declarado:
- **Nome**: `atirar`
- **Tipo de Símbolo**: FUNCAO
* **Assinatura**: Recebe `int`, não produz retorno.

### 2. Escopos (Scopes)
O Zenith (como a maioria das linguagens modernas) usa **Escopo Lexical**. Isso significa que a visibilidade de um nome depende de onde ele foi escrito no código.

Imagine uma cebola:
- **Camada Externa (Global)**: Coisas visíveis em qualquer lugar.
- **Camada Média (Namespace/Classe)**: Coisas visíveis dentro de um módulo.
- **Camada Interna (Local)**: Coisas que só existem dentro de uma função ou um `if`.

#### Regra de Busca (Lookup)
Quando o compilador vê um nome, ele olha no escopo mais interno (o local). Se não encontrar, ele olha no pai (o namespace), e assim por diante até o global. Se não encontrar em lugar nenhum, ele gera o erro: `Símbolo não encontrado`.

### 3. Sombreamento (Shadowing)
Ocorre quando você declara um nome no escopo interno que já existe no escopo externo.
```text
let x = 10 (Global)
func teste()
    let x = 5 (Local - Sombra o Global)
end
```
O compilador deve ser capaz de distinguir qual `x` você está usando em cada momento.

## Decisões e Justificativas

- **Decisão**: Resolução de nomes em múltiplas passagens (Multi-pass).
- **Motivo**: Em linguagens simples (scripts), você deve declarar antes de usar. No Zenith, queremos que uma função `A` possa chamar a função `B` mesmo que `B` esteja escrita depois de `A`. Para isso, fazemos uma primeira passagem apenas coletando nomes, e uma segunda validando o uso.

## Exemplos Comentados

### O Erro Semântico
Diferente do erro de sintaxe (frase mal formada), o erro semântico ocorre em frases gramaticalmente corretas, mas logicamente impossíveis:
- `3 + "oi"` -> O Parser aceita (é `Valor Op Valor`), mas o Binder rejeita (Não se soma int com text).

## Impactos na Implementação
O Binder é quem transforma a AST "fria" em uma rede conectada de significados. Sem ele, o gerador de código não saberia para qual endereço de memória apontar.

---

### 🏆 Desafio Zenith
Imagine que você tem um `if` dentro de um `while` dentro de uma `func`.
1. Quantos níveis de escopo existem aqui?
2. Se você declarar uma variável dentro do `if`, ela deve ser visível fora do `while`? Por que a hierarquia de escopos impede que isso aconteça?

**Status**: Teoria Semântica Concluída
