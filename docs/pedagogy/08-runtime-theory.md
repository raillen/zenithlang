# Módulo 08: Teoria do Runtime e Reatividade

**Objetivo**: Entender como o Zenith gerencia estados que "reagem" a mudanças automaticamente.  
**Público-alvo**: Estudantes de ciência da computação e criadores de linguagens.  
**Contexto**: Trilha Teórica (Agnóstica)

## Conteúdo Principal

O que diferencia o Zenith de uma linguagem de script comum é a sua **Reatividade Nativa**. Ao contrário de ter que atualizar manualmente a tela ou o log sempre que um valor muda, o Zenith faz isso por você.

### 1. O Padrão Observer (Observador)
A reatividade baseia-se no padrão **Observer**. 
- Um **Estado** (`state`) é um objeto que guarda um valor e uma lista de "interessados".
- Um **Observador** (`watch`) é uma função que se registra nessa lista.
Quando o valor do Estado muda, ele percorre sua lista de interessados e avisa: "Ei, mudei! Podem se atualizar".

### 2. Rastreamento de Dependências
O "pulo do gato" do Zenith é que você não precisa dizer quem observa quem. O compilador (com ajuda do runtime) faz isso automaticamente.
- Se você tem um `computed dano_total = forca + bonus`.
- Quando você acessa `forca` dentro da função do `computed`, o sistema registra: "Ah, o `dano_total` depende da `forca`".

### 3. Propagação de Mudanças
Quando um estado pai muda, todos os seus filhos (`computed`) e observadores (`watch`) são disparados em uma reação em cadeia. Isso garante que o estado do seu programa esteja sempre consistente.

#### Pseudocódigo de Reatividade
```text
CLASSE Estado:
    valor: 10
    observadores: []

    FUNCAO set_valor(novo):
        valor = novo
        PARA CADA obs EM observadores:
            obs.executar()
```

## Decisões e Justificativas

- **Decisão**: Reatividade Síncrona por padrão.
- **Motivo**: Quando um valor muda, queremos que as dependências sejam atualizadas imediatamente. Isso evita que o programador leia um valor "velho" (stale) durante a execução de um frame de jogo ou automação, mantendo o baixo atrito cognitivo.

## Exemplos Comentados

### O Fluxo da Vida
1. Você declara `state x = 1`.
2. Você declara `watch x: print(it)`.
3. Você faz `x = 2`.
4. O Runtime percebe a mudança e chama o `print(2)` automaticamente.

## Impactos na Implementação
A reatividade exige que o Runtime seja leve. Se cada mudança de variável gerar uma sobrecarga enorme, a linguagem será lenta. Por isso, usamos o poder das Metatabelas do Lua para interceptar leituras e escritas de forma performática.

---

### 🏆 Desafio Zenith
1. O que acontece se o Estado A mudar o Estado B, e o Estado B mudar o Estado A? Como evitar um "loop infinito" de reatividade?
2. Em um motor de jogo, por que a reatividade é melhor do que checar todos os valores em cada frame (polling)?

**Status**: Teoria de Runtime Concluída
